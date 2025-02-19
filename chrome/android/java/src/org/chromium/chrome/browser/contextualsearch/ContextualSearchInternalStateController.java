// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.contextualsearch;

import org.chromium.base.Log;
import org.chromium.base.VisibleForTesting;
import org.chromium.chrome.browser.compositor.bottombar.OverlayPanel.StateChangeReason;

import javax.annotation.Nullable;

/**
 * Controls the internal state of the Contextual Search Manager.
 * <p>
 * This class keeps track of the current internal state of the {@code ContextualSearchManager} and
 * helps it to transition between states and return to the idle state when work has been
 * interrupted.
 * <p>
 * Usage: Call {@link #reset(StateChangeReason)} to reset to the {@code IDLE} state, which hides
 * the UI.<br>
 * Call {@link #enter(InternalState)} to enter a start-state (when a user gesture is recognized).
 * When doing some work on a state, which may be done in an asynchronous manner:<ol>
 * <li>call {@link #notifyStartingWorkOn(InternalState)} to note that work is starting on that state
 * <li>call {@link #notifyFinishedWorkOn(InternalState)} when work is completed.
 * <li>If a handler of an async response needs to do additional work, such as updating the UI, it
 * should first call {@link #isStillWorkingOn(InternalState)} to check that work has not been
 * interrupted since the async operation was started.
 * </ol><p>
 * The {@link #notifyFinishedWorkOn(InternalState)} method will automatically start a transition to
 * the appropriate next state.
 * <p>
 * Policy decisions about state transitions should only be done in the private
 * {@link #transitionTo(InternalState)} method of this class (not within the
 * {@code ContextualSearchManager} itself).
 */
class ContextualSearchInternalStateController {
    private static final String TAG = "ContextualSearch";

    private final ContextualSearchPolicy mPolicy;
    private final ContextualSearchInternalStateHandler mStateHandler;

    /**
     * The current internal state of the {@code ContextualSearchManager}.
     * States can be "start states" which can be passed to #enter(), or "transitional states" which
     * automatically transition to the appropriate next state when work is done on them, or
     * "resting states" which do not transition into any next state, or a combination of the
     * above.
     */
    public static enum InternalState {
        /** This start state should only be used when the manager is not yet initialized or already
         * destroyed.
         */
        UNDEFINED,
        /** This start/resting state shows no UI (panel is closed). */
        IDLE,

        /** This starts a transition that leads to the SHOWING_LONGPRESS_SEARCH resting state. */
        LONG_PRESS_RECOGNIZED,
        /** Resting state when showing the panel in response to a Long-press gesture. */
        SHOWING_LONGPRESS_SEARCH,

        /** This is a start state when the selection is cleared typically due to a tap on the base
         * page.  If the previous state wasn't IDLE then it could be a tap near a previous Tap.
         * Transitions to WAITING_FOR_POSSIBLE_TAP_NEAR_PREVIOUS to wait for a Tap and hide the Bar
         * if no tap ever happens. */
        SELECTION_CLEARED_RECOGNIZED,
        /** Waits to see if the tap gesture was valid so we can just update the Bar instead of
         * hiding/showing it. */
        WAITING_FOR_POSSIBLE_TAP_NEAR_PREVIOUS,

        /** This starts a sequence of states needed to get to the SHOWING_TAP_SEARCH resting state.
         */
        TAP_RECOGNIZED,
        /** Waits to see if the Tap was on a previous tap-selection, which will show the selection
         * manipulation pins and be subsumed by a LONG_PRESS_RECOGNIZED.  If that doesn't happen
         * within the waiting period we'll advance.
         */
        WAITING_FOR_POSSIBLE_TAP_ON_TAP_SELECTION,
        /** Gathers text surrounding the selection. */
        GATHERING_SURROUNDINGS,
        /** Decides if the gesture should trigger the UX or be suppressed. */
        DECIDING_SUPPRESSION,
        /** Start showing the Tap UI.  Currently this means select the word that was tapped. */
        START_SHOWING_TAP_UI,
        /** Show the full Tap UI.  Currently this means showing the Overlay Panel. */
        SHOW_FULL_TAP_UI,
        /** Resolving the Search Term using the surrounding text and additional context.
         * Currently this makes a server request, which could take a long time. */
        RESOLVING,
        /** Resting state when showing the panel in response to a Tap gesture. */
        SHOWING_TAP_SEARCH
    }

