// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser.androidoverlay;

import android.annotation.SuppressLint;
import android.app.Dialog;
import android.content.Context;
import android.os.IBinder;
import android.view.Gravity;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.Window;
import android.view.WindowManager;

import org.chromium.gfx.mojom.Rect;
import org.chromium.media.mojom.AndroidOverlayConfig;

/**
 * Core class for control of a single Dialog-based AndroidOverlay instance.  Everything runs on the
 * overlay thread, which is not the Browser UI thread.
 *
 * Note that this does not implement AndroidOverlay; we assume that, and the associated thread-
 * hopping, is handled elsewhere (DialogOverlayImpl).
 */
class DialogOverlayCore {
    private static final String TAG = "DSCore";

    // Host interface, since we're on the wrong thread to talk to mojo, or anything else, really.
    public interface Host {
        // Notify the host that we have a surface.
        void onSurfaceReady(Surface surface);

        // Notify the host that we have failed to get a surface or the surface was destroyed.
        void onOverlayDestroyed();

        // Wait until the host has been told to clean up.  We are allowed to let surfaceDestroyed
        // proceed once this happens.
        void waitForCleanup();
    }

    private Host mHost;

    // When initialized via Init, we'll create mDialog.  We'll clear it when we send
    // onOverlayDestroyed to the host.  In general, when this is null, either we haven't been
    // initialized yet, or we've been torn down.  It shouldn't be the case that anything calls
    // methods after construction but before |initialize()|, though.
    private Dialog mDialog;

    private Callbacks mDialogCallbacks;

    // Most recent layout parameters.
    private WindowManager.LayoutParams mLayoutParams;

    // If true, then we'll be a panel rather than media overlay.  This is for testing.
    private boolean mAsPanel;

    /**
     * Construction may be called from a random thread, for simplicity.  Call initialize from the
     * proper thread before doing anything else.
     */
    public DialogOverlayCore() {}

    /**
     * Finish init on the proper thread.  We'll use this thread for the Dialog Looper thread.
     * @param dialog the dialog, which uses our current thread as the UI thread.
     * @param config initial config.
     * @param host host interface, for sending messages that (probably) need to thread hop.
     * @param asPanel if true, then we'll be a panel.  This is intended for tests only.
     */
    public void initialize(
            Context context, AndroidOverlayConfig config, Host host, boolean asPanel) {
        mHost = host;
        mAsPanel = asPanel;

        mDialog = new Dialog(context, android.R.style.Theme_NoDisplay);
        mDialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        mDialog.setCancelable(false);

        mLayoutParams = createLayoutParams(config.secure);
        copyRectToLayoutParams(config.rect);
    }

    /**
     * Release the underlying surface, and generally clean up, in response to
     * the client releasing the AndroidOverlay.
     */
    public void release() {
        // If we've not released the dialog yet, then do so.
        if (mDialog != null) {
            if (mDialog.isShowing()) mDialog.dismiss();
            mDialog = null;
            mDialogCallbacks = null;
        }

        mLayoutParams.token = null;

        // We don't bother to notify |mHost| that we've been destroyed; it told us.
        mHost = null;
    }

    private void copyRectToLayoutParams(final Rect rect) {
        // TODO(liberato): adjust for CompositorView screen location here if we want to support
        // non-full screen use cases.
        mLayoutParams.x = rect.x;
        mLayoutParams.y = rect.y;
        mLayoutParams.width = rect.width;
        mLayoutParams.height = rect.height;
    }

    /**
     * Layout the AndroidOverlay.  If we don't have a token, then we ignore it, since a well-behaved
     * client shouldn't call us before getting the surface anyway.
     */
    public void layoutSurface(final Rect rect) {
        if (mDialog == null || mLayoutParams.token == null) return;

        copyRectToLayoutParams(rect);
        mDialog.getWindow().setAttributes(mLayoutParams);
    }

