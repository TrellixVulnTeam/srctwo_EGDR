// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <string>

#include "base/callback.h"
#include "components/nacl/renderer/ppb_nacl_private.h"
#include "third_party/WebKit/public/web/WebAssociatedURLLoaderClient.h"

namespace blink {
class WebAssociatedURLLoader;
struct WebURLError;
class WebURLRequest;
class WebURLResponse;
}

namespace nacl {

// Downloads a NaCl manifest (.nmf) and returns the contents of the file to
// caller through a callback.
class ManifestDownloader : public blink::WebAssociatedURLLoaderClient {
 public:
  typedef base::Callback<void(PP_NaClError, const std::string&)> Callback;

  // This is a pretty arbitrary limit on the byte size of the NaCl manifest
  // file.
  // Note that the resulting string object has to have at least one byte extra
  // for the null termination character.
  static const size_t kNaClManifestMaxFileBytes = 1024 * 1024;

  ManifestDownloader(std::unique_ptr<blink::WebAssociatedURLLoader> url_loader,
                     bool is_installed,
                     Callback cb);
  ~ManifestDownloader() override;

  void Load(const blink::WebURLRequest& request);

 private:
  void Close();

  // WebAssociatedURLLoaderClient implementation.
  void DidReceiveResponse(const blink::WebURLResponse& response) override;
  void DidReceiveData(const char* data, int data_length) override;
  void DidFinishLoading(double finish_time) override;
  void DidFail(const blink::WebURLError& error) override;

  std::unique_ptr<blink::WebAssociatedURLLoader> url_loader_;
  bool is_installed_;
  Callback cb_;
  std::string buffer_;
  int status_code_;
  PP_NaClError pp_nacl_error_;
};

}  // namespace nacl
