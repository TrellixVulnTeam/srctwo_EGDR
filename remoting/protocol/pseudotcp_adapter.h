// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_PROTOCOL_PSEUDOTCP_ADAPTER_H_
#define REMOTING_PROTOCOL_PSEUDOTCP_ADAPTER_H_

#include <stdint.h>

#include <memory>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/sequence_checker.h"
#include "net/log/net_log_with_source.h"
#include "remoting/protocol/p2p_stream_socket.h"
#include "third_party/webrtc/p2p/base/pseudotcp.h"

namespace remoting {
namespace protocol {

class P2PDatagramSocket;

// PseudoTcpAdapter adapts a P2PDatagramSocket to P2PStreamSocket using
// PseudoTcp. Because P2PStreamSockets can be deleted during callbacks,
// while PseudoTcp cannot, the core of the PseudoTcpAdapter is reference
// counted, with a reference held by the adapter, and an additional reference
// held on the stack during callbacks.
class PseudoTcpAdapter : public P2PStreamSocket {
 public:
  explicit PseudoTcpAdapter(std::unique_ptr<P2PDatagramSocket> socket);
  ~PseudoTcpAdapter() override;

  // P2PStreamSocket implementation.
  int Read(const scoped_refptr<net::IOBuffer>& buffer, int buffer_size,
           const net::CompletionCallback& callback) override;
  int Write(const scoped_refptr<net::IOBuffer>& buffer, int buffer_size,
            const net::CompletionCallback& callback) override;

  int Connect(const net::CompletionCallback& callback);

  // Set receive and send buffer sizes.
  int SetReceiveBufferSize(int32_t size);
  int SetSendBufferSize(int32_t size);

  // Set the delay for sending ACK.
  void SetAckDelay(int delay_ms);

  // Set whether Nagle's algorithm is enabled.
  void SetNoDelay(bool no_delay);

  // When write_waits_for_send flag is set to true the Write() method
  // will wait until the data is sent to the remote end before the
  // write completes (it still doesn't wait until the data is received
  // and acknowledged by the remote end). Otherwise write completes
  // after the data has been copied to the send buffer.
  //
  // This flag is useful in cases when the sender needs to get
  // feedback from the connection when it is congested. E.g. remoting
  // host uses this feature to adjust screen capturing rate according
  // to the available bandwidth. In the same time it may negatively
  // impact performance in some cases. E.g. when the sender writes one
  // byte at a time then each byte will always be sent in a separate
  // packet.
  //
  // TODO(sergeyu): Remove this flag once remoting has a better
  // flow-control solution.
  void SetWriteWaitsForSend(bool write_waits_for_send);

 private:
  class Core;

  scoped_refptr<Core> core_;

  net::NetLogWithSource net_log_;

  SEQUENCE_CHECKER(sequence_checker_);

  DISALLOW_COPY_AND_ASSIGN(PseudoTcpAdapter);
};

}  // namespace protocol
}  // namespace remoting

#endif  // REMOTING_PROTOCOL_PSEUDOTCP_ADAPTER_H_
