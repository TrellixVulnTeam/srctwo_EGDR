// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Polymer element hosting <network-proxy> in the internet
 * detail page. This element is responsible for setting 'Allow proxies for
 * shared networks'.
 */
Polymer({
  is: 'network-proxy-section',

  behaviors: [
    CrPolicyNetworkBehavior,
    I18nBehavior,
    PrefsBehavior,
    settings.RouteObserverBehavior,
  ],

  properties: {
    /**
     * The network properties dictionary containing the proxy properties to
     * display and modify.
     * @type {!CrOnc.NetworkProperties|undefined}
     */
    networkProperties: Object,

    /**
     * Reflects prefs.settings.use_shared_proxies for data binding.
     * @private
     */
    useSharedProxies_: Boolean,
  },

  observers: [
    'useSharedProxiesChanged_(prefs.settings.use_shared_proxies.value)',
  ],

  /** @protected settings.RouteObserverBehavior */
  currentRouteChanged: function(newRoute) {
    if (newRoute == settings.routes.NETWORK_DETAIL)
      /** @type {NetworkProxyElement} */ (this.$$('network-proxy')).reset();
  },

  /** @private */
  useSharedProxiesChanged_: function() {
    var pref = this.getPref('settings.use_shared_proxies');
    this.useSharedProxies_ = !!pref && !!pref.value;
  },

  /**
   * @return {boolean}
   * @private
   */
  isShared_: function() {
    return this.networkProperties.Source == 'Device' ||
        this.networkProperties.Source == 'DevicePolicy';
  },

  /**
   * @return {!CrOnc.ManagedProperty|undefined}
   * @private
   */
  getProxySettingsTypeProperty_: function() {
    return /** @type {!CrOnc.ManagedProperty|undefined} */ (
        this.get('ProxySettings.Type', this.networkProperties));
  },

  /**
   * @return {boolean}
   * @private
   */
  shouldShowNetworkPolicyIndicator_: function() {
    var property = this.getProxySettingsTypeProperty_();
    return !!property && !this.isExtensionControlled(property) &&
        this.isNetworkPolicyEnforced(property);
  },

  /**
   * @return {boolean}
   * @private
   */
  shouldShowExtensionIndicator_: function() {
    var property = this.getProxySettingsTypeProperty_();
    return !!property && this.isExtensionControlled(property);
  },

  /**
   * @param {!CrOnc.ManagedProperty} property
   * @return {boolean}
   * @private
   */
  shouldShowAllowShared_: function(property) {
    return !this.isControlled(property) && this.isShared_();
  },

  /**
   * Handles the change event for the shared proxy checkbox. Shows a
   * confirmation dialog.
   * @param {!Event} event
   * @private
   */
  onAllowSharedProxiesChange_: function(event) {
    this.$.confirmAllowSharedDialog.showModal();
  },

  /**
   * Handles the shared proxy confirmation dialog 'Confirm' button.
   * @private
   */
  onAllowSharedDialogConfirm_: function() {
    /** @type {!SettingsCheckboxElement} */ (this.$.allowShared)
        .sendPrefChange();
    this.$.confirmAllowSharedDialog.close();
  },

  /**
   * Handles the shared proxy confirmation dialog 'Cancel' button or a cancel
   * event.
   * @private
   */
  onAllowSharedDialogCancel_: function() {
    /** @type {!SettingsCheckboxElement} */ (this.$.allowShared)
        .resetToPrefValue();
    this.$.confirmAllowSharedDialog.close();
  },

  /** @private */
  onAllowSharedDialogClose_: function() {
    cr.ui.focusWithoutInk(assert(this.$$('#allowShared')));
  },
});
