// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_ARC_VOICE_INTERACTION_ARC_VOICE_INTERACTION_FRAMEWORK_SERVICE_H_
#define CHROME_BROWSER_CHROMEOS_ARC_VOICE_INTERACTION_ARC_VOICE_INTERACTION_FRAMEWORK_SERVICE_H_

#include <memory>

#include "base/macros.h"
#include "base/time/time.h"
#include "chrome/browser/chromeos/arc/arc_session_manager.h"
#include "components/arc/common/voice_interaction_framework.mojom.h"
#include "components/arc/instance_holder.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/session_manager/core/session_manager_observer.h"
#include "mojo/public/cpp/bindings/binding.h"

class KeyedServiceBaseFactory;

namespace content {
class BrowserContext;
}  // namespace content

namespace gfx {
class Rect;
}  // namespace gfx

namespace arc {

class ArcBridgeService;

// This provides voice interaction context (currently screenshots)
// to ARC to be used by VoiceInteractionSession. This class lives on the UI
// thread.
class ArcVoiceInteractionFrameworkService
    : public KeyedService,
      public mojom::VoiceInteractionFrameworkHost,
      public InstanceHolder<mojom::VoiceInteractionFrameworkInstance>::Observer,
      public ArcSessionManager::Observer,
      public session_manager::SessionManagerObserver {
 public:
  // Returns singleton instance for the given BrowserContext,
  // or nullptr if the browser |context| is not allowed to use ARC.
  static ArcVoiceInteractionFrameworkService* GetForBrowserContext(
      content::BrowserContext* context);

  // Returns factory for ArcVoiceInteractionFrameworkService.
  static KeyedServiceBaseFactory* GetFactory();

  ArcVoiceInteractionFrameworkService(content::BrowserContext* context,
                                      ArcBridgeService* bridge_service);
  ~ArcVoiceInteractionFrameworkService() override;

  // InstanceHolder<mojom::VoiceInteractionFrameworkInstance> overrides.
  void OnInstanceReady() override;
  void OnInstanceClosed() override;

  // mojom::VoiceInteractionFrameworkHost overrides.
  void CaptureFocusedWindow(
      const CaptureFocusedWindowCallback& callback) override;
  void CaptureFullscreen(const CaptureFullscreenCallback& callback) override;
  // TODO(kaznacheev) remove usages of this obsolete method from the container.
  void OnMetalayerClosed() override;
  void SetMetalayerEnabled(bool enabled) override;
  void SetVoiceInteractionRunning(bool running) override;
  void SetVoiceInteractionState(ash::VoiceInteractionState state) override;

  void ShowMetalayer();
  void HideMetalayer();

  // ArcSessionManager::Observer overrides.
  void OnArcPlayStoreEnabledChanged(bool enabled) override;

  // session_manager::SessionManagerObserver overrides.
  void OnSessionStateChanged() override;

  // Starts a voice interaction session after user-initiated interaction.
  // Records a timestamp and sets number of allowed requests to 2 since by
  // design, there will be one request for screenshot and the other for
  // voice interaction context.
  // |region| refers to the selected region on the screen to be passed to
  // VoiceInteractionFrameworkInstance::StartVoiceInteractionSessionForRegion().
  // If |region| is empty,
  // VoiceInteractionFrameworkInstance::StartVoiceInteraction() is called.
  void StartSessionFromUserInteraction(const gfx::Rect& region);

  // Similar to StartSessionFromUserInteraction but stops voice interaction
  // seesion if it is already running.
  void ToggleSessionFromUserInteraction();

  // Turn on / off voice interaction in ARC.
  // TODO(muyuanli): We should also check on Chrome side once CrOS side settings
  // are ready (tracked separately at crbug.com/727873).
  void SetVoiceInteractionEnabled(bool enable);

  // Turn on / off voice interaction context (screenshot and structural data)
  // in ARC.
  void SetVoiceInteractionContextEnabled(bool enable);

  // Checks whether the caller is called within the time limit since last user
  // initiated interaction. Logs UMA metric when it's not.
  bool ValidateTimeSinceUserInteraction();

  // Start the voice interaction setup wizard in container.
  void StartVoiceInteractionSetupWizard();

  // Show the voice interaction settings in container.
  void ShowVoiceInteractionSettings();

  // Updates voice interaction flags. These flags are set only once when ARC
  // container is enabled.
  void UpdateVoiceInteractionPrefs();

  // Set voice interaction setup completed flag and notify the change.
  void SetVoiceInteractionSetupCompleted();

  // For supporting ArcServiceManager::GetService<T>().
  static const char kArcServiceName[];

 private:
  void NotifyMetalayerStatusChanged(bool visible);

  bool InitiateUserInteraction();

  content::BrowserContext* context_;
  ArcBridgeService* const arc_bridge_service_;  // Owned by ArcServiceManager
  mojo::Binding<mojom::VoiceInteractionFrameworkHost> binding_;

  // Whether there is a pending request to start voice interaction.
  bool is_request_pending_ = false;

  // Whether we should launch runtime setup flow for voice interaction.
  bool should_start_runtime_flow_ = false;

  // The current state voice interaction service is. There is usually a long
  // delay after boot before the service is ready. We wait for the container
  // to tell us if it is ready to quickly serve voice interaction requests.
  // We also give user proper feedback based on the state.
  ash::VoiceInteractionState state_ = ash::VoiceInteractionState::NOT_READY;

  // The time when a user initated an interaction.
  base::TimeTicks user_interaction_start_time_;

  // The number of allowed requests from container. Maximum is 2 (1 for
  // screenshot and 1 for view hierarchy). This amount decreases after each
  // context request or resets when allowed time frame is elapsed.  When this
  // quota is 0, but we still get requests from the container side, we assume
  // something malicious is going on.
  int32_t context_request_remaining_count_ = 0;

  DISALLOW_COPY_AND_ASSIGN(ArcVoiceInteractionFrameworkService);
};

}  // namespace arc
#endif  // CHROME_BROWSER_CHROMEOS_ARC_VOICE_INTERACTION_ARC_VOICE_INTERACTION_FRAMEWORK_SERVICE_H_
