// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/webgl/WebGLTimerQueryEXT.h"

#include "core/dom/TaskRunnerHelper.h"
#include "gpu/command_buffer/client/gles2_interface.h"
#include "modules/webgl/WebGLRenderingContextBase.h"
#include "public/platform/Platform.h"

namespace blink {

WebGLTimerQueryEXT* WebGLTimerQueryEXT::Create(WebGLRenderingContextBase* ctx) {
  return new WebGLTimerQueryEXT(ctx);
}

WebGLTimerQueryEXT::WebGLTimerQueryEXT(WebGLRenderingContextBase* ctx)
    : WebGLContextObject(ctx),
      target_(0),
      query_id_(0),
      can_update_availability_(false),
      query_result_available_(false),
      query_result_(0),
      task_runner_(TaskRunnerHelper::Get(TaskType::kUnthrottled,
                                         &ctx->canvas()->GetDocument())) {
  Context()->ContextGL()->GenQueriesEXT(1, &query_id_);
}

WebGLTimerQueryEXT::~WebGLTimerQueryEXT() {
  RunDestructor();
}

void WebGLTimerQueryEXT::ResetCachedResult() {
  can_update_availability_ = false;
  query_result_available_ = false;
  query_result_ = 0;
  // When this is called, the implication is that we should start
  // keeping track of whether we can update the cached availability
  // and result.
  ScheduleAllowAvailabilityUpdate();
}

void WebGLTimerQueryEXT::UpdateCachedResult(gpu::gles2::GLES2Interface* gl) {
  if (query_result_available_)
    return;

  if (!can_update_availability_)
    return;

  if (!HasTarget())
    return;

  // If this is a timestamp query, set the result to 0 and make it available as
  // we don't support timestamps in WebGL due to very poor driver support for
  // them.
  if (target_ == GL_TIMESTAMP_EXT) {
    query_result_ = 0;
    query_result_available_ = true;
    return;
  }

  // We can only update the cached result when control returns to the browser.
  can_update_availability_ = false;
  GLuint available = 0;
  gl->GetQueryObjectuivEXT(Object(), GL_QUERY_RESULT_AVAILABLE_EXT, &available);
  query_result_available_ = !!available;
  if (query_result_available_) {
    GLuint64 result = 0;
    gl->GetQueryObjectui64vEXT(Object(), GL_QUERY_RESULT_EXT, &result);
    query_result_ = result;
    task_handle_.Cancel();
  } else {
    ScheduleAllowAvailabilityUpdate();
  }
}

bool WebGLTimerQueryEXT::IsQueryResultAvailable() {
  return query_result_available_;
}

GLuint64 WebGLTimerQueryEXT::GetQueryResult() {
  return query_result_;
}

void WebGLTimerQueryEXT::DeleteObjectImpl(gpu::gles2::GLES2Interface* gl) {
  gl->DeleteQueriesEXT(1, &query_id_);
  query_id_ = 0;
}

void WebGLTimerQueryEXT::ScheduleAllowAvailabilityUpdate() {
  if (task_handle_.IsActive())
    return;
  task_handle_ = task_runner_->PostCancellableTask(
      BLINK_FROM_HERE, WTF::Bind(&WebGLTimerQueryEXT::AllowAvailabilityUpdate,
                                 WrapWeakPersistent(this)));
}

void WebGLTimerQueryEXT::AllowAvailabilityUpdate() {
  can_update_availability_ = true;
}

}  // namespace blink
