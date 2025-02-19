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

#ifndef WebSearchableFormData_h
#define WebSearchableFormData_h

#include "public/platform/WebString.h"
#include "public/platform/WebURL.h"
#include "WebInputElement.h"

namespace blink {

class WebFormElement;

// SearchableFormData encapsulates a URL and encoding of an INPUT field that
// corresponds to a searchable form request.
class WebSearchableFormData {
 public:
  // If the provided form is suitable for automated searching, isValid()
  // will return false.
  BLINK_EXPORT WebSearchableFormData(
      const WebFormElement&,
      const WebInputElement& selected_input_element = WebInputElement());

  bool IsValid() { return url_.IsValid(); }

  // URL for the searchable form request.
  const WebURL& Url() const { return url_; }

  // Encoding used to encode the form parameters; never empty.
  const WebString& Encoding() const { return encoding_; }

 private:
  WebURL url_;
  WebString encoding_;
};

}  // namespace blink

#endif
