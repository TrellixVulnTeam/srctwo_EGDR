// Copyright 2017 The Chromium Authors. All
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview using private properties isn't a Closure violation in tests.
 * @suppress {accessControls}
 */

ElementsTestRunner.events = [];
ElementsTestRunner.containerId;

ElementsTestRunner.setUpTestSuite = function(next) {
  ElementsTestRunner.expandElementsTree(step1);

  function step1() {
    ElementsTestRunner.selectNodeWithId('container', step2);
  }

  function step2(node) {
    ElementsTestRunner.containerId = node.id;
    TestRunner.DOMAgent.getOuterHTML(ElementsTestRunner.containerId).then(step3);
  }

  function step3(text) {
    ElementsTestRunner.containerText = text;

    for (var key in SDK.DOMModel.Events) {
      var eventName = SDK.DOMModel.Events[key];

      if (eventName === SDK.DOMModel.Events.MarkersChanged || eventName === SDK.DOMModel.Events.DOMMutated)
        continue;

      TestRunner.domModel.addEventListener(
          eventName, ElementsTestRunner.recordEvent.bind(ElementsTestRunner, eventName));
    }

    next();
  }
};

ElementsTestRunner.recordEvent = function(eventName, event) {
  if (!event.data)
    return;

  var node = event.data.node || event.data;
  var parent = event.data.parent;

  for (var currentNode = parent || node; currentNode; currentNode = currentNode.parentNode) {
    if (currentNode.getAttribute('id') === 'output')
      return;
  }

  ElementsTestRunner.events.push('Event ' + eventName.toString() + ': ' + node.nodeName());
};

ElementsTestRunner.patchOuterHTML = function(pattern, replacement, next) {
  TestRunner.addResult('Replacing \'' + pattern + '\' with \'' + replacement + '\'\n');
  ElementsTestRunner.setOuterHTML(ElementsTestRunner.containerText.replace(pattern, replacement), next);
};

ElementsTestRunner.patchOuterHTMLUseUndo = function(pattern, replacement, next) {
  TestRunner.addResult('Replacing \'' + pattern + '\' with \'' + replacement + '\'\n');
  ElementsTestRunner.setOuterHTMLUseUndo(ElementsTestRunner.containerText.replace(pattern, replacement), next);
};

ElementsTestRunner.setOuterHTML = function(newText, next) {
  ElementsTestRunner.innerSetOuterHTML(newText, false, bringBack);

  function bringBack() {
    TestRunner.addResult('\nBringing things back\n');
    ElementsTestRunner.innerSetOuterHTML(ElementsTestRunner.containerText, true, next);
  }
};

ElementsTestRunner.setOuterHTMLUseUndo = function(newText, next) {
  ElementsTestRunner.innerSetOuterHTML(newText, false, bringBack);

  async function bringBack() {
    TestRunner.addResult('\nBringing things back\n');
    await TestRunner.domModel.undo();
    ElementsTestRunner._dumpOuterHTML(true, next);
  }
};

ElementsTestRunner.innerSetOuterHTML = async function(newText, last, next) {
  await TestRunner.DOMAgent.setOuterHTML(ElementsTestRunner.containerId, newText);
  ElementsTestRunner._dumpOuterHTML(last, next);
};

ElementsTestRunner._dumpOuterHTML = async function(last, next) {
  var result = await TestRunner.RuntimeAgent.evaluate('document.getElementById("identity").wrapperIdentity');
  TestRunner.addResult('Wrapper identity: ' + result.value);
  ElementsTestRunner.events.sort();

  for (var i = 0; i < ElementsTestRunner.events.length; ++i)
    TestRunner.addResult(ElementsTestRunner.events[i]);

  ElementsTestRunner.events = [];
  var text = await TestRunner.DOMAgent.getOuterHTML(ElementsTestRunner.containerId);
  TestRunner.addResult('==========8<==========');
  TestRunner.addResult(text);
  TestRunner.addResult('==========>8==========');

  if (last)
    TestRunner.addResult('\n\n\n');

  next();
};
