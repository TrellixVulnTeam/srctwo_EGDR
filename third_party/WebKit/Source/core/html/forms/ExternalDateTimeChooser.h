/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef ExternalDateTimeChooser_h
#define ExternalDateTimeChooser_h

#include "core/CoreExport.h"
#include "core/html/forms/DateTimeChooser.h"

namespace blink {

class ChromeClient;
class DateTimeChooserClient;
class WebString;
class WebViewClient;

class CORE_EXPORT ExternalDateTimeChooser final : public DateTimeChooser {
 public:
  static ExternalDateTimeChooser* Create(ChromeClient*,
                                         WebViewClient*,
                                         DateTimeChooserClient*,
                                         const DateTimeChooserParameters&);
  ~ExternalDateTimeChooser() override;
  DECLARE_VIRTUAL_TRACE();

  // The following functions are for DateTimeChooserCompletion.
  void DidChooseValue(const WebString&);
  void DidChooseValue(double);
  void DidCancelChooser();

 private:
  ExternalDateTimeChooser(DateTimeChooserClient*);
  bool OpenDateTimeChooser(ChromeClient*,
                           WebViewClient*,
                           const DateTimeChooserParameters&);

  // DateTimeChooser function:
  void EndChooser() override;
  AXObject* RootAXObject() override;

  Member<DateTimeChooserClient> client_;
};
}
#endif
