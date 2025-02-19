// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_NET_DEFAULT_NETWORK_CONTEXT_PARAMS_H_
#define CHROME_BROWSER_NET_DEFAULT_NETWORK_CONTEXT_PARAMS_H_

#include "content/public/common/network_service.mojom.h"

// Returns default set of parameters for configuring the network service.
content::mojom::NetworkContextParamsPtr CreateDefaultNetworkContextParams();

#endif  // CHROME_BROWSER_NET_DEFAULT_NETWORK_CONTEXT_PARAMS_H_
