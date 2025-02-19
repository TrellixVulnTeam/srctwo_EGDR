// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_CLIENT_INPUT_KEY_EVENT_MAPPER_H_
#define REMOTING_CLIENT_INPUT_KEY_EVENT_MAPPER_H_

#include <stdint.h>

#include <map>
#include <set>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "remoting/protocol/input_filter.h"

namespace remoting {

// Filtering InputStub which can be used to re-map the USB keycodes of events
// before they are passed on to the next InputStub in the chain, or to trap
// events with specific USB keycodes for special handling.
class KeyEventMapper : public protocol::InputFilter {
 public:
  KeyEventMapper();
  explicit KeyEventMapper(InputStub* input_stub);
  ~KeyEventMapper() override;

  // Callback type for use with SetTrapCallback(), below.
  typedef base::Callback<void(const protocol::KeyEvent&)> KeyTrapCallback;

  // Sets the callback to which trapped keys will be delivered.
  void SetTrapCallback(KeyTrapCallback callback);

  // Causes events matching |usb_keycode| to be delivered to the trap callback.
  // Trapped events are not dispatched to the next InputStub in the chain.
  void TrapKey(uint32_t usb_keycode, bool trap_key);

  // Causes events matching |in_usb_keycode| to be mapped to |out_usb_keycode|.
  // Keys are remapped at most once. Traps are processed before remapping.
  void RemapKey(uint32_t in_usb_keycode, uint32_t out_usb_keycode);

  // InputFilter overrides.
  void InjectKeyEvent(const protocol::KeyEvent& event) override;

 private:
  std::map<uint32_t, uint32_t> mapped_keys;
  std::set<uint32_t> trapped_keys;
  KeyTrapCallback trap_callback;

  DISALLOW_COPY_AND_ASSIGN(KeyEventMapper);
};

}  // namespace remoting

#endif  // REMOTING_CLIENT_INPUT_KEY_EVENT_MAPPER_H_
