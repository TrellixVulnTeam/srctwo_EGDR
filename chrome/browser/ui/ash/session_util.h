// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_ASH_SESSION_UTIL_H_
#define CHROME_BROWSER_UI_ASH_SESSION_UTIL_H_

#include "base/callback.h"
#include "ui/gfx/image/image_skia.h"

namespace aura {
class Window;
}

namespace content {
class BrowserContext;
}

namespace user_manager {
class User;
}

content::BrowserContext* GetActiveBrowserContext();

using GetActiveBrowserContextCallback =
    base::Callback<content::BrowserContext*(void)>;

// See documentation in ash::ShellDelegate for the method of the same name.
// |context| is the content::BrowserContext deemed active for the current
// scenario. This is passed in because it can differ in tests vs. production.
// See for example MultiUserWindowManagerTestChromeOS.
bool CanShowWindowForUser(
    aura::Window* window,
    const GetActiveBrowserContextCallback& get_context_callback);

gfx::ImageSkia GetAvatarImageForContext(content::BrowserContext* context);
gfx::ImageSkia GetAvatarImageForUser(const user_manager::User* user);

#endif  // CHROME_BROWSER_UI_ASH_SESSION_UTIL_H_
