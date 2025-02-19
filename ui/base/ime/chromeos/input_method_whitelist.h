// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_IME_CHROMEOS_INPUT_METHOD_WHITELIST_H_
#define UI_BASE_IME_CHROMEOS_INPUT_METHOD_WHITELIST_H_

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/macros.h"
#include "ui/base/ime/ui_base_ime_export.h"

namespace chromeos {
namespace input_method {

class InputMethodDescriptor;
typedef std::vector<InputMethodDescriptor> InputMethodDescriptors;

// A class which has white listed input method list. The list is generated by
// gen_input_methods.py from input_methods.txt.
class UI_BASE_IME_EXPORT InputMethodWhitelist {
 public:
  InputMethodWhitelist();
  ~InputMethodWhitelist();

  // Returns true if |input_method_id| is whitelisted.
  bool InputMethodIdIsWhitelisted(const std::string& input_method_id) const;

  // Returns all input methods that are supported, including ones not active.
  // This function never returns NULL. Note that input method extensions are not
  // included in the result.
  std::unique_ptr<InputMethodDescriptors> GetSupportedInputMethods() const;

 private:
  std::set<std::string> supported_input_methods_;

  DISALLOW_COPY_AND_ASSIGN(InputMethodWhitelist);
};

}  // namespace input_method
}  // namespace chromeos

#endif  // UI_BASE_IME_CHROMEOS_INPUT_METHOD_WHITELIST_H_
