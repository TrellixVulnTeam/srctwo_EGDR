<?xml version='1.0' encoding='utf-8'?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html"/>
  <xsl:template match="//*">
    <script>
        if (window.testRunner)
            testRunner.dumpAsText();
    </script>
    <div>PASS</div>
  </xsl:template>
</xsl:stylesheet>
