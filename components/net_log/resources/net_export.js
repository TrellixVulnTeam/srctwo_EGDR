// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Main entry point called once the page has loaded.
 */
function onLoad() {
  NetExportView.getInstance();
}

document.addEventListener('DOMContentLoaded', onLoad);

/**
 * This class handles the presentation of the net-export view. Used as a
 * singleton.
 */
var NetExportView = (function() {
  'use strict';

  // --------------------------------------------------------------------------

  var kIdStateDivUninitialized = 'state-uninitialized';
  var kIdStateDivInitial = 'state-initial';
  var kIdStateDivLogging = 'state-logging';
  var kIdStateDivStopped = 'state-stopped';
  var kIdStartLoggingButton = 'start-logging';
  var kIdStopLoggingButton = 'stop-logging';
  var kIdEmailLogButton = 'mobile-email';
  var kIdShowFileButton = 'show-file';
  var kIdCaptureModeLogging = 'capture-mode-logging';
  var kIdFilePathLogging = 'file-path-logging';
  var kIdCaptureModeStopped = 'capture-mode-stopped';
  var kIdFilePathStoppedLogging = 'file-path-stopped';
  var kIdStartOverButton = 'startover';
  var kIdPrivacyReadMoreLink = 'privacy-read-more-link';
  var kIdPrivacyReadMoreDiv = 'privacy-read-more'
  var kIdTooBigReadMoreLink = 'toobig-read-more-link';
  var kIdTooBigReadMoreDiv = 'toobig-read-more'
  var kIdLogMaxFileSizeInput = 'log-max-filesize'

  /**
   * @constructor
   */
  function NetExportView() {
    // Tell NetExportMessageHandler to notify the UI of future state changes
    // from this point on (through onExportNetLogInfoChanged()).
    chrome.send('enableNotifyUIWithState');

    this.infoForLoggedFile_ = null;
  }

  cr.addSingletonGetter(NetExportView);

  NetExportView.prototype = {
    /**
     * Starts saving NetLog data to a file.
     */
    onStartLogging_: function() {
      // Determine the capture mode to use.
      var logMode =
          document.querySelector('input[name="log-mode"]:checked').value;

      // Determine the maximum file size, as the number of bytes (or -1 to mean
      // no limit)
      var maxLogFileSizeBytes = -1;
      var fileSizeString = $(kIdLogMaxFileSizeInput).value;
      var numMegabytes = parseFloat(fileSizeString);
      if (!isNaN(numMegabytes)) {
        // Convert to an integral number of bytes.
        maxLogFileSizeBytes = Math.round(numMegabytes * 1024 * 1024);
      }

      chrome.send('startNetLog', [logMode, maxLogFileSizeBytes]);
    },

    /**
     * Stops saving NetLog data to a file.
     */
    onStopLogging_: function() {
      chrome.send('stopNetLog');
    },

    /**
     * Sends NetLog data via email from browser (mobile only).
     */
    onSendEmail_: function() {
      chrome.send('sendNetLog');
    },

    /**
     * Reveals the log file in the shell (i.e. selects it in the Finder on
     * Mac).
     */
    onShowFile_: function() {
      chrome.send('showFile');
    },

    /**
     * Transitions back to the "Start logging to disk" state.
     */
    onStartOver_: function() {
      this.infoForLoggedFile_ = null;
      this.renderInitial_();
    },

    /**
     * Updates the UI to reflect the current state. The state transitions are
     * sent by the browser controller (NetLogFileWriter):
     *
     *   * UNINITIALIZED - This is the initial state when net-export is opened
     *         for the first time, or there was an error during initialization.
     *         This state is short-lived and likely not observed; will
     *         immediately transition to INITIALIZING).
     *
     *   * INITIALIZING - On desktop UI this is pretty much a no-op. On the
     *         mobile UI, this state is when the controller checks the disk for
     *         a previous net-log file (from past run of the browser). After
     *         success will transition to NOT_LOGGING. On failure will
     *         transition to UNINITIALIZED (rare).
     *
     *   * NOT_LOGGING - This is the steady-state. It means initialization
     *        completed and we are not currently logging. Being in this state
     *        either means:
     *         (1) We were logging and then the user stopped (earlier states
     *             were STATE_LOGGING / STATE_STOPPING_LOG).
     *         (2) We have never started logging (previous state was
     *             INITIALIZING).
     *
     *   * STARTING_LOG - This state means the user has clicked the "Start log"
     *        button and logging is about to get started (files may not have
     *        been created yet).
     *
     *   * LOGGING - This state means that logging is currently in progress.
     *        The destination path of the log, and the capture mode are known
     *        and will be reflected in the parameters.
     *
     *   * STOPPING_LOG - This state means the user has clicked the "Stop
     *        logging" button, and the log file is in the process of being
     *        finalized. Once the state transitions to NOT_LOGGING then the log
     *        is complete, and can safely be copied/emailed.
     */
    onExportNetLogInfoChanged: function(info) {
      switch (info.state) {
        case 'UNINITIALIZED':
        case 'INITIALIZING':
          this.renderUninitialized_();
          break;

        case 'NOT_LOGGING':
          if (this.infoForLoggedFile_) {
            // There is no "stopped logging" state. We manufacture that in the
            // UI in response to a transition from LOGGING --> NOT_LOGGING.
            this.renderStoppedLogging_(this.infoForLoggedFile_);

            // TODO(eroman): prevent future state transitions. In desktop UI
            // could start logging in a new tab, and it would reset this one.
          } else if (info.logExists) {
            // In the mobile UI, initialization may have found a
            // pre-existing log file.
            this.renderStoppedLogging_(info);
          } else {
            this.renderInitial_();
          }
          break;

        case 'STARTING_LOG':
          // This is a short-lived state, no need to do anything special.
          // Disabling the buttons would be nice, however it is not crucial as
          // the controller will reject commands while in this state anyway.
          this.renderInitial_();
          break;

        case 'LOGGING':
          // Cache the last information for this logging session, so once
          // logging is stopped will know what path information to display.
          this.infoForLoggedFile_ = info;
          this.renderLogging_(info);
          break;

        case 'STOPPING_LOG':
          // This is a short-lived state, no need to do anything special.
          this.renderLogging_(info);
          break;
      }
    },

    /**
     * Updates the UI to display the "uninitialized" state. This is only
     * visible for a short period of time, or longer if initialization failed
     * (and didn't transition to a different state).
     */
    renderUninitialized_: function() {
      this.showStateDiv_(kIdStateDivUninitialized);
    },

    /**
     * Updates the UI to display the "initial" state. This is the state when
     * logging has not been started yet, and there are controls to start
     * logging.
     */
    renderInitial_: function() {
      this.showStateDiv_(kIdStateDivInitial);
      $(kIdStartLoggingButton).onclick = this.onStartLogging_.bind(this);
    },

    /**
     * Updates the UI to display the "logging" state. This is the state while
     * capturing is in progress and being written to disk.
     */
    renderLogging_: function(info) {
      this.showStateDiv_(kIdStateDivLogging);

      $(kIdStopLoggingButton).onclick = this.onStopLogging_.bind(this);
      $(kIdCaptureModeLogging).textContent = this.getCaptureModeText_(info);
      $(kIdFilePathLogging).textContent = info.file;
    },

    /*
     * Updates the UI to display the state when logging has stopped.
     */
    renderStoppedLogging_: function(info) {
      this.showStateDiv_(kIdStateDivStopped);

      // The email button is only available in the mobile UI.
      if ($(kIdEmailLogButton))
        $(kIdEmailLogButton).onclick = this.onSendEmail_.bind(this);
      // The show file button is only available in the desktop UI.
      if ($(kIdShowFileButton))
        $(kIdShowFileButton).onclick = this.onShowFile_.bind(this);
      $(kIdStartOverButton).onclick = this.onStartOver_.bind(this);

      $(kIdFilePathStoppedLogging).textContent = info.file;

      $(kIdCaptureModeStopped).textContent = this.getCaptureModeText_(info);

      // Hook up the "read more..." link for privacy information.
      $(kIdPrivacyReadMoreLink).onclick =
          this.showPrivacyReadMore_.bind(this, true);
      this.showPrivacyReadMore_(false);

      // Hook up the "read more..." link for reducing log size information.
      $(kIdTooBigReadMoreLink).onclick =
          this.showTooBigReadMore_.bind(this, true);
      this.showTooBigReadMore_(false);
    },

    /**
     * Gets the textual label for a capture mode from the HTML.
     */
    getCaptureModeText_: function(info) {
      // TODO(eroman): Should not hardcode "Unknown" (will not work properly if
      //               the HTML is internationalized).
      if (!info.logCaptureModeKnown)
        return "Unknown";

      var radioButton = document.querySelector(
          'input[name="log-mode"][value="' + info.captureMode + '"]');
      if (!radioButton)
        return 'Unknown';
      return radioButton.parentElement.textContent;
    },

    showPrivacyReadMore_: function(show) {
      $(kIdPrivacyReadMoreDiv).hidden = !show;
      $(kIdPrivacyReadMoreLink).hidden = show;
    },

    showTooBigReadMore_: function(show) {
      $(kIdTooBigReadMoreDiv).hidden = !show;
      $(kIdTooBigReadMoreLink).hidden = show;
    },

    showStateDiv_: function(divId) {
      var kAllDivIds = [
        kIdStateDivUninitialized,
        kIdStateDivInitial,
        kIdStateDivLogging,
        kIdStateDivStopped
      ];

      for (var curDivId of kAllDivIds) {
        $(curDivId).hidden = divId != curDivId;
      }
    },
  };

  return NetExportView;
})();
