// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.imageData.get.nonfinite
// Description:getImageData() throws TypeError if arguments are not finite
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("getImageData() throws TypeError if arguments are not finite");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

assert_throws(new TypeError(), function() { ctx.getImageData(Infinity, 10, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(-Infinity, 10, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(NaN, 10, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, Infinity, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, -Infinity, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, NaN, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, Infinity, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, -Infinity, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, NaN, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, 10, Infinity); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, 10, -Infinity); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, 10, NaN); });
assert_throws(new TypeError(), function() { ctx.getImageData(Infinity, Infinity, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(Infinity, Infinity, Infinity, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(Infinity, Infinity, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.getImageData(Infinity, Infinity, 10, Infinity); });
assert_throws(new TypeError(), function() { ctx.getImageData(Infinity, 10, Infinity, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(Infinity, 10, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.getImageData(Infinity, 10, 10, Infinity); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, Infinity, Infinity, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, Infinity, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, Infinity, 10, Infinity); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, Infinity, Infinity); });
var posinfobj = { valueOf: function() { return Infinity; } },
    neginfobj = { valueOf: function() { return -Infinity; } },
    nanobj = { valueOf: function() { return -Infinity; } };
assert_throws(new TypeError(), function() { ctx.getImageData(posinfobj, 10, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(neginfobj, 10, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(nanobj, 10, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, posinfobj, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, neginfobj, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, nanobj, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, posinfobj, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, neginfobj, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, nanobj, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, 10, posinfobj); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, 10, neginfobj); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, 10, nanobj); });
assert_throws(new TypeError(), function() { ctx.getImageData(posinfobj, posinfobj, 10, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(posinfobj, posinfobj, posinfobj, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(posinfobj, posinfobj, posinfobj, posinfobj); });
assert_throws(new TypeError(), function() { ctx.getImageData(posinfobj, posinfobj, 10, posinfobj); });
assert_throws(new TypeError(), function() { ctx.getImageData(posinfobj, 10, posinfobj, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(posinfobj, 10, posinfobj, posinfobj); });
assert_throws(new TypeError(), function() { ctx.getImageData(posinfobj, 10, 10, posinfobj); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, posinfobj, posinfobj, 10); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, posinfobj, posinfobj, posinfobj); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, posinfobj, 10, posinfobj); });
assert_throws(new TypeError(), function() { ctx.getImageData(10, 10, posinfobj, posinfobj); });

t.done();

});
done();
