description("Test path animation where coordinate modes of start and end differ. You should see PASS messages");
createSVGTestCase();

// Setup test document
var path = createSVGElement("path");
path.setAttribute("id", "path");
path.setAttribute("d", 'm -70 30 a 160 170 60 1 1 60 40 m 120 70 a 180 190 120 1 1 100 150 Z m 120 -60');
path.setAttribute("fill", "green");
path.setAttribute("onclick", "executeTest()");
path.setAttribute("transform", "translate(50, 50)");

var animate = createSVGElement("animate");
animate.setAttribute("id", "animation");
animate.setAttribute("attributeName", "d");
animate.setAttribute("from", 'm -70 30 a 160 170 60 1 1 60 40 m 120 70 a 180 190 120 1 1 100 150 Z m 120 -60');
animate.setAttribute("to", 'M -80 40 A 150 160 30 1 1 0 100 M 40 60 A 170 180 90 1 1 300 200 Z M 300 100');
animate.setAttribute("begin", "click");
animate.setAttribute("dur", "4s");
path.appendChild(animate);
rootSVGElement.appendChild(path);

// Setup animation test
function sample1() {
    // Check initial/end conditions
    shouldBeEqualToString("path.getAttribute('d')", "m -70 30 a 160 170 60 1 1 60 40 m 120 70 a 180 190 120 1 1 100 150 Z m 120 -60");
}

function sample2() {
    shouldBeEqualToString("path.getAttribute('d')", "m -72.5 32.5 a 157.5 167.5 52.5 1 1 65 45 m 100 42.5 a 177.5 187.5 112.5 1 1 140 147.5 Z m 155 -35");
}

function sample3() {
    shouldBeEqualToString("path.getAttribute('d')", "M -77.5 37.5 A 152.5 162.5 37.5 1 1 -2.5 92.5 M 57.5 80 A 172.5 182.5 97.5 1 1 277.5 222.5 Z M 282.5 95");
}

function sample4() {
    shouldBeEqualToString("path.getAttribute('d')", "M -79.9975 39.9975 A 150.003 160.003 30.0075 1 1 -0.00249481 99.9925 M 40.0175 60.02 A 170.003 180.003 90.0075 1 1 299.977 200.022 Z M 299.982 99.995");
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
