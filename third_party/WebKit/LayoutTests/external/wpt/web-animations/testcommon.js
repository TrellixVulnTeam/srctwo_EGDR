/*
Distributed under both the W3C Test Suite License [1] and the W3C
3-clause BSD License [2]. To contribute to a W3C Test Suite, see the
policies and contribution forms [3].

[1] http://www.w3.org/Consortium/Legal/2008/04-testsuite-license
[2] http://www.w3.org/Consortium/Legal/2008/03-bsd-license
[3] http://www.w3.org/2004/10/27-testcases
 */

'use strict';

var MS_PER_SEC = 1000;

// The recommended minimum precision to use for time values[1].
//
// [1] https://w3c.github.io/web-animations/#precision-of-time-values
var TIME_PRECISION = 0.0005; // ms

// Allow implementations to substitute an alternative method for comparing
// times based on their precision requirements.
if (!window.assert_times_equal) {
  window.assert_times_equal = function(actual, expected, description) {
    assert_approx_equals(actual, expected, TIME_PRECISION, description);
  }
}

// creates div element, appends it to the document body and
// removes the created element during test cleanup
function createDiv(test, doc) {
  return createElement(test, 'div', doc);
}

// creates element of given tagName, appends it to the document body and
// removes the created element during test cleanup
// if tagName is null or undefined, returns div element
function createElement(test, tagName, doc) {
  if (!doc) {
    doc = document;
  }
  var element = doc.createElement(tagName || 'div');
  doc.body.appendChild(element);
  test.add_cleanup(function() {
    element.remove();
  });
  return element;
}

// Creates a style element with the specified rules, appends it to the document
// head and removes the created element during test cleanup.
// |rules| is an object. For example:
// { '@keyframes anim': '' ,
//   '.className': 'animation: anim 100s;' };
// or
// { '.className1::before': 'content: ""; width: 0px; transition: all 10s;',
//   '.className2::before': 'width: 100px;' };
// The object property name could be a keyframes name, or a selector.
// The object property value is declarations which are property:value pairs
// split by a space.
function createStyle(test, rules, doc) {
  if (!doc) {
    doc = document;
  }
  var extraStyle = doc.createElement('style');
  doc.head.appendChild(extraStyle);
  if (rules) {
    var sheet = extraStyle.sheet;
    for (var selector in rules) {
      sheet.insertRule(selector + '{' + rules[selector] + '}',
                       sheet.cssRules.length);
    }
  }
  test.add_cleanup(function() {
    extraStyle.remove();
  });
}

// Create a pseudo element
function createPseudo(test, type) {
  createStyle(test, { '@keyframes anim': '',
                      ['.pseudo::' + type]: 'animation: anim 10s; ' +
                                            'content: \'\';'  });
  var div = createDiv(test);
  div.classList.add('pseudo');
  var anims = document.getAnimations();
  assert_true(anims.length >= 1);
  var anim = anims[anims.length - 1];
  assert_equals(anim.effect.target.parentElement, div);
  assert_equals(anim.effect.target.type, '::' + type);
  anim.cancel();
  return anim.effect.target;
}

// Cubic bezier with control points (0, 0), (x1, y1), (x2, y2), and (1, 1).
function cubicBezier(x1, y1, x2, y2) {
  function xForT(t) {
    var omt = 1-t;
    return 3 * omt * omt * t * x1 + 3 * omt * t * t * x2 + t * t * t;
  }

  function yForT(t) {
    var omt = 1-t;
    return 3 * omt * omt * t * y1 + 3 * omt * t * t * y2 + t * t * t;
  }

  function tForX(x) {
    // Binary subdivision.
    var mint = 0, maxt = 1;
    for (var i = 0; i < 30; ++i) {
      var guesst = (mint + maxt) / 2;
      var guessx = xForT(guesst);
      if (x < guessx) {
        maxt = guesst;
      } else {
        mint = guesst;
      }
    }
    return (mint + maxt) / 2;
  }

  return function bezierClosure(x) {
    if (x == 0) {
      return 0;
    }
    if (x == 1) {
      return 1;
    }
    return yForT(tForX(x));
  }
}

