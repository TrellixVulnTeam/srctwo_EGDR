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
      return "http://www.w3.org/2001/DOM-Test-Suite/level3/core/nodeisdefaultnamespace15";
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

/**
*
    Using isDefaultNamespace on a Element's new cloned Comment node, which has a namespace attribute
    declaration without a namespace prefix in its parent Element node and  verify if the
    value returned is true.

* @author IBM
* @author Neil Delima
* @see http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core#Node3-isDefaultNamespace
*/
function nodeisdefaultnamespace15() {
   var success;
    if(checkInitialization(builder, "nodeisdefaultnamespace15") != null) return;
    var doc;
      var bodyElem;
      var elem;
      var comment;
      var clonedComment;
      var isDefault;
      var appendedChild;
      var bodyList;

      var docRef = null;
      if (typeof(this.doc) != 'undefined') {
        docRef = this.doc;
      }
      doc = load(docRef, "doc", "hc_staff");
      bodyList = doc.getElementsByTagName("body");
      bodyElem = bodyList.item(0);
      elem = doc.createElementNS("http://www.w3.org/1999/xhtml","p");
      comment = doc.createComment("Text");
      clonedComment = comment.cloneNode(true);
      appendedChild = elem.appendChild(clonedComment);
      appendedChild = bodyElem.appendChild(elem);
      isDefault = clonedComment.isDefaultNamespace("http://www.w3.org/1999/xhtml");
      assertTrue("nodeisdefaultnamespace15",isDefault);

}

function runTest() {
   nodeisdefaultnamespace15();
}
