// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function setCompositionTest() {
  chrome.input.ime.setComposition({
    "contextID": 1,
    "text": "Pie",
    "selectionStart": 1,
    "selectionEnd": 2,
    "cursor": 3,
    "segments": [{
      "start": 0,
      "end": 1,
      "style": "underline"
    }]
  }, chrome.test.callbackPass());
}


function clearCompositionTest() {
  chrome.input.ime.clearComposition({
    "contextID": 1
  }, chrome.test.callbackPass());
}


function commitTextTest() {
  chrome.input.ime.commitText({
    "contextID": 2,
    "text": "Seaguls"
  }, chrome.test.callbackPass());
}


// Disabled: crbug.com/641425.
// function setCandidateWindowPropertiesTest() {
//   chrome.input.ime.setCandidateWindowProperties({
//     "engineID": "test",
//     "properties": {
//       "visible": true,
//       "cursorVisible": false,
//       "vertical": true,
//       "pageSize": 6,
//       "auxiliaryText": "notes",
//       "auxiliaryTextVisible": true
//     }
//   }, chrome.test.callbackPass());
// }


function setCandidatesTest() {
  chrome.input.ime.setCandidates({
    "contextID": 8,
    "candidates": [{
      "candidate": "one",
      "id": 1,
      "label": "first",
      "annotation": "The first one"
    }, {
      "candidate": "two",
      "id": 2,
      "label": "second",
      "annotation": "The second one"
    }, {
      "candidate": "three",
      "id": 3,
      "label": "third",
      "annotation": "The third one"
    }]
  }, chrome.test.callbackPass());
}


function setCursorPositionTest() {
  chrome.input.ime.setCursorPosition({
    "contextID": 9,
    "candidateID": 1
  }, chrome.test.callbackPass());
}


// Disabled: crbug.com/641425.
// function setMenuItemsTest() {
//   chrome.input.ime.setMenuItems({
//     "engineID": "test",
//     "items": [{
//       "id": "Menu 1",
//       "label": "Menu 1",
//       "style": "check",
//       "visible": true,
//       "enabled": true
//     }, {
//       "id": "Menu 2",
//       "label": "Menu 2",
//       "style": "radio",
//       "visible": true,
//       "enabled": true
//     }]
//   }, chrome.test.callbackPass());
// }

// Disabled: crbug.com/641425.
// function updateMenuItemsTest() {
//   chrome.input.ime.updateMenuItems({
//     "engineID": "test",
//     "items": [{
//       "id": "Menu 1",
//       "enabled": false
//     }, {
//       "id": "Menu 2",
//       "visible": false,
//     }]
//   }, chrome.test.callbackPass());
// }

// Disabled: crbug.com/641425.
// function deleteSurroundingText() {
//   chrome.input.ime.deleteSurroundingText({
//     "engineID": "test",
//     "contextID": 1,
//     "offset": -1,
//     "length": 1
//   }, chrome.test.callbackPass());
// }

chrome.test.runTests([
    setCompositionTest,
    clearCompositionTest,
    commitTextTest,
    // setCandidateWindowPropertiesTest,
    setCandidatesTest,
    setCursorPositionTest,
    // setMenuItemsTest,
    // updateMenuItemsTest,
    // deleteSurroundingText,
]);
