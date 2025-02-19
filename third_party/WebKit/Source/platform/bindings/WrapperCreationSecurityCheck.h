// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WrapperCreationSecurityCheck_h
#define WrapperCreationSecurityCheck_h

#include "platform/PlatformExport.h"
#include "platform/wtf/Allocator.h"
#include "v8/include/v8.h"

namespace blink {

struct WrapperTypeInfo;

// This class holds pointers to functions that implement creation context access
// and exception rethrowing logic required by V8WrapperInstantiationScope when
// creating wrappers.
class PLATFORM_EXPORT WrapperCreationSecurityCheck {
  STATIC_ONLY(WrapperCreationSecurityCheck);

 public:
  using SecurityCheckFunction = bool (*)(v8::Local<v8::Context>,
                                         const WrapperTypeInfo*);
  using RethrowExceptionFunction = void (*)(v8::Local<v8::Context>,
                                            const WrapperTypeInfo*,
                                            v8::Local<v8::Value>);

  static void SetSecurityCheckFunction(SecurityCheckFunction);
  static void SetRethrowExceptionFunction(RethrowExceptionFunction);

  static bool VerifyContextAccess(v8::Local<v8::Context> creation_context,
                                  const WrapperTypeInfo*);
  static void RethrowCrossContextException(
      v8::Local<v8::Context> creation_context,
      const WrapperTypeInfo*,
      v8::Local<v8::Value> cross_context_exception);

 private:
  static SecurityCheckFunction security_check_;
  static RethrowExceptionFunction rethrow_exception_;
};

}  // namespace blink

#endif  // WrapperCreationSecurityCheck_h
