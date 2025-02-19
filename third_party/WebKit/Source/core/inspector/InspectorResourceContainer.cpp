// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/inspector/InspectorResourceContainer.h"

#include "core/inspector/InspectedFrames.h"

namespace blink {

InspectorResourceContainer::InspectorResourceContainer(
    InspectedFrames* inspected_frames)
    : inspected_frames_(inspected_frames) {}

InspectorResourceContainer::~InspectorResourceContainer() {}

DEFINE_TRACE(InspectorResourceContainer) {
  visitor->Trace(inspected_frames_);
}

void InspectorResourceContainer::DidCommitLoadForLocalFrame(LocalFrame* frame) {
  if (frame != inspected_frames_->Root())
    return;
  style_sheet_contents_.clear();
  style_element_contents_.clear();
}

void InspectorResourceContainer::StoreStyleSheetContent(const String& url,
                                                        const String& content) {
  style_sheet_contents_.Set(url, content);
}

bool InspectorResourceContainer::LoadStyleSheetContent(const String& url,
                                                       String* content) {
  if (!style_sheet_contents_.Contains(url))
    return false;
  *content = style_sheet_contents_.at(url);
  return true;
}

void InspectorResourceContainer::StoreStyleElementContent(
    int backend_node_id,
    const String& content) {
  style_element_contents_.Set(backend_node_id, content);
}

bool InspectorResourceContainer::LoadStyleElementContent(int backend_node_id,
                                                         String* content) {
  if (!style_element_contents_.Contains(backend_node_id))
    return false;
  *content = style_element_contents_.at(backend_node_id);
  return true;
}

}  // namespace blink
