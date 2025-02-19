// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/omnibox/browser/answers_cache.h"

#include "base/i18n/case_conversion.h"
#include "base/strings/string_util.h"

AnswersQueryData::AnswersQueryData() {
}
AnswersQueryData::AnswersQueryData(const base::string16& text,
                                   const base::string16& type)
    : full_query_text(text), query_type(type) {
}

AnswersCache::AnswersCache(size_t max_entries) : max_entries_(max_entries) {
}

AnswersCache::~AnswersCache() {
}

AnswersQueryData AnswersCache::GetTopAnswerEntry(const base::string16& query) {
  base::string16 collapsed_query = base::i18n::ToLower(
      base::CollapseWhitespace(query, false));
  for (Cache::iterator it = cache_.begin(); it != cache_.end(); ++it) {
    // If the query text starts with trimmed input, this is valid prefetch data.
    if (base::StartsWith(base::i18n::ToLower(it->full_query_text),
                         collapsed_query, base::CompareCase::SENSITIVE)) {
      // Move the touched item to the front of the list.
      cache_.splice(cache_.begin(), cache_, it);
      return cache_.front();
    }
  }
  return AnswersQueryData();
}

void AnswersCache::UpdateRecentAnswers(const base::string16& full_query_text,
                                       const base::string16& query_type) {
  // If this entry is already part of the cache, just update recency.
  for (Cache::iterator it = cache_.begin(); it != cache_.end(); ++it) {
    if (full_query_text == it->full_query_text &&
        query_type == it->query_type) {
      cache_.splice(cache_.begin(), cache_, it);
      return;
    }
  }

  // Evict if cache size is exceeded.
  if (cache_.size() >= max_entries_)
    cache_.pop_back();

  cache_.push_front(AnswersQueryData(full_query_text, query_type));
}
