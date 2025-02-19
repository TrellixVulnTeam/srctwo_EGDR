// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.gradient.radial.nonfinite
// Description:createRadialGradient() throws TypeError if arguments are not finite
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("createRadialGradient() throws TypeError if arguments are not finite");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');

assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, 1, 0, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(-Infinity, 0, 1, 0, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(NaN, 0, 1, 0, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, 1, 0, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, -Infinity, 1, 0, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, NaN, 1, 0, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, Infinity, 0, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, -Infinity, 0, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, NaN, 0, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, 1, Infinity, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, 1, -Infinity, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, 1, NaN, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, 1, 0, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, 1, 0, -Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, 1, 0, NaN, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, 1, 0, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, 1, 0, 0, -Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, 1, 0, 0, NaN); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, 1, 0, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, Infinity, 0, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, Infinity, Infinity, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, Infinity, Infinity, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, Infinity, Infinity, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, Infinity, Infinity, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, Infinity, 0, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, Infinity, 0, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, Infinity, 0, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, 1, Infinity, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, 1, Infinity, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, 1, Infinity, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, 1, Infinity, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, 1, 0, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, 1, 0, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, Infinity, 1, 0, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, Infinity, 0, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, Infinity, Infinity, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, Infinity, Infinity, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, Infinity, Infinity, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, Infinity, Infinity, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, Infinity, 0, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, Infinity, 0, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, Infinity, 0, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, 1, Infinity, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, 1, Infinity, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, 1, Infinity, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, 1, Infinity, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, 1, 0, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, 1, 0, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(Infinity, 0, 1, 0, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, Infinity, 0, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, Infinity, Infinity, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, Infinity, Infinity, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, Infinity, Infinity, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, Infinity, Infinity, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, Infinity, 0, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, Infinity, 0, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, Infinity, 0, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, 1, Infinity, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, 1, Infinity, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, 1, Infinity, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, 1, Infinity, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, 1, 0, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, 1, 0, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, Infinity, 1, 0, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, Infinity, Infinity, 0, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, Infinity, Infinity, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, Infinity, Infinity, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, Infinity, Infinity, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, Infinity, 0, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, Infinity, 0, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, Infinity, 0, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, 1, Infinity, Infinity, 1); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, 1, Infinity, Infinity, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, 1, Infinity, 0, Infinity); });
assert_throws(new TypeError(), function() { ctx.createRadialGradient(0, 0, 1, 0, Infinity, Infinity); });

t.done();

});
done();
