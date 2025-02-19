// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// See http://dev.chromium.org/developers/design-documents/multi-process-resource-loading

#ifndef CONTENT_PUBLIC_COMMON_RESOURCE_RESPONSE_H_
#define CONTENT_PUBLIC_COMMON_RESOURCE_RESPONSE_H_

#include <string>

#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "content/common/content_export.h"
#include "content/public/common/resource_response_info.h"
#include "net/url_request/url_request_status.h"
#include "url/gurl.h"

namespace content {

// Parameters for a resource response header.
struct ResourceResponseHead : ResourceResponseInfo {
  // TimeTicks::Now() when the browser received the request from the renderer.
  base::TimeTicks request_start;
  // TimeTicks::Now() when the browser sent the response to the renderer.
  base::TimeTicks response_start;
};

// Parameters for a synchronous resource response.
struct SyncLoadResult : ResourceResponseHead {
  // The response error_code.
  int error_code;
  // The final URL after any redirects.
  GURL final_url;

  // The response data.
  std::string data;
};

// Simple wrapper that refcounts ResourceResponseHead.
// Inherited, rather than typedef'd, to allow forward declarations.
struct CONTENT_EXPORT ResourceResponse
    : public base::RefCountedThreadSafe<ResourceResponse> {
 public:
  ResourceResponseHead head;

  // Performs a deep copy of the ResourceResponse and all fields in it, safe to
  // pass across threads.
  //
  // TODO(davidben): This structure should be passed along in a scoped_ptr. It's
  // currently reference-counted to avoid copies, but may be
  // modified. https://crbug.com/416050
  scoped_refptr<ResourceResponse> DeepCopy() const;

 private:
  friend class base::RefCountedThreadSafe<ResourceResponse>;
  ~ResourceResponse() {}
};

}  // namespace content

#endif  // CONTENT_PUBLIC_COMMON_RESOURCE_RESPONSE_H_
