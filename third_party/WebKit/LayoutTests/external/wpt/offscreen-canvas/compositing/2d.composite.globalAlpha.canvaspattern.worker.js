// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.composite.globalAlpha.canvaspattern
// Description:
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

var offscreenCanvas2 = new OffscreenCanvas(100, 50);
var ctx2 = offscreenCanvas2.getContext('2d');
ctx2.fillStyle = '#f00';
ctx2.fillRect(0, 0, 100, 50);
ctx.fillStyle = '#0f0';
ctx.fillRect(0, 0, 100, 50);
ctx.fillStyle = ctx.createPattern(offscreenCanvas2, 'no-repeat');
ctx.globalAlpha = 0.01; // avoid any potential alpha=0 optimisations
ctx.fillRect(0, 0, 100, 50);
_assertPixelApprox(offscreenCanvas, 50,25, 2,253,0,255, "50,25", "2,253,0,255", 2);

t.done();

});
done();
