// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

cr.exportPath('chrome.sync.traffic_log_tab', new class {
  constructor() {
    this.protocolEvents = [];
    this.knownEventTimestamps = new Set();
  }

  /**
   * Helper to determine if the window is scrolled to its bottom limit.
   * @return {boolean} true if the container is scrolled to the bottom
   * @private
   */
  _isScrolledToBottom() {
    return (window.innerHeight + window.scrollY) >= document.body.offsetHeight;
  }

  /**
   * Helper to scroll the window to its bottom.
   * @private
   */
  _scrollToBottom() {
    window.scrollTo(0, document.body.scrollHeight);
  }

  /**
   * Callback for incoming protocol events.
   * @param {Event} e The protocol event.
   * @private
   */
  _onReceivedProtocolEvent(e) {
    var details = e.details;

    if (this.knownEventTimestamps.has(details.time)) {
      return;
    }

    this.knownEventTimestamps.add(details.time);
    this.protocolEvents.push(details);

    var shouldScrollDown = this._isScrolledToBottom();

    jstProcess(
      new JsEvalContext({ events: this.protocolEvents }),
      this.container);

    if (shouldScrollDown) {
      this._scrollToBottom();
    }
  }

  /**
   * Toggles the given traffic event entry div's "expanded" state.
   * @param {MouseEvent} e
   * @private
   */
  _expandListener(e) {
    e.target.classList.toggle('traffic-event-entry-expanded-fullscreen');
  }

  /**
   * Attaches a listener to the given traffic event entry div.
   * @param {HTMLElement} element
   */
  addExpandListener(element) {
    element.addEventListener('click', this._expandListener, false);
  }

  onLoad() {
    this.container = $('traffic-event-fullscreen-container');

    chrome.sync.events.addEventListener(
      'onProtocolEvent', this._onReceivedProtocolEvent.bind(this));

    // Make the prototype jscontent element disappear.
    jstProcess({}, this.container);
  }
});

document.addEventListener(
  'DOMContentLoaded',
  chrome.sync.traffic_log_tab.onLoad.bind(chrome.sync.traffic_log_tab),
  false);
