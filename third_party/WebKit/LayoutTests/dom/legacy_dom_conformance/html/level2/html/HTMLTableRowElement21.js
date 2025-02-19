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
      return "http://www.w3.org/2001/DOM-Test-Suite/level2/html/HTMLTableRowElement21";
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
      docsLoaded += preload(docRef, "doc", "tablerow");

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
    The deleteCell() method deletes a cell from the currtent row.  If
    the index is -1 the last cell in the row is deleted.

    Retrieve the fourth TR element and examine the value of
    the cells length attribute which should be set to six.
    Check the value of the last TD element.  Invoke the
    deleteCell() with an index of negative one
    which will delete the last cell in the row.
    Check the value of the of the last cell
    and also the numbers of cells should now be five.

* @author NIST
* @author Rick Rivello
* @see http://www.w3.org/TR/DOM-Level-2-HTML/html#ID-11738598
*/
function HTMLTableRowElement21() {
   var success;
    if(checkInitialization(builder, "HTMLTableRowElement21") != null) return;
    var nodeList;
      var cellsnodeList;
      var testNode;
      var trNode;
      var cellNode;
      var value;
      var vcells;
      var doc;

      var docRef = null;
      if (typeof(this.doc) != 'undefined') {
        docRef = this.doc;
      }
      doc = load(docRef, "doc", "tablerow");
      nodeList = doc.getElementsByTagName("tr");
      assertSize("Asize",5,nodeList);
testNode = nodeList.item(3);
      cellsnodeList = testNode.cells;

      vcells = cellsnodeList.length;

      assertEquals("cellsLink1",6,vcells);
       trNode = cellsnodeList.item(5);
      cellNode = trNode.firstChild;

      value = cellNode.nodeValue;

      assertEquals("value1Link","1230 North Ave. Dallas, Texas 98551",value);
       testNode.deleteCell(-1);
      testNode = nodeList.item(3);
      cellsnodeList = testNode.cells;

      vcells = cellsnodeList.length;

      assertEquals("cellsLink2",5,vcells);
       trNode = cellsnodeList.item(4);
      cellNode = trNode.firstChild;

      value = cellNode.nodeValue;

      assertEquals("value2Link","Female",value);

}

function runTest() {
   HTMLTableRowElement21();
}
