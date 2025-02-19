// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/bitmap_fetcher/bitmap_fetcher_service.h"

#include <stddef.h>
#include <utility>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/bitmap_fetcher/bitmap_fetcher.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/storage_partition.h"
#include "net/base/load_flags.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace {

const size_t kMaxRequests = 25;  // Maximum number of inflight requests allowed.
const int kMaxCacheEntries = 5;  // Maximum number of cache entries.

}  // namespace.

class BitmapFetcherRequest {
 public:
  BitmapFetcherRequest(BitmapFetcherService::RequestId request_id,
                       BitmapFetcherService::Observer* observer);
  ~BitmapFetcherRequest();

  void NotifyImageChanged(const SkBitmap* bitmap);
  BitmapFetcherService::RequestId request_id() const { return request_id_; }

  // Weak ptr |fetcher| is used to identify associated fetchers.
  void set_fetcher(const chrome::BitmapFetcher* fetcher) { fetcher_ = fetcher; }
  const chrome::BitmapFetcher* get_fetcher() const { return fetcher_; }

 private:
  const BitmapFetcherService::RequestId request_id_;
  std::unique_ptr<BitmapFetcherService::Observer> observer_;
  const chrome::BitmapFetcher* fetcher_;

  DISALLOW_COPY_AND_ASSIGN(BitmapFetcherRequest);
};

BitmapFetcherRequest::BitmapFetcherRequest(
    BitmapFetcherService::RequestId request_id,
    BitmapFetcherService::Observer* observer)
    : request_id_(request_id), observer_(observer) {
}

BitmapFetcherRequest::~BitmapFetcherRequest() {
}

void BitmapFetcherRequest::NotifyImageChanged(const SkBitmap* bitmap) {
  if (bitmap && !bitmap->empty())
    observer_->OnImageChanged(request_id_, *bitmap);
}

BitmapFetcherService::CacheEntry::CacheEntry() {
}

BitmapFetcherService::CacheEntry::~CacheEntry() {
}

BitmapFetcherService::BitmapFetcherService(content::BrowserContext* context)
    : cache_(kMaxCacheEntries), current_request_id_(1), context_(context) {
}

BitmapFetcherService::~BitmapFetcherService() {
}

void BitmapFetcherService::CancelRequest(int request_id) {
  for (auto iter = requests_.begin(); iter != requests_.end(); ++iter) {
    if ((*iter)->request_id() == request_id) {
      requests_.erase(iter);
      // Deliberately leave the associated fetcher running to populate cache.
      return;
    }
  }
}

BitmapFetcherService::RequestId BitmapFetcherService::RequestImage(
    const GURL& url,
    Observer* observer,
    const net::NetworkTrafficAnnotationTag& traffic_annotation) {
  // Create a new request, assigning next available request ID.
  ++current_request_id_;
  if (current_request_id_ == REQUEST_ID_INVALID)
    ++current_request_id_;
  int request_id = current_request_id_;
  auto request = base::MakeUnique<BitmapFetcherRequest>(request_id, observer);

  // Reject invalid URLs.
  if (!url.is_valid())
    return REQUEST_ID_INVALID;

  // Check for existing images first.
  auto iter = cache_.Get(url);
  if (iter != cache_.end()) {
    BitmapFetcherService::CacheEntry* entry = iter->second.get();
    request->NotifyImageChanged(entry->bitmap.get());

    // There is no request ID associated with this - data is already delivered.
    return REQUEST_ID_INVALID;
  }

  // Limit number of simultaneous in-flight requests.
  if (requests_.size() > kMaxRequests)
    return REQUEST_ID_INVALID;

  // Make sure there's a fetcher for this URL and attach to request.
  const chrome::BitmapFetcher* fetcher =
      EnsureFetcherForUrl(url, traffic_annotation);
  request->set_fetcher(fetcher);

  requests_.push_back(std::move(request));
  return requests_.back()->request_id();
}

void BitmapFetcherService::Prefetch(
    const GURL& url,
    const net::NetworkTrafficAnnotationTag& traffic_annotation) {
  if (url.is_valid())
    EnsureFetcherForUrl(url, traffic_annotation);
}

std::unique_ptr<chrome::BitmapFetcher> BitmapFetcherService::CreateFetcher(
    const GURL& url,
    const net::NetworkTrafficAnnotationTag& traffic_annotation) {
  std::unique_ptr<chrome::BitmapFetcher> new_fetcher(
      new chrome::BitmapFetcher(url, this, traffic_annotation));

  new_fetcher->Init(
      content::BrowserContext::GetDefaultStoragePartition(context_)->
          GetURLRequestContext(),
      std::string(),
      net::URLRequest::CLEAR_REFERRER_ON_TRANSITION_FROM_SECURE_TO_INSECURE,
      net::LOAD_NORMAL);
  new_fetcher->Start();
  return new_fetcher;
}

const chrome::BitmapFetcher* BitmapFetcherService::EnsureFetcherForUrl(
    const GURL& url,
    const net::NetworkTrafficAnnotationTag& traffic_annotation) {
  const chrome::BitmapFetcher* fetcher = FindFetcherForUrl(url);
  if (fetcher)
    return fetcher;

  std::unique_ptr<chrome::BitmapFetcher> new_fetcher =
      CreateFetcher(url, traffic_annotation);
  active_fetchers_.push_back(std::move(new_fetcher));
  return active_fetchers_.back().get();
}

const chrome::BitmapFetcher* BitmapFetcherService::FindFetcherForUrl(
    const GURL& url) {
  for (auto it = active_fetchers_.begin(); it != active_fetchers_.end(); ++it) {
    if (url == (*it)->url())
      return it->get();
  }
  return nullptr;
}

void BitmapFetcherService::RemoveFetcher(const chrome::BitmapFetcher* fetcher) {
  auto it = active_fetchers_.begin();
  for (; it != active_fetchers_.end(); ++it) {
    if (it->get() == fetcher)
      break;
  }
  // RemoveFetcher should always result in removal.
  DCHECK(it != active_fetchers_.end());
  active_fetchers_.erase(it);
}

void BitmapFetcherService::OnFetchComplete(const GURL& url,
                                           const SkBitmap* bitmap) {
  const chrome::BitmapFetcher* fetcher = FindFetcherForUrl(url);
  DCHECK(fetcher);

  // Notify all attached requests of completion.
  auto iter = requests_.begin();
  while (iter != requests_.end()) {
    if ((*iter)->get_fetcher() == fetcher) {
      (*iter)->NotifyImageChanged(bitmap);
      iter = requests_.erase(iter);
    } else {
      ++iter;
    }
  }

  if (bitmap && !bitmap->isNull()) {
    std::unique_ptr<CacheEntry> entry(new CacheEntry);
    entry->bitmap.reset(new SkBitmap(*bitmap));
    cache_.Put(fetcher->url(), std::move(entry));
  }

  RemoveFetcher(fetcher);
}
