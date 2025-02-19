/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
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

#ifndef WebImeTextSpan_h
#define WebImeTextSpan_h

#include <string>
#include <vector>
#include "public/platform/WebColor.h"

namespace blink {

// Class WebImeTextSpan is intended to be used with WebWidget's
// setComposition() method.
struct WebImeTextSpan {
  enum class Type {
    // Creates a composition marker
    kComposition,
    // Creates a suggestion marker that isn't cleared after the user picks a
    // replacement
    kSuggestion,
  };

  WebImeTextSpan()
      : type(Type::kComposition),
        start_offset(0),
        end_offset(0),
        underline_color(0),
        thick(false),
        background_color(0),
        suggestion_highlight_color(0),
        suggestions(std::vector<std::string>()) {}

  WebImeTextSpan(
      Type ty,
      unsigned s,
      unsigned e,
      WebColor uc,
      bool th,
      WebColor bc,
      WebColor shc = 0,
      const std::vector<std::string>& su = std::vector<std::string>())
      : type(ty),
        start_offset(s),
        end_offset(e),
        underline_color(uc),
        thick(th),
        background_color(bc),
        suggestion_highlight_color(shc),
        suggestions(su) {}

  bool operator<(const WebImeTextSpan& other) const {
    return start_offset != other.start_offset
               ? start_offset < other.start_offset
               : end_offset < other.end_offset;
  }

  // Need to update IPC_STRUCT_TRAITS_BEGIN(blink::WebImeTextSpan)
  // if members change.
  Type type;
  unsigned start_offset;
  unsigned end_offset;
  WebColor underline_color;
  bool thick;
  WebColor background_color;
  WebColor suggestion_highlight_color;
  std::vector<std::string> suggestions;
};

}  // namespace blink

#endif
