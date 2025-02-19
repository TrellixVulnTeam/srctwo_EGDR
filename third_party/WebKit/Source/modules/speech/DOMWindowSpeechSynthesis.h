/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DOMWindowSpeechSynthesis_h
#define DOMWindowSpeechSynthesis_h

#include "modules/ModulesExport.h"
#include "modules/speech/SpeechSynthesis.h"
#include "platform/Supplementable.h"
#include "platform/heap/Handle.h"

namespace blink {

class LocalDOMWindow;
class ScriptState;

class MODULES_EXPORT DOMWindowSpeechSynthesis final
    : public GarbageCollected<DOMWindowSpeechSynthesis>,
      public Supplement<LocalDOMWindow> {
  USING_GARBAGE_COLLECTED_MIXIN(DOMWindowSpeechSynthesis);

 public:
  static SpeechSynthesis* speechSynthesis(ScriptState*, LocalDOMWindow&);
  static DOMWindowSpeechSynthesis& From(LocalDOMWindow&);

  DECLARE_TRACE();

 private:
  explicit DOMWindowSpeechSynthesis(LocalDOMWindow&);

  SpeechSynthesis* speechSynthesis(ScriptState*);
  static const char* SupplementName();

  Member<SpeechSynthesis> speech_synthesis_;
};

}  // namespace blink

#endif  // DOMWindowSpeechSynthesis_h
