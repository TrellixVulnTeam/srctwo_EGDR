// Copyright 2017 The Chromium Authors. All
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview using private properties isn't a Closure violation in tests.
 * @suppress {accessControls}
 */

SourcesTestRunner.replaceInSource = function(sourceFrame, string, replacement) {
  sourceFrame._textEditor.setReadOnly(false);

  for (var i = 0; i < sourceFrame._textEditor.linesCount; ++i) {
    var line = sourceFrame._textEditor.line(i);
    var column = line.indexOf(string);

    if (column === -1)
      continue;

    range = new TextUtils.TextRange(i, column, i, column + string.length);
    break;
  }
};

SourcesTestRunner.commitSource = function(sourceFrame) {
  sourceFrame.commitEditing();
};

SourcesTestRunner.undoSourceEditing = function(sourceFrame) {
  sourceFrame._textEditor.undo();
};
