// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PASSWORD_MANAGER_CONTENT_BROWSER_CONTENT_CREDENTIAL_MANAGER_H_
#define COMPONENTS_PASSWORD_MANAGER_CONTENT_BROWSER_CONTENT_CREDENTIAL_MANAGER_H_

#include "components/password_manager/core/browser/credential_manager_impl.h"
#include "components/password_manager/core/common/credential_manager_types.h"
#include "mojo/public/cpp/bindings/associated_binding.h"
#include "third_party/WebKit/public/platform/modules/credentialmanager/credential_manager.mojom.h"

class GURL;

namespace password_manager {
class PasswordManagerClient;
struct CredentialInfo;

// Implements mojom::CredentialManager using core class CredentialManagerImpl.
// Methods Store, PreventSilentAccess and Get are invoked from the renderer
// with callbacks as arguments. PasswordManagerClient is used to invoke UI.
class ContentCredentialManager : public mojom::CredentialManager {
 public:
  explicit ContentCredentialManager(PasswordManagerClient* client);
  ~ContentCredentialManager() override;

  void BindRequest(mojom::CredentialManagerAssociatedRequest request);
  bool HasBinding() const;
  void DisconnectBinding();

  // mojom::CredentialManager methods:
  void Store(const CredentialInfo& credential, StoreCallback callback) override;
  void PreventSilentAccess(PreventSilentAccessCallback callback) override;
  void Get(CredentialMediationRequirement mediation,
           bool include_passwords,
           const std::vector<GURL>& federations,
           GetCallback callback) override;

 private:
  CredentialManagerImpl impl_;

  mojo::AssociatedBinding<mojom::CredentialManager> binding_;

  DISALLOW_COPY_AND_ASSIGN(ContentCredentialManager);
};

}  // namespace password_manager

#endif  // COMPONENTS_PASSWORD_MANAGER_CONTENT_BROWSER_CONTENT_CREDENTIAL_MANAGER_H_
