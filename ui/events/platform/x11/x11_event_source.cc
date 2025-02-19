// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/events/platform/x11/x11_event_source.h"

#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>

#include "base/logging.h"
#include "base/metrics/histogram_macros.h"
#include "ui/base/x/x11_window_event_manager.h"
#include "ui/events/devices/x11/device_data_manager_x11.h"
#include "ui/events/devices/x11/touch_factory_x11.h"
#include "ui/events/event_utils.h"
#include "ui/events/platform/platform_event_dispatcher.h"
#include "ui/events/platform/x11/x11_hotplug_event_handler.h"
#include "ui/gfx/x/x11_atom_cache.h"

namespace ui {

namespace {

bool InitializeXkb(XDisplay* display) {
  if (!display)
    return false;

  int opcode, event, error;
  int major = XkbMajorVersion;
  int minor = XkbMinorVersion;
  if (!XkbQueryExtension(display, &opcode, &event, &error, &major, &minor)) {
    DVLOG(1) << "Xkb extension not available.";
    return false;
  }

  // Ask the server not to send KeyRelease event when the user holds down a key.
  // crbug.com/138092
  Bool supported_return;
  if (!XkbSetDetectableAutoRepeat(display, True, &supported_return)) {
    DVLOG(1) << "XKB not supported in the server.";
    return false;
  }

  return true;
}

Time ExtractTimeFromXEvent(const XEvent& xevent) {
  switch (xevent.type) {
    case KeyPress:
    case KeyRelease:
      return xevent.xkey.time;
    case ButtonPress:
    case ButtonRelease:
      return xevent.xbutton.time;
    case MotionNotify:
      return xevent.xmotion.time;
    case EnterNotify:
    case LeaveNotify:
      return xevent.xcrossing.time;
    case PropertyNotify:
      return xevent.xproperty.time;
    case SelectionClear:
      return xevent.xselectionclear.time;
    case SelectionRequest:
      return xevent.xselectionrequest.time;
    case SelectionNotify:
      return xevent.xselection.time;
    case GenericEvent:
      if (DeviceDataManagerX11::GetInstance()->IsXIDeviceEvent(xevent))
        return static_cast<XIDeviceEvent*>(xevent.xcookie.data)->time;
      else
        break;
  }
  return CurrentTime;
}

void UpdateDeviceList() {
  XDisplay* display = gfx::GetXDisplay();
  DeviceListCacheX11::GetInstance()->UpdateDeviceList(display);
  TouchFactory::GetInstance()->UpdateDeviceList(display);
  DeviceDataManagerX11::GetInstance()->UpdateDeviceList(display);
}

Bool IsPropertyNotifyForTimestamp(Display* display,
                                  XEvent* event,
                                  XPointer arg) {
  return event->type == PropertyNotify &&
         event->xproperty.window == *reinterpret_cast<Window*>(arg);
}

}  // namespace

X11EventSource* X11EventSource::instance_ = nullptr;

X11EventSource::X11EventSource(X11EventSourceDelegate* delegate,
                               XDisplay* display)
    : delegate_(delegate),
      display_(display),
      dispatching_event_(nullptr),
      dummy_initialized_(false),
      continue_stream_(true) {
  DCHECK(!instance_);
  instance_ = this;

  DCHECK(delegate_);
  DCHECK(display_);
  DeviceDataManagerX11::CreateInstance();
  InitializeXkb(display_);
}

X11EventSource::~X11EventSource() {
  DCHECK_EQ(this, instance_);
  instance_ = nullptr;
  if (dummy_initialized_)
    XDestroyWindow(display_, dummy_window_);
}

bool X11EventSource::HasInstance() {
  return instance_;
}

// static
X11EventSource* X11EventSource::GetInstance() {
  DCHECK(instance_);
  return instance_;
}

////////////////////////////////////////////////////////////////////////////////
// X11EventSource, public

void X11EventSource::DispatchXEvents() {
  DCHECK(display_);
  // Handle all pending events.
  // It may be useful to eventually align this event dispatch with vsync, but
  // not yet.
  continue_stream_ = true;
  while (XPending(display_) && continue_stream_) {
    XEvent xevent;
    XNextEvent(display_, &xevent);
    ExtractCookieDataDispatchEvent(&xevent);
  }
}

void X11EventSource::DispatchXEventNow(XEvent* event) {
  ExtractCookieDataDispatchEvent(event);
}

void X11EventSource::BlockUntilWindowMapped(XID window) {
  BlockOnWindowStructureEvent(window, MapNotify);
}

Time X11EventSource::GetCurrentServerTime() {
  DCHECK(display_);

  if (!dummy_initialized_) {
    // Create a new Window and Atom that will be used for the property change.
    dummy_window_ = XCreateSimpleWindow(display_, DefaultRootWindow(display_),
                                        0, 0, 1, 1, 0, 0, 0);
    dummy_atom_ = gfx::GetAtom("CHROMIUM_TIMESTAMP");
    dummy_window_events_.reset(
        new XScopedEventSelector(dummy_window_, PropertyChangeMask));
    dummy_initialized_ = true;
  }

  base::TimeTicks start = base::TimeTicks::Now();

  // Make a no-op property change on |dummy_window_|.
  XChangeProperty(display_, dummy_window_, dummy_atom_, XA_STRING, 8,
                  PropModeAppend, nullptr, 0);

  // Observe the resulting PropertyNotify event to obtain the timestamp.
  XEvent event;
  XIfEvent(display_, &event, IsPropertyNotifyForTimestamp,
           reinterpret_cast<XPointer>(&dummy_window_));

  UMA_HISTOGRAM_CUSTOM_COUNTS(
      "Linux.X11.ServerRTT", (base::TimeTicks::Now() - start).InMicroseconds(),
      1, base::TimeDelta::FromMilliseconds(50).InMicroseconds(), 50);
  return event.xproperty.time;
}

Time X11EventSource::GetTimestamp() {
  if (dispatching_event_) {
    Time timestamp = ExtractTimeFromXEvent(*dispatching_event_);
    if (timestamp != CurrentTime)
      return timestamp;
  }
  DVLOG(1) << "Making a round trip to get a recent server timestamp.";
  return GetCurrentServerTime();
}

base::Optional<gfx::Point>
X11EventSource::GetRootCursorLocationFromCurrentEvent() const {
  if (!dispatching_event_)
    return base::nullopt;

  XEvent* event = dispatching_event_;
  DCHECK(event);

  bool is_xi2_event = event->type == GenericEvent;
  int event_type = is_xi2_event
                       ? reinterpret_cast<XIDeviceEvent*>(event)->evtype
                       : event->type;

  bool is_valid_event = false;
  static_assert(XI_ButtonPress == ButtonPress, "");
  static_assert(XI_ButtonRelease == ButtonRelease, "");
  static_assert(XI_Motion == MotionNotify, "");
  static_assert(XI_Enter == EnterNotify, "");
  static_assert(XI_Leave == LeaveNotify, "");
  switch (event_type) {
    case ButtonPress:
    case ButtonRelease:
    case MotionNotify:
    case EnterNotify:
    case LeaveNotify:
      is_valid_event =
          is_xi2_event
              ? ui::TouchFactory::GetInstance()->ShouldProcessXI2Event(event)
              : true;
  }

  if (is_valid_event)
    return ui::EventSystemLocationFromNative(event);
  return base::nullopt;
}

////////////////////////////////////////////////////////////////////////////////
// X11EventSource, protected

void X11EventSource::ExtractCookieDataDispatchEvent(XEvent* xevent) {
  bool have_cookie = false;
  if (xevent->type == GenericEvent &&
      XGetEventData(xevent->xgeneric.display, &xevent->xcookie)) {
    have_cookie = true;
  }

  dispatching_event_ = xevent;

  delegate_->ProcessXEvent(xevent);
  PostDispatchEvent(xevent);

  dispatching_event_ = nullptr;

  if (have_cookie)
    XFreeEventData(xevent->xgeneric.display, &xevent->xcookie);
}

void X11EventSource::PostDispatchEvent(XEvent* xevent) {
  bool should_update_device_list = false;

  if (xevent->type == GenericEvent) {
    if (xevent->xgeneric.evtype == XI_HierarchyChanged) {
      should_update_device_list = true;
    } else if (xevent->xgeneric.evtype == XI_DeviceChanged) {
      XIDeviceChangedEvent* xev =
          static_cast<XIDeviceChangedEvent*>(xevent->xcookie.data);
      if (xev->reason == XIDeviceChange) {
        should_update_device_list = true;
      } else if (xev->reason == XISlaveSwitch) {
        ui::DeviceDataManagerX11::GetInstance()->InvalidateScrollClasses(
            xev->sourceid);
      }
    }
  }

  if (should_update_device_list) {
    UpdateDeviceList();
    hotplug_event_handler_->OnHotplugEvent();
  }

  if (xevent->type == EnterNotify &&
      xevent->xcrossing.detail != NotifyInferior &&
      xevent->xcrossing.mode != NotifyUngrab) {
    // Clear stored scroll data
    ui::DeviceDataManagerX11::GetInstance()->InvalidateScrollClasses(
        DeviceDataManagerX11::kAllDevices);
  }
}

void X11EventSource::BlockOnWindowStructureEvent(XID window, int type) {
  XEvent event;
  do {
    // Block until there's a StructureNotify event of |type| on |window|. Then
    // remove it from the queue and stuff it in |event|.
    XWindowEvent(display_, window, StructureNotifyMask, &event);
    ExtractCookieDataDispatchEvent(&event);
  } while (event.type != type);
}

void X11EventSource::StopCurrentEventStream() {
  continue_stream_ = false;
}

void X11EventSource::OnDispatcherListChanged() {
  if (!hotplug_event_handler_) {
    hotplug_event_handler_.reset(new X11HotplugEventHandler());
    // Force the initial device query to have an update list of active devices.
    hotplug_event_handler_->OnHotplugEvent();
  }
}

}  // namespace ui
