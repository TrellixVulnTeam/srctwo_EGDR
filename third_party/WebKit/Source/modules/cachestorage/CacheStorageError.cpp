// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/cachestorage/CacheStorageError.h"

#include "core/dom/DOMException.h"
#include "core/dom/ExceptionCode.h"
#include "modules/cachestorage/Cache.h"
#include "public/platform/modules/serviceworker/WebServiceWorkerCacheError.h"

namespace blink {

DOMException* CacheStorageError::CreateException(
    WebServiceWorkerCacheError web_error) {
  switch (web_error) {
    case kWebServiceWorkerCacheErrorNotImplemented:
      return DOMException::Create(kNotSupportedError,
                                  "Method is not implemented.");
    case kWebServiceWorkerCacheErrorNotFound:
      return DOMException::Create(kNotFoundError, "Entry was not found.");
    case kWebServiceWorkerCacheErrorExists:
      return DOMException::Create(kInvalidAccessError, "Entry already exists.");
    case kWebServiceWorkerCacheErrorQuotaExceeded:
      return DOMException::Create(kQuotaExceededError, "Quota exceeded.");
    case kWebServiceWorkerCacheErrorCacheNameNotFound:
      return DOMException::Create(kNotFoundError, "Cache was not found.");
    case kWebServiceWorkerCacheErrorTooLarge:
      return DOMException::Create(kAbortError, "Operation too large.");
  }
  NOTREACHED();
  return nullptr;
}

}  // namespace blink
