// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_BASE_CHROME_RENDER_VIEW_TEST_H_
#define CHROME_TEST_BASE_CHROME_RENDER_VIEW_TEST_H_

#include <memory>
#include <string>

#include "chrome/renderer/chrome_mock_render_thread.h"
#include "content/public/test/render_view_test.h"
#include "extensions/features/features.h"
#include "services/service_manager/public/cpp/binder_registry.h"

class ChromeContentRendererClient;

namespace autofill {
class AutofillAgent;
class TestPasswordAutofillAgent;
class TestPasswordGenerationAgent;
}

namespace extensions {
class DispatcherDelegate;
}

class ChromeRenderViewTest : public content::RenderViewTest {
 public:
  ChromeRenderViewTest();
  ~ChromeRenderViewTest() override;

 protected:
  // testing::Test
  void SetUp() override;
  void TearDown() override;
  content::ContentClient* CreateContentClient() override;
  content::ContentBrowserClient* CreateContentBrowserClient() override;
  content::ContentRendererClient* CreateContentRendererClient() override;

  // Called from SetUp(). Override to register mojo interfaces.
  virtual void RegisterMainFrameRemoteInterfaces();

  // Initializes commonly needed global state and renderer client parts.
  // Use when overriding CreateContentRendererClient.
  void InitChromeContentRendererClient(ChromeContentRendererClient* client);

  void EnableUserGestureSimulationForAutofill();
  void DisableUserGestureSimulationForAutofill();
  void WaitForAutofillDidAssociateFormControl();

#if BUILDFLAG(ENABLE_EXTENSIONS)
  std::unique_ptr<extensions::DispatcherDelegate>
      extension_dispatcher_delegate_;
#endif

  autofill::TestPasswordAutofillAgent* password_autofill_agent_;
  autofill::TestPasswordGenerationAgent* password_generation_;
  autofill::AutofillAgent* autofill_agent_;

  std::unique_ptr<service_manager::BinderRegistry> registry_;

  // Naked pointer as ownership is with content::RenderViewTest::render_thread_.
  ChromeMockRenderThread* chrome_render_thread_;
};

#endif  // CHROME_TEST_BASE_CHROME_RENDER_VIEW_TEST_H_
