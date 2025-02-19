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
      return "http://www.w3.org/2001/DOM-Test-Suite/level3/core/documentadoptnode04";
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
    Invoke adoptNode on a new document to adopt a new namespace aware attribute node created by
    this document.  Check if this attribute has been adopted successfully by verifying the nodeName,
    namespaceURI, prefix, specified and ownerElement attributes of the adopted node.

* @author IBM
* @author Neil Delima
* @see http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core#Document3-adoptNode
*/
function documentadoptnode04() {
   var success;
    if(checkInitialization(builder, "documentadoptnode04") != null) return;
    var doc;
      var newDoc;
      var domImpl;
      var newAttr;
      var adoptedAttr;
      var nodeName;
      var nodeNamespaceURI;
      var nodePrefix;
      var attrOwnerElem;
      var isSpecified;
      var nullDocType = null;

      var docElem;
      var rootNS;
      var rootName;
      var xmlNS = "http://www.w3.org/XML/1998/namespace";

      var docRef = null;
      if (typeof(this.doc) != 'undefined') {
        docRef = this.doc;
      }
      doc = load(docRef, "doc", "hc_staff");
      docElem = doc.documentElement;

      rootName = docElem.tagName;

      rootNS = docElem.namespaceURI;

      domImpl = doc.implementation;
newDoc = domImpl.createDocument(rootNS,rootName,nullDocType);
      newAttr = doc.createAttributeNS(xmlNS,"xml:lang");
      adoptedAttr = newDoc.adoptNode(newAttr);

    if(

    (adoptedAttr != null)

    ) {
    nodeName = adoptedAttr.nodeName;

      nodeNamespaceURI = adoptedAttr.namespaceURI;

      nodePrefix = adoptedAttr.prefix;

      attrOwnerElem = adoptedAttr.ownerElement;

      isSpecified = adoptedAttr.specified;

      assertEquals("documentadoptnode04_nodeName","xml:lang",nodeName);
       assertEquals("documentadoptnode04_namespaceURI",xmlNS,nodeNamespaceURI);
       assertEquals("documentadoptnode04_prefix","xml",nodePrefix);
       assertNull("documentadoptnode04_ownerDoc",attrOwnerElem);
    assertTrue("documentadoptnode04_specified",isSpecified);

    }

}

function runTest() {
   documentadoptnode04();
}
