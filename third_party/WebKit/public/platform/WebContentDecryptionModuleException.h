// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebContentDecryptionModuleException_h
#define WebContentDecryptionModuleException_h

namespace blink {

// From https://w3c.github.io/encrypted-media/#exceptions.
enum WebContentDecryptionModuleException {
  kWebContentDecryptionModuleExceptionTypeError,
  kWebContentDecryptionModuleExceptionNotSupportedError,
  kWebContentDecryptionModuleExceptionInvalidStateError,
  kWebContentDecryptionModuleExceptionQuotaExceededError,
  // TODO(jrummell): UnknownError is not part of the spec, but CDMs can
  // generate other error codes (in addition to the 4 listed above). Remove
  // UnknownError when the CDMs no longer use other error codes.
  kWebContentDecryptionModuleExceptionUnknownError
};

}  // namespace blink

#endif  // WebContentDecryptionModuleException_h
