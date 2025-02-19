// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/base/gaia_oauth_client.h"

#include "base/callback_helpers.h"
#include "base/logging.h"

namespace {
const int kMaxGaiaRetries = 3;
}  // namespace

namespace remoting {

GaiaOAuthClient::GaiaOAuthClient(
    scoped_refptr<net::URLRequestContextGetter> url_request_context_getter)
    : gaia_oauth_client_(url_request_context_getter.get()) {}

GaiaOAuthClient::~GaiaOAuthClient() {}

void GaiaOAuthClient::GetCredentialsFromAuthCode(
    const gaia::OAuthClientInfo& oauth_client_info,
    const std::string& auth_code,
    bool need_user_email,
    CompletionCallback on_done) {
  if (!on_done_.is_null()) {
    pending_requests_.push(
        Request(oauth_client_info, auth_code, need_user_email, on_done));
    return;
  }

  need_user_email_ = need_user_email;
  on_done_ = on_done;
  // Map the authorization code to refresh and access tokens.
  gaia_oauth_client_.GetTokensFromAuthCode(oauth_client_info, auth_code,
                                           kMaxGaiaRetries, this);
}

void GaiaOAuthClient::OnGetTokensResponse(const std::string& refresh_token,
                                          const std::string& access_token,
                                          int expires_in_seconds) {
  refresh_token_ = refresh_token;
  if (need_user_email_) {
    // Get the email corresponding to the access token.
    gaia_oauth_client_.GetUserEmail(access_token, kMaxGaiaRetries, this);
  } else {
    SendResponse("", refresh_token_);
  }
}

void GaiaOAuthClient::OnRefreshTokenResponse(const std::string& access_token,
                                             int expires_in_seconds) {
  // We never request a refresh token, so this call is not expected.
  NOTREACHED();
}

void GaiaOAuthClient::SendResponse(const std::string& user_email,
                                   const std::string& refresh_token) {
  base::ResetAndReturn(&on_done_).Run(user_email, refresh_token);

  // Process the next request in the queue.
  if (pending_requests_.size()) {
    Request request = pending_requests_.front();
    pending_requests_.pop();
    // GetCredentialsFromAuthCode is asynchronous, so it's safe to call it here.
    GetCredentialsFromAuthCode(request.oauth_client_info, request.auth_code,
                               request.need_user_email, request.on_done);
  }
}

void GaiaOAuthClient::OnGetUserEmailResponse(const std::string& user_email) {
  SendResponse(user_email, refresh_token_);
}

void GaiaOAuthClient::OnOAuthError() {
  SendResponse("", "");
}

void GaiaOAuthClient::OnNetworkError(int response_code) {
  SendResponse("", "");
}

GaiaOAuthClient::Request::Request(
    const gaia::OAuthClientInfo& oauth_client_info,
    const std::string& auth_code,
    bool need_user_email,
    CompletionCallback on_done) {
  this->oauth_client_info = oauth_client_info;
  this->auth_code = auth_code;
  this->need_user_email = need_user_email;
  this->on_done = on_done;
}

GaiaOAuthClient::Request::Request(const Request& other) = default;

GaiaOAuthClient::Request::~Request() {}

}  // namespace remoting
