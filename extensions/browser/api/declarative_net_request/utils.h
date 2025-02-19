// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#ifndef EXTENSIONS_BROWSER_API_DECLARATIVE_NET_REQUEST_UTILS_H_
#define EXTENSIONS_BROWSER_API_DECLARATIVE_NET_REQUEST_UTILS_H_

namespace base {
class FilePath;
class Value;
}  // namespace base

namespace extensions {
class Extension;
struct InstallWarning;

namespace declarative_net_request {

// Indexes and persists |rules| for |extension|. In case of an error, returns
// false and populates |error|.
// Note: This must be called on a thread where file IO is allowed.
bool IndexAndPersistRules(const base::Value& rules,
                          const Extension& extension,
                          std::string* error,
                          std::vector<InstallWarning>* warnings);

// Returns the path where the indexed ruleset file should be persisted for
// |extension_path|.
base::FilePath GetIndexedRulesetPath(const base::FilePath& extension_path);

}  // namespace declarative_net_request
}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_API_DECLARATIVE_NET_REQUEST_UTILS_H_
