<xsl:stylesheet xmlns:xsl = "http://www.w3.org/1999/XSL/Transform" version = "1.0">
<xsl:output omit-xml-declaration="no" method="xml" doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd" indent="yes" encoding="UTF-8" />
<xsl:template match = "/icestats">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
	<title>Icecast Streaming Media Server</title>
	<link rel="stylesheet" type="text/css" href="style.css" />
	<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=yes" />
</head>
<body>
	<h1 id="header">Icecast2 Status</h1>
	<!--index header menu -->
	<div id="menu">
		<ul>
			<li><a href="admin/">Administration</a></li>
			<li><a href="status.xsl">Server Status</a></li>
			<li><a href="server_version.xsl">Version</a></li>
		</ul>
	</div>
	<!--end index header menu -->
	<xsl:text disable-output-escaping="yes">
	&lt;!-- WARNING:
	 DO NOT ATTEMPT TO PARSE ICECAST HTML OUTPUT!
	 The web interface may change completely between releases.
	 If you have a need for automatic processing of server data,
	 please read the appropriate documentation. Latest docs:
	 http://icecast.org/docs/icecast-latest/icecast2_stats.html
	-->
	</xsl:text>
	<!--mount point stats-->
	<xsl:for-each select="source">
		<xsl:choose>
			<xsl:when test="listeners">
			<div class="roundbox">
				<div class="mounthead">
					<h3 class="mount">Mount Point <xsl:value-of select="@mount" /></h3>
					<div class="right">
						<xsl:choose>
							<xsl:when test="authenticator">
								<a class="auth" href="/auth.xsl">Login</a>
							</xsl:when>
							<xsl:otherwise>
								<ul class="mountlist">
									<li><a class="play" href="{@mount}.m3u">M3U</a></li>
									<li><a class="play" href="{@mount}.xspf">XSPF</a></li>
									<!-- <li><a class="play" href="{@mount}.vclt">VCLT</a></li> -->
								</ul>
							</xsl:otherwise>
						</xsl:choose>
					</div>
				</div>
				<div class="mountcont">
					<xsl:if test="server_type and ((server_type = 'application/ogg') or (server_type = 'audio/ogg'))">
						<div class="audioplayer">
							<audio controls="controls" preload="none">
								<source src="{@mount}" type="{server_type}" />
							</audio>
						</div>
					</xsl:if>
					<table class="yellowkeys">
						<tbody>
							<xsl:if test="server_name">
								<tr>
									<td>Stream Name:</td>
									<td><xsl:value-of select="server_name" /></td>
								</tr>
							</xsl:if>
							<xsl:if test="server_description">
								<tr>
									<td>Stream Description:</td>
									<td><xsl:value-of select="server_description" /></td>
								</tr>
							</xsl:if>
							<xsl:if test="server_type">
								<tr>
									<td>Content Type:</td>
									<td><xsl:value-of select="server_type" /></td>
								</tr>
							</xsl:if>
							<xsl:if test="stream_start">
								<tr>
									<td>Stream started:</td>
									<td class="streamstats"><xsl:value-of select="stream_start" /></td>
								</tr>
							</xsl:if>
							<xsl:if test="bitrate">
								<tr>
									<td>Bitrate:</td>
									<td class="streamstats"><xsl:value-of select="bitrate" /></td>
								</tr>
							</xsl:if>
							<xsl:if test="quality">
								<tr>
									<td>Quality:</td>
									<td class="streamstats"><xsl:value-of select="quality" /></td>
								</tr>
							</xsl:if>
							<xsl:if test="video_quality">
								<tr>
									<td>Video Quality:</td>
									<td class="streamstats"><xsl:value-of select="video_quality" /></td>
								</tr>
							</xsl:if>
							<xsl:if test="frame_size">
								<tr>
									<td>Framesize:</td>
									<td class="streamstats"><xsl:value-of select="frame_size" /></td>
								</tr>
							</xsl:if>
							<xsl:if test="frame_rate">
								<tr>
									<td>Framerate:</td>
									<td class="streamstats"><xsl:value-of select="frame_rate" /></td>
								</tr>
							</xsl:if>
							<xsl:if test="listeners">
								<tr>
									<td>Listeners (current):</td>
									<td class="streamstats"><xsl:value-of select="listeners" /></td>
								</tr>
							</xsl:if>
							<xsl:if test="listener_peak">
								<tr>
									<td>Listeners (peak):</td>
									<td class="streamstats"><xsl:value-of select="listener_peak" /></td>
								</tr>
							</xsl:if>
							<xsl:if test="genre">
								<tr>
									<td>Genre:</td>
									<td class="streamstats"><xsl:value-of select="genre" /></td>
								</tr>
							</xsl:if>
							<xsl:if test="server_url">
								<tr>
									<td>Stream URL:</td>
									<td class="streamstats">
										<a href="{server_url}"><xsl:value-of select="server_url" /></a>
									</td>
								</tr>
							</xsl:if>
							<tr>
								<td>Currently playing:</td>
								<td class="streamstats">
								<xsl:if test="artist">
									<xsl:value-of select="artist" /> - 
								</xsl:if>
									<xsl:value-of select="title" />
								</td>
							</tr>
						</tbody>
					</table>
				</div>
			</div>
			</xsl:when>
			<xsl:otherwise>
				<h3><xsl:value-of select="@mount" /> - Not Connected</h3>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:for-each>
	<div id="footer">
		Support icecast development at <a href="http://www.icecast.org">www.icecast.org</a>
	</div>
</body>
</html>
</xsl:template>
</xsl:stylesheet>
