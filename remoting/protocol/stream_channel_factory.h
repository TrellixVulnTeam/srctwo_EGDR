// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_PROTOCOL_STREAM_CHANNEL_FACTORY_H_
#define REMOTING_PROTOCOL_STREAM_CHANNEL_FACTORY_H_

#include <memory>
#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "base/sequence_checker.h"

namespace remoting {
namespace protocol {

class P2PStreamSocket;

class StreamChannelFactory {
 public:
  // TODO(sergeyu): Specify connection error code when channel
  // connection fails.
  typedef base::Callback<void(std::unique_ptr<P2PStreamSocket>)>
      ChannelCreatedCallback;

  StreamChannelFactory() {}

  // Creates new channels and calls the |callback| when then new channel is
  // created and connected. The |callback| is called with nullptr if connection
  // failed for any reason. Callback may be called synchronously, before the
  // call returns. All channels must be destroyed, and CancelChannelCreation()
  // called for any pending channels, before the factory is destroyed.
  virtual void CreateChannel(const std::string& name,
                             const ChannelCreatedCallback& callback) = 0;

  // Cancels a pending CreateChannel() operation for the named channel. If the
  // channel creation already completed then canceling it has no effect. When
  // shutting down this method must be called for each channel pending creation.
  virtual void CancelChannelCreation(const std::string& name) = 0;

 protected:
  virtual ~StreamChannelFactory() {}

  SEQUENCE_CHECKER(sequence_checker_);

 private:
  DISALLOW_COPY_AND_ASSIGN(StreamChannelFactory);
};

}  // namespace protocol
}  // namespace remoting

#endif  // REMOTING_PROTOCOL_STREAM_CHANNEL_FACTORY_H_
