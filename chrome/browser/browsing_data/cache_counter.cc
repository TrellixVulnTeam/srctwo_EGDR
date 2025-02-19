// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/browsing_data/cache_counter.h"
#include "chrome/browser/profiles/profile.h"
#include "components/browsing_data/content/conditional_cache_counting_helper.h"
#include "components/browsing_data/core/pref_names.h"
#include "components/offline_pages/features/features.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/net_errors.h"

#if BUILDFLAG(ENABLE_OFFLINE_PAGES)
#include "chrome/browser/offline_pages/offline_page_utils.h"
#endif  // BUILDFLAG(ENABLE_OFFLINE_PAGES)

CacheCounter::CacheResult::CacheResult(const CacheCounter* source,
                                       int64_t cache_size,
                                       bool is_upper_limit)
    : FinishedResult(source, cache_size),
      cache_size_(cache_size),
      is_upper_limit_(is_upper_limit) {}

CacheCounter::CacheResult::~CacheResult() {}

CacheCounter::CacheCounter(Profile* profile)
    : profile_(profile),
      weak_ptr_factory_(this) {}

CacheCounter::~CacheCounter() {
}

const char* CacheCounter::GetPrefName() const {
  return GetTab() == browsing_data::ClearBrowsingDataTab::BASIC
             ? browsing_data::prefs::kDeleteCacheBasic
             : browsing_data::prefs::kDeleteCache;
}

void CacheCounter::Count() {
  // Cancel existing requests and reset states.
  weak_ptr_factory_.InvalidateWeakPtrs();
  calculated_size_ = 0;
  is_upper_limit_ = false;
  pending_sources_ = 1;
  base::WeakPtr<browsing_data::ConditionalCacheCountingHelper> counter =
      browsing_data::ConditionalCacheCountingHelper::CreateForRange(
          content::BrowserContext::GetDefaultStoragePartition(profile_),
          GetPeriodStart(), base::Time::Max())
          ->CountAndDestroySelfWhenFinished(
              base::Bind(&CacheCounter::OnCacheSizeCalculated,
                         weak_ptr_factory_.GetWeakPtr()));
#if BUILDFLAG(ENABLE_OFFLINE_PAGES)
  if (offline_pages::OfflinePageUtils::GetCachedOfflinePageSizeBetween(
          profile_,
          base::Bind(&CacheCounter::OnCacheSizeCalculated,
                     weak_ptr_factory_.GetWeakPtr(),
                     false /* is_upper_limit */),
          GetPeriodStart(), base::Time::Max())) {
    pending_sources_++;
  }
#endif  // BUILDFLAG(ENABLE_OFFLINE_PAGES)
}

void CacheCounter::OnCacheSizeCalculated(bool is_upper_limit,
                                         int64_t cache_bytes) {
  // A value less than 0 means a net error code.
  if (cache_bytes < 0)
    return;

  pending_sources_--;
  calculated_size_ += cache_bytes;
  is_upper_limit_ |= is_upper_limit;
  if (pending_sources_ == 0) {
    auto result =
        base::MakeUnique<CacheResult>(this, calculated_size_, is_upper_limit_);
    ReportResult(std::move(result));
  }
}
