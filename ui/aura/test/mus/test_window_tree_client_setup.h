// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_TEST_MUS_TEST_WINDOW_TREE_CLIENT_SETUP_H_
#define UI_AURA_TEST_MUS_TEST_WINDOW_TREE_CLIENT_SETUP_H_

#include <memory>

#include "base/macros.h"

namespace aura {

class TestWindowManagerClient;
class TestWindowTree;
class WindowManagerDelegate;
class WindowTreeClient;
class WindowTreeClientDelegate;

// TestWindowTreeClientSetup is used to create a WindowTreeClient that is not
// connected to mus.
class TestWindowTreeClientSetup {
 public:
  TestWindowTreeClientSetup();
  ~TestWindowTreeClientSetup();

  // Initializes the WindowTreeClient.
  void Init(WindowTreeClientDelegate* window_tree_delegate);
  void InitForWindowManager(WindowTreeClientDelegate* window_tree_delegate,
                            WindowManagerDelegate* window_manager_delegate);

  // The WindowTree that WindowTreeClient talks to.
  TestWindowTree* window_tree() { return window_tree_.get(); }

  // Returns ownership of WindowTreeClient to the caller.
  std::unique_ptr<WindowTreeClient> OwnWindowTreeClient();

  WindowTreeClient* window_tree_client();

  TestWindowManagerClient* test_window_manager_client() {
    return test_window_manager_client_.get();
  }

 private:
  // Called by both implementations of init to perform common initialization.
  void CommonInit(WindowTreeClientDelegate* window_tree_delegate,
                  WindowManagerDelegate* window_manager_delegate);

  std::unique_ptr<TestWindowTree> window_tree_;

  std::unique_ptr<WindowTreeClient> window_tree_client_;

  std::unique_ptr<TestWindowManagerClient> test_window_manager_client_;

  DISALLOW_COPY_AND_ASSIGN(TestWindowTreeClientSetup);
};

}  // namespace aura

#endif  // UI_AURA_TEST_MUS_TEST_WINDOW_TREE_CLIENT_SETUP_H_
