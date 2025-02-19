// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/extensions/api/file_system_provider_capabilities/file_system_provider_capabilities_handler.h"

#include <memory>

#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "chrome/common/extensions/api/manifest_types.h"
#include "extensions/common/error_utils.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/common/manifest_handlers/permissions_parser.h"
#include "url/gurl.h"

namespace extensions {

FileSystemProviderCapabilities::FileSystemProviderCapabilities()
    : configurable_(false),
      watchable_(false),
      multiple_mounts_(false),
      source_(SOURCE_FILE) {
}

FileSystemProviderCapabilities::FileSystemProviderCapabilities(
    bool configurable,
    bool watchable,
    bool multiple_mounts,
    FileSystemProviderSource source)
    : configurable_(configurable),
      watchable_(watchable),
      multiple_mounts_(multiple_mounts),
      source_(source) {
}

FileSystemProviderCapabilities::~FileSystemProviderCapabilities() {
}

FileSystemProviderCapabilitiesHandler::FileSystemProviderCapabilitiesHandler() {
}

FileSystemProviderCapabilitiesHandler::
    ~FileSystemProviderCapabilitiesHandler() {
}

// static
const FileSystemProviderCapabilities* FileSystemProviderCapabilities::Get(
    const Extension* extension) {
  return static_cast<FileSystemProviderCapabilities*>(
      extension->GetManifestData(
          manifest_keys::kFileSystemProviderCapabilities));
}

bool FileSystemProviderCapabilitiesHandler::Parse(Extension* extension,
                                                  base::string16* error) {
  const bool has_permission = extensions::PermissionsParser::HasAPIPermission(
      extension, extensions::APIPermission::ID::kFileSystemProvider);
  const base::DictionaryValue* section = nullptr;
  extension->manifest()->GetDictionary(
      manifest_keys::kFileSystemProviderCapabilities, &section);

  if (has_permission && !section) {
    *error = base::ASCIIToUTF16(
        manifest_errors::kInvalidFileSystemProviderMissingCapabilities);
    return false;
  }

  if (!has_permission && section) {
    // If the permission is not specified, then the section has simply no
    // effect, hence just a warning.
    extension->AddInstallWarning(extensions::InstallWarning(
        manifest_errors::kInvalidFileSystemProviderMissingPermission));
    return true;
  }

  if (!has_permission && !section) {
    // No permission and no capabilities, so ignore.
    return true;
  }

  api::manifest_types::FileSystemProviderCapabilities idl_capabilities;
  if (!api::manifest_types::FileSystemProviderCapabilities::Populate(
          *section, &idl_capabilities, error)) {
    return false;
  }

  FileSystemProviderSource source = SOURCE_FILE;
  switch (idl_capabilities.source) {
    case api::manifest_types::FILE_SYSTEM_PROVIDER_SOURCE_FILE:
      source = SOURCE_FILE;
      break;
    case api::manifest_types::FILE_SYSTEM_PROVIDER_SOURCE_DEVICE:
      source = SOURCE_DEVICE;
      break;
    case api::manifest_types::FILE_SYSTEM_PROVIDER_SOURCE_NETWORK:
      source = SOURCE_NETWORK;
      break;
    case api::manifest_types::FILE_SYSTEM_PROVIDER_SOURCE_NONE:
      NOTREACHED();
  }

  std::unique_ptr<FileSystemProviderCapabilities> capabilities(
      new FileSystemProviderCapabilities(
          idl_capabilities.configurable.get()
              ? *idl_capabilities.configurable.get()
              : false /* false by default */,
          idl_capabilities.watchable.get() ? *idl_capabilities.watchable.get()
                                           : false /* false by default */,
          idl_capabilities.multiple_mounts.get()
              ? *idl_capabilities.multiple_mounts.get()
              : false /* false by default */,
          source));

  extension->SetManifestData(manifest_keys::kFileSystemProviderCapabilities,
                             std::move(capabilities));
  return true;
}

const std::vector<std::string> FileSystemProviderCapabilitiesHandler::Keys()
    const {
  return SingleKey(manifest_keys::kFileSystemProviderCapabilities);
}

bool FileSystemProviderCapabilitiesHandler::AlwaysParseForType(
    Manifest::Type type) const {
  return true;
}

}  // namespace extensions
