// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_PERMISSION_BUBBLE_PERMISSION_PROMPT_H_
#define CHROME_BROWSER_UI_PERMISSION_BUBBLE_PERMISSION_PROMPT_H_

#include <memory>
#include <vector>

#include "base/callback.h"
#include "ui/gfx/native_widget_types.h"

class PermissionRequest;

namespace content {
class WebContents;
}

// This class is the platform-independent interface through which the permission
// request managers (which are one per tab) communicate to the UI surface.
// When the visible tab changes, the UI code must provide an object of this type
// to the manager for the visible tab.
class PermissionPrompt {
 public:
  // The delegate will receive events caused by user action which need to
  // be persisted in the per-tab UI state.
  class Delegate {
   public:
    virtual ~Delegate() {}

    // These pointers should not be stored as the actual request objects may be
    // deleted upon navigation and so on.
    virtual const std::vector<PermissionRequest*>& Requests() = 0;

    virtual void TogglePersist(bool new_value) = 0;
    virtual void Accept() = 0;
    virtual void Deny() = 0;
    virtual void Closing() = 0;
  };

  typedef base::Callback<
      std::unique_ptr<PermissionPrompt>(content::WebContents*, Delegate*)>
      Factory;

  // Create and display a platform specific prompt.
  static std::unique_ptr<PermissionPrompt> Create(
      content::WebContents* web_contents,
      Delegate* delegate);
  virtual ~PermissionPrompt() {}

  // Returns true if the view can accept a new Show() command to coalesce
  // requests. Currently the policy is that this should return true if the view
  // is being shown and the mouse is not over the view area (!IsMouseHovered).
  virtual bool CanAcceptRequestUpdate() = 0;

  // Updates where the prompt should be anchored. ex: fullscreen toggle.
  virtual void UpdateAnchorPosition() = 0;

  // Returns a reference to this prompt's native window.
  // TODO(hcarmona): Remove this as part of the bubble API work.
  virtual gfx::NativeWindow GetNativeWindow() = 0;
};

#endif  // CHROME_BROWSER_UI_PERMISSION_BUBBLE_PERMISSION_PROMPT_H_
