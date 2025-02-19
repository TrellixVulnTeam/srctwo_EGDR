// [Name] SVGFEComponentTransferElement-svgdom-type-prop.js
// [Expected rendering result] An image with feComponentTransfer filter - and a series of PASS messages

description("Tests dynamic updates of the 'type' property of the SVGFEComponentTransferElement object")
createSVGTestCase();

var feRFunc = createSVGElement("feFuncR");
feRFunc.setAttribute("type", "gamma");
feRFunc.setAttribute("amplitude", "10");

var feGFunc = createSVGElement("feFuncG");
feGFunc.setAttribute("type", "gamma");
feGFunc.setAttribute("amplitude", "10");

var feBFunc = createSVGElement("feFuncB");
feBFunc.setAttribute("type", "gamma");
feBFunc.setAttribute("amplitude", "110");

var feAFunc = createSVGElement("feFuncA");
feAFunc.setAttribute("type", "gamma");
feAFunc.setAttribute("amplitude", "110");

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

shouldBe("feRFunc.type.baseVal", "SVGComponentTransferFunctionElement.SVG_FECOMPONENTTRANSFER_TYPE_GAMMA");
shouldBe("feGFunc.type.baseVal", "SVGComponentTransferFunctionElement.SVG_FECOMPONENTTRANSFER_TYPE_GAMMA");
shouldBe("feBFunc.type.baseVal", "SVGComponentTransferFunctionElement.SVG_FECOMPONENTTRANSFER_TYPE_GAMMA");
shouldBe("feAFunc.type.baseVal", "SVGComponentTransferFunctionElement.SVG_FECOMPONENTTRANSFER_TYPE_GAMMA");

function repaintTest() {
    feRFunc.type.baseVal = SVGComponentTransferFunctionElement.SVG_FECOMPONENTTRANSFER_TYPE_LINEAR;
    feGFunc.type.baseVal = SVGComponentTransferFunctionElement.SVG_FECOMPONENTTRANSFER_TYPE_LINEAR;
    feBFunc.type.baseVal = SVGComponentTransferFunctionElement.SVG_FECOMPONENTTRANSFER_TYPE_LINEAR;
    feAFunc.type.baseVal = SVGComponentTransferFunctionElement.SVG_FECOMPONENTTRANSFER_TYPE_LINEAR;

    shouldBe("feRFunc.type.baseVal", "SVGComponentTransferFunctionElement.SVG_FECOMPONENTTRANSFER_TYPE_LINEAR");
    shouldBe("feGFunc.type.baseVal", "SVGComponentTransferFunctionElement.SVG_FECOMPONENTTRANSFER_TYPE_LINEAR");
    shouldBe("feBFunc.type.baseVal", "SVGComponentTransferFunctionElement.SVG_FECOMPONENTTRANSFER_TYPE_LINEAR");
    shouldBe("feAFunc.type.baseVal", "SVGComponentTransferFunctionElement.SVG_FECOMPONENTTRANSFER_TYPE_LINEAR");
}

var successfullyParsed = true;
