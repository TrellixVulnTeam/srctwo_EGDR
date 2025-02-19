// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/input_injector.h"

#include <stddef.h>
#include <stdint.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XTest.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#undef Status  // Xlib.h #defines this, which breaks protobuf headers.

#include <set>
#include <utility>

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/utf_string_conversion_utils.h"
#include "build/build_config.h"
#include "remoting/base/logging.h"
#include "remoting/host/clipboard.h"
#include "remoting/host/linux/unicode_to_keysym.h"
#include "remoting/host/linux/x11_character_injector.h"
#include "remoting/host/linux/x11_keyboard_impl.h"
#include "remoting/host/linux/x11_util.h"
#include "remoting/proto/internal.pb.h"
#include "third_party/webrtc/modules/desktop_capture/desktop_geometry.h"
#include "ui/events/keycodes/dom/dom_code.h"
#include "ui/events/keycodes/dom/keycode_converter.h"

#if defined(OS_CHROMEOS)
#include "remoting/host/chromeos/point_transformer.h"
#endif

namespace remoting {

namespace {

using protocol::ClipboardEvent;
using protocol::KeyEvent;
using protocol::TextEvent;
using protocol::MouseEvent;
using protocol::TouchEvent;

bool IsModifierKey(ui::DomCode dom_code) {
  return dom_code == ui::DomCode::CONTROL_LEFT ||
         dom_code == ui::DomCode::SHIFT_LEFT ||
         dom_code == ui::DomCode::ALT_LEFT ||
         dom_code == ui::DomCode::META_LEFT ||
         dom_code == ui::DomCode::CONTROL_RIGHT ||
         dom_code == ui::DomCode::SHIFT_RIGHT ||
         dom_code == ui::DomCode::ALT_RIGHT ||
         dom_code == ui::DomCode::META_RIGHT;
}

// Pixel-to-wheel-ticks conversion ratio used by GTK.
// From third_party/WebKit/Source/web/gtk/WebInputEventFactory.cpp .
const float kWheelTicksPerPixel = 3.0f / 160.0f;

// A class to generate events on X11.
class InputInjectorX11 : public InputInjector {
 public:
  explicit InputInjectorX11(
      scoped_refptr<base::SingleThreadTaskRunner> task_runner);
  ~InputInjectorX11() override;

  bool Init();

  // Clipboard stub interface.
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
  // The actual implementation resides in InputInjectorX11::Core class.
  class Core : public base::RefCountedThreadSafe<Core> {
   public:
    explicit Core(scoped_refptr<base::SingleThreadTaskRunner> task_runner);

    bool Init();

    // Mirrors the ClipboardStub interface.
    void InjectClipboardEvent(const ClipboardEvent& event);

    // Mirrors the InputStub interface.
    void InjectKeyEvent(const KeyEvent& event);
    void InjectTextEvent(const TextEvent& event);
    void InjectMouseEvent(const MouseEvent& event);

    // Mirrors the InputInjector interface.
    void Start(std::unique_ptr<protocol::ClipboardStub> client_clipboard);

    void Stop();

   private:
    friend class base::RefCountedThreadSafe<Core>;
    virtual ~Core();

    void InitClipboard();

    // Queries whether keyboard auto-repeat is globally enabled. This is used
    // to decide whether to temporarily disable then restore this setting. If
    // auto-repeat has already been disabled, this class should leave it
    // untouched.
    bool IsAutoRepeatEnabled();

    // Enables or disables keyboard auto-repeat globally.
    void SetAutoRepeatEnabled(bool enabled);

    // Check if the given scan code is caps lock or num lock.
    bool IsLockKey(KeyCode keycode);

    // Sets the keyboard lock states to those provided.
    void SetLockStates(uint32_t states);

    void InjectScrollWheelClicks(int button, int count);
    // Compensates for global button mappings and resets the XTest device
    // mapping.
    void InitMouseButtonMap();
    int MouseButtonToX11ButtonNumber(MouseEvent::MouseButton button);
    int HorizontalScrollWheelToX11ButtonNumber(int dx);
    int VerticalScrollWheelToX11ButtonNumber(int dy);

    scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

    std::set<int> pressed_keys_;
    webrtc::DesktopVector latest_mouse_position_;
    float wheel_ticks_x_;
    float wheel_ticks_y_;

    // X11 graphics context.
    Display* display_;
    Window root_window_;

    // Number of buttons we support.
    // Left, Right, Middle, VScroll Up/Down, HScroll Left/Right.
    static const int kNumPointerButtons = 7;

