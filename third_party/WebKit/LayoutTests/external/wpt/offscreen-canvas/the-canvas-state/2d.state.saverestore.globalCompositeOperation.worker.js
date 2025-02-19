// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.state.saverestore.globalCompositeOperation
// Description:save()/restore() works for globalCompositeOperation
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("save()/restore() works for globalCompositeOperation");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

// Test that restore() undoes any modifications
var old = ctx.globalCompositeOperation;
ctx.save();
ctx.globalCompositeOperation = "copy";
ctx.restore();
_assertSame(ctx.globalCompositeOperation, old, "ctx.globalCompositeOperation", "old");

// Also test that save() doesn't modify the values
ctx.globalCompositeOperation = "copy";
old = ctx.globalCompositeOperation;
    // we're not interested in failures caused by get(set(x)) != x (e.g.
    // from rounding), so compare against 'old' instead of against "copy"
ctx.save();
_assertSame(ctx.globalCompositeOperation, old, "ctx.globalCompositeOperation", "old");
ctx.restore();

t.done();

});
done();
