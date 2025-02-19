// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/web/public/cert_policy.h"

#include "base/logging.h"
#include "net/cert/x509_certificate.h"

namespace web {

CertPolicy::CertPolicy() {
}

CertPolicy::~CertPolicy() {
}

// We consider a given |cert| to be a match to a saved allowed cert if the
// |error| is an exact match to or subset of the errors in the saved CertStatus.
CertPolicy::Judgment CertPolicy::Check(net::X509Certificate* cert,
                                       net::CertStatus error) const {
  auto allowed_iter =
      allowed_.find(net::X509Certificate::CalculateChainFingerprint256(
          cert->os_cert_handle(), cert->GetIntermediateCertificates()));
  if ((allowed_iter != allowed_.end()) && (allowed_iter->second & error) &&
      !(~(allowed_iter->second & error) ^ ~error)) {
    return ALLOWED;
  }
  return UNKNOWN;  // We don't have a policy for this cert.
}

void CertPolicy::Allow(net::X509Certificate* cert, net::CertStatus error) {
  // If this same cert had already been saved with a different error status,
  // this will replace it with the new error status.
  allowed_[net::X509Certificate::CalculateChainFingerprint256(
      cert->os_cert_handle(), cert->GetIntermediateCertificates())] = error;
}

}  // namespace web
