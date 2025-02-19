function test(value, inlineValue, computedValue)
{
    if (value !== null)
        e.style.textDecorationColor = value;
    shouldBeEqualToString("e.style.textDecorationColor", inlineValue);
    computedStyle = window.getComputedStyle(e, null);
    shouldBeEqualToString("computedStyle.textDecorationColor", computedValue);
    debug('');
}

description("Test to make sure text-decoration-color is computed properly.")

var testContainer = document.createElement("div");
testContainer.contentEditable = true;
document.body.appendChild(testContainer);

testContainer.innerHTML = '<div id="test-parent" style="text-decoration-color: green !important;">hello <span id="test-ancestor">world</span></div>';
debug("Ancestor should not inherit 'green' value from parent (fallback to initial value):")
e = document.getElementById('test-ancestor');
test(null, "", "rgb(0, 0, 0)");

debug("Parent should contain 'green':");
e = document.getElementById('test-parent');
test(null, "green", "rgb(0, 128, 0)");

testContainer.innerHTML = '<div id="test-js">test</div>';
debug("JavaScript setter tests for valid, initial, invalid and blank values:");
e = document.getElementById('test-js');
shouldBeEmptyString("e.style.textDecorationColor");

debug("\nValid value 'blue':");
test("blue", "blue", "rgb(0, 0, 255)");

debug("Valid value '#FFFFFF':");
test("#FFFFFF", "rgb(255, 255, 255)", "rgb(255, 255, 255)");

debug("Valid value 'rgb(0, 255, 0)':");
test("rgb(0, 255, 0)", "rgb(0, 255, 0)", "rgb(0, 255, 0)");

debug("Valid value 'rgba(100, 100, 100, 0.5)':");
test("rgba(100, 100, 100, 0.5)", "rgba(100, 100, 100, 0.5)", "rgba(100, 100, 100, 0.5)");

debug("Valid value 'hsl(240, 100%, 50%)':");
test("hsl(240, 100%, 50%)", "rgb(0, 0, 255)", "rgb(0, 0, 255)");

debug("Valid value 'hsla(240, 100%, 50%, 0.5)':");
test("hsla(240, 100%, 50%, 0.5)", "rgba(0, 0, 255, 0.498)", "rgba(0, 0, 255, 0.498)");

debug("Initial value:");
test("initial", "initial", "rgb(0, 0, 0)");

debug("Invalid value (ie. 'unknown'):");
test("unknown", "initial", "rgb(0, 0, 0)");

debug("Empty value (resets the property):");
test("", "", "rgb(0, 0, 0)");

debug("Empty value with different 'currentColor' initial value (green):")
e.style.color = 'green';
test("", "", "rgb(0, 128, 0)");

document.body.removeChild(testContainer);
