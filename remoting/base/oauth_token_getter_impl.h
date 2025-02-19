// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_BASE_OAUTH_TOKEN_GETTER_IMPL_H_
#define REMOTING_BASE_OAUTH_TOKEN_GETTER_IMPL_H_

#include <queue>

#include "base/callback.h"
#include "base/sequence_checker.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "google_apis/gaia/gaia_oauth_client.h"
#include "remoting/base/oauth_token_getter.h"

namespace net {
class URLRequestContextGetter;
}  // namespace net

namespace remoting {

// OAuthTokenGetter accepts an authorization code in the intermediate
// credentials or a refresh token in the authorization credentials. It will
// convert authorization code into a refresh token and access token, you may
// pass in a callback to be notified when a refresh token has been updated.
// OAuthTokenGetter will exchange refresh tokens for access tokens and will
// cache access tokens, refreshing them as needed.
// On first usage it is likely an application will only have an auth code,
// from this you can get a refresh token which can be reused next app launch.
class OAuthTokenGetterImpl : public OAuthTokenGetter,
                             public gaia::GaiaOAuthClient::Delegate {
 public:
  OAuthTokenGetterImpl(
      std::unique_ptr<OAuthIntermediateCredentials> intermediate_credentials,
      const OAuthTokenGetter::CredentialsUpdatedCallback& on_credentials_update,
      const scoped_refptr<net::URLRequestContextGetter>&
          url_request_context_getter,
      bool auto_refresh);
  OAuthTokenGetterImpl(
      std::unique_ptr<OAuthAuthorizationCredentials> authorization_credentials,
      const scoped_refptr<net::URLRequestContextGetter>&
          url_request_context_getter,
      bool auto_refresh);
  ~OAuthTokenGetterImpl() override;

  // OAuthTokenGetter interface.
  void CallWithToken(
      const OAuthTokenGetter::TokenCallback& on_access_token) override;
  void InvalidateCache() override;

 private:
  // gaia::GaiaOAuthClient::Delegate interface.
  void OnGetTokensResponse(const std::string& user_email,
                           const std::string& access_token,
                           int expires_seconds) override;
  void OnRefreshTokenResponse(const std::string& access_token,
                              int expires_in_seconds) override;
  void OnGetUserEmailResponse(const std::string& user_email) override;
  void OnOAuthError() override;
  void OnNetworkError(int response_code) override;

  void UpdateAccessToken(const std::string& access_token, int expires_seconds);
  void NotifyTokenCallbacks(Status status,
                            const std::string& user_email,
                            const std::string& access_token);
  void NotifyUpdatedCallbacks(const std::string& user_email,
                              const std::string& refresh_token);
  void GetOauthTokensFromAuthCode();
  void RefreshAccessToken();

  std::unique_ptr<OAuthIntermediateCredentials> intermediate_credentials_;
  std::unique_ptr<OAuthAuthorizationCredentials> authorization_credentials_;
  std::unique_ptr<gaia::GaiaOAuthClient> gaia_oauth_client_;
  OAuthTokenGetter::CredentialsUpdatedCallback credentials_updated_callback_;
  scoped_refptr<net::URLRequestContextGetter> url_request_context_getter_;

  bool response_pending_ = false;
  bool email_verified_ = false;
  bool email_discovery_ = false;
  std::string oauth_access_token_;
  base::Time access_token_expiry_time_;
  std::queue<OAuthTokenGetter::TokenCallback> pending_callbacks_;
  std::unique_ptr<base::OneShotTimer> refresh_timer_;

  SEQUENCE_CHECKER(sequence_checker_);
};

}  // namespace remoting

#endif  // REMOTING_BASE_OAUTH_TOKEN_GETTER_IMPL_H_
