// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/mojo/clients/mojo_media_log_service.h"

#include <memory>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "media/base/media_log_event.h"

namespace media {

MojoMediaLogService::MojoMediaLogService(media::MediaLog* media_log)
    : media_log_(media_log) {
  DVLOG(1) << __func__;
  DCHECK(media_log_);
}

MojoMediaLogService::~MojoMediaLogService() {
  DVLOG(1) << __func__;
}

void MojoMediaLogService::AddEvent(const media::MediaLogEvent& event) {
  DVLOG(1) << __func__;

  // Make a copy so that we can transfer ownership to |media_log_|.
  std::unique_ptr<media::MediaLogEvent> modified_event =
      base::MakeUnique<media::MediaLogEvent>(event);

  // |id| is player-unique per-process, but the remote side does not know the
  // correct value (nor would we necessarily trust it). Overwrite with the
  // correct value.
  modified_event->id = media_log_->id();

  media_log_->AddEvent(std::move(modified_event));
}

}  // namespace media
