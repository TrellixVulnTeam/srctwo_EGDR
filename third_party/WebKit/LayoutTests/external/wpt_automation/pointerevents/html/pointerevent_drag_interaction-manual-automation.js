importAutomationScript('/pointerevents/pointerevent_common_input.js');

function inject_input() {
  return new Promise(function(resolve, reject) {
      for (var i=0; i<3; i++) {
          var target0Rect = document.querySelector('#target0').getBoundingClientRect();
          var target1Rect = document.querySelector('#target1').getBoundingClientRect();

          eventSender.mouseMoveTo(target0Rect.left + target0Rect.width/2, target0Rect.top + target0Rect.height/2);
          eventSender.mouseDown();
          eventSender.mouseMoveTo(target0Rect.left + target0Rect.width/2, target0Rect.top + 2*target0Rect.height/3);
          eventSender.mouseMoveTo(target1Rect.left + target1Rect.width/2, target1Rect.top + target1Rect.height/2);
          eventSender.mouseUp();
      }
      resolve();
  });
}
