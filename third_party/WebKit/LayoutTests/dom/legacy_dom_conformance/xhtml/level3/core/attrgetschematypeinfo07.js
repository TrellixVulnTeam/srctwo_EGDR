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
      return "http://www.w3.org/2001/DOM-Test-Suite/level3/core/attrgetschematypeinfo07";
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
       setImplementationAttribute("schemaValidating", true);

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
    The getSchemaTypeInfo method retrieves the type information associated with this attribute.

    Load a valid document with an XML Schema.

    Invoke getSchemaTypeInfo method on an attribute having [type definition] property.  Expose {name} and {target namespace}
    properties of the [type definition] property.  Verity that the typeName and typeNamespace of the title attribute's
    schemaTypeInfo are correct. getSchemaTypeInfo on the 'id' attribute of the fourth 'acronym' element

* @author IBM
* @author Jenny Hsu
* @see http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core#Attr-schemaTypeInfo
*/
function attrgetschematypeinfo07() {
   var success;
    if(checkInitialization(builder, "attrgetschematypeinfo07") != null) return;
    var doc;
      var elemList;
      var acElem;
      var attr;
      var attrTypeInfo;
      var typeName;
      var typeNamespace;

      var docRef = null;
      if (typeof(this.doc) != 'undefined') {
        docRef = this.doc;
      }
      doc = load(docRef, "doc", "hc_staff");
      elemList = doc.getElementsByTagName("acronym");
      acElem = elemList.item(3);
      attr = acElem.getAttributeNode("id");
      attrTypeInfo = attr.schemaTypeInfo;

      assertNotNull("typeInfoNotNull",attrTypeInfo);
typeName = attrTypeInfo.typeName;

      typeNamespace = attrTypeInfo.typeNamespace;

      assertEquals("attrgetschematypeinfo07_typeName","ID",typeName);
       assertEquals("attrgetschematypeinfo07_typeNamespace","http://www.w3.org/2001/XMLSchema",typeNamespace);

}

function runTest() {
   attrgetschematypeinfo07();
}
