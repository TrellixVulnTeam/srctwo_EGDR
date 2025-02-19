// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/test_media_client.h"

#include "mojo/public/cpp/bindings/associated_interface_ptr.h"

namespace ash {

TestMediaClient::TestMediaClient() : binding_(this) {}

TestMediaClient::~TestMediaClient() = default;

mojom::MediaClientAssociatedPtrInfo TestMediaClient::CreateAssociatedPtrInfo() {
  mojom::MediaClientAssociatedPtr ptr;
  binding_.Bind(mojo::MakeIsolatedRequest(&ptr));
  return ptr.PassInterface();
}

void TestMediaClient::HandleMediaNextTrack() {}

void TestMediaClient::HandleMediaPlayPause() {}

void TestMediaClient::HandleMediaPrevTrack() {}

void TestMediaClient::RequestCaptureState() {}

void TestMediaClient::SuspendMediaSessions() {
  media_sessions_suspended_ = true;
}

}  // namespace ash
