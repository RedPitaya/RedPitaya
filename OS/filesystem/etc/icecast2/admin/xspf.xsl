<!--
    XSPF xslt stylesheet for Icecast and above
    Copyright (C) 2007 Thomas B. Ruecker, thomas@ruecker.fi

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
-->

<xsl:stylesheet xmlns:xsl = "http://www.w3.org/1999/XSL/Transform" version = "1.0" >
<xsl:output omit-xml-declaration="no" media-type="application/xspf+xml"
        method="xml" indent="yes" encoding="UTF-8" />
<xsl:template match = "/icestats" >
<playlist version="1" xmlns="http://xspf.org/ns/0/">
	<title><xsl:value-of select="server" /></title>
	<creator><xsl:value-of select="server" /></creator>
	<trackList >
<!-- end of "header" -->

<xsl:for-each select="source">

<track>
    <location><xsl:value-of select="listenurl" /></location>


<xsl:if test="artist"><creator><xsl:value-of select="artist" /></creator></xsl:if>
<title><xsl:value-of select="title" /></title>
<!-- The <xsl:text>\n</xsl:text> elements in the following part are used 
to enforce linebreaks this format seems to be expected by clients -->
<annotation>
<xsl:if test="server_name">Stream Title: <xsl:value-of select="server_name" /><xsl:text>
</xsl:text></xsl:if>
<xsl:if test="server_description">Stream Description: <xsl:value-of select="server_description" /></xsl:if>
Content Type:<xsl:value-of select="server_type" /><xsl:text>
</xsl:text>
<xsl:if test="bitrate">Bitrate: <xsl:value-of select="bitrate" /><xsl:text>
</xsl:text></xsl:if>
<xsl:if test="quality">Quality: <xsl:value-of select="quality" /><xsl:text>
</xsl:text></xsl:if>
<xsl:if test="video_quality">Video Quality: <xsl:value-of select="video_quality" /><xsl:text>
</xsl:text></xsl:if>
<xsl:if test="frame_size">Framesize: <xsl:value-of select="frame_size" /><xsl:text>
</xsl:text></xsl:if>
<xsl:if test="frame_rate">Framerate: <xsl:value-of select="frame_rate" /><xsl:text>
</xsl:text></xsl:if>
<xsl:if test="listeners">Current Listeners: <xsl:value-of select="listeners" /><xsl:text>
</xsl:text></xsl:if>
<xsl:if test="listener_peak">Peak Listeners: <xsl:value-of select="listener_peak" /><xsl:text>
</xsl:text></xsl:if>
<xsl:if test="genre">Stream Genre: <xsl:value-of select="genre" /></xsl:if>
</annotation>

<xsl:if test="server_url"><info><xsl:value-of select="server_url" /></info></xsl:if>

</track>

</xsl:for-each>
</trackList>
</playlist>

</xsl:template>
</xsl:stylesheet>
