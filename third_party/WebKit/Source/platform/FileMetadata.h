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

#ifndef FileMetadata_h
#define FileMetadata_h

#include <time.h>
#include "platform/PlatformExport.h"
#include "platform/weborigin/KURL.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/MathExtras.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

inline double InvalidFileTime() {
  return std::numeric_limits<double>::quiet_NaN();
}
inline bool IsValidFileTime(double time) {
  return std::isfinite(time);
}

class FileMetadata {
  DISALLOW_NEW();

 public:
  FileMetadata()
      : modification_time(InvalidFileTime()), length(-1), type(kTypeUnknown) {}

  // The last modification time of the file, in milliseconds.
  // The value NaN means that the time is not known.
  double modification_time;

  // The length of the file in bytes.
  // The value -1 means that the length is not set.
  long long length;

  enum Type { kTypeUnknown = 0, kTypeFile, kTypeDirectory };

  Type type;
  String platform_path;
};

PLATFORM_EXPORT bool GetFileSize(const String&, long long& result);
PLATFORM_EXPORT bool GetFileModificationTime(const String&, double& result);
PLATFORM_EXPORT bool GetFileMetadata(const String&, FileMetadata&);
PLATFORM_EXPORT String DirectoryName(const String&);
PLATFORM_EXPORT KURL FilePathToURL(const String&);

}  // namespace blink

#endif  // FileMetadata_h
