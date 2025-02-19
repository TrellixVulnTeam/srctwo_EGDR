// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebContentDecryptionModuleResult_h
#define WebContentDecryptionModuleResult_h

#include "WebCommon.h"
#include "WebContentDecryptionModuleException.h"
#include "WebEncryptedMediaKeyInformation.h"
#include "WebPrivatePtr.h"

namespace blink {

class ContentDecryptionModuleResult;
class WebContentDecryptionModule;
class WebString;

class WebContentDecryptionModuleResult {
 public:
  enum SessionStatus {
    // New session has been initialized.
    kNewSession,

    // CDM could not find the requested session.
    kSessionNotFound,

    // CDM already has a non-closed session that matches the provided
    // parameters.
    kSessionAlreadyExists,
  };

  WebContentDecryptionModuleResult(const WebContentDecryptionModuleResult& o) {
    Assign(o);
  }

  ~WebContentDecryptionModuleResult() { Reset(); }

  WebContentDecryptionModuleResult& operator=(
      const WebContentDecryptionModuleResult& o) {
    Assign(o);
    return *this;
  }

  // Called when the CDM completes an operation and has no additional data to
  // pass back.
  BLINK_PLATFORM_EXPORT void Complete();

  // Called when a CDM is created.
  BLINK_PLATFORM_EXPORT void CompleteWithContentDecryptionModule(
      WebContentDecryptionModule*);

  // Called when the CDM completes a session operation.
  BLINK_PLATFORM_EXPORT void CompleteWithSession(SessionStatus);

  // Called when the CDM completes getting key status for policy.
  BLINK_PLATFORM_EXPORT void CompleteWithKeyStatus(
      WebEncryptedMediaKeyInformation::KeyStatus);

  // Called when the operation fails.
  BLINK_PLATFORM_EXPORT void CompleteWithError(
      WebContentDecryptionModuleException,
      unsigned long system_code,
      const WebString& message);

#if INSIDE_BLINK
  BLINK_PLATFORM_EXPORT explicit WebContentDecryptionModuleResult(
      ContentDecryptionModuleResult*);
#endif

 private:
  BLINK_PLATFORM_EXPORT void Reset();
  BLINK_PLATFORM_EXPORT void Assign(const WebContentDecryptionModuleResult&);

  WebPrivatePtr<ContentDecryptionModuleResult,
                kWebPrivatePtrDestructionCrossThread>
      impl_;
};

}  // namespace blink

#endif  // WebContentDecryptionModuleSession_h
