// Copyright 2017 The Chromium Authors. All
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview using private properties isn't a Closure violation in tests.
 * @suppress {accessControls}
 */

ElementsTestRunner.doAddAttribute = function(testName, dataNodeId, attributeText, next) {
  ElementsTestRunner.domActionTestForNodeId(testName, dataNodeId, testBody, next);

  function testBody(node, done) {
    ElementsTestRunner.editNodePart(node, 'webkit-html-attribute');
    eventSender.keyDown('Tab');
    TestRunner.deprecatedRunAfterPendingDispatches(testContinuation);

    function testContinuation() {
      var editorElement = UI.panels.elements._treeOutlines[0]._shadowRoot.getSelection().anchorNode.parentElement;
      editorElement.textContent = attributeText;
      editorElement.dispatchEvent(TestRunner.createKeyEvent('Enter'));
      TestRunner.addSniffer(Elements.ElementsTreeOutline.prototype, '_updateModifiedNodes', done);
    }
  }
};

ElementsTestRunner.domActionTestForNodeId = function(testName, dataNodeId, testBody, next) {
  function callback(testNode, continuation) {
    ElementsTestRunner.selectNodeWithId(dataNodeId, continuation);
  }

  ElementsTestRunner.domActionTest(testName, callback, testBody, next);
};

ElementsTestRunner.domActionTest = function(testName, dataNodeSelectionCallback, testBody, next) {
  var testNode = ElementsTestRunner.expandedNodeWithId(testName);
  TestRunner.addResult('==== before ====');
  ElementsTestRunner.dumpElementsTree(testNode);
  dataNodeSelectionCallback(testNode, step0);

  function step0(node) {
    TestRunner.deprecatedRunAfterPendingDispatches(step1.bind(null, node));
  }

  function step1(node) {
    testBody(node, step2);
  }

  function step2() {
    TestRunner.addResult('==== after ====');
    ElementsTestRunner.dumpElementsTree(testNode);
    next();
  }
};

ElementsTestRunner.editNodePart = function(node, className) {
  var treeElement = ElementsTestRunner.firstElementsTreeOutline().findTreeElement(node);
  var textElement = treeElement.listItemElement.getElementsByClassName(className)[0];

  if (!textElement && treeElement.childrenListElement)
    textElement = treeElement.childrenListElement.getElementsByClassName(className)[0];

  treeElement._startEditingTarget(textElement);
  return textElement;
};

ElementsTestRunner.editNodePartAndRun = function(node, className, newValue, step2, useSniffer) {
  var editorElement = ElementsTestRunner.editNodePart(node, className);
  editorElement.textContent = newValue;
  editorElement.dispatchEvent(TestRunner.createKeyEvent('Enter'));

  if (useSniffer)
    TestRunner.addSniffer(Elements.ElementsTreeOutline.prototype, '_updateModifiedNodes', step2);
  else
    TestRunner.deprecatedRunAfterPendingDispatches(step2);
};
