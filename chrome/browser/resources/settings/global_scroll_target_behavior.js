// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview |GlobalScrollTargetBehavior| allows an element to be aware of
 * the global scroll target.
 *
 * |scrollTarget| will be populated async by |setGlobalScrollTarget|.
 *
 * |subpageScrollTarget| will be equal to the |scrollTarget|, but will only be
 * populated when the current route is the |subpageRoute|.
 *
 * |setGlobalScrollTarget| should only be called once.
 */

cr.define('settings', function() {
  var scrollTargetResolver = new PromiseResolver();

  /** @polymerBehavior */
  var GlobalScrollTargetBehaviorImpl = {
    properties: {
      /**
       * Read only property for the scroll target.
       * @type {HTMLElement}
       */
      scrollTarget: {
        type: Object,
        readOnly: true,
      },

      /**
       * Read only property for the scroll target that a subpage should use.
       * It will be set/cleared based on the current route.
       * @type {HTMLElement}
       */
      subpageScrollTarget: {
        type: Object,
        computed: 'getActiveTarget_(scrollTarget, active_)',
      },

      /**
       * The |subpageScrollTarget| should only be set for this route.
       * @type {settings.Route}
       * @private
       */
      subpageRoute: Object,

      /** Whether the |subpageRoute| is active or not. */
      active_: Boolean,
    },

    /** @override */
    attached: function() {
      scrollTargetResolver.promise.then(this._setScrollTarget.bind(this));
    },

    /** @param {!settings.Route} route */
    currentRouteChanged: function(route) {
      this.active_ = route == this.subpageRoute;
    },

    /**
     * Returns the target only when the route is active.
     * @param {HTMLElement} target
     * @param {boolean} active
     * @return {?HTMLElement}
     * @private
     */
    getActiveTarget_: function(target, active) {
      return active ? target : null;
    },
  };

  /**
   * This should only be called once.
   * @param {HTMLElement} scrollTarget
   */
  var setGlobalScrollTarget = function(scrollTarget) {
    scrollTargetResolver.resolve(scrollTarget);
  };

  return {
    GlobalScrollTargetBehaviorImpl: GlobalScrollTargetBehaviorImpl,
    setGlobalScrollTarget: setGlobalScrollTarget,
    scrollTargetResolver: scrollTargetResolver,
  };
});

// This is done to make the closure compiler happy: it needs fully qualified
// names when specifying an array of behaviors.
/** @polymerBehavior */
settings.GlobalScrollTargetBehavior =
    [settings.RouteObserverBehavior, settings.GlobalScrollTargetBehaviorImpl];
