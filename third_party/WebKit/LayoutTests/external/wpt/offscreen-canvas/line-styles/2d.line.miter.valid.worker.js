// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.line.miter.valid
// Description:Setting miterLimit to valid values works
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("Setting miterLimit to valid values works");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

ctx.miterLimit = 1.5;
_assertSame(ctx.miterLimit, 1.5, "ctx.miterLimit", "1.5");
ctx.miterLimit = "1e1";
_assertSame(ctx.miterLimit, 10, "ctx.miterLimit", "10");
ctx.miterLimit = 1/1024;
_assertSame(ctx.miterLimit, 1/1024, "ctx.miterLimit", "1/1024");
ctx.miterLimit = 1000;
_assertSame(ctx.miterLimit, 1000, "ctx.miterLimit", "1000");

t.done();

});
done();
