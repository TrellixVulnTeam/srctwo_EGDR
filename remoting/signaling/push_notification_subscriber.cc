// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/signaling/push_notification_subscriber.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/memory/ptr_util.h"
#include "remoting/base/logging.h"
#include "remoting/signaling/iq_sender.h"
#include "remoting/signaling/jid_util.h"
#include "remoting/signaling/signaling_address.h"
#include "third_party/libjingle_xmpp/xmllite/xmlelement.h"

namespace remoting {

namespace {

const char kGooglePushNamespace[] = "google:push";

}  // namespace

PushNotificationSubscriber::Subscription::Subscription() {}
PushNotificationSubscriber::Subscription::~Subscription() {}

PushNotificationSubscriber::PushNotificationSubscriber(
    SignalStrategy* signal_strategy,
    const SubscriptionList& subscriptions)
    : signal_strategy_(signal_strategy), subscriptions_(subscriptions) {
  signal_strategy_->AddListener(this);
}

PushNotificationSubscriber::~PushNotificationSubscriber() {
  signal_strategy_->RemoveListener(this);
}

void PushNotificationSubscriber::OnSignalStrategyStateChange(
    SignalStrategy::State state) {
  if (state == SignalStrategy::CONNECTED) {
    for (const Subscription& subscription : subscriptions_) {
      Subscribe(subscription);
    }
    subscriptions_.clear();  // no longer needed
  }
}

bool PushNotificationSubscriber::OnSignalStrategyIncomingStanza(
    const buzz::XmlElement* stanza) {
  // Ignore all XMPP stanzas.
  return false;
}

void PushNotificationSubscriber::Subscribe(const Subscription& subscription) {
  VLOG(0) << "Subscribing to push notifications on channel: "
          << subscription.channel << ".";

  std::string bare_jid;
  SplitJidResource(signal_strategy_->GetLocalAddress().jid(), &bare_jid,
                   nullptr);

  // Build a subscription request.
  buzz::XmlElement* subscribe_element =
      new buzz::XmlElement(buzz::QName(kGooglePushNamespace, "subscribe"));
  buzz::XmlElement* item_element =
      new buzz::XmlElement(buzz::QName(kGooglePushNamespace, "item"));
  subscribe_element->AddElement(item_element);
  item_element->SetAttr(buzz::QName(std::string(), "channel"),
                        subscription.channel);
  item_element->SetAttr(buzz::QName(std::string(), "from"), subscription.from);

  // Send the request.
  iq_sender_.reset(new IqSender(signal_strategy_));
  iq_request_ = iq_sender_->SendIq(
      "set", bare_jid, base::WrapUnique(subscribe_element),
      base::Bind(&PushNotificationSubscriber::OnSubscriptionResult,
                 base::Unretained(this)));
}

void PushNotificationSubscriber::OnSubscriptionResult(
    IqRequest* request,
    const buzz::XmlElement* response) {
  std::string response_type =
      response->Attr(buzz::QName(std::string(), "type"));
  if (response_type != "result") {
    LOG(ERROR) << "Invalid response type for subscription: " << response_type;
  }

  // The IqSender and IqRequest are no longer needed after receiving a
  // reply to the subscription request.
  iq_request_.reset();
  iq_sender_.reset();
}

}  // namespace remoting