function stepEnd(nsteps) {
  return function stepEndClosure(x) {
    return Math.floor(x * nsteps) / nsteps;
  }
}

function stepStart(nsteps) {
  return function stepStartClosure(x) {
    var result = Math.floor(x * nsteps + 1.0) / nsteps;
    return (result > 1.0) ? 1.0 : result;
  }
}

function framesTiming(nframes) {
  return function framesClosure(x) {
    var result = Math.floor(x * nframes) / (nframes - 1);
    return (result > 1.0 && x <= 1.0) ? 1.0 : result;
  }
}

function waitForAnimationFrames(frameCount) {
  return new Promise(function(resolve, reject) {
    function handleFrame() {
      if (--frameCount <= 0) {
        resolve();
      } else {
        window.requestAnimationFrame(handleFrame); // wait another frame
      }
    }
    window.requestAnimationFrame(handleFrame);
  });
}

// Continually calls requestAnimationFrame until |minDelay| has elapsed
// as recorded using document.timeline.currentTime (i.e. frame time not
// wall-clock time).
function waitForAnimationFramesWithDelay(minDelay) {
  var startTime = document.timeline.currentTime;
  return new Promise(function(resolve) {
    (function handleFrame() {
      if (document.timeline.currentTime - startTime >= minDelay) {
        resolve();
      } else {
        window.requestAnimationFrame(handleFrame);
      }
    }());
  });
}

// Returns 'matrix()' or 'matrix3d()' function string generated from an array.
function createMatrixFromArray(array) {
  return (array.length == 16 ? 'matrix3d' : 'matrix') +
         '(' + array.join() + ')';
}

// Returns 'matrix3d()' function string equivalent to
// 'rotate3d(x, y, z, radian)'.
function rotate3dToMatrix3d(x, y, z, radian) {
  return createMatrixFromArray(rotate3dToMatrix(x, y, z, radian));
}

// Returns an array of the 4x4 matrix equivalent to 'rotate3d(x, y, z, radian)'.
// https://www.w3.org/TR/css-transforms-1/#Rotate3dDefined
function rotate3dToMatrix(x, y, z, radian) {
  var sc = Math.sin(radian / 2) * Math.cos(radian / 2);
  var sq = Math.sin(radian / 2) * Math.sin(radian / 2);

  // Normalize the vector.
  var length = Math.sqrt(x*x + y*y + z*z);
  x /= length;
  y /= length;
  z /= length;

  return [
    1 - 2 * (y*y + z*z) * sq,
    2 * (x * y * sq + z * sc),
    2 * (x * z * sq - y * sc),
    0,
    2 * (x * y * sq - z * sc),
    1 - 2 * (x*x + z*z) * sq,
    2 * (y * z * sq + x * sc),
    0,
    2 * (x * z * sq + y * sc),
    2 * (y * z * sq - x * sc),
    1 - 2 * (x*x + y*y) * sq,
    0,
    0,
    0,
    0,
    1
  ];
}

// Compare matrix string like 'matrix(1, 0, 0, 1, 100, 0)' with tolerances.
function assert_matrix_equals(actual, expected, description) {
  var matrixRegExp = /^matrix(?:3d)*\((.+)\)/;
  assert_regexp_match(actual, matrixRegExp,
    'Actual value is not a matrix')
  assert_regexp_match(expected, matrixRegExp,
    'Expected value is not a matrix');

  var actualMatrixArray =
    actual.match(matrixRegExp)[1].split(',').map(Number);
  var expectedMatrixArray =
    expected.match(matrixRegExp)[1].split(',').map(Number);

  assert_equals(actualMatrixArray.length, expectedMatrixArray.length,
    'dimension of the matrix: ' + description);
  for (var i = 0; i < actualMatrixArray.length; i++) {
    assert_approx_equals(actualMatrixArray[i], expectedMatrixArray[i], 0.0001,
      'expected ' + expected + ' but got ' + actual + ": " + description);
  }
}
