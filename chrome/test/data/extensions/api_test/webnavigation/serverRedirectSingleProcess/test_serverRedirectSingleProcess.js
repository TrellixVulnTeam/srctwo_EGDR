// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

onload = function() {
  // TODO(jochen): Remove once the reason for http://crbug.com/235171 is found.
  debug = true;

  var URL_LOAD =
      "http://www.a.com:PORT/extensions/api_test/webnavigation/" +
      "serverRedirectSingleProcess/a.html";
  var URL_REDIRECT = "http://www.b.com:PORT/server-redirect";
  var URL_TARGET = "http://www.b.com:PORT/test";
  chrome.tabs.create({"url": "about:blank"}, function(tab) {
    var tabId = tab.id;
    chrome.test.getConfig(function(config) {
      var fixPort = function(url) {
        return url.replace(/PORT/g, config.testServer.port);
      };
      URL_REDIRECT = fixPort(URL_REDIRECT);
      URL_TARGET = fixPort(URL_TARGET);
      URL_LOAD = fixPort(URL_LOAD);
      chrome.test.runTests([
        // Two navigations initiated by the user while we ran out of renderer
        // processes before. The second navigation results in a server redirect.
        // At this point, we have two different render views attached to the
        // web contents that are in the same process. Should not DCHECK.
        function serverRedirectSingleProcess() {
          expect([
            { label: "a-onBeforeNavigate",
              event: "onBeforeNavigate",
              details: { frameId: 0,
                         parentFrameId: -1,
                         processId: -1,
                         tabId: 0,
                         timeStamp: 0,
                         url: URL_LOAD }},
            { label: "a-onCommitted",
              event: "onCommitted",
              details: { frameId: 0,
                         processId: 0,
                         tabId: 0,
                         timeStamp: 0,
                         transitionQualifiers: [],
                         transitionType: "typed",
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
                         url: URL_LOAD }},
            { label: "b-onBeforeNavigate",
              event: "onBeforeNavigate",
              details: { frameId: 0,
                         parentFrameId: -1,
                         processId: -1,
                         tabId: 0,
                         timeStamp: 0,
                         url: URL_REDIRECT }},
            { label: "b-onCommitted",
              event: "onCommitted",
              details: { frameId: 0,
                         processId: 0,
                         tabId: 0,
                         timeStamp: 0,
                         transitionQualifiers: ["server_redirect"],
                         transitionType: "typed",
                         url: URL_TARGET }},
            { label: "b-onDOMContentLoaded",
              event: "onDOMContentLoaded",
              details: { frameId: 0,
                         processId: 0,
                         tabId: 0,
                         timeStamp: 0,
                         url: URL_TARGET }},
            { label: "b-onCompleted",
              event: "onCompleted",
              details: { frameId: 0,
                         processId: 0,
                         tabId: 0,
                         timeStamp: 0,
                         url: URL_TARGET }}],
            [ navigationOrder("a-"), navigationOrder("b-") ]);

          // Notify the api test that we're waiting for the user.
          chrome.test.notifyPass();
        },
      ]);
    });
  });
};
