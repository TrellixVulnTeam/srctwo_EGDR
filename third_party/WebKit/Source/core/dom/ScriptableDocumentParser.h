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

#ifndef ScriptableDocumentParser_h
#define ScriptableDocumentParser_h

#include "core/CoreExport.h"
#include "core/dom/DecodedDataDocumentParser.h"
#include "core/dom/ParserContentPolicy.h"
#include "platform/wtf/text/TextPosition.h"

namespace blink {

class CORE_EXPORT ScriptableDocumentParser : public DecodedDataDocumentParser {
 public:
  // Only used by Document::open for deciding if its safe to act on a
  // JavaScript document.open() call right now, or it should be ignored.
  virtual bool IsExecutingScript() const { return false; }

  // FIXME: Only the HTMLDocumentParser ever blocks script execution on
  // stylesheet load, which is likely a bug in the XMLDocumentParser.
  virtual void ExecuteScriptsWaitingForResources() {}

  virtual bool IsWaitingForScripts() const = 0;
  virtual void DidAddPendingStylesheetInBody() {}
  virtual void DidLoadAllBodyStylesheets() {}

  // These are used to expose the current line/column to the scripting system.
  virtual bool IsParsingAtLineNumber() const;
  virtual OrdinalNumber LineNumber() const = 0;
  virtual TextPosition GetTextPosition() const = 0;

  void SetWasCreatedByScript(bool was_created_by_script) {
    was_created_by_script_ = was_created_by_script;
  }
  bool WasCreatedByScript() const { return was_created_by_script_; }

  ParserContentPolicy GetParserContentPolicy() {
    return parser_content_policy_;
  }

 protected:
  explicit ScriptableDocumentParser(
      Document&,
      ParserContentPolicy = kAllowScriptingContent);

 private:
  ScriptableDocumentParser* AsScriptableDocumentParser() final { return this; }

  // http://www.whatwg.org/specs/web-apps/current-work/#script-created-parser
  bool was_created_by_script_;
  ParserContentPolicy parser_content_policy_;
};

}  // namespace blink

#endif  // ScriptableDocumentParser_h
