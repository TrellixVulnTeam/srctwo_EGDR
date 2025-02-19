// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_POLICY_CLOUD_USER_POLICY_SIGNIN_SERVICE_MOBILE_H_
#define CHROME_BROWSER_POLICY_CLOUD_USER_POLICY_SIGNIN_SERVICE_MOBILE_H_

#include <memory>
#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "build/build_config.h"
#include "chrome/browser/policy/cloud/user_policy_signin_service_base.h"

class ProfileOAuth2TokenService;
class Profile;

namespace net {
class URLRequestContextGetter;
}

namespace policy {

class CloudPolicyClientRegistrationHelper;

// A specialization of UserPolicySigninServiceBase for the Android.
class UserPolicySigninService : public UserPolicySigninServiceBase {
 public:
  // Creates a UserPolicySigninService associated with the passed |profile|.
  UserPolicySigninService(
      Profile* profile,
      PrefService* local_state,
      DeviceManagementService* device_management_service,
      UserCloudPolicyManager* policy_manager,
      SigninManager* signin_manager,
      scoped_refptr<net::URLRequestContextGetter> system_request_context,
      ProfileOAuth2TokenService* token_service);
  ~UserPolicySigninService() override;

  // Registers a CloudPolicyClient for fetching policy for |username|.
  // This requests an OAuth2 token for the services involved, and contacts
  // the policy service if the account has management enabled.
  // |account_id| is the obfuscated identitifcation of |username| to get OAuth2
  // token services.
  // |callback| is invoked once we have registered this device to fetch policy,
  // or once it is determined that |username| is not a managed account.
  void RegisterForPolicyWithAccountId(
      const std::string& username,
      const std::string& account_id,
      const PolicyRegistrationCallback& callback);

  // Overridden from UserPolicySigninServiceBase to cancel the pending delayed
  // registration.
  void ShutdownUserCloudPolicyManager() override;

 private:
  void RegisterForPolicyInternal(const std::string& username,
                                 const std::string& account_id,
                                 const std::string& access_token,
                                 const PolicyRegistrationCallback& callback);

  void CallPolicyRegistrationCallback(std::unique_ptr<CloudPolicyClient> client,
                                      PolicyRegistrationCallback callback);

  // KeyedService implementation:
  void Shutdown() override;

  // CloudPolicyService::Observer implementation:
  void OnInitializationCompleted(CloudPolicyService* service) override;

  // Registers for cloud policy for an already signed-in user.
  void RegisterCloudPolicyService();

  // Cancels a pending cloud policy registration attempt.
  void CancelPendingRegistration();

  void OnRegistrationDone();

  std::unique_ptr<CloudPolicyClientRegistrationHelper> registration_helper_;

  // Weak pointer to the token service used to authenticate the
  // CloudPolicyClient during registration.
  ProfileOAuth2TokenService* oauth2_token_service_;

  // The PrefService associated with the profile.
  PrefService* profile_prefs_;

  base::WeakPtrFactory<UserPolicySigninService> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(UserPolicySigninService);
};

}  // namespace policy

#endif  // CHROME_BROWSER_POLICY_CLOUD_USER_POLICY_SIGNIN_SERVICE_MOBILE_H_
