// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/input_injector.h"

#include <stdint.h>
#include <windows.h>

#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/windows_version.h"
#include "remoting/base/util.h"
#include "remoting/host/clipboard.h"
#include "remoting/host/touch_injector_win.h"
#include "remoting/proto/event.pb.h"
#include "ui/events/keycodes/dom/keycode_converter.h"

namespace remoting {

namespace {

using protocol::ClipboardEvent;
using protocol::KeyEvent;
using protocol::TextEvent;
using protocol::MouseEvent;
using protocol::TouchEvent;

// Helper used to call SendInput() API.
void SendKeyboardInput(uint32_t flags, uint16_t scancode) {
  // Populate a Windows INPUT structure for the event.
  INPUT input;
  memset(&input, 0, sizeof(input));
  input.type = INPUT_KEYBOARD;
  input.ki.time = 0;
  input.ki.dwFlags = flags;
  input.ki.wScan = scancode;

  if ((flags & KEYEVENTF_UNICODE) == 0) {
    // Windows scancodes are only 8-bit, so store the low-order byte into the
    // event and set the extended flag if any high-order bits are set. The only
    // high-order values we should see are 0xE0 or 0xE1. The extended bit
    // usually distinguishes keys with the same meaning, e.g. left & right
    // shift.
    input.ki.wScan &= 0xFF;
    if ((scancode & 0xFF00) != 0x0000)
      input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
  }

  if (SendInput(1, &input, sizeof(INPUT)) == 0)
    PLOG(ERROR) << "Failed to inject a key event";
}

// Parse move related operations from the input MouseEvent, and insert the
// result into output.
void ParseMouseMoveEvent(const MouseEvent& event, std::vector<INPUT>* output) {
  INPUT input = {0};
  input.type = INPUT_MOUSE;

  if (event.has_delta_x() && event.has_delta_y()) {
    input.mi.dx = event.delta_x();
    input.mi.dy = event.delta_y();
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK;
  } else if (event.has_x() && event.has_y()) {
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (width > 1 && height > 1) {
      int x = std::max(0, std::min(width, event.x()));
      int y = std::max(0, std::min(height, event.y()));
      input.mi.dx = static_cast<int>((x * 65535) / (width - 1));
      input.mi.dy = static_cast<int>((y * 65535) / (height - 1));
      input.mi.dwFlags =
          MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;
    }
  } else {
    return;
  }

  output->push_back(std::move(input));
}

// Parse click related operations from the input MouseEvent, and insert the
// result into output.
void ParseMouseClickEvent(const MouseEvent& event, std::vector<INPUT>* output) {
  if (event.has_button() && event.has_button_down()) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;

    MouseEvent::MouseButton button = event.button();
    bool down = event.button_down();

    // If the host is configured to swap left & right buttons, inject swapped
    // events to un-do that re-mapping.
    if (GetSystemMetrics(SM_SWAPBUTTON)) {
      if (button == MouseEvent::BUTTON_LEFT) {
        button = MouseEvent::BUTTON_RIGHT;
      } else if (button == MouseEvent::BUTTON_RIGHT) {
        button = MouseEvent::BUTTON_LEFT;
      }
    }

    if (button == MouseEvent::BUTTON_MIDDLE) {
      input.mi.dwFlags = down ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
    } else if (button == MouseEvent::BUTTON_RIGHT) {
      input.mi.dwFlags = down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
    } else {
      input.mi.dwFlags = down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
    }

    output->push_back(std::move(input));
  }
}

// Parse wheel related operations from the input MouseEvent, and insert the
// result into output.
void ParseMouseWheelEvent(const MouseEvent& event, std::vector<INPUT>* output) {
  if (event.has_wheel_delta_x()) {
    int delta = static_cast<int>(event.wheel_delta_x());
    if (delta != 0) {
      INPUT input = {0};
      input.type = INPUT_MOUSE;
      input.mi.mouseData = delta;
      // According to MSDN, MOUSEEVENTF_HWHELL and MOUSEEVENTF_WHEEL are both
      // required for a horizontal wheel event.
      input.mi.dwFlags = MOUSEEVENTF_HWHEEL | MOUSEEVENTF_WHEEL;
      output->push_back(std::move(input));
    }
  }

  if (event.has_wheel_delta_y()) {
    int delta = static_cast<int>(event.wheel_delta_y());
    if (delta != 0) {
      INPUT input = {0};
      input.type = INPUT_MOUSE;
      input.mi.mouseData = delta;
      input.mi.dwFlags = MOUSEEVENTF_WHEEL;
      output->push_back(std::move(input));
    }
  }
}

// A class to generate events on Windows.
class InputInjectorWin : public InputInjector {
 public:
  InputInjectorWin(scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
                   scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner);
  ~InputInjectorWin() override;

  // ClipboardStub interface.
  void InjectClipboardEvent(const ClipboardEvent& event) override;

