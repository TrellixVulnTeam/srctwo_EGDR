// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media/router/offscreen_presentation_manager.h"

#include <utility>

#include "base/memory/ptr_util.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "url/gurl.h"

namespace media_router {

// OffscreenPresentationManager implementation.
OffscreenPresentationManager::OffscreenPresentationManager() {}

OffscreenPresentationManager::~OffscreenPresentationManager() {}

OffscreenPresentationManager::OffscreenPresentation*
OffscreenPresentationManager::GetOrCreateOffscreenPresentation(
    const content::PresentationInfo& presentation_info) {
  auto it = offscreen_presentations_.find(presentation_info.presentation_id);
  if (it == offscreen_presentations_.end()) {
    it = offscreen_presentations_
             .insert(std::make_pair(
                 presentation_info.presentation_id,
                 base::MakeUnique<OffscreenPresentation>(presentation_info)))
             .first;
  }
  return it->second.get();
}

void OffscreenPresentationManager::RegisterOffscreenPresentationController(
    const content::PresentationInfo& presentation_info,
    const RenderFrameHostId& render_frame_host_id,
    content::PresentationConnectionPtr controller_connection_ptr,
    content::PresentationConnectionRequest receiver_connection_request,
    const MediaRoute& route) {
  DVLOG(2) << __func__
           << " [presentation_id]: " << presentation_info.presentation_id
           << ", [render_frame_host_id]: " << render_frame_host_id.second;
  DCHECK(thread_checker_.CalledOnValidThread());

  auto* presentation = GetOrCreateOffscreenPresentation(presentation_info);
  presentation->RegisterController(
      render_frame_host_id, std::move(controller_connection_ptr),
      std::move(receiver_connection_request), route);
}

void OffscreenPresentationManager::UnregisterOffscreenPresentationController(
    const std::string& presentation_id,
    const RenderFrameHostId& render_frame_host_id) {
  DVLOG(2) << __func__ << " [presentation_id]: " << presentation_id
           << ", [render_frame_host_id]: " << render_frame_host_id.second;
  DCHECK(thread_checker_.CalledOnValidThread());

  auto it = offscreen_presentations_.find(presentation_id);
  if (it == offscreen_presentations_.end())
    return;

  // Remove presentation if no controller and receiver.
  it->second->UnregisterController(render_frame_host_id);
  if (!it->second->IsValid()) {
    DLOG(WARNING) << __func__ << " no receiver callback has been registered to "
                  << "[presentation_id]: " << presentation_id;
    offscreen_presentations_.erase(presentation_id);
  }
}

void OffscreenPresentationManager::OnOffscreenPresentationReceiverCreated(
    const content::PresentationInfo& presentation_info,
    const content::ReceiverConnectionAvailableCallback& receiver_callback) {
  DVLOG(2) << __func__
           << " [presentation_id]: " << presentation_info.presentation_id;
  DCHECK(thread_checker_.CalledOnValidThread());
  auto* presentation = GetOrCreateOffscreenPresentation(presentation_info);
  presentation->RegisterReceiver(receiver_callback);
}

void OffscreenPresentationManager::OnOffscreenPresentationReceiverTerminated(
    const std::string& presentation_id) {
  DVLOG(2) << __func__ << " [presentation_id]: " << presentation_id;
  DCHECK(thread_checker_.CalledOnValidThread());

  offscreen_presentations_.erase(presentation_id);
}

bool OffscreenPresentationManager::IsOffscreenPresentation(
    const std::string& presentation_id) {
  return base::ContainsKey(offscreen_presentations_, presentation_id);
}

const MediaRoute* OffscreenPresentationManager::GetRoute(
    const std::string& presentation_id) {
  auto it = offscreen_presentations_.find(presentation_id);
  return (it != offscreen_presentations_.end() &&
          it->second->route_.has_value())
             ? &(it->second->route_.value())
             : nullptr;
}

// OffscreenPresentation implementation.
OffscreenPresentationManager::OffscreenPresentation::OffscreenPresentation(
    const content::PresentationInfo& presentation_info)
    : presentation_info_(presentation_info) {}

OffscreenPresentationManager::OffscreenPresentation::~OffscreenPresentation() {}

void OffscreenPresentationManager::OffscreenPresentation::RegisterController(
    const RenderFrameHostId& render_frame_host_id,
    content::PresentationConnectionPtr controller_connection_ptr,
    content::PresentationConnectionRequest receiver_connection_request,
    const MediaRoute& route) {
  if (!receiver_callback_.is_null()) {
    receiver_callback_.Run(presentation_info_,
                           std::move(controller_connection_ptr),
                           std::move(receiver_connection_request));
  } else {
    pending_controllers_.insert(std::make_pair(
        render_frame_host_id, base::MakeUnique<ControllerConnection>(
                                  std::move(controller_connection_ptr),
                                  std::move(receiver_connection_request))));
  }

  route_ = route;
}

void OffscreenPresentationManager::OffscreenPresentation::UnregisterController(
    const RenderFrameHostId& render_frame_host_id) {
  pending_controllers_.erase(render_frame_host_id);
}

void OffscreenPresentationManager::OffscreenPresentation::RegisterReceiver(
    const content::ReceiverConnectionAvailableCallback& receiver_callback) {
  DCHECK(receiver_callback_.is_null());

  for (auto& controller : pending_controllers_) {
    receiver_callback.Run(
        presentation_info_,
        std::move(controller.second->controller_connection_ptr),
        std::move(controller.second->receiver_connection_request));
  }
  receiver_callback_ = receiver_callback;
  pending_controllers_.clear();
}

bool OffscreenPresentationManager::OffscreenPresentation::IsValid() const {
  return !(pending_controllers_.empty() && receiver_callback_.is_null());
}

OffscreenPresentationManager::OffscreenPresentation::ControllerConnection::
    ControllerConnection(
        content::PresentationConnectionPtr controller_connection_ptr,
        content::PresentationConnectionRequest receiver_connection_request)
    : controller_connection_ptr(std::move(controller_connection_ptr)),
      receiver_connection_request(std::move(receiver_connection_request)) {}

OffscreenPresentationManager::OffscreenPresentation::ControllerConnection::
    ~ControllerConnection() {}

}  // namespace media_router
