<?xml version='1.0' encoding='utf-8'?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:NonHtml="http://www.progress.com/StylusStudio/NonHtml" exclude-result-prefixes="NonHtml">
<xsl:output method="html"/>

<xsl:template match="/">
	<html><head></head>
		<body>
			<h1 align="left">
				<font color="#ffffff" size="5" style="background-color: #800000">
					<font face="Arial">&#160;Selected TA-Hub configurations for project : 
						<xsl:value-of select="actualConfiguration/head/projectName"/>
					</font>
				</font>
			</h1>
			<p>
				<table width="40%">
					<tbody>
						<tr>
							<td width="20%">
								<font face="Arial" size="2">
									<strong>Project Name :</strong>
								</font>
							</td>
							<td width="20%">
								<font face="Arial" size="2">
									<xsl:value-of select="actualConfiguration/head/projectName"/>
								</font>
							</td>
						</tr>
						<tr>
							<td width="20%">
								<font face="Arial" size="2">
									<strong>Customer Name :</strong>
								</font>
							</td>
							<td width="20%">
								<font face="Arial" size="2">
									<xsl:value-of select="actualConfiguration/head/customerName"/>
								</font>
							</td>
						</tr>
						<tr>
							<td width="20%">
								<font face="Arial" size="2">
									<strong>Currency :</strong>
								</font>
							</td>
							<td width="20%">
								<font face="Arial" size="2">
									<xsl:value-of select="actualConfiguration/head/currency"/>
								</font>
							</td>
						</tr>
					</tbody>
				</table>
			</p>
			<font size="2"/>
			<xsl:for-each select="actualConfiguration/orderLine">
				<p>
					<font size="4">
						<font color="#ffffff">
							<font style="background-color: #808080">
								<font face="Arial">&#160;TA-Hub :&#160; 
									<xsl:value-of select="orderLineProductID"/>
								</font>
							</font>
						</font>
					</font>
				</p>
				<p>
					<font face="Arial"/>
				</p>
				<p>
					<table width="40%" border="0">
						<tbody>
							<tr>
								<td width="20%">
									<strong>
										<font face="Arial" size="2">Model Part Number :</font>
									</strong>
								</td>
								<td width="20%">
									<font face="Arial" size="2">
										<xsl:value-of select="modelPartNumber"/>
									</font>
								</td>
							</tr>
							<tr>
								<td width="20%">
									<strong>
										<font face="Arial" size="2">Revision :</font>
									</strong>
								</td>
								<td width="20%">
									<font face="Arial" size="2">
										<xsl:value-of select="revision"/>
									</font>
								</td>
							</tr>
							<tr>
								<td width="20%">
									<font face="Arial" size="2">
										<strong>ID nr :</strong>
									</font>
								</td>
								<td width="20%">
									<font face="Arial" size="2">
										<xsl:value-of select="@id"/>
									</font>
								</td>
							</tr>
							<tr>
								<td width="20%">
									<strong>
										<font face="Arial" size="2">Label ID :</font>
									</strong>
								</td>
								<td width="20%">
									<font face="Arial" size="2">
										<xsl:value-of select="orderLineProductID"/>
									</font>
								</td>
							</tr>
							<tr>
								<td width="20%">
									<strong>
										<font face="Arial" size="2">Description :</font>
									</strong>
								</td>
								<td width="20%">
									<font face="Arial" size="2">
										<xsl:value-of select="orderLineDescription"/>
									</font>
								</td>
							</tr>
							<tr>
								<td width="20%">
									<strong>
										<font face="Arial" size="2">Quantity :</font>
									</strong>
								</td>
								<td width="20%">
									<font face="Arial" size="2">
										<xsl:value-of select="quantity"/>
									</font>
								</td>
							</tr>
							<tr>
								<td width="20%">
									<strong>
										<font face="Arial" size="2">Price :</font>
									</strong>
								</td>
								<td width="20%">
									<font face="Arial" size="2">
										<xsl:value-of select="orderLinePrice"/>
									</font>
								</td>
							</tr>
						</tbody>
					</table>
				</p>
				<p>
					<font face="Arial" size="2"/>
				</p>
				<p>
					<font face="Arial" size="2"/>
				</p>
				<p>
					<table width="45%" border="1">
						<tbody>
							<xsl:for-each select="configuration/conf">
								<tr>
									<td width="25%">
										<font face="Arial" size="2">
											<xsl:value-of select="@charId"/>
										</font>
									</td>
									<td width="20%">
										<font face="Arial" size="2">
											<xsl:value-of select="@valueId"/>
										</font>
									</td>
								</tr>
							</xsl:for-each>
						</tbody>
					</table>
				</p>
				<p>
					<font face="Arial"/>
				</p>
				<p>
					<font face="Arial"/>
				</p>
			</xsl:for-each>
		</body>
	</html>
</xsl:template>

<xsl:template match="configuration"/>
<xsl:template match="@valueId"/>
<xsl:template name="NewTemplate0"/>
</xsl:stylesheet><!-- Stylus Studio meta-information - (c) 2004-2006. Progress Software Corporation. All rights reserved.
<metaInformation>
<scenarios ><scenario default="yes" name="TA&#x2D;Hub stylesheet" userelativepaths="yes" externalpreview="no" url="Exported Configurations.xml" htmlbaseurl="" outputurl="" processortype="internal" useresolver="yes" profilemode="0" profiledepth="" profilelength="" urlprofilexml="" commandline="" additionalpath="" additionalclasspath="" postprocessortype="none" postprocesscommandline="" postprocessadditionalpath="" postprocessgeneratedext="" validateoutput="no" validator="internal" customvalidator=""/></scenarios><MapperMetaTag><MapperInfo srcSchemaPathIsRelative="yes" srcSchemaInterpretAsXML="no" destSchemaPath="" destSchemaRoot="" destSchemaPathIsRelative="yes" destSchemaInterpretAsXML="no" ><SourceSchema srcSchemaPath="Exported Configurations.xml" srcSchemaRoot="actualConfiguration" AssociatedInstance="" loaderFunction="document" loaderFunctionUsesURI="no"/></MapperInfo><MapperBlockPosition><template match="/"><block path="html/body/xsl:for&#x2D;each" x="309" y="216"/></template></MapperBlockPosition><TemplateContext></TemplateContext><MapperFilter side="source"></MapperFilter></MapperMetaTag>
</metaInformation>
-->