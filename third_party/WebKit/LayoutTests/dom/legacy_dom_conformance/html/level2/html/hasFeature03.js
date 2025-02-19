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
      return "http://www.w3.org/2001/DOM-Test-Suite/level2/html/hasFeature03";
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

       if (docsLoaded == 0) {
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
    if (++docsLoaded == 0) {
        setUpPageStatus = 'complete';
    }
}

/**
*
hasFeature("xhTmL", null) should return true if hasFeature("XML", null) returns true.

* @author Curt Arnold
* @see http://www.w3.org/TR/DOM-Level-2-Core/core#ID-5CED94D7
*/
function hasFeature03() {
   var success;
    if(checkInitialization(builder, "hasFeature03") != null) return;
    var doc;
      var domImpl;
      var version = null;

      var state;
      var hasXML;
      domImpl = getImplementation();
hasXML = domImpl.hasFeature("XML",version);
state = domImpl.hasFeature("xhTmL",version);
assertEquals("hasXHTML",hasXML,state);

}

function runTest() {
   hasFeature03();
}