    // The current state of this instance.
    private InternalState mState;

    // Whether work has started on the current state.
    private boolean mDidStartWork;

    // The previous state of this instance.
    private InternalState mPreviousState;

    /**
     * Constructs an instance of this class, which has the same lifetime as the
     * {@code ContextualSearchManager} and the given parameters.
     */
    ContextualSearchInternalStateController(
            ContextualSearchPolicy policy, ContextualSearchInternalStateHandler stateHandler) {
        mPolicy = policy;
        mStateHandler = stateHandler;
    }

    // ============================================================================================
    // State-transition management.
    // This code is designed to solve several problems:
    // 1) Document the sequencing of handling a gesture in code.  Now there's a single method that
    //    determines the sequence that should be followed for Tap handling (our most complicated
    //    case.
    // 2) Document the initiation and subsequent notification/handling of operations.  Now the
    //    method that starts an operation and the notification handler are tied together by their
    //    references to the same state.  This allows a simple search to find the
    //    initiation and handler together (which is not always easy, e.g. SelectWordAroundCaret
    //    does not yet have an ACK so we infer that it's complete when the selection change -- or
    //    does not change after some short waiting period).
    // 3) Gracefully handle sequence interruptions.  When an asynchronous operation is in progress
    //    the user may start a new sequence or abort the current sequence.  Now the handler for an
    //    asynchronous operation can easily detect that it's no longer working on that operation
    //    and skip the normal completion of the operation.
    // ============================================================================================

    /**
     * Reset the current state to the IDLE state.
     * @param reason The reason for the reset.
     */
    void reset(StateChangeReason reason) {
        transitionTo(InternalState.IDLE, reason);
    }

    /**
     * Enters the given starting state immediately.
     * @param state The new starting {@link InternalState} we're now in.
     */
    void enter(InternalState state) {
        assert state == InternalState.UNDEFINED || state == InternalState.IDLE
                || state == InternalState.LONG_PRESS_RECOGNIZED
                || state == InternalState.TAP_RECOGNIZED
                || state == InternalState.SELECTION_CLEARED_RECOGNIZED;
        mPreviousState = mState;
        mState = state;

        notifyStartingWorkOn(mState);
        notifyFinishedWorkOn(mState);
    }

    /**
     * Confirms that work is starting on the given state.
     * @param state The {@link InternalState} that we're now working on.
     */
    void notifyStartingWorkOn(InternalState state) {
        assert mState == state;
        mDidStartWork = true;
    }

    /**
     * @return Whether we're still working on the given state.
     */
    boolean isStillWorkingOn(InternalState state) {
        return mState == state;
    }

    /**
     * Confirms that work has been finished on the given state.
     * This should be called by every operation that waits for some kind of completion when it
     * completes.  The operation's start must be flagged using {@link #notifyStartingWorkOn}.
     * @param state The {@link InternalState} that we've finished working on.
     */
    void notifyFinishedWorkOn(InternalState state) {
        finishWorkingOn(state);
    }

    /**
     * @return The current internal state for testing purposes.
     */
    @VisibleForTesting
    protected InternalState getState() {
        return mState;
    }

    /**
     * Establishes the given state by calling code that starts work on that state.
     * @param state The new {@link InternalState} to establish.
     */
    private void transitionTo(InternalState state) {
        transitionTo(state, null);
    }

    /**
     * Establishes the given state by calling code that starts work on that state or simply
     * displays the appropriate UX for that state.
     * @param state The new {@link InternalState} to establish.
     * @param reason The reason we're starting this state, or {@code null} if not significant
     *        or known.  Only needed when we enter the IDLE state.
     */
    private void transitionTo(final InternalState state, @Nullable final StateChangeReason reason) {
        if (state == mState) return;

        // This should be the only part of the code that changes the state (other than #enter)!
        mPreviousState = mState;
        mState = state;

        mDidStartWork = false;
        startWorkingOn(state, reason);
    }

