// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_COMMON_PAGE_TYPE_H_
#define CONTENT_PUBLIC_COMMON_PAGE_TYPE_H_

namespace content {

// The type of the page an entry corresponds to.  Used by chrome_frame and the
// automation layer to detect the state of a WebContents.
enum PageType {
  PAGE_TYPE_NORMAL = 0,
  PAGE_TYPE_ERROR,
  PAGE_TYPE_INTERSTITIAL
};

}  // namespace content

#endif  // CONTENT_PUBLIC_COMMON_PAGE_TYPE_H_
