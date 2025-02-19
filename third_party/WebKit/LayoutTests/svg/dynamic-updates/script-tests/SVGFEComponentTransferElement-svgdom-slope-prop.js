// [Name] SVGFEComponentTransferElement-svgdom-slope-prop.js
// [Expected rendering result] An image with feComponentTransfer filter - and a series of PASS messages

description("Tests dynamic updates of the 'slope' property of the SVGFEComponentTransferElement object")
createSVGTestCase();

var feRFunc = createSVGElement("feFuncR");
feRFunc.setAttribute("type", "linear");
feRFunc.setAttribute("slope", "1");

var feGFunc = createSVGElement("feFuncG");
feGFunc.setAttribute("type", "linear");
feGFunc.setAttribute("slope", "1");

var feBFunc = createSVGElement("feFuncB");
feBFunc.setAttribute("type", "linear");
feBFunc.setAttribute("slope", "1");

var feAFunc = createSVGElement("feFuncA");
feAFunc.setAttribute("type", "linear");
feAFunc.setAttribute("slope", "1");

var feComponentTransferElement = createSVGElement("feComponentTransfer");
feComponentTransferElement.appendChild(feRFunc);
feComponentTransferElement.appendChild(feGFunc);
feComponentTransferElement.appendChild(feBFunc);
feComponentTransferElement.appendChild(feAFunc);

var compTranFilter = createSVGElement("filter");
compTranFilter.setAttribute("id", "compTranFilter");
compTranFilter.setAttribute("filterUnits", "objectBoundingBox");
compTranFilter.setAttribute("x", "0%");
compTranFilter.setAttribute("y", "0%");
compTranFilter.setAttribute("width", "100%");
compTranFilter.setAttribute("height", "100%");
compTranFilter.appendChild(feComponentTransferElement);

var defsElement = createSVGElement("defs");
defsElement.appendChild(compTranFilter);
rootSVGElement.appendChild(defsElement);

var imageElement = createSVGElement("image");
imageElement.setAttributeNS(xlinkNS, "xlink:href", "../W3C-SVG-1.1/resources/struct-image-01.png");
imageElement.setAttribute("width", "400");
imageElement.setAttribute("height", "200");
imageElement.setAttribute("preserveAspectRatio", "none");
imageElement.setAttribute("filter", "url(#compTranFilter)");
rootSVGElement.appendChild(imageElement);

rootSVGElement.setAttribute("width", "400");
rootSVGElement.setAttribute("height", "200");

shouldBe("feRFunc.slope.baseVal", "1");
shouldBe("feGFunc.slope.baseVal", "1");
shouldBe("feBFunc.slope.baseVal", "1");
shouldBe("feAFunc.slope.baseVal", "1");

function repaintTest() {
    feRFunc.slope.baseVal = 2;
    feGFunc.slope.baseVal = 2;
    feBFunc.slope.baseVal = 2;
    feAFunc.slope.baseVal = 2;

    shouldBe("feRFunc.slope.baseVal", "2");
    shouldBe("feGFunc.slope.baseVal", "2");
    shouldBe("feBFunc.slope.baseVal", "2");
    shouldBe("feAFunc.slope.baseVal", "2");
}

var successfullyParsed = true;
