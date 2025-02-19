// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_IMAGE_FETCHER_CORE_REQUEST_METADATA_H_
#define COMPONENTS_IMAGE_FETCHER_CORE_REQUEST_METADATA_H_

#include <string>

#include "net/url_request/url_fetcher.h"

namespace image_fetcher {

// Metadata for a URL request.
struct RequestMetadata {
  // Impossible http response code. Used to signal that no http response code
  // was received.
  enum ResponseCode {
    RESPONSE_CODE_INVALID = net::URLFetcher::RESPONSE_CODE_INVALID
  };

  RequestMetadata();

  std::string mime_type;
  int http_response_code;
  bool from_http_cache;
  std::string content_location_header;
};

bool operator==(const RequestMetadata& lhs, const RequestMetadata& rhs);
bool operator!=(const RequestMetadata& lhs, const RequestMetadata& rhs);

}  // namespace image_fetcher

#endif  // COMPONENTS_IMAGE_FETCHER_CORE_REQUEST_METADATA_H_
