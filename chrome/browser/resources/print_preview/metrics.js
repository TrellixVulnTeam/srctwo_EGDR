// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

cr.define('print_preview', function() {
  'use strict';

  /**
   * Object used to measure usage statistics.
   * @constructor
   */
  function Metrics() {}

  /**
   * Enumeration of buckets that a user can enter while using the destination
   * search widget.
   * @enum {number}
   */
  Metrics.DestinationSearchBucket = {
    // Used when the print destination search widget is shown.
    DESTINATION_SHOWN: 0,
    // Used when the user selects a print destination.
    DESTINATION_CLOSED_CHANGED: 1,
    // Used when the print destination search widget is closed without selecting
    // a print destination.
    DESTINATION_CLOSED_UNCHANGED: 2,
    // Used when the Google Cloud Print promotion (shown in the destination
    // search widget) is shown to the user.
    SIGNIN_PROMPT: 3,
    // Used when the user chooses to sign-in to their Google account.
    SIGNIN_TRIGGERED: 4,
    // Used when a user selects the Privet printer in a pair of duplicate
    // Privet and cloud printers.
    PRIVET_DUPLICATE_SELECTED: 5,
    // Used when a user selects the cloud printer in a pair of duplicate
    // Privet and cloud printers.
    CLOUD_DUPLICATE_SELECTED: 6,
    // Used when a user sees a register promo for a cloud print printer.
    REGISTER_PROMO_SHOWN: 7,
    // Used when a user selects a register promo for a cloud print printer.
    REGISTER_PROMO_SELECTED: 8,
    // User changed active account.
    ACCOUNT_CHANGED: 9,
    // User tried to log into another account.
    ADD_ACCOUNT_SELECTED: 10,
    // Printer sharing invitation was shown to the user.
    INVITATION_AVAILABLE: 11,
    // User accepted printer sharing invitation.
    INVITATION_ACCEPTED: 12,
    // User rejected printer sharing invitation.
    INVITATION_REJECTED: 13,
    // Max value.
    DESTINATION_SEARCH_MAX_BUCKET: 14
  };

  /**
   * Enumeration of buckets that a user can enter while using the Google Cloud
   * Print promotion.
   * @enum {number}
   */
  Metrics.GcpPromoBucket = {
    // Used when the Google Cloud Print promotion (shown above the PDF preview
    // plugin) is shown to the user.
    PROMO_SHOWN: 0,
    // Used when the user clicks the "Get started" link in the promotion shown
    // in CLOUDPRINT_BIG_PROMO_SHOWN.
    PROMO_CLICKED: 1,
    // Used when the user dismisses the promotion shown in
    // CLOUDPRINT_BIG_PROMO_SHOWN.
    PROMO_CLOSED: 2,
    // Max value.
    GCP_PROMO_MAX_BUCKET: 3
  };

  /**
   * Print settings UI usage metrics buckets.
   * @enum {number}
   */
  Metrics.PrintSettingsUiBucket = {
    // Advanced settings dialog is shown.
    ADVANCED_SETTINGS_DIALOG_SHOWN: 0,
    // Advanced settings dialog is closed without saving a selection.
    ADVANCED_SETTINGS_DIALOG_CANCELED: 1,
    // 'More/less settings' expanded.
    MORE_SETTINGS_CLICKED: 2,
    // 'More/less settings' collapsed.
    LESS_SETTINGS_CLICKED: 3,
    // User printed with extra settings expanded.
    PRINT_WITH_SETTINGS_EXPANDED: 4,
    // User printed with extra settings collapsed.
    PRINT_WITH_SETTINGS_COLLAPSED: 5,
    // Max value.
    PRINT_SETTINGS_UI_MAX_BUCKET: 6
  };

  /**
   * A context for recording a value in a specific UMA histogram.
   * @param {string} histogram The name of the histogram to be recorded in.
   * @param {number} maxBucket The max value for the last histogram bucket.
   * @constructor
   */
  function MetricsContext(histogram, maxBucket) {
    /** @private {string} */
    this.histogram_ = histogram;

    /** @private {number} */
    this.maxBucket_ = maxBucket;

    /** @private {!print_preview.NativeLayer} */
    this.nativeLayer_ = print_preview.NativeLayer.getInstance();
  }

  MetricsContext.prototype = {
    /**
     * Record a histogram value in UMA. If specified value is larger than the
     * max bucket value, record the value in the largest bucket
     * @param {number} bucket Value to record.
     */
    record: function(bucket) {
      this.nativeLayer_.recordInHistogram(
          this.histogram_,
          (bucket > this.maxBucket_) ? this.maxBucket_ : bucket,
          this.maxBucket_);
    }
  };

  /**
   * Destination Search specific usage statistics context.
   * @constructor
   * @extends {print_preview.MetricsContext}
   */
  function DestinationSearchMetricsContext() {
    MetricsContext.call(
        this, 'PrintPreview.DestinationAction',
        Metrics.DestinationSearchBucket.DESTINATION_SEARCH_MAX_BUCKET);
  }

  DestinationSearchMetricsContext.prototype = {
    __proto__: MetricsContext.prototype
  };

  /**
   * GCP promotion specific usage statistics context.
   * @constructor
   * @extends {print_preview.MetricsContext}
   */
  function GcpPromoMetricsContext() {
    MetricsContext.call(
        this, 'PrintPreview.GcpPromo',
        Metrics.GcpPromoBucket.GCP_PROMO_MAX_BUCKET);
  }

  GcpPromoMetricsContext.prototype = {__proto__: MetricsContext.prototype};

  /**
   * Print settings UI specific usage statistics context.
   * @constructor
   * @extends {print_preview.MetricsContext}
   */
  function PrintSettingsUiMetricsContext() {
    MetricsContext.call(
        this, 'PrintPreview.PrintSettingsUi',
        Metrics.PrintSettingsUiBucket.PRINT_SETTINGS_UI_MAX_BUCKET);
  }

  PrintSettingsUiMetricsContext.prototype = {
    __proto__: MetricsContext.prototype
  };

  // Export
  return {
    Metrics: Metrics,
    MetricsContext: MetricsContext,
    DestinationSearchMetricsContext: DestinationSearchMetricsContext,
    GcpPromoMetricsContext: GcpPromoMetricsContext,
    PrintSettingsUiMetricsContext: PrintSettingsUiMetricsContext
  };
});
