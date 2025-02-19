<?php
    header("Content-Security-Policy-Report-Only: img-src http://allowed.test");
?>
<!DOCTYPE html>
<script src="/resources/testharness.js"></script>
<script src="/resources/testharnessreport.js"></script>
<script>
function createListener(expectedURL, test) {
    var listener = test.step_func(e => {
        if (e.blockedURI == expectedURL) {
            document.removeEventListener('securitypolicyviolation', listener);
            test.done();
        }
    });
    document.addEventListener('securitypolicyviolation', listener);
}

async_test(t => {
    var i = document.createElement('img');
    createListener("http://127.0.0.1:8000/security/resources/compass.jpg?t=1", t);
    i.src = "http://127.0.0.1:8000/security/resources/compass.jpg?t=1";
}, "Direct block, same-origin = full URL in report");

async_test(t => {
    var i = document.createElement('img');
    createListener("http://blocked.test:8000/security/resources/compass.jpg?t=2", t);
    i.src = "http://blocked.test:8000/security/resources/compass.jpg?t=2";
}, "Direct block, cross-origin = full URL in report");

async_test(t => {
    var i = document.createElement('img');
    var url = "http://allowed.test:8000/security/resources/redir.php?url=" + encodeURIComponent("http://127.0.0.1:8000/security/resources/compass.jpg?t=3");
    createListener(url, t);
    i.src = url;
}, "Block after redirect, same-origin = original URL in report");

async_test(t => {
    var i = document.createElement('img');
    var url = "http://allowed.test:8000/security/resources/redir.php?url=" + encodeURIComponent("http://blocked.test:8000/security/resources/compass.jpg?t=4");
    createListener(url, t);
    i.src = url;
}, "Block after redirect, cross-origin = original URL in report");
</script>
