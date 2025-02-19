// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "components/browsing_data/core/pref_names.h"
#include "ios/chrome/browser/browsing_data/cache_counter.h"
#include "ios/web/public/browser_state.h"
#include "ios/web/public/web_thread.h"
#include "net/base/completion_callback.h"
#include "net/base/net_errors.h"
#include "net/disk_cache/disk_cache.h"
#include "net/http/http_cache.h"
#include "net/http/http_transaction_factory.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"

namespace {

class IOThreadCacheCounter {
 public:
  IOThreadCacheCounter(
      const scoped_refptr<net::URLRequestContextGetter>& context_getter,
      const net::CompletionCallback& result_callback)
      : next_step_(STEP_GET_BACKEND),
        context_getter_(context_getter),
        result_callback_(result_callback),
        result_(0),
        backend_(nullptr) {}

  void Count() {
    web::WebThread::PostTask(web::WebThread::IO, FROM_HERE,
                             base::Bind(&IOThreadCacheCounter::CountInternal,
                                        base::Unretained(this), net::OK));
  }

 private:
  enum Step {
    STEP_GET_BACKEND,  // Get the disk_cache::Backend instance.
    STEP_COUNT,        // Run CalculateSizeOfAllEntries() on it.
    STEP_CALLBACK,     // Respond on the UI thread.
    STEP_DONE          // Calculation completed.
  };

  void CountInternal(int rv) {
    DCHECK_CURRENTLY_ON(web::WebThread::IO);

    while (rv != net::ERR_IO_PENDING && next_step_ != STEP_DONE) {
      // In case of an error, skip to the last step.
      if (rv < 0)
        next_step_ = STEP_CALLBACK;

      // Process the counting in three steps: STEP_GET_BACKEND -> STEP_COUNT ->
      // -> STEP_CALLBACK.
      switch (next_step_) {
        case STEP_GET_BACKEND: {
          next_step_ = STEP_COUNT;

          net::HttpCache* http_cache = context_getter_->GetURLRequestContext()
                                           ->http_transaction_factory()
                                           ->GetCache();

          rv = http_cache->GetBackend(
              &backend_, base::Bind(&IOThreadCacheCounter::CountInternal,
                                    base::Unretained(this)));
          break;
        }

        case STEP_COUNT: {
          next_step_ = STEP_CALLBACK;

          DCHECK(backend_);
          rv = backend_->CalculateSizeOfAllEntries(base::Bind(
              &IOThreadCacheCounter::CountInternal, base::Unretained(this)));
          break;
        }

        case STEP_CALLBACK: {
          next_step_ = STEP_DONE;
          result_ = rv;

          web::WebThread::PostTask(
              web::WebThread::UI, FROM_HERE,
              base::Bind(&IOThreadCacheCounter::OnCountingFinished,
                         base::Unretained(this)));

          break;
        }

        case STEP_DONE: {
          NOTREACHED();
        }
      }
    }
  }

  void OnCountingFinished() {
    DCHECK_CURRENTLY_ON(web::WebThread::UI);
    result_callback_.Run(result_);
    delete this;
  }

  Step next_step_;
  scoped_refptr<net::URLRequestContextGetter> context_getter_;
  net::CompletionCallback result_callback_;
  int result_;
  disk_cache::Backend* backend_;
};

}  // namespace

CacheCounter::CacheCounter(web::BrowserState* browser_state)
    : browser_state_(browser_state), weak_ptr_factory_(this) {}

CacheCounter::~CacheCounter() {}

const char* CacheCounter::GetPrefName() const {
  return browsing_data::prefs::kDeleteCache;
}

void CacheCounter::Count() {
  // TODO(msramek): disk_cache::Backend currently does not implement counting
  // for subsets of cache, only for the entire cache. Thus, we ignore the time
  // period setting and always request counting for the unbounded time interval.
  // It is up to the UI to interpret the results for finite time intervals as
  // upper estimates.
  // IOThreadCacheCounter deletes itself when done.
  (new IOThreadCacheCounter(browser_state_->GetRequestContext(),
                            base::Bind(&CacheCounter::OnCacheSizeCalculated,
                                       weak_ptr_factory_.GetWeakPtr())))
      ->Count();
}

void CacheCounter::OnCacheSizeCalculated(int result_bytes) {
  // A value less than 0 means a net error code.
  if (result_bytes < 0)
    return;

  ReportResult(result_bytes);
}
