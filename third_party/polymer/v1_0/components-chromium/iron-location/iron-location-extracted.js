(function() {
    'use strict';

    Polymer({
      is: 'iron-location',
      properties: {
        /**
         * The pathname component of the URL.
         */
        path: {
          type: String,
          notify: true,
          value: function() {
            return window.decodeURIComponent(window.location.pathname);
          }
        },
        /**
         * The query string portion of the URL.
         */
        query: {
          type: String,
          notify: true,
          value: function() {
            return window.decodeURIComponent(window.location.search.slice(1));
          }
        },
        /**
         * The hash component of the URL.
         */
        hash: {
          type: String,
          notify: true,
          value: function() {
            return window.decodeURIComponent(window.location.hash.slice(1));
          }
        },
        /**
         * If the user was on a URL for less than `dwellTime` milliseconds, it
         * won't be added to the browser's history, but instead will be replaced
         * by the next entry.
         *
         * This is to prevent large numbers of entries from clogging up the user's
         * browser history. Disable by setting to a negative number.
         */
        dwellTime: {
          type: Number,
          value: 2000
        },

        /**
         * A regexp that defines the set of URLs that should be considered part
         * of this web app.
         *
         * Clicking on a link that matches this regex won't result in a full page
         * navigation, but will instead just update the URL state in place.
         *
         * This regexp is given everything after the origin in an absolute
         * URL. So to match just URLs that start with /search/ do:
         *     url-space-regex="^/search/"
         *
         * @type {string|RegExp}
         */
        urlSpaceRegex: {
          type: String,
          value: ''
        },

        /**
         * urlSpaceRegex, but coerced into a regexp.
         *
         * @type {RegExp}
         */
        _urlSpaceRegExp: {
          computed: '_makeRegExp(urlSpaceRegex)'
        },

        _lastChangedAt: {
          type: Number
        },

        _initialized: {
          type: Boolean,
          value: false
        }
      },
      hostAttributes: {
        hidden: true
      },
      observers: [
        '_updateUrl(path, query, hash)'
      ],
      attached: function() {
        this.listen(window, 'hashchange', '_hashChanged');
        this.listen(window, 'location-changed', '_urlChanged');
        this.listen(window, 'popstate', '_urlChanged');
        this.listen(/** @type {!HTMLBodyElement} */(document.body), 'click', '_globalOnClick');
        // Give a 200ms grace period to make initial redirects without any
        // additions to the user's history.
        this._lastChangedAt = window.performance.now() - (this.dwellTime - 200);

        this._initialized = true;
        this._urlChanged();
      },
      detached: function() {
        this.unlisten(window, 'hashchange', '_hashChanged');
        this.unlisten(window, 'location-changed', '_urlChanged');
        this.unlisten(window, 'popstate', '_urlChanged');
        this.unlisten(/** @type {!HTMLBodyElement} */(document.body), 'click', '_globalOnClick');
        this._initialized = false;
      },
      _hashChanged: function() {
        this.hash = window.decodeURIComponent(window.location.hash.substring(1));
      },
      _urlChanged: function() {
        // We want to extract all info out of the updated URL before we
        // try to write anything back into it.
        //
        // i.e. without _dontUpdateUrl we'd overwrite the new path with the old
        // one when we set this.hash. Likewise for query.
        this._dontUpdateUrl = true;
        this._hashChanged();
        this.path = window.decodeURIComponent(window.location.pathname);
        this.query = window.decodeURIComponent(
            window.location.search.substring(1));
        this._dontUpdateUrl = false;
        this._updateUrl();
      },
      _getUrl: function() {
        var partiallyEncodedPath = window.encodeURI(
            this.path).replace(/\#/g, '%23').replace(/\?/g, '%3F');
        var partiallyEncodedQuery = '';
        if (this.query) {
          partiallyEncodedQuery = '?' + window.encodeURI(
              this.query).replace(/\#/g, '%23');
        }
        var partiallyEncodedHash = '';
        if (this.hash) {
          partiallyEncodedHash = '#' + window.encodeURI(this.hash);
        }
        return (
            partiallyEncodedPath + partiallyEncodedQuery + partiallyEncodedHash);
      },
      _updateUrl: function() {
        if (this._dontUpdateUrl || !this._initialized) {
          return;
        }
        if (this.path === window.decodeURIComponent(window.location.pathname) &&
            this.query === window.decodeURIComponent(
                window.location.search.substring(1)) &&
            this.hash === window.decodeURIComponent(
                window.location.hash.substring(1))) {
          // Nothing to do, the current URL is a representation of our properties.
          return;
        }
        var newUrl = this._getUrl();
        // Need to use a full URL in case the containing page has a base URI.
        var fullNewUrl = new URL(
            newUrl, window.location.protocol + '//' + window.location.host).href;
        var now = window.performance.now();
        var shouldReplace =
            this._lastChangedAt + this.dwellTime > now;
        this._lastChangedAt = now;
        if (shouldReplace) {
          window.history.replaceState({}, '', fullNewUrl);
        } else {
          window.history.pushState({}, '', fullNewUrl);
        }
        this.fire('location-changed', {}, {node: window});
      },
      /**
       * A necessary evil so that links work as expected. Does its best to
       * bail out early if possible.
       *
       * @param {MouseEvent} event .
       */
      _globalOnClick: function(event) {
        // If another event handler has stopped this event then there's nothing
        // for us to do. This can happen e.g. when there are multiple
        // iron-location elements in a page.
        if (event.defaultPrevented) {
          return;
        }
        var href = this._getSameOriginLinkHref(event);
        if (!href) {
          return;
        }
        event.preventDefault();
        // If the navigation is to the current page we shouldn't add a history
        // entry or fire a change event.
        if (href === window.location.href) {
          return;
        }
        window.history.pushState({}, '', href);
        this.fire('location-changed', {}, {node: window});
      },
      /**
       * Returns the absolute URL of the link (if any) that this click event
       * is clicking on, if we can and should override the resulting full
       * page navigation. Returns null otherwise.
       *
       * @param {MouseEvent} event .
       * @return {string?} .
       */
      _getSameOriginLinkHref: function(event) {
        // We only care about left-clicks.
        if (event.button !== 0) {
          return null;
        }
        // We don't want modified clicks, where the intent is to open the page
        // in a new tab.
        if (event.metaKey || event.ctrlKey) {
          return null;
        }
        var eventPath = Polymer.dom(event).path;
        var anchor = null;
        for (var i = 0; i < eventPath.length; i++) {
          var element = eventPath[i];
          if (element.tagName === 'A' && element.href) {
            anchor = element;
            break;
          }
        }

        // If there's no link there's nothing to do.
        if (!anchor) {
          return null;
        }

        // Target blank is a new tab, don't intercept.
        if (anchor.target === '_blank') {
          return null;
        }
        // If the link is for an existing parent frame, don't intercept.
        if ((anchor.target === '_top' ||
             anchor.target === '_parent') &&
            window.top !== window) {
          return null;
        }

        var href = anchor.href;

        // It only makes sense for us to intercept same-origin navigations.
        // pushState/replaceState don't work with cross-origin links.
        var url;
        if (document.baseURI != null) {
          url = new URL(href, /** @type {string} */(document.baseURI));
        } else {
          url = new URL(href);
        }

        var origin;

        // IE Polyfill
        if (window.location.origin) {
          origin = window.location.origin;
        } else {
          origin = window.location.protocol + '//' + window.location.hostname;

          if (window.location.port) {
            origin += ':' + window.location.port;
          }
        }

        if (url.origin !== origin) {
          return null;
        }
        var normalizedHref = url.pathname + url.search + url.hash;

        // If we've been configured not to handle this url... don't handle it!
        if (this._urlSpaceRegExp &&
            !this._urlSpaceRegExp.test(normalizedHref)) {
          return null;
        }
        // Need to use a full URL in case the containing page has a base URI.
        var fullNormalizedHref = new URL(
            normalizedHref, window.location.href).href;
        return fullNormalizedHref;
      },
      _makeRegExp: function(urlSpaceRegex) {
        return RegExp(urlSpaceRegex);
      }
    });
  })();