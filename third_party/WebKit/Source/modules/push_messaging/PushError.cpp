// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/push_messaging/PushError.h"

#include "core/dom/DOMException.h"
#include "core/dom/ExceptionCode.h"
#include "platform/wtf/Assertions.h"

namespace blink {

DOMException* PushError::Take(ScriptPromiseResolver*,
                              const WebPushError& web_error) {
  switch (web_error.error_type) {
    case WebPushError::kErrorTypeAbort:
      return DOMException::Create(kAbortError, web_error.message);
    case WebPushError::kErrorTypeInvalidState:
      return DOMException::Create(kInvalidStateError, web_error.message);
    case WebPushError::kErrorTypeNetwork:
      return DOMException::Create(kNetworkError, web_error.message);
    case WebPushError::kErrorTypeNone:
      NOTREACHED();
      return DOMException::Create(kUnknownError, web_error.message);
    case WebPushError::kErrorTypeNotAllowed:
      return DOMException::Create(kNotAllowedError, web_error.message);
    case WebPushError::kErrorTypeNotFound:
      return DOMException::Create(kNotFoundError, web_error.message);
    case WebPushError::kErrorTypeNotSupported:
      return DOMException::Create(kNotSupportedError, web_error.message);
  }
  NOTREACHED();
  return DOMException::Create(kUnknownError);
}

}  // namespace blink
