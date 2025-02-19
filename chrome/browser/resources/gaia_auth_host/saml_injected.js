// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * Script to be injected into SAML provider pages, serving three main purposes:
 * 1. Signal hosting extension that an external page is loaded so that the
 *    UI around it should be changed accordingly;
 * 2. Provide an API via which the SAML provider can pass user credentials to
 *    Chrome OS, allowing the password to be used for encrypting user data and
 *    offline login.
 * 3. Scrape password fields, making the password available to Chrome OS even if
 *    the SAML provider does not support the credential passing API.
 */

(function() {
function APICallForwarder() {}

/**
 * The credential passing API is used by sending messages to the SAML page's
 * |window| object. This class forwards API calls from the SAML page to a
 * background script and API responses from the background script to the SAML
 * page. Communication with the background script occurs via a |Channel|.
 */
APICallForwarder.prototype = {
  // Channel to which API calls are forwarded.
  channel_: null,

  /**
   * Initialize the API call forwarder.
   * @param {!Object} channel Channel to which API calls should be forwarded.
   */
  init: function(channel) {
    this.channel_ = channel;
    this.channel_.registerMessage(
        'apiResponse', this.onAPIResponse_.bind(this));

    window.addEventListener('message', this.onMessage_.bind(this));
  },

  onMessage_: function(event) {
    if (event.source != window || typeof event.data != 'object' ||
        !event.data.hasOwnProperty('type') ||
        event.data.type != 'gaia_saml_api') {
      return;
    }
    // Forward API calls to the background script.
    this.channel_.send({name: 'apiCall', call: event.data.call});
  },

  onAPIResponse_: function(msg) {
    // Forward API responses to the SAML page.
    window.postMessage(
        {type: 'gaia_saml_api_reply', response: msg.response}, '/');
  }
};

/**
 * A class to scrape password from type=password input elements under a given
 * docRoot and send them back via a Channel.
 */
function PasswordInputScraper() {}

PasswordInputScraper.prototype = {
  // URL of the page.
  pageURL_: null,

  // Channel to send back changed password.
  channel_: null,

  // An array to hold password fields.
  passwordFields_: null,

  // An array to hold cached password values.
  passwordValues_: null,

  // A MutationObserver to watch for dynamic password field creation.
  passwordFieldsObserver: null,

  /**
   * Initialize the scraper with given channel and docRoot. Note that the
   * scanning for password fields happens inside the function and does not
   * handle DOM tree changes after the call returns.
   * @param {!Object} channel The channel to send back password.
   * @param {!string} pageURL URL of the page.
   * @param {!HTMLElement} docRoot The root element of the DOM tree that
   *     contains the password fields of interest.
   */
  init: function(channel, pageURL, docRoot) {
    this.pageURL_ = pageURL;
    this.channel_ = channel;

    this.passwordFields_ = [];
    this.passwordValues_ = [];

    this.findAndTrackChildren(docRoot);

    this.passwordFieldsObserver = new MutationObserver(function(mutations) {
      mutations.forEach(function(mutation) {
        Array.prototype.forEach.call(mutation.addedNodes, function(addedNode) {
          if (addedNode.nodeType != Node.ELEMENT_NODE)
            return;

          if (addedNode.matches('input[type=password]')) {
            this.trackPasswordField(addedNode);
          } else {
            this.findAndTrackChildren(addedNode);
          }
        }.bind(this));
      }.bind(this));
    }.bind(this));
    this.passwordFieldsObserver.observe(
        docRoot, {subtree: true, childList: true});
  },

  /**
   * Find and track password fields that are descendants of the given element.
   * @param {!HTMLElement} element The parent element to search from.
   */
  findAndTrackChildren: function(element) {
    Array.prototype.forEach.call(
        element.querySelectorAll('input[type=password]'), function(field) {
          this.trackPasswordField(field);
        }.bind(this));
  },

  /**
   * Start tracking value changes of the given password field if it is
   * not being tracked yet.
   * @param {!HTMLInputElement} passworField The password field to track.
   */
  trackPasswordField: function(passwordField) {
    var existing = this.passwordFields_.filter(function(element) {
      return element === passwordField;
    });
    if (existing.length != 0)
      return;

    var index = this.passwordFields_.length;
    var fieldId = passwordField.id || passwordField.name || '';
    passwordField.addEventListener(
        'input', this.onPasswordChanged_.bind(this, index, fieldId));
    this.passwordFields_.push(passwordField);
    this.passwordValues_.push(passwordField.value);
  },

  /**
   * Check if the password field at |index| has changed. If so, sends back
   * the updated value.
   */
  maybeSendUpdatedPassword: function(index, fieldId) {
    var newValue = this.passwordFields_[index].value;
    if (newValue == this.passwordValues_[index])
      return;

    this.passwordValues_[index] = newValue;

    // Use an invalid char for URL as delimiter to concatenate page url,
    // password field index and id to construct a unique ID for the password
    // field.
    var passwordId =
        this.pageURL_.split('#')[0].split('?')[0] + '|' + index + '|' + fieldId;
    this.channel_.send(
        {name: 'updatePassword', id: passwordId, password: newValue});
  },

  /**
   * Handles 'change' event in the scraped password fields.
   * @param {number} index The index of the password fields in
   *     |passwordFields_|.
   * @param {string} fieldId The id or name of the password field or blank.
   */
  onPasswordChanged_: function(index, fieldId) {
    this.maybeSendUpdatedPassword(index, fieldId);
  }
};

function onGetSAMLFlag(channel, isSAMLPage) {
  if (!isSAMLPage)
    return;
  var pageURL = window.location.href;

  channel.send({name: 'pageLoaded', url: pageURL});

  var initPasswordScraper = function() {
    var passwordScraper = new PasswordInputScraper();
    passwordScraper.init(channel, pageURL, document.documentElement);
  };

  if (document.readyState == 'loading') {
    window.addEventListener('readystatechange', function listener(event) {
      if (document.readyState == 'loading')
        return;
      initPasswordScraper();
      window.removeEventListener(event.type, listener, true);
    }, true);
  } else {
    initPasswordScraper();
  }
}

var channel = Channel.create();
channel.connect('injected');
channel.sendWithCallback(
    {name: 'getSAMLFlag'}, onGetSAMLFlag.bind(undefined, channel));

var apiCallForwarder = new APICallForwarder();
apiCallForwarder.init(channel);
})();