    int pointer_button_map_[kNumPointerButtons];

#if defined(OS_CHROMEOS)
    PointTransformer point_transformer_;
#endif

    std::unique_ptr<Clipboard> clipboard_;

    std::unique_ptr<X11CharacterInjector> character_injector_;

    bool saved_auto_repeat_enabled_;

    DISALLOW_COPY_AND_ASSIGN(Core);
  };

  scoped_refptr<Core> core_;

  DISALLOW_COPY_AND_ASSIGN(InputInjectorX11);
};

InputInjectorX11::InputInjectorX11(
    scoped_refptr<base::SingleThreadTaskRunner> task_runner) {
  core_ = new Core(task_runner);
}

InputInjectorX11::~InputInjectorX11() {
  core_->Stop();
}

bool InputInjectorX11::Init() {
  return core_->Init();
}

void InputInjectorX11::InjectClipboardEvent(const ClipboardEvent& event) {
  core_->InjectClipboardEvent(event);
}

void InputInjectorX11::InjectKeyEvent(const KeyEvent& event) {
  core_->InjectKeyEvent(event);
}

void InputInjectorX11::InjectTextEvent(const TextEvent& event) {
  core_->InjectTextEvent(event);
}

void InputInjectorX11::InjectMouseEvent(const MouseEvent& event) {
  core_->InjectMouseEvent(event);
}

void InputInjectorX11::InjectTouchEvent(const TouchEvent& event) {
  NOTIMPLEMENTED() << "Raw touch event injection not implemented for X11.";
}

void InputInjectorX11::Start(
    std::unique_ptr<protocol::ClipboardStub> client_clipboard) {
  core_->Start(std::move(client_clipboard));
}

InputInjectorX11::Core::Core(
    scoped_refptr<base::SingleThreadTaskRunner> task_runner)
    : task_runner_(task_runner),
      latest_mouse_position_(-1, -1),
      wheel_ticks_x_(0.0f),
      wheel_ticks_y_(0.0f),
      display_(XOpenDisplay(nullptr)),
      root_window_(BadValue),
      saved_auto_repeat_enabled_(false) {
}

bool InputInjectorX11::Core::Init() {
  CHECK(display_);

  if (!task_runner_->BelongsToCurrentThread())
    task_runner_->PostTask(FROM_HERE, base::Bind(&Core::InitClipboard, this));

  root_window_ = RootWindow(display_, DefaultScreen(display_));
  if (root_window_ == BadValue) {
    LOG(ERROR) << "Unable to get the root window";
    return false;
  }

  if (!IgnoreXServerGrabs(display_, true)) {
    LOG(ERROR) << "Server does not support XTest.";
    return false;
  }
  InitMouseButtonMap();
  return true;
}

void InputInjectorX11::Core::InjectClipboardEvent(
    const ClipboardEvent& event) {
  if (!task_runner_->BelongsToCurrentThread()) {
    task_runner_->PostTask(
        FROM_HERE, base::Bind(&Core::InjectClipboardEvent, this, event));
    return;
  }

  // |clipboard_| will ignore unknown MIME-types, and verify the data's format.
  clipboard_->InjectClipboardEvent(event);
}

void InputInjectorX11::Core::InjectKeyEvent(const KeyEvent& event) {
  // HostEventDispatcher should filter events missing the pressed field.
  if (!event.has_pressed() || !event.has_usb_keycode())
    return;

  if (!task_runner_->BelongsToCurrentThread()) {
    task_runner_->PostTask(FROM_HERE,
                           base::Bind(&Core::InjectKeyEvent, this, event));
    return;
  }

  int keycode =
      ui::KeycodeConverter::UsbKeycodeToNativeKeycode(event.usb_keycode());

  VLOG(3) << "Converting USB keycode: " << std::hex << event.usb_keycode()
          << " to keycode: " << keycode << std::dec;

  // Ignore events which can't be mapped.
  if (keycode == ui::KeycodeConverter::InvalidNativeKeycode())
    return;

  if (event.pressed()) {
    if (pressed_keys_.find(keycode) != pressed_keys_.end()) {
      // Ignore repeats for modifier keys.
      if (IsModifierKey(static_cast<ui::DomCode>(event.usb_keycode())))
        return;
      // Key is already held down, so lift the key up to ensure this repeated
      // press takes effect.
      XTestFakeKeyEvent(display_, keycode, False, CurrentTime);
    }

    if (event.has_lock_states() && !IsLockKey(keycode)) {
      SetLockStates(event.lock_states());
    }

    if (pressed_keys_.empty()) {
      // Disable auto-repeat, if necessary, to avoid triggering auto-repeat
      // if network congestion delays the key-up event from the client.
      saved_auto_repeat_enabled_ = IsAutoRepeatEnabled();
      if (saved_auto_repeat_enabled_)
        SetAutoRepeatEnabled(false);
    }
    pressed_keys_.insert(keycode);
  } else {
    pressed_keys_.erase(keycode);
    if (pressed_keys_.empty()) {
      // Re-enable auto-repeat, if necessary, when all keys are released.
      if (saved_auto_repeat_enabled_)
        SetAutoRepeatEnabled(true);
    }
  }

  XTestFakeKeyEvent(display_, keycode, event.pressed(), CurrentTime);
  XFlush(display_);
}

void InputInjectorX11::Core::InjectTextEvent(const TextEvent& event) {
  if (!task_runner_->BelongsToCurrentThread()) {
    task_runner_->PostTask(FROM_HERE,
                           base::Bind(&Core::InjectTextEvent, this, event));
    return;
  }

  // Release all keys before injecting text event. This is necessary to avoid
  // any interference with the currently pressed keys. E.g. if Shift is pressed
  // when TextEvent is received.
  for (int key : pressed_keys_) {
    XTestFakeKeyEvent(display_, key, False, CurrentTime);
  }
  pressed_keys_.clear();

  const std::string text = event.text();
  for (int32_t index = 0; index < static_cast<int32_t>(text.size()); ++index) {
    uint32_t code_point;
    if (!base::ReadUnicodeCharacter(
            text.c_str(), text.size(), &index, &code_point)) {
      continue;
    }
    character_injector_->Inject(code_point);
  }
}

InputInjectorX11::Core::~Core() {
  CHECK(pressed_keys_.empty());
}

void InputInjectorX11::Core::InitClipboard() {
  DCHECK(task_runner_->BelongsToCurrentThread());
  clipboard_ = Clipboard::Create();
}

bool InputInjectorX11::Core::IsAutoRepeatEnabled() {
  XKeyboardState state;
  if (!XGetKeyboardControl(display_, &state)) {
    LOG(ERROR) << "Failed to get keyboard auto-repeat status, assuming ON.";
    return true;
  }
  return state.global_auto_repeat == AutoRepeatModeOn;
}

void InputInjectorX11::Core::SetAutoRepeatEnabled(bool mode) {
  XKeyboardControl control;
  control.auto_repeat_mode = mode ? AutoRepeatModeOn : AutoRepeatModeOff;
  XChangeKeyboardControl(display_, KBAutoRepeatMode, &control);
}

bool InputInjectorX11::Core::IsLockKey(KeyCode keycode) {
  XkbStateRec state;
  KeySym keysym;
  if (XkbGetState(display_, XkbUseCoreKbd, &state) == Success &&
      XkbLookupKeySym(display_, keycode, XkbStateMods(&state), nullptr,
                      &keysym) == True) {
    return keysym == XK_Caps_Lock || keysym == XK_Num_Lock;
  } else {
    return false;
  }
}

void InputInjectorX11::Core::SetLockStates(uint32_t states) {
  // TODO(jamiewalch): Reinstate NumLock synchronization when the protocol
  //     supports the client reporting it as "unknown".
  unsigned int caps_lock_mask = XkbKeysymToModifiers(display_, XK_Caps_Lock);
  unsigned int lock_values = 0;
  if (states & protocol::KeyEvent::LOCK_STATES_CAPSLOCK) {
    lock_values |= caps_lock_mask;
  }
  XkbLockModifiers(display_, XkbUseCoreKbd, caps_lock_mask, lock_values);
}

void InputInjectorX11::Core::InjectScrollWheelClicks(int button, int count) {
  if (button < 0) {
    LOG(WARNING) << "Ignoring unmapped scroll wheel button";
    return;
  }
  for (int i = 0; i < count; i++) {
    // Generate a button-down and a button-up to simulate a wheel click.
    XTestFakeButtonEvent(display_, button, true, CurrentTime);
    XTestFakeButtonEvent(display_, button, false, CurrentTime);
  }
}

void InputInjectorX11::Core::InjectMouseEvent(const MouseEvent& event) {
  if (!task_runner_->BelongsToCurrentThread()) {
    task_runner_->PostTask(FROM_HERE,
                           base::Bind(&Core::InjectMouseEvent, this, event));
    return;
  }

  if (event.has_delta_x() &&
      event.has_delta_y() &&
      (event.delta_x() != 0 || event.delta_y() != 0)) {
    latest_mouse_position_.set(-1, -1);
    VLOG(3) << "Moving mouse by " << event.delta_x() << "," << event.delta_y();
    XTestFakeRelativeMotionEvent(display_,
                                 event.delta_x(), event.delta_y(),
                                 CurrentTime);

  } else if (event.has_x() && event.has_y()) {
    // Injecting a motion event immediately before a button release results in
    // a MotionNotify even if the mouse position hasn't changed, which confuses
    // apps which assume MotionNotify implies movement. See crbug.com/138075.
    bool inject_motion = true;
    webrtc::DesktopVector new_mouse_position(event.x(), event.y());
#if defined(OS_CHROMEOS)
    // Interim hack to handle display rotation on Chrome OS.
    // TODO(kelvin): Remove this when Chrome OS has completely migrated to
    // Ozone (crbug.com/439287).
    gfx::PointF screen_location = point_transformer_.ToScreenCoordinates(
        gfx::PointF(event.x(), event.y()));
    new_mouse_position.set(screen_location.x(), screen_location.y());
#endif
    if (event.has_button() && event.has_button_down() && !event.button_down()) {
      if (new_mouse_position.equals(latest_mouse_position_))
        inject_motion = false;
    }

    if (inject_motion) {
      latest_mouse_position_.set(std::max(0, new_mouse_position.x()),
                                 std::max(0, new_mouse_position.y()));

      VLOG(3) << "Moving mouse to " << latest_mouse_position_.x()
              << "," << latest_mouse_position_.y();
      XTestFakeMotionEvent(display_, DefaultScreen(display_),
                           latest_mouse_position_.x(),
                           latest_mouse_position_.y(),
                           CurrentTime);
    }
  }

  if (event.has_button() && event.has_button_down()) {
    int button_number = MouseButtonToX11ButtonNumber(event.button());

    if (button_number < 0) {
      LOG(WARNING) << "Ignoring unknown button type: " << event.button();
      return;
    }

    VLOG(3) << "Button " << event.button()
            << " received, sending "
            << (event.button_down() ? "down " : "up ")
            << button_number;
    XTestFakeButtonEvent(display_, button_number, event.button_down(),
                         CurrentTime);
  }

  // Older client plugins always send scroll events in pixels, which
  // must be accumulated host-side. Recent client plugins send both
  // pixels and ticks with every scroll event, allowing the host to
  // choose the best model on a per-platform basis. Since we can only
  // inject ticks on Linux, use them if available.
  int ticks_y = 0;
  if (event.has_wheel_ticks_y()) {
    ticks_y = event.wheel_ticks_y();
  } else if (event.has_wheel_delta_y()) {
    wheel_ticks_y_ += event.wheel_delta_y() * kWheelTicksPerPixel;
    ticks_y = static_cast<int>(wheel_ticks_y_);
    wheel_ticks_y_ -= ticks_y;
  }
  if (ticks_y != 0) {
    InjectScrollWheelClicks(VerticalScrollWheelToX11ButtonNumber(ticks_y),
                            abs(ticks_y));
  }

  int ticks_x = 0;
  if (event.has_wheel_ticks_x()) {
    ticks_x = event.wheel_ticks_x();
  } else if (event.has_wheel_delta_x()) {
    wheel_ticks_x_ += event.wheel_delta_x() * kWheelTicksPerPixel;
    ticks_x = static_cast<int>(wheel_ticks_x_);
    wheel_ticks_x_ -= ticks_x;
  }
  if (ticks_x != 0) {
    InjectScrollWheelClicks(HorizontalScrollWheelToX11ButtonNumber(ticks_x),
                            abs(ticks_x));
  }

  XFlush(display_);
}

void InputInjectorX11::Core::InitMouseButtonMap() {
  // TODO(rmsousa): Run this on global/device mapping change events.

  // Do not touch global pointer mapping, since this may affect the local user.
  // Instead, try to work around it by reversing the mapping.
  // Note that if a user has a global mapping that completely disables a button
  // (by assigning 0 to it), we won't be able to inject it.
  int num_buttons = XGetPointerMapping(display_, nullptr, 0);
  std::unique_ptr<unsigned char[]> pointer_mapping(
      new unsigned char[num_buttons]);
  num_buttons = XGetPointerMapping(display_, pointer_mapping.get(),
                                   num_buttons);
  for (int i = 0; i < kNumPointerButtons; i++) {
    pointer_button_map_[i] = -1;
  }
  for (int i = 0; i < num_buttons; i++) {
    // Reverse the mapping.
    if (pointer_mapping[i] > 0 && pointer_mapping[i] <= kNumPointerButtons)
      pointer_button_map_[pointer_mapping[i] - 1] = i + 1;
  }
  for (int i = 0; i < kNumPointerButtons; i++) {
    if (pointer_button_map_[i] == -1)
      LOG(ERROR) << "Global pointer mapping does not support button " << i + 1;
  }

  int opcode, event, error;
  if (!XQueryExtension(display_, "XInputExtension", &opcode, &event, &error)) {
    // If XInput is not available, we're done. But it would be very unusual to
    // have a server that supports XTest but not XInput, so log it as an error.
    LOG(ERROR) << "X Input extension not available: " << error;
    return;
  }

  // Make sure the XTEST XInput pointer device mapping is trivial. It should be
  // safe to reset this mapping, as it won't affect the user's local devices.
  // In fact, the reason why we do this is because an old gnome-settings-daemon
  // may have mistakenly applied left-handed preferences to the XTEST device.
  XID device_id = 0;
  bool device_found = false;
  int num_devices;
  XDeviceInfo* devices;
  devices = XListInputDevices(display_, &num_devices);
  for (int i = 0; i < num_devices; i++) {
    XDeviceInfo* device_info = &devices[i];
    if (device_info->use == IsXExtensionPointer &&
        strcmp(device_info->name, "Virtual core XTEST pointer") == 0) {
      device_id = device_info->id;
      device_found = true;
      break;
    }
  }
  XFreeDeviceList(devices);

  if (!device_found) {
    HOST_LOG << "Cannot find XTest device.";
    return;
  }

  XDevice* device = XOpenDevice(display_, device_id);
  if (!device) {
    LOG(ERROR) << "Cannot open XTest device.";
    return;
  }

  int num_device_buttons =
      XGetDeviceButtonMapping(display_, device, nullptr, 0);
  std::unique_ptr<unsigned char[]> button_mapping(
      new unsigned char[num_buttons]);
  for (int i = 0; i < num_device_buttons; i++) {
    button_mapping[i] = i + 1;
  }
  error = XSetDeviceButtonMapping(display_, device, button_mapping.get(),
                                  num_device_buttons);
  if (error != Success)
    LOG(ERROR) << "Failed to set XTest device button mapping: " << error;

  XCloseDevice(display_, device);
}

int InputInjectorX11::Core::MouseButtonToX11ButtonNumber(
    MouseEvent::MouseButton button) {
  switch (button) {
    case MouseEvent::BUTTON_LEFT:
      return pointer_button_map_[0];

    case MouseEvent::BUTTON_RIGHT:
      return pointer_button_map_[2];

    case MouseEvent::BUTTON_MIDDLE:
      return pointer_button_map_[1];

    case MouseEvent::BUTTON_UNDEFINED:
    default:
      return -1;
  }
}

int InputInjectorX11::Core::HorizontalScrollWheelToX11ButtonNumber(int dx) {
  return (dx > 0 ? pointer_button_map_[5] : pointer_button_map_[6]);
}

int InputInjectorX11::Core::VerticalScrollWheelToX11ButtonNumber(int dy) {
  // Positive y-values are wheel scroll-up events (button 4), negative y-values
  // are wheel scroll-down events (button 5).
  return (dy > 0 ? pointer_button_map_[3] : pointer_button_map_[4]);
}

void InputInjectorX11::Core::Start(
    std::unique_ptr<protocol::ClipboardStub> client_clipboard) {
  if (!task_runner_->BelongsToCurrentThread()) {
    task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&Core::Start, this, base::Passed(&client_clipboard)));
    return;
  }

  InitMouseButtonMap();

  clipboard_->Start(std::move(client_clipboard));

  character_injector_.reset(
      new X11CharacterInjector(base::MakeUnique<X11KeyboardImpl>(display_)));
}

void InputInjectorX11::Core::Stop() {
  if (!task_runner_->BelongsToCurrentThread()) {
    task_runner_->PostTask(FROM_HERE, base::Bind(&Core::Stop, this));
    return;
  }

  clipboard_.reset();
  character_injector_.reset();
}

}  // namespace

// static
std::unique_ptr<InputInjector> InputInjector::Create(
    scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner) {
  std::unique_ptr<InputInjectorX11> injector(
      new InputInjectorX11(main_task_runner));
  if (!injector->Init())
    return nullptr;
  return std::move(injector);
}

// static
bool InputInjector::SupportsTouchEvents() {
  return false;
}

}  // namespace remoting
