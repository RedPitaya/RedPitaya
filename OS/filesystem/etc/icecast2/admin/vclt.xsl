<!--
    VCLT xslt stylesheet for Icecast 2.3.2 and above
    based on XSPF xslt stylesheet.
    Copyright (C) 2007 Thomas B. Ruecker, tbr@ruecker-itk.de
    Copyright (C) 2011 Philipp Schafft, lion@lion.leolix.org

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
<xsl:output omit-xml-declaration="yes" media-type="audio/x-vclt"
        method="text" indent="no" encoding="UTF-8" />
<xsl:template match = "/icestats" >
<xsl:for-each select="source">STREAMURL=<xsl:value-of select="listenurl" />
<xsl:if test="artist"><xsl:text>
</xsl:text>ARTIST=<xsl:value-of select="artist" /></xsl:if>
<xsl:if test="title"><xsl:text>
</xsl:text>TITLE=<xsl:value-of select="title" /></xsl:if>

<xsl:if test="server_name"><xsl:text>
</xsl:text>SERVER_NAME=<xsl:value-of select="server_name" /></xsl:if>
<xsl:if test="server_description"><xsl:text>
</xsl:text>DESCRIPTION=<xsl:value-of select="server_description" /></xsl:if>
SIGNALINFO=<xsl:choose>
 <xsl:when test="server_type = 'application/ogg'">
  <xsl:choose>
   <xsl:when test="subtype = 'Vorbis'">codec:ogg_vorbis</xsl:when>
   <!-- More codecs should be added here, however I don't know how to find out about the codec.
        At the moment we just guess that application/ogg is a Ogg bitstream with some undefined
        codec. Everything else is undefined.
   -->
   <xsl:otherwise>codec:ogg_general</xsl:otherwise>
  </xsl:choose>
 </xsl:when>
</xsl:choose><xsl:text>
</xsl:text>
<xsl:if test="genre">GENRE=<xsl:value-of select="genre" /></xsl:if>
==
</xsl:for-each>
</xsl:template>
</xsl:stylesheet>
