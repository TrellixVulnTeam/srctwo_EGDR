// [Name] SVGFESpotTurbulenceElement-svgdom-stitchTiles-prop.js
// [Expected rendering result] An image with turbulence filter - and a series of PASS messages

description("Tests dynamic updates of the 'stitchTiles' property of the SVGFETurbulenceElment object")
createSVGTestCase();

var turbulence = createSVGElement("feTurbulence");
turbulence.setAttribute("baseFrequency", "0.05");
turbulence.setAttribute("numOctaves", "3");
turbulence.setAttribute("seed", "5");
turbulence.setAttribute("stitchTiles", "stitch");
turbulence.setAttribute("type", "turbulence");

var filterElement = createSVGElement("filter");
filterElement.setAttribute("id", "myFilter");
filterElement.setAttribute("filterUnits", "userSpaceOnUse");
filterElement.setAttribute("x", "0");
filterElement.setAttribute("y", "0");
filterElement.setAttribute("width", "200");
filterElement.setAttribute("height", "200");
filterElement.appendChild(turbulence);

var defsElement = createSVGElement("defs");
defsElement.appendChild(filterElement);

rootSVGElement.appendChild(defsElement);

var rectElement = createSVGElement("rect");
rectElement.setAttribute("width", "200");
rectElement.setAttribute("height", "200");
rectElement.setAttribute("filter", "url(#myFilter)");
rootSVGElement.appendChild(rectElement);

shouldBe("turbulence.stitchTiles.baseVal", "SVGFETurbulenceElement.SVG_STITCHTYPE_STITCH");

function repaintTest() {
    turbulence.stitchTiles.baseVal = SVGFETurbulenceElement.SVG_STITCHTYPE_NOSTITCH;
    shouldBe("turbulence.stitchTiles.baseVal", "SVGFETurbulenceElement.SVG_STITCHTYPE_NOSTITCH");
}

var successfullyParsed = true;
