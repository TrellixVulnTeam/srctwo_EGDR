// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Provides an interface to check whether the user has approved
 * an origin to use security keys.
 *
 */
'use strict';

/**
 * Allows the caller to check whether the user has approved the use of
 * security keys from an origin.
 * @interface
 */
function ApprovedOrigins() {}

/**
 * Checks whether the origin is approved to use security keys. (If not, an
 * approval prompt may be shown.)
 * @param {string} origin The origin to approve.
 * @param {number=} opt_tabId A tab id to display approval prompt in, if
 *     necessary.
 * @return {Promise<boolean>} A promise for the result of the check.
 */
ApprovedOrigins.prototype.isApprovedOrigin = function(origin, opt_tabId) {};
