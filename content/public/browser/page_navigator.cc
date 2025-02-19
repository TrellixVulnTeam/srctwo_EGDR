// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/browser/page_navigator.h"

namespace content {

OpenURLParams::OpenURLParams(const GURL& url,
                             const Referrer& referrer,
                             WindowOpenDisposition disposition,
                             ui::PageTransition transition,
                             bool is_renderer_initiated)
    : url(url),
      referrer(referrer),
      uses_post(false),
      frame_tree_node_id(-1),
      disposition(disposition),
      force_new_process_for_new_contents(false),
      transition(transition),
      is_renderer_initiated(is_renderer_initiated),
      should_replace_current_entry(false),
      user_gesture(!is_renderer_initiated),
      started_from_context_menu(false) {}

OpenURLParams::OpenURLParams(const GURL& url,
                             const Referrer& referrer,
                             WindowOpenDisposition disposition,
                             ui::PageTransition transition,
                             bool is_renderer_initiated,
                             bool started_from_context_menu)
    : url(url),
      referrer(referrer),
      uses_post(false),
      frame_tree_node_id(-1),
      disposition(disposition),
      force_new_process_for_new_contents(false),
      transition(transition),
      is_renderer_initiated(is_renderer_initiated),
      should_replace_current_entry(false),
      user_gesture(!is_renderer_initiated),
      started_from_context_menu(started_from_context_menu) {}

OpenURLParams::OpenURLParams(const GURL& url,
                             const Referrer& referrer,
                             int frame_tree_node_id,
                             WindowOpenDisposition disposition,
                             ui::PageTransition transition,
                             bool is_renderer_initiated)
    : url(url),
      referrer(referrer),
      uses_post(false),
      frame_tree_node_id(frame_tree_node_id),
      disposition(disposition),
      force_new_process_for_new_contents(false),
      transition(transition),
      is_renderer_initiated(is_renderer_initiated),
      should_replace_current_entry(false),
      user_gesture(!is_renderer_initiated),
      started_from_context_menu(false) {}

OpenURLParams::OpenURLParams()
    : uses_post(false),
      frame_tree_node_id(-1),
      disposition(WindowOpenDisposition::UNKNOWN),
      force_new_process_for_new_contents(false),
      transition(ui::PAGE_TRANSITION_LINK),
      is_renderer_initiated(false),
      should_replace_current_entry(false),
      user_gesture(true),
      started_from_context_menu(false) {}

OpenURLParams::OpenURLParams(const OpenURLParams& other) = default;

OpenURLParams::~OpenURLParams() {
}

}  // namespace content
