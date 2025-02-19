// [Name] SVGTextElement-svgdom-textLength-prop.js
// [Expected rendering result] Text streteched using spaces with a length of 200 - and a series of PASS messages

description("Tests dynamic updates of the 'textLength' property of the SVGTextElement object")
createSVGTestCase();

var textElement = createSVGElement("text");
textElement.setAttribute("x", "0");
textElement.setAttribute("y", "215");
textElement.textContent = "Stretched text";
rootSVGElement.appendChild(textElement);

shouldBeNull("textElement.getAttribute('textLength')");
shouldBeTrue("lastLength = textElement.getComputedTextLength(); lastLength > 0 && lastLength < 200");
shouldBeTrue("lastLength == textElement.textLength.baseVal.value");

function repaintTest() {
    textElement.textLength.baseVal.value = 200;
    shouldBeEqualToString("textElement.getAttribute('textLength')", "200");
    shouldBe("textElement.textLength.baseVal.value", "200");
    shouldBeTrue("textElement.getComputedTextLength() == lastLength");
}

var successfullyParsed = true;
