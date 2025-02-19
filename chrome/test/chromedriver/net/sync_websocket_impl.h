// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_NET_SYNC_WEBSOCKET_IMPL_H_
#define CHROME_TEST_CHROMEDRIVER_NET_SYNC_WEBSOCKET_IMPL_H_

#include <list>
#include <memory>
#include <string>

#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/sequenced_task_runner_helpers.h"
#include "base/synchronization/condition_variable.h"
#include "base/synchronization/lock.h"
#include "chrome/test/chromedriver/net/sync_websocket.h"
#include "chrome/test/chromedriver/net/websocket.h"
#include "net/base/completion_callback.h"

namespace base {
class WaitableEvent;
}

namespace net {
class URLRequestContextGetter;
}

class GURL;

class SyncWebSocketImpl : public SyncWebSocket {
 public:
  explicit SyncWebSocketImpl(net::URLRequestContextGetter* context_getter);
  ~SyncWebSocketImpl() override;

  // Overridden from SyncWebSocket:
  bool IsConnected() override;
  bool Connect(const GURL& url) override;
  bool Send(const std::string& message) override;
  StatusCode ReceiveNextMessage(std::string* message,
                                const Timeout& timeout) override;
  bool HasNextMessage() override;

 private:
  struct CoreTraits;
  class Core : public WebSocketListener,
               public base::RefCountedThreadSafe<Core, CoreTraits> {
   public:
    explicit Core(net::URLRequestContextGetter* context_getter);

    bool IsConnected();
    bool Connect(const GURL& url);
    bool Send(const std::string& message);
    SyncWebSocket::StatusCode ReceiveNextMessage(
        std::string* message,
        const Timeout& timeout);
    bool HasNextMessage();

    // Overriden from WebSocketListener:
    void OnMessageReceived(const std::string& message) override;
    void OnClose() override;

   private:
    friend class base::RefCountedThreadSafe<Core, CoreTraits>;
    friend class base::DeleteHelper<Core>;
    friend struct CoreTraits;

    ~Core() override;

    void ConnectOnIO(const GURL& url,
                     bool* success,
                     base::WaitableEvent* event);
    void OnConnectCompletedOnIO(bool* connected,
                                base::WaitableEvent* event,
                                int error);
    void SendOnIO(const std::string& message,
                  bool* result,
                  base::WaitableEvent* event);

    // OnDestruct is meant to ensure deletion on the IO thread.
    void OnDestruct() const;

    scoped_refptr<net::URLRequestContextGetter> context_getter_;

    // Only accessed on IO thread.
    std::unique_ptr<WebSocket> socket_;

    base::Lock lock_;

    // Protected by |lock_|.
    bool is_connected_;

    // Protected by |lock_|.
    std::list<std::string> received_queue_;

    // Protected by |lock_|.
    // Signaled when the socket closes or a message is received.
    base::ConditionVariable on_update_event_;
  };

  scoped_refptr<Core> core_;
};

struct SyncWebSocketImpl::CoreTraits {
  static void Destruct(const SyncWebSocketImpl::Core* core) {
    core->OnDestruct();
  }
};

#endif  // CHROME_TEST_CHROMEDRIVER_NET_SYNC_WEBSOCKET_IMPL_H_
