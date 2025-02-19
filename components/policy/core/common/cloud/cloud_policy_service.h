// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_POLICY_CORE_COMMON_CLOUD_CLOUD_POLICY_SERVICE_H_
#define COMPONENTS_POLICY_CORE_COMMON_CLOUD_CLOUD_POLICY_SERVICE_H_

#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/observer_list.h"
#include "components/policy/core/common/cloud/cloud_policy_client.h"
#include "components/policy/core/common/cloud/cloud_policy_store.h"
#include "components/policy/policy_export.h"

namespace policy {

// Coordinates cloud policy handling, moving downloaded policy from the client
// to the store, and setting up client registrations from cached data in the
// store. Also coordinates actions on policy refresh triggers.
class POLICY_EXPORT CloudPolicyService : public CloudPolicyClient::Observer,
                                         public CloudPolicyStore::Observer {
 public:
  // Callback invoked once the policy refresh attempt has completed. Passed
  // bool parameter is true if the refresh was successful (no error).
  using RefreshPolicyCallback = base::Callback<void(bool)>;

  // Callback invoked once the unregister attempt has completed. Passed bool
  // parameter is true if unregistering was successful (no error).
  using UnregisterCallback = base::Callback<void(bool)>;

  class POLICY_EXPORT Observer {
   public:
    // Invoked when CloudPolicyService has finished initializing (any initial
    // policy load activity has completed and the CloudPolicyClient has
    // been registered, if possible).
    virtual void OnInitializationCompleted(CloudPolicyService* service) = 0;
    virtual ~Observer() {}
  };

  // |client| and |store| must remain valid for the object life time.
  CloudPolicyService(const std::string& policy_type,
                     const std::string& settings_entity_id,
                     CloudPolicyClient* client,
                     CloudPolicyStore* store);
  ~CloudPolicyService() override;

  // Returns the domain that manages this user/device, according to the current
  // policy blob. Empty if not managed/not available.
  std::string ManagedBy() const;

  // Refreshes policy. |callback| will be invoked after the operation completes
  // or aborts because of errors.
  void RefreshPolicy(const RefreshPolicyCallback& callback);

  // Unregisters the device. |callback| will be invoked after the operation
  // completes or aborts because of errors. All pending refresh policy requests
  // will be aborted, and no further refresh policy requests will be allowed.
  void Unregister(const UnregisterCallback& callback);

  // Adds/Removes an Observer for this object.
  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  // CloudPolicyClient::Observer:
  void OnPolicyFetched(CloudPolicyClient* client) override;
  void OnRegistrationStateChanged(CloudPolicyClient* client) override;
  void OnClientError(CloudPolicyClient* client) override;

  // CloudPolicyStore::Observer:
  void OnStoreLoaded(CloudPolicyStore* store) override;
  void OnStoreError(CloudPolicyStore* store) override;

  bool IsInitializationComplete() const { return initialization_complete_; }

 private:
  // Helper function that is called when initialization may be complete, and
  // which is responsible for notifying observers.
  void CheckInitializationCompleted();

  // Invokes the refresh callbacks and clears refresh state. The |success| flag
  // is passed through to the refresh callbacks.
  void RefreshCompleted(bool success);

  // Invokes the unregister callback and clears unregister state. The |success|
  // flag is passed through to the unregister callback.
  void UnregisterCompleted(bool success);

  // The policy type that will be fetched by the |client_|, with the optional
  // |settings_entity_id_|.
  std::string policy_type_;
  std::string settings_entity_id_;

  // The client used to talk to the cloud.
  CloudPolicyClient* client_;

  // Takes care of persisting and decoding cloud policy.
  CloudPolicyStore* store_;

  // Tracks the state of a pending refresh operation, if any.
  enum {
    // No refresh pending.
    REFRESH_NONE,
    // Policy fetch is pending.
    REFRESH_POLICY_FETCH,
    // Policy store is pending.
    REFRESH_POLICY_STORE,
  } refresh_state_;

  // Tracks the state of a pending unregister operation, if any.
  enum {
    UNREGISTER_NONE,
    UNREGISTER_PENDING,
  } unregister_state_;

  // Callbacks to invoke upon policy refresh.
  std::vector<RefreshPolicyCallback> refresh_callbacks_;

  UnregisterCallback unregister_callback_;

  // Set to true once the service is initialized (initial policy load/refresh
  // is complete).
  bool initialization_complete_;

  // Observers who will receive notifications when the service has finished
  // initializing.
  base::ObserverList<Observer, true> observers_;

  DISALLOW_COPY_AND_ASSIGN(CloudPolicyService);
};

}  // namespace policy

#endif  // COMPONENTS_POLICY_CORE_COMMON_CLOUD_CLOUD_POLICY_SERVICE_H_
