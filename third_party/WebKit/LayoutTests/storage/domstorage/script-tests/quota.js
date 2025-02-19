description("Test the DOM Storage quota code.");

function testQuota(storageString)
{
    storage = eval(storageString);
    if (!storage) {
        testFailed(storageString + " DOES NOT exist");
        return;
    }

    debug("Testing " + storageString);

    evalAndLog("storage.clear()");
    shouldBe("storage.length", "0");

    debug("Creating 'data' which contains 64K of data");
    data = "X";
    for (var i=0; i<16; i++)
        data += data;
    shouldBe("data.length", "65536");

    debug("Putting 'data' into 39 " + storageString + " buckets.");
    for (var i=0; i<39; i++)
        storage[i] = data;

    debug("Putting 'data' into another bucket.h");
    try {
        storage[39] = data;
        testFailed("Did not hit quota error.");
    } catch (e) {
        testPassed("Hit exception as expected");
    }

    debug("Verify that data was never inserted.");
    shouldBeNull("storage.getItem(39)");

    debug("Removing bucket 38.");
    storage.removeItem('38');

    debug("Adding 'Hello!' into a new bucket.");
    try {
        storage['foo'] = "Hello!";
        testPassed("Insertion worked.");
    } catch (e) {
        testFailed("Exception: " + e);
    }
}

function testNoQuota(storageString)
{
    storage = eval(storageString);
    if (!storage) {
        testFailed(storageString + " DOES NOT exist");
        return;
    }

    debug("Testing " + storageString);

    evalAndLog("storage.clear()");
    shouldBe("storage.length", "0");

    debug("Creating 'data' which contains 64K of data");
    data = "X";
    for (var i=0; i<16; i++)
        data += data;
    shouldBe("data.length", "65536");

    debug("Putting 'data' into 39 " + storageString + " buckets.");
    for (var i=0; i<39; i++)
        storage[i] = data;

    debug("Putting 'data' into another bucket.h");
    try {
        storage[39] = data;
        testPassed("Insertion worked.");
    } catch (e) {
        testFailed("Exception: " + e);
    }
}

testNoQuota("sessionStorage");
debug("");
debug("");
testQuota("localStorage");

window.successfullyParsed = true;
isSuccessfullyParsed();
