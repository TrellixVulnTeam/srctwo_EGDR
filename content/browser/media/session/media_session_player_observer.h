// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_MEDIA_SESSION_MEDIA_SESSION_PLAYER_OBSERVER_H_
#define CONTENT_BROWSER_MEDIA_SESSION_MEDIA_SESSION_PLAYER_OBSERVER_H_

namespace content {

class RenderFrameHost;

class MediaSessionPlayerObserver {
 public:
  MediaSessionPlayerObserver() = default;
  virtual ~MediaSessionPlayerObserver() = default;

  // The given |player_id| has been suspended by the MediaSession.
  virtual void OnSuspend(int player_id) = 0;

  // The given |player_id| has been resumed by the MediaSession.
  virtual void OnResume(int player_id) = 0;

  // The given |player_id| has been set a new volume multiplier by
  // the MediaSession.
  virtual void OnSetVolumeMultiplier(int player_id,
                                     double volume_multiplier) = 0;

  // Returns the RenderFrameHost this player observer belongs to. Returns
  // nullptr if unavailable.
  virtual RenderFrameHost* render_frame_host() const = 0;
};

}  // namespace content

#endif  // CONTENT_BROWSER_MEDIA_SESSION_MEDIA_SESSION_PLAYER_OBSERVER_H_
