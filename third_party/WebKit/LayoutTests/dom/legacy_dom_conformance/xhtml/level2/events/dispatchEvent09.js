/*
Copyright Â© 2001-2004 World Wide Web Consortium,
(Massachusetts Institute of Technology, European Research Consortium
for Informatics and Mathematics, Keio University). All
Rights Reserved. This work is distributed under the W3CÂ® Software License [1] in the
hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

[1] http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231
*/

   /**
    *  Gets URI that identifies the test.
    *  @return uri identifier of test
    */
function getTargetURI() {
      return "http://www.w3.org/2001/DOM-Test-Suite/level2/events/dispatchEvent09";
   }

var docsLoaded = -1000000;
var builder = null;

//
//   This function is called by the testing framework before
//      running the test suite.
//
//   If there are no configuration exceptions, asynchronous
//        document loading is started.  Otherwise, the status
//        is set to complete and the exception is immediately
//        raised when entering the body of the test.
//
function setUpPage() {
   setUpPageStatus = 'running';
   try {
     //
     //   creates test document builder, may throw exception
     //
     builder = createConfiguredBuilder();

      docsLoaded = 0;

      var docRef = null;
      if (typeof(this.doc) != 'undefined') {
        docRef = this.doc;
      }
      docsLoaded += preload(docRef, "doc", "hc_staff");

       if (docsLoaded == 1) {
          setUpPageStatus = 'complete';
       }
    } catch(ex) {
        catchInitializationError(builder, ex);
        setUpPageStatus = 'complete';
    }
}

//
//   This method is called on the completion of
//      each asychronous load started in setUpTests.
//
//   When every synchronous loaded document has completed,
//      the page status is changed which allows the
//      body of the test to be executed.
function loadComplete() {
    if (++docsLoaded == 1) {
        setUpPageStatus = 'complete';
    }
}

//EventMonitor's require a document level variable named monitor
var monitor;

/**
*
An event is dispatched to the document with a capture listener attached.
A capturing EventListener will not be triggered by events dispatched directly to the EventTarget upon which it is registered.

* @author Curt Arnold
* @see http://www.w3.org/TR/DOM-Level-2-Events/events#Events-EventTarget-dispatchEvent
* @see http://www.w3.org/TR/DOM-Level-2-Events/events#xpointer(id('Events-EventTarget-dispatchEvent')/raises/exception[@name='EventException']/descr/p[substring-before(.,':')='UNSPECIFIED_EVENT_TYPE_ERR'])
*/
function dispatchEvent09() {
   var success;
    if(checkInitialization(builder, "dispatchEvent09") != null) return;
    var doc;
      var target;
      var evt;
      var preventDefault;
      monitor = new EventMonitor();

      var atEvents = new Array();

      var bubbledEvents = new Array();

      var capturedEvents = new Array();

      var docRef = null;
      if (typeof(this.doc) != 'undefined') {
        docRef = this.doc;
      }
      doc = load(docRef, "doc", "hc_staff");
      doc.addEventListener("foo", monitor.handleEvent, true);
     evt = doc.createEvent("Events");
      evt.initEvent("foo",true,false);
      preventDefault = doc.dispatchEvent(evt);
      atEvents = monitor.atEvents;
assertSize("atCount",0,atEvents);
bubbledEvents = monitor.bubbledEvents;
assertSize("bubbleCount",0,bubbledEvents);
capturedEvents = monitor.capturedEvents;
assertSize("captureCount",0,capturedEvents);

}

function runTest() {
   dispatchEvent09();
}
