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

#ifndef Metadata_h
#define Metadata_h

#include "platform/FileMetadata.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"

namespace blink {

class Metadata final : public GarbageCollectedFinalized<Metadata>,
                       public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static Metadata* Create(const FileMetadata& platform_metadata) {
    return new Metadata(platform_metadata);
  }

  static Metadata* Create(Metadata* metadata) {
    return new Metadata(metadata->platform_metadata_);
  }

  // Return Epoch time in milliseconds for Date.
  double modificationTime() const {
    return platform_metadata_.modification_time;
  }
  unsigned long long size() const {
    return static_cast<unsigned long long>(platform_metadata_.length);
  }

  DEFINE_INLINE_TRACE() {}

 private:
  explicit Metadata(const FileMetadata& platform_metadata)
      : platform_metadata_(platform_metadata) {}

  FileMetadata platform_metadata_;
};

}  // namespace blink

#endif  // Metadata_h
