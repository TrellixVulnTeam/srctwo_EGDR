// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_GPU_ACTION_PARSER_H_
#define CONTENT_RENDERER_GPU_ACTION_PARSER_H_

#include <cstddef>
#include <set>
#include <string>

#include "content/common/input/synthetic_pointer_action_list_params.h"

namespace base {
class DictionaryValue;
class ListValue;
}  // namespace base

namespace content {

// This class takes the arugment of json format from
// GpuBenchmarking::PointerActionSequence, parses it and warps
// it into a SyntheticPointerActionListParams object.
class ActionsParser {
 public:
  explicit ActionsParser(base::Value* value);
  ~ActionsParser();
  bool ParsePointerActionSequence();
  std::string error_message() { return error_message_; }
  const SyntheticPointerActionListParams& gesture_params() const {
    return gesture_params_;
  }

 private:
  bool ParsePointerActions(const base::DictionaryValue& pointer_actions);
  bool ParseActions(const base::ListValue& actions);
  bool ParseAction(const base::DictionaryValue& action,
                   SyntheticPointerActionListParams::ParamList& param_list);

  SyntheticPointerActionListParams gesture_params_;
  std::vector<SyntheticPointerActionListParams::ParamList>
      pointer_actions_list_;
  size_t longest_action_sequence_;
  std::string source_type_;
  std::string error_message_;

  base::Value* pointer_actions_value_;
  int action_index_;

  DISALLOW_COPY_AND_ASSIGN(ActionsParser);
};

}  // namespace content

#endif  // CONTENT_RENDERER_GPU_ACTION_PARSER_H_
