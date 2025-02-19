// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/devtools/device/adb/adb_client_socket.h"

#include <stddef.h>

#include <utility>

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "net/base/ip_address.h"
#include "net/base/net_errors.h"
#include "net/log/net_log_source.h"
#include "net/socket/tcp_client_socket.h"

namespace {

const int kBufferSize = 16 * 1024;
const char kOkayResponse[] = "OKAY";
const char kHostTransportCommand[] = "host:transport:%s";
const char kLocalhost[] = "127.0.0.1";

std::string EncodeMessage(const std::string& message) {
  static const char kHexChars[] = "0123456789ABCDEF";

  size_t length = message.length();
  std::string result(4, '\0');
  char b = reinterpret_cast<const char*>(&length)[1];
  result[0] = kHexChars[(b >> 4) & 0xf];
  result[1] = kHexChars[b & 0xf];
  b = reinterpret_cast<const char*>(&length)[0];
  result[2] = kHexChars[(b >> 4) & 0xf];
  result[3] = kHexChars[b & 0xf];
  return result + message;
}

class AdbTransportSocket : public AdbClientSocket {
 public:
  AdbTransportSocket(int port,
                     const std::string& serial,
                     const std::string& socket_name,
                     const SocketCallback& callback)
    : AdbClientSocket(port),
      serial_(serial),
      socket_name_(socket_name),
      callback_(callback) {
    Connect(base::Bind(&AdbTransportSocket::OnConnected,
                       base::Unretained(this)));
  }

 private:
  ~AdbTransportSocket() {}

  void OnConnected(int result) {
    if (!CheckNetResultOrDie(result))
      return;
    SendCommand(base::StringPrintf(kHostTransportCommand, serial_.c_str()),
        true, base::Bind(&AdbTransportSocket::SendLocalAbstract,
                         base::Unretained(this)));
  }

  void SendLocalAbstract(int result, const std::string& response) {
    if (!CheckNetResultOrDie(result))
      return;
    SendCommand(socket_name_, true,
                base::Bind(&AdbTransportSocket::OnSocketAvailable,
                           base::Unretained(this)));
  }

  void OnSocketAvailable(int result, const std::string& response) {
    if (!CheckNetResultOrDie(result))
      return;
    callback_.Run(net::OK, std::move(socket_));
    delete this;
  }

  bool CheckNetResultOrDie(int result) {
    if (result >= 0)
      return true;
    callback_.Run(result, base::WrapUnique<net::StreamSocket>(NULL));
    delete this;
    return false;
  }

  std::string serial_;
  std::string socket_name_;
  SocketCallback callback_;
};

class AdbQuerySocket : AdbClientSocket {
 public:
  AdbQuerySocket(int port,
                 const std::string& query,
                 const CommandCallback& callback)
      : AdbClientSocket(port),
        current_query_(0),
        callback_(callback) {
    queries_ = base::SplitString(query, "|", base::KEEP_WHITESPACE,
                                 base::SPLIT_WANT_NONEMPTY);
    if (queries_.empty()) {
      CheckNetResultOrDie(net::ERR_INVALID_ARGUMENT);
      return;
    }
    Connect(base::Bind(&AdbQuerySocket::SendNextQuery,
                       base::Unretained(this)));
  }

 private:
  ~AdbQuerySocket() {
  }

  void SendNextQuery(int result) {
    if (!CheckNetResultOrDie(result))
      return;
    std::string query = queries_[current_query_];
    if (query.length() > 0xFFFF) {
      CheckNetResultOrDie(net::ERR_MSG_TOO_BIG);
      return;
    }
    bool is_void = current_query_ < queries_.size() - 1;
    SendCommand(query, is_void,
        base::Bind(&AdbQuerySocket::OnResponse, base::Unretained(this)));
  }

  void OnResponse(int result, const std::string& response) {
    if (++current_query_ < queries_.size()) {
      SendNextQuery(net::OK);
    } else {
      callback_.Run(result, response);
      delete this;
    }
  }

  bool CheckNetResultOrDie(int result) {
    if (result >= 0)
      return true;
    callback_.Run(result, std::string());
    delete this;
    return false;
  }

  std::vector<std::string> queries_;
  size_t current_query_;
  CommandCallback callback_;
};

}  // namespace

// static
void AdbClientSocket::AdbQuery(int port,
                               const std::string& query,
                               const CommandCallback& callback) {
  new AdbQuerySocket(port, query, callback);
}

