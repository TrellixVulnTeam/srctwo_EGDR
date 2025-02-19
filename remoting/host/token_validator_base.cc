// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/token_validator_base.h"

#include <stddef.h>

#include "base/base64.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/memory/weak_ptr.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "build/build_config.h"
#include "net/base/escape.h"
#include "net/base/io_buffer.h"
#include "net/base/request_priority.h"
#include "net/base/upload_bytes_element_reader.h"
#include "net/base/upload_data_stream.h"
#include "net/ssl/client_cert_store.h"
#if defined(USE_NSS_CERTS)
#include "net/ssl/client_cert_store_nss.h"
#elif defined(OS_WIN)
#include "net/ssl/client_cert_store_win.h"
#elif defined(OS_MACOSX)
#include "net/ssl/client_cert_store_mac.h"
#endif
#include "net/ssl/ssl_cert_request_info.h"
#include "net/ssl/ssl_private_key.h"
#include "net/url_request/redirect_info.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_status.h"
#include "remoting/base/logging.h"
#include "url/gurl.h"

namespace {

const int kBufferSize = 4096;
const char kCertIssuerWildCard[] = "*";

// The certificate is valid if:
// * The certificate issuer matches exactly |issuer| or the |issuer| is a
//   wildcard. And
// * |now| is within [valid_start, valid_expiry].
bool IsCertificateValid(const std::string& issuer,
                        const base::Time& now,
                        const net::X509Certificate* cert) {
  return (issuer == kCertIssuerWildCard ||
      issuer == cert->issuer().common_name) &&
      cert->valid_start() <= now && cert->valid_expiry() > now;
}

// Returns true if the certificate |c1| is worse than |c2|.
//
// Criteria:
// 1. An invalid certificate is always worse than a valid certificate.
// 2. Invalid certificates are equally bad, in which case false will be
//    returned.
// 3. A certificate with earlier |valid_start| time is worse.
// 4. When |valid_start| are the same, the certificate with earlier
//    |valid_expiry| is worse.
bool WorseThan(const std::string& issuer,
               const base::Time& now,
               const net::X509Certificate* c1,
               const net::X509Certificate* c2) {
  if (!IsCertificateValid(issuer, now, c2))
    return false;

  if (!IsCertificateValid(issuer, now, c1))
    return true;

  if (c1->valid_start() != c2->valid_start())
    return c1->valid_start() < c2->valid_start();

  return c1->valid_expiry() < c2->valid_expiry();
}

}  // namespace

