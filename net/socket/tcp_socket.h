// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_SOCKET_TCP_SOCKET_H_
#define NET_SOCKET_TCP_SOCKET_H_

#include "build/build_config.h"
#include "net/base/net_export.h"
#include "net/socket/socket_descriptor.h"

#if defined(OS_WIN)
#include "net/socket/tcp_socket_win.h"
#elif defined(OS_POSIX)
#include "net/socket/tcp_socket_posix.h"
#endif

namespace net {

// TCPSocket provides a platform-independent interface for TCP sockets.
//
// It is recommended to use TCPClientSocket/TCPServerSocket instead of this
// class, unless a clear separation of client and server socket functionality is
// not suitable for your use case (e.g., a socket needs to be created and bound
// before you know whether it is a client or server socket).
#if defined(OS_WIN)
typedef TCPSocketWin TCPSocket;
#elif defined(OS_POSIX)
typedef TCPSocketPosix TCPSocket;
#endif

// Check if TCP FastOpen is supported by the OS.
bool IsTCPFastOpenSupported();

// Check if TCP FastOpen is enabled by the user.
bool IsTCPFastOpenUserEnabled();

// Checks if TCP FastOpen is supported by the kernel. Also enables TFO for all
// connections if indicated by user.
// Not thread safe.  Must be called during initialization/startup only.
NET_EXPORT void CheckSupportAndMaybeEnableTCPFastOpen(bool user_enabled);

}  // namespace net

#endif  // NET_SOCKET_TCP_SOCKET_H_
