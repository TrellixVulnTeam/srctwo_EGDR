<?php
header("Content-Security-Policy: sandbox allow-scripts allow-popups");
?>
<!DOCTYPE html>
<html>
<head>
    <script src="/resources/testharness.js"></script>
    <script src="/resources/testharnessreport.js"></script>
</head>
<body>
    <a target='_blank' href='/security/resources/post-origin-to-opener.html'></a>
    <script>
        if (window.testRunner) {
            testRunner.setCanOpenWindows();
            testRunner.setCloseRemainingWindowsWhenComplete(true);
        }

        var test = async_test("Testing sandbox inherited via target='_blank'");

        window.addEventListener("message", test.step_func(function (e) {
            assert_equals(document.origin, 'null');
            assert_equals(e.data.origin, 'null');
            test.done();
        }));

        test.step(function () {
            var link = document.querySelector('a');
            link.click();
        });
    </script>
</body>
</html>
