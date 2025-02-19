/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
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

#include "public/platform/WebURL.h"

#include "platform/weborigin/KURL.h"
#include "platform/wtf/text/StringView.h"

namespace blink {

bool WebURL::ProtocolIs(const char* protocol) const {
  const url::Component& scheme = parsed_.scheme;
  StringView url_view = string_;
  // For subtlety why this works in all cases, see KURL::componentString.
  return is_valid_ &&
         StringView(url_view, scheme.begin, scheme.len) == protocol;
}

WebURL::WebURL(const KURL& url)
    : string_(url.GetString()),
      parsed_(url.GetParsed()),
      is_valid_(url.IsValid()) {}

WebURL& WebURL::operator=(const KURL& url) {
  string_ = url.GetString();
  parsed_ = url.GetParsed();
  is_valid_ = url.IsValid();
  return *this;
}

WebURL::operator KURL() const {
  return KURL(string_, parsed_, is_valid_);
}

}  // namespace blink
