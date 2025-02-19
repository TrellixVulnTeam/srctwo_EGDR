// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_PASSWORD_REUSE_DETECTION_MANAGER_H_
#define COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_PASSWORD_REUSE_DETECTION_MANAGER_H_

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/time/time.h"
#include "components/password_manager/core/browser/password_reuse_detector_consumer.h"
#include "url/gurl.h"

namespace base {
class Clock;
}

namespace password_manager {

class PasswordManagerClient;

// Class for managing password reuse detection. Now it receives keystrokes and
// does nothing with them. TODO(crbug.com/657041): write other features of this
// class when they are implemented. This class is one per-tab.
class PasswordReuseDetectionManager : public PasswordReuseDetectorConsumer {
 public:
  explicit PasswordReuseDetectionManager(PasswordManagerClient* client);
  ~PasswordReuseDetectionManager() override;
  void DidNavigateMainFrame(const GURL& main_frame_url);
  void OnKeyPressed(const base::string16& text);

  // PasswordReuseDetectorConsumer implementation
  void OnReuseFound(const base::string16& password,
                    bool matches_sync_password,
                    const std::vector<std::string>& matching_domains,
                    int saved_passwords) override;

  void SetClockForTesting(std::unique_ptr<base::Clock> clock);

 private:
  PasswordManagerClient* client_;
  base::string16 input_characters_;
  GURL main_frame_url_;
  base::Time last_keystroke_time_;
  // Used to retrieve the current time, in base::Time units.
  std::unique_ptr<base::Clock> clock_;
  bool reuse_on_this_page_was_found_ = false;

  DISALLOW_COPY_AND_ASSIGN(PasswordReuseDetectionManager);
};

}  // namespace password_manager

#endif  // COMPONENTS_PASSWORD_MANAGER_CORE_BROWSER_PASSWORD_REUSE_DETECTION_MANAGER_H_
