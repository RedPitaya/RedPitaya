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
	<!--global server stats-->
	<div class="roundbox">
		<h3>Global Server Stats</h3>
		<table class="yellowkeys">
			<tbody>
				<xsl:for-each select="/icestats">
					<xsl:for-each select="*">
						<xsl:if test = "name()!='source'"> 
							<tr>
							   <td><xsl:value-of select="name()" /></td>
							   <td><xsl:value-of select="." /></td>
							</tr>
						</xsl:if>
					</xsl:for-each>
				</xsl:for-each>
			</tbody>
		</table>
	</div>
	<!--end global server stats-->

	<!--mount point stats-->
	<xsl:for-each select="source">
		<xsl:if test = "listeners!=''"> 
			<div class="roundbox">
				<div class="mounthead">
					<h3 class="mount">Mountpoint <xsl:value-of select="@mount" /></h3>
					<div class="right">
						<xsl:choose>
							<xsl:when test="authenticator">
								<a class="auth" href="/auth.xsl">Login</a>
							</xsl:when>
							<xsl:otherwise>
								<ul class="mountlist">
									<li><a class="play" href="{@mount}.m3u">M3U</a></li>
									<li><a class="play" href="{@mount}.xspf">XSPF</a></li>
									<li><a class="play" href="{@mount}.vclt">VCLT</a></li>
								</ul>
							</xsl:otherwise>
						</xsl:choose>
					</div>
				</div>
				<div class="mountcont">
					<ul class="nav">
						<li><a href="listclients.xsl?mount={@mount}">List Clients</a></li>
						<li><a href="moveclients.xsl?mount={@mount}">Move Listeners</a></li>
						<li><a href="updatemetadata.xsl?mount={@mount}">Update Metadata</a></li>
						<xsl:if test="authenticator">
							<li><a href="manageauth.xsl?mount={@mount}">Manage Authentication</a></li>
						</xsl:if>
						<li><a href="killsource.xsl?mount={@mount}">Kill Source</a></li>
					</ul>
					<xsl:if test="server_type and ((server_type = 'application/ogg') or (server_type = 'audio/ogg'))">
						<div class="audioplayer">
							<audio controls="controls" preload="none">
								<source src="{@mount}" type="{server_type}" />
							</audio>
						</div>
					</xsl:if>
					<table class="yellowkeys">
						<tbody>
							<xsl:for-each select="*">
								<tr>
									<td><xsl:value-of select="name()" /></td>
									<td><xsl:value-of select="." /></td>
								</tr>
							</xsl:for-each>
						</tbody>
					</table>
				</div>
			</div>
		</xsl:if>
	</xsl:for-each>
	<!--end mount point stats-->
	<div id="footer">
		Support icecast development at <a href="http://www.icecast.org">www.icecast.org</a>
	</div>
</body>
</html>
</xsl:template>
</xsl:stylesheet>
