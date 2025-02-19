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

#ifndef DateTimeChooserImpl_h
#define DateTimeChooserImpl_h

#include <memory>
#include "core/CoreExport.h"
#include "core/html/forms/DateTimeChooser.h"
#include "core/page/PagePopupClient.h"

namespace blink {

class ChromeClient;
class DateTimeChooserClient;
class PagePopup;

class CORE_EXPORT DateTimeChooserImpl final : public DateTimeChooser,
                                              public PagePopupClient {
 public:
  static DateTimeChooserImpl* Create(ChromeClient*,
                                     DateTimeChooserClient*,
                                     const DateTimeChooserParameters&);
  ~DateTimeChooserImpl() override;

  // DateTimeChooser functions:
  void EndChooser() override;
  AXObject* RootAXObject() override;

  DECLARE_VIRTUAL_TRACE();

 private:
  DateTimeChooserImpl(ChromeClient*,
                      DateTimeChooserClient*,
                      const DateTimeChooserParameters&);
  // PagePopupClient functions:
  void WriteDocument(SharedBuffer*) override;
  void SelectFontsFromOwnerDocument(Document&) override {}
  Locale& GetLocale() override;
  void SetValueAndClosePopup(int, const String&) override;
  void SetValue(const String&) override;
  void ClosePopup() override;
  Element& OwnerElement() override;
  void DidClosePopup() override;

  Member<ChromeClient> chrome_client_;
  Member<DateTimeChooserClient> client_;
  PagePopup* popup_;
  DateTimeChooserParameters parameters_;
  std::unique_ptr<Locale> locale_;
};

}  // namespace blink

#endif  // DateTimeChooserImpl_h
