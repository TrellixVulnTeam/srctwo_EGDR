/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#ifndef PagePopupClient_h
#define PagePopupClient_h

#include "core/CoreExport.h"
#include "platform/SharedBuffer.h"
#include "platform/geometry/IntRect.h"
#include "platform/wtf/text/CString.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class Document;
class Element;
class Locale;

class CORE_EXPORT PagePopupClient {
 public:
  // Provide an HTML source to the specified buffer. The HTML
  // source is rendered in a PagePopup.
  // The content HTML supports:
  //  - No <select> popups
  //  - window.setValueAndClosePopup(number, string).
  virtual void WriteDocument(SharedBuffer*) = 0;

  // This is called after the document is ready to do additionary setup.
  virtual void SelectFontsFromOwnerDocument(Document&) = 0;

  virtual Element& OwnerElement() = 0;
  // Returns effective zoom factor of ownerElement, or the page zoom factor if
  // the effective zoom factor is not available.
  virtual float ZoomFactor();
  // Returns a Locale object associated to the client.
  virtual Locale& GetLocale() = 0;

  // This is called by the content HTML of a PagePopup.
  // An implementation of this function should call
  // ChromeClient::closePagePopup().
  virtual void SetValueAndClosePopup(int num_value,
                                     const String& string_value) = 0;

  // This is called by the content HTML of a PagePopup.
  virtual void SetValue(const String&) = 0;

  // This is called by the content HTML of a PagePopup.
  virtual void ClosePopup() = 0;

  // This is called whenever a PagePopup was closed.
  virtual void DidClosePopup() = 0;

  virtual ~PagePopupClient() {}

  // Helper functions to be used in PagePopupClient::writeDocument().
  static void AddString(const String&, SharedBuffer*);
  static void AddJavaScriptString(const String&, SharedBuffer*);
  static void AddHTMLString(const String&, SharedBuffer*);
  static void AddProperty(const char* name, const String& value, SharedBuffer*);
  static void AddProperty(const char* name, int value, SharedBuffer*);
  static void AddProperty(const char* name, unsigned value, SharedBuffer*);
  static void AddProperty(const char* name, bool value, SharedBuffer*);
  static void AddProperty(const char* name, double, SharedBuffer*);
  static void AddProperty(const char* name,
                          const Vector<String>& values,
                          SharedBuffer*);
  static void AddProperty(const char* name, const IntRect&, SharedBuffer*);
};

inline void PagePopupClient::AddString(const String& str, SharedBuffer* data) {
  CString str8 = str.Utf8();
  data->Append(str8.data(), str8.length());
}

}  // namespace blink
#endif
