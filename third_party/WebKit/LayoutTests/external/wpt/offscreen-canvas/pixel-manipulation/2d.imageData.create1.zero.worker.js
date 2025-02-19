// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.imageData.create1.zero
// Description:createImageData(null) throws TypeError
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("createImageData(null) throws TypeError");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

assert_throws(new TypeError(), function() { ctx.createImageData(null); });

t.done();

});
done();
