function test(value, inlineValue, computedValue)
{
    if (value !== null)
        e.style.textUnderlinePosition = value;
    shouldBeEqualToString("e.style.textUnderlinePosition", inlineValue);
    computedStyle = window.getComputedStyle(e, null);
    shouldBeEqualToString("computedStyle.textUnderlinePosition", computedValue);
    debug('');
}

description("Test to make sure text-underline-position is computed properly.")

// FIXME: This test tests property values 'auto' and 'under'. We don't fully match
// the specification as we don't support [ left | right ] and this is left for another implementation
// as the rendering will need to be added.

var testContainer = document.createElement("div");
testContainer.contentEditable = true;
document.body.appendChild(testContainer);

testContainer.innerHTML = '<div id="test">hello world</div>';

debug("Initial value:");
e = document.getElementById('test');
test(null, "", "auto");

debug("Value '':");
test("", "", "auto");

debug("Initial value (explicit):");
test("initial", "initial", "auto");

debug("Value 'auto':");
test("auto", "auto", "auto");

debug("Value 'under':");
test("under", "under", "under");

testContainer.innerHTML = '<div id="test-parent" style="text-underline-position: under;">hello <span id="test-ancestor">world</span></div>';
debug("Ancestor inherits values from parent:");
e = document.getElementById('test-ancestor');
test(null, "", "under");

debug("Value 'auto under':");
test("auto under", "", "under");

debug("Value 'under under':");
test("under under", "", "under");

debug("Value 'auto alphabetic under':");
test("auto alphabetic under", "", "under");

document.body.removeChild(testContainer);
