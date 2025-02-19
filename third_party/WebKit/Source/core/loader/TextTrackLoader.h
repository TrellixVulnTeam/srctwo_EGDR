/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TextTrackLoader_h
#define TextTrackLoader_h

#include "core/html/track/vtt/VTTParser.h"
#include "platform/CrossOriginAttributeValue.h"
#include "platform/heap/Handle.h"
#include "platform/loader/fetch/RawResource.h"
#include "platform/loader/fetch/ResourceOwner.h"

namespace blink {

class Document;
class TextTrackLoader;

class TextTrackLoaderClient : public GarbageCollectedMixin {
 public:
  virtual ~TextTrackLoaderClient() {}

  virtual void NewCuesAvailable(TextTrackLoader*) = 0;
  virtual void CueLoadingCompleted(TextTrackLoader*, bool loading_failed) = 0;
};

class TextTrackLoader final : public GarbageCollectedFinalized<TextTrackLoader>,
                              public ResourceOwner<RawResource>,
                              private VTTParserClient {
  USING_GARBAGE_COLLECTED_MIXIN(TextTrackLoader);

 public:
  static TextTrackLoader* Create(TextTrackLoaderClient& client,
                                 Document& document) {
    return new TextTrackLoader(client, document);
  }
  ~TextTrackLoader() override;

  bool Load(const KURL&, CrossOriginAttributeValue);
  void CancelLoad();

  enum State { kLoading, kFinished, kFailed };
  State LoadState() { return state_; }

  void GetNewCues(HeapVector<Member<TextTrackCue>>& output_cues);

  DECLARE_TRACE();

 private:
  // RawResourceClient
  bool RedirectReceived(Resource*,
                        const ResourceRequest&,
                        const ResourceResponse&) override;
  void DataReceived(Resource*, const char* data, size_t length) override;
  void NotifyFinished(Resource*) override;
  String DebugName() const override { return "TextTrackLoader"; }

  // VTTParserClient
  void NewCuesParsed() override;
  void FileFailedToParse() override;

  TextTrackLoader(TextTrackLoaderClient&, Document&);

  void CueLoadTimerFired(TimerBase*);
  void CorsPolicyPreventedLoad(SecurityOrigin*, const KURL&);

  Document& GetDocument() const { return *document_; }

  Member<TextTrackLoaderClient> client_;
  Member<VTTParser> cue_parser_;
  // FIXME: Remove this pointer and get the Document from m_client.
  Member<Document> document_;
  TaskRunnerTimer<TextTrackLoader> cue_load_timer_;
  State state_;
  bool new_cues_available_;
};

}  // namespace blink

#endif