// static
void AdbClientSocket::TransportQuery(int port,
                                     const std::string& serial,
                                     const std::string& socket_name,
                                     const SocketCallback& callback) {
  new AdbTransportSocket(port, serial, socket_name, callback);
}

AdbClientSocket::AdbClientSocket(int port)
    : host_(kLocalhost), port_(port) {
}

AdbClientSocket::~AdbClientSocket() {
}

void AdbClientSocket::Connect(const net::CompletionCallback& callback) {
  net::IPAddress ip_address;
  if (!ip_address.AssignFromIPLiteral(host_)) {
    callback.Run(net::ERR_FAILED);
    return;
  }

  net::AddressList address_list =
      net::AddressList::CreateFromIPAddress(ip_address, port_);
  socket_.reset(new net::TCPClientSocket(address_list, NULL, NULL,
                                         net::NetLogSource()));
  int result = socket_->Connect(callback);
  if (result != net::ERR_IO_PENDING)
    callback.Run(result);
}

void AdbClientSocket::SendCommand(const std::string& command,
                                  bool is_void,
                                  const CommandCallback& callback) {
  scoped_refptr<net::StringIOBuffer> request_buffer =
      new net::StringIOBuffer(EncodeMessage(command));
  int result = socket_->Write(request_buffer.get(),
                              request_buffer->size(),
                              base::Bind(&AdbClientSocket::ReadResponse,
                                         base::Unretained(this),
                                         callback,
                                         is_void));
  if (result != net::ERR_IO_PENDING)
    ReadResponse(callback, is_void, result);
}

void AdbClientSocket::ReadResponse(const CommandCallback& callback,
                                   bool is_void,
                                   int result) {
  if (result < 0) {
    callback.Run(result, "IO error");
    return;
  }
  scoped_refptr<net::IOBuffer> response_buffer =
      new net::IOBuffer(kBufferSize);
  result = socket_->Read(response_buffer.get(),
                         kBufferSize,
                         base::Bind(&AdbClientSocket::OnResponseHeader,
                                    base::Unretained(this),
                                    callback,
                                    is_void,
                                    response_buffer));
  if (result != net::ERR_IO_PENDING)
    OnResponseHeader(callback, is_void, response_buffer, result);
}

void AdbClientSocket::OnResponseHeader(
    const CommandCallback& callback,
    bool is_void,
    scoped_refptr<net::IOBuffer> response_buffer,
    int result) {
  if (result <= 0) {
    callback.Run(result == 0 ? net::ERR_CONNECTION_CLOSED : result,
                 "IO error");
    return;
  }

  std::string data = std::string(response_buffer->data(), result);
  if (result < 4) {
    callback.Run(net::ERR_FAILED, "Response is too short: " + data);
    return;
  }

  std::string status = data.substr(0, 4);
  if (status != kOkayResponse) {
    callback.Run(net::ERR_FAILED, data);
    return;
  }

  // Trim OKAY.
  data = data.substr(4);
  if (!is_void)
    OnResponseData(callback, data, response_buffer, -1, 0);
  else
    callback.Run(net::OK, data);
}

void AdbClientSocket::OnResponseData(
    const CommandCallback& callback,
    const std::string& response,
    scoped_refptr<net::IOBuffer> response_buffer,
    int bytes_left,
    int result) {
  if (result < 0) {
    callback.Run(result, "IO error");
    return;
  }

  std::string new_response = response +
      std::string(response_buffer->data(), result);

  if (bytes_left == -1) {
    // First read the response header.
    int payload_length = 0;
    if (new_response.length() >= 4 &&
        base::HexStringToInt(new_response.substr(0, 4), &payload_length)) {
      new_response = new_response.substr(4);
      bytes_left = payload_length - new_response.size();
    }
  } else {
    bytes_left -= result;
  }

  if (bytes_left == 0) {
    callback.Run(net::OK, new_response);
    return;
  }

  // Read tail
  result = socket_->Read(response_buffer.get(),
                         kBufferSize,
                         base::Bind(&AdbClientSocket::OnResponseData,
                                    base::Unretained(this),
                                    callback,
                                    new_response,
                                    response_buffer,
                                    bytes_left));
  if (result > 0)
    OnResponseData(callback, new_response, response_buffer, bytes_left, result);
  else if (result != net::ERR_IO_PENDING)
    callback.Run(net::OK, new_response);
}
