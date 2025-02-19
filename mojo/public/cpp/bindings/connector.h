// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_BINDINGS_CONNECTOR_H_
#define MOJO_PUBLIC_CPP_BINDINGS_CONNECTOR_H_

#include <memory>
#include <utility>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/optional.h"
#include "base/sequence_checker.h"
#include "base/sequenced_task_runner.h"
#include "mojo/public/cpp/bindings/bindings_export.h"
#include "mojo/public/cpp/bindings/message.h"
#include "mojo/public/cpp/bindings/sync_handle_watcher.h"
#include "mojo/public/cpp/system/core.h"
#include "mojo/public/cpp/system/handle_signal_tracker.h"
#include "mojo/public/cpp/system/simple_watcher.h"

namespace base {
class Lock;
}

namespace mojo {

// The Connector class is responsible for performing read/write operations on a
// MessagePipe. It writes messages it receives through the MessageReceiver
// interface that it subclasses, and it forwards messages it reads through the
// MessageReceiver interface assigned as its incoming receiver.
//
// NOTE:
//   - MessagePipe I/O is non-blocking.
//   - Sending messages can be configured to be thread safe (please see comments
//     of the constructor). Other than that, the object should only be accessed
//     on the creating sequence.
class MOJO_CPP_BINDINGS_EXPORT Connector : public MessageReceiver {
 public:
  enum ConnectorConfig {
    // Connector::Accept() is only called from a single sequence.
    SINGLE_THREADED_SEND,
    // Connector::Accept() is allowed to be called from multiple sequences.
    MULTI_THREADED_SEND
  };

  // Determines how this Connector should behave with respect to serialization
  // of outgoing messages.
  enum class OutgoingSerializationMode {
    // Lazy serialization. The Connector prefers to transmit serialized messages
    // only when it knows its peer endpoint is remote. This ensures outgoing
    // requests are unserialized by default (when possible, i.e. when generated
    // bindings support it) and serialized only if and when necessary.
    kLazy,

    // Eager serialization. The Connector always prefers serialized messages,
    // ensuring that interface calls will be serialized immediately before
    // sending on the Connector.
    kEager,
  };

  // Determines how this Connector should behave with respect to serialization
  // of incoming messages.
  enum class IncomingSerializationMode {
    // Accepts and dispatches either serialized or unserialized messages. This
    // is the only mode that should be used in production.
    kDispatchAsIs,

    // Accepts either serialized or unserialized messages, but always forces
    // serialization (if applicable) before dispatch. Should be used only in
    // test environments to coerce the lazy serialization of a message after
    // transmission.
    kSerializeBeforeDispatchForTesting,
  };

  // The Connector takes ownership of |message_pipe|.
  Connector(ScopedMessagePipeHandle message_pipe,
            ConnectorConfig config,
            scoped_refptr<base::SequencedTaskRunner> runner);
  ~Connector() override;

  // Sets outgoing serialization mode.
  void SetOutgoingSerializationMode(OutgoingSerializationMode mode);
  void SetIncomingSerializationMode(IncomingSerializationMode mode);

  // Sets the receiver to handle messages read from the message pipe.  The
  // Connector will read messages from the pipe regardless of whether or not an
  // incoming receiver has been set.
  void set_incoming_receiver(MessageReceiver* receiver) {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    incoming_receiver_ = receiver;
  }

  // Errors from incoming receivers will force the connector into an error
  // state, where no more messages will be processed. This method is used
  // during testing to prevent that from happening.
  void set_enforce_errors_from_incoming_receiver(bool enforce) {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    enforce_errors_from_incoming_receiver_ = enforce;
  }

  // Sets the error handler to receive notifications when an error is
  // encountered while reading from the pipe or waiting to read from the pipe.
  void set_connection_error_handler(base::OnceClosure error_handler) {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    connection_error_handler_ = std::move(error_handler);
  }

  // Returns true if an error was encountered while reading from the pipe or
  // waiting to read from the pipe.
  bool encountered_error() const {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    return error_;
  }

  // Closes the pipe. The connector is put into a quiescent state.
  //
  // Please note that this method shouldn't be called unless it results from an
  // explicit request of the user of bindings (e.g., the user sets an
  // InterfacePtr to null or closes a Binding).
  void CloseMessagePipe();

  // Releases the pipe. Connector is put into a quiescent state.
  ScopedMessagePipeHandle PassMessagePipe();

  // Enters the error state. The upper layer may do this for unrecoverable
  // issues such as invalid messages are received. If a connection error handler
  // has been set, it will be called asynchronously.
  //
  // It is a no-op if the connector is already in the error state or there isn't
  // a bound message pipe. Otherwise, it closes the message pipe, which notifies
  // the other end and also prevents potential danger (say, the caller raises
  // an error because it believes the other end is malicious). In order to
  // appear to the user that the connector still binds to a message pipe, it
  // creates a new message pipe, closes one end and binds to the other.
  void RaiseError();

  // Is the connector bound to a MessagePipe handle?
  bool is_valid() const {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    return message_pipe_.is_valid();
  }

