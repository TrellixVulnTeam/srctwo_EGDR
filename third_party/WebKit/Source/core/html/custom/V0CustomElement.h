/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Google Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

#ifndef V0CustomElement_h
#define V0CustomElement_h

#include "core/CoreExport.h"
#include "core/html/custom/V0CustomElementDefinition.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/HashMap.h"
#include "platform/wtf/Vector.h"
#include "platform/wtf/text/AtomicString.h"

namespace blink {

class V0CustomElementMicrotaskImportStep;
class Document;
class HTMLImportChild;

class CORE_EXPORT V0CustomElement {
  STATIC_ONLY(V0CustomElement);

 public:
  enum NameSet {
    kEmbedderNames = 1 << 0,
    kStandardNames = 1 << 1,
    kAllNames = kEmbedderNames | kStandardNames
  };
  static bool IsValidName(const AtomicString& name,
                          NameSet valid_names = kAllNames);
  static void AddEmbedderCustomElementName(const AtomicString& name);

  // API to notify of document-level changes
  static V0CustomElementMicrotaskImportStep* DidCreateImport(HTMLImportChild*);
  static void DidFinishLoadingImport(Document& master);

  // API for registration contexts
  static void Define(Element*, V0CustomElementDefinition*);

  // API for Element to kick off changes

  static void AttributeDidChange(Element*,
                                 const AtomicString& name,
                                 const AtomicString& old_value,
                                 const AtomicString& new_value);
  static void DidAttach(Element*, const Document&);
  static void DidDetach(Element*, const Document&);
  static void WasDestroyed(Element*);

 private:
  static Vector<AtomicString>& EmbedderCustomElementNames();
};

}  // namespace blink

#endif  // V0CustomElement_h
