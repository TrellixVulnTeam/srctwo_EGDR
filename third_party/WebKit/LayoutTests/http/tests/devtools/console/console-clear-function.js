// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(async function() {
  TestRunner.addResult(`Tests that console is cleared via console.clear() method\n`);

  await TestRunner.loadModule('console_test_runner');
  await TestRunner.showPanel('console');
  await TestRunner.loadHTML(`
    <a href="https://bugs.webkit.org/show_bug.cgi?id=101021">Bug 101021</a>
  `);
  await TestRunner.evaluateInPagePromise(`
    function log()
    {
      // Fill console.
      console.log("one");
      console.log("two");
      console.log("three");
    }

    function clearConsoleFromPage()
    {
      console.clear();
    }
  `);

  TestRunner.runTestSuite([
    async function clearFromConsoleAPI(next) {
      await TestRunner.RuntimeAgent.evaluate('log();');
      TestRunner.addResult('=== Before clear ===');
      ConsoleTestRunner.dumpConsoleMessages();

      await TestRunner.RuntimeAgent.evaluate('clearConsoleFromPage();');

      TestRunner.addResult('=== After clear ===');
      ConsoleTestRunner.dumpConsoleMessages();
      next();
    },

    async function shouldNotClearWithPreserveLog(next) {
      await TestRunner.RuntimeAgent.evaluate('log();');
      TestRunner.addResult('=== Before clear ===');
      ConsoleTestRunner.dumpConsoleMessages();
      Common.moduleSetting('preserveConsoleLog').set(true);

      await TestRunner.RuntimeAgent.evaluate('clearConsoleFromPage();');

      TestRunner.addResult('=== After clear ===');
      ConsoleTestRunner.dumpConsoleMessages();
      Common.moduleSetting('preserveConsoleLog').set(false);
      next();
    }
  ]);
})();
