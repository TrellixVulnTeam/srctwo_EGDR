// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/api/socket/socket.h"

#include "base/bind.h"
#include "base/lazy_instance.h"
#include "extensions/browser/api/api_resource_manager.h"
#include "net/base/address_list.h"
#include "net/base/io_buffer.h"
#include "net/base/ip_address.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_errors.h"
#include "net/socket/socket.h"

namespace extensions {

const char kSocketTypeNotSupported[] = "Socket type does not support this API";

static base::LazyInstance<
    BrowserContextKeyedAPIFactory<ApiResourceManager<Socket>>>::DestructorAtExit
    g_factory = LAZY_INSTANCE_INITIALIZER;

// static
template <>
BrowserContextKeyedAPIFactory<ApiResourceManager<Socket> >*
ApiResourceManager<Socket>::GetFactoryInstance() {
  return g_factory.Pointer();
}

Socket::Socket(const std::string& owner_extension_id)
    : ApiResource(owner_extension_id), is_connected_(false) {}

Socket::~Socket() {
  // Derived destructors should make sure the socket has been closed.
  DCHECK(!is_connected_);
}

void Socket::Write(scoped_refptr<net::IOBuffer> io_buffer,
                   int byte_count,
                   const CompletionCallback& callback) {
  DCHECK(!callback.is_null());
  write_queue_.push(WriteRequest(io_buffer, byte_count, callback));
  WriteData();
}

void Socket::WriteData() {
  // IO is pending.
  if (io_buffer_write_.get())
    return;

  WriteRequest& request = write_queue_.front();

  DCHECK(request.byte_count >= request.bytes_written);
  io_buffer_write_ = new net::WrappedIOBuffer(request.io_buffer->data() +
                                              request.bytes_written);
  int result =
      WriteImpl(io_buffer_write_.get(),
                request.byte_count - request.bytes_written,
                base::Bind(&Socket::OnWriteComplete, base::Unretained(this)));

  if (result != net::ERR_IO_PENDING)
    OnWriteComplete(result);
}

void Socket::OnWriteComplete(int result) {
  io_buffer_write_ = NULL;

  WriteRequest& request = write_queue_.front();

  if (result >= 0) {
    request.bytes_written += result;
    if (request.bytes_written < request.byte_count) {
      WriteData();
      return;
    }
    DCHECK(request.bytes_written == request.byte_count);
    result = request.bytes_written;
  }

  request.callback.Run(result);
  write_queue_.pop();

  if (!write_queue_.empty())
    WriteData();
}

bool Socket::SetKeepAlive(bool enable, int delay) { return false; }

bool Socket::SetNoDelay(bool no_delay) { return false; }

int Socket::Listen(const std::string& address,
                   uint16_t port,
                   int backlog,
                   std::string* error_msg) {
  *error_msg = kSocketTypeNotSupported;
  return net::ERR_FAILED;
}

void Socket::Accept(const AcceptCompletionCallback& callback) {
  callback.Run(net::ERR_FAILED, NULL);
}

// static
bool Socket::StringAndPortToIPEndPoint(const std::string& ip_address_str,
                                       uint16_t port,
                                       net::IPEndPoint* ip_end_point) {
  DCHECK(ip_end_point);
  net::IPAddress ip_address;
  if (!ip_address.AssignFromIPLiteral(ip_address_str))
    return false;

  *ip_end_point = net::IPEndPoint(ip_address, port);
  return true;
}

void Socket::IPEndPointToStringAndPort(const net::IPEndPoint& address,
                                       std::string* ip_address_str,
                                       uint16_t* port) {
  DCHECK(ip_address_str);
  DCHECK(port);
  *ip_address_str = address.ToStringWithoutPort();
  if (ip_address_str->empty()) {
    *port = 0;
  } else {
    *port = address.port();
  }
}

Socket::WriteRequest::WriteRequest(scoped_refptr<net::IOBuffer> io_buffer,
                                   int byte_count,
                                   const CompletionCallback& callback)
    : io_buffer(io_buffer),
      byte_count(byte_count),
      callback(callback),
      bytes_written(0) {}

Socket::WriteRequest::WriteRequest(const WriteRequest& other) = default;

Socket::WriteRequest::~WriteRequest() {}

}  // namespace extensions
