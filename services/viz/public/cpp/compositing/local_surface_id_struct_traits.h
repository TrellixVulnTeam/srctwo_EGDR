// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_VIZ_PUBLIC_CPP_COMPOSITING_LOCAL_SURFACE_ID_STRUCT_TRAITS_H_
#define SERVICES_VIZ_PUBLIC_CPP_COMPOSITING_LOCAL_SURFACE_ID_STRUCT_TRAITS_H_

#include "components/viz/common/surfaces/local_surface_id.h"
#include "mojo/common/common_custom_types_struct_traits.h"
#include "services/viz/public/interfaces/compositing/local_surface_id.mojom-shared.h"

namespace mojo {

template <>
struct StructTraits<viz::mojom::LocalSurfaceIdDataView, viz::LocalSurfaceId> {
  static uint32_t local_id(const viz::LocalSurfaceId& local_surface_id) {
    return local_surface_id.local_id();
  }

  static const base::UnguessableToken& nonce(
      const viz::LocalSurfaceId& local_surface_id) {
    return local_surface_id.nonce();
  }

  static bool Read(viz::mojom::LocalSurfaceIdDataView data,
                   viz::LocalSurfaceId* out) {
    out->local_id_ = data.local_id();
    return data.ReadNonce(&out->nonce_);
  }
};

}  // namespace mojo

#endif  // SERVICES_VIZ_PUBLIC_CPP_COMPOSITING_LOCAL_SURFACE_ID_STRUCT_TRAITS_H_
