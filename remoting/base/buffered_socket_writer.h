// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_BASE_BUFFERED_SOCKET_WRITER_H_
#define REMOTING_BASE_BUFFERED_SOCKET_WRITER_H_

#include <list>
#include <memory>

#include "base/callback.h"
#include "base/memory/weak_ptr.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread_checker.h"
#include "net/base/completion_callback.h"
#include "net/base/io_buffer.h"

namespace net {
class Socket;
}  // namespace net

namespace remoting {

// BufferedSocketWriter implement write data queue for stream sockets.
class BufferedSocketWriter {
 public:
  typedef base::Callback<int(const scoped_refptr<net::IOBuffer>& buf,
                             int buf_len,
                             const net::CompletionCallback& callback)>
      WriteCallback;
  typedef base::Callback<void(int)> WriteFailedCallback;

  static std::unique_ptr<BufferedSocketWriter> CreateForSocket(
      net::Socket* socket,
      const WriteFailedCallback& write_failed_callback);

  BufferedSocketWriter();
  virtual ~BufferedSocketWriter();

  // Starts the writer. |write_callback| is called to write data to the
  // socket. |write_failed_callback| is called when write operation fails.
  // Writing stops after the first failed write.
  void Start(const WriteCallback& write_callback,
             const WriteFailedCallback& write_failed_callback);

  // Puts a new data chunk in the buffer. If called before Start() then all data
  // is buffered until Start().
  void Write(const scoped_refptr<net::IOBufferWithSize>& buffer,
             const base::Closure& done_task);

  // Returns true when there is data waiting to be written.
  bool has_data_pending() { return !queue_.empty(); }

 private:
  struct PendingPacket;

  void DoWrite();
  void HandleWriteResult(int result);
  void OnWritten(int result);

  base::ThreadChecker thread_checker_;

  WriteCallback write_callback_;
  WriteFailedCallback write_failed_callback_;

  bool closed_ = false;

  std::list<std::unique_ptr<PendingPacket>> queue_;

  bool write_pending_ = false;

  base::WeakPtrFactory<BufferedSocketWriter> weak_factory_;
};

}  // namespace remoting

#endif  // REMOTING_BASE_BUFFERED_SOCKET_WRITER_H_
