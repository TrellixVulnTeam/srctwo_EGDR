// [Name] SVGFEDisplacementMapElement-svgdom-scale-prop.js
// [Expected rendering result] A rectangle with displacementMap filter - and a series of PASS messages

description("Tests dynamic updates of the 'scale' property of the SVGFEDisplacementMapElement object")
createSVGTestCase();

var defsElement = createSVGElement("defs");
rootSVGElement.appendChild(defsElement);

var image1 = createSVGElement("feImage");
image1.setAttribute("result", "Map");
image1.setAttributeNS(xlinkNS, "xlink:href", "../W3C-SVG-1.1/resources/sphere.png");

var image2 = createSVGElement("feImage");
image2.setAttribute("result", "Texture");
image2.setAttributeNS(xlinkNS, "xlink:href", "../W3C-SVG-1.1/resources/DisplaceChecker.png");

var displacementMap = createSVGElement("feDisplacementMap");
displacementMap.setAttribute("in", "Texture");
displacementMap.setAttribute("in2", "Map");
displacementMap.setAttribute("scale", "34");
displacementMap.setAttribute("xChannelSelector", "R");
displacementMap.setAttribute("yChannelSelector", "G");

var displacemenMapFilter = createSVGElement("filter");
displacemenMapFilter.setAttribute("id", "myFilter");
displacemenMapFilter.setAttribute("filterUnit", "objectBoundingBox");
displacemenMapFilter.setAttribute("x", "0");
displacemenMapFilter.setAttribute("y", "0");
displacemenMapFilter.setAttribute("width", "1");
displacemenMapFilter.setAttribute("height", "1");
displacemenMapFilter.appendChild(image1);
displacemenMapFilter.appendChild(image2);
displacemenMapFilter.appendChild(displacementMap);

defsElement.appendChild(displacemenMapFilter);

var myRect = createSVGElement("rect");
myRect.setAttribute("x", "15");
myRect.setAttribute("y", "15");
myRect.setAttribute("width", "128");
myRect.setAttribute("height", "128");
myRect.setAttribute("filter", "url(#myFilter)");

rootSVGElement.setAttribute("height", "200");
rootSVGElement.appendChild(myRect);

shouldBe("displacementMap.scale.baseVal", "34");

function repaintTest() {
    displacementMap.scale.baseVal = 64;
    shouldBe("displacementMap.scale.baseVal", "64");
}

var successfullyParsed = true;
