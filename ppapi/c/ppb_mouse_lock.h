/* Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* From ppb_mouse_lock.idl modified Mon Dec 17 16:09:50 2012. */

#ifndef PPAPI_C_PPB_MOUSE_LOCK_H_
#define PPAPI_C_PPB_MOUSE_LOCK_H_

#include "ppapi/c/pp_completion_callback.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/c/pp_macros.h"
#include "ppapi/c/pp_stdint.h"

#define PPB_MOUSELOCK_INTERFACE_1_0 "PPB_MouseLock;1.0"
#define PPB_MOUSELOCK_INTERFACE PPB_MOUSELOCK_INTERFACE_1_0

/**
 * @file
 * This file defines the <code>PPB_MouseLock</code> interface for
 * locking the target of mouse events to a specific module instance.
 */


/**
 * @addtogroup Interfaces
 * @{
 */
/**
 * The <code>PPB_MouseLock</code> interface is implemented by the browser.
 * This interface provides a way of locking the target of mouse events to a
 * single module instance and removing the cursor from view. This mode is
 * useful for certain classes of applications, especially first-person
 * perspective 3D applications and 3D modeling software.
 */
struct PPB_MouseLock_1_0 {
  /**
   * LockMouse() requests the mouse to be locked.
   *
   * While the mouse is locked, the cursor is implicitly hidden from the user.
   * Any movement of the mouse will generate a
   * <code>PP_INPUTEVENT_TYPE_MOUSEMOVE</code> event. The
   * <code>GetPosition()</code> function in the <code>PPB_MouseInputEvent</code>
   * interface reports the last known mouse position just as mouse lock was
   * entered. The <code>GetMovement()</code> function provides relative movement
   * information indicating what the change in position of the mouse would be
   * had it not been locked.
   *
   * The browser may revoke the mouse lock for reasons including (but not
   * limited to) the user pressing the ESC key, the user activating another
   * program using a reserved keystroke (e.g. ALT+TAB), or some other system
   * event.
   *
   * @param[in] instance A <code>PP_Instance</code> identifying one instance
   * of a module.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion.
   *
   * @return An int32_t containing an error code from <code>pp_errors.h</code>.
   */
  int32_t (*LockMouse)(PP_Instance instance,
                       struct PP_CompletionCallback callback);
  /**
   * UnlockMouse() causes the mouse to be unlocked, allowing it to track user
   * movement again. This is an asynchronous operation. The module instance
   * will be notified using the <code>PPP_MouseLock</code> interface when it
   * has lost the mouse lock.
   *
   * @param[in] instance A <code>PP_Instance</code> identifying one instance
   * of a module.
   */
  void (*UnlockMouse)(PP_Instance instance);
};

typedef struct PPB_MouseLock_1_0 PPB_MouseLock;
/**
 * @}
 */

#endif  /* PPAPI_C_PPB_MOUSE_LOCK_H_ */

