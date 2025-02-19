description("Test path animation where coordinate modes of start and end differ. You should see PASS messages");
createSVGTestCase();

// Setup test document
var path = createSVGElement("path");
path.setAttribute("id", "path");
path.setAttribute("d", "M 30 30 l -60 -30 v -30 h 30 Z");
path.setAttribute("fill", "green");
path.setAttribute("onclick", "executeTest()");
path.setAttribute("transform", "translate(50, 50)");

var animate = createSVGElement("animate");
animate.setAttribute("id", "animation");
animate.setAttribute("attributeName", "d");
animate.setAttribute("from", "M 30 30 l -60 -30 v -30 h 30 Z");
animate.setAttribute("to", "M -30 -30 L 30 0 V 30 H 0 Z");
animate.setAttribute("begin", "click");
animate.setAttribute("dur", "4s");
path.appendChild(animate);
rootSVGElement.appendChild(path);

// Setup animation test
function sample1() {
    // Check initial/end conditions
    shouldBeEqualToString("path.getAttribute('d')", "M 30 30 l -60 -30 v -30 h 30 Z");
}

function sample2() {
    shouldBeEqualToString("path.getAttribute('d')", "M 15 15 l -30 -15 v -15 h 15 Z");
}

function sample3() {
    shouldBeEqualToString("path.getAttribute('d')", "M -15 -15 L 15 0 V 15 H 0 Z");
}

function sample4() {
    shouldBeEqualToString("path.getAttribute('d')", "M -29.985 -29.985 L 29.985 0 V 29.985 H 0 Z");
}

function executeTest() {
    const expectedValues = [
        // [animationId, time, sampleCallback]
        ["animation", 0.0,   sample1],
        ["animation", 1.0,   sample2],
        ["animation", 3.0,   sample3],
        ["animation", 3.999, sample4],
        ["animation", 4.001, sample1]
    ];

    runAnimationTest(expectedValues);
}

var successfullyParsed = true;
