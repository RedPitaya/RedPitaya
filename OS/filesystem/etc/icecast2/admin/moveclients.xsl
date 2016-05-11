<xsl:stylesheet xmlns:xsl = "http://www.w3.org/1999/XSL/Transform" version = "1.0" >
<xsl:output omit-xml-declaration="no" method="xml" doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd" indent="yes" encoding="UTF-8" />
<xsl:template match = "/icestats" >
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
	<title>Icecast Streaming Media Server</title>
	<link rel="stylesheet" type="text/css" href="/style.css" />
	<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=yes" />
</head>
<body>
	<h1>Icecast2 Admin</h1>
	<!--index header menu -->
	<div id="menu">
		<ul>
			<li><a href="stats.xsl">Admin Home</a></li>
			<li><a href="listmounts.xsl">Mountpoint List</a></li>
			<li><a href="/status.xsl">Public Home</a></li>
		</ul>
	</div>
	<!--end index header menu -->
	<xsl:variable name = "currentmount" ><xsl:value-of select="current_source" /></xsl:variable>
	<h2>Moving listeners from <xsl:value-of select="current_source" /></h2>
	<div class="roundbox">
		<h3>Move to which mountpoint?</h3>
		<xsl:for-each select="source">
			<p>
				Move from <code><xsl:copy-of select="$currentmount" /></code> to <code><xsl:value-of select="@mount" /></code><br />
				<xsl:value-of select="listeners" /> listeners<br />
				<a href="moveclients.xsl?mount={$currentmount}&amp;destination={@mount}">Move clients</a>
			</p>
		</xsl:for-each>
	</div>
	<div id="footer">
		Support icecast development at <a href="http://www.icecast.org">www.icecast.org</a>
	</div>
</body>
</html>
</xsl:template>
</xsl:stylesheet>