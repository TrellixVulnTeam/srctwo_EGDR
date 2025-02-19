// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_LAYERS_NINE_PATCH_LAYER_IMPL_H_
#define CC_LAYERS_NINE_PATCH_LAYER_IMPL_H_

#include <string>

#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "cc/cc_export.h"
#include "cc/layers/layer_impl.h"
#include "cc/layers/ui_resource_layer_impl.h"
#include "cc/quads/nine_patch_generator.h"
#include "cc/resources/resource_provider.h"
#include "cc/resources/ui_resource_client.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"

namespace base {
class DictionaryValue;
}

namespace cc {

class CC_EXPORT NinePatchLayerImpl : public UIResourceLayerImpl {
 public:
  static std::unique_ptr<NinePatchLayerImpl> Create(LayerTreeImpl* tree_impl,
                                                    int id) {
    return base::WrapUnique(new NinePatchLayerImpl(tree_impl, id));
  }
  ~NinePatchLayerImpl() override;

  // For parameter meanings, see the declaration of NinePatchGenerator.
  void SetLayout(const gfx::Rect& image_aperture,
                 const gfx::Rect& border,
                 const gfx::Rect& layer_occlusion,
                 bool fill_center,
                 bool nearest_neighbor);

  std::unique_ptr<LayerImpl> CreateLayerImpl(LayerTreeImpl* tree_impl) override;
  void PushPropertiesTo(LayerImpl* layer) override;

  void AppendQuads(RenderPass* render_pass,
                   AppendQuadsData* append_quads_data) override;

  std::unique_ptr<base::DictionaryValue> LayerTreeAsJson() override;

 protected:
  NinePatchLayerImpl(LayerTreeImpl* tree_impl, int id);

 private:
  const char* LayerTypeAsString() const override;

  NinePatchGenerator quad_generator_;

  DISALLOW_COPY_AND_ASSIGN(NinePatchLayerImpl);
};

}  // namespace cc

#endif  // CC_LAYERS_NINE_PATCH_LAYER_IMPL_H_
