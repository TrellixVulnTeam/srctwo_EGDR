// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(async function() {
  TestRunner.addResult(`Tests that console dir makes messages expandable only when necessary.\n`);

  await TestRunner.loadModule('console_test_runner');
  await TestRunner.showPanel('console');

  await TestRunner.evaluateInPagePromise(`
    // Primitive values should not get properties section or expand arrow.
    console.dir(true);
    console.dir(new Boolean(true));

    console.dir("foo");
    console.dir(new String("foo"));

    //# sourceURL=console-dir-primitives.js
  `);

  var consoleView = Console.ConsoleView.instance();
  var viewMessages = consoleView._visibleViewMessages;
  for (var i = 0; i < viewMessages.length; ++i) {
    var messageElement = viewMessages[i].element();
    TestRunner.addResult('Message text: ' + messageElement.deepTextContent());
    if (messageElement.querySelector('.console-view-object-properties-section'))
      TestRunner.addResult('Message is expandable');
    else
      TestRunner.addResult('Message is not expandable');
    TestRunner.addResult("");
  }
  TestRunner.completeTest();
})();
