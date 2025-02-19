// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.drawImage.zerocanvas
// Description:
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

ctx.fillStyle = '#0f0';
ctx.fillRect(0, 0, 100, 50);
var offscreenCanvas2 = new OffscreenCanvas(0, 10);
ctx.drawImage(offscreenCanvas2, 0, 0);
offscreenCanvas2.width = 10;
offscreenCanvas2.height = 0;
ctx.drawImage(offscreenCanvas2, 0, 0);
offscreenCanvas2.width = 0;
offscreenCanvas2.height = 0;
ctx.drawImage(offscreenCanvas2, 0, 0);
_assertPixelApprox(offscreenCanvas, 50,25, 0,255,0,255, "50,25", "0,255,0,255", 2);

t.done();

});
done();
