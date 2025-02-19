// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(async function() {
  TestRunner.addResult(`Tests that console.group/groupEnd messages won't be coalesced. Bug 56114. Bug 63521.\n`);

  await TestRunner.loadModule('console_test_runner');
  await TestRunner.showPanel('console');
  await TestRunner.loadHTML(`
    <p>
    Tests that console.group/groupEnd messages won't be coalesced. <a href="https://bugs.webkit.org/show_bug.cgi?id=56114">Bug 56114.</a>
    <a href="https://bugs.webkit.org/show_bug.cgi?id=63521">Bug 63521.</a>

    </p>
  `);
  await TestRunner.evaluateInPagePromise(`
    console.group("outer group");
    console.group("inner group");
    console.log("Message inside inner group");
    console.groupEnd();
    console.groupEnd();
    console.log("Message that must not be in any group");


    var groupCount = 3;
    for (var i = 0; i < groupCount; i++) {
      console.group("One of several groups which shouldn't be coalesced.");
    }
    console.log("Message inside third group");
    for (var i = 0; i < groupCount; i++) {
      console.groupEnd();
    }
  `);

  ConsoleTestRunner.dumpConsoleMessagesWithClasses();
  TestRunner.completeTest();
})();
