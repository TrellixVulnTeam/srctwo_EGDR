// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BLINK_WEBMEDIAPLAYER_UTIL_H_
#define MEDIA_BLINK_WEBMEDIAPLAYER_UTIL_H_

#include "base/memory/ref_counted.h"
#include "base/time/time.h"
#include "media/base/audio_renderer_sink.h"
#include "media/base/eme_constants.h"
#include "media/base/pipeline_status.h"
#include "media/base/ranges.h"
#include "media/blink/media_blink_export.h"
#include "third_party/WebKit/public/platform/WebEncryptedMediaTypes.h"
#include "third_party/WebKit/public/platform/WebMediaPlayer.h"
#include "third_party/WebKit/public/platform/WebSecurityOrigin.h"
#include "third_party/WebKit/public/platform/WebSetSinkIdCallbacks.h"
#include "third_party/WebKit/public/platform/WebTimeRange.h"
#include "url/gurl.h"

namespace media {

class MediaLog;

blink::WebTimeRanges MEDIA_BLINK_EXPORT
ConvertToWebTimeRanges(const Ranges<base::TimeDelta>& ranges);

blink::WebMediaPlayer::NetworkState MEDIA_BLINK_EXPORT
PipelineErrorToNetworkState(PipelineStatus error);

// Report various metrics to UMA and RAPPOR.
void MEDIA_BLINK_EXPORT
ReportMetrics(blink::WebMediaPlayer::LoadType load_type,
              const GURL& url,
              const blink::WebSecurityOrigin& security_origin,
              MediaLog* media_log);

// Report metrics about pipeline errors.
void MEDIA_BLINK_EXPORT
ReportPipelineError(blink::WebMediaPlayer::LoadType load_type,
                    PipelineStatus error,
                    MediaLog* media_log);

// TODO(ddorwin): Move this function to an EME-specific file.
// We may also want to move the next one and pass Blink types to WMPI.
EmeInitDataType MEDIA_BLINK_EXPORT
ConvertToEmeInitDataType(blink::WebEncryptedMediaInitDataType init_data_type);

blink::WebEncryptedMediaInitDataType MEDIA_BLINK_EXPORT
ConvertToWebInitDataType(EmeInitDataType init_data_type);

// Wraps a blink::WebSetSinkIdCallbacks into a media::OutputDeviceStatusCB
// and binds it to the current thread
OutputDeviceStatusCB MEDIA_BLINK_EXPORT
ConvertToOutputDeviceStatusCB(blink::WebSetSinkIdCallbacks* web_callbacks);

}  // namespace media

#endif  // MEDIA_BLINK_WEBMEDIAPLAYER_UTIL_H_
