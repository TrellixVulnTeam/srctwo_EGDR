if (this.importScripts) {
    importScripts('../resources/fs-worker-common.js');
    importScripts('../../../resources/js-test.js');
    importScripts('../resources/fs-test-util.js');
}

description("requestFileSystem PERSISTENT test.");

var fileSystem = null;

function errorCallback(error) {
    debug("Error occurred while requesting a PERSISTENT file system:" + error.name);
    finishJSTest();
}

function successCallback(fs) {
    fileSystem = fs;
    debug("Successfully obtained PERSISTENT FileSystem:" + fileSystem.name);
    shouldBeTrue("fileSystem.name.length > 0");
    shouldBe("fileSystem.root.fullPath", '"/"');
    finishJSTest();
}

var jsTestIsAsync = true;
webkitRequestFileSystem(PERSISTENT, 100, successCallback, errorCallback);
