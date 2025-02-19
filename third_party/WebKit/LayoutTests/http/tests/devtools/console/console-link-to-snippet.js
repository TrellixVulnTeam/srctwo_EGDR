// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(async function() {
  TestRunner.addResult(`Test that link to snippet works.\n`);

  await TestRunner.loadModule('sources_test_runner');
  await TestRunner.loadModule('console_test_runner');
  await TestRunner.showPanel('console');

  TestRunner.addSniffer(Workspace.UISourceCode.prototype, 'addLineMessage', dumpLineMessage, true);

  TestRunner.runTestSuite([
    function testConsoleLogAndReturnMessageLocation(next) {
      ConsoleTestRunner.waitUntilNthMessageReceivedPromise(2)
        .then(() => ConsoleTestRunner.dumpConsoleMessages())
        .then(() => Console.ConsoleView.clearConsole())
        .then(() => next());

      createSnippetPromise('console.log(239);42')
        .then(uiSourceCode => selectSourceCode(uiSourceCode))
        .then(uiSourceCode => renameSourceCodePromise('name1', uiSourceCode))
        .then(() => runSelectedSnippet());
    },

    function testSnippetSyntaxError(next) {
      ConsoleTestRunner.waitUntilNthMessageReceivedPromise(1)
        .then(() => ConsoleTestRunner.dumpConsoleMessages())
        .then(() => Console.ConsoleView.clearConsole())
        .then(() => next());

      createSnippetPromise('\n }')
        .then(uiSourceCode => selectSourceCode(uiSourceCode))
        .then(uiSourceCode => renameSourceCodePromise('name2', uiSourceCode))
        .then(() => runSelectedSnippet());
    },

    function testConsoleErrorHighlight(next) {
      ConsoleTestRunner.waitUntilNthMessageReceivedPromise(1)
        .then(() => ConsoleTestRunner.dumpConsoleMessages())
        .then(() => Console.ConsoleView.clearConsole())
        .then(() => next());

      createSnippetPromise('\n  console.error(42);')
        .then(uiSourceCode => selectSourceCode(uiSourceCode))
        .then(uiSourceCode => renameSourceCodePromise('name3', uiSourceCode))
        .then(() => runSelectedSnippet());
    }
  ]);

  function createSnippetPromise(content) {
    var callback;
    var promise = new Promise(fullfill => (callback = fullfill));
    Snippets.scriptSnippetModel._project.createFile('', null, content, callback);
    return promise;
  }

  function renameSourceCodePromise(newName, uiSourceCode) {
    var callback;
    var promise = new Promise(fullfill => (callback = fullfill));
    uiSourceCode.rename(newName).then(() => callback(uiSourceCode));
    return promise;
  }

  function selectSourceCode(uiSourceCode) {
    return Common.Revealer.revealPromise(uiSourceCode).then(() => uiSourceCode);
  }

  function dumpLineMessage(level, text, lineNumber, columnNumber) {
    TestRunner.addResult(`Line Message was added: ${this.url()} ${level} '${text}':${lineNumber}:${columnNumber}`);
  }

  function runSelectedSnippet() {
    Sources.SourcesPanel.instance()._runSnippet();
  }
})();
