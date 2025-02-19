// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function testGetSizeAfterCancelling() {
  // Crockwise 90 degrees image orientation.
  var orientation = new ImageOrientation(0, 1, 1, 0);

  // After cancelling orientation, the width and the height are swapped.
  var size = orientation.getSizeAfterCancelling(100, 200);
  assertEquals(200, size.width);
  assertEquals(100, size.height);
}

function testCancelImageOrientation() {
  // Crockwise 90 degrees image orientation.
  var orientation = new ImageOrientation(0, 1, 1, 0);

  var canvas = document.createElement('canvas');
  canvas.width = 2;
  canvas.height = 1;

  var context = canvas.getContext('2d');
  var imageData = context.createImageData(2, 1);
  imageData.data[0] = 255;  // R
  imageData.data[1] = 0;  // G
  imageData.data[2] = 0;  // B
  imageData.data[3] = 100;  // A
  imageData.data[4] = 0;  // R
  imageData.data[5] = 0;  // G
  imageData.data[6] = 0;  // B
  imageData.data[7] = 100;  // A
  context.putImageData(imageData, 0, 0);

  var destinationCanvas = document.createElement('canvas');
  destinationCanvas.width = 1;
  destinationCanvas.height = 2;
  var destinationContext = destinationCanvas.getContext('2d');
  orientation.cancelImageOrientation(destinationContext, 2, 1);
  destinationContext.drawImage(canvas, 0, 0);
  var destinationImageData = destinationContext.getImageData(0, 0, 1, 2);
  assertArrayEquals([255, 0, 0, 100, 0, 0, 0, 100], destinationImageData.data);
}

function assertImageOrientationEquals(expected, actual, message) {
  assertEquals(expected.a, actual.a, message);
  assertEquals(expected.b, actual.b, message);
  assertEquals(expected.c, actual.c, message);
  assertEquals(expected.d, actual.d, message);
}

function testFromRotationAndScale() {
  var rotate270 = {scaleX: 1, scaleY: 1, rotate90: -1};
  var rotate90 = {scaleX: 1, scaleY: 1, rotate90: 1};
  var flipX = {scaleX: -1, scaleY: 1, rotate90: 0 };
  var flipY = {scaleX: 1, scaleY: -1, rotate90: 0 };
  var flipBoth = {scaleX: -1, scaleY: -1, rotate90: 0};
  var rotate180 = {scaleX: 1, scaleY: 1, rotate90: 2};
  var flipXAndRotate90 = {scaleX: -1, scaleY: 1, rotate90: 1};
  var flipYAndRotate90 = {scaleX: 1, scaleY: -1, rotate90: 1};
  var rotate1080 = {scaleX: 1, scaleY: 1, rotate90: 12};
  var flipBothAndRotate180 = {scaleX: -1, scaleY: -1, rotate90: 2};
  /*
   The image coordinate system is aligned to the screen. (Y+ pointing down)
   O----> e_x                 ^
   |           rotate 270 CW  | e'_x = (0, -1)' = (a, b)'
   |             =====>       |
   V e_y                      O----> e'_y = (1, 0)' = (c, d)'
  */
  assertImageOrientationEquals(new ImageOrientation(0, -1, 1, 0),
      ImageOrientation.fromRotationAndScale(rotate270), 'rotate270');
  assertImageOrientationEquals(new ImageOrientation(0, 1, -1, 0),
      ImageOrientation.fromRotationAndScale(rotate90), 'rotate90');
  assertImageOrientationEquals(new ImageOrientation(-1, 0, 0, 1),
      ImageOrientation.fromRotationAndScale(flipX), 'flipX');
  assertImageOrientationEquals(new ImageOrientation(1, 0, 0, -1),
      ImageOrientation.fromRotationAndScale(flipY), 'flipY');
  assertImageOrientationEquals(new ImageOrientation(-1, 0, 0, -1),
      ImageOrientation.fromRotationAndScale(flipBoth), 'flipBoth');
  assertImageOrientationEquals(new ImageOrientation(-1, 0, 0, -1),
      ImageOrientation.fromRotationAndScale(rotate180), 'rotate180');
  assertImageOrientationEquals(new ImageOrientation(0, -1, -1, 0),
      ImageOrientation.fromRotationAndScale(flipXAndRotate90),
      'flipXAndRotate90');
  assertImageOrientationEquals(new ImageOrientation(0, 1, 1, 0),
      ImageOrientation.fromRotationAndScale(flipYAndRotate90),
      'flipYAndRotate90');
  assertTrue(ImageOrientation.fromRotationAndScale(flipBothAndRotate180)
      .isIdentity(), 'flipBothAndRotate180');
  assertTrue(ImageOrientation.fromRotationAndScale(rotate1080).isIdentity(),
      'rotate1080');
}
