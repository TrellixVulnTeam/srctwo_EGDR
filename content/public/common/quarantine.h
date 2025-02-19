// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_COMMON_QUARANTINE_H_
#define CONTENT_PUBLIC_COMMON_QUARANTINE_H_

#include <string>

#include "content/common/content_export.h"

class GURL;

namespace base {
class FilePath;
}

namespace content {

// Return value for QuarantineFile.
enum class QuarantineFileResult {
  OK,             // Success.
  ACCESS_DENIED,  // Access to the file was denied. The safety of the file could
                  // not be determined.
  BLOCKED_BY_POLICY,  // Downloads from |source_url| are not allowed by policy.
                      // The file has been deleted.
  ANNOTATION_FAILED,  // Unable to write the mark-of-the-web or otherwise
                      // annotate the file as being downloaded from
                      // |source_url|.
  FILE_MISSING,       // |file| does not name a valid file.
  SECURITY_CHECK_FAILED,  // An unknown error occurred while checking |file|.
                          // The file may have been deleted.
  VIRUS_INFECTED  // |file| was found to be infected by a virus and was deleted.
};

// Quarantine a file that was downloaded from the internet.
//
// Ensures that |file| is handled as safely as possible given that it was
// downloaded from |source_url|. The details of how a downloaded file is handled
// are platform dependent. Please refer to the individual quarantine_<os>
// implementation.
//
// This function should be called for all files downloaded from the internet and
// placed in a manner discoverable by the user, or exposed to an external
// application. Furthermore, it should be called:
//
// * **AFTER** all the data has been written to the file. On Windows, registered
//   anti-virus products will be invoked for scanning the contents of the file.
//   Hence it's important to have the final contents of the file be available at
//   the point at which this function is called.
//
//   Exception: Zero-length files will be handled solely on the basis of the
//   |source_url| and the file type. This exception accommodates situations
//   where the file contents cannot be determined before it is made visible to
//   an external application.
//
// * **AFTER** the file has been renamed to its final name. The file type is
//   significant and is derived from the filename.
//
// * **BEFORE** the file is made visible to an external application or the user.
//   Security checks and mark-of-the-web annotations must be made prior to
//   exposing the file externally.
//
// Note that it is possible for this method to take a long time to complete
// (several seconds or more). In addition to blocking during this time, this
// delay also introduces a window during which a browser shutdown may leave the
// downloaded file unannotated.
//
// Parameters:
//   |file| : Final name of the file.
//   |source_url|: URL from which the file content was downloaded.
//   |referrer_url|: Referring URL.
//   |client_guid|: Only used on Windows. Identifies the client application
//     that downloaded the file.
CONTENT_EXPORT QuarantineFileResult
QuarantineFile(const base::FilePath& file,
               const GURL& source_url,
               const GURL& referrer_url,
               const std::string& client_guid);

// Determine if a file has quarantine metadata attached to it.
//
// If |source_url| is non-empty, then the download source URL in
// quarantine metadata should match |source_url| exactly. The function returns
// |false| if there is a mismatch. If |source_url| is empty, then this function
// only checks for the existence of a download source URL in quarantine
// metadata.
//
// If |referrer_url| is valid, then the download referrer URL in quarantine
// metadata must match |referrer_url| exactly. The function returns |false| if
// there is a mismatch in the |referrer_url| even if the |source_url| matches.
// No referrer URL checks are performed if |referrer_url| is empty.
//
// If both |source_url| and |referrer_url|, then the funciton returns true if
// any quarantine metadata is present for the file.
//
// **Note**: On Windows, this function only checks if the |ZoneIdentifier|
// metadata is present. |source_url| and |referrer_url| are ignored. Windows
// currently doesn't store individual URLs as part of the mark-of-the-web.
CONTENT_EXPORT bool IsFileQuarantined(const base::FilePath& file,
                                      const GURL& source_url,
                                      const GURL& referrer_url);

}  // namespace content

#endif  // CONTENT_PUBLIC_COMMON_QUARANTINE_H_
