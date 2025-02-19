// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/content_verifier_io_data.h"

#include <utility>

#include "content/public/browser/browser_thread.h"

namespace extensions {

ContentVerifierIOData::ExtensionData::ExtensionData(
    std::unique_ptr<std::set<base::FilePath>> browser_image_paths,
    const base::Version& version) {
  this->browser_image_paths = std::move(browser_image_paths);
  this->version = version;
}

ContentVerifierIOData::ContentVerifierIOData() {
}

ContentVerifierIOData::ExtensionData::~ExtensionData() {
}

ContentVerifierIOData::~ContentVerifierIOData() {
}

void ContentVerifierIOData::AddData(const std::string& extension_id,
                                    std::unique_ptr<ExtensionData> data) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  CHECK(data->browser_image_paths.get());
  data_map_[extension_id] = std::move(data);
}

void ContentVerifierIOData::RemoveData(const std::string& extension_id) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  data_map_.erase(extension_id);
}

void ContentVerifierIOData::Clear() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  data_map_.clear();
}

const ContentVerifierIOData::ExtensionData* ContentVerifierIOData::GetData(
    const std::string& extension_id) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  std::map<std::string, std::unique_ptr<ExtensionData>>::iterator found =
      data_map_.find(extension_id);
  if (found != data_map_.end())
    return found->second.get();
  else
    return NULL;
}

}  // namespace extensions
