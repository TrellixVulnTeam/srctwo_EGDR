// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/signaling/delegating_signal_strategy.h"

#include "base/bind.h"
#include "base/rand_util.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/thread_task_runner_handle.h"
#include "third_party/libjingle_xmpp/xmllite/xmlelement.h"
#include "third_party/libjingle_xmpp/xmpp/constants.h"

namespace remoting {

DelegatingSignalStrategy::DelegatingSignalStrategy(
    const SignalingAddress& local_address,
    scoped_refptr<base::SingleThreadTaskRunner> client_task_runner,
    const IqCallback& send_iq_callback)
    : local_address_(local_address),
      delegate_task_runner_(base::ThreadTaskRunnerHandle::Get()),
      client_task_runner_(client_task_runner),
      send_iq_callback_(send_iq_callback),
      weak_factory_(this) {
  incoming_iq_callback_ =
      base::BindRepeating(&OnIncomingMessageFromDelegate,
                          weak_factory_.GetWeakPtr(), client_task_runner_);
}

DelegatingSignalStrategy::~DelegatingSignalStrategy() {}

DelegatingSignalStrategy::IqCallback
DelegatingSignalStrategy::GetIncomingMessageCallback() {
  return incoming_iq_callback_;
}

// static
void DelegatingSignalStrategy::OnIncomingMessageFromDelegate(
    base::WeakPtr<DelegatingSignalStrategy> weak_ptr,
    scoped_refptr<base::SingleThreadTaskRunner> client_task_runner,
    const std::string& message) {
  if (client_task_runner->BelongsToCurrentThread()) {
    weak_ptr->OnIncomingMessage(message);
    return;
  }

  client_task_runner->PostTask(
      FROM_HERE, base::Bind(&DelegatingSignalStrategy::OnIncomingMessage,
                            weak_ptr, message));
}

void DelegatingSignalStrategy::OnIncomingMessage(const std::string& message) {
  DCHECK(client_task_runner_->BelongsToCurrentThread());
  std::unique_ptr<buzz::XmlElement> stanza(buzz::XmlElement::ForStr(message));
  if (!stanza.get()) {
    LOG(WARNING) << "Malformed XMPP stanza received: " << message;
    return;
  }

  for (auto& listener : listeners_) {
    if (listener.OnSignalStrategyIncomingStanza(stanza.get()))
      break;
  }
}

void DelegatingSignalStrategy::Connect() {
  DCHECK(client_task_runner_->BelongsToCurrentThread());
  for (auto& observer : listeners_)
    observer.OnSignalStrategyStateChange(CONNECTED);
}

void DelegatingSignalStrategy::Disconnect() {
  DCHECK(client_task_runner_->BelongsToCurrentThread());
}

SignalStrategy::State DelegatingSignalStrategy::GetState() const {
  DCHECK(client_task_runner_->BelongsToCurrentThread());
  return CONNECTED;
}

SignalStrategy::Error DelegatingSignalStrategy::GetError() const {
  DCHECK(client_task_runner_->BelongsToCurrentThread());
  return OK;
}

const SignalingAddress& DelegatingSignalStrategy::GetLocalAddress() const {
  DCHECK(client_task_runner_->BelongsToCurrentThread());
  return local_address_;
}

void DelegatingSignalStrategy::AddListener(Listener* listener) {
  DCHECK(client_task_runner_->BelongsToCurrentThread());
  listeners_.AddObserver(listener);
}

void DelegatingSignalStrategy::RemoveListener(Listener* listener) {
  DCHECK(client_task_runner_->BelongsToCurrentThread());
  listeners_.RemoveObserver(listener);
}

bool DelegatingSignalStrategy::SendStanza(
    std::unique_ptr<buzz::XmlElement> stanza) {
  DCHECK(client_task_runner_->BelongsToCurrentThread());
  GetLocalAddress().SetInMessage(stanza.get(), SignalingAddress::FROM);
  delegate_task_runner_->PostTask(FROM_HERE,
                                  base::Bind(send_iq_callback_, stanza->Str()));
  return true;
}

std::string DelegatingSignalStrategy::GetNextId() {
  return base::Uint64ToString(base::RandUint64());
}

}  // namespace remoting
