// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.path.isPointInPath.empty
// Description:isPointInPath() works when there is no path
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("isPointInPath() works when there is no path");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

_assertSame(ctx.isPointInPath(0, 0), false, "ctx.isPointInPath(0, 0)", "false");

t.done();

});
done();
