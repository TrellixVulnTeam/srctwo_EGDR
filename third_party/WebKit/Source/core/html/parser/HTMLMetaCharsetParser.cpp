/*
 * Copyright (C) 2010 Google Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "core/html/parser/HTMLMetaCharsetParser.h"

#include "core/HTMLNames.h"
#include "core/html/parser/HTMLParserIdioms.h"
#include "core/html/parser/HTMLParserOptions.h"
#include "core/html/parser/HTMLTokenizer.h"
#include "platform/wtf/text/TextEncodingRegistry.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

using namespace HTMLNames;

HTMLMetaCharsetParser::HTMLMetaCharsetParser()
    : tokenizer_(HTMLTokenizer::Create(HTMLParserOptions(0))),
      assumed_codec_(NewTextCodec(Latin1Encoding())),
      in_head_section_(true),
      done_checking_(false) {}

HTMLMetaCharsetParser::~HTMLMetaCharsetParser() {}

bool HTMLMetaCharsetParser::ProcessMeta() {
  const HTMLToken::AttributeList& token_attributes = token_.Attributes();
  HTMLAttributeList attributes;
  for (const HTMLToken::Attribute& token_attribute : token_attributes) {
    String attribute_name = token_attribute.NameAttemptStaticStringCreation();
    String attribute_value = token_attribute.Value8BitIfNecessary();
    attributes.push_back(std::make_pair(attribute_name, attribute_value));
  }

  encoding_ = EncodingFromMetaAttributes(attributes);
  return encoding_.IsValid();
}

// That many input bytes will be checked for meta charset even if <head> section
// is over.
static const int kBytesToCheckUnconditionally = 1024;

bool HTMLMetaCharsetParser::CheckForMetaCharset(const char* data,
                                                size_t length) {
  if (done_checking_)
    return true;

  DCHECK(!encoding_.IsValid());

  // We still don't have an encoding, and are in the head. The following tags
  // are allowed in <head>: SCRIPT|STYLE|META|LINK|OBJECT|TITLE|BASE

  // We stop scanning when a tag that is not permitted in <head> is seen, rather
  // when </head> is seen, because that more closely matches behavior in other
  // browsers; more details in <http://bugs.webkit.org/show_bug.cgi?id=3590>.

  // Additionally, we ignore things that looks like tags in <title>, <script>
  // and <noscript>; see:
  // <http://bugs.webkit.org/show_bug.cgi?id=4560>
  // <http://bugs.webkit.org/show_bug.cgi?id=12165>
  // <http://bugs.webkit.org/show_bug.cgi?id=12389>

  // Since many sites have charset declarations after <body> or other tags that
  // are disallowed in <head>, we don't bail out until we've checked at least
  // bytesToCheckUnconditionally bytes of input.

  input_.Append(SegmentedString(assumed_codec_->Decode(data, length)));

  while (tokenizer_->NextToken(input_, token_)) {
    bool end = token_.GetType() == HTMLToken::kEndTag;
    if (end || token_.GetType() == HTMLToken::kStartTag) {
      String tag_name =
          AttemptStaticStringCreation(token_.GetName(), kLikely8Bit);
      if (!end) {
        tokenizer_->UpdateStateFor(tag_name);
        if (ThreadSafeMatch(tag_name, metaTag) && ProcessMeta()) {
          done_checking_ = true;
          return true;
        }
      }

      if (!ThreadSafeMatch(tag_name, scriptTag) &&
          !ThreadSafeMatch(tag_name, noscriptTag) &&
          !ThreadSafeMatch(tag_name, styleTag) &&
          !ThreadSafeMatch(tag_name, linkTag) &&
          !ThreadSafeMatch(tag_name, metaTag) &&
          !ThreadSafeMatch(tag_name, objectTag) &&
          !ThreadSafeMatch(tag_name, titleTag) &&
          !ThreadSafeMatch(tag_name, baseTag) &&
          (end || !ThreadSafeMatch(tag_name, htmlTag)) &&
          (end || !ThreadSafeMatch(tag_name, headTag))) {
        in_head_section_ = false;
      }
    }

    if (!in_head_section_ &&
        input_.NumberOfCharactersConsumed() >= kBytesToCheckUnconditionally) {
      done_checking_ = true;
      return true;
    }

    token_.Clear();
  }

  return false;
}

}  // namespace blink
