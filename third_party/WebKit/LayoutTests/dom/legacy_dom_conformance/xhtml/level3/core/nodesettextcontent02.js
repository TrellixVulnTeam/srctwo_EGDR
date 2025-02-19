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
      return "http://www.w3.org/2001/DOM-Test-Suite/level3/core/nodesettextcontent02";
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
       setImplementationAttribute("namespaceAware", true);

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
    The method setTextContent has no effect when the node is defined to be null.

    Using setTextContent on a new Document node, attempt to set the textContent of this
    new Document node to textContent.  Check if it was not set by checking the nodeName
    attribute of a new Element of this Document node.

* @author IBM
* @author Neil Delima
* @see http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core#Node3-textContent
*/
function nodesettextcontent02() {
   var success;
    if(checkInitialization(builder, "nodesettextcontent02") != null) return;
    var doc;
      var domImpl;
      var newDoc;
      var nodeName;
      var elemChild;
      var newElem;
      var elemList;
      var nullDocType = null;

      var appendedChild;
      var documentElem;

      var docRef = null;
      if (typeof(this.doc) != 'undefined') {
        docRef = this.doc;
      }
      doc = load(docRef, "doc", "hc_staff");
      domImpl = doc.implementation;
newDoc = domImpl.createDocument("http://www.w3.org/DOM/Test","dom3:elem",nullDocType);
      newElem = newDoc.createElementNS("http://www.w3.org/DOM/Test","dom3:childElem");
      documentElem = newDoc.documentElement;

      appendedChild = documentElem.appendChild(newElem);
      newDoc.textContent = "textContent";

      elemList = newDoc.getElementsByTagNameNS("*","childElem");
      elemChild = elemList.item(0);
      nodeName = elemChild.nodeName;

      assertEquals("nodesettextcontent02","dom3:childElem",nodeName);

}

function runTest() {
   nodesettextcontent02();
}
