// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_PROTOCOL_CONNECTION_TO_CLIENT_H_
#define REMOTING_PROTOCOL_CONNECTION_TO_CLIENT_H_

#include <stdint.h>

#include <string>

#include "remoting/protocol/message_pipe.h"
#include "remoting/protocol/transport.h"

namespace webrtc {
class DesktopCapturer;
}  // namespace webrtc

namespace remoting {

namespace protocol {

class AudioSource;
class AudioStream;
class ClientStub;
class ClipboardStub;
class HostStub;
class InputStub;
class Session;
class VideoStream;

// This interface represents a remote viewer connection to the chromoting host.
// It sets up all protocol channels and connects them to the stubs.
class ConnectionToClient {
 public:
  class EventHandler {
   public:
    // Called when the network connection is authenticating
    virtual void OnConnectionAuthenticating() = 0;

    // Called when the network connection is authenticated.
    virtual void OnConnectionAuthenticated() = 0;

    // Called to request creation of video streams. May be called before or
    // after OnConnectionChannelsConnected().
    virtual void CreateMediaStreams() = 0;

    // Called when the network connection is authenticated and all
    // channels are connected.
    virtual void OnConnectionChannelsConnected() = 0;

    // Called when the network connection is closed or failed.
    virtual void OnConnectionClosed(ErrorCode error) = 0;

    // Called on notification of a route change event, which happens when a
    // channel is connected.
    virtual void OnRouteChange(const std::string& channel_name,
                               const TransportRoute& route) = 0;

    // Called when a new Data Channel has been created by the client.
    virtual void OnIncomingDataChannel(const std::string& channel_name,
                                       std::unique_ptr<MessagePipe> pipe) = 0;

   protected:
    virtual ~EventHandler() {}
  };

  ConnectionToClient() {}
  virtual ~ConnectionToClient() {}

  // Set |event_handler| for connection events. Must be called once when this
  // object is created.
  virtual void SetEventHandler(EventHandler* event_handler) = 0;

  // Returns the Session object for the connection.
  // TODO(sergeyu): Remove this method.
  virtual Session* session() = 0;

  // Disconnect the client connection.
  virtual void Disconnect(ErrorCode error) = 0;

  // Start video stream that sends screen content from |desktop_capturer| to the
  // client.
  virtual std::unique_ptr<VideoStream> StartVideoStream(
      std::unique_ptr<webrtc::DesktopCapturer> desktop_capturer) = 0;

  // Starts an audio stream. Returns nullptr if audio is not supported by the
  // client.
  virtual std::unique_ptr<AudioStream> StartAudioStream(
      std::unique_ptr<AudioSource> audio_source) = 0;

  // The client stubs used by the host to send control messages to the client.
  // The stub must not be accessed before OnConnectionAuthenticated(), or
  // after OnConnectionClosed().
  virtual ClientStub* client_stub() = 0;

  // Set the stubs which will handle messages we receive from the client. These
  // must be called in EventHandler::OnConnectionAuthenticated().
  virtual void set_clipboard_stub(ClipboardStub* clipboard_stub) = 0;
  virtual void set_host_stub(HostStub* host_stub) = 0;
  virtual void set_input_stub(InputStub* input_stub) = 0;
};

}  // namespace protocol
}  // namespace remoting

#endif  // REMOTING_PROTOCOL_CONNECTION_TO_CLIENT_H_
