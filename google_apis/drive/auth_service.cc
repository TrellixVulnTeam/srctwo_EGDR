// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "google_apis/drive/auth_service.h"

#include <string>
#include <vector>

#include "base/bind.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/metrics/histogram_macros.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "google_apis/drive/auth_service_observer.h"
#include "google_apis/gaia/google_service_auth_error.h"
#include "net/url_request/url_request_context_getter.h"

namespace google_apis {

namespace {

// Used for success ratio histograms. 0 for failure, 1 for success,
// 2 for no connection (likely offline).
const int kSuccessRatioHistogramFailure = 0;
const int kSuccessRatioHistogramSuccess = 1;
const int kSuccessRatioHistogramNoConnection = 2;
const int kSuccessRatioHistogramTemporaryFailure = 3;
const int kSuccessRatioHistogramMaxValue = 4;  // The max value is exclusive.

void RecordAuthResultHistogram(int value) {
  UMA_HISTOGRAM_ENUMERATION("GData.AuthSuccess",
                            value,
                            kSuccessRatioHistogramMaxValue);
}

// OAuth2 authorization token retrieval request.
class AuthRequest : public OAuth2TokenService::Consumer {
 public:
  AuthRequest(OAuth2TokenService* oauth2_token_service,
              const std::string& account_id,
              net::URLRequestContextGetter* url_request_context_getter,
              const AuthStatusCallback& callback,
              const std::vector<std::string>& scopes);
  ~AuthRequest() override;

 private:
  // Overridden from OAuth2TokenService::Consumer:
  void OnGetTokenSuccess(const OAuth2TokenService::Request* request,
                         const std::string& access_token,
                         const base::Time& expiration_time) override;
  void OnGetTokenFailure(const OAuth2TokenService::Request* request,
                         const GoogleServiceAuthError& error) override;

  AuthStatusCallback callback_;
  std::unique_ptr<OAuth2TokenService::Request> request_;
  base::ThreadChecker thread_checker_;

  DISALLOW_COPY_AND_ASSIGN(AuthRequest);
};

AuthRequest::AuthRequest(
    OAuth2TokenService* oauth2_token_service,
    const std::string& account_id,
    net::URLRequestContextGetter* url_request_context_getter,
    const AuthStatusCallback& callback,
    const std::vector<std::string>& scopes)
    : OAuth2TokenService::Consumer("auth_service"),
      callback_(callback) {
  DCHECK(!callback_.is_null());
  request_ = oauth2_token_service->
      StartRequestWithContext(
          account_id,
          url_request_context_getter,
          OAuth2TokenService::ScopeSet(scopes.begin(), scopes.end()),
          this);
}

AuthRequest::~AuthRequest() {}

// Callback for OAuth2AccessTokenFetcher on success. |access_token| is the token
// used to start fetching user data.
void AuthRequest::OnGetTokenSuccess(const OAuth2TokenService::Request* request,
                                    const std::string& access_token,
                                    const base::Time& expiration_time) {
  DCHECK(thread_checker_.CalledOnValidThread());

  RecordAuthResultHistogram(kSuccessRatioHistogramSuccess);
  callback_.Run(HTTP_SUCCESS, access_token);
  delete this;
}

// Callback for OAuth2AccessTokenFetcher on failure.
void AuthRequest::OnGetTokenFailure(const OAuth2TokenService::Request* request,
                                    const GoogleServiceAuthError& error) {
  DCHECK(thread_checker_.CalledOnValidThread());

  LOG(WARNING) << "AuthRequest: token request using refresh token failed: "
               << error.ToString();

  // There are many ways to fail, but if the failure is due to connection,
  // it's likely that the device is off-line. We treat the error differently
  // so that the file manager works while off-line.
  if (error.state() == GoogleServiceAuthError::CONNECTION_FAILED) {
    RecordAuthResultHistogram(kSuccessRatioHistogramNoConnection);
    callback_.Run(DRIVE_NO_CONNECTION, std::string());
  } else if (error.state() == GoogleServiceAuthError::SERVICE_UNAVAILABLE) {
    RecordAuthResultHistogram(kSuccessRatioHistogramTemporaryFailure);
    callback_.Run(HTTP_FORBIDDEN, std::string());
  } else {
    // Permanent auth error.
    RecordAuthResultHistogram(kSuccessRatioHistogramFailure);
    callback_.Run(HTTP_UNAUTHORIZED, std::string());
  }
  delete this;
}

}  // namespace

AuthService::AuthService(
    OAuth2TokenService* oauth2_token_service,
    const std::string& account_id,
    net::URLRequestContextGetter* url_request_context_getter,
    const std::vector<std::string>& scopes)
    : oauth2_token_service_(oauth2_token_service),
      account_id_(account_id),
      url_request_context_getter_(url_request_context_getter),
      scopes_(scopes),
      weak_ptr_factory_(this) {
  DCHECK(oauth2_token_service);

  // Get OAuth2 refresh token (if we have any) and register for its updates.
  oauth2_token_service_->AddObserver(this);
  has_refresh_token_ = oauth2_token_service_->RefreshTokenIsAvailable(
      account_id_);
}

AuthService::~AuthService() {
  oauth2_token_service_->RemoveObserver(this);
}

void AuthService::StartAuthentication(const AuthStatusCallback& callback) {
  DCHECK(thread_checker_.CalledOnValidThread());

  if (HasAccessToken()) {
    // We already have access token. Give it back to the caller asynchronously.
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::Bind(callback, HTTP_SUCCESS, access_token_));
  } else if (HasRefreshToken()) {
    // We have refresh token, let's get an access token.
    new AuthRequest(oauth2_token_service_,
                    account_id_,
                    url_request_context_getter_.get(),
                    base::Bind(&AuthService::OnAuthCompleted,
                               weak_ptr_factory_.GetWeakPtr(),
                               callback),
                    scopes_);
  } else {
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE, base::Bind(callback, DRIVE_NOT_READY, std::string()));
  }
}

