// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_COMMON_REQUEST_CONTEXT_FRAME_TYPE_H_
#define CONTENT_PUBLIC_COMMON_REQUEST_CONTEXT_FRAME_TYPE_H_

namespace content {

enum RequestContextFrameType {
  REQUEST_CONTEXT_FRAME_TYPE_AUXILIARY = 0,
  REQUEST_CONTEXT_FRAME_TYPE_NESTED,
  REQUEST_CONTEXT_FRAME_TYPE_NONE,
  REQUEST_CONTEXT_FRAME_TYPE_TOP_LEVEL,
  REQUEST_CONTEXT_FRAME_TYPE_LAST = REQUEST_CONTEXT_FRAME_TYPE_TOP_LEVEL
};

}  // namespace content

#endif  // CONTENT_PUBLIC_COMMON_REQUEST_CONTEXT_FRAME_TYPE_H_