namespace remoting {

TokenValidatorBase::TokenValidatorBase(
    const ThirdPartyAuthConfig& third_party_auth_config,
    const std::string& token_scope,
    scoped_refptr<net::URLRequestContextGetter> request_context_getter)
    : third_party_auth_config_(third_party_auth_config),
      token_scope_(token_scope),
      request_context_getter_(request_context_getter),
      buffer_(new net::IOBuffer(kBufferSize)),
      weak_factory_(this) {
  DCHECK(third_party_auth_config_.token_url.is_valid());
  DCHECK(third_party_auth_config_.token_validation_url.is_valid());
}

TokenValidatorBase::~TokenValidatorBase() {
}

// TokenValidator interface.
void TokenValidatorBase::ValidateThirdPartyToken(
    const std::string& token,
    const base::Callback<void(
        const std::string& shared_secret)>& on_token_validated) {
  DCHECK(!request_);
  DCHECK(!on_token_validated.is_null());

  on_token_validated_ = on_token_validated;
  token_ = token;
  StartValidateRequest(token);
}

const GURL& TokenValidatorBase::token_url() const {
  return third_party_auth_config_.token_url;
}

const std::string& TokenValidatorBase::token_scope() const {
  return token_scope_;
}

// URLFetcherDelegate interface.
void TokenValidatorBase::OnResponseStarted(net::URLRequest* source,
                                           int net_result) {
  DCHECK_NE(net_result, net::ERR_IO_PENDING);
  DCHECK_EQ(request_.get(), source);

  if (net_result != net::OK) {
    // Process all network errors in the same manner as read errors.
    OnReadCompleted(request_.get(), net_result);
    return;
  }

  int bytes_read = request_->Read(buffer_.get(), kBufferSize);
  if (bytes_read != net::ERR_IO_PENDING)
    OnReadCompleted(request_.get(), bytes_read);
}

void TokenValidatorBase::OnReadCompleted(net::URLRequest* source,
                                         int net_result) {
  DCHECK_NE(net_result, net::ERR_IO_PENDING);
  DCHECK_EQ(request_.get(), source);

  while (net_result > 0) {
    data_.append(buffer_->data(), net_result);
    net_result = request_->Read(buffer_.get(), kBufferSize);
  }

  if (net_result == net::ERR_IO_PENDING)
    return;

  retrying_request_ = false;
  std::string shared_token = ProcessResponse(net_result);
  request_.reset();
  on_token_validated_.Run(shared_token);
}

void TokenValidatorBase::OnReceivedRedirect(
    net::URLRequest* request,
    const net::RedirectInfo& redirect_info,
    bool* defer_redirect) {
  if (!retrying_request_ && redirect_info.new_method == "GET" &&
      redirect_info.new_url == third_party_auth_config_.token_validation_url) {
    // A sequence of redirects caused the original POST request to become a GET
    // request for this URL. Cancel the request, and re-submit the POST request.
    // The chain of redirects are expected to set some cookies that will
    // ensure the new POST request succeeds.
    retrying_request_ = true;
    DCHECK(data_.empty());
    StartValidateRequest(token_);
  }
}

void TokenValidatorBase::OnCertificateRequested(
    net::URLRequest* source,
    net::SSLCertRequestInfo* cert_request_info) {
  DCHECK_EQ(request_.get(), source);

  net::ClientCertStore* client_cert_store;
#if defined(USE_NSS_CERTS)
  client_cert_store = new net::ClientCertStoreNSS(
      net::ClientCertStoreNSS::PasswordDelegateFactory());
#elif defined(OS_WIN)
  // The network process is running as "Local Service" whose "Current User"
  // cert store doesn't contain any certificates. Use the "Local Machine"
  // store instead.
  // The ACL on the private key of the machine certificate in the "Local
  // Machine" cert store needs to allow access by "Local Service".
  HCERTSTORE cert_store = ::CertOpenStore(
      CERT_STORE_PROV_SYSTEM, 0, NULL,
      CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_READONLY_FLAG, L"MY");
  client_cert_store = new net::ClientCertStoreWin(cert_store);
#elif defined(OS_MACOSX)
  client_cert_store = new net::ClientCertStoreMac();
#else
  // OpenSSL does not use the ClientCertStore infrastructure.
  client_cert_store = nullptr;
#endif
  // The callback is uncancellable, and GetClientCert requires
  // client_cert_store to stay alive until the callback is called. So we must
  // give it a WeakPtr for |this|, and ownership of the other parameters.
  client_cert_store->GetClientCerts(
      *cert_request_info,
      base::Bind(&TokenValidatorBase::OnCertificatesSelected,
                 weak_factory_.GetWeakPtr(), base::Owned(client_cert_store)));
}

void TokenValidatorBase::OnCertificatesSelected(
    net::ClientCertStore* unused,
    net::ClientCertIdentityList selected_certs) {
  const std::string& issuer =
      third_party_auth_config_.token_validation_cert_issuer;

  base::Time now = base::Time::Now();

  auto best_match_position = std::max_element(
      selected_certs.begin(), selected_certs.end(),
      [&issuer, now](const std::unique_ptr<net::ClientCertIdentity>& i1,
                     const std::unique_ptr<net::ClientCertIdentity>& i2) {
        return WorseThan(issuer, now, i1->certificate(), i2->certificate());
      });

  if (best_match_position == selected_certs.end() ||
      !IsCertificateValid(issuer, now, (*best_match_position)->certificate())) {
    ContinueWithCertificate(nullptr, nullptr);
  } else {
    scoped_refptr<net::X509Certificate> cert =
        (*best_match_position)->certificate();
    net::ClientCertIdentity::SelfOwningAcquirePrivateKey(
        std::move(*best_match_position),
        base::Bind(&TokenValidatorBase::ContinueWithCertificate,
                   weak_factory_.GetWeakPtr(), std::move(cert)));
  }
}

void TokenValidatorBase::ContinueWithCertificate(
    scoped_refptr<net::X509Certificate> client_cert,
    scoped_refptr<net::SSLPrivateKey> client_private_key) {
  if (request_) {
    if (client_cert) {
      HOST_LOG << "Using certificate issued by: '"
               << client_cert->issuer().common_name << "' with start date: '"
               << client_cert->valid_start() << "' and expiry date: '"
               << client_cert->valid_expiry() << "'";
    }

    request_->ContinueWithCertificate(std::move(client_cert),
                                      std::move(client_private_key));
  }
}

bool TokenValidatorBase::IsValidScope(const std::string& token_scope) {
  // TODO(rmsousa): Deal with reordering/subsets/supersets/aliases/etc.
  return token_scope == token_scope_;
}

std::string TokenValidatorBase::ProcessResponse(int net_result) {
  // Verify that we got a successful response.
  if (net_result != net::OK) {
    LOG(ERROR) << "Error validating token, err=" << net_result;
    return std::string();
  }

  int response = request_->GetResponseCode();
  if (response != 200) {
    LOG(ERROR) << "Error " << response << " validating token: '" << data_
               << "'";
    return std::string();
  }

  // Decode the JSON data from the response.
  std::unique_ptr<base::Value> value = base::JSONReader::Read(data_);
  base::DictionaryValue* dict;
  if (!value || !value->GetAsDictionary(&dict)) {
    LOG(ERROR) << "Invalid token validation response: '" << data_ << "'";
    return std::string();
  }

  std::string token_scope;
  dict->GetStringWithoutPathExpansion("scope", &token_scope);
  if (!IsValidScope(token_scope)) {
    LOG(ERROR) << "Invalid scope: '" << token_scope << "', expected: '"
               << token_scope_ << "'.";
    return std::string();
  }

  std::string shared_secret;
  // Everything is valid, so return the shared secret to the caller.
  dict->GetStringWithoutPathExpansion("access_token", &shared_secret);
  return shared_secret;
}

}  // namespace remoting
