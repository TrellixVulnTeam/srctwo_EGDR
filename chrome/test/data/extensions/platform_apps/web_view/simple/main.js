// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function CreateWebViewAndGuest(callback) {
  var webview = document.createElement('webview');
  webview.allowtransparency = true;
  webview.allowscaling = true;
  var onLoadStop = function(e) {
    chrome.test.sendMessage('WebViewTest.LAUNCHED');
    webview.removeEventListener('loadstop', onLoadStop);
    webview.removeEventListener('loadabort', onLoadAbort);
    callback();
  };
  webview.addEventListener('loadstop', onLoadStop);

  var onLoadAbort = function(e) {
    chrome.test.sendMessage('WebViewTest.FAILURE');
    webview.removeEventListener('loadstop', onLoadStop);
    webview.removeEventListener('loadabort', onLoadAbort);
  };
  webview.src = 'data:text/html,<html><body>simple test</body></html>';
  return webview;
}

onload = function() {
  var webview = CreateWebViewAndGuest(function() {
    webview.addEventListener('newwindow', function(e) {
      var newwebview = document.createElement('webview');
      newwebview.addEventListener('loadstop', function(e) {
        chrome.test.sendMessage('WebViewTest.NEWWINDOW');
      });
      e.window.attach(newwebview);
      document.body.appendChild(newwebview);
    });

    webview.addEventListener('loadstop', function(e) {
      chrome.test.sendMessage('WebViewTest.LOADSTOP');
    });
  });
  document.body.appendChild(webview);
};
