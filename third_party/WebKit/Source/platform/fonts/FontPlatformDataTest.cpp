/*
 * Copyright (C) 2015 Google Inc. All rights reserved.
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

#include "platform/fonts/Font.h"

#include "platform/fonts/TypesettingFeatures.h"
#include "platform/testing/FontTestHelpers.h"
#include "platform/testing/UnitTestHelpers.h"
#include "testing/gtest/include/gtest/gtest.h"

using blink::testing::CreateTestFont;

namespace blink {

TEST(FontPlatformDataTest, AhemHasNoSpaceInLigaturesOrKerning) {
  Font font =
      CreateTestFont("Ahem", testing::PlatformTestDataPath("Ahem.woff"), 16);
  const FontPlatformData& platform_data = font.PrimaryFont()->PlatformData();
  TypesettingFeatures features = kKerning | kLigatures;

  EXPECT_FALSE(platform_data.HasSpaceInLigaturesOrKerning(features));
}

TEST(FontPlatformDataTest, AhemSpaceLigatureHasSpaceInLigaturesOrKerning) {
  Font font = CreateTestFont(
      "AhemSpaceLigature",
      testing::PlatformTestDataPath("AhemSpaceLigature.woff"), 16);
  const FontPlatformData& platform_data = font.PrimaryFont()->PlatformData();
  TypesettingFeatures features = kKerning | kLigatures;

  EXPECT_TRUE(platform_data.HasSpaceInLigaturesOrKerning(features));
}

TEST(FontPlatformDataTest, AhemSpaceLigatureHasNoSpaceWithoutFontFeatures) {
  Font font = CreateTestFont(
      "AhemSpaceLigature",
      testing::PlatformTestDataPath("AhemSpaceLigature.woff"), 16);
  const FontPlatformData& platform_data = font.PrimaryFont()->PlatformData();
  TypesettingFeatures features = 0;

  EXPECT_FALSE(platform_data.HasSpaceInLigaturesOrKerning(features));
}

}  // namespace blink
