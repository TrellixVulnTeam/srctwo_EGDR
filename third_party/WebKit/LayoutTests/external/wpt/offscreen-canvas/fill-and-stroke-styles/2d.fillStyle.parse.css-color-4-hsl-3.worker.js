// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.fillStyle.parse.css-color-4-hsl-3
// Description:
// Note:<p class="notes">

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');


ctx.fillStyle = '#f00';
ctx.fillStyle = 'hsl(120.0, 100.0%, 50.0%, 0.2)';
ctx.fillRect(0, 0, 100, 50);
_assertPixel(offscreenCanvas, 50,25, 0,255,0,51, "50,25", "0,255,0,51");

t.done();

});
done();
