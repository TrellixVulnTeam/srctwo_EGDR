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
      return "http://www.w3.org/2001/DOM-Test-Suite/level3/core/nodereplacechild22";
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
    Using replaceChild on a new EntityReference node attempt to replace an EntityReference node with
    its Element parent, with itself and vice versa verify if a NO_MODIFICATION_ALLOWED_ERR is thrown.

* @author IBM
* @author Neil Delima
* @see http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/core#ID-785887307
*/
function nodereplacechild22() {
   var success;
    if(checkInitialization(builder, "nodereplacechild22") != null) return;
    var doc;
      var entRefMain;
      var entRef;
      var elem;
      var appendedChild;
      var replacedChild;

      var docRef = null;
      if (typeof(this.doc) != 'undefined') {
        docRef = this.doc;
      }
      doc = load(docRef, "doc", "hc_staff");
      elem = doc.createElementNS("http://www.w3.org/1999/xhtml","dom3:p");
      entRefMain = doc.createEntityReference("delta");
      entRef = doc.createEntityReference("beta");
      appendedChild = elem.appendChild(entRef);

    {
        success = false;
        try {
            replacedChild = entRefMain.replaceChild(elem,entRef);
        }
        catch(ex) {
      success = (typeof(ex.code) != 'undefined' && ex.code == 7);
        }
        assertTrue("throw_NO_MODIFICATION_ALLOWED_ERR_1",success);
    }

    {
        success = false;
        try {
            replacedChild = entRefMain.replaceChild(entRef,elem);
        }
        catch(ex) {
      success = (typeof(ex.code) != 'undefined' && ex.code == 7);
        }
        assertTrue("throw_NO_MODIFICATION_ALLOWED_ERR_2",success);
    }

    {
        success = false;
        try {
            replacedChild = entRefMain.replaceChild(entRefMain,entRef);
        }
        catch(ex) {
      success = (typeof(ex.code) != 'undefined' && ex.code == 7);
        }
        assertTrue("throw_NO_MODIFICATION_ALLOWED_ERR_3",success);
    }

}

function runTest() {
   nodereplacechild22();
}
