// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_CHROME_EXTENSION_WEB_CONTENTS_OBSERVER_H_
#define CHROME_BROWSER_EXTENSIONS_CHROME_EXTENSION_WEB_CONTENTS_OBSERVER_H_

#include <stdint.h>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "content/public/browser/web_contents_user_data.h"
#include "extensions/browser/extension_web_contents_observer.h"
#include "extensions/common/stack_frame.h"

namespace content {
class RenderFrameHost;
}

namespace extensions {

// An ExtensionWebContentsObserver that adds support for the extension error
// console, reloading crashed extensions and routing extension messages between
// renderers.
class ChromeExtensionWebContentsObserver
    : public ExtensionWebContentsObserver,
      public content::WebContentsUserData<ChromeExtensionWebContentsObserver> {
 public:
  ~ChromeExtensionWebContentsObserver() override;

 private:
  friend class content::WebContentsUserData<ChromeExtensionWebContentsObserver>;

  explicit ChromeExtensionWebContentsObserver(
      content::WebContents* web_contents);

  // ExtensionWebContentsObserver:
  void InitializeRenderFrame(
      content::RenderFrameHost* render_frame_host) override;

  // content::WebContentsObserver overrides.
  void RenderFrameCreated(content::RenderFrameHost* render_frame_host) override;
  void DidFinishNavigation(
      content::NavigationHandle* navigation_handle) override;

  // Silence a warning about hiding a virtual function.
  bool OnMessageReceived(const IPC::Message& message,
                         content::RenderFrameHost* render_frame_host) override;

  // Adds a message to the extensions ErrorConsole.
  void OnDetailedConsoleMessageAdded(
      content::RenderFrameHost* render_frame_host,
      const base::string16& message,
      const base::string16& source,
      const StackTrace& stack_trace,
      int32_t severity_level);

  // Reloads an extension if it is on the terminated list.
  void ReloadIfTerminated(content::RenderFrameHost* render_frame_host);

  DISALLOW_COPY_AND_ASSIGN(ChromeExtensionWebContentsObserver);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_CHROME_EXTENSION_WEB_CONTENTS_OBSERVER_H_
