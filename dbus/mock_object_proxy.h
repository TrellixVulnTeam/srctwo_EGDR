// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DBUS_MOCK_OBJECT_PROXY_H_
#define DBUS_MOCK_OBJECT_PROXY_H_

#include <memory>
#include <string>

#include "dbus/message.h"
#include "dbus/object_path.h"
#include "dbus/object_proxy.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace dbus {

// Mock for ObjectProxy.
class MockObjectProxy : public ObjectProxy {
 public:
  MockObjectProxy(Bus* bus,
                  const std::string& service_name,
                  const ObjectPath& object_path);

  MOCK_METHOD3(CallMethodAndBlockWithErrorDetails,
               std::unique_ptr<Response>(MethodCall* method_call,
                                         int timeout_ms,
                                         ScopedDBusError* error));
  MOCK_METHOD2(CallMethodAndBlock,
               std::unique_ptr<Response>(MethodCall* method_call,
                                         int timeout_ms));

  // This method is not mockable because it takes a move-only argument. To work
  // around this, CallMethod() implementation here calls DoCallMethod() which is
  // mockable.
  void CallMethod(MethodCall* method_call,
                  int timeout_ms,
                  ResponseCallback callback) override;
  MOCK_METHOD3(DoCallMethod,
               void(MethodCall* method_call,
                    int timeout_ms,
                    ResponseCallback* callback));

  // This method is not mockable because it takes a move-only argument. To work
  // around this, CallMethodWithErrorCallback() implementation here calls
  // DoCallMethodWithErrorCallback() which is mockable.
  void CallMethodWithErrorCallback(MethodCall* method_call,
                                   int timeout_ms,
                                   ResponseCallback callback,
                                   ErrorCallback error_callback) override;
  MOCK_METHOD4(DoCallMethodWithErrorCallback,
               void(MethodCall* method_call,
                    int timeout_ms,
                    ResponseCallback* callback,
                    ErrorCallback* error_callback));
  MOCK_METHOD4(ConnectToSignal,
               void(const std::string& interface_name,
                    const std::string& signal_name,
                    SignalCallback signal_callback,
                    OnConnectedCallback on_connected_callback));
  MOCK_METHOD1(SetNameOwnerChangedCallback,
               void(NameOwnerChangedCallback callback));
  MOCK_METHOD1(WaitForServiceToBeAvailable,
               void(WaitForServiceToBeAvailableCallback callback));
  MOCK_METHOD0(Detach, void());

 protected:
  ~MockObjectProxy() override;
};

}  // namespace dbus

#endif  // DBUS_MOCK_OBJECT_PROXY_H_
