// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/network/ParsedContentDisposition.h"

#include "platform/network/HeaderFieldTokenizer.h"

namespace blink {

ParsedContentDisposition::ParsedContentDisposition(
    const String& content_disposition,
    Mode mode) {
  HeaderFieldTokenizer tokenizer(content_disposition);

  StringView type;
  if (!tokenizer.ConsumeToken(Mode::kNormal, type)) {
    DVLOG(1) << "Failed to find `type' in '" << content_disposition << "'";
    return;
  }
  type_ = type.ToString();

  parameters_.ParseParameters(std::move(tokenizer), mode);
}

String ParsedContentDisposition::Filename() const {
  return ParameterValueForName("filename");
}

}  // namespace blink
