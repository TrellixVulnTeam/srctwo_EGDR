/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "platform/graphics/ImageDecodingStore.h"

#include <memory>
#include "platform/graphics/ImageFrameGenerator.h"
#include "platform/instrumentation/tracing/TraceEvent.h"
#include "platform/wtf/Threading.h"

namespace blink {

namespace {

static const size_t kDefaultMaxTotalSizeOfHeapEntries = 32 * 1024 * 1024;

}  // namespace

ImageDecodingStore::ImageDecodingStore()
    : heap_limit_in_bytes_(kDefaultMaxTotalSizeOfHeapEntries),
      heap_memory_usage_in_bytes_(0) {}

ImageDecodingStore::~ImageDecodingStore() {
#if DCHECK_IS_ON()
  SetCacheLimitInBytes(0);
  DCHECK(!decoder_cache_map_.size());
  DCHECK(!ordered_cache_list_.size());
  DCHECK(!decoder_cache_key_map_.size());
#endif
}

ImageDecodingStore& ImageDecodingStore::Instance() {
  DEFINE_THREAD_SAFE_STATIC_LOCAL(ImageDecodingStore, store, ());
  return store;
}

bool ImageDecodingStore::LockDecoder(const ImageFrameGenerator* generator,
                                     const SkISize& scaled_size,
                                     ImageDecoder::AlphaOption alpha_option,
                                     ImageDecoder** decoder) {
  DCHECK(decoder);

  MutexLocker lock(mutex_);
  DecoderCacheMap::iterator iter = decoder_cache_map_.find(
      DecoderCacheEntry::MakeCacheKey(generator, scaled_size, alpha_option));
  if (iter == decoder_cache_map_.end())
    return false;

  DecoderCacheEntry* cache_entry = iter->value.get();

  // There can only be one user of a decoder at a time.
  DCHECK(!cache_entry->UseCount());
  cache_entry->IncrementUseCount();
  *decoder = cache_entry->CachedDecoder();
  return true;
}

void ImageDecodingStore::UnlockDecoder(const ImageFrameGenerator* generator,
                                       const ImageDecoder* decoder) {
  MutexLocker lock(mutex_);
  DecoderCacheMap::iterator iter = decoder_cache_map_.find(
      DecoderCacheEntry::MakeCacheKey(generator, decoder));
  SECURITY_DCHECK(iter != decoder_cache_map_.end());

  CacheEntry* cache_entry = iter->value.get();
  cache_entry->DecrementUseCount();

  // Put the entry to the end of list.
  ordered_cache_list_.Remove(cache_entry);
  ordered_cache_list_.Append(cache_entry);
}

void ImageDecodingStore::InsertDecoder(const ImageFrameGenerator* generator,
                                       std::unique_ptr<ImageDecoder> decoder) {
  // Prune old cache entries to give space for the new one.
  Prune();

  std::unique_ptr<DecoderCacheEntry> new_cache_entry =
      DecoderCacheEntry::Create(generator, std::move(decoder));

  MutexLocker lock(mutex_);
  DCHECK(!decoder_cache_map_.Contains(new_cache_entry->CacheKey()));
  InsertCacheInternal(std::move(new_cache_entry), &decoder_cache_map_,
                      &decoder_cache_key_map_);
}

void ImageDecodingStore::RemoveDecoder(const ImageFrameGenerator* generator,
                                       const ImageDecoder* decoder) {
  Vector<std::unique_ptr<CacheEntry>> cache_entries_to_delete;
  {
    MutexLocker lock(mutex_);
    DecoderCacheMap::iterator iter = decoder_cache_map_.find(
        DecoderCacheEntry::MakeCacheKey(generator, decoder));
    SECURITY_DCHECK(iter != decoder_cache_map_.end());

    CacheEntry* cache_entry = iter->value.get();
    DCHECK(cache_entry->UseCount());
    cache_entry->DecrementUseCount();

    // Delete only one decoder cache entry. Ownership of the cache entry
    // is transfered to cacheEntriesToDelete such that object can be deleted
    // outside of the lock.
    RemoveFromCacheInternal(cache_entry, &cache_entries_to_delete);

    // Remove from LRU list.
    RemoveFromCacheListInternal(cache_entries_to_delete);
  }
}

void ImageDecodingStore::RemoveCacheIndexedByGenerator(
    const ImageFrameGenerator* generator) {
  Vector<std::unique_ptr<CacheEntry>> cache_entries_to_delete;
  {
    MutexLocker lock(mutex_);

    // Remove image cache objects and decoder cache objects associated
    // with a ImageFrameGenerator.
    RemoveCacheIndexedByGeneratorInternal(&decoder_cache_map_,
                                          &decoder_cache_key_map_, generator,
                                          &cache_entries_to_delete);

    // Remove from LRU list as well.
    RemoveFromCacheListInternal(cache_entries_to_delete);
  }
}

void ImageDecodingStore::Clear() {
  size_t cache_limit_in_bytes;
  {
    MutexLocker lock(mutex_);
    cache_limit_in_bytes = heap_limit_in_bytes_;
    heap_limit_in_bytes_ = 0;
  }

  Prune();

  {
    MutexLocker lock(mutex_);
    heap_limit_in_bytes_ = cache_limit_in_bytes;
  }
}

void ImageDecodingStore::SetCacheLimitInBytes(size_t cache_limit) {
  {
    MutexLocker lock(mutex_);
    heap_limit_in_bytes_ = cache_limit;
  }
  Prune();
}

size_t ImageDecodingStore::MemoryUsageInBytes() {
  MutexLocker lock(mutex_);
  return heap_memory_usage_in_bytes_;
}

int ImageDecodingStore::CacheEntries() {
  MutexLocker lock(mutex_);
  return decoder_cache_map_.size();
}

void ImageDecodingStore::Prune() {
  TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("blink.image_decoding"),
               "ImageDecodingStore::prune");

