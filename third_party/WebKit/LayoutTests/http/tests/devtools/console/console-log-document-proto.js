// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(async function() {
  TestRunner.addResult(
    `Test that console.dir(document.__proto__) won't result in an exception when the message is formatted in the inspector.Bug 27169.\n`
  );

  await TestRunner.loadModule('console_test_runner');
  await TestRunner.showPanel('console');
  await TestRunner.loadHTML(`
    <p>
    Test that console.dir(document.__proto__) won't result in an exception when the message
    is formatted in the inspector.<a bug="https://bugs.webkit.org/show_bug.cgi?id=27169">Bug  27169.</a>
    </p>
  `);
  await TestRunner.evaluateInPagePromise(`
    console.dir(document.__proto__);
  `);

  ConsoleTestRunner.dumpConsoleMessages();
  TestRunner.completeTest();
})();
