/* Copyright 2017 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

Logs = {
  controller_: null,

  /**
   * Initializes the logs UI.
   */
  init: function() {
    Logs.controller_ = new LogsListController();

    var clearLogsButton = document.getElementById('clear-logs-button');
    clearLogsButton.onclick = function() {
      WebUI.clearLogs();
    };

    var copyLogsButton = document.getElementById('copy-logs-button');
    copyLogsButton.onclick = () => {
      this.copyLogsToClipboard();
    };

    WebUI.getLogMessages();
  },

  copyLogsToClipboard: function() {
    window.getSelection().selectAllChildren(
        document.getElementById('logs-list'));
    document.execCommand('copy');
    window.getSelection().removeAllRanges();
  },
};

/**
 * Interface with the native WebUI component for LogBuffer events. The functions
 * contained in this object will be invoked by the browser for each operation
 * performed on the native LogBuffer.
 */
LogBufferInterface = {
  /**
   * Called when a new log message is added.
   */
  onLogMessageAdded: function(log) {
    if (Logs.controller_) {
      Logs.controller_.add(log);
    }
  },

  /**
   * Called when the log buffer is cleared.
   */
  onLogBufferCleared: function() {
    if (Logs.controller_) {
      Logs.controller_.clear();
    }
  },

  /**
   * Called in response to chrome.send('getLogMessages') with the log messages
   * currently in the buffer.
   */
  onGotLogMessages: function(messages) {
    if (Logs.controller_) {
      Logs.controller_.set(messages);
    }
  },
};

/**
 * Controller for the logs list element, updating it based on user input and
 * logs received from native code.
 */
class LogsListController {
  constructor() {
    this.logsList_ = document.getElementById('logs-list');
    this.itemTemplate_ = document.getElementById('item-template');
    this.shouldSnapToBottom_ = true;

    this.logsList_.onscroll = this.onScroll_.bind(this);
  }

  /**
   * Listener for scroll event of the logs list element, used for snap to bottom
   * logic.
   */
  onScroll_() {
    const list = this.logsList_;
    this.shouldSnapToBottom_ =
        list.scrollTop + list.offsetHeight == list.scrollHeight;
  }

  /**
   * Clears all log items from the logs list.
   */
  clear() {
    var items = this.logsList_.querySelectorAll('.log-item');
    for (var i = 0; i < items.length; ++i) {
      items[i].remove();
    }
    this.shouldSnapToBottom_ = true;
  }

  /**
   * Adds a log to the logs list.
   */
  add(log) {
    var directories = log.file.split('/');
    var source = directories[directories.length - 1] + ':' + log.line;

    var t = this.itemTemplate_.content;
    t.querySelector('.log-item').attributes.severity.value = log.severity;
    t.querySelector('.item-time').textContent = log.time;
    t.querySelector('.item-source').textContent = source;
    t.querySelector('.item-text').textContent = log.text;

    var newLogItem = document.importNode(this.itemTemplate_.content, true);
    this.logsList_.appendChild(newLogItem);
    if (this.shouldSnapToBottom_) {
      this.logsList_.scrollTop = this.logsList_.scrollHeight;
    }
  }

  /**
   * Initializes the log list from an array of logs.
   */
  set(logs) {
    this.clear();
    for (var i = 0; i < logs.length; ++i) {
      this.add(logs[i]);
    }
  }
}
