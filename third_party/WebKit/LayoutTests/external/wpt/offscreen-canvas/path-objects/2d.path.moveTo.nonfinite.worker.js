// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.path.moveTo.nonfinite
// Description:moveTo() with Infinity/NaN is ignored
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("moveTo() with Infinity/NaN is ignored");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

ctx.moveTo(0, 0);
ctx.lineTo(100, 0);
ctx.moveTo(Infinity, 50);
ctx.moveTo(-Infinity, 50);
ctx.moveTo(NaN, 50);
ctx.moveTo(0, Infinity);
ctx.moveTo(0, -Infinity);
ctx.moveTo(0, NaN);
ctx.moveTo(Infinity, Infinity);
ctx.lineTo(100, 50);
ctx.lineTo(0, 50);
ctx.fillStyle = '#0f0';
ctx.fill();
_assertPixel(offscreenCanvas, 50,25, 0,255,0,255, "50,25", "0,255,0,255");

t.done();

});
done();
