// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_PROVISION_FETCHER_IMPL_H_
#define CONTENT_PUBLIC_BROWSER_PROVISION_FETCHER_IMPL_H_

#include <memory>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/provision_fetcher_factory.h"
#include "media/base/provision_fetcher.h"
#include "media/mojo/interfaces/provision_fetcher.mojom.h"

namespace net {
class URLRequestContextGetter;
}

namespace content {

// A media::mojom::ProvisionFetcher implementation based on
// media::ProvisionFetcher.
class CONTENT_EXPORT ProvisionFetcherImpl
    : public media::mojom::ProvisionFetcher {
 public:
  static void Create(net::URLRequestContextGetter* context_getter,
                     media::mojom::ProvisionFetcherRequest request);

  explicit ProvisionFetcherImpl(
      std::unique_ptr<media::ProvisionFetcher> provision_fetcher);
  ~ProvisionFetcherImpl() override;

  // media::mojom::ProvisionFetcher implementation.
  void Retrieve(const std::string& default_url,
                const std::string& request_data,
                RetrieveCallback callback) final;

 private:
  // Callback for media::ProvisionFetcher::Retrieve().
  void OnResponse(RetrieveCallback callback,
                  bool success,
                  const std::string& response);

  std::unique_ptr<media::ProvisionFetcher> provision_fetcher_;

  base::WeakPtrFactory<ProvisionFetcherImpl> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(ProvisionFetcherImpl);
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_PROVISION_FETCHER_IMPL_H_
