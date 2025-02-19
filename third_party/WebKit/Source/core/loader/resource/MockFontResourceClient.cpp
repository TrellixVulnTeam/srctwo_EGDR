// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/loader/resource/MockFontResourceClient.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

MockFontResourceClient::MockFontResourceClient(Resource* resource)
    : resource_(resource),
      font_load_short_limit_exceeded_called_(false),
      font_load_long_limit_exceeded_called_(false) {
  resource_->AddClient(this);
}

MockFontResourceClient::~MockFontResourceClient() {}

void MockFontResourceClient::FontLoadShortLimitExceeded(FontResource*) {
  ASSERT_FALSE(font_load_short_limit_exceeded_called_);
  ASSERT_FALSE(font_load_long_limit_exceeded_called_);
  font_load_short_limit_exceeded_called_ = true;
}

void MockFontResourceClient::FontLoadLongLimitExceeded(FontResource*) {
  ASSERT_TRUE(font_load_short_limit_exceeded_called_);
  ASSERT_FALSE(font_load_long_limit_exceeded_called_);
  font_load_long_limit_exceeded_called_ = true;
}

void MockFontResourceClient::Dispose() {
  if (resource_) {
    resource_->RemoveClient(this);
    resource_ = nullptr;
  }
}

}  // namespace blink
