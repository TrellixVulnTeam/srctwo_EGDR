/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PositionError_h
#define PositionError_h

#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class PositionError final : public GarbageCollectedFinalized<PositionError>,
                            public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  enum ErrorCode {
    kPermissionDenied = 1,
    kPositionUnavailable = 2,
    kTimeout = 3
  };

  static PositionError* Create(ErrorCode code, const String& message) {
    return new PositionError(code, message);
  }
  DEFINE_INLINE_TRACE() {}

  ErrorCode code() const { return code_; }
  const String& message() const { return message_; }
  void SetIsFatal(bool is_fatal) { is_fatal_ = is_fatal; }
  bool IsFatal() const { return is_fatal_; }

 private:
  PositionError(ErrorCode code, const String& message)
      : code_(code), message_(message), is_fatal_(false) {}

  ErrorCode code_;
  String message_;
  // Whether the error is fatal, such that no request can ever obtain a good
  // position fix in the future.
  bool is_fatal_;
};

}  // namespace blink

#endif  // PositionError_h
