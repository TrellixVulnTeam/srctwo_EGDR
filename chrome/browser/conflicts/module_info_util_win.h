// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CONFLICTS_MODULE_INFO_UTIL_WIN_H_
#define CHROME_BROWSER_CONFLICTS_MODULE_INFO_UTIL_WIN_H_

#include <utility>
#include <vector>

#include "base/files/file_path.h"
#include "base/strings/string16.h"

// A format string for generating paths to COM class in-proc server keys under
// HKEY_CLASSES_ROOT.
extern const wchar_t kClassIdRegistryKeyFormat[];

// The type of certificate found for the module.
enum class CertificateType {
  // The module is not signed.
  NO_CERTIFICATE,
  // The module is signed and the certificate is in the module.
  CERTIFICATE_IN_FILE,
  // The module is signed and the certificate is in an external catalog.
  CERTIFICATE_IN_CATALOG,
};

// Information about the certificate of a file.
struct CertificateInfo {
  CertificateInfo();

  // The type of signature encountered.
  CertificateType type;

  // Path to the file containing the certificate. Empty if |type| is
  // NO_CERTIFICATE.
  base::FilePath path;

  // The "Subject" name of the certificate. This is the signer (ie,
  // "Google Inc." or "Microsoft Inc.").
  base::string16 subject;
};

// Extracts information about the certificate of the given |file|, populating
// |certificate_info|. It is expected that |certificate_info| be freshly
// constructed.
void GetCertificateInfo(const base::FilePath& file,
                        CertificateInfo* certificate_info);

// Returns a mapping of the value of an environment variable to its name.
// Removes any existing trailing backslash in the values.
//
// e.g. c:\windows\system32 -> %systemroot%
using StringMapping = std::vector<std::pair<base::string16, base::string16>>;
StringMapping GetEnvironmentVariablesMapping(
    const std::vector<base::string16>& environment_variables);

// If |prefix_mapping| contains a matching prefix with |path|, substitutes that
// prefix with its associated value. If multiple matches are found, the longest
// prefix is chosen. Unmodified if no matches are found.
//
// This function expects |prefix_mapping| and |path| to contain lowercase
// strings. Also, |prefix_mapping| must not contain any trailing backslashes.
void CollapseMatchingPrefixInPath(const StringMapping& prefix_mapping,
                                  base::string16* path);

// Reads the file on disk to find out the SizeOfImage and TimeDateStamp
// properties of the module. Returns false on error.
bool GetModuleImageSizeAndTimeDateStamp(const base::FilePath& path,
                                        uint32_t* size_of_image,
                                        uint32_t* time_date_stamp);

#endif  // CHROME_BROWSER_CONFLICTS_MODULE_INFO_UTIL_WIN_H_
