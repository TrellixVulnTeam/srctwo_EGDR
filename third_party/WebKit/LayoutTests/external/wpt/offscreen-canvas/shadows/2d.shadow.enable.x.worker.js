// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.shadow.enable.x
// Description:Shadows are drawn if shadowOffsetX is set
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("Shadows are drawn if shadowOffsetX is set");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

ctx.globalCompositeOperation = 'destination-atop';
ctx.shadowColor = '#0f0';
ctx.shadowOffsetX = 0.1;
ctx.fillStyle = '#f00';
ctx.fillRect(0, 0, 100, 50);
_assertPixel(offscreenCanvas, 50,25, 0,255,0,255, "50,25", "0,255,0,255");

t.done();

});
done();
