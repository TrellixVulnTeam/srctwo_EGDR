// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/net/http_cache_helper.h"

#include <utility>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "net/base/sdch_manager.h"
#include "net/disk_cache/disk_cache.h"
#include "net/http/http_cache.h"
#include "net/http/http_network_session.h"
#include "net/http/http_transaction_factory.h"
#include "net/quic/chromium/quic_stream_factory.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "url/gurl.h"

namespace {

// Posts |callback| on |task_runner|.
void PostCallback(const scoped_refptr<base::TaskRunner>& task_runner,
                  const net::CompletionCallback& callback,
                  int error) {
  task_runner->PostTask(FROM_HERE, base::Bind(callback, error));
}

// Clears the disk_cache::Backend on the IO thread and deletes |backend|.
void DoomHttpCache(std::unique_ptr<disk_cache::Backend*> backend,
                   const scoped_refptr<base::TaskRunner>& client_task_runner,
                   const base::Time& delete_begin,
                   const base::Time& delete_end,
                   const net::CompletionCallback& callback,
                   int error) {
  // |*backend| may be null in case of error.
  if (*backend) {
    (*backend)->DoomEntriesBetween(
        delete_begin, delete_end,
        base::Bind(&PostCallback, client_task_runner, callback));
  } else {
    client_task_runner->PostTask(FROM_HERE, base::Bind(callback, error));
  }
}

// Clears various caches synchronously and the disk_cache::Backend
// asynchronously.
void ClearHttpCacheOnIOThread(
    const scoped_refptr<net::URLRequestContextGetter>& getter,
    const scoped_refptr<base::TaskRunner>& client_task_runner,
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const net::CompletionCallback& callback) {
  net::HttpCache* http_cache =
      getter->GetURLRequestContext()->http_transaction_factory()->GetCache();

  // Clear QUIC server information from memory and the disk cache.
  http_cache->GetSession()
      ->quic_stream_factory()
      ->ClearCachedStatesInCryptoConfig(base::Callback<bool(const GURL&)>());

  // Clear SDCH dictionary state.
  net::SdchManager* sdch_manager =
      getter->GetURLRequestContext()->sdch_manager();
  // The test is probably overkill, since chrome should always have an
  // SdchManager.  But in general the URLRequestContext  is *not*
  // guaranteed to have an SdchManager, so checking is wise.
  if (sdch_manager)
    sdch_manager->ClearData();

  std::unique_ptr<disk_cache::Backend*> backend(
      new disk_cache::Backend*(nullptr));
  disk_cache::Backend** backend_ptr = backend.get();
  net::CompletionCallback doom_callback =
      base::Bind(&DoomHttpCache, base::Passed(std::move(backend)),
                 client_task_runner, delete_begin, delete_end, callback);

  int rv = http_cache->GetBackend(backend_ptr, doom_callback);

  if (rv != net::ERR_IO_PENDING) {
    // GetBackend doesn't call the callback if it completes synchronously, so
    // call it directly here.
    doom_callback.Run(rv);
  }
}

}  // namespace

namespace net {

void ClearHttpCache(const scoped_refptr<net::URLRequestContextGetter>& getter,
                    const scoped_refptr<base::TaskRunner>& network_task_runner,
                    const base::Time& delete_begin,
                    const base::Time& delete_end,
                    const net::CompletionCallback& callback) {
  DCHECK(delete_end != base::Time());
  network_task_runner->PostTask(
      FROM_HERE, base::Bind(&ClearHttpCacheOnIOThread, getter,
                            base::ThreadTaskRunnerHandle::Get(), delete_begin,
                            delete_end, callback));
}

}  // namespace net
