// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(async function() {
  TestRunner.addResult('Tests that console logging dumps proper messages.\n');

  await TestRunner.loadModule('console_test_runner');
  await TestRunner.showPanel('console');

  await TestRunner.evaluateInPagePromise(`
    function logToConsole()
    {
        var fragment = document.createDocumentFragment();
        fragment.appendChild(document.createElement("p"));

        console.dirxml(document);
        console.dirxml(fragment);
        console.dirxml(fragment.firstChild);
        console.log([fragment.firstChild]);
        console.dirxml([document, fragment, document.createElement("span")]);
    }
  `);

  runtime.loadModulePromise('elements').then(function() {
    TestRunner.evaluateInPage('logToConsole()', onLoggedToConsole);
  });

  function onLoggedToConsole() {
    ConsoleTestRunner.waitForRemoteObjectsConsoleMessages(onRemoteObjectsLoaded);
  }

  function onRemoteObjectsLoaded() {
    ConsoleTestRunner.dumpConsoleMessages();
    TestRunner.completeTest();
  }
})();
