/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CryptoResult_h
#define CryptoResult_h

#include "platform/PlatformExport.h"
#include "platform/wtf/ThreadSafeRefCounted.h"
#include "public/platform/WebCrypto.h"

namespace blink {

// Result cancellation status interface to allow non-Blink webcrypto threads
// to query for status.
class CryptoResultCancel : public ThreadSafeRefCounted<CryptoResultCancel> {
 public:
  virtual ~CryptoResultCancel() {}

  virtual bool Cancelled() const = 0;
};

// Receives notification of completion of the crypto operation.
class PLATFORM_EXPORT CryptoResult
    : public GarbageCollectedFinalized<CryptoResult> {
 public:
  virtual ~CryptoResult() {}

  virtual void CompleteWithError(WebCryptoErrorType, const WebString&) = 0;
  virtual void CompleteWithBuffer(const void* bytes, unsigned bytes_size) = 0;
  virtual void CompleteWithJson(const char* utf8_data, unsigned length) = 0;
  virtual void CompleteWithBoolean(bool) = 0;
  virtual void CompleteWithKey(const WebCryptoKey&) = 0;
  virtual void CompleteWithKeyPair(const WebCryptoKey& public_key,
                                   const WebCryptoKey& private_key) = 0;

  DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}  // namespace blink

#endif  // CryptoResult_h
