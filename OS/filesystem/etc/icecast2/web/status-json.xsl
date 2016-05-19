<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:import href="xml2json.xslt"/>
<xsl:output indent="no" omit-xml-declaration="yes" method="text" encoding="UTF-8" media-type="application/json"/>
<xsl:strip-space elements="*"/>

<!-- override imported transform variable to enable output -->
<xsl:variable name="output">true</xsl:variable>

<!-- hide certain nodes from all sources -->
<xsl:template match="icestats/source/max_listeners" />
<xsl:template match="icestats/source/public" />
<xsl:template match="icestats/source/source_ip" />
<xsl:template match="icestats/source/slow_listeners" />
<xsl:template match="icestats/source/*[contains(name(), 'total_bytes')]" />
<xsl:template match="icestats/source/user_agent" >
	<!-- user_agent is most of the time the last node in a mount, 
	     if we just delete it, then we will malform the output, 
	     so special handling applies. -->
	<xsl:if test="following-sibling::*"></xsl:if>
	<xsl:if test="not(following-sibling::*)">"dummy":null}</xsl:if> 
</xsl:template>

<!-- hide certain global nodes -->
<xsl:template match="icestats/sources" />
<xsl:template match="icestats/clients" />
<xsl:template match="icestats/stats" />
<xsl:template match="icestats/listeners" />
<xsl:template match="node()[contains(name(), 'connections')]" />

</xsl:stylesheet>
