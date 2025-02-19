// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.path.isPointInPath.outside
// Description:isPointInPath() works on paths outside the canvas
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("isPointInPath() works on paths outside the canvas");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

ctx.rect(0, -100, 20, 20);
ctx.rect(20, -10, 20, 20);
_assertSame(ctx.isPointInPath(10, -110), false, "ctx.isPointInPath(10, -110)", "false");
_assertSame(ctx.isPointInPath(10, -90), true, "ctx.isPointInPath(10, -90)", "true");
_assertSame(ctx.isPointInPath(10, -70), false, "ctx.isPointInPath(10, -70)", "false");
_assertSame(ctx.isPointInPath(30, -20), false, "ctx.isPointInPath(30, -20)", "false");
_assertSame(ctx.isPointInPath(30, 0), true, "ctx.isPointInPath(30, 0)", "true");
_assertSame(ctx.isPointInPath(30, 20), false, "ctx.isPointInPath(30, 20)", "false");

t.done();

});
done();
