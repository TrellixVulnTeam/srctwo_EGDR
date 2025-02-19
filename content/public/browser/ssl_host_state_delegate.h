// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_SSL_HOST_STATE_DELEGATE_H_
#define CONTENT_PUBLIC_BROWSER_SSL_HOST_STATE_DELEGATE_H_

#include <memory>

#include "base/callback_forward.h"
#include "base/memory/ref_counted.h"
#include "content/common/content_export.h"
#include "net/cert/x509_certificate.h"

namespace content {

// The SSLHostStateDelegate encapulates the host-specific state for SSL errors.
// For example, SSLHostStateDelegate remembers whether the user has whitelisted
// a particular broken cert for use with particular host.  We separate this
// state from the SSLManager because this state is shared across many navigation
// controllers.
//
// SSLHostStateDelegate may be implemented by the embedder to provide a storage
// strategy for certificate decisions or it may be left unimplemented to use a
// default strategy of not remembering decisions at all.
class SSLHostStateDelegate {
 public:
  // The judgements that can be reached by a user for invalid certificates.
  enum CertJudgment {
    DENIED,
    ALLOWED
  };

  // The types of nonsecure subresources that this class keeps track of.
  enum InsecureContentType {
    // A  MIXED subresource was loaded over HTTP on an HTTPS page.
    MIXED_CONTENT,
    // A CERT_ERRORS subresource was loaded over HTTPS with certificate
    // errors on an HTTPS page.
    CERT_ERRORS_CONTENT,
  };

  // Records that |cert| is permitted to be used for |host| in the future, for
  // a specified |error| type.
  virtual void AllowCert(const std::string&,
                         const net::X509Certificate& cert,
                         net::CertStatus error) = 0;

  // Clear allow preferences matched by |host_filter|. If the filter is null,
  // clear all preferences.
  virtual void Clear(
      const base::Callback<bool(const std::string&)>& host_filter) = 0;

  // Queries whether |cert| is allowed for |host| and |error|. Returns true in
  // |expired_previous_decision| if a previous user decision expired immediately
  // prior to this query, otherwise false.
  virtual CertJudgment QueryPolicy(const std::string& host,
                                   const net::X509Certificate& cert,
                                   net::CertStatus error,
                                   bool* expired_previous_decision) = 0;

  // Records that a host has run insecure content of the given |content_type|.
  virtual void HostRanInsecureContent(const std::string& host,
                                      int child_id,
                                      InsecureContentType content_type) = 0;

  // Returns whether the specified host ran insecure content of the given
  // |content_type|.
  virtual bool DidHostRunInsecureContent(
      const std::string& host,
      int child_id,
      InsecureContentType content_type) const = 0;

  // Revokes all SSL certificate error allow exceptions made by the user for
  // |host|.
  virtual void RevokeUserAllowExceptions(const std::string& host) = 0;

  // Returns whether the user has allowed a certificate error exception for
  // |host|. This does not mean that *all* certificate errors are allowed, just
  // that there exists an exception. To see if a particular certificate and
  // error combination exception is allowed, use QueryPolicy().
  virtual bool HasAllowException(const std::string& host) const = 0;

 protected:
  virtual ~SSLHostStateDelegate() {}
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_SSL_HOST_STATE_DELEGATE_H_
