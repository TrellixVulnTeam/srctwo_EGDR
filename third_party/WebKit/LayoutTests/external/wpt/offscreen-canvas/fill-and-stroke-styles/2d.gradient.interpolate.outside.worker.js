// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.gradient.interpolate.outside
// Description:
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

ctx.fillStyle = '#f00';
ctx.fillRect(0, 0, 100, 50);
var g = ctx.createLinearGradient(25, 0, 75, 0);
g.addColorStop(0.4, '#0f0');
g.addColorStop(0.6, '#0f0');
ctx.fillStyle = g;
ctx.fillRect(0, 0, 100, 50);
_assertPixelApprox(offscreenCanvas, 20,25, 0,255,0,255, "20,25", "0,255,0,255", 2);
_assertPixelApprox(offscreenCanvas, 50,25, 0,255,0,255, "50,25", "0,255,0,255", 2);
_assertPixelApprox(offscreenCanvas, 80,25, 0,255,0,255, "80,25", "0,255,0,255", 2);

t.done();

});
done();
