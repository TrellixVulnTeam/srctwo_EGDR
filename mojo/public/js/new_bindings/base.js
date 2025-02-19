// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

if (mojo && mojo.internal) {
  throw new Error('The Mojo bindings library has been initialized.');
}

var mojo = mojo || {};
mojo.internal = {};
mojo.internal.global = this;
mojo.config = {
  // Whether to automatically load mojom dependencies.
  // For example, if foo.mojom imports bar.mojom, |autoLoadMojomDeps| set to
  // true means that loading foo.mojom.js will insert a <script> tag to load
  // bar.mojom.js, if it hasn't been loaded.
  //
  // The URL of bar.mojom.js is determined by the relative path of bar.mojom
  // (relative to the position of foo.mojom at build time) and the URL of
  // foo.mojom.js. For exmple, if at build time the two mojom files are
  // located at:
  //   a/b/c/foo.mojom
  //   a/b/d/bar.mojom
  // and the URL of foo.mojom.js is:
  //   http://example.org/scripts/b/c/foo.mojom.js
  // then the URL of bar.mojom.js will be:
  //   http://example.org/scripts/b/d/bar.mojom.js
  //
  // If you would like bar.mojom.js to live at a different location, you need
  // to turn off |autoLoadMojomDeps| before loading foo.mojom.js, and manually
  // load bar.mojom.js yourself. Similarly, you need to turn off the option if
  // you merge bar.mojom.js and foo.mojom.js into a single file.
  //
  // Performance tip: Avoid loading the same mojom.js file multiple times.
  // Assume that |autoLoadMojomDeps| is set to true,
  //
  // <!--
  // (This comment tag is necessary on IOS to avoid interpreting the closing
  // script tags in the example.)
  //
  // No duplicate loading; recommended:
  // <script src="http://example.org/scripts/b/c/foo.mojom.js"></script>
  //
  // No duplicate loading, although unnecessary:
  // <script src="http://example.org/scripts/b/d/bar.mojom.js"></script>
  // <script src="http://example.org/scripts/b/c/foo.mojom.js"></script>
  //
  // Load bar.mojom.js twice; should be avoided:
  // <script src="http://example.org/scripts/b/c/foo.mojom.js"></script>
  // <script src="http://example.org/scripts/b/d/bar.mojom.js"></script>
  //
  // -->
  autoLoadMojomDeps: true
};

(function() {
  var internal = mojo.internal;

  var LoadState = {
    PENDING_LOAD: 1,
    LOADED: 2
  };

  var mojomRegistry = new Map();

  function exposeNamespace(namespace) {
    var current = internal.global;
    var parts = namespace.split('.');

    for (var part; parts.length && (part = parts.shift());) {
      if (!current[part]) {
        current[part] = {};
      }
      current = current[part];
    }

    return current;
  }

  function isMojomPendingLoad(id) {
    return mojomRegistry.get(id) === LoadState.PENDING_LOAD;
  }

  function isMojomLoaded(id) {
    return mojomRegistry.get(id) === LoadState.LOADED;
  }

  function markMojomPendingLoad(id) {
    if (isMojomLoaded(id)) {
      throw new Error('The following mojom file has been loaded: ' + id);
    }

    mojomRegistry.set(id, LoadState.PENDING_LOAD);
  }

  function markMojomLoaded(id) {
    mojomRegistry.set(id, LoadState.LOADED);
  }

  function loadMojomIfNecessary(id, relativePath) {
    if (mojomRegistry.has(id)) {
      return;
    }

    if (internal.global.document === undefined) {
      throw new Error(
          'Mojom dependency autoloading is not implemented in workers. ' +
          'Please see config variable mojo.config.autoLoadMojomDeps for more ' +
          'details.');
    }

    markMojomPendingLoad(id);
    var url = new URL(relativePath, document.currentScript.src).href;
    internal.global.document.write('<script type="text/javascript" src="' +
                                   url + '"><' + '/script>');
  }

  internal.exposeNamespace = exposeNamespace;
  internal.isMojomPendingLoad = isMojomPendingLoad;
  internal.isMojomLoaded = isMojomLoaded;
  internal.markMojomPendingLoad = markMojomPendingLoad;
  internal.markMojomLoaded = markMojomLoaded;
  internal.loadMojomIfNecessary = loadMojomIfNecessary;
})();
