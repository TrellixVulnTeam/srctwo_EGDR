// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/register_support_host_request.h"

#include <stdint.h>

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringize_macros.h"
#include "base/time/time.h"
#include "remoting/base/constants.h"
#include "remoting/host/host_config.h"
#include "remoting/host/host_details.h"
#include "remoting/protocol/errors.h"
#include "remoting/signaling/iq_sender.h"
#include "remoting/signaling/jid_util.h"
#include "remoting/signaling/signal_strategy.h"
#include "remoting/signaling/signaling_address.h"
#include "third_party/libjingle_xmpp/xmllite/xmlelement.h"
#include "third_party/libjingle_xmpp/xmpp/constants.h"

using buzz::QName;
using buzz::XmlElement;

namespace remoting {

using protocol::ErrorCode;

namespace {
// Strings used in the request message we send to the bot.
const char kRegisterQueryTag[] = "register-support-host";
const char kPublicKeyTag[] = "public-key";
const char kSignatureTag[] = "signature";
const char kSignatureTimeAttr[] = "time";
const char kHostVersionTag[] = "host-version";
const char kHostOsNameTag[] = "host-os-name";
const char kHostOsVersionTag[] = "host-os-version";

// Strings used to parse responses received from the bot.
const char kRegisterQueryResultTag[] = "register-support-host-result";
const char kSupportIdTag[] = "support-id";
const char kSupportIdLifetimeTag[] = "support-id-lifetime";

// The signaling timeout for register support host requests.
constexpr int kRegisterRequestTimeoutInSeconds = 10;
}

RegisterSupportHostRequest::RegisterSupportHostRequest(
    SignalStrategy* signal_strategy,
    scoped_refptr<RsaKeyPair> key_pair,
    const std::string& directory_bot_jid,
    const RegisterCallback& callback)
    : signal_strategy_(signal_strategy),
      key_pair_(key_pair),
      directory_bot_jid_(directory_bot_jid),
      callback_(callback) {
  DCHECK(signal_strategy_);
  DCHECK(key_pair_.get());
  signal_strategy_->AddListener(this);
  iq_sender_.reset(new IqSender(signal_strategy_));
}

RegisterSupportHostRequest::~RegisterSupportHostRequest() {
  if (signal_strategy_)
    signal_strategy_->RemoveListener(this);
}

void RegisterSupportHostRequest::OnSignalStrategyStateChange(
    SignalStrategy::State state) {
  if (state == SignalStrategy::CONNECTED) {
    DCHECK(!callback_.is_null());
    // The host_jid will be written to the SupportHostStore for lookup. Use id()
    // instead of jid() so that we can write the lcs address instead of the
    // remoting bot JID.
    std::string host_jid = signal_strategy_->GetLocalAddress().id();
    request_ = iq_sender_->SendIq(
        buzz::STR_SET, directory_bot_jid_, CreateRegistrationRequest(host_jid),
        base::Bind(&RegisterSupportHostRequest::ProcessResponse,
                   base::Unretained(this)));
    if (!request_) {
      LOG(ERROR) << "Error sending the register-support-host request.";
      CallCallback(std::string(), base::TimeDelta(),
                   ErrorCode::SIGNALING_ERROR);
      return;
    }

    request_->SetTimeout(
        base::TimeDelta::FromSeconds(kRegisterRequestTimeoutInSeconds));

  } else if (state == SignalStrategy::DISCONNECTED) {
    // We will reach here if signaling fails to connect.
    LOG(ERROR) << "Signal strategy disconnected.";
    CallCallback(std::string(), base::TimeDelta(), ErrorCode::SIGNALING_ERROR);
  }
}

bool RegisterSupportHostRequest::OnSignalStrategyIncomingStanza(
    const buzz::XmlElement* stanza) {
  return false;
}

std::unique_ptr<XmlElement>
RegisterSupportHostRequest::CreateRegistrationRequest(const std::string& jid) {
  auto query = base::MakeUnique<XmlElement>(
      QName(kChromotingXmlNamespace, kRegisterQueryTag));

  auto public_key = base::MakeUnique<XmlElement>(
      QName(kChromotingXmlNamespace, kPublicKeyTag));
  public_key->AddText(key_pair_->GetPublicKey());
  query->AddElement(public_key.release());

  query->AddElement(CreateSignature(jid).release());

  // Add host version.
  auto host_version = base::MakeUnique<XmlElement>(
      QName(kChromotingXmlNamespace, kHostVersionTag));
  host_version->AddText(STRINGIZE(VERSION));
  query->AddElement(host_version.release());

  // Add host os name.
  auto host_os_name = base::MakeUnique<XmlElement>(
      QName(kChromotingXmlNamespace, kHostOsNameTag));
  host_os_name->AddText(GetHostOperatingSystemName());
  query->AddElement(host_os_name.release());

  // Add host os version.
  auto host_os_version = base::MakeUnique<XmlElement>(
      QName(kChromotingXmlNamespace, kHostOsVersionTag));
  host_os_version->AddText(GetHostOperatingSystemVersion());
  query->AddElement(host_os_version.release());

  return query;
}

std::unique_ptr<XmlElement> RegisterSupportHostRequest::CreateSignature(
    const std::string& jid) {
  std::unique_ptr<XmlElement> signature_tag(
      new XmlElement(QName(kChromotingXmlNamespace, kSignatureTag)));

  int64_t time = static_cast<int64_t>(base::Time::Now().ToDoubleT());
  std::string time_str(base::Int64ToString(time));
  signature_tag->AddAttr(
      QName(kChromotingXmlNamespace, kSignatureTimeAttr), time_str);

  std::string message = NormalizeJid(jid) + ' ' + time_str;
  std::string signature(key_pair_->SignMessage(message));
  signature_tag->AddText(signature);

  return signature_tag;
}

void RegisterSupportHostRequest::ParseResponse(const XmlElement* response,
                                               std::string* support_id,
                                               base::TimeDelta* lifetime,
                                               ErrorCode* error_code) {
  if (!response) {
    LOG(ERROR) << "register-support-host request timed out.";
    *error_code = ErrorCode::SIGNALING_TIMEOUT;
    return;
  }

  std::string type = response->Attr(buzz::QN_TYPE);
  if (type == buzz::STR_ERROR) {
    LOG(ERROR) << "Received error in response to heartbeat: "
               << response->Str();
    *error_code = ErrorCode::HOST_REGISTRATION_ERROR;
    return;
  }

  // This method must only be called for error or result stanzas.
  if (type != buzz::STR_RESULT) {
    LOG(ERROR) << "Received unexpect stanza of type \"" << type << "\"";
    *error_code = ErrorCode::HOST_REGISTRATION_ERROR;
    return;
  }

  const XmlElement* result_element = response->FirstNamed(QName(
      kChromotingXmlNamespace, kRegisterQueryResultTag));
  if (!result_element) {
    LOG(ERROR) << "<" << kRegisterQueryResultTag
               << "> is missing in the host registration response: "
               << response->Str();
    *error_code = ErrorCode::HOST_REGISTRATION_ERROR;
    return;
  }

  const XmlElement* support_id_element =
      result_element->FirstNamed(QName(kChromotingXmlNamespace, kSupportIdTag));
  if (!support_id_element) {
    LOG(ERROR) << "<" << kSupportIdTag
               << "> is missing in the host registration response: "
               << response->Str();
    *error_code = ErrorCode::HOST_REGISTRATION_ERROR;
    return;
  }

  const XmlElement* lifetime_element =
      result_element->FirstNamed(QName(kChromotingXmlNamespace,
                                       kSupportIdLifetimeTag));
  if (!lifetime_element) {
    LOG(ERROR) << "<" << kSupportIdLifetimeTag
               << "> is missing in the host registration response: "
               << response->Str();
    *error_code = ErrorCode::HOST_REGISTRATION_ERROR;
    return;
  }

  int lifetime_int;
  if (!base::StringToInt(lifetime_element->BodyText(), &lifetime_int) ||
      lifetime_int <= 0) {
    LOG(ERROR) << "<" << kSupportIdLifetimeTag
               << "> is malformed in the host registration response: "
               << response->Str();
    *error_code = ErrorCode::HOST_REGISTRATION_ERROR;
    return;
  }

  *support_id = support_id_element->BodyText();
  *lifetime = base::TimeDelta::FromSeconds(lifetime_int);
  return;
}

void RegisterSupportHostRequest::ProcessResponse(IqRequest* request,
                                                 const XmlElement* response) {
  std::string support_id;
  base::TimeDelta lifetime;
  ErrorCode error_code = ErrorCode::OK;
  ParseResponse(response, &support_id, &lifetime, &error_code);
  CallCallback(support_id, lifetime, error_code);
}

void RegisterSupportHostRequest::CallCallback(const std::string& support_id,
                                              base::TimeDelta lifetime,
                                              ErrorCode error_code) {
  // Cleanup state before calling the callback.
  request_.reset();
  iq_sender_.reset();
  signal_strategy_->RemoveListener(this);
  signal_strategy_ = nullptr;

  base::ResetAndReturn(&callback_).Run(support_id, lifetime, error_code);
}

}  // namespace remoting