  // InputStub interface.
  void InjectKeyEvent(const KeyEvent& event) override;
  void InjectTextEvent(const TextEvent& event) override;
  void InjectMouseEvent(const MouseEvent& event) override;
  void InjectTouchEvent(const TouchEvent& event) override;

  // InputInjector interface.
  void Start(
      std::unique_ptr<protocol::ClipboardStub> client_clipboard) override;

 private:
  // The actual implementation resides in InputInjectorWin::Core class.
  class Core : public base::RefCountedThreadSafe<Core> {
   public:
    Core(scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
         scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner);

    // Mirrors the ClipboardStub interface.
    void InjectClipboardEvent(const ClipboardEvent& event);

    // Mirrors the InputStub interface.
    void InjectKeyEvent(const KeyEvent& event);
    void InjectTextEvent(const TextEvent& event);
    void InjectMouseEvent(const MouseEvent& event);
    void InjectTouchEvent(const TouchEvent& event);

    // Mirrors the InputInjector interface.
    void Start(std::unique_ptr<protocol::ClipboardStub> client_clipboard);

    void Stop();

   private:
    friend class base::RefCountedThreadSafe<Core>;
    virtual ~Core();

    void HandleKey(const KeyEvent& event);
    void HandleText(const TextEvent& event);
    void HandleMouse(const MouseEvent& event);
    void HandleTouch(const TouchEvent& event);

    // Check if the given scan code is caps lock or num lock.
    bool IsLockKey(int scancode);

    // Sets the keyboard lock states to those provided.
    void SetLockStates(uint32_t states);

    scoped_refptr<base::SingleThreadTaskRunner> main_task_runner_;
    scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner_;
    std::unique_ptr<Clipboard> clipboard_;
    std::unique_ptr<TouchInjectorWin> touch_injector_;

    DISALLOW_COPY_AND_ASSIGN(Core);
  };

  scoped_refptr<Core> core_;

  DISALLOW_COPY_AND_ASSIGN(InputInjectorWin);
};

InputInjectorWin::InputInjectorWin(
    scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner) {
  core_ = new Core(main_task_runner, ui_task_runner);
}

InputInjectorWin::~InputInjectorWin() {
  core_->Stop();
}

void InputInjectorWin::InjectClipboardEvent(const ClipboardEvent& event) {
  core_->InjectClipboardEvent(event);
}

void InputInjectorWin::InjectKeyEvent(const KeyEvent& event) {
  core_->InjectKeyEvent(event);
}

void InputInjectorWin::InjectTextEvent(const TextEvent& event) {
  core_->InjectTextEvent(event);
}

void InputInjectorWin::InjectMouseEvent(const MouseEvent& event) {
  core_->InjectMouseEvent(event);
}

void InputInjectorWin::InjectTouchEvent(const TouchEvent& event) {
  core_->InjectTouchEvent(event);
}

void InputInjectorWin::Start(
    std::unique_ptr<protocol::ClipboardStub> client_clipboard) {
  core_->Start(std::move(client_clipboard));
}

InputInjectorWin::Core::Core(
    scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner)
    : main_task_runner_(main_task_runner),
      ui_task_runner_(ui_task_runner),
      clipboard_(Clipboard::Create()) {
}

void InputInjectorWin::Core::InjectClipboardEvent(const ClipboardEvent& event) {
  if (!ui_task_runner_->BelongsToCurrentThread()) {
    ui_task_runner_->PostTask(
        FROM_HERE, base::Bind(&Core::InjectClipboardEvent, this, event));
    return;
  }

  // |clipboard_| will ignore unknown MIME-types, and verify the data's format.
  clipboard_->InjectClipboardEvent(event);
}

void InputInjectorWin::Core::InjectKeyEvent(const KeyEvent& event) {
  if (!main_task_runner_->BelongsToCurrentThread()) {
    main_task_runner_->PostTask(FROM_HERE,
                                base::Bind(&Core::InjectKeyEvent, this, event));
    return;
  }

  HandleKey(event);
}

void InputInjectorWin::Core::InjectTextEvent(const TextEvent& event) {
  if (!main_task_runner_->BelongsToCurrentThread()) {
    main_task_runner_->PostTask(
        FROM_HERE, base::Bind(&Core::InjectTextEvent, this, event));
    return;
  }

  HandleText(event);
}

void InputInjectorWin::Core::InjectMouseEvent(const MouseEvent& event) {
  if (!main_task_runner_->BelongsToCurrentThread()) {
    main_task_runner_->PostTask(
        FROM_HERE, base::Bind(&Core::InjectMouseEvent, this, event));
    return;
  }

  HandleMouse(event);
}

void InputInjectorWin::Core::InjectTouchEvent(const TouchEvent& event) {
  if (!main_task_runner_->BelongsToCurrentThread()) {
    main_task_runner_->PostTask(
        FROM_HERE, base::Bind(&Core::InjectTouchEvent, this, event));
    return;
  }

  HandleTouch(event);
}

void InputInjectorWin::Core::Start(
    std::unique_ptr<protocol::ClipboardStub> client_clipboard) {
  if (!ui_task_runner_->BelongsToCurrentThread()) {
    ui_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&Core::Start, this, base::Passed(&client_clipboard)));
    return;
  }

  clipboard_->Start(std::move(client_clipboard));
  touch_injector_.reset(new TouchInjectorWin());
  touch_injector_->Init();
}

