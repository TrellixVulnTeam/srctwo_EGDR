<?php header("X-XSS-Protection: 1"); ?>
<!DOCTYPE html>
<html>
<head>
</head>
<body>
<script>
window.onload = function()
{
    window.setTimeout(clickAnchorLink, 100);
}
function clickAnchorLink()
{
    var event = document.createEvent('MouseEvent');
    event.initEvent('click', true, true);
    document.getElementById("anchorLink").dispatchEvent(event);
    
    if (window.testRunner)
        testRunner.notifyDone();
}
</script>
<script>document.write(unescape(window.location));</script>
</body>
</html>
