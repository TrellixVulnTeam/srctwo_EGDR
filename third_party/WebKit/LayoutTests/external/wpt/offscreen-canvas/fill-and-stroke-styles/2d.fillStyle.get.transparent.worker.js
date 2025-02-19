// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.fillStyle.get.transparent
// Description:
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

ctx.fillStyle = 'rgba(0,0,0,0)';
_assertSame(ctx.fillStyle, 'rgba(0, 0, 0, 0)', "ctx.fillStyle", "'rgba(0, 0, 0, 0)'");

t.done();

});
done();
