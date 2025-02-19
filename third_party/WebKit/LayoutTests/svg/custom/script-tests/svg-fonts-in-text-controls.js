description('This tests that the width of textareas and inputs is correctly calculated based on the metrics of the SVG font.');

var styleElement = document.createElement('style');
// FIXME: Is there a better way to create a font-face from JS?
styleElement.innerText = '@font-face { font-family: "SVGraffiti"; src: url("resources/graffiti.svg#SVGraffiti") format(svg) }';
document.getElementsByTagName('head')[0].appendChild(styleElement);

var textarea = document.createElement('textarea');
textarea.style.fontFamily = 'SVGraffiti';
textarea.style.fontSize = '11px';
textarea.style.padding = 0;
textarea.cols = 20;
document.body.appendChild(textarea);

var input = document.createElement('input');
input.style.fontFamily = 'SVGraffiti';
input.style.fontSize = '11px';
input.style.padding = 0;
input.size = 20;
document.body.appendChild(input);

// Force a layout to ensure SVGGraffiti gets loaded.
// Needs to happen before onLoad.
document.body.offsetWidth;

// Need to wait for the load event to make sure the font is loaded.
window.addEventListener('load', function()
{
    debug("Textarea offsetWidth: " + textarea.offsetWidth);
    debug("Input offsetWidth: " + input.offsetWidth);
}, true);

var successfullyParsed = true;
