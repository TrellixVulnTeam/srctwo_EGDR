<?php
header("Content-Security-Policy: img-src 'none'; report-uri resources/save-report.php?test=report-blocked-file-uri.php");
?>
<!DOCTYPE html>
<html>
<head>
    <script src="resources/report-test.js"></script>
</head>
<body>
    <script>
        testRunner.addOriginAccessWhitelistEntry('http://127.0.0.1:8000', 'file', '', true);
        var localImageLocation = testRunner.pathToLocalResource('file:///tmp/LayoutTests/http/tests/security/resources/compass.jpg');

        var localImageElement = document.createElement('img');
        localImageElement.src = localImageLocation;
        document.body.appendChild(localImageElement);
    </script>
    <script src="resources/go-to-echo-report.js"></script>
</body>
</html>
