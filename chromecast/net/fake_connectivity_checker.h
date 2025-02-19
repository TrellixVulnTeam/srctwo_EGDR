// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "chromecast/net/connectivity_checker.h"

namespace chromecast {

// A simple fake connectivity checker for testing. Will appeared to be
// connected by default.
class FakeConnectivityChecker : public ConnectivityChecker {
 public:
  FakeConnectivityChecker();

  // ConnectivityChecker implementation:
  bool Connected() const override;
  void Check() override;

  // Sets connectivity and notifies observers if it has changed.
  void SetConnectedForTest(bool connected);

 protected:
  ~FakeConnectivityChecker() override;

 private:
  friend class base::RefCountedThreadSafe<FakeConnectivityChecker>;
  bool connected_;

  DISALLOW_COPY_AND_ASSIGN(FakeConnectivityChecker);
};

}  // namespace chromecast