    /**
     * Starts working on the given state by calling code that starts work on that state or simply
     * displays the appropriate UX for that state.
     * @param state The new {@link InternalState} to establish.
     * @param reason The reason we're starting this state, or {@code null} if not significant
     *        or known.  Only needed when we enter the IDLE state.
     */
    private void startWorkingOn(InternalState state, @Nullable StateChangeReason reason) {
        switch (state) {
            case IDLE:
                assert reason != null;
                mStateHandler.hideContextualSearchUi(reason);
                break;

            case LONG_PRESS_RECOGNIZED:
                break;
            case SHOWING_LONGPRESS_SEARCH:
                mStateHandler.showContextualSearchLongpressUi();
                break;

            case WAITING_FOR_POSSIBLE_TAP_NEAR_PREVIOUS:
                mStateHandler.waitForPossibleTapNearPrevious();
                break;
            case TAP_RECOGNIZED:
                break;
            case WAITING_FOR_POSSIBLE_TAP_ON_TAP_SELECTION:
                mStateHandler.waitForPossibleTapOnTapSelection();
                break;
            case GATHERING_SURROUNDINGS:
                mStateHandler.gatherSurroundingText();
                break;
            case DECIDING_SUPPRESSION:
                mStateHandler.decideSuppression();
                break;
            case START_SHOWING_TAP_UI:
                mStateHandler.startShowingTapUi();
                break;
            case SHOW_FULL_TAP_UI:
                mStateHandler.showContextualSearchTapUi();
                break;
            case RESOLVING:
                mStateHandler.resolveSearchTerm();
                break;
            case SHOWING_TAP_SEARCH:
                break;
            default:
                Log.w(TAG, "Warning: unexpected startWorkingOn " + state.toString());
                break;
        }
    }

    /**
     * Finishes working on the given state by making a transition to the next state if needed.
     * @param state The {@link InternalState} that we've finished working on.
     */
    private void finishWorkingOn(InternalState state) {
        // When an async task finishes work some action may have caused a reset and now we're
        // in a new sequence, so no need to finish work on the abandoned state.
        if (state != mState) return;

        // Should have called #nofifyStartingWorkOn this state already.
        assert mDidStartWork;

        if (mState == InternalState.IDLE || mState == InternalState.UNDEFINED) {
            Log.w(TAG, "Warning, the " + state.toString() + " state was aborted.");
            return;
        }

        switch (state) {
            case LONG_PRESS_RECOGNIZED:
                transitionTo(InternalState.GATHERING_SURROUNDINGS);
                break;
            case SHOWING_LONGPRESS_SEARCH:
                break;
            case SELECTION_CLEARED_RECOGNIZED:
                if (mPreviousState != null && mPreviousState != InternalState.IDLE) {
                    transitionTo(InternalState.WAITING_FOR_POSSIBLE_TAP_NEAR_PREVIOUS);
                } else {
                    reset(StateChangeReason.BASE_PAGE_TAP);
                }
                break;
            case WAITING_FOR_POSSIBLE_TAP_NEAR_PREVIOUS:
                // If a tap near the previous was detected we've started another sequence and won't
                // get here.  So we know the wait completed without any other action so we need to
                // reset the UX.
                reset(StateChangeReason.BASE_PAGE_TAP);
                break;
            case TAP_RECOGNIZED:
                if (mPreviousState != null && mPreviousState != InternalState.IDLE) {
                    transitionTo(InternalState.WAITING_FOR_POSSIBLE_TAP_ON_TAP_SELECTION);
                } else {
                    transitionTo(InternalState.GATHERING_SURROUNDINGS);
                }
                break;
            case WAITING_FOR_POSSIBLE_TAP_ON_TAP_SELECTION:
                transitionTo(InternalState.GATHERING_SURROUNDINGS);
                break;
            case GATHERING_SURROUNDINGS:
                // We gather surroundings for both Tap and Long-press in order to notify icing.
                if (mPreviousState == InternalState.LONG_PRESS_RECOGNIZED) {
                    transitionTo(InternalState.SHOWING_LONGPRESS_SEARCH);
                } else {
                    transitionTo(InternalState.DECIDING_SUPPRESSION);
                }
                break;
            case DECIDING_SUPPRESSION:
                transitionTo(InternalState.START_SHOWING_TAP_UI);
                break;
            case START_SHOWING_TAP_UI:
                transitionTo(InternalState.SHOW_FULL_TAP_UI);
                break;
            case SHOW_FULL_TAP_UI:
                if (mPolicy.shouldPreviousTapResolve()) {
                    transitionTo(InternalState.RESOLVING);
                } else {
                    transitionTo(InternalState.SHOWING_TAP_SEARCH);
                }
                break;
            case RESOLVING:
                transitionTo(InternalState.SHOWING_TAP_SEARCH);
                break;
            default:
                Log.e(TAG, "The state " + state.toString() + " is not transitional!");
                assert false;
        }
    }
}
