// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.state.saverestore.underflow
// Description:restore() with an empty stack has no effect
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("restore() with an empty stack has no effect");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

for (var i = 0; i < 16; ++i)
    ctx.restore();
ctx.lineWidth = 0.5;
ctx.restore();
_assertSame(ctx.lineWidth, 0.5, "ctx.lineWidth", "0.5");

t.done();

});
done();
