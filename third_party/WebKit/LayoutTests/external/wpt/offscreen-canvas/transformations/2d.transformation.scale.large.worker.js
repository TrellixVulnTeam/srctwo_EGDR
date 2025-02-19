// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.transformation.scale.large
// Description:scale() with large scale factors works
// Note:<p class="notes">Not really that large at all, but it hits the limits in Firefox.

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("scale() with large scale factors works");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

ctx.fillStyle = '#f00';
ctx.fillRect(0, 0, 100, 50);
ctx.scale(1e5, 1e5);
ctx.fillStyle = '#0f0';
ctx.fillRect(0, 0, 1, 1);
_assertPixel(offscreenCanvas, 50,25, 0,255,0,255, "50,25", "0,255,0,255");

t.done();

});
done();
