<?php header("X-XSS-Protection: 1"); ?>
<!DOCTYPE html>
<html>
<head>
<script src="http://127.0.0.1:8000/foo"></script>
<img src="http://127.0.0.1:8000/foo"/>
</head>
<body>
</body>
</html>
