// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function compareId(a, b) {
  return a.id > b.id;
}

chrome.app.runtime.onLaunched.addListener(function() {
  chrome.test.runTests([

    function testGetAllNoWindows() {
      chrome.test.assertEq({}, chrome.app.window.getAll());
      chrome.test.succeed();
    },

    function testGetAllOneWindow() {
      chrome.app.window.create('index.html', {id: 'win1'}, function(win) {
        win.contentWindow.addEventListener('load', function() {
          chrome.test.assertEq([win], chrome.app.window.getAll());
          win.onClosed.addListener(function() {
            chrome.test.succeed();
          });
          win.close();
        });
      });
    },

    function testGetAllMultipleWindows() {
      chrome.app.window.create('index.html', {id: 'win1'}, function(win1) {
        win1.contentWindow.addEventListener('load', function() {
          chrome.app.window.create('index.html', {id: 'win2'}, function(win2) {
            win2.contentWindow.addEventListener('load', function() {
              var windows = chrome.app.window.getAll().sort(compareId);
              chrome.test.assertEq([win1, win2], windows);
              win2.onClosed.addListener(function() {
                chrome.test.succeed();
              });
              win1.onClosed.addListener(function() {
                win2.close();
              });
              win1.close();
            });
          });
        });
      });
    },

    function testGetNoWindows() {
      chrome.test.assertEq(null, chrome.app.window.get(''));
      chrome.test.succeed();
    },

    function testGet() {
      chrome.app.window.create('index.html', {id: 'win1'}, function(win1) {
        win1.contentWindow.addEventListener('load', function() {
          chrome.app.window.create('index.html', {id: 'win2'}, function(win2) {
            win2.contentWindow.addEventListener('load', function() {
              chrome.test.assertEq(win1, chrome.app.window.get('win1'));
              chrome.test.assertEq(win2, chrome.app.window.get('win2'));
              chrome.test.assertEq(null, chrome.app.window.get('win3'));
              win2.onClosed.addListener(function() {
                chrome.test.succeed();
              });
              win1.onClosed.addListener(function() {
                win2.close();
              });
              win1.close();
            });
          });
        });
      });
    }
  ]);
});
