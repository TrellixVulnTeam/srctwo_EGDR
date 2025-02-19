// [Name] SVGFESpecularLightingElement-inhertit-lighting-color-css-prop.js
// [Expected rendering result] A shining circle - and a series of PASS messages

description("Tests dynamic updates of the inherited 'lighting-color' attribute of the SVGFESpecularLightingElement object")
createSVGTestCase();

var distantElement = createSVGElement("feDistantLight");
distantElement.setAttribute("azimuth", "45");
distantElement.setAttribute("elevation", "45");

var specularElement = createSVGElement("feSpecularLighting");
specularElement.setAttribute("in", "SourceGraphic");
specularElement.setAttribute("specularConstant", "1");
specularElement.setAttribute("specularExponent", "1");
specularElement.setAttribute("surfaceScale", "1");
specularElement.setAttribute("lighting-color", "inherit");
specularElement.appendChild(distantElement);

var filterElement = createSVGElement("filter");
filterElement.setAttribute("id", "myFilter");
filterElement.setAttribute("filterUnits", "objectBoundingBox");
filterElement.setAttribute("x", "0%");
filterElement.setAttribute("y", "0%");
filterElement.setAttribute("width", "100%");
filterElement.setAttribute("height", "100%");
filterElement.setAttribute("style", "lighting-color:red");
filterElement.appendChild(specularElement);

var defsElement = createSVGElement("defs");
defsElement.appendChild(filterElement);

rootSVGElement.appendChild(defsElement);
rootSVGElement.setAttribute("width", "200");
rootSVGElement.setAttribute("height", "200");

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

shouldBeEqualToString("document.defaultView.getComputedStyle(specularElement).getPropertyValue('lighting-color')", "rgb(255, 0, 0)");

function repaintTest() {
    filterElement.setAttribute("style", "lighting-color:rgb(173, 255, 47)");
//    specularElement.style.setProperty("lighting-color", "greenyellow", "");
    shouldBeEqualToString("document.defaultView.getComputedStyle(specularElement).getPropertyValue('lighting-color')", "rgb(173, 255, 47)");
}

var successfullyParsed = true;
