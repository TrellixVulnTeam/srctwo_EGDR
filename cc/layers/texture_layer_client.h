// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_LAYERS_TEXTURE_LAYER_CLIENT_H_
#define CC_LAYERS_TEXTURE_LAYER_CLIENT_H_

#include "components/viz/common/quads/single_release_callback.h"

namespace viz {
class TextureMailbox;
}

namespace cc {

class TextureLayerClient {
 public:
  // Returns true and provides a mailbox if a new frame is available.
  // Returns false if no new data is available
  // and the old mailbox is to be reused.
  virtual bool PrepareTextureMailbox(
      viz::TextureMailbox* mailbox,
      std::unique_ptr<viz::SingleReleaseCallback>* release_callback) = 0;

 protected:
  virtual ~TextureLayerClient() {}
};

}  // namespace cc

#endif  // CC_LAYERS_TEXTURE_LAYER_CLIENT_H_
