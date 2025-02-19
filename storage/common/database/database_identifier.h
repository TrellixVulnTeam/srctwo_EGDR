// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STORAGE_COMMON_DATABASE_DATABASE_IDENTIFIER_H_
#define STORAGE_COMMON_DATABASE_DATABASE_IDENTIFIER_H_

#include <string>

#include "base/strings/string_piece.h"
#include "storage/common/storage_common_export.h"
#include "url/gurl.h"

namespace storage {

STORAGE_COMMON_EXPORT std::string GetIdentifierFromOrigin(
    const GURL& origin);
STORAGE_COMMON_EXPORT GURL GetOriginFromIdentifier(
    const std::string& identifier);
STORAGE_COMMON_EXPORT bool IsValidOriginIdentifier(
    const std::string& identifier);

class STORAGE_COMMON_EXPORT DatabaseIdentifier {
 public:
  static const DatabaseIdentifier UniqueFileIdentifier();
  static DatabaseIdentifier CreateFromOrigin(const GURL& origin);
  static DatabaseIdentifier Parse(const std::string& identifier);
  ~DatabaseIdentifier();

  std::string ToString() const;
  GURL ToOrigin() const;

  std::string scheme() const { return scheme_; }
  std::string hostname() const { return hostname_; }
  int port() const { return port_; }
  bool is_unique() const { return is_unique_; }

 private:
  DatabaseIdentifier();
  DatabaseIdentifier(const std::string& scheme,
                     const std::string& hostname,
                     int port,
                     bool is_unique,
                     bool is_file);

  std::string scheme_;
  std::string hostname_;
  int port_;
  bool is_unique_;
  bool is_file_;
};

}  // namespace storage

#endif  // STORAGE_COMMON_DATABASE_DATABASE_IDENTIFIER_H_
