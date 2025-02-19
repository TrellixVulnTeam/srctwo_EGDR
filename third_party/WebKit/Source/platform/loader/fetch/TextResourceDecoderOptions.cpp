// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/loader/fetch/TextResourceDecoderOptions.h"
#include "platform/Language.h"

namespace blink {

TextResourceDecoderOptions::TextResourceDecoderOptions(
    ContentType content_type,
    const WTF::TextEncoding& default_encoding)
    : TextResourceDecoderOptions(kUseContentAndBOMBasedDetection,
                                 content_type,
                                 default_encoding,
                                 nullptr,
                                 KURL()) {}

TextResourceDecoderOptions
TextResourceDecoderOptions::CreateAlwaysUseUTF8ForText() {
  return TextResourceDecoderOptions(kAlwaysUseUTF8ForText, kPlainTextContent,
                                    UTF8Encoding(), nullptr, NullURL());
}

TextResourceDecoderOptions TextResourceDecoderOptions::CreateWithAutoDetection(
    ContentType content_type,
    const WTF::TextEncoding& default_encoding,
    const WTF::TextEncoding& hint_encoding,
    const KURL& hint_url) {
  return TextResourceDecoderOptions(kUseAllAutoDetection, content_type,
                                    default_encoding, hint_encoding.GetName(),
                                    hint_url);
}

TextResourceDecoderOptions::TextResourceDecoderOptions(
    EncodingDetectionOption encoding_detection_option,
    ContentType content_type,
    const WTF::TextEncoding& default_encoding,
    const char* hint_encoding,
    const KURL& hint_url)
    : encoding_detection_option_(encoding_detection_option),
      content_type_(content_type),
      default_encoding_(default_encoding),
      use_lenient_xml_decoding_(false),
      hint_encoding_(hint_encoding),
      hint_url_(hint_url) {
  hint_language_[0] = 0;
  if (encoding_detection_option_ == kUseAllAutoDetection) {
    // Checking empty URL helps unit testing. Providing DefaultLanguage() is
    // sometimes difficult in tests.
    if (!hint_url_.IsEmpty()) {
      // This object is created in the main thread, but used in another thread.
      // We should not share an AtomicString.
      AtomicString locale = DefaultLanguage();
      if (locale.length() >= 2) {
        // DefaultLanguage() is always an ASCII string.
        hint_language_[0] = static_cast<char>(locale[0]);
        hint_language_[1] = static_cast<char>(locale[1]);
        hint_language_[2] = 0;
      }
    }
  }
}

}  // namespace blink
