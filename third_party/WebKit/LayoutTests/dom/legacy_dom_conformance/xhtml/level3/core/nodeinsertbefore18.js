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
      return "http://www.w3.org/2001/DOM-Test-Suite/level3/core/nodeinsertbefore18";
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
       setImplementationAttribute("expandEntityReferences", false);

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
    The method insertBefore inserts the node newChild before the existing child node refChild.
    If refChild is null, insert newChild at the end of the list of children.

    Using insertBefore on an Element node attempt to insert new Comment/PI and CDATA nodes
    before each other and verify the names of the newly inserted nodes.

* @author IBM
* @author Neil Delima
* @see http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core#ID-952280727
*/
function nodeinsertbefore18() {
   var success;
    if(checkInitialization(builder, "nodeinsertbefore18") != null) return;
    var doc;
      var element;
      var newElem;
      var newComment;
      var newPI;
      var newCDATA;
      var insertedNode;
      var data;
      var target;
      var appendedChild;
      var inserted;

      var docRef = null;
      if (typeof(this.doc) != 'undefined') {
        docRef = this.doc;
      }
      doc = load(docRef, "doc", "hc_staff");
      element = doc.createElement("element");
      newElem = doc.createElementNS("http://www.w3.org/DOM","dom3:elem");
      newComment = doc.createComment("Comment");
      newCDATA = doc.createCDATASection("CDATASection");
      newPI = doc.createProcessingInstruction("target","data");
      appendedChild = element.appendChild(newElem);
      appendedChild = element.appendChild(newComment);
      appendedChild = element.appendChild(newPI);
      appendedChild = element.appendChild(newCDATA);
      inserted = element.insertBefore(newComment,newElem);
      insertedNode = element.firstChild;

      data = insertedNode.data;

      assertEquals("nodeinsertbefore18","Comment",data);

}

function runTest() {
   nodeinsertbefore18();
}
