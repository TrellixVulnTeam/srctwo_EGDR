description('Tests that we find controls if they have a range limitation and are out-of-range.');

var parentDiv = document.createElement('div');
document.body.appendChild(parentDiv);
parentDiv.innerHTML = '<input id="number1" type="number" min=0 max=10><input id="text1" type="text" min=0 max=10 value=50><input id="checkbox1" type="checkbox"><input id="radio1" type="radio">';

debug('==> :out-of-range doesn\'t match to an INPUT without a value:');
shouldBeNull('document.querySelector("input[type=number]:out-of-range")');

debug('');
debug('==> :out-of-range should match to an INPUT with an out-of-range value:');
parentDiv.firstChild.value = '50';
shouldBe('document.querySelector("input[type=number]:out-of-range").id', '"number1"');
shouldBe('document.querySelectorAll(":out-of-range").length', '1');

debug('');
debug('==> When the value becomes in-range dynamically, we do not find the control anymore:');
document.getElementById("number1").value = 5;

shouldBe('document.querySelector("input[type=number]:out-of-range")', 'null');
shouldBe('document.querySelectorAll(":out-of-range").length', '0');
