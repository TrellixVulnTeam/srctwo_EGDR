/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY GOOGLE INC. AND ITS CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL GOOGLE INC.
 * OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "modules/indexeddb/WebIDBCallbacksImpl.h"

#include <memory>
#include "core/dom/DOMException.h"
#include "core/probe/CoreProbes.h"
#include "modules/IndexedDBNames.h"
#include "modules/indexeddb/IDBMetadata.h"
#include "modules/indexeddb/IDBRequest.h"
#include "modules/indexeddb/IDBValue.h"
#include "platform/SharedBuffer.h"
#include "platform/wtf/PtrUtil.h"
#include "public/platform/modules/indexeddb/WebIDBCursor.h"
#include "public/platform/modules/indexeddb/WebIDBDatabase.h"
#include "public/platform/modules/indexeddb/WebIDBDatabaseError.h"
#include "public/platform/modules/indexeddb/WebIDBKey.h"
#include "public/platform/modules/indexeddb/WebIDBValue.h"

using blink::WebIDBCursor;
using blink::WebIDBDatabase;
using blink::WebIDBDatabaseError;
using blink::WebIDBKey;
using blink::WebIDBKeyPath;
using blink::WebIDBMetadata;
using blink::WebIDBValue;
using blink::WebVector;

namespace blink {

// static
std::unique_ptr<WebIDBCallbacksImpl> WebIDBCallbacksImpl::Create(
    IDBRequest* request) {
  return WTF::WrapUnique(new WebIDBCallbacksImpl(request));
}

WebIDBCallbacksImpl::WebIDBCallbacksImpl(IDBRequest* request)
    : request_(request) {
  probe::AsyncTaskScheduled(request_->GetExecutionContext(),
                            IndexedDBNames::IndexedDB, this);
}

WebIDBCallbacksImpl::~WebIDBCallbacksImpl() {
  if (request_) {
    probe::AsyncTaskCanceled(request_->GetExecutionContext(), this);
#if DCHECK_IS_ON()
    DCHECK_EQ(static_cast<WebIDBCallbacks*>(this), request_->WebCallbacks());
#endif  // DCHECK_IS_ON()
    request_->WebCallbacksDestroyed();
  }
}

void WebIDBCallbacksImpl::OnError(const WebIDBDatabaseError& error) {
  if (!request_)
    return;

  probe::AsyncTask async_task(request_->GetExecutionContext(), this, "error");
  request_->HandleResponse(DOMException::Create(error.Code(), error.Message()));
}

void WebIDBCallbacksImpl::OnSuccess(
    const WebVector<WebString>& web_string_list) {
  if (!request_)
    return;

  Vector<String> string_list;
  for (size_t i = 0; i < web_string_list.size(); ++i)
    string_list.push_back(web_string_list[i]);
  probe::AsyncTask async_task(request_->GetExecutionContext(), this, "success");
#if DCHECK_IS_ON()
  DCHECK(!request_->TransactionHasQueuedResults());
#endif  // DCHECK_IS_ON()
  request_->EnqueueResponse(string_list);
}

void WebIDBCallbacksImpl::OnSuccess(WebIDBCursor* cursor,
                                    const WebIDBKey& key,
                                    const WebIDBKey& primary_key,
                                    const WebIDBValue& value) {
  if (!request_)
    return;

  probe::AsyncTask async_task(request_->GetExecutionContext(), this, "success");
  request_->HandleResponse(WTF::WrapUnique(cursor), key, primary_key,
                           IDBValue::Create(value, request_->GetIsolate()));
}

void WebIDBCallbacksImpl::OnSuccess(WebIDBDatabase* backend,
                                    const WebIDBMetadata& metadata) {
  std::unique_ptr<WebIDBDatabase> db = WTF::WrapUnique(backend);
  if (request_) {
    probe::AsyncTask async_task(request_->GetExecutionContext(), this,
                                "success");
#if DCHECK_IS_ON()
    DCHECK(!request_->TransactionHasQueuedResults());
#endif  // DCHECK_IS_ON()
    request_->EnqueueResponse(std::move(db), IDBDatabaseMetadata(metadata));
  } else if (db) {
    db->Close();
  }
}

void WebIDBCallbacksImpl::OnSuccess(const WebIDBKey& key) {
  if (!request_)
    return;

  probe::AsyncTask async_task(request_->GetExecutionContext(), this, "success");
  request_->HandleResponse(key);
}

void WebIDBCallbacksImpl::OnSuccess(const WebIDBValue& value) {
  if (!request_)
    return;

  probe::AsyncTask async_task(request_->GetExecutionContext(), this, "success");
  request_->HandleResponse(IDBValue::Create(value, request_->GetIsolate()));
}

void WebIDBCallbacksImpl::OnSuccess(const WebVector<WebIDBValue>& values) {
  if (!request_)
    return;

  probe::AsyncTask async_task(request_->GetExecutionContext(), this, "success");
  Vector<RefPtr<IDBValue>> idb_values(values.size());
  for (size_t i = 0; i < values.size(); ++i)
    idb_values[i] = IDBValue::Create(values[i], request_->GetIsolate());
  request_->HandleResponse(idb_values);
}

void WebIDBCallbacksImpl::OnSuccess(long long value) {
  if (!request_)
    return;

  probe::AsyncTask async_task(request_->GetExecutionContext(), this, "success");
  request_->HandleResponse(value);
}

void WebIDBCallbacksImpl::OnSuccess() {
  if (!request_)
    return;

  probe::AsyncTask async_task(request_->GetExecutionContext(), this, "success");
  request_->HandleResponse();
}

void WebIDBCallbacksImpl::OnSuccess(const WebIDBKey& key,
                                    const WebIDBKey& primary_key,
                                    const WebIDBValue& value) {
  if (!request_)
    return;

  probe::AsyncTask async_task(request_->GetExecutionContext(), this, "success");
  request_->HandleResponse(key, primary_key,
                           IDBValue::Create(value, request_->GetIsolate()));
}

void WebIDBCallbacksImpl::OnBlocked(long long old_version) {
  if (!request_)
    return;

  probe::AsyncTask async_task(request_->GetExecutionContext(), this, "blocked");
#if DCHECK_IS_ON()
  DCHECK(!request_->TransactionHasQueuedResults());
#endif  // DCHECK_IS_ON()
  request_->EnqueueBlocked(old_version);
}

void WebIDBCallbacksImpl::OnUpgradeNeeded(long long old_version,
                                          WebIDBDatabase* database,
                                          const WebIDBMetadata& metadata,
                                          unsigned short data_loss,
                                          WebString data_loss_message) {
  std::unique_ptr<WebIDBDatabase> db = WTF::WrapUnique(database);
  if (request_) {
    probe::AsyncTask async_task(request_->GetExecutionContext(), this,
                                "upgradeNeeded");
#if DCHECK_IS_ON()
    DCHECK(!request_->TransactionHasQueuedResults());
#endif  // DCHECK_IS_ON()
    request_->EnqueueUpgradeNeeded(
        old_version, std::move(db), IDBDatabaseMetadata(metadata),
        static_cast<WebIDBDataLoss>(data_loss), data_loss_message);
  } else {
    db->Close();
  }
}

void WebIDBCallbacksImpl::Detach() {
  request_.Clear();
}

}  // namespace blink
