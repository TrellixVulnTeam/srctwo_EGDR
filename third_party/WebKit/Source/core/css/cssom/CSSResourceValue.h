// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CSSResourceValue_h
#define CSSResourceValue_h

#include "core/css/cssom/CSSStyleValue.h"

#include "platform/loader/fetch/Resource.h"

namespace blink {

class CORE_EXPORT CSSResourceValue : public CSSStyleValue {
  WTF_MAKE_NONCOPYABLE(CSSResourceValue);
  DEFINE_WRAPPERTYPEINFO();

 public:
  virtual ~CSSResourceValue() {}

  const String state() const {
    switch (Status()) {
      case ResourceStatus::kNotStarted:
        return "unloaded";
      case ResourceStatus::kPending:
        return "loading";
      case ResourceStatus::kCached:
        return "loaded";
      case ResourceStatus::kLoadError:
      case ResourceStatus::kDecodeError:
        return "error";
      default:
        NOTREACHED();
        return "";
    }
  }

  DEFINE_INLINE_VIRTUAL_TRACE() { CSSStyleValue::Trace(visitor); }

 protected:
  CSSResourceValue() {}

  virtual ResourceStatus Status() const = 0;
};

}  // namespace blink

#endif  // CSSResourceValue_h