bool AuthService::HasAccessToken() const {
  return !access_token_.empty();
}

bool AuthService::HasRefreshToken() const {
  return has_refresh_token_;
}

const std::string& AuthService::access_token() const {
  return access_token_;
}

void AuthService::ClearAccessToken() {
  access_token_.clear();
}

void AuthService::ClearRefreshToken() {
  OnHandleRefreshToken(false);
}

void AuthService::OnAuthCompleted(const AuthStatusCallback& callback,
                                  DriveApiErrorCode error,
                                  const std::string& access_token) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(!callback.is_null());

  if (error == HTTP_SUCCESS) {
    access_token_ = access_token;
  } else if (error == HTTP_UNAUTHORIZED) {
    // Refreshing access token using the refresh token is failed with 401 error
    // (HTTP_UNAUTHORIZED). This means the current refresh token is invalid for
    // Drive, hence we clear the refresh token here to make HasRefreshToken()
    // false, thus the invalidness is clearly observable.
    // This is not for triggering refetch of the refresh token. UI should
    // show some message to encourage user to log-off and log-in again in order
    // to fetch new valid refresh token.
    ClearRefreshToken();
  }

  callback.Run(error, access_token);
}

void AuthService::AddObserver(AuthServiceObserver* observer) {
  observers_.AddObserver(observer);
}

void AuthService::RemoveObserver(AuthServiceObserver* observer) {
  observers_.RemoveObserver(observer);
}

void AuthService::OnRefreshTokenAvailable(const std::string& account_id) {
  if (account_id == account_id_)
    OnHandleRefreshToken(true);
}

void AuthService::OnRefreshTokenRevoked(const std::string& account_id) {
  if (account_id == account_id_)
    OnHandleRefreshToken(false);
}

void AuthService::OnHandleRefreshToken(bool has_refresh_token) {
  access_token_.clear();
  has_refresh_token_ = has_refresh_token;

  for (auto& observer : observers_)
    observer.OnOAuth2RefreshTokenChanged();
}

}  // namespace google_apis
