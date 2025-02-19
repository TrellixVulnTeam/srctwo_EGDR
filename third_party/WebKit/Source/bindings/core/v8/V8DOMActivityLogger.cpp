// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bindings/core/v8/V8DOMActivityLogger.h"

#include <memory>
#include "platform/bindings/V8Binding.h"
#include "platform/weborigin/KURL.h"
#include "platform/wtf/HashMap.h"
#include "platform/wtf/text/StringHash.h"

namespace blink {

typedef HashMap<String, std::unique_ptr<V8DOMActivityLogger>>
    DOMActivityLoggerMapForMainWorld;
typedef HashMap<int,
                std::unique_ptr<V8DOMActivityLogger>,
                WTF::IntHash<int>,
                WTF::UnsignedWithZeroKeyHashTraits<int>>
    DOMActivityLoggerMapForIsolatedWorld;

static DOMActivityLoggerMapForMainWorld& DomActivityLoggersForMainWorld() {
  DCHECK(IsMainThread());
  DEFINE_STATIC_LOCAL(DOMActivityLoggerMapForMainWorld, map, ());
  return map;
}

static DOMActivityLoggerMapForIsolatedWorld&
DomActivityLoggersForIsolatedWorld() {
  DCHECK(IsMainThread());
  DEFINE_STATIC_LOCAL(DOMActivityLoggerMapForIsolatedWorld, map, ());
  return map;
}

void V8DOMActivityLogger::SetActivityLogger(
    int world_id,
    const String& extension_id,
    std::unique_ptr<V8DOMActivityLogger> logger) {
  if (world_id)
    DomActivityLoggersForIsolatedWorld().Set(world_id, std::move(logger));
  else
    DomActivityLoggersForMainWorld().Set(extension_id, std::move(logger));
}

V8DOMActivityLogger* V8DOMActivityLogger::ActivityLogger(
    int world_id,
    const String& extension_id) {
  if (world_id) {
    DOMActivityLoggerMapForIsolatedWorld& loggers =
        DomActivityLoggersForIsolatedWorld();
    DOMActivityLoggerMapForIsolatedWorld::iterator it = loggers.find(world_id);
    return it == loggers.end() ? 0 : it->value.get();
  }

  if (extension_id.IsEmpty())
    return 0;

  DOMActivityLoggerMapForMainWorld& loggers = DomActivityLoggersForMainWorld();
  DOMActivityLoggerMapForMainWorld::iterator it = loggers.find(extension_id);
  return it == loggers.end() ? 0 : it->value.get();
}

V8DOMActivityLogger* V8DOMActivityLogger::ActivityLogger(int world_id,
                                                         const KURL& url) {
  // extension ID is ignored for worldId != 0.
  if (world_id)
    return ActivityLogger(world_id, String());

  // To find an activity logger that corresponds to the main world of an
  // extension, we need to obtain the extension ID. Extension ID is a hostname
  // of a background page's URL.
  if (!url.ProtocolIs("chrome-extension"))
    return 0;

  return ActivityLogger(world_id, url.Host());
}

V8DOMActivityLogger* V8DOMActivityLogger::CurrentActivityLogger() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  if (!isolate->InContext())
    return 0;

  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  V8PerContextData* context_data = ScriptState::From(context)->PerContextData();
  if (!context_data)
    return 0;

  return context_data->ActivityLogger();
}

V8DOMActivityLogger*
V8DOMActivityLogger::CurrentActivityLoggerIfIsolatedWorld() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  if (!isolate->InContext())
    return 0;

  v8::HandleScope handle_scope(isolate);
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  ScriptState* script_state = ScriptState::From(context);
  if (!script_state->World().IsIsolatedWorld())
    return 0;

  V8PerContextData* context_data = script_state->PerContextData();
  if (!context_data)
    return 0;

  return context_data->ActivityLogger();
}

}  // namespace blink
