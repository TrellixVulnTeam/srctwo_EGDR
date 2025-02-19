/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
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

#include "core/dom/IconURL.h"
#include "platform/wtf/Assertions.h"
#include "public/web/WebIconURL.h"

namespace blink {

IconURL IconURL::DefaultFavicon(const KURL& document_url) {
  DCHECK(document_url.ProtocolIsInHTTPFamily());
  KURL url;
  bool could_set_protocol = url.SetProtocol(document_url.Protocol());
  DCHECK(could_set_protocol);
  url.SetHost(document_url.Host());
  if (document_url.HasPort())
    url.SetPort(document_url.Port());
  url.SetPath("/favicon.ico");

  IconURL result(url, Vector<IntSize>(), g_empty_string, kFavicon);
  result.is_default_icon_ = true;
  return result;
}

bool operator==(const IconURL& lhs, const IconURL& rhs) {
  return lhs.icon_type_ == rhs.icon_type_ &&
         lhs.is_default_icon_ == rhs.is_default_icon_ &&
         lhs.icon_url_ == rhs.icon_url_ && lhs.sizes_ == rhs.sizes_ &&
         lhs.mime_type_ == rhs.mime_type_;
}

STATIC_ASSERT_ENUM(WebIconURL::kTypeInvalid, kInvalidIcon);
STATIC_ASSERT_ENUM(WebIconURL::kTypeFavicon, kFavicon);
STATIC_ASSERT_ENUM(WebIconURL::kTypeTouch, kTouchIcon);
STATIC_ASSERT_ENUM(WebIconURL::kTypeTouchPrecomposed, kTouchPrecomposedIcon);

}  // namespace blink
