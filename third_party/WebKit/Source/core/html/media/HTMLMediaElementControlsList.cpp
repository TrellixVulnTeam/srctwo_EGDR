// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/html/media/HTMLMediaElementControlsList.h"

#include "core/html/HTMLMediaElement.h"

namespace blink {

namespace {

const char kNoDownload[] = "nodownload";
const char kNoFullscreen[] = "nofullscreen";
const char kNoRemotePlayback[] = "noremoteplayback";

const char* const kSupportedTokens[] = {kNoDownload, kNoFullscreen,
                                        kNoRemotePlayback};

}  // namespace

HTMLMediaElementControlsList::HTMLMediaElementControlsList(
    HTMLMediaElement* element)
    : DOMTokenList(*element, HTMLNames::controlslistAttr) {}

bool HTMLMediaElementControlsList::ValidateTokenValue(
    const AtomicString& token_value,
    ExceptionState&) const {
  for (const char* supported_token : kSupportedTokens) {
    if (token_value == supported_token)
      return true;
  }
  return false;
}

bool HTMLMediaElementControlsList::ShouldHideDownload() const {
  return contains(kNoDownload);
}

bool HTMLMediaElementControlsList::ShouldHideFullscreen() const {
  return contains(kNoFullscreen);
}

bool HTMLMediaElementControlsList::ShouldHideRemotePlayback() const {
  return contains(kNoRemotePlayback);
}

}  // namespace blink
