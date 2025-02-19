// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/strings/utf_string_conversions.h"
#include "media/base/localized_strings.h"

namespace media {

base::string16 FakeLocalizedStringProvider(MessageId message_id) {
  if (message_id == DEFAULT_AUDIO_DEVICE_NAME)
    return base::ASCIIToUTF16("Default");

  return base::ASCIIToUTF16("FakeString");
}

void SetUpFakeLocalizedStrings() {
  SetLocalizedStringProvider(FakeLocalizedStringProvider);
}

}  // namespace media
