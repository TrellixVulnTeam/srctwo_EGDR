// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/cert/internal/cert_errors.h"

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string_split.h"
#include "base/strings/stringprintf.h"
#include "net/cert/internal/cert_error_params.h"
#include "net/cert/internal/parse_name.h"
#include "net/cert/internal/parsed_certificate.h"

namespace net {

namespace {

void AppendLinesWithIndentation(const std::string& text,
                                const std::string& indentation,
                                std::string* out) {
  std::vector<base::StringPiece> lines = base::SplitStringPieceUsingSubstr(
      text, "\n", base::KEEP_WHITESPACE, base::SPLIT_WANT_ALL);

  for (const auto& line : lines) {
    *out += indentation;
    line.AppendToString(out);
    *out += "\n";
  }
}

}  // namespace

CertError::CertError() = default;

CertError::CertError(Severity severity,
                     CertErrorId id,
                     std::unique_ptr<CertErrorParams> params)
    : severity(severity), id(id), params(std::move(params)) {}

CertError::CertError(CertError&& other) = default;

CertError& CertError::operator=(CertError&&) = default;

CertError::~CertError() = default;

std::string CertError::ToDebugString() const {
  std::string result;
  switch (severity) {
    case SEVERITY_WARNING:
      result += "WARNING: ";
      break;
    case SEVERITY_HIGH:
      result += "ERROR: ";
      break;
  }
  result += CertErrorIdToDebugString(id);
  result += +"\n";

  if (params)
    AppendLinesWithIndentation(params->ToDebugString(), "  ", &result);

  return result;
}

CertErrors::CertErrors() = default;
CertErrors::CertErrors(CertErrors&& other) = default;
CertErrors& CertErrors::operator=(CertErrors&&) = default;
CertErrors::~CertErrors() = default;

void CertErrors::Add(CertError::Severity severity,
                     CertErrorId id,
                     std::unique_ptr<CertErrorParams> params) {
  nodes_.push_back(CertError(severity, id, std::move(params)));
}

void CertErrors::AddError(CertErrorId id,
                          std::unique_ptr<CertErrorParams> params) {
  Add(CertError::SEVERITY_HIGH, id, std::move(params));
}

void CertErrors::AddError(CertErrorId id) {
  AddError(id, nullptr);
}

void CertErrors::AddWarning(CertErrorId id,
                            std::unique_ptr<CertErrorParams> params) {
  Add(CertError::SEVERITY_WARNING, id, std::move(params));
}

void CertErrors::AddWarning(CertErrorId id) {
  AddWarning(id, nullptr);
}

std::string CertErrors::ToDebugString() const {
  std::string result;
  for (const CertError& node : nodes_)
    result += node.ToDebugString();

  return result;
}

bool CertErrors::ContainsError(CertErrorId id) const {
  for (const CertError& node : nodes_) {
    if (node.id == id)
      return true;
  }
  return false;
}

bool CertErrors::ContainsAnyErrorWithSeverity(
    CertError::Severity severity) const {
  for (const CertError& node : nodes_) {
    if (node.severity == severity)
      return true;
  }
  return false;
}

CertPathErrors::CertPathErrors() = default;

CertPathErrors::CertPathErrors(CertPathErrors&& other) = default;
CertPathErrors& CertPathErrors::operator=(CertPathErrors&&) = default;

CertPathErrors::~CertPathErrors() = default;

CertErrors* CertPathErrors::GetErrorsForCert(size_t cert_index) {
  if (cert_index >= cert_errors_.size())
    cert_errors_.resize(cert_index + 1);
  return &cert_errors_[cert_index];
}

const CertErrors* CertPathErrors::GetErrorsForCert(size_t cert_index) const {
  if (cert_index >= cert_errors_.size())
    return nullptr;
  return &cert_errors_[cert_index];
}

CertErrors* CertPathErrors::GetOtherErrors() {
  return &other_errors_;
}

bool CertPathErrors::ContainsError(CertErrorId id) const {
  for (const CertErrors& errors : cert_errors_) {
    if (errors.ContainsError(id))
      return true;
  }

  if (other_errors_.ContainsError(id))
    return true;

  return false;
}

bool CertPathErrors::ContainsAnyErrorWithSeverity(
    CertError::Severity severity) const {
  for (const CertErrors& errors : cert_errors_) {
    if (errors.ContainsAnyErrorWithSeverity(severity))
      return true;
  }

  if (other_errors_.ContainsAnyErrorWithSeverity(severity))
    return true;

  return false;
}

std::string CertPathErrors::ToDebugString(
    const ParsedCertificateList& certs) const {
  std::string result;

  for (size_t i = 0; i < cert_errors_.size(); ++i) {
    // Pretty print the current CertErrors. If there were no errors/warnings,
    // then continue.
    const CertErrors& errors = cert_errors_[i];
    std::string cert_errors_string = errors.ToDebugString();
    if (cert_errors_string.empty())
      continue;

    // Add a header that identifies which certificate this CertErrors pertains
    // to.
    std::string cert_name_debug_str;
    if (i < certs.size() && certs[i]) {
      RDNSequence subject;
      if (ParseName(certs[i]->tbs().subject_tlv, &subject) &&
          ConvertToRFC2253(subject, &cert_name_debug_str)) {
        cert_name_debug_str = " (" + cert_name_debug_str + ")";
      }
    }

    result +=
        base::StringPrintf("----- Certificate i=%d%s -----\n",
                           static_cast<int>(i), cert_name_debug_str.c_str());

    result += cert_errors_string;
    result += "\n";
  }

  // Print any other errors that aren't associated with a particular certificate
  // in the chain.
  std::string other_errors = other_errors_.ToDebugString();
  if (!other_errors.empty()) {
    result += "----- Other errors (not certificate specific) -----\n";
    result += other_errors;
    result += "\n";
  }

  return result;
}

}  // namespace net
