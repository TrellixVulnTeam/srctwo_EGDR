// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_WM_WORKSPACE_MULTI_WINDOW_RESIZE_CONTROLLER_H_
#define ASH_WM_WORKSPACE_MULTI_WINDOW_RESIZE_CONTROLLER_H_

#include <memory>
#include <vector>

#include "ash/ash_export.h"
#include "base/macros.h"
#include "base/timer/timer.h"
#include "ui/aura/window_observer.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/views/mouse_watcher.h"

namespace views {
class Widget;
}

namespace ash {
class MultiWindowResizeControllerTest;
class WorkspaceWindowResizer;

// Two directions resizes happen in.
enum Direction {
  TOP_BOTTOM,
  LEFT_RIGHT,
};

// MultiWindowResizeController is responsible for determining and showing a
// widget that allows resizing multiple windows at the same time.
// MultiWindowResizeController is driven by WorkspaceEventFilter.
class ASH_EXPORT MultiWindowResizeController
    : public views::MouseWatcherListener,
      public aura::WindowObserver {
 public:
  MultiWindowResizeController();
  ~MultiWindowResizeController() override;

  // If necessary, shows the resize widget. |window| is the window the mouse
  // is over, |component| the edge and |point| the location of the mouse.
  void Show(aura::Window* window, int component, const gfx::Point& point);

  // Hides the resize widget.
  void Hide();

  // MouseWatcherListenre overrides:
  void MouseMovedOutOfHost() override;

  // WindowObserver overrides:
  void OnWindowDestroying(aura::Window* window) override;

 private:
  friend class MultiWindowResizeControllerTest;

  // Used to track the two resizable windows and direction.
  struct ResizeWindows {
    ResizeWindows();
    ResizeWindows(const ResizeWindows& other);
    ~ResizeWindows();

    // Returns true if |other| equals this ResizeWindows. This does *not*
    // consider the windows in |other_windows|.
    bool Equals(const ResizeWindows& other) const;

    // Returns true if this ResizeWindows is valid.
    bool is_valid() const { return window1 && window2; }

    // The left/top window to resize.
    aura::Window* window1;

    // Other window to resize.
    aura::Window* window2;

    // Direction
    Direction direction;

    // Windows after |window2| that are to be resized. Determined at the time
    // the resize starts.
    std::vector<aura::Window*> other_windows;
  };

  class ResizeMouseWatcherHost;
  class ResizeView;

  void CreateMouseWatcher();

  // Returns a ResizeWindows based on the specified arguments. Use is_valid()
  // to test if the return value is a valid multi window resize location.
  ResizeWindows DetermineWindows(aura::Window* window,
                                 int window_component,
                                 const gfx::Point& point) const;

  // Variant of DetermineWindows() that uses the current location of the mouse
  // to determine the resize windows.
  ResizeWindows DetermineWindowsFromScreenPoint(aura::Window* window) const;

  // Finds a window by edge (one of the constants HitTestCompat.
  aura::Window* FindWindowByEdge(aura::Window* window_to_ignore,
                                 int edge_want,
                                 int x_in_parent,
                                 int y_in_parent) const;

  // Returns the first window touching |window|.
  aura::Window* FindWindowTouching(aura::Window* window,
                                   Direction direction) const;

  // Places any windows touching |start| into |others|.
  void FindWindowsTouching(aura::Window* start,
                           Direction direction,
                           std::vector<aura::Window*>* others) const;

  // Shows the resizer if the mouse is still at a valid location. This is called
  // from the |show_timer_|.
  void ShowIfValidMouseLocation();

  // Shows the widget immediately.
  void ShowNow();

  // Returns true if the widget is showing.
  bool IsShowing() const;

  // Initiates a resize.
  void StartResize(const gfx::Point& location_in_screen);

  // Resizes to the new location.
  void Resize(const gfx::Point& location_in_screen, int event_flags);

  // Completes the resize.
  void CompleteResize();

  // Cancels the resize.
  void CancelResize();

  // Returns the bounds for the resize widget.
  gfx::Rect CalculateResizeWidgetBounds(
      const gfx::Point& location_in_parent) const;

  // Returns true if |location_in_screen| is over the resize widget.
  bool IsOverResizeWidget(const gfx::Point& location_in_screen) const;

  // Returns true if |location_in_screen| is over the resize windows
  // (or the resize widget itself).
  bool IsOverWindows(const gfx::Point& location_in_screen) const;

  // Returns true if |location_in_screen| is over |component| in |window|.
  bool IsOverComponent(aura::Window* window,
                       const gfx::Point& location_in_screen,
                       int component) const;

  // Windows and direction to resize.
  ResizeWindows windows_;

  // Timer used before showing.
  base::OneShotTimer show_timer_;

  std::unique_ptr<views::Widget> resize_widget_;

  // If non-null we're in a resize loop.
  std::unique_ptr<WorkspaceWindowResizer> window_resizer_;

  // Mouse coordinate passed to Show() in container's coodinates.
  gfx::Point show_location_in_parent_;

  // Bounds the widget was last shown at in screen coordinates.
  gfx::Rect show_bounds_in_screen_;

  // Used to detect whether the mouse is over the windows. While
  // |resize_widget_| is non-NULL (ie the widget is showing) we ignore calls
  // to Show().
  std::unique_ptr<views::MouseWatcher> mouse_watcher_;

  DISALLOW_COPY_AND_ASSIGN(MultiWindowResizeController);
};

}  // namespace ash

#endif  // ASH_WM_WORKSPACE_MULTI_WINDOW_RESIZE_CONTROLLER_H_
