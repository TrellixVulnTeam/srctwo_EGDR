// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/cast/test/utility/standalone_cast_environment.h"

#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread_restrictions.h"
#include "base/time/default_tick_clock.h"

namespace media {
namespace cast {

StandaloneCastEnvironment::StandaloneCastEnvironment()
    : CastEnvironment(
          base::WrapUnique<base::TickClock>(new base::DefaultTickClock()),
          NULL,
          NULL,
          NULL),
      main_thread_("StandaloneCastEnvironment Main"),
      audio_thread_("StandaloneCastEnvironment Audio"),
      video_thread_("StandaloneCastEnvironment Video") {
#define CREATE_TASK_RUNNER(name, options)   \
  name##_thread_.StartWithOptions(options); \
  CastEnvironment::name##_thread_proxy_ = name##_thread_.task_runner()

  CREATE_TASK_RUNNER(main,
                     base::Thread::Options(base::MessageLoop::TYPE_IO, 0));
  CREATE_TASK_RUNNER(audio, base::Thread::Options());
  CREATE_TASK_RUNNER(video, base::Thread::Options());
#undef CREATE_TASK_RUNNER
}

StandaloneCastEnvironment::~StandaloneCastEnvironment() {
  CHECK(CalledOnValidThread());
  CHECK(!main_thread_.IsRunning());
  CHECK(!audio_thread_.IsRunning());
  CHECK(!video_thread_.IsRunning());
}

void StandaloneCastEnvironment::Shutdown() {
  CHECK(CalledOnValidThread());

  base::ThreadRestrictions::ScopedAllowIO
      because_i_brought_you_into_this_world_and_i_am_gonna_take_you_out;
  main_thread_.Stop();
  audio_thread_.Stop();
  video_thread_.Stop();
}

}  // namespace cast
}  // namespace media