    /**
     * Callbacks for finding out about the Dialog's Surface.
     * These happen on the looper thread.
     */
    private class Callbacks implements SurfaceHolder.Callback2 {
        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {}

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            // Make sure that we haven't torn down the dialog yet.
            if (mDialog == null) return;

            if (mHost != null) mHost.onSurfaceReady(holder.getSurface());
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            if (mDialog == null || mHost == null) return;

            // Notify the host that we've been destroyed, and wait for it to clean up.
            mHost.onOverlayDestroyed();
            mHost.waitForCleanup();
            mHost = null;
        }

        @Override
        public void surfaceRedrawNeeded(SurfaceHolder holder) {}
    }

    public void onWindowToken(IBinder token) {
        if (mDialog == null || mHost == null) return;

        if (token == null || (mLayoutParams.token != null && token != mLayoutParams.token)) {
            // We've lost the token, if we had one, or we got a new one.
            // Notify the client.
            mHost.onOverlayDestroyed();
            mHost = null;
            if (mDialog.isShowing()) mDialog.dismiss();
            return;
        }

        if (mLayoutParams.token == token) {
            // Same token, do nothing.
            return;
        }

        // We have a token, so layout the dialog.
        mLayoutParams.token = token;
        mDialog.getWindow().setAttributes(mLayoutParams);
        mDialogCallbacks = new Callbacks();
        mDialog.getWindow().takeSurface(mDialogCallbacks);
        mDialog.show();

        // We don't notify the client here.  We'll wait until the Android Surface is created.
    }

    @SuppressLint("RtlHardcoded")
    private WindowManager.LayoutParams createLayoutParams(boolean secure) {
        // Rather than using getAttributes, we just create them from scratch.
        // The default dialog attributes aren't what we want.
        WindowManager.LayoutParams layoutParams = new WindowManager.LayoutParams();

        // NOTE: we really do want LEFT here, since we're dealing in compositor
        // coordinates.  Those are always from the left.
        layoutParams.gravity = Gravity.TOP | Gravity.LEFT;

        // Use a media surface, which is what SurfaceView uses by default.  For
        // debugging overlay drawing, consider using TYPE_APPLICATION_PANEL to
        // move the dialog over the CompositorView.
        layoutParams.type = mAsPanel ? WindowManager.LayoutParams.TYPE_APPLICATION_PANEL
                                     : WindowManager.LayoutParams.TYPE_APPLICATION_MEDIA;

        layoutParams.flags = WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
                | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS;

        if (secure) {
            layoutParams.flags |= WindowManager.LayoutParams.FLAG_SECURE;
        }

        // Don't set FLAG_SCALED.  in addition to not being sure what it does
        // (SV uses it), it also causes a crash in WindowManager when we hide
        // (not dismiss), navigate, and/or exit the app without hide/dismiss.
        // There's a missing null check in WindowManagerService.java@3170
        // on M MR2.  To repro, change dimiss() to hide(), bring up a SV, and
        // navigate away or press home.

        // Turn off the position animation, so that it doesn't animate from one
        // position to the next.  Ignore errors.
        // 0x40 is PRIVATE_FLAG_NO_MOVE_ANIMATION.
        try {
            int currentFlags =
                    (Integer) layoutParams.getClass().getField("privateFlags").get(layoutParams);
            layoutParams.getClass()
                    .getField("privateFlags")
                    .set(layoutParams, currentFlags | 0x00000040);
            // It would be nice to just catch Exception, but findbugs doesn't
            // allow it.  If we cannot set the flag, then that's okay too.
        } catch (NoSuchFieldException e) {
        } catch (NullPointerException e) {
        } catch (SecurityException e) {
        } catch (IllegalAccessException e) {
        } catch (IllegalArgumentException e) {
        } catch (ExceptionInInitializerError e) {
        }

        return layoutParams;
    }

    /**
     * Package-private to retrieve our current dialog for tests.
     */
    Dialog getDialog() {
        return mDialog;
    }
}
