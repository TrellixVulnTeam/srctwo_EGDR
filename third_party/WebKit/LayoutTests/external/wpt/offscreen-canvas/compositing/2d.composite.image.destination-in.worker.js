// DO NOT EDIT! This test has been generated by tools/gentest.py.
// OffscreenCanvas test in a worker:2d.composite.image.destination-in
// Description:
// Note:

importScripts("/resources/testharness.js");
importScripts("/common/canvas-tests.js");

var t = async_test("");
t.step(function() {

var offscreenCanvas = new OffscreenCanvas(100, 50);
var ctx = offscreenCanvas.getContext('2d');


ctx.fillStyle = 'rgba(0, 255, 255, 0.5)';
ctx.fillRect(0, 0, 100, 50);
ctx.globalCompositeOperation = 'destination-in';
var promise = new Promise(function(resolve, reject) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", '/images/yellow75.png');
    xhr.responseType = 'blob';
    xhr.send();
    xhr.onload = function() {
        resolve(xhr.response);
    };
});
promise.then(function(response) {
    ctx.drawImage(response, 0, 0);
    _assertPixelApprox(offscreenCanvas, 50,25, 0,255,255,96, "50,25", "0,255,255,96", 5);
});

t.done();

});
done();
