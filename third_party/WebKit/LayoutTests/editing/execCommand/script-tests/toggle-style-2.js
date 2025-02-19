description("Test to make sure we remove span tags with no attributes if we removed the last attribute.")

var testContainer = document.createElement("div");
testContainer.contentEditable = true;
document.body.appendChild(testContainer);

function testSingleToggle(toggleCommand, initialContents, expectedContents)
{
    testContainer.innerHTML = initialContents;
    window.getSelection().selectAllChildren(testContainer);
    document.execCommand(toggleCommand, false, null);
    if (testContainer.innerHTML === expectedContents) {
        testPassed("one " + toggleCommand + " command converted " + initialContents + " to " + expectedContents);
    } else {
        testFailed("one " + toggleCommand + " command converted " + initialContents + " to " + testContainer.innerHTML + ", expected " + expectedContents);
    }
}

function testDoubleToggle(toggleCommand, initialContents, expectedContents)
{
    testContainer.innerHTML = initialContents;
    window.getSelection().selectAllChildren(testContainer);
    document.execCommand(toggleCommand, false, null);
    document.execCommand(toggleCommand, false, null);
    if (testContainer.innerHTML === expectedContents) {
        testPassed("two " + toggleCommand + " commands converted " + initialContents + " to " + expectedContents);
    } else {
        testFailed("two " + toggleCommand + " commands converted " + initialContents + " to " + testContainer.innerHTML + ", expected " + expectedContents);
    }
}

testSingleToggle("underline", "test", "<u>test</u>");
testSingleToggle("underline", "<u><b><strike>test</strike></b></u>", "<b style=\"\"><strike style=\"\">test</strike></b>");
testDoubleToggle("underline", "test", "test");
testSingleToggle("strikethrough", "test", "<strike>test</strike>");
testSingleToggle("strikethrough", "<u><b><strike>test</strike></b></u>", "<u><b>test</b></u>");
testDoubleToggle("strikethrough", "test", "test");

testSingleToggle("strikethrough", "<u>test</u>", "<u><strike>test</strike></u>");
testSingleToggle("underline", "<strike>test</strike>", "<u><strike>test</strike></u>");

testSingleToggle("strikethrough", '<span style="text-decoration: overline;">test</span>', '<span style="text-decoration-line: overline;"><strike>test</strike></span>');
testSingleToggle("underline", '<span style="text-decoration: overline;">test</span>', '<span style="text-decoration-line: overline;"><u>test</u></span>');

document.body.removeChild(testContainer);

var successfullyParsed = true;
