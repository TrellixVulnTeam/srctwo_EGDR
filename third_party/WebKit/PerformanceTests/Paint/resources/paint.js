// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function measurePaint(test) {
  test.tracingCategories = 'blink';
  test.traceEventsToMeasure = [
    'LocalFrameView::prePaint',
    'LocalFrameView::paintTree'
  ];
  PerfTestRunner.measureFrameTime(test);
}
