// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_COMPOSITOR_LAYER_OWNER_H_
#define UI_COMPOSITOR_LAYER_OWNER_H_

#include <memory>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "ui/compositor/compositor_export.h"
#include "ui/compositor/layer.h"

namespace ui {
class LayerOwnerDelegate;

class COMPOSITOR_EXPORT LayerOwner {
 public:
  LayerOwner();
  virtual ~LayerOwner();

  void SetLayer(std::unique_ptr<Layer> layer);

  // Releases the owning reference to its layer, and returns it.
  // This is used when you need to animate the presentation of the owner just
  // prior to destroying it. The Owner can be destroyed soon after calling this
  // function, and the caller is then responsible for disposing of the layer
  // once any animation completes. Note that layer() will remain valid until the
  // end of ~LayerOwner().
  std::unique_ptr<Layer> AcquireLayer();

  // Asks the owner to recreate the layer, returning the old Layer. NULL is
  // returned if there is no existing layer, or recreate is not supported.
  //
  // This does not recurse. Existing children of the layer are moved to the new
  // layer.
  virtual std::unique_ptr<Layer> RecreateLayer();

  ui::Layer* layer() { return layer_; }
  const ui::Layer* layer() const { return layer_; }

  void set_layer_owner_delegate(LayerOwnerDelegate* delegate) {
    layer_owner_delegate_ = delegate;
  }

  bool OwnsLayer() const;

 protected:
  void DestroyLayer();

 private:
  // The LayerOwner owns its layer unless ownership is relinquished via a call
  // to AcquireLayer(). After that moment |layer_| will still be valid but
  // |layer_owner_| will be NULL. The reason for releasing ownership is that
  // the client may wish to animate the layer beyond the lifetime of the owner,
  // e.g. fading it out when it is destroyed.
  std::unique_ptr<Layer> layer_owner_;
  Layer* layer_;

  LayerOwnerDelegate* layer_owner_delegate_;

  DISALLOW_COPY_AND_ASSIGN(LayerOwner);
};

}  // namespace ui

#endif  // UI_COMPOSITOR_LAYER_OWNER_H_
