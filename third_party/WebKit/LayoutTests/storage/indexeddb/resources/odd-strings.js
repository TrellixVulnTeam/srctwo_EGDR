if (this.importScripts) {
    importScripts('../../../resources/js-test.js');
    importScripts('shared.js');
}

description("Test IndexedDB odd value datatypes");

function test()
{
    testData = [{ description: 'empty string',       name: '' },
                { description: 'null',               name: '\u0000' },
                { description: 'faihu',              name: '\ud800\udf46' },
                { description: 'unpaired surrogate', name: '\ud800' },
                { description: 'fffe',               name: '\ufffe' },
                { description: 'ffff',               name: '\uffff' },
                { description: 'line separator',     name: '\u2028' }
    ];
    nextToOpen = 0;
    var numDeleted = 0;
    debug("Deleting all the databases...");
    for (var i = 0; i < testData.length; i++)
    {
        request = indexedDB.deleteDatabase(testData[i].name);
        request.onblocked = unexpectedBlockedCallback;
        request.onerror = unexpectedErrorCallback;
        request.onsuccess = function() {
            if (++numDeleted == testData.length)
                openNextDatabase();
        };
    }
}

function openNextDatabase()
{
    debug("opening a database named " + testData[nextToOpen].description);
    request = evalAndLog("indexedDB.open(testData[nextToOpen].name, 1)");
    request.onsuccess = unexpectedSuccessCallback;
    request.onblocked = unexpectedBlockedCallback;
    request.onupgradeneeded = addAKey;
}

function addAKey()
{
    db = event.target.result;
    evalAndLog("objectStore = db.createObjectStore(testData[nextToOpen].name);");
    evalAndLog("index = objectStore.createIndex(testData[nextToOpen].name, 'keyPath');");
    evalAndLog("key = testData[nextToOpen].name");
    evalAndLog("request = objectStore.add(key, key);");
    request.onsuccess = closeDatabase;
    request.onerror = unexpectedErrorCallback;
}

function closeDatabase()
{
    evalAndLog("db.close()");
    debug("");
    if (++nextToOpen < testData.length) {
        openNextDatabase();
    } else {
        nextToOpen = 0;
        verifyNextDatabase();
    }
}

function verifyNextDatabase()
{
    debug("reopening a database named " + testData[nextToOpen].description);
    request = evalAndLog("indexedDB.open(testData[nextToOpen].name, 2)");
    request.onblocked = unexpectedBlockedCallback;
    request.onerror = unexpectedErrorCallback;
    request.onupgradeneeded = getAKey;
}

function getAKey()
{
    evalAndLog("trans = event.target.transaction");
    evalAndLog("objectStore = trans.objectStore(testData[nextToOpen].name);");
    evalAndLog("key = testData[nextToOpen].name");
    evalAndLog("request = objectStore.openCursor();");
    request.onerror = unexpectedErrorCallback;
    request.onsuccess = openCursorSuccess;
}

function openCursorSuccess()
{
    evalAndLog("cursor = event.target.result;");
    shouldBe("cursor.key", "testData[nextToOpen].name");
    shouldBe("cursor.value", "testData[nextToOpen].name");
    if (++nextToOpen < testData.length) {
        debug("");
        verifyNextDatabase();
    } else {
        finishJSTest();
    }
}

test();
