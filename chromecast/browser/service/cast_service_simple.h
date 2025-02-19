// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_BROWSER_SERVICE_CAST_SERVICE_SIMPLE_H_
#define CHROMECAST_BROWSER_SERVICE_CAST_SERVICE_SIMPLE_H_

#include <memory>

#include "base/macros.h"
#include "chromecast/browser/cast_web_view.h"
#include "chromecast/service/cast_service.h"
#include "url/gurl.h"

namespace chromecast {
class CastWebContentsManager;
class CastWindowManager;

namespace shell {

class CastServiceSimple : public CastService, public CastWebView::Delegate {
 public:
  CastServiceSimple(content::BrowserContext* browser_context,
                    PrefService* pref_service,
                    CastWindowManager* window_manager);
  ~CastServiceSimple() override;

 protected:
  // CastService implementation:
  void InitializeInternal() override;
  void FinalizeInternal() override;
  void StartInternal() override;
  void StopInternal() override;

  // CastWebView::Delegate implementation:
  void OnPageStopped(int error_code) override;
  void OnLoadingStateChanged(bool loading) override;

  // CastContentWindow::Delegate implementation:
  void OnWindowDestroyed() override;
  void OnKeyEvent(const ui::KeyEvent& key_event) override;

 private:
  CastWindowManager* const window_manager_;
  const std::unique_ptr<CastWebContentsManager> web_contents_manager_;
  std::unique_ptr<CastWebView> cast_web_view_;
  GURL startup_url_;

  DISALLOW_COPY_AND_ASSIGN(CastServiceSimple);
};

}  // namespace shell
}  // namespace chromecast

#endif  // CHROMECAST_BROWSER_SERVICE_CAST_SERVICE_SIMPLE_H_
