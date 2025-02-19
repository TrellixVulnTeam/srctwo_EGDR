// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/gpu/frame_swap_message_queue.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>

#include "base/containers/hash_tables.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/stl_util.h"
#include "ipc/ipc_message.h"

using std::vector;

namespace content {

class FrameSwapMessageSubQueue {
 public:
  FrameSwapMessageSubQueue() {}
  virtual ~FrameSwapMessageSubQueue() {}
  virtual bool Empty() const = 0;
  virtual void QueueMessage(int source_frame_number,
                            std::unique_ptr<IPC::Message> msg,
                            bool* is_first) = 0;
  virtual void DrainMessages(
      int source_frame_number,
      std::vector<std::unique_ptr<IPC::Message>>* messages) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(FrameSwapMessageSubQueue);
};

namespace {

// Queue specific to MESSAGE_DELIVERY_POLICY_WITH_VISUAL_STATE.
class SendMessageScopeImpl : public FrameSwapMessageQueue::SendMessageScope {
 public:
  SendMessageScopeImpl(base::Lock* lock) : auto_lock_(*lock) {}
  ~SendMessageScopeImpl() override {}

 private:
  base::AutoLock auto_lock_;
};

class VisualStateQueue : public FrameSwapMessageSubQueue {
 public:
  VisualStateQueue() = default;

  ~VisualStateQueue() override = default;

  bool Empty() const override { return queue_.empty(); }

  void QueueMessage(int source_frame_number,
                    std::unique_ptr<IPC::Message> msg,
                    bool* is_first) override {
    if (is_first)
      *is_first = (queue_.count(source_frame_number) == 0);

    queue_[source_frame_number].push_back(std::move(msg));
  }

  void DrainMessages(
      int source_frame_number,
      std::vector<std::unique_ptr<IPC::Message>>* messages) override {
    auto end = queue_.upper_bound(source_frame_number);
    for (auto i = queue_.begin(); i != end; i++) {
      DCHECK(i->first <= source_frame_number);
      std::move(i->second.begin(), i->second.end(),
                std::back_inserter(*messages));
    }
    queue_.erase(queue_.begin(), end);
  }

 private:
  std::map<int, std::vector<std::unique_ptr<IPC::Message>>> queue_;

  DISALLOW_COPY_AND_ASSIGN(VisualStateQueue);
};

// Queue specific to MESSAGE_DELIVERY_POLICY_WITH_NEXT_SWAP.
class SwapQueue : public FrameSwapMessageSubQueue {
 public:
  SwapQueue() {}
  bool Empty() const override { return queue_.empty(); }

  void QueueMessage(int source_frame_number,
                    std::unique_ptr<IPC::Message> msg,
                    bool* is_first) override {
    if (is_first)
      *is_first = Empty();
    queue_.push_back(std::move(msg));
  }

  void DrainMessages(
      int source_frame_number,
      std::vector<std::unique_ptr<IPC::Message>>* messages) override {
    std::move(queue_.begin(), queue_.end(), std::back_inserter(*messages));
    queue_.clear();
  }

 private:
  std::vector<std::unique_ptr<IPC::Message>> queue_;

  DISALLOW_COPY_AND_ASSIGN(SwapQueue);
};

}  // namespace

FrameSwapMessageQueue::FrameSwapMessageQueue(int32_t routing_id)
    : visual_state_queue_(new VisualStateQueue()),
      swap_queue_(new SwapQueue()),
      routing_id_(routing_id) {
  DETACH_FROM_THREAD(impl_thread_checker_);
}

FrameSwapMessageQueue::~FrameSwapMessageQueue() {
}

bool FrameSwapMessageQueue::Empty() const {
  base::AutoLock lock(lock_);
  return next_drain_messages_.empty() && visual_state_queue_->Empty() &&
         swap_queue_->Empty();
}

FrameSwapMessageSubQueue* FrameSwapMessageQueue::GetSubQueue(
    MessageDeliveryPolicy policy) {
  switch (policy) {
    case MESSAGE_DELIVERY_POLICY_WITH_NEXT_SWAP:
      return swap_queue_.get();
      break;
    case MESSAGE_DELIVERY_POLICY_WITH_VISUAL_STATE:
      return visual_state_queue_.get();
      break;
  }
  NOTREACHED();
  return NULL;
}

void FrameSwapMessageQueue::QueueMessageForFrame(
    MessageDeliveryPolicy policy,
    int source_frame_number,
    std::unique_ptr<IPC::Message> msg,
    bool* is_first) {
  base::AutoLock lock(lock_);
  GetSubQueue(policy)
      ->QueueMessage(source_frame_number, std::move(msg), is_first);
}

void FrameSwapMessageQueue::DidActivate(int source_frame_number) {
  base::AutoLock lock(lock_);
  visual_state_queue_->DrainMessages(source_frame_number,
                                     &next_drain_messages_);
}

void FrameSwapMessageQueue::DidSwap(int source_frame_number) {
  base::AutoLock lock(lock_);
  swap_queue_->DrainMessages(0, &next_drain_messages_);
}

void FrameSwapMessageQueue::DidNotSwap(
    int source_frame_number,
    cc::SwapPromise::DidNotSwapReason reason,
    std::vector<std::unique_ptr<IPC::Message>>* messages) {
  base::AutoLock lock(lock_);
  switch (reason) {
    case cc::SwapPromise::SWAP_FAILS:
    case cc::SwapPromise::COMMIT_NO_UPDATE:
      DrainMessages(messages);
      swap_queue_->DrainMessages(source_frame_number, messages);
      visual_state_queue_->DrainMessages(source_frame_number, messages);
      break;
    case cc::SwapPromise::COMMIT_FAILS:
    case cc::SwapPromise::ACTIVATION_FAILS:
      // Do not queue any responses here. If ACTIVATION_FAILS or
      // COMMIT_FAILS the renderer is shutting down, which will result
      // in the RenderFrameHostImpl destructor firing the remaining
      // response callbacks itself.
      break;
  }
}

void FrameSwapMessageQueue::DrainMessages(
    std::vector<std::unique_ptr<IPC::Message>>* messages) {
  lock_.AssertAcquired();
  std::move(next_drain_messages_.begin(), next_drain_messages_.end(),
            std::back_inserter(*messages));
  next_drain_messages_.clear();
}

std::unique_ptr<FrameSwapMessageQueue::SendMessageScope>
FrameSwapMessageQueue::AcquireSendMessageScope() {
  return base::MakeUnique<SendMessageScopeImpl>(&lock_);
}

// static
void FrameSwapMessageQueue::TransferMessages(
    std::vector<std::unique_ptr<IPC::Message>>* source,
    vector<IPC::Message>* dest) {
  for (const auto& msg : *source) {
    dest->push_back(*msg.get());
  }
  source->clear();
}

uint32_t FrameSwapMessageQueue::AllocateFrameToken() {
  return ++last_used_frame_token_;
}

void FrameSwapMessageQueue::NotifyFramesAreDiscarded(
    bool frames_are_discarded) {
  DCHECK_CALLED_ON_VALID_THREAD(impl_thread_checker_);
  frames_are_discarded_ = frames_are_discarded;
}

bool FrameSwapMessageQueue::AreFramesDiscarded() {
  DCHECK_CALLED_ON_VALID_THREAD(impl_thread_checker_);
  return frames_are_discarded_;
}

}  // namespace content
