var svgNS = "http://www.w3.org/2000/svg";

var svgRoot = document.createElementNS(svgNS, "svg");
document.documentElement.appendChild(svgRoot);

var svgText = document.createElementNS(svgNS, "text");
svgText.style.fontFamily = "Ahem";
svgText.style.fontSize = "20px";
svgText.appendChild(document.createTextNode("abc"));
svgRoot.appendChild(svgText);

shouldThrow("svgText.selectSubString(-1, 0)", '"IndexSizeError: Failed to execute \'selectSubString\' on \'SVGTextContentElement\': The charnum provided (4294967295) is greater than the maximum bound (3)."');
shouldNotThrow("svgText.getSubStringLength(0, -1)");
shouldThrow("svgText.getSubStringLength(3, 0)", '"IndexSizeError: Failed to execute \'getSubStringLength\' on \'SVGTextContentElement\': The charnum provided (3) is greater than or equal to the maximum bound (3)."');

// cleanup
document.documentElement.removeChild(svgRoot);

var successfullyParsed = true;
