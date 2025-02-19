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

#ifndef InspectorDatabaseAgent_h
#define InspectorDatabaseAgent_h

#include "core/inspector/InspectorBaseAgent.h"
#include "core/inspector/protocol/Database.h"
#include "modules/ModulesExport.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/HashMap.h"
#include "platform/wtf/Noncopyable.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class Database;
class InspectorDatabaseResource;
class Page;

class MODULES_EXPORT InspectorDatabaseAgent final
    : public InspectorBaseAgent<protocol::Database::Metainfo> {
  WTF_MAKE_NONCOPYABLE(InspectorDatabaseAgent);

 public:
  static InspectorDatabaseAgent* Create(Page* page) {
    return new InspectorDatabaseAgent(page);
  }
  ~InspectorDatabaseAgent() override;
  DECLARE_VIRTUAL_TRACE();

  protocol::Response disable() override;
  void Restore() override;
  void DidCommitLoadForLocalFrame(LocalFrame*) override;

  // Called from the front-end.
  protocol::Response enable() override;
  protocol::Response getDatabaseTableNames(
      const String& database_id,
      std::unique_ptr<protocol::Array<String>>* names) override;
  void executeSQL(const String& database_id,
                  const String& query,
                  std::unique_ptr<ExecuteSQLCallback>) override;

  void DidOpenDatabase(blink::Database*,
                       const String& domain,
                       const String& name,
                       const String& version);

 private:
  explicit InspectorDatabaseAgent(Page*);
  void RegisterDatabaseOnCreation(blink::Database*);

  blink::Database* DatabaseForId(const String& database_id);
  InspectorDatabaseResource* FindByFileName(const String& file_name);

  Member<Page> page_;
  typedef HeapHashMap<String, Member<InspectorDatabaseResource>>
      DatabaseResourcesHeapMap;
  DatabaseResourcesHeapMap resources_;
  bool enabled_;
};

}  // namespace blink

#endif  // !defined(InspectorDatabaseAgent_h)
