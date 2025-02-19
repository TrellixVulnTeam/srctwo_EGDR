// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media/router/media_router_dialog_controller.h"

#include <utility>

#include "chrome/browser/media/router/media_router_metrics.h"
#include "chrome/common/media_router/media_route.h"
#include "chrome/common/media_router/route_request_result.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/common/presentation_info.h"

#if defined(OS_ANDROID)
#include "chrome/browser/media/android/router/media_router_dialog_controller_android.h"
#else
#include "chrome/browser/ui/webui/media_router/media_router_dialog_controller_impl.h"
#endif

namespace media_router {

// static
MediaRouterDialogController*
MediaRouterDialogController::GetOrCreateForWebContents(
    content::WebContents* contents) {
#if defined(OS_ANDROID)
  return MediaRouterDialogControllerAndroid::GetOrCreateForWebContents(
      contents);
#else
  return MediaRouterDialogControllerImpl::GetOrCreateForWebContents(contents);
#endif
}

class MediaRouterDialogController::InitiatorWebContentsObserver
    : public content::WebContentsObserver {
 public:
  InitiatorWebContentsObserver(
      content::WebContents* web_contents,
      MediaRouterDialogController* dialog_controller)
      : content::WebContentsObserver(web_contents),
        dialog_controller_(dialog_controller) {
    DCHECK(dialog_controller_);
  }

 private:
  void WebContentsDestroyed() override {
    // NOTE: |this| is deleted after CloseMediaRouterDialog() returns.
    dialog_controller_->CloseMediaRouterDialog();
  }

  void NavigationEntryCommitted(
      const content::LoadCommittedDetails& load_details) override {
    // NOTE: |this| is deleted after CloseMediaRouterDialog() returns.
    dialog_controller_->CloseMediaRouterDialog();
  }

  void RenderProcessGone(base::TerminationStatus status) override {
    // NOTE: |this| is deleted after CloseMediaRouterDialog() returns.
    dialog_controller_->CloseMediaRouterDialog();
  }

  MediaRouterDialogController* const dialog_controller_;
};

StartPresentationContext::StartPresentationContext(
    const content::PresentationRequest& presentation_request,
    PresentationConnectionCallback success_cb,
    PresentationConnectionErrorCallback error_cb)
    : presentation_request_(presentation_request),
      success_cb_(std::move(success_cb)),
      error_cb_(std::move(error_cb)) {
  DCHECK(success_cb_);
  DCHECK(error_cb_);
}

StartPresentationContext::~StartPresentationContext() {
  if (!cb_invoked_) {
    std::move(error_cb_).Run(content::PresentationError(
        content::PRESENTATION_ERROR_UNKNOWN, "Unknown error."));
  }
}

void StartPresentationContext::InvokeSuccessCallback(
    const std::string& presentation_id,
    const GURL& presentation_url,
    const MediaRoute& route) {
  if (!cb_invoked_) {
    std::move(success_cb_)
        .Run(content::PresentationInfo(presentation_url, presentation_id),
             route);
    cb_invoked_ = true;
  }
}

void StartPresentationContext::InvokeErrorCallback(
    const content::PresentationError& error) {
  if (!cb_invoked_) {
    std::move(error_cb_).Run(error);
    cb_invoked_ = true;
  }
}

// static
void StartPresentationContext::HandleRouteResponse(
    std::unique_ptr<StartPresentationContext> context,
    const RouteRequestResult& result) {
  if (!result.route()) {
    context->InvokeErrorCallback(content::PresentationError(
        content::PRESENTATION_ERROR_UNKNOWN, result.error()));
  } else {
    context->InvokeSuccessCallback(result.presentation_id(),
                                   result.presentation_url(), *result.route());
  }
}

MediaRouterDialogController::MediaRouterDialogController(
    content::WebContents* initiator)
    : initiator_(initiator) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK(initiator_);
}

MediaRouterDialogController::~MediaRouterDialogController() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

bool MediaRouterDialogController::ShowMediaRouterDialogForPresentation(
    const content::PresentationRequest& presentation_request,
    StartPresentationContext::PresentationConnectionCallback success_cb,
    StartPresentationContext::PresentationConnectionErrorCallback error_cb) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  if (IsShowingMediaRouterDialog()) {
    std::move(error_cb).Run(content::PresentationError(
        content::PRESENTATION_ERROR_UNKNOWN,
        "Unable to create dialog: dialog already shown"));
    return false;
  }

  start_presentation_context_ = base::MakeUnique<StartPresentationContext>(
      presentation_request, std::move(success_cb), std::move(error_cb));
  MediaRouterMetrics::RecordMediaRouterDialogOrigin(
      MediaRouterDialogOpenOrigin::PAGE);
  FocusOnMediaRouterDialog(true);
  return true;
}

bool MediaRouterDialogController::ShowMediaRouterDialog() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  bool dialog_needs_creation = !IsShowingMediaRouterDialog();
  FocusOnMediaRouterDialog(dialog_needs_creation);
  return dialog_needs_creation;
}

void MediaRouterDialogController::HideMediaRouterDialog() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  CloseMediaRouterDialog();
  Reset();
}

void MediaRouterDialogController::FocusOnMediaRouterDialog(
    bool dialog_needs_creation) {
  if (dialog_needs_creation) {
    initiator_observer_.reset(
        new InitiatorWebContentsObserver(initiator_, this));
    CreateMediaRouterDialog();
  }
  initiator_->GetDelegate()->ActivateContents(initiator_);
}

void MediaRouterDialogController::Reset() {
  initiator_observer_.reset();
  start_presentation_context_.reset();
}

}  // namespace media_router
