// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/installer/test/resource_loader.h"

#include <windows.h>

#include "base/files/file_path.h"
#include "base/logging.h"

namespace {

// Populates |resource_data| with the address and size of the resource in
// |module| identified by |name_or_id| of type |type_name_or_id|, returning
// true on success.
bool DoLoad(HMODULE module,
            const wchar_t* name_or_id,
            const wchar_t* type_name_or_id,
            std::pair<const uint8_t*, DWORD>* resource_data) {
  bool loaded = false;
  HRSRC resource_info;

  resource_info = FindResource(module, name_or_id, type_name_or_id);
  if (resource_info != NULL) {
    HGLOBAL loaded_resource;

    loaded_resource = LoadResource(module, resource_info);
    if (loaded_resource != NULL) {
      resource_data->first =
          static_cast<const uint8_t*>(LockResource(loaded_resource));
      if (resource_data->first != NULL) {
        resource_data->second = SizeofResource(module, resource_info);
        DPCHECK(resource_data->second != 0);
        loaded = true;
      } else {
        DPCHECK(false) << "LockResource failed";
      }
    } else {
      DPCHECK(false) << "LoadResource failed";
    }
  } else {
    DPLOG(INFO) << "FindResource failed";
  }

  return loaded;
}

}  // namespace

namespace upgrade_test {

ResourceLoader::ResourceLoader() : module_(NULL) {
}

ResourceLoader::~ResourceLoader() {
  if (module_ != NULL) {
    BOOL result = FreeLibrary(module_);
    DPCHECK(result != 0) << "FreeLibrary failed";
  }
}

bool ResourceLoader::Initialize(const base::FilePath& pe_image_path) {
  DCHECK(module_ == NULL);
  module_ = LoadLibraryEx(pe_image_path.value().c_str(), NULL,
                          (LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE |
                           LOAD_LIBRARY_AS_IMAGE_RESOURCE));
  DPLOG_IF(INFO, module_ == NULL)
      << "Failed loading \"" << pe_image_path.value() << "\"";
  return module_ != NULL;
}

bool ResourceLoader::Load(const std::wstring& name,
                          const std::wstring& type,
                          std::pair<const uint8_t*, DWORD>* resource_data) {
  DCHECK(resource_data != NULL);
  DCHECK(module_ != NULL);

  return DoLoad(module_, name.c_str(), type.c_str(), resource_data);
}

bool ResourceLoader::Load(WORD id,
                          WORD type,
                          std::pair<const uint8_t*, DWORD>* resource_data) {
  DCHECK(resource_data != NULL);
  DCHECK(module_ != NULL);

  return DoLoad(module_, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type),
                resource_data);
}

}  // namespace upgrade_test
