/*
 * Copyright (C) 2010 Google Inc.  All rights reserved.
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

#ifndef FileReader_h
#define FileReader_h

#include <memory>
#include "core/CoreExport.h"
#include "core/dom/ContextLifecycleObserver.h"
#include "core/dom/events/EventTarget.h"
#include "core/fileapi/FileError.h"
#include "core/fileapi/FileReaderLoader.h"
#include "core/fileapi/FileReaderLoaderClient.h"
#include "platform/bindings/ActiveScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"

namespace blink {

class Blob;
class ExceptionState;
class ExecutionContext;
class StringOrArrayBuffer;

class CORE_EXPORT FileReader final : public EventTargetWithInlineData,
                                     public ActiveScriptWrappable<FileReader>,
                                     public ContextLifecycleObserver,
                                     public FileReaderLoaderClient {
  DEFINE_WRAPPERTYPEINFO();
  USING_GARBAGE_COLLECTED_MIXIN(FileReader);

 public:
  static FileReader* Create(ExecutionContext*);

  ~FileReader() override;

  enum ReadyState { kEmpty = 0, kLoading = 1, kDone = 2 };

  void readAsArrayBuffer(Blob*, ExceptionState&);
  void readAsBinaryString(Blob*, ExceptionState&);
  void readAsText(Blob*, const String& encoding, ExceptionState&);
  void readAsText(Blob*, ExceptionState&);
  void readAsDataURL(Blob*, ExceptionState&);
  void abort();

  ReadyState getReadyState() const { return state_; }
  DOMException* error() { return error_; }
  void result(StringOrArrayBuffer& result_attribute) const;

  // ContextLifecycleObserver
  void ContextDestroyed(ExecutionContext*) override;

  // ScriptWrappable
  bool HasPendingActivity() const final;

  // EventTarget
  const AtomicString& InterfaceName() const override;
  ExecutionContext* GetExecutionContext() const override {
    return ContextLifecycleObserver::GetExecutionContext();
  }

  // FileReaderLoaderClient
  void DidStartLoading() override;
  void DidReceiveData() override;
  void DidFinishLoading() override;
  void DidFail(FileError::ErrorCode) override;

  DEFINE_ATTRIBUTE_EVENT_LISTENER(loadstart);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(progress);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(load);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(abort);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(error);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(loadend);

  DECLARE_VIRTUAL_TRACE();

 private:
  class ThrottlingController;

  explicit FileReader(ExecutionContext*);

  void Terminate();
  void ReadInternal(Blob*, FileReaderLoader::ReadType, ExceptionState&);
  void FireEvent(const AtomicString& type);

  void ExecutePendingRead();

  ReadyState state_;

  // Internal loading state, which could differ from ReadyState as it's
  // for script-visible state while this one's for internal state.
  enum LoadingState {
    kLoadingStateNone,
    kLoadingStatePending,
    kLoadingStateLoading,
    kLoadingStateAborted
  };
  LoadingState loading_state_;
  bool still_firing_events_;

  String blob_type_;
  RefPtr<BlobDataHandle> blob_data_handle_;
  FileReaderLoader::ReadType read_type_;
  String encoding_;

  std::unique_ptr<FileReaderLoader> loader_;
  Member<DOMException> error_;
  double last_progress_notification_time_ms_;
};

}  // namespace blink

#endif  // FileReader_h
