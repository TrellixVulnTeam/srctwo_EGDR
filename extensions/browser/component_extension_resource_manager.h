// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_COMPONENT_EXTENSION_RESOURCE_MANAGER_H_
#define EXTENSIONS_BROWSER_COMPONENT_EXTENSION_RESOURCE_MANAGER_H_

namespace base {
class FilePath;
}

namespace extensions {

// This class manages which extension resources actually come from
// the resource bundle.
class ComponentExtensionResourceManager {
 public:
  virtual ~ComponentExtensionResourceManager() {}

  // Checks whether image is a component extension resource. Returns false
  // if a given |resource| does not have a corresponding image in bundled
  // resources. Otherwise fills |resource_id|. This doesn't check if the
  // extension the resource is in is actually a component extension.
  virtual bool IsComponentExtensionResource(
      const base::FilePath& extension_path,
      const base::FilePath& resource_path,
      int* resource_id) const = 0;
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_COMPONENT_EXTENSION_RESOURCE_MANAGER_H_
