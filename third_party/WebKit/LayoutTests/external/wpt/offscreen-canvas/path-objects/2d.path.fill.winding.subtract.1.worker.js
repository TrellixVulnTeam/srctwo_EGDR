// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.path.fill.winding.subtract.1
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
ctx.fillStyle = '#f00';
ctx.moveTo(-10, -10);
ctx.lineTo(110, -10);
ctx.lineTo(110, 60);
ctx.lineTo(-10, 60);
ctx.lineTo(-10, -10);
ctx.lineTo(0, 0);
ctx.lineTo(0, 50);
ctx.lineTo(100, 50);
ctx.lineTo(100, 0);
ctx.fill();
_assertPixel(offscreenCanvas, 50,25, 0,255,0,255, "50,25", "0,255,0,255");

t.done();

});
done();
