// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_SCREENS_ERROR_SCREEN_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_SCREENS_ERROR_SCREEN_H_

#include <memory>

#include "base/callback_list.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/chromeos/login/screens/base_screen.h"
#include "chrome/browser/chromeos/login/screens/network_error.h"
#include "chrome/browser/chromeos/settings/device_settings_service.h"
#include "chrome/browser/ui/webui/chromeos/login/network_state_informer.h"
#include "chromeos/login/auth/login_performer.h"

namespace chromeos {

class BaseScreenDelegate;
class CaptivePortalWindowProxy;
class NetworkErrorView;

// Controller for the error screen.
class ErrorScreen : public BaseScreen, public LoginPerformer::Delegate {
 public:
  using ConnectRequestCallbackSubscription =
      std::unique_ptr<base::CallbackList<void()>::Subscription>;

  // TODO(jdufault): Some of these are no longer used and can be removed. See
  // crbug.com/672142.
  static const char kUserActionConfigureCertsButtonClicked[];
  static const char kUserActionDiagnoseButtonClicked[];
  static const char kUserActionLaunchOobeGuestSessionClicked[];
  static const char kUserActionLocalStateErrorPowerwashButtonClicked[];
  static const char kUserActionRebootButtonClicked[];
  static const char kUserActionShowCaptivePortalClicked[];
  static const char kUserActionConnectRequested[];

  ErrorScreen(BaseScreenDelegate* base_screen_delegate, NetworkErrorView* view);
  ~ErrorScreen() override;

  // Toggles the guest sign-in prompt.
  void AllowGuestSignin(bool allowed);

  // Toggles the offline sign-in.
  void AllowOfflineLogin(bool allowed);

  // Initializes captive portal dialog and shows that if needed.
  virtual void FixCaptivePortal();

  NetworkError::UIState GetUIState() const;
  NetworkError::ErrorState GetErrorState() const;

  // Returns id of the screen behind error screen ("caller" screen).
  // Returns OobeScreen::SCREEN_UNKNOWN if error screen isn't the current
  // screen.
  OobeScreen GetParentScreen() const;

  // Called when we're asked to hide captive portal dialog.
  void HideCaptivePortal();

  // This method is called, when view is being destroyed. Note, if model
  // is destroyed earlier then it has to call Unbind().
  void OnViewDestroyed(NetworkErrorView* view);

  // Sets current UI state.
  virtual void SetUIState(NetworkError::UIState ui_state);

  // Sets current error screen content according to current UI state,
  // |error_state|, and |network|.
  virtual void SetErrorState(NetworkError::ErrorState error_state,
                             const std::string& network);

  // Sets "parent screen" i.e. one that has initiated this network error screen
  // instance.
  void SetParentScreen(OobeScreen parent_screen);

  // Sets callback that is called on hide.
  void SetHideCallback(const base::Closure& on_hide);

  // Shows captive portal dialog.
  void ShowCaptivePortal();

  // Toggles the connection pending indicator.
  void ShowConnectingIndicator(bool show);

  // Register a callback to be invoked when the user indicates that an attempt
  // to connect to the network should be made.
  ConnectRequestCallbackSubscription RegisterConnectRequestCallback(
      const base::Closure& callback);

  // BaseScreen overrides:
  void Show() override;
  void Hide() override;
  void OnShow() override;
  void OnHide() override;
  void OnUserAction(const std::string& action_id) override;

 private:
  // LoginPerformer::Delegate overrides:
  void OnAuthFailure(const AuthFailure& error) override;
  void OnAuthSuccess(const UserContext& user_context) override;
  void OnOffTheRecordAuthSuccess() override;
  void OnPasswordChangeDetected() override;
  void WhiteListCheckFailed(const std::string& email) override;
  void PolicyLoadFailed() override;
  void SetAuthFlowOffline(bool offline) override;

  // Default hide_closure for Hide().
  void DefaultHideCallback();

  // Handle user action to configure certificates.
  void OnConfigureCerts();

  // Handle user action to diagnose network configuration.
  void OnDiagnoseButtonClicked();

  // Handle user action to launch guest session from out-of-box.
  void OnLaunchOobeGuestSession();

  // Handle user action to launch Powerwash in case of
  // Local State critical error.
  void OnLocalStateErrorPowerwashButtonClicked();

  // Handle uses action to reboot device.
  void OnRebootButtonClicked();

  // The user indicated to make an attempt to connect to the network.
  void OnConnectRequested();

  // Handles the response of an ownership check and starts the guest session if
  // applicable.
  void StartGuestSessionAfterOwnershipCheck(
      DeviceSettingsService::OwnershipStatus ownership_status);

  NetworkErrorView* view_ = nullptr;

  std::unique_ptr<LoginPerformer> guest_login_performer_;

  // Proxy which manages showing of the window for captive portal entering.
  std::unique_ptr<CaptivePortalWindowProxy> captive_portal_window_proxy_;

  // Network state informer used to keep error screen up.
  scoped_refptr<NetworkStateInformer> network_state_informer_;

  NetworkError::UIState ui_state_ = NetworkError::UI_STATE_UNKNOWN;
  NetworkError::ErrorState error_state_ = NetworkError::ERROR_STATE_UNKNOWN;

  OobeScreen parent_screen_ = OobeScreen::SCREEN_UNKNOWN;

  // Optional callback that is called when NetworkError screen is hidden.
  std::unique_ptr<base::Closure> on_hide_callback_;

  // Callbacks to be invoked when a connection attempt is requested.
  base::CallbackList<void()> connect_request_callbacks_;

  base::WeakPtrFactory<ErrorScreen> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(ErrorScreen);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_SCREENS_ERROR_SCREEN_H_
