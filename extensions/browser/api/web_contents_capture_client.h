// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_API_WEB_CONTENTS_CAPTURE_CLIENT_H_
#define EXTENSIONS_BROWSER_API_WEB_CONTENTS_CAPTURE_CLIENT_H_

#include "base/macros.h"
#include "content/public/browser/readback_types.h"
#include "extensions/browser/extension_function.h"
#include "extensions/common/api/extension_types.h"

class SkBitmap;

namespace content {
class WebContents;
}

namespace extensions {

// Base class for capturing visible area of a WebContents.
// This is used by both webview.captureVisibleRegion and tabs.captureVisibleTab.
class WebContentsCaptureClient {
 public:
  WebContentsCaptureClient() {}

 protected:
  virtual ~WebContentsCaptureClient() {}

  virtual bool IsScreenshotEnabled() = 0;
  virtual bool ClientAllowsTransparency() = 0;

  enum FailureReason {
    FAILURE_REASON_UNKNOWN,
    FAILURE_REASON_ENCODING_FAILED,
    FAILURE_REASON_VIEW_INVISIBLE
  };
  bool CaptureAsync(content::WebContents* web_contents,
                    const api::extension_types::ImageDetails* image_detail,
                    const content::ReadbackRequestCallback callback);
  bool EncodeBitmap(const SkBitmap& bitmap, std::string* base64_result);
  virtual void OnCaptureFailure(FailureReason reason) = 0;
  virtual void OnCaptureSuccess(const SkBitmap& bitmap) = 0;
  void CopyFromSurfaceComplete(const SkBitmap& bitmap,
                               content::ReadbackResponse response);

 private:
  // The format (JPEG vs PNG) of the resulting image.  Set in RunAsync().
  api::extension_types::ImageFormat image_format_;

  // Quality setting to use when encoding jpegs.  Set in RunAsync().
  int image_quality_;

  DISALLOW_COPY_AND_ASSIGN(WebContentsCaptureClient);
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_API_WEB_CONTENTS_CAPTURE_CLIENT_H_
