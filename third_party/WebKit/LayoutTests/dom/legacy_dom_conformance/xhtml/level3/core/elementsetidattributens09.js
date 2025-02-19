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
      return "http://www.w3.org/2001/DOM-Test-Suite/level3/core/elementsetidattributens09";
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
     The method setIdAttributeNS declares the attribute specified by local name and namespace URI to be of type ID.
     If the value of the specified attribute is unique then this element node can later be retrieved using getElementById on Document.
     Note, however, that this simply affects this node and does not change any grammar that may be in use.

     Invoke setIdAttributeNS on the xmlns attribute of ent4.  Verify that NO_MODIFICATION_ALLOWED_ERR is raised.

* @author IBM
* @author Jenny Hsu
* @see http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core#ID-ElSetIdAttrNS
*/
function elementsetidattributens09() {
   var success;
    if(checkInitialization(builder, "elementsetidattributens09") != null) return;
    var doc;
      var elemList;
      var varElem;
      var entRef;
      var entElement;

      var docRef = null;
      if (typeof(this.doc) != 'undefined') {
        docRef = this.doc;
      }
      doc = load(docRef, "doc", "hc_staff");
      elemList = doc.getElementsByTagNameNS("*","var");
      varElem = elemList.item(2);
      entRef = varElem.firstChild;

      entElement = entRef.firstChild;

    {
        success = false;
        try {
            entElement.setIdAttributeNS("http://www.w3.org/2000/xmlns/","xmlns",true);
        }
        catch(ex) {
      success = (typeof(ex.code) != 'undefined' && ex.code == 7);
        }
        assertTrue("throw_NO_MODIFICATION_ALLOWED_ERR",success);
    }

}

function runTest() {
   elementsetidattributens09();
}