  Vector<std::unique_ptr<CacheEntry>> cache_entries_to_delete;
  {
    MutexLocker lock(mutex_);

    // Head of the list is the least recently used entry.
    const CacheEntry* cache_entry = ordered_cache_list_.Head();

    // Walk the list of cache entries starting from the least recently used
    // and then keep them for deletion later.
    while (cache_entry) {
      const bool is_prune_needed =
          heap_memory_usage_in_bytes_ > heap_limit_in_bytes_ ||
          !heap_limit_in_bytes_;
      if (!is_prune_needed)
        break;

      // Cache is not used; Remove it.
      if (!cache_entry->UseCount())
        RemoveFromCacheInternal(cache_entry, &cache_entries_to_delete);
      cache_entry = cache_entry->Next();
    }

    // Remove from cache list as well.
    RemoveFromCacheListInternal(cache_entries_to_delete);
  }
}

template <class T, class U, class V>
void ImageDecodingStore::InsertCacheInternal(std::unique_ptr<T> cache_entry,
                                             U* cache_map,
                                             V* identifier_map) {
  const size_t cache_entry_bytes = cache_entry->MemoryUsageInBytes();
  heap_memory_usage_in_bytes_ += cache_entry_bytes;

  // m_orderedCacheList is used to support LRU operations to reorder cache
  // entries quickly.
  ordered_cache_list_.Append(cache_entry.get());

  typename U::KeyType key = cache_entry->CacheKey();
  typename V::AddResult result = identifier_map->insert(
      cache_entry->Generator(), typename V::MappedType());
  result.stored_value->value.insert(key);
  cache_map->insert(key, std::move(cache_entry));

  TRACE_COUNTER1(TRACE_DISABLED_BY_DEFAULT("blink.image_decoding"),
                 "ImageDecodingStoreHeapMemoryUsageBytes",
                 heap_memory_usage_in_bytes_);
  TRACE_COUNTER1(TRACE_DISABLED_BY_DEFAULT("blink.image_decoding"),
                 "ImageDecodingStoreNumOfDecoders", decoder_cache_map_.size());
}

template <class T, class U, class V>
void ImageDecodingStore::RemoveFromCacheInternal(
    const T* cache_entry,
    U* cache_map,
    V* identifier_map,
    Vector<std::unique_ptr<CacheEntry>>* deletion_list) {
  const size_t cache_entry_bytes = cache_entry->MemoryUsageInBytes();
  DCHECK_GE(heap_memory_usage_in_bytes_, cache_entry_bytes);
  heap_memory_usage_in_bytes_ -= cache_entry_bytes;

  // Remove entry from identifier map.
  typename V::iterator iter = identifier_map->find(cache_entry->Generator());
  DCHECK(iter != identifier_map->end());
  iter->value.erase(cache_entry->CacheKey());
  if (!iter->value.size())
    identifier_map->erase(iter);

  // Remove entry from cache map.
  deletion_list->push_back(cache_map->Take(cache_entry->CacheKey()));

  TRACE_COUNTER1(TRACE_DISABLED_BY_DEFAULT("blink.image_decoding"),
                 "ImageDecodingStoreHeapMemoryUsageBytes",
                 heap_memory_usage_in_bytes_);
  TRACE_COUNTER1(TRACE_DISABLED_BY_DEFAULT("blink.image_decoding"),
                 "ImageDecodingStoreNumOfDecoders", decoder_cache_map_.size());
}

void ImageDecodingStore::RemoveFromCacheInternal(
    const CacheEntry* cache_entry,
    Vector<std::unique_ptr<CacheEntry>>* deletion_list) {
  if (cache_entry->GetType() == CacheEntry::kTypeDecoder) {
    RemoveFromCacheInternal(static_cast<const DecoderCacheEntry*>(cache_entry),
                            &decoder_cache_map_, &decoder_cache_key_map_,
                            deletion_list);
  } else {
    DCHECK(false);
  }
}

template <class U, class V>
void ImageDecodingStore::RemoveCacheIndexedByGeneratorInternal(
    U* cache_map,
    V* identifier_map,
    const ImageFrameGenerator* generator,
    Vector<std::unique_ptr<CacheEntry>>* deletion_list) {
  typename V::iterator iter = identifier_map->find(generator);
  if (iter == identifier_map->end())
    return;

  // Get all cache identifiers associated with generator.
  Vector<typename U::KeyType> cache_identifier_list;
  CopyToVector(iter->value, cache_identifier_list);

  // For each cache identifier find the corresponding CacheEntry and remove it.
  for (size_t i = 0; i < cache_identifier_list.size(); ++i) {
    DCHECK(cache_map->Contains(cache_identifier_list[i]));
    const auto& cache_entry = cache_map->at(cache_identifier_list[i]);
    DCHECK(!cache_entry->UseCount());
    RemoveFromCacheInternal(cache_entry, cache_map, identifier_map,
                            deletion_list);
  }
}

void ImageDecodingStore::RemoveFromCacheListInternal(
    const Vector<std::unique_ptr<CacheEntry>>& deletion_list) {
  for (size_t i = 0; i < deletion_list.size(); ++i)
    ordered_cache_list_.Remove(deletion_list[i].get());
}

}  // namespace blink
