/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
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

#include "modules/filesystem/DirectoryReaderSync.h"

#include "bindings/core/v8/ExceptionState.h"
#include "modules/filesystem/DirectoryEntry.h"
#include "modules/filesystem/DirectoryEntrySync.h"
#include "modules/filesystem/EntriesCallback.h"
#include "modules/filesystem/EntrySync.h"
#include "modules/filesystem/ErrorCallback.h"
#include "modules/filesystem/FileEntrySync.h"
#include "modules/filesystem/FileSystemCallbacks.h"

namespace blink {

class DirectoryReaderSync::EntriesCallbackHelper final
    : public EntriesCallback {
 public:
  explicit EntriesCallbackHelper(DirectoryReaderSync* reader)
      : reader_(reader) {}

  void handleEvent(const EntryHeapVector& entries) override {
    EntrySyncHeapVector sync_entries;
    sync_entries.ReserveInitialCapacity(entries.size());
    for (size_t i = 0; i < entries.size(); ++i)
      sync_entries.UncheckedAppend(EntrySync::Create(entries[i].Get()));
    reader_->AddEntries(sync_entries);
  }

  DEFINE_INLINE_VIRTUAL_TRACE() {
    visitor->Trace(reader_);
    EntriesCallback::Trace(visitor);
  }

 private:
  Member<DirectoryReaderSync> reader_;
};

class DirectoryReaderSync::ErrorCallbackHelper final
    : public ErrorCallbackBase {
 public:
  explicit ErrorCallbackHelper(DirectoryReaderSync* reader) : reader_(reader) {}

  void Invoke(FileError::ErrorCode error) override { reader_->SetError(error); }

  DEFINE_INLINE_VIRTUAL_TRACE() {
    visitor->Trace(reader_);
    ErrorCallbackBase::Trace(visitor);
  }

 private:
  Member<DirectoryReaderSync> reader_;
};

DirectoryReaderSync::DirectoryReaderSync(DOMFileSystemBase* file_system,
                                         const String& full_path)
    : DirectoryReaderBase(file_system, full_path),
      callbacks_id_(0),
      error_code_(FileError::kOK) {}

DirectoryReaderSync::~DirectoryReaderSync() {}

EntrySyncHeapVector DirectoryReaderSync::readEntries(
    ExceptionState& exception_state) {
  if (!callbacks_id_) {
    callbacks_id_ = Filesystem()->ReadDirectory(
        this, full_path_, new EntriesCallbackHelper(this),
        new ErrorCallbackHelper(this), DOMFileSystemBase::kSynchronous);
  }

  if (error_code_ == FileError::kOK && has_more_entries_ && entries_.IsEmpty())
    file_system_->WaitForAdditionalResult(callbacks_id_);

  if (error_code_ != FileError::kOK) {
    FileError::ThrowDOMException(exception_state, error_code_);
    return EntrySyncHeapVector();
  }

  EntrySyncHeapVector result;
  result.swap(entries_);
  return result;
}

DEFINE_TRACE(DirectoryReaderSync) {
  visitor->Trace(entries_);
  DirectoryReaderBase::Trace(visitor);
}

}  // namespace blink
