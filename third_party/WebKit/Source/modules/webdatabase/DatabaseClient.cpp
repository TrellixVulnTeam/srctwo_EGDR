/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#include "modules/webdatabase/DatabaseClient.h"

#include "core/dom/Document.h"
#include "core/frame/ContentSettingsClient.h"
#include "core/frame/LocalFrame.h"
#include "core/page/Page.h"
#include "modules/webdatabase/Database.h"
#include "modules/webdatabase/InspectorDatabaseAgent.h"

namespace blink {

DatabaseClient::DatabaseClient() : inspector_agent_(nullptr) {}

DEFINE_TRACE(DatabaseClient) {
  visitor->Trace(inspector_agent_);
  Supplement<Page>::Trace(visitor);
}

DatabaseClient* DatabaseClient::FromPage(Page* page) {
  return static_cast<DatabaseClient*>(
      Supplement<Page>::From(page, SupplementName()));
}

DatabaseClient* DatabaseClient::From(ExecutionContext* context) {
  return DatabaseClient::FromPage(ToDocument(context)->GetPage());
}

const char* DatabaseClient::SupplementName() {
  return "DatabaseClient";
}

bool DatabaseClient::AllowDatabase(ExecutionContext* context,
                                   const String& name,
                                   const String& display_name,
                                   unsigned estimated_size) {
  DCHECK(context->IsContextThread());
  Document* document = ToDocument(context);
  DCHECK(document->GetFrame());
  if (document->GetFrame()->GetContentSettingsClient()) {
    return document->GetFrame()->GetContentSettingsClient()->AllowDatabase(
        name, display_name, estimated_size);
  }
  return true;
}

void DatabaseClient::DidOpenDatabase(blink::Database* database,
                                     const String& domain,
                                     const String& name,
                                     const String& version) {
  if (inspector_agent_)
    inspector_agent_->DidOpenDatabase(database, domain, name, version);
}

void DatabaseClient::SetInspectorAgent(InspectorDatabaseAgent* agent) {
  // TODO(dgozman): we should not set agent twice, but it's happening in OOPIF
  // case.
  inspector_agent_ = agent;
}

void ProvideDatabaseClientTo(Page& page, DatabaseClient* client) {
  page.ProvideSupplement(DatabaseClient::SupplementName(), client);
}

}  // namespace blink
