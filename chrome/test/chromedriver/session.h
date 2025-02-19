// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_CHROMEDRIVER_SESSION_H_
#define CHROME_TEST_CHROMEDRIVER_SESSION_H_

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "base/time/time.h"
#include "chrome/test/chromedriver/basic_types.h"
#include "chrome/test/chromedriver/chrome/device_metrics.h"
#include "chrome/test/chromedriver/chrome/geoposition.h"
#include "chrome/test/chromedriver/chrome/network_conditions.h"
#include "chrome/test/chromedriver/chrome/scoped_temp_dir_with_retry.h"
#include "chrome/test/chromedriver/chrome/ui_events.h"
#include "chrome/test/chromedriver/command_listener.h"

static const char kAccept[] = "accept";
static const char kDismiss[] = "dismiss";
static const char kIgnore[] = "ignore";

namespace base {
class DictionaryValue;
}

class Chrome;
class Status;
class WebDriverLog;
class WebView;

struct FrameInfo {
  FrameInfo(const std::string& parent_frame_id,
            const std::string& frame_id,
            const std::string& chromedriver_frame_id);

  std::string parent_frame_id;
  std::string frame_id;
  std::string chromedriver_frame_id;
};

struct Session {
  static const base::TimeDelta kDefaultPageLoadTimeout;
  static const base::TimeDelta kDefaultScriptTimeout;

  explicit Session(const std::string& id);
  Session(const std::string& id, std::unique_ptr<Chrome> chrome);
  ~Session();

  Status GetTargetWindow(WebView** web_view);

  void SwitchToTopFrame();
  void SwitchToParentFrame();
  void SwitchToSubFrame(const std::string& frame_id,
                        const std::string& chromedriver_frame_id);
  std::string GetCurrentFrameId() const;
  std::vector<WebDriverLog*> GetAllLogs() const;
  std::string GetFirstBrowserError() const;

  const std::string id;
  bool w3c_compliant;
  bool quit;
  bool detach;
  bool force_devtools_screenshot;
  std::unique_ptr<Chrome> chrome;
  std::string window;
  int sticky_modifiers;
  // List of |FrameInfo|s for each frame to the current target frame from the
  // first frame element in the root document. If target frame is window.top,
  // this list will be empty.
  std::list<FrameInfo> frames;
  WebPoint mouse_position;
  MouseButton pressed_mouse_button;
  base::TimeDelta implicit_wait;
  base::TimeDelta page_load_timeout;
  base::TimeDelta script_timeout;
  std::unique_ptr<std::string> prompt_text;
  std::unique_ptr<Geoposition> overridden_geoposition;
  std::unique_ptr<DeviceMetrics> overridden_device_metrics;
  std::unique_ptr<NetworkConditions> overridden_network_conditions;
  std::string orientation_type;
  // Logs that populate from DevTools events.
  std::vector<std::unique_ptr<WebDriverLog>> devtools_logs;
  std::unique_ptr<WebDriverLog> driver_log;
  ScopedTempDirWithRetry temp_dir;
  std::unique_ptr<base::DictionaryValue> capabilities;
  bool auto_reporting_enabled;
  // |command_listeners| should be declared after |chrome|. When the |Session|
  // is destroyed, |command_listeners| should be freed first, since some
  // |CommandListener|s might be |CommandListenerProxy|s that forward to
  // |DevToolsEventListener|s owned by |chrome|.
  std::vector<std::unique_ptr<CommandListener>> command_listeners;
  std::string unexpected_alert_behaviour;
};

Session* GetThreadLocalSession();

void SetThreadLocalSession(std::unique_ptr<Session> session);

#endif  // CHROME_TEST_CHROMEDRIVER_SESSION_H_
