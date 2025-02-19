description('Test to make sure we push down inline styles properly.');

if (window.internals)
    internals.settings.setEditingBehavior('win');
var testContainer = document.createElement("div");
testContainer.contentEditable = true;
document.body.appendChild(testContainer);

function testSingleToggle(toggleCommand, selector, initialContents, expectedContents)
{
    testContainer.innerHTML = initialContents;
    var selected = selector(testContainer);
    document.execCommand('styleWithCSS', false, 'false');
    document.execCommand(toggleCommand, false, null);
    var action = toggleCommand + ' on ' + selected + ' of ' + initialContents + " yields " + testContainer.innerHTML;
    if (testContainer.innerHTML == expectedContents)
        testPassed(action);
    else
        testFailed(action + ", expected " + expectedContents);
}

function selectFirstWord(container) {
    window.getSelection().collapse(container, 0);
    window.getSelection().modify('extend', 'forward', 'word');
    return 'first word';
}

function selectLastWord(container) {
    window.getSelection().collapse(container, container.childNodes.length);
    window.getSelection().modify('extend', 'backward', 'word');
    return 'last word';
}

testSingleToggle("bold", selectFirstWord, '<b><ul><li><b>a</b></li></ul></b>', '<ul style=""><li style="">a</li></ul>');
testSingleToggle("bold", selectFirstWord, '<b><ul><li>hello</li><li>world</li></ul></b>', '<ul style=""><li style="">hello</li><li style="font-weight: bold;">world</li></ul>');
testSingleToggle("bold", selectLastWord, '<ul><li>hello</li><li style="font-weight: bold;">world</li></ul>', '<ul><li>hello</li><li style="">world</li></ul>');
testSingleToggle("bold", selectFirstWord, '<b><ul><li>hello world</li><li>webkit</li></ul></b>', '<ul style=""><li style="">hello <b>world</b></li><li style="font-weight: bold;">webkit</li></ul>');

testSingleToggle("italic", selectFirstWord, '<i><ul><li><i>a</i></li></ul></i>', '<ul style=""><li style="">a</li></ul>');
testSingleToggle("italic", selectFirstWord, '<i><ul><li>hello</li><li>world</li></ul></i>', '<ul style=""><li style="">hello</li><li style="font-style: italic;">world</li></ul>');
testSingleToggle("italic", selectLastWord, '<ul><li>hello</li><li style="font-style: italic;">world</li></ul>', '<ul><li>hello</li><li style="">world</li></ul>');
testSingleToggle("italic", selectFirstWord, '<i><ul><li>hello world</li><li>webkit</li></ul></i>', '<ul style=""><li style="">hello <i>world</i></li><li style="font-style: italic;">webkit</li></ul>');

testSingleToggle("underline", selectFirstWord, '<u><ul><li><u>a</u></li></ul></u>', '<ul style=""><li style="">a</li></ul>');
testSingleToggle("underline", selectFirstWord, '<u><ul><li>hello</li><li>world</li></ul></u>', '<ul style=""><li style="">hello</li><li style="text-decoration-line: underline;">world</li></ul>');
testSingleToggle("underline", selectLastWord, '<ul><li>hello</li><li style="text-decoration: underline;">world</li></ul>', '<ul><li>hello</li><li style="">world</li></ul>');
testSingleToggle("underline", selectFirstWord, '<u><ul><li>hello world</li><li>webkit</li></ul></u>', '<ul style=""><li style="">hello <u>world</u></li><li style="text-decoration-line: underline;">webkit</li></ul>');

testSingleToggle("strikethrough", selectFirstWord, '<strike><ul><li><strike>a</strike></li></ul></strike>', '<ul style=""><li style="">a</li></ul>');
testSingleToggle("strikethrough", selectFirstWord, '<strike><ul><li>hello</li><li>world</li></ul></strike>', '<ul style=""><li style="">hello</li><li style="text-decoration-line: line-through;">world</li></ul>');
testSingleToggle("strikethrough", selectLastWord, '<ul><li>hello</li><li style="text-decoration: line-through;">world</li></ul>', '<ul><li>hello</li><li style="">world</li></ul>');
testSingleToggle("strikethrough", selectFirstWord, '<strike><ul><li>hello world</li><li>webkit</li></ul></strike>', '<ul style=""><li style="">hello <strike>world</strike></li><li style="text-decoration-line: line-through;">webkit</li></ul>');

document.body.removeChild(testContainer);

var successfullyParsed = true;
