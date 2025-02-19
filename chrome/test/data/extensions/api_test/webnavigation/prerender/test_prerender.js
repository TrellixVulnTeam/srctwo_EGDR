// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

onload = function() {
  var getURL = chrome.extension.getURL;
  var URL_LOAD =
      "http://127.0.0.1:PORT/prerender/prerender_loader.html";
  var URL_TARGET =
      "http://127.0.0.1:PORT/prerender/prerender_page.html";
  chrome.tabs.create({"url": "about:blank"}, function(tab) {
    var tabId = tab.id;
    chrome.test.getConfig(function(config) {
      var fixPort = function(url) {
        return url.replace(/PORT/g, config.testServer.port);
      };
      URL_LOAD = fixPort(URL_LOAD);
      URL_TARGET = fixPort(URL_TARGET);

      chrome.test.runTests([
        // A prerendered tab replaces the current tab.
        function prerendered() {
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
                         tabId: 1,
                         timeStamp: 0,
                         url: URL_TARGET }},
            { label: "b-onCommitted",
              event: "onCommitted",
              details: { frameId: 0,
                         processId: 1,
                         tabId: 1,
                         timeStamp: 0,
                         transitionQualifiers: [],
                         transitionType: "link",
                         url: URL_TARGET }},
            { label: "b-onDOMContentLoaded",
              event: "onDOMContentLoaded",
              details: { frameId: 0,
                         processId: 1,
                         tabId: 1,
                         timeStamp: 0,
                         url: URL_TARGET }},
            { label: "b-onCompleted",
              event: "onCompleted",
              details: { frameId: 0,
                         processId: 1,
                         tabId: 1,
                         timeStamp: 0,
                         url: URL_TARGET }},
            { label: "onTabReplaced",
              event: "onTabReplaced",
              details: { replacedTabId: 0,
                         tabId: 1,
                         timeStamp: 0 }}],
            [ navigationOrder("a-"),
              navigationOrder("b-"),
              [ "a-onCompleted", "b-onCompleted", "onTabReplaced" ]]);

          // Notify the api test that we're waiting for the user.
          chrome.test.notifyPass();
        },
      ]);
    });
  });
};
