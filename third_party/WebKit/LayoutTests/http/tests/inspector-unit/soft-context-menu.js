var menu = new UI.SoftContextMenu([{
  type: 'item',
  label: 'First',
  enabled: true
},
{
  type: 'subMenu',
  label: 'Second',
  enabled: true,
  subItems: [
    {type: 'subMenu', label: 'Child 1', enabled: true, subItems: [{type: 'item', label: 'Grandchild', id: 'Grandchild', enabled: true}]},
    {type: 'item', label: 'Child 2', enabled: true},
    {type: 'item', label: 'Child 3', enabled: true},
    {type: 'item', label: 'Child 4', enabled: true}
  ]
},
{
  type: 'separator',
},{
  type: 'item',
  label: 'Third',
  enabled: true
}], item => TestRunner.addResult('Item Selected: ' + item));

var initialFocusedElement = UI.inspectorView.element.createChild('div');
initialFocusedElement.textContent = 'Initial Focused Element';
initialFocusedElement.tabIndex = -1;
initialFocusedElement.focus();

dumpContextMenu();
menu.show(document, new AnchorBox(50, 50, 0, 0));
dumpContextMenu();
pressKey('ArrowDown');
pressKey('ArrowDown');
pressKey('ArrowDown');
pressKey('ArrowUp');
pressKey('ArrowUp');
pressKey('ArrowUp');
pressKey('ArrowDown');
TestRunner.addResult('Enter Submenu');
pressKey('ArrowRight');
pressKey('ArrowDown');
pressKey('ArrowDown');
pressKey('ArrowDown');
TestRunner.addResult('Leave Submenu ArrowLeft');
pressKey('ArrowLeft');
pressKey('ArrowRight');
TestRunner.addResult('Leave Submenu Escape');
pressKey('Escape');
TestRunner.addResult('Enter Sub-Submenu');
pressKey(' ');
pressKey('Enter');
pressKey('Enter');
TestRunner.completeTest();

function pressKey(key) {
  var element = document.deepActiveElement();
  if (!element)
    return;
  element.dispatchEvent(TestRunner.createKeyEvent(key));
  if (key === ' ')
    key = 'Space';
  TestRunner.addResult(key);
  dumpContextMenu();
}

function dumpContextMenu() {
  if (initialFocusedElement.hasFocus()) {
    TestRunner.addResult('Initial focused element has focus');
    return;
  }
  var selection = '';
  var subMenu = menu;
  var activeElement = document.deepActiveElement();
  do {
    if (selection)
      selection += ' -> ';
    if (subMenu._contextMenuElement === activeElement)
      selection += '[';
    if (subMenu._highlightedMenuItemElement)
      selection += subMenu._highlightedMenuItemElement.textContent.replace(/[^A-z0-9 ]/g, '');
    else
      selection += 'null'
    if (subMenu._contextMenuElement === activeElement)
      selection += ']';
  }
  while (subMenu = subMenu._subMenu)
  TestRunner.addResult(selection);
}