  // Waits for the next message on the pipe, blocking until one arrives,
  // |deadline| elapses, or an error happens. Returns |true| if a message has
  // been delivered, |false| otherwise.
  bool WaitForIncomingMessage(MojoDeadline deadline);

  // See Binding for details of pause/resume.
  void PauseIncomingMethodCallProcessing();
  void ResumeIncomingMethodCallProcessing();

  // MessageReceiver implementation:
  bool PrefersSerializedMessages() override;
  bool Accept(Message* message) override;

  MessagePipeHandle handle() const {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    return message_pipe_.get();
  }

  // Allows |message_pipe_| to be watched while others perform sync handle
  // watching on the same sequence. Please see comments of
  // SyncHandleWatcher::AllowWokenUpBySyncWatchOnSameThread().
  void AllowWokenUpBySyncWatchOnSameThread();

  // Watches |message_pipe_| (as well as other handles registered to be watched
  // together) synchronously.
  // This method:
  //   - returns true when |should_stop| is set to true;
  //   - return false when any error occurs, including |message_pipe_| being
  //     closed.
  bool SyncWatch(const bool* should_stop);

  // Whether currently the control flow is inside the sync handle watcher
  // callback.
  // It always returns false after CloseMessagePipe()/PassMessagePipe().
  bool during_sync_handle_watcher_callback() const {
    return sync_handle_watcher_callback_count_ > 0;
  }

  base::SequencedTaskRunner* task_runner() const { return task_runner_.get(); }

  // Sets the tag used by the heap profiler.
  // |tag| must be a const string literal.
  void SetWatcherHeapProfilerTag(const char* tag);

  // Allows testing environments to override the default serialization behavior
  // of newly constructed Connector instances. Must be called before any
  // Connector instances are constructed.
  static void OverrideDefaultSerializationBehaviorForTesting(
      OutgoingSerializationMode outgoing_mode,
      IncomingSerializationMode incoming_mode);

 private:
  class ActiveDispatchTracker;
  class RunLoopNestingObserver;

  // Callback of mojo::SimpleWatcher.
  void OnWatcherHandleReady(MojoResult result);
  // Callback of SyncHandleWatcher.
  void OnSyncHandleWatcherHandleReady(MojoResult result);
  void OnHandleReadyInternal(MojoResult result);

  void WaitToReadMore();

  // Returns false if it is impossible to receive more messages in the future.
  // |this| may have been destroyed in that case.
  WARN_UNUSED_RESULT bool ReadSingleMessage(MojoResult* read_result);

  // |this| can be destroyed during message dispatch.
  void ReadAllAvailableMessages();

  // If |force_pipe_reset| is true, this method replaces the existing
  // |message_pipe_| with a dummy message pipe handle (whose peer is closed).
  // If |force_async_handler| is true, |connection_error_handler_| is called
  // asynchronously.
  void HandleError(bool force_pipe_reset, bool force_async_handler);

  // Cancels any calls made to |waiter_|.
  void CancelWait();

  void EnsureSyncWatcherExists();

  base::OnceClosure connection_error_handler_;

  ScopedMessagePipeHandle message_pipe_;
  MessageReceiver* incoming_receiver_ = nullptr;

  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  std::unique_ptr<SimpleWatcher> handle_watcher_;
  base::Optional<HandleSignalTracker> peer_remoteness_tracker_;

  bool error_ = false;
  bool drop_writes_ = false;
  bool enforce_errors_from_incoming_receiver_ = true;

  bool paused_ = false;

  OutgoingSerializationMode outgoing_serialization_mode_;
  IncomingSerializationMode incoming_serialization_mode_;

  // If sending messages is allowed from multiple sequences, |lock_| is used to
  // protect modifications to |message_pipe_| and |drop_writes_|.
  base::Optional<base::Lock> lock_;

  std::unique_ptr<SyncHandleWatcher> sync_watcher_;
  bool allow_woken_up_by_others_ = false;
  // If non-zero, currently the control flow is inside the sync handle watcher
  // callback.
  size_t sync_handle_watcher_callback_count_ = 0;

  SEQUENCE_CHECKER(sequence_checker_);

  base::Lock connected_lock_;
  bool connected_ = true;

  // The tag used to track heap allocations that originated from a Watcher
  // notification.
  const char* heap_profiler_tag_ = "unknown interface";

  // A cached pointer to the RunLoopNestingObserver for the thread on which this
  // Connector was created.
  RunLoopNestingObserver* const nesting_observer_;

  // |true| iff the Connector is currently dispatching a message. Used to detect
  // nested dispatch operations.
  bool is_dispatching_ = false;

  // Create a single weak ptr and use it everywhere, to avoid the malloc/free
  // cost of creating a new weak ptr whenever it is needed.
  // NOTE: This weak pointer is invalidated when the message pipe is closed or
  // transferred (i.e., when |connected_| is set to false).
  base::WeakPtr<Connector> weak_self_;
  base::WeakPtrFactory<Connector> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(Connector);
};

}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_BINDINGS_CONNECTOR_H_
