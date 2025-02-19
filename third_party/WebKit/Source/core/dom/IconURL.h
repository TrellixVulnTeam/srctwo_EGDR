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

#ifndef IconURL_h
#define IconURL_h

#include "platform/geometry/IntSize.h"
#include "platform/weborigin/KURL.h"
#include "platform/wtf/Allocator.h"

namespace blink {

enum IconType {
  kInvalidIcon = 0,
  kFavicon = 1,
  kTouchIcon = 1 << 1,
  kTouchPrecomposedIcon = 1 << 2,
};

struct IconURL {
  DISALLOW_NEW_EXCEPT_PLACEMENT_NEW();
  IconType icon_type_;
  Vector<IntSize> sizes_;
  String mime_type_;
  KURL icon_url_;
  bool is_default_icon_;

  IconURL() : icon_type_(kInvalidIcon), is_default_icon_(false) {}

  IconURL(const KURL& url,
          const Vector<IntSize>& sizes,
          const String& mime_type,
          IconType type)
      : icon_type_(type),
        sizes_(sizes),
        mime_type_(mime_type),
        icon_url_(url),
        is_default_icon_(false) {}

  static IconURL DefaultFavicon(const KURL&);
};

bool operator==(const IconURL&, const IconURL&);

}  // namespace blink

#endif  // IconURL_h
