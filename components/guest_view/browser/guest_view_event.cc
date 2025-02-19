// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/guest_view/browser/guest_view_event.h"

#include <utility>

#include "base/logging.h"
#include "components/guest_view/browser/guest_view_base.h"
#include "components/guest_view/browser/guest_view_manager.h"

namespace guest_view {

GuestViewEvent::GuestViewEvent(const std::string& name,
                               std::unique_ptr<base::DictionaryValue> args)
    : name_(name), args_(std::move(args)) {}

GuestViewEvent::~GuestViewEvent() {
}

void GuestViewEvent::Dispatch(GuestViewBase* guest, int instance_id) {
  DCHECK(args_) << "Dispatch was probably invoked twice!";
  GuestViewManager::FromBrowserContext(guest->browser_context())
      ->DispatchEvent(name_, std::move(args_), guest, instance_id);
}

}  // namespace guest_view
