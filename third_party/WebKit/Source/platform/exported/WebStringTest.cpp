// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/platform/WebString.h"

#include "platform/wtf/text/WTFString.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

TEST(WebStringTest, UTF8ConversionRoundTrip) {
  // Valid characters.
  for (UChar uchar = 0; uchar <= 0xD7FF; ++uchar) {
    WebString utf16_string(&uchar, 1);
    std::string utf8_string(utf16_string.Utf8());
    WebString utf16_new_string =
        WebString::FromUTF8(utf8_string.data(), utf8_string.length());
    EXPECT_FALSE(utf16_new_string.IsNull());
    EXPECT_TRUE(utf16_string == utf16_new_string);
  }

  // Unpaired surrogates.
  for (UChar uchar = 0xD800; uchar <= 0xDFFF; ++uchar) {
    WebString utf16_string(&uchar, 1);

    // Conversion with Strict mode results in an empty string.
    std::string utf8_string(
        utf16_string.Utf8(WebString::UTF8ConversionMode::kStrict));
    EXPECT_TRUE(utf8_string.empty());

    // Unpaired surrogates can't be converted back in Lenient mode.
    utf8_string = utf16_string.Utf8(WebString::UTF8ConversionMode::kLenient);
    EXPECT_FALSE(utf8_string.empty());
    WebString utf16_new_string =
        WebString::FromUTF8(utf8_string.data(), utf8_string.length());
    EXPECT_TRUE(utf16_new_string.IsNull());

    // Round-trip works with StrictReplacingErrorsWithFFFD mode.
    utf8_string = utf16_string.Utf8(
        WebString::UTF8ConversionMode::kStrictReplacingErrorsWithFFFD);
    EXPECT_FALSE(utf8_string.empty());
    utf16_new_string =
        WebString::FromUTF8(utf8_string.data(), utf8_string.length());
    EXPECT_FALSE(utf16_new_string.IsNull());
    EXPECT_FALSE(utf16_string == utf16_new_string);
  }
}

}  // namespace blink
