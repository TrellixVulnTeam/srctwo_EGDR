<?php
header("Content-Security-Policy-Report-Only: script-src 'self'; report-uri resources/save-report.php?test=report-only.php");
?>
<script src="resources/report-test.js"></script>
<script>
// This script block will trigger a violation report but shouldn't be blocked.
alert('PASS');
</script>
<script src="resources/go-to-echo-report.js"></script>
