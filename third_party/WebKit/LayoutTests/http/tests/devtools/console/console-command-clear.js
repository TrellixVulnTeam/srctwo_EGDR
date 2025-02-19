// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(async function() {
  TestRunner.addResult(`Tests that console is cleared upon clear() eval in console.\n`);

  await TestRunner.loadModule('console_test_runner');
  await TestRunner.showPanel('console');

  await TestRunner.evaluateInPagePromise(`
     function log() {
        // Fill console.
        console.log("one");
        console.log("two");
        console.log("three");
    }
    log();
  `);

  TestRunner.addResult('=== Before clear ===');
  ConsoleTestRunner.dumpConsoleMessages();

  function callback() {
    TestRunner.addResult('=== After clear ===');
    ConsoleTestRunner.dumpConsoleMessages();
    TestRunner.completeTest();
  }

  ConsoleTestRunner.evaluateInConsole('clear()', callback);
})();
