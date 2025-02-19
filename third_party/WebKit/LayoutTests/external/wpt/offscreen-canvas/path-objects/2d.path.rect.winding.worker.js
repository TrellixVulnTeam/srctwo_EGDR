// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.path.rect.winding
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
ctx.beginPath();
ctx.fillStyle = '#f00';
ctx.rect(0, 0, 50, 50);
ctx.rect(100, 50, -50, -50);
ctx.rect(0, 25, 100, -25);
ctx.rect(100, 25, -100, 25);
ctx.fill();
_assertPixel(offscreenCanvas, 25,12, 0,255,0,255, "25,12", "0,255,0,255");
_assertPixel(offscreenCanvas, 75,12, 0,255,0,255, "75,12", "0,255,0,255");
_assertPixel(offscreenCanvas, 25,37, 0,255,0,255, "25,37", "0,255,0,255");
_assertPixel(offscreenCanvas, 75,37, 0,255,0,255, "75,37", "0,255,0,255");

t.done();

});
done();
