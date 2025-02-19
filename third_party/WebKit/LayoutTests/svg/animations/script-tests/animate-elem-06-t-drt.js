description("A copy of the corresponding W3C-SVG-1.1 test, which dumps the animation at certain times");
embedSVGTestCase("../W3C-SVG-1.1/animate-elem-06-t.svg");

// Setup animation test
function sample1() {
    expectTranslationMatrix("getTransformToElement(rootSVGElement, path)", "-15", "-43");
}

function sample2() {
    expectTranslationMatrix("getTransformToElement(rootSVGElement, path)", "-38.5", "-30");
}

function sample3() {
    expectTranslationMatrix("getTransformToElement(rootSVGElement, path)", "-65", "-33");
}

function executeTest() {
    path = rootSVGElement.ownerDocument.getElementsByTagName("path")[1];

    const expectedValues = [
        // [animationId, time, sampleCallback]
        ["an1", 0.0,  sample1],
        ["an1", 3.0,  sample2],
        ["an1", 6.0,  sample3],
        ["an1", 60.0, sample3]
    ];

    runAnimationTest(expectedValues);
}

window.animationStartsImmediately = true;
var successfullyParsed = true;
