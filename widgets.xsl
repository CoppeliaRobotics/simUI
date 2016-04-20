<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html" encoding="utf-8" indent="yes"/>
    <xsl:template match="/">
        <xsl:text disable-output-escaping='yes'>&lt;!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Strict//EN"&gt;</xsl:text>
        <html>
            <head>
                <meta http-equiv="Content-Language" content="en-us"/>
                <title>CustomUI Plugin - UI XML Syntax</title>
                <style type="text/css">
                    td, th, a, h1, h2, h3 { font-family: Helvetica, Arial; }
                    a, a:active, a:visited { color: black; text-decoration: none; }
                    a:hover { text-decoration: underline; }
                    td, th { vertical-align: top; }
                    .tt { font-family: Courier New, fixed; font-weight: bold; }
                    th { text-align: right; }
                    table.elements td { background: #eee; }
                    table.attributes td { background: #ddd; }
                    tr.spacer td { background: none; height: 1em; }
                </style>
            </head>
            <body>
                <a name="top"/>
                <h1>CustomUI Plugin - UI XML Syntax</h1>
                <h3>Elements:</h3>
                <ul>
                    <xsl:for-each select="elements/element">
                        <li><a href="#{name/node()}" class="tt">&lt;<xsl:copy-of select="name/node()"/>&gt;</a></li>
                    </xsl:for-each>
                </ul>
                <hr/>
                <table class="elements">
                    <xsl:for-each select="elements/element">
                        <!-- <xsl:sort select="name/node()"/> -->
                        <tr>
                            <th><a name="{name/node()}"/><a href="#top">^</a></th>
                        </tr>
                        <tr>
                            <th>Element:</th>
                            <td class="tt">&lt;<xsl:copy-of select="name/node()"/>&gt;</td>
                        </tr>
                        <tr>
                            <th>Description:</th>
                            <td><xsl:copy-of select="description/node()"/></td>
                        </tr>
                        <tr>
                            <th>Attributes:</th>
                            <td>
                                <table class="attributes">
                                    <xsl:for-each select="attributes/attribute">
                                        <tr>
                                            <th>Name:</th>
                                            <td class="tt"><xsl:copy-of select="name/node()"/></td>
                                        </tr>
                                        <tr>
                                            <th>Type:</th>
                                            <td><xsl:apply-templates select="type"/></td>
                                        </tr>
                                        <tr>
                                            <th>Default:</th>
                                            <td><xsl:copy-of select="default/node()"/></td>
                                        </tr>
                                        <tr>
                                            <th>Description:</th>
                                            <td><xsl:copy-of select="description/node()"/></td>
                                        </tr>
                                        <xsl:if test="not(position() = last())">
                                            <tr class="spacer">
                                                <td colspan="2"></td>
                                            </tr>
                                        </xsl:if>
                                    </xsl:for-each>
                                </table>
                            </td>
                        </tr>
                    </xsl:for-each>
                </table>
            </body>
        </html>
    </xsl:template>
    <xsl:template match="type/one-of">
        One of:
        <ul>
            <xsl:for-each select="value">
                <li><xsl:copy-of select="node()"/></li>
            </xsl:for-each>
        </ul>
    </xsl:template>
</xsl:stylesheet>
