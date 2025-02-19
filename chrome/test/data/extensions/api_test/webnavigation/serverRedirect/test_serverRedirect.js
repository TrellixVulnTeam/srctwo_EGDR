// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

onload = function() {
  var URL_LOAD =
      "http://www.a.com:PORT/extensions/api_test/webnavigation/serverRedirect/a.html";
  var URL_LOAD_REDIRECT = "http://www.a.com:PORT/server-redirect";
  chrome.tabs.create({"url": "about:blank"}, function(tab) {
    var tabId = tab.id;
    chrome.test.getConfig(function(config) {
      var fixPort = function(url) {
        return url.replace(/PORT/g, config.testServer.port);
      };
      URL_LOAD_REDIRECT = fixPort(URL_LOAD_REDIRECT);
      URL_LOAD = fixPort(URL_LOAD);
      chrome.test.runTests([
        // Navigates to a page that redirects (on the server side) to a.html.
        function serverRedirect() {
          expect([
            { label: "a-onBeforeNavigate",
              event: "onBeforeNavigate",
              details: { frameId: 0,
                         parentFrameId: -1,
                         processId: -1,
                         tabId: 0,
                         timeStamp: 0,
                         url: URL_LOAD_REDIRECT }},
            { label: "a-onCommitted",
              event: "onCommitted",
              details: { frameId: 0,
                         processId: 0,
                         tabId: 0,
                         timeStamp: 0,
                         transitionQualifiers: ["server_redirect"],
                         transitionType: "link",
                         url: URL_LOAD }},
            { label: "a-onDOMContentLoaded",
              event: "onDOMContentLoaded",
              details: { frameId: 0,
                         processId: 0,
                         tabId: 0,
                         timeStamp: 0,
                         url: URL_LOAD }},
            { label: "a-onCompleted",
              event: "onCompleted",
              details: { frameId: 0,
                         processId: 0,
                         tabId: 0,
                         timeStamp: 0,
                         url: URL_LOAD }}],
            [ navigationOrder("a-") ]);
          chrome.tabs.update(tabId,
                             { url: URL_LOAD_REDIRECT + "?" + URL_LOAD });
        },
      ]);
    });
  });
};
