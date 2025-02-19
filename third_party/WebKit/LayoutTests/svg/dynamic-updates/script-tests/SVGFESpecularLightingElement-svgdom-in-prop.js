// [Name] SVGFESpecularLightingElement-svgdom-in-prop.js
// [Expected rendering result] A shining circle - and a series of PASS messages

description("Tests dynamic updates of the 'in' property of the SVGFESpecularLightingElement object")
createSVGTestCase();

var distantElement = createSVGElement("feDistantLight");
distantElement.setAttribute("azimuth", "45");
distantElement.setAttribute("elevation", "45");

var specularElement = createSVGElement("feSpecularLighting");
specularElement.setAttribute("in", "SourceAlpha");
specularElement.setAttribute("specularConstant", "1");
specularElement.setAttribute("specularExponent", "1");
specularElement.setAttribute("surfaceScale", "10");
specularElement.setAttribute("lighting-color", "greenyellow");
specularElement.appendChild(distantElement);

var filterElement = createSVGElement("filter");
filterElement.setAttribute("id", "myFilter");
filterElement.setAttribute("filterUnits", "objectBoundingBox");
filterElement.setAttribute("x", "0%");
filterElement.setAttribute("y", "0%");
filterElement.setAttribute("width", "100%");
filterElement.setAttribute("height", "100%");
filterElement.appendChild(specularElement);

var defsElement = createSVGElement("defs");
defsElement.appendChild(filterElement);

rootSVGElement.appendChild(defsElement);
rootSVGElement.setAttribute("width", "200");
rootSVGElement.setAttribute("height", "150");

var gElement = createSVGElement("g");
gElement.setAttribute("fill", "black");

var backgroundElement = createSVGElement("rect");
backgroundElement.setAttribute("x", 60);
backgroundElement.setAttribute("y", 20);
backgroundElement.setAttribute("width", 100);
backgroundElement.setAttribute("height", 60);
gElement.appendChild(backgroundElement);
rootSVGElement.appendChild(gElement);

var rectElement = createSVGElement("image");
rectElement.setAttribute("x", 60);
rectElement.setAttribute("y", 20);
rectElement.setAttribute("width", 100);
rectElement.setAttribute("height", 60);
rectElement.setAttributeNS(xlinkNS, "xlink:href", "../W3C-SVG-1.1/resources/bumpMap2.png");
rectElement.setAttribute("filter", "url(#myFilter)");
rootSVGElement.appendChild(rectElement);

shouldBeEqualToString("specularElement.in1.baseVal", "SourceAlpha");

function repaintTest() {
    specularElement.in1.baseVal = "SourceGraphic";
    shouldBeEqualToString("specularElement.in1.baseVal", "SourceGraphic");
}


var successfullyParsed = true;
