// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_PREFETCH_SERVER_URLS_H_
#define COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_PREFETCH_SERVER_URLS_H_

#include <string>
#include "components/version_info/channel.h"
#include "url/gurl.h"

namespace offline_pages {

extern const char kPrefetchServer[];

// Returns the URL to send a request to generate page bundle.
GURL GeneratePageBundleRequestURL(version_info::Channel channel);

// Returns the URL to send a request to get operation info.
GURL GetOperationRequestURL(const std::string& name,
                            version_info::Channel channel);

// Returns the URL to download an archive.
GURL PrefetchDownloadURL(const std::string& download_location,
                         version_info::Channel channel);

}  // namespace offline_pages

#endif  // COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_PREFETCH_SERVER_URLS_H_