void InputInjectorWin::Core::Stop() {
  if (!ui_task_runner_->BelongsToCurrentThread()) {
    ui_task_runner_->PostTask(FROM_HERE, base::Bind(&Core::Stop, this));
    return;
  }

  clipboard_.reset();
  touch_injector_->Deinitialize();
}

InputInjectorWin::Core::~Core() {}

void InputInjectorWin::Core::HandleKey(const KeyEvent& event) {
  // HostEventDispatcher should filter events missing the pressed field.
  DCHECK(event.has_pressed() && event.has_usb_keycode());

  // Reset the system idle suspend timeout.
  SetThreadExecutionState(ES_SYSTEM_REQUIRED);

  int scancode =
      ui::KeycodeConverter::UsbKeycodeToNativeKeycode(event.usb_keycode());
  VLOG(3) << "Converting USB keycode: " << std::hex << event.usb_keycode()
          << " to scancode: " << scancode << std::dec;

  // Ignore events which can't be mapped.
  if (scancode == ui::KeycodeConverter::InvalidNativeKeycode())
    return;

  if (event.has_lock_states() && event.pressed() && !IsLockKey(scancode)) {
    SetLockStates(event.lock_states());
  }

  uint32_t flags = KEYEVENTF_SCANCODE | (event.pressed() ? 0 : KEYEVENTF_KEYUP);
  SendKeyboardInput(flags, scancode);
}

void InputInjectorWin::Core::HandleText(const TextEvent& event) {
  // HostEventDispatcher should filter events missing the pressed field.
  DCHECK(event.has_text());

  base::string16 text = base::UTF8ToUTF16(event.text());
  for (base::string16::const_iterator it = text.begin();
       it != text.end(); ++it)  {
    SendKeyboardInput(KEYEVENTF_UNICODE, *it);
    SendKeyboardInput(KEYEVENTF_UNICODE | KEYEVENTF_KEYUP, *it);
  }
}

void InputInjectorWin::Core::HandleMouse(const MouseEvent& event) {
  // Reset the system idle suspend timeout.
  SetThreadExecutionState(ES_SYSTEM_REQUIRED);

  std::vector<INPUT> inputs;
  ParseMouseMoveEvent(event, &inputs);
  ParseMouseClickEvent(event, &inputs);
  ParseMouseWheelEvent(event, &inputs);

  if (!inputs.empty()) {
    if (SendInput(inputs.size(), inputs.data(), sizeof(INPUT)) != inputs.size())
      PLOG(ERROR) << "Failed to inject a mouse event";
  }
}

void InputInjectorWin::Core::HandleTouch(const TouchEvent& event) {
  DCHECK(touch_injector_);
  touch_injector_->InjectTouchEvent(event);
}

bool InputInjectorWin::Core::IsLockKey(int scancode) {
  UINT virtual_key = MapVirtualKey(scancode, MAPVK_VSC_TO_VK);
  return virtual_key == VK_CAPITAL || virtual_key == VK_NUMLOCK;
}

void InputInjectorWin::Core::SetLockStates(uint32_t states) {
  // Can't use SendKeyboardInput because we need to send virtual key codes, not
  // scan codes.
  INPUT input[2] = {};
  input[0].type = INPUT_KEYBOARD;
  input[1].type = INPUT_KEYBOARD;
  input[1].ki.dwFlags = KEYEVENTF_KEYUP;

  bool client_capslock_state =
      (states & protocol::KeyEvent::LOCK_STATES_CAPSLOCK) != 0;
  bool host_capslock_state = (GetKeyState(VK_CAPITAL) & 1) != 0;
  if (client_capslock_state != host_capslock_state) {
    input[0].ki.wVk = VK_CAPITAL;
    input[1].ki.wVk = VK_CAPITAL;
    SendInput(arraysize(input), input, sizeof(INPUT));
  }
  // TODO(jamiewalch): Reinstate NumLock synchronization when the protocol
  //     supports the client reporting it as "unknown".
}

}  // namespace

// static
std::unique_ptr<InputInjector> InputInjector::Create(
    scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner) {
  return base::WrapUnique(
      new InputInjectorWin(main_task_runner, ui_task_runner));
}

// static
bool InputInjector::SupportsTouchEvents() {
  return base::win::GetVersion() >= base::win::VERSION_WIN8;
}

}  // namespace remoting
