/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef IDBFactory_h
#define IDBFactory_h

#include "modules/indexeddb/IDBOpenDBRequest.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class ExceptionState;
class ScriptState;

class IDBFactory final : public GarbageCollected<IDBFactory>,
                         public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static IDBFactory* Create() { return new IDBFactory(); }
  DEFINE_INLINE_VIRTUAL_TRACE() {}

  // Implement the IDBFactory IDL
  IDBOpenDBRequest* open(ScriptState*, const String& name, ExceptionState&);
  IDBOpenDBRequest* open(ScriptState*,
                         const String& name,
                         unsigned long long version,
                         ExceptionState&);
  IDBOpenDBRequest* deleteDatabase(ScriptState*,
                                   const String& name,
                                   ExceptionState&);
  short cmp(ScriptState*,
            const ScriptValue& first,
            const ScriptValue& second,
            ExceptionState&);

  // These are not exposed to the web applications and only used by DevTools.
  IDBRequest* GetDatabaseNames(ScriptState*, ExceptionState&);
  IDBOpenDBRequest* CloseConnectionsAndDeleteDatabase(ScriptState*,
                                                      const String& name,
                                                      ExceptionState&);

 private:
  IDBFactory();

  IDBOpenDBRequest* OpenInternal(ScriptState*,
                                 const String& name,
                                 int64_t version,
                                 ExceptionState&);

  IDBOpenDBRequest* DeleteDatabaseInternal(ScriptState*,
                                           const String& name,
                                           ExceptionState&,
                                           bool);
};

}  // namespace blink

#endif  // IDBFactory_h
