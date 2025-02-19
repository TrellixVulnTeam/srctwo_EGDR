/*
 * Copyright (C) 2010 Google, Inc. All Rights Reserved.
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

#ifndef HTMLViewSourceParser_h
#define HTMLViewSourceParser_h

#include "core/CoreExport.h"
#include "core/dom/DecodedDataDocumentParser.h"
#include "core/html/HTMLViewSourceDocument.h"
#include "core/html/parser/HTMLInputStream.h"
#include "core/html/parser/HTMLSourceTracker.h"
#include "core/html/parser/HTMLTokenizer.h"
#include "core/html/parser/XSSAuditor.h"
#include <memory>

namespace blink {

class CORE_EXPORT HTMLViewSourceParser final
    : public DecodedDataDocumentParser {
 public:
  static HTMLViewSourceParser* Create(HTMLViewSourceDocument& document,
                                      const String& mime_type) {
    return new HTMLViewSourceParser(document, mime_type);
  }
  ~HTMLViewSourceParser() override {}

 private:
  HTMLViewSourceParser(HTMLViewSourceDocument&, const String& mime_type);

  // DocumentParser
  void insert(const SegmentedString&) override { NOTREACHED(); }
  void Append(const String&) override;
  void Finish() override;

  HTMLViewSourceDocument* GetDocument() const {
    return static_cast<HTMLViewSourceDocument*>(
        DecodedDataDocumentParser::GetDocument());
  }

  void PumpTokenizer();
  void UpdateTokenizerState();

  HTMLInputStream input_;
  HTMLToken token_;
  HTMLSourceTracker source_tracker_;
  std::unique_ptr<HTMLTokenizer> tokenizer_;
  XSSAuditor xss_auditor_;
};

}  // namespace blink

#endif
