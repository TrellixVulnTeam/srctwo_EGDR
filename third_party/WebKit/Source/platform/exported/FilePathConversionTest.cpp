// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/platform/FilePathConversion.h"

#include "base/files/file_path.h"
#include "platform/wtf/text/WTFString.h"
#include "public/platform/WebString.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

TEST(FilePathConversionTest, convert) {
  String test8bit_string("path");
  String test8bit_latin1("a\xC4");

  static const UChar kTest[5] = {0x0070, 0x0061, 0x0074, 0x0068, 0};  // path
  static const UChar kTestLatin1[3] = {0x0061, 0x00C4, 0};            // a\xC4
  static const UChar kTestUTF16[3] = {0x6587, 0x5B57, 0};  // \u6587 \u5B57
  String test16bit_string(kTest);
  String test16bit_latin1(kTestLatin1);
  String test16bit_utf16(kTestUTF16);

  // Latin1 a\xC4 == UTF8 a\xC3\x84
  base::FilePath path_latin1 = base::FilePath::FromUTF8Unsafe("a\xC3\x84");
  // UTF16 \u6587\u5B57 == \xE6\x96\x87\xE5\xAD\x97
  base::FilePath path_utf16 =
      base::FilePath::FromUTF8Unsafe("\xE6\x96\x87\xE5\xAD\x97");

  EXPECT_TRUE(test8bit_string.Is8Bit());
  EXPECT_TRUE(test8bit_latin1.Is8Bit());
  EXPECT_FALSE(test16bit_string.Is8Bit());
  EXPECT_FALSE(test16bit_latin1.Is8Bit());

  EXPECT_EQ(FILE_PATH_LITERAL("path"),
            WebStringToFilePath(test8bit_string).value());
  EXPECT_EQ(path_latin1.value(), WebStringToFilePath(test8bit_latin1).value());
  EXPECT_EQ(FILE_PATH_LITERAL("path"),
            WebStringToFilePath(test16bit_string).value());
  EXPECT_EQ(path_latin1.value(), WebStringToFilePath(test16bit_latin1).value());
  EXPECT_EQ(path_utf16.value(), WebStringToFilePath(test16bit_utf16).value());

  EXPECT_STREQ("path",
               FilePathToWebString(base::FilePath(FILE_PATH_LITERAL("path")))
                   .Utf8()
                   .data());
  EXPECT_STREQ(test8bit_latin1.Utf8().data(),
               FilePathToWebString(path_latin1).Utf8().data());
  EXPECT_STREQ(test16bit_utf16.Utf8().data(),
               FilePathToWebString(path_utf16).Utf8().data());
}

}  // namespace blink
