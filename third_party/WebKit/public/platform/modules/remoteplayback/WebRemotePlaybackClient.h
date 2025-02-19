// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebRemotePlaybackClient_h
#define WebRemotePlaybackClient_h

namespace blink {

enum class WebRemotePlaybackAvailability;
enum class WebRemotePlaybackState;
class WebURL;

// The interface between the HTMLMediaElement and its
// HTMLMediaElementRemotePlayback supplement.
class WebRemotePlaybackClient {
 public:
  virtual ~WebRemotePlaybackClient() = default;

  // Notifies the client that the media element state has changed.
  virtual void StateChanged(WebRemotePlaybackState) = 0;

  // Notifies the client of the remote playback device availability change.
  virtual void AvailabilityChanged(WebRemotePlaybackAvailability) = 0;

  // Notifies the client that the user cancelled the prompt shown via the API.
  virtual void PromptCancelled() = 0;

  // Returns if the remote playback available for this media element.
  virtual bool RemotePlaybackAvailable() const = 0;

  // Notifies the client that the source of the HTMLMediaElement has changed as
  // well as if the new source is supported for remote playback.
  virtual void SourceChanged(const WebURL&, bool is_source_supported) = 0;
};

}  // namespace blink

#endif  // WebRemotePlaybackState_h
