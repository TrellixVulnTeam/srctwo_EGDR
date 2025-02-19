// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Account picker screen implementation.
 */

login.createScreen('AccountPickerScreen', 'account-picker', function() {
  /**
   * Maximum number of offline login failures before online login.
   * @type {number}
   * @const
   */
  var MAX_LOGIN_ATTEMPTS_IN_POD = 3;

  return {
    EXTERNAL_API: [
      'loadUsers',
      'runAppForTesting',
      'setApps',
      'setShouldShowApps',
      'showAppError',
      'updateUserImage',
      'setCapsLockState',
      'forceLockedUserPodFocus',
      'removeUser',
      'showBannerMessage',
      'showUserPodCustomIcon',
      'hideUserPodCustomIcon',
      'setUserPodFingerprintIcon',
      'removeUserPodFingerprintIcon',
      'setPinEnabledForUser',
      'setAuthType',
      'setTabletModeState',
      'setPublicSessionDisplayName',
      'setPublicSessionLocales',
      'setPublicSessionKeyboardLayouts',
      'setLockScreenAppsState',
      'setOverlayColors',
    ],

    preferredWidth_: 0,
    preferredHeight_: 0,

    // Whether this screen is shown for the first time.
    firstShown_: true,

    // Whether this screen is currently being shown.
    showing_: false,

    // Last reported lock screen app activity state.
    lockScreenAppsState_: LOCK_SCREEN_APPS_STATE.NONE,

    /** @override */
    decorate: function() {
      login.PodRow.decorate($('pod-row'));
    },

    /** @override */
    getPreferredSize: function() {
      return {width: this.preferredWidth_, height: this.preferredHeight_};
    },

    /** @override */
    onWindowResize: function() {
      $('pod-row').onWindowResize();
    },

    /**
     * Sets preferred size for account picker screen.
     */
    setPreferredSize: function(width, height) {
      this.preferredWidth_ = width;
      this.preferredHeight_ = height;
    },

    /**
      * Sets login screen overlay colors based on colors extracted from the
      * wallpaper.
      * @param {string} maskColor Color for the gradient mask.
      * @param {string} scrollColor Color for the small pods container.
      */
     setOverlayColors: function(maskColor, scrollColor) {
      $('pod-row').setOverlayColors(maskColor, scrollColor);
     },

    /**
     * When the account picker is being used to lock the screen, pressing the
     * exit accelerator key will sign out the active user as it would when
     * they are signed in.
     */
    exit: function() {
      // Check and disable the sign out button so that we can never have two
      // sign out requests generated in a row.
      if ($('pod-row').lockedPod && !$('sign-out-user-button').disabled) {
        $('sign-out-user-button').disabled = true;
        chrome.send('signOutUser');
      }
    },

    /* Cancel user adding if ESC was pressed.
     */
    cancel: function() {
      if (Oobe.getInstance().displayType == DISPLAY_TYPE.USER_ADDING)
        chrome.send('cancelUserAdding');
    },

    /**
     * Event handler that is invoked just after the frame is shown.
     * @param {string} data Screen init payload.
     */
    onAfterShow: function(data) {
      $('pod-row').handleAfterShow();
    },

    /**
     * Event handler that is invoked just before the frame is shown.
     * @param {string} data Screen init payload.
     */
    onBeforeShow: function(data) {
      this.showing_ = true;

      this.ownerDocument.addEventListener('click',
          this.handleOwnerDocClick_.bind(this));

      chrome.send('loginUIStateChanged', ['account-picker', true]);
      $('login-header-bar').signinUIState = SIGNIN_UI_STATE.ACCOUNT_PICKER;
      chrome.send('hideCaptivePortal');
      var podRow = $('pod-row');
      podRow.handleBeforeShow();

      // In case of the preselected pod onShow will be called once pod
      // receives focus.
      if (!podRow.preselectedPod)
        this.onShow();
    },

    /**
     * Event handler invoked when the page is shown and ready.
     */
    onShow: function() {
      if (!this.showing_) {
        // This method may be called asynchronously when the pod row finishes
        // initializing. However, at that point, the screen may have been hidden
        // again already. If that happens, ignore the onShow() call.
        return;
      }
      chrome.send('getTabletModeState');
      if (!this.firstShown_) return;
      this.firstShown_ = false;

      // Ensure that login is actually visible.
      window.requestAnimationFrame(function() {
        chrome.send('accountPickerReady');
        chrome.send('loginVisible', ['account-picker']);
      });
    },

    /**
     * Event handler that is invoked just before the frame is hidden.
     */
    onBeforeHide: function() {
      $('pod-row').clearFocusedPod();
      this.showing_ = false;
      chrome.send('loginUIStateChanged', ['account-picker', false]);
      $('login-header-bar').signinUIState = SIGNIN_UI_STATE.HIDDEN;
      $('pod-row').handleHide();
    },

    /**
     * Shows sign-in error bubble.
     * @param {number} loginAttempts Number of login attemps tried.
     * @param {HTMLElement} error Error to show in bubble.
     */
    showErrorBubble: function(loginAttempts, error) {
      var activatedPod = $('pod-row').activatedPod;
      if (!activatedPod) {
        $('bubble').showContentForElement($('pod-row'),
                                          cr.ui.Bubble.Attachment.RIGHT,
                                          error);
        return;
      }
      // Show web authentication if this is not a supervised user.
      if (loginAttempts > MAX_LOGIN_ATTEMPTS_IN_POD &&
          !activatedPod.user.supervisedUser) {
        chrome.send('maxIncorrectPasswordAttempts',
            [activatedPod.user.emailAddress]);
        activatedPod.showSigninUI();
      } else {
        if (loginAttempts == 1) {
          chrome.send('firstIncorrectPasswordAttempt',
              [activatedPod.user.emailAddress]);
        }
        // Update the pod row display if incorrect password.
        $('pod-row').setFocusedPodErrorDisplay(true);
        activatedPod.showBubble(error);
      }
    },

   /**
     * Loads given users in pod row.
     * @param {array} users Array of user.
     * @param {boolean} showGuest Whether to show guest session button.
     */
    loadUsers: function(users, showGuest) {
      $('pod-row').loadPods(users);
      $('login-header-bar').showGuestButton = showGuest;
      // On Desktop, #login-header-bar has a shadow if there are 8+ profiles.
      if (Oobe.getInstance().displayType == DISPLAY_TYPE.DESKTOP_USER_MANAGER)
        $('login-header-bar').classList.toggle('shadow', users.length > 8);
    },

    /**
     * Runs app with a given id from the list of loaded apps.
     * @param {!string} app_id of an app to run.
     * @param {boolean=} opt_diagnostic_mode Whether to run the app in
     *     diagnostic mode.  Default is false.
     */
    runAppForTesting: function(app_id, opt_diagnostic_mode) {
      $('pod-row').findAndRunAppForTesting(app_id, opt_diagnostic_mode);
    },

    /**
     * Adds given apps to the pod row.
     * @param {array} apps Array of apps.
     */
    setApps: function(apps) {
      $('pod-row').setApps(apps);
    },

    /**
     * Sets the flag of whether app pods should be visible.
     * @param {boolean} shouldShowApps Whether to show app pods.
     */
    setShouldShowApps: function(shouldShowApps) {
      $('pod-row').setShouldShowApps(shouldShowApps);
    },

    /**
     * Shows the given kiosk app error message.
     * @param {!string} message Error message to show.
     */
    showAppError: function(message) {
      // TODO(nkostylev): Figure out a way to show kiosk app launch error
      // pointing to the kiosk app pod.
      /** @const */ var BUBBLE_PADDING = 12;
      $('bubble').showTextForElement($('pod-row'),
                                     message,
                                     cr.ui.Bubble.Attachment.BOTTOM,
                                     $('pod-row').offsetWidth / 2,
                                     BUBBLE_PADDING);
    },

    /**
     * Updates current image of a user.
     * @param {string} username User for which to update the image.
     */
    updateUserImage: function(username) {
      $('pod-row').updateUserImage(username);
    },

    /**
     * Updates Caps Lock state (for Caps Lock hint in password input field).
     * @param {boolean} enabled Whether Caps Lock is on.
     */
    setCapsLockState: function(enabled) {
      $('pod-row').classList.toggle('capslock-on', enabled);
    },

    /**
     * Enforces focus on user pod of locked user.
     */
    forceLockedUserPodFocus: function() {
      var row = $('pod-row');
      if (row.lockedPod)
        row.focusPod(row.lockedPod, true);
    },

    /**
     * Remove given user from pod row if it is there.
     * @param {string} user name.
     */
    removeUser: function(username) {
      $('pod-row').removeUserPod(username);
    },

    /**
     * Displays a banner containing |message|. If the banner is already present
     * this function updates the message in the banner. This function is used
     * by the chrome.screenlockPrivate.showMessage API.
     * @param {string} message Text to be displayed or empty to hide the banner.
     */
    showBannerMessage: function(message) {
      $('pod-row').showBannerMessage(message);
    },

    /**
     * Shows a custom icon in the user pod of |username|. This function
     * is used by the chrome.screenlockPrivate API.
     * @param {string} username Username of pod to add button
     * @param {!{id: !string,
     *           hardlockOnClick: boolean,
     *           isTrialRun: boolean,
     *           tooltip: ({text: string, autoshow: boolean} | undefined)}} icon
     *     The icon parameters.
     */
    showUserPodCustomIcon: function(username, icon) {
      $('pod-row').showUserPodCustomIcon(username, icon);
    },

    /**
     * Hides the custom icon in the user pod of |username| added by
     * showUserPodCustomIcon(). This function is used by the
     * chrome.screenlockPrivate API.
     * @param {string} username Username of pod to remove button
     */
    hideUserPodCustomIcon: function(username) {
      $('pod-row').hideUserPodCustomIcon(username);
    },

    /**
     * Set a fingerprint icon in the user pod of |username|.
     * @param {string} username Username of the selected user
     * @param {number} state Fingerprint unlock state
     */
    setUserPodFingerprintIcon: function(username, state) {
      $('pod-row').setUserPodFingerprintIcon(username, state);
    },

    /**
     * Removes the fingerprint icon in the user pod of |username|.
     * @param {string} username Username of the selected user.
     */
    removeUserPodFingerprintIcon: function(username) {
      $('pod-row').removeUserPodFingerprintIcon(username);
    },

    /**
     * Sets the authentication type used to authenticate the user.
     * @param {string} username Username of selected user
     * @param {number} authType Authentication type, must be a valid value in
     *                          the AUTH_TYPE enum in user_pod_row.js.
     * @param {string} value The initial value to use for authentication.
     */
    setAuthType: function(username, authType, value) {
      $('pod-row').setAuthType(username, authType, value);
    },

    /**
     * Sets the state of tablet mode.
     * @param {boolean} isTabletModeEnabled true if the mode is on.
     */
    setTabletModeState: function(isTabletModeEnabled) {
      $('pod-row').setTabletModeState(isTabletModeEnabled);
    },

    /**
     * Enables or disables the pin keyboard for the given user. This may change
     * pin keyboard visibility.
     * @param {!string} user
     * @param {boolean} enabled
     */
    setPinEnabledForUser: function(user, enabled) {
      $('pod-row').setPinEnabled(user, enabled);
    },

    /**
     * Updates the display name shown on a public session pod.
     * @param {string} userID The user ID of the public session
     * @param {string} displayName The new display name
     */
    setPublicSessionDisplayName: function(userID, displayName) {
      $('pod-row').setPublicSessionDisplayName(userID, displayName);
    },

    /**
     * Updates the list of locales available for a public session.
     * @param {string} userID The user ID of the public session
     * @param {!Object} locales The list of available locales
     * @param {string} defaultLocale The locale to select by default
     * @param {boolean} multipleRecommendedLocales Whether |locales| contains
     *     two or more recommended locales
     */
    setPublicSessionLocales: function(userID,
                                      locales,
                                      defaultLocale,
                                      multipleRecommendedLocales) {
      $('pod-row').setPublicSessionLocales(userID,
                                           locales,
                                           defaultLocale,
                                           multipleRecommendedLocales);
    },

    /**
     * Updates the list of available keyboard layouts for a public session pod.
     * @param {string} userID The user ID of the public session
     * @param {string} locale The locale to which this list of keyboard layouts
     *     applies
     * @param {!Object} list List of available keyboard layouts
     */
    setPublicSessionKeyboardLayouts: function(userID, locale, list) {
      $('pod-row').setPublicSessionKeyboardLayouts(userID, locale, list);
    },

    /**
     * Updates UI based on the provided lock screen apps state.
     *
     * @param {LOCK_SCREEN_APPS_STATE} state The current lock screen apps state.
     */
    setLockScreenAppsState: function(state) {
      if (Oobe.getInstance().displayType != DISPLAY_TYPE.LOCK ||
          state == this.lockScreenAppsState_) {
        return;
      }

      var wasForeground =
          this.lockScreenAppsState_ === LOCK_SCREEN_APPS_STATE.FOREGROUND;
      this.lockScreenAppsState_ = state;

      $('login-header-bar').lockScreenAppsState = state;
      $('top-header-bar').lockScreenAppsState = state;

      // Reset the focused pod if app window is being shown on top of the user
      // pods. Main goal is to clear any credentials the user might have input.
      if (state === LOCK_SCREEN_APPS_STATE.FOREGROUND) {
        $('pod-row').clearFocusedPod();
        $('pod-row').disabled = true;
      } else {
        $('pod-row').disabled = false;
        if (wasForeground) {
          // If the app window was moved to background, ensure the active pod is
          // focused.
          $('pod-row').maybePreselectPod();
          $('pod-row').refocusCurrentPod();
        }
      }
    },

    /**
     * Handles clicks on the document which displays the account picker UI.
     * If the click event target is outer container - i.e. background portion of
     * UI with no other UI elements, and lock screen apps are in background, a
     * request is issued to chrome to move lock screen apps to foreground.
     * @param {Event} event The click event.
     */
    handleOwnerDocClick_: function(event) {
      if (this.lockScreenAppsState_ != LOCK_SCREEN_APPS_STATE.BACKGROUND ||
          (event.target != $('account-picker') &&
           event.target != $('version'))) {
        return;
      }
      chrome.send('setLockScreenAppsState',
                  [LOCK_SCREEN_APPS_STATE.FOREGROUND]);

      event.preventDefault();
      event.stopPropagation();
    },
  };
});
