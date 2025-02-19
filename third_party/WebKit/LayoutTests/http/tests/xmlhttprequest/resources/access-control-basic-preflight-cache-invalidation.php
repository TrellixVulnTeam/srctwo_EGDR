<?php
require_once '../../resources/portabilityLayer.php';

$tmpFile = sys_get_temp_dir() . "/" . $_GET['filename'];

function fail()
{
    header("Access-Control-Allow-Origin: http://127.0.0.1:8000");
    header("Access-Control-Allow-Credentials: true");
    header("Access-Control-Allow-Methods: PUT");
    header("Access-Control-Allow-Headers: x-webkit-test");
    echo "FAIL: " . $_SERVER['REQUEST_METHOD'] . "\n";
    exit();
}

function setState($newState, $file)
{
    file_put_contents($file, $newState);
}

function getState($file)
{
    if (!file_exists($file)) {
        return "Uninitialized";
    }
    return file_get_contents($file);
}

$state = getState($tmpFile);

if ($state == "Uninitialized") {
    if ($_SERVER['REQUEST_METHOD'] == "OPTIONS") {
        header("Access-Control-Allow-Origin: http://127.0.0.1:8000");
        header("Access-Control-Allow-Credentials: true");
        header("Access-Control-Allow-Methods: PUT");
        header("Access-Control-Max-Age: 10"); // 10 seconds
        setState("OptionsSent", $tmpFile);
    } else {
        fail();
    }
} else if ($state == "OptionsSent") {
    if ($_SERVER['REQUEST_METHOD'] == "PUT") {
        header("Access-Control-Allow-Origin: http://127.0.0.1:8000");
        header("Access-Control-Allow-Credentials: true");
        echo "PASS: First PUT request.";
        setState("FirstPUTSent", $tmpFile);
    } else {
        fail();
    }
} else if ($state == "FirstPUTSent") {
    if ($_SERVER['REQUEST_METHOD'] == "OPTIONS") {
        header("Access-Control-Allow-Origin: http://127.0.0.1:8000");
        header("Access-Control-Allow-Credentials: true");
        header("Access-Control-Allow-Methods: PUT, XMETHOD");
        header("Access-Control-Allow-Headers: x-webkit-test");
        setState("SecondOPTIONSSent", $tmpFile);
    } else if ($_SERVER['REQUEST_METHOD'] == "PUT") {
        header("Access-Control-Allow-Origin: http://127.0.0.1:8000");
        header("Access-Control-Allow-Credentials: true");
        echo "FAIL: Second PUT request sent without preflight";
    }
} else if ($state == "SecondOPTIONSSent") {
    if ($_SERVER['REQUEST_METHOD'] == "PUT" || $_SERVER['REQUEST_METHOD'] == "XMETHOD") {
        header("Access-Control-Allow-Origin: http://127.0.0.1:8000");
        header("Access-Control-Allow-Credentials: true");
        echo "PASS: Second OPTIONS request was sent.";
    } else {
        fail();
    }
} else {
    fail();
}
?>
