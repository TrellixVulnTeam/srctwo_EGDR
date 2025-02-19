// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebEncryptedMediaTypes_h
#define WebEncryptedMediaTypes_h

namespace blink {

// From
// https://w3c.github.io/encrypted-media/initdata-format-registry.html#registry
enum class WebEncryptedMediaInitDataType {
  kUnknown,
  kCenc,
  kKeyids,
  kWebm,
};

// From https://w3c.github.io/encrypted-media/#idl-def-MediaKeySessionType
enum class WebEncryptedMediaSessionType {
  kUnknown,
  kTemporary,
  kPersistentLicense,
  kPersistentReleaseMessage,
};

}  // namespace blink

#endif  // WebEncryptedMediaTypes_h
