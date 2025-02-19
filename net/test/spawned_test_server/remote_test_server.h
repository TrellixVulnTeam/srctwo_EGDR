// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_TEST_SPAWNED_TEST_SERVER_REMOTE_TEST_SERVER_H_
#define NET_TEST_SPAWNED_TEST_SERVER_REMOTE_TEST_SERVER_H_

#include <string>

#include "base/macros.h"
#include "base/threading/thread.h"
#include "net/test/spawned_test_server/base_test_server.h"
#include "net/test/spawned_test_server/remote_test_server_config.h"

namespace net {

class RemoteTestServerSpawnerRequest;
class RemoteTestServerProxy;

// The RemoteTestServer runs an external Python-based test server in another
// machine that is different from the machine that executes the tests. It is
// necessary because it's not always possible to run the test server on the same
// machine (it doesn't run on Android and Fuchsia because it's written in
// Python).
//
// The actual test server is executed on the host machine, while the unit tests
// themselves continue running on the device. To control the test server on the
// host machine, a second HTTP server is started, the spawner server, which
// controls the life cycle of remote test servers. Calls to start/kill the
// SpawnedTestServer are then redirected to the spawner server via
// this spawner communicator. The spawner is implemented in
// build/util/lib/common/chrome_test_server_spawner.py .
//
// Currently the following two commands are supported by spawner.
//
// (1) Start Python test server, format is:
// Path: "/start".
// Method: "POST".
// Data to server: all arguments needed to launch the Python test server, in
//   JSON format.
// Data from server: a JSON dict includes the following two field if success,
//   "port": the port the Python test server actually listen on that.
//   "message": must be "started".
//
// (2) Kill Python test server, format is:
// Path: "/kill".
// Method: "GET".
// Data to server: port=<server_port>.
// Data from server: String "killed" returned if success.
//
// The internal I/O thread is required by net stack to perform net I/O.
// The Start/StopServer methods block the caller thread until result is
// fetched from spawner server or timed-out.
class RemoteTestServer : public BaseTestServer {
 public:
  // Initialize a TestServer. |document_root| must be a relative path under the
  // root tree.
  RemoteTestServer(Type type, const base::FilePath& document_root);

  // Initialize a TestServer with a specific set of SSLOptions.
  // |document_root| must be a relative path under the root tree.
  RemoteTestServer(Type type,
                   const SSLOptions& ssl_options,
                   const base::FilePath& document_root);

  ~RemoteTestServer() override;

  // BaseTestServer overrides.
  bool StartInBackground() override WARN_UNUSED_RESULT;
  bool BlockUntilStarted() override WARN_UNUSED_RESULT;

  // Stops the Python test server that is running on the host machine.
  bool Stop();

  // Returns the actual path of document root for the test cases. This function
  // should be called by test cases to retrieve the actual document root path
  // on the Android device, otherwise document_root() function is used to get
  // the document root.
  base::FilePath GetDocumentRoot() const;

 private:
  bool Init(const base::FilePath& document_root);

  RemoteTestServerConfig config_;

  // Thread used to run all IO activity in RemoteTestServerSpawnerRequest and
  // |test_server_proxy_|.
  base::Thread io_thread_;

  std::unique_ptr<RemoteTestServerSpawnerRequest> start_request_;

  // Server port. Non-zero when the server is running.
  int remote_port_ = 0;

  std::unique_ptr<RemoteTestServerProxy> test_server_proxy_;

  DISALLOW_COPY_AND_ASSIGN(RemoteTestServer);
};

}  // namespace net

#endif  // NET_TEST_SPAWNED_TEST_SERVER_REMOTE_TEST_SERVER_H_
