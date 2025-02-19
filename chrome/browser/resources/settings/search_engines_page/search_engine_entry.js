// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview 'settings-search-engine-entry' is a component for showing a
 * search engine with its name, domain and query URL.
 */
Polymer({
  is: 'settings-search-engine-entry',

  behaviors: [FocusRowBehavior],

  properties: {
    /** @type {!SearchEngine} */
    engine: Object,

    /** @type {boolean} */
    isDefault: {
      reflectToAttribute: true,
      type: Boolean,
      computed: 'computeIsDefault_(engine)'
    },

    /** @private {boolean} */
    showDots_: {
      reflectToAttribute: true,
      type: Boolean,
      computed: 'computeShowDots_(engine.canBeDefault,' +
          'engine.canBeEdited,' +
          'engine.canBeRemoved)',
    },

    /** @private {boolean} */
    showEditSearchEngineDialog_: Boolean,
  },

  /** @private {settings.SearchEnginesBrowserProxy} */
  browserProxy_: null,

  /** @override */
  created: function() {
    this.browserProxy_ = settings.SearchEnginesBrowserProxyImpl.getInstance();
  },

  /** @private */
  closePopupMenu_: function() {
    this.$$('dialog[is=cr-action-menu]').close();
  },

  /**
   * @return {boolean}
   * @private
   */
  computeIsDefault_: function() {
    return this.engine.default;
  },

  /**
   * @param {boolean} canBeDefault
   * @param {boolean} canBeEdited
   * @param {boolean} canBeRemoved
   * @return {boolean} Whether to show the dots menu.
   * @private
   */
  computeShowDots_: function(canBeDefault, canBeEdited, canBeRemoved) {
    return canBeDefault || canBeEdited || canBeRemoved;
  },

  /**
   * @param {?string} url The icon URL if available.
   * @return {string} A set of icon URLs.
   * @private
   */
  getIconSet_: function(url) {
    // Force default icon, if no |engine.iconURL| is available.
    return cr.icon.getFavicon(url || '');
  },

  /** @private */
  onDeleteTap_: function() {
    this.browserProxy_.removeSearchEngine(this.engine.modelIndex);
    this.closePopupMenu_();
  },

  /** @private */
  onDotsTap_: function() {
    /** @type {!CrActionMenuElement} */ (this.$$('dialog[is=cr-action-menu]'))
        .showAt(assert(this.$$('button[is="paper-icon-button-light"]')));
  },

  /**
   * @param {!Event} e
   * @private
   */
  onEditTap_: function(e) {
    e.preventDefault();
    this.closePopupMenu_();

    this.showEditSearchEngineDialog_ = true;
    this.async(() => {
      var dialog = this.$$('settings-search-engine-dialog');
      // Register listener to detect when the dialog is closed. Flip the boolean
      // once closed to force a restamp next time it is shown such that the
      // previous dialog's contents are cleared.
      dialog.addEventListener('close', () => {
        this.showEditSearchEngineDialog_ = false;
        cr.ui.focusWithoutInk(
            assert(this.$$('button[is="paper-icon-button-light"]')));
      });
    });
  },

  /** @private */
  onMakeDefaultTap_: function() {
    this.closePopupMenu_();
    this.browserProxy_.setDefaultSearchEngine(this.engine.modelIndex);
  },
});
