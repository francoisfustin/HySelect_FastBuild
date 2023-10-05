#include "stdafx.h"
#include "Excel_Workbook.h"
#include "Excel_Tools.h"
#include "EnBitmap.h"
#include "..\ZipArchive\ZipArchive.h"
#include "TASelect.h"

Excel_Workbook::Excel_Workbook()
{
	m_tempPath[0] = _T( '\0' );
	m_xlPath[0] = _T( '\0' );
	m_xlrelsPath[0] = _T( '\0' );
	m_xlworksheetsPath[0] = _T( '\0' );
	m_xlworksheetsRelsPath[0] = _T( '\0' );
	m_xlmediaPath[0] = _T( '\0' );
	m_xldrawingsPath[0] = _T( '\0' );
	m_xldrawingsRelsPath[0] = _T( '\0' );
	m_docPropsPath[0] = _T( '\0' );
	m_relsPath[0] = _T( '\0' );
	m_vSheet.clear();
	m_sheetCount = 0;
	m_vPictureCount.clear();
	m_vDrawingParam.clear();
}

bool Excel_Workbook::Write( const TCHAR *filePath, vector<int> *vectShownCols, bool bCustomSheet )
{
	bool retVal = true;
	// *** INIT ***
	m_vColor.clear();
	m_vCellXfs.clear();
	m_vSheetName.clear();
	m_vFontParam.clear();
	m_vDrawingParam.clear();

	if( m_vSheet.empty() )
	{
		return false;
	}

	// Create Temp Dir
	Excel_Tools::CreateTmpDirectory( m_tempPath, MAX_PATH );
	Excel_Tools::DeleteDirectory( m_tempPath, false ); // Must be be empty

	CreateSubDirs();

	if( true == bCustomSheet )
	{
		CreateCustomizedSheets( *vectShownCols );
	}
	else
	{
		CreateSheets();
	}

	CreateContentTypes();
	CreateRels();
	CreateCore();
	CreateApp();
	CreateXlStyles();
	CreateXlWorkbook();
	CreateXlDrawings();
	CreateXlDrawingsRels();
	CreateXlRels();

	if( false == ZipOutput( filePath ) )
	{
		retVal = false;
	}

	// Remove Temp Dir
	Excel_Tools::DeleteDirectory( m_tempPath );
	return retVal;
}

void Excel_Workbook::CreateSubDirs()
{
	wcscat_s( m_xlPath, m_tempPath );
	wcscat_s( m_xlPath, _T("\\xl") );
	CreateDirectory( m_xlPath, NULL ); // xl

	wcscat_s( m_xlrelsPath, m_xlPath );
	wcscat_s( m_xlrelsPath, _T("\\_rels") );
	CreateDirectory( m_xlrelsPath, NULL ); // xl\_rels

	wcscat_s( m_xlworksheetsPath, m_xlPath );
	wcscat_s( m_xlworksheetsPath, _T("\\worksheets") );
	CreateDirectory( m_xlworksheetsPath, NULL ); // xl\worksheets

	wcscat_s( m_xlworksheetsRelsPath, m_xlworksheetsPath );
	wcscat_s( m_xlworksheetsRelsPath, _T("\\_rels") );
	CreateDirectory( m_xlworksheetsRelsPath, NULL ); // xl\worksheets\_rels

	wcscat_s( m_xlmediaPath, m_xlPath );
	wcscat_s( m_xlmediaPath, _T("\\media") );
	CreateDirectory( m_xlmediaPath, NULL ); // xl\media

	wcscat_s( m_xldrawingsPath, m_xlPath );
	wcscat_s( m_xldrawingsPath, _T("\\drawings") );
	CreateDirectory( m_xldrawingsPath, NULL ); // xl\drawings

	wcscat_s( m_xldrawingsRelsPath, m_xldrawingsPath );
	wcscat_s( m_xldrawingsRelsPath, _T("\\_rels") );
	CreateDirectory( m_xldrawingsRelsPath, NULL ); // xl\drawings\_rels

	wcscat_s( m_docPropsPath, m_tempPath );
	wcscat_s( m_docPropsPath, _T("\\docProps") );
	CreateDirectory( m_docPropsPath, NULL ); // docProps

	wcscat_s( m_relsPath, m_tempPath );
	wcscat_s( m_relsPath, _T("\\_rels") );
	CreateDirectory( m_relsPath, NULL ); // _rels
}

void Excel_Workbook::CreateContentTypes()
{
	TCHAR contentTypes[MAX_PATH + 1] = { 0 };
	wcscat_s( contentTypes, m_tempPath );
	wcscat_s( contentTypes, _T("\\[Content_Types].xml") );

	FILE *fpContentTypes;
	errno_t err = _wfopen_s( &fpContentTypes, contentTypes, _T("wb") );

	if( 0 != err )
	{
		return;
	}

	std::fprintf( fpContentTypes, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" );
	std::fprintf( fpContentTypes, "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">" );

	if( PicturesUsed() )
	{
		std::fprintf( fpContentTypes, "<Default Extension=\"png\" ContentType=\"image/png\"/>" );
		std::fprintf( fpContentTypes, "<Default Extension=\"jpeg\" ContentType=\"image/jpeg\"/>" );
	}

	std::fprintf( fpContentTypes, "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\" />" );
	std::fprintf( fpContentTypes, "<Default Extension=\"xml\" ContentType=\"application/xml\" />" );
	
	std::fprintf( fpContentTypes, "<Override PartName=\"/xl/workbook.xml\" " );
	std::fprintf( fpContentTypes, "ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\" />" );

	for( int i = 0; i < m_sheetCount; i++ )
	{
		std::fprintf( fpContentTypes, "<Override PartName=\"/xl/worksheets/sheet%d.xml\" ", i + 1 );
		std::fprintf( fpContentTypes, "ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\" />" );
	}

	std::fprintf( fpContentTypes, "<Override PartName=\"/xl/styles.xml\" " );
	std::fprintf( fpContentTypes, "ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\" />" );

	for( int i = 0; i < m_sheetCount; i++ )
	{
		std::fprintf( fpContentTypes, "<Override PartName=\"/xl/drawings/drawing%d.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawing+xml\"/>",
				 i + 1 );
	}

	std::fprintf( fpContentTypes, "<Override PartName=\"/docProps/core.xml\" ContentType=\"application/vnd.openxmlformats-package.core-properties+xml\" />" );

	std::fprintf( fpContentTypes, "<Override PartName=\"/docProps/app.xml\" " );
	std::fprintf( fpContentTypes, "ContentType=\"application/vnd.openxmlformats-officedocument.extended-properties+xml\" />" );
	
	std::fprintf( fpContentTypes, "</Types>" );

	fclose( fpContentTypes );
}

void Excel_Workbook::CreateRels()
{
	TCHAR relsPath[MAX_PATH + 1] = { 0 };
	wcscat_s( relsPath, m_relsPath );
	wcscat_s( relsPath, _T("\\.rels") );

	FILE *fpRels;
	errno_t err = _wfopen_s( &fpRels, relsPath, _T("wb") );

	if( 0 != err )
	{
		return;
	}

	std::fprintf( fpRels, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" );
	std::fprintf( fpRels, "<Relationships xmlns = \"http://schemas.openxmlformats.org/package/2006/relationships\">" );

	std::fprintf( fpRels, "<Relationship Id=\"rId3\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties\" " );
	std::fprintf( fpRels, "Target=\"docProps/app.xml\" />" );
	
	std::fprintf( fpRels, "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties\" " );
	std::fprintf( fpRels, "Target=\"docProps/core.xml\" />" );
	
	std::fprintf( fpRels, "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" " );
	std::fprintf( fpRels, "Target=\"xl/workbook.xml\" />" );
	
	std::fprintf( fpRels, "</Relationships>" );

	fclose( fpRels );
}

void Excel_Workbook::CreateCore()
{
	TCHAR corePath[MAX_PATH + 1] = { 0 };
	wcscat_s( corePath, m_docPropsPath );
	wcscat_s( corePath, _T("\\core.xml") );

	FILE *fpCore;
	errno_t err = _wfopen_s( &fpCore, corePath, _T("wb") );

	if( 0 != err )
	{
		return;
	}

	CString appName = TASApp.LoadLocalizedString( IDS_ABOUTBOX_STATICSOFTNAME );
	std::string xmlAppName = Excel_Tools::ws2s( appName.GetString() );

	std::fprintf( fpCore, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" );
	std::fprintf( fpCore, "<cp:coreProperties xmlns:cp = \"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" " );
	std::fprintf( fpCore, "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:dcterms = \"http://purl.org/dc/terms/\" " );
	std::fprintf( fpCore, "xmlns:dcmitype=\"http://purl.org/dc/dcmitype/\" xmlns:xsi = \"http://www.w3.org/2001/XMLSchema-instance\">" );
	
	std::fprintf( fpCore, "<dc:creator>%s</dc:creator>", xmlAppName.c_str() );
	std::fprintf( fpCore, "<cp:lastModifiedBy>%s</cp:lastModifiedBy>", xmlAppName.c_str() );

	time_t rawtime;
	struct tm timeinfo;
	char buffer[80];
	time( &rawtime );
	err = localtime_s( &timeinfo, &rawtime );

#ifdef DEBUG
	sprintf_s( buffer, "1976-08-02T08:07:06Z" ); // Hard coded time for testing purpose in debug
#else
	strftime( buffer, 80, "%Y-%m-%dT%H:%M:%SZ", &timeinfo ); // 2014-10-21T12:34:18Z
#endif

	std::fprintf( fpCore, "<dcterms:created xsi:type=\"dcterms:W3CDTF\">%s</dcterms:created>", buffer );
	std::fprintf( fpCore, "<dcterms:modified xsi:type=\"dcterms:W3CDTF\">%s</dcterms:modified>", buffer );

	std::fprintf( fpCore, "</cp:coreProperties>" );

	fclose( fpCore );
}

void Excel_Workbook::CreateApp()
{
	TCHAR appPath[MAX_PATH + 1] = { 0 };
	wcscat_s( appPath, m_docPropsPath );
	wcscat_s( appPath, _T("\\app.xml") );

	FILE *fpApp;
	errno_t err = _wfopen_s( &fpApp, appPath, _T("wb") );

	if( 0 != err )
	{
		return;
	}

	std::fprintf( fpApp, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" );

	std::fprintf( fpApp, "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/extended-properties\" " );
	std::fprintf( fpApp, "xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\">" );
	
	std::fprintf( fpApp, "<Application>Microsoft Excel</Application>" );
	std::fprintf( fpApp, "<DocSecurity>0</DocSecurity >" );
	std::fprintf( fpApp, "<ScaleCrop>false</ScaleCrop>" );

	std::fprintf( fpApp, "<HeadingPairs>" );
	std::fprintf( fpApp, "<vt:vector size=\"2\" baseType=\"variant\">" );
	std::fprintf( fpApp, "<vt:variant>" );
	std::fprintf( fpApp, "<vt:lpstr>Worksheets</vt:lpstr>" );
	std::fprintf( fpApp, "</vt:variant>" );
	std::fprintf( fpApp, "<vt:variant>" );
	std::fprintf( fpApp, "<vt:i4>1 </vt:i4 >" );
	std::fprintf( fpApp, "</vt:variant>" );
	std::fprintf( fpApp, "</vt:vector>" );
	std::fprintf( fpApp, "</HeadingPairs>" );

	std::fprintf( fpApp, "<TitlesOfParts>" );
	std::fprintf( fpApp, "<vt:vector size = \"1\" baseType = \"lpstr\">" );
	std::fprintf( fpApp, "<vt:lpstr>Sheet1</vt:lpstr>" );
	std::fprintf( fpApp, "</vt:vector>" );
	std::fprintf( fpApp, "</TitlesOfParts>" );

	std::fprintf( fpApp, "<Company/>" );
	std::fprintf( fpApp, "<LinksUpToDate>false</LinksUpToDate>" );
	std::fprintf( fpApp, "<SharedDoc>false</SharedDoc>" );
	std::fprintf( fpApp, "<HyperlinksChanged>false</HyperlinksChanged>" );
	std::fprintf( fpApp, "<AppVersion>15.0300</AppVersion>" );

	std::fprintf( fpApp, "</Properties>" );

	fclose( fpApp );
}

void Excel_Workbook::CreateXlStyles()
{
	TCHAR stylesPath[MAX_PATH + 1] = { 0 };
	wcscat_s( stylesPath, m_xlPath );
	wcscat_s( stylesPath, _T("\\styles.xml") );

	FILE *fpStyles;
	errno_t err = _wfopen_s( &fpStyles, stylesPath, _T("wb") );

	if( 0 != err )
	{
		return;
	}

	std::fprintf( fpStyles, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" );
	std::fprintf( fpStyles, "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" " );
	std::fprintf( fpStyles, "xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" " );
	std::fprintf( fpStyles, "mc:Ignorable=\"x14ac\" xmlns:x14ac=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/ac\">\n" );

	std::fprintf( fpStyles, "<fonts count=\"%lu\" x14ac:knownFonts=\"1\">\n", m_vFontParam.size() + 1 );
	std::fprintf( fpStyles, "<font>" );
	std::fprintf( fpStyles, "<sz val=\"11\" /><color theme=\"1\" />" );
	std::fprintf( fpStyles, "<name val=\"Calibri\" /><family val=\"2\" />" );
	std::fprintf( fpStyles, "<scheme val=\"minor\" />" );
	std::fprintf( fpStyles, "</font>\n" );

	for( unsigned int i = 0; i < m_vFontParam.size(); ++i )
	{
		std::fprintf( fpStyles, "<font>" );
		std::fprintf( fpStyles, "<sz val=\"%g\"/>", m_vFontParam[i].fontSize );

		if( m_vFontParam[i].bold )
		{
			std::fprintf( fpStyles, "<b/>" );
		}

		char fontColor[9] = { 0 };
		DWORD color = m_vFontParam[i].fontColor;
		sprintf_s( fontColor, "FF%02X%02X%02X", ( color & 0xFF ), ( color & 0xFF00 ) >> 8, ( color & 0xFF0000 ) >> 16 );
		std::fprintf( fpStyles, "<color rgb=\"%s\"/>", fontColor );
		std::fprintf( fpStyles, "</font>\n" );
	}

	std::fprintf( fpStyles, "</fonts>\n" );

	std::fprintf( fpStyles, "<fills count=\"%lu\">\n", m_vColor.size() + 2 );
	std::fprintf( fpStyles, "<fill><patternFill patternType=\"none\" /></fill>\n" );
	std::fprintf( fpStyles, "<fill><patternFill patternType=\"gray125\" /></fill>\n" );

	for( unsigned int i = 0; i < m_vColor.size(); i++ )
	{
		// Most of fill pattern uses two colors (fgColor + bgColor)
		// "Solid" pattern uses only fgColor

		char fillColor[9] = {0};
		sprintf_s( fillColor, "FF%02X%02X%02X", ( m_vColor[i] & 0xFF ), ( m_vColor[i] & 0xFF00 ) >> 8, ( m_vColor[i] & 0xFF0000 ) >> 16 );

		std::fprintf( fpStyles, "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"%s\"/></patternFill></fill>\n", fillColor );
	}

	std::fprintf( fpStyles, "</fills>\n" );

	std::fprintf( fpStyles, "<borders count=\"%lu\">\n", m_vBorderType.size() + 1 );
	std::fprintf( fpStyles, "<border>" );
	std::fprintf( fpStyles, "<left/><right/><top/><bottom/><diagonal/>" );
	std::fprintf( fpStyles, "</border>\n" );

	for( unsigned int i = 0; i < m_vBorderType.size(); i++ )
	{
		std::fprintf( fpStyles, "<border>" );
		WriteBorders( m_vBorderType[i], fpStyles );
		std::fprintf( fpStyles, "</border>\n" );
	}

	std::fprintf( fpStyles, "</borders>" );

	std::fprintf( fpStyles, "<cellStyleXfs count=\"1\">" );
	std::fprintf( fpStyles, "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" />" );
	std::fprintf( fpStyles, "</cellStyleXfs>\n" );

	std::fprintf( fpStyles, "<cellXfs count=\"%lu\">\n", m_vCellXfs.size() + 1 );
	std::fprintf( fpStyles, "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\" />\n" );

	for( unsigned int i = 0; i < m_vCellXfs.size(); ++i )
	{
		std::fprintf( fpStyles, "<xf numFmtId=\"0\" fontId=\"%d\" fillId=\"%d\" xfId=\"0\" borderId=\"%d\" applyFill=\"1\">",
				 m_vCellXfs[i].fontId,
				 m_vCellXfs[i].fillId,
				 m_vCellXfs[i].borderTypeId );
		std::fprintf( fpStyles, "<alignment " );

		WriteHAlign( i, fpStyles );
		WriteVAlign( i, fpStyles );

		std::fprintf( fpStyles, "wrapText=\"1\"/></xf>\n" );
	}

	std::fprintf( fpStyles, "</cellXfs>" );

	std::fprintf( fpStyles, "<cellStyles count = \"1\">" );
	std::fprintf( fpStyles, "<cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\" />" );
	std::fprintf( fpStyles, "</cellStyles>" );

	std::fprintf( fpStyles, "<dxfs count=\"0\" />" );
	std::fprintf( fpStyles, "<tableStyles count=\"0\" defaultTableStyle=\"TableStyleMedium2\" defaultPivotStyle=\"PivotStyleLight16\" />" );

	std::fprintf( fpStyles, "<extLst>" );
	std::fprintf( fpStyles, "<ext uri=\"{EB79DEF2-80B8-43e5-95BD-54CBDDF9020C}\" xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\">" );
	std::fprintf( fpStyles, "<x14:slicerStyles defaultSlicerStyle = \"SlicerStyleLight1\" />" );
	std::fprintf( fpStyles, "</ext>" );
	std::fprintf( fpStyles, "<ext uri=\"{9260A510-F301-46a8-8635-F512D64BE5F5}\" xmlns:x15=\"http://schemas.microsoft.com/office/spreadsheetml/2010/11/main\">" );
	std::fprintf( fpStyles, "<x15:timelineStyles defaultTimelineStyle=\"TimeSlicerStyleLight1\" />" );
	std::fprintf( fpStyles, "</ext>" );
	std::fprintf( fpStyles, "</extLst>" );
	
	std::fprintf( fpStyles, "</styleSheet>" );

	fclose( fpStyles );
}

void Excel_Workbook::CreateXlWorkbook()
{
	TCHAR workbookPath[MAX_PATH + 1] = { 0 };
	wcscat_s( workbookPath, m_xlPath );
	wcscat_s( workbookPath, _T("\\workbook.xml") );

	FILE *fpWorkbook;
	errno_t err = _wfopen_s( &fpWorkbook, workbookPath, _T("wb") );

	if( 0 != err )
	{
		return;
	}

	std::fprintf( fpWorkbook, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" );
	std::fprintf( fpWorkbook, "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" " );
	std::fprintf( fpWorkbook, "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" " );
	std::fprintf( fpWorkbook, "xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" " );
	std::fprintf( fpWorkbook, "mc:Ignorable=\"x15\" xmlns:x15=\"http://schemas.microsoft.com/office/spreadsheetml/2010/11/main\">" );
	std::fprintf( fpWorkbook, "<fileVersion appName=\"xl\" lastEdited=\"6\" lowestEdited=\"6\" rupBuild=\"14420\"/>" );
	std::fprintf( fpWorkbook, "<workbookPr defaultThemeVersion=\"153222\"/>" );
	std::fprintf( fpWorkbook, "<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\">" );
	std::fprintf( fpWorkbook, "<mc:Choice Requires=\"x15\">" );
	std::fprintf( fpWorkbook, "<x15ac:absPath url=\"C:\\Users\\\" xmlns:x15ac=\"http://schemas.microsoft.com/office/spreadsheetml/2010/11/ac\"/>" );
	std::fprintf( fpWorkbook, "</mc:Choice>" );
	std::fprintf( fpWorkbook, "</mc:AlternateContent>" );

	std::fprintf( fpWorkbook, "<bookViews>" );
	std::fprintf( fpWorkbook, "<workbookView xWindow=\"0\" yWindow=\"0\" windowWidth=\"27840\" windowHeight=\"11385\"/>" );
	std::fprintf( fpWorkbook, "</bookViews>" );
	
	std::fprintf( fpWorkbook, "<sheets>" );

	for( int i = 0; i < m_sheetCount; i++ )
	{
		// *** SHEET NAME : MAX LENGTH 31 CHARS ***
		char count[8] = { 0 };
		_itoa_s( i + 1, count, 10 );
		std::string strName = Excel_Tools::ws2s( m_vSheetName[i] ).substr( 0, 31 - 3 - strlen( count ) ).c_str();
		Excel_Tools::ClearNameForSheet( strName );
		std::fprintf( fpWorkbook, "<sheet name=\"%s (%d)\" sheetId=\"%d\" r:id=\"rId%d\"/>", strName.c_str(), i + 1, i + 1, i + 1 );
	}

	std::fprintf( fpWorkbook, "</sheets>" );

	std::fprintf( fpWorkbook, "<calcPr calcId=\"152511\"/>" );

	std::fprintf( fpWorkbook, "<extLst>" );
	std::fprintf( fpWorkbook, "<ext uri=\"{140A7094-0E35-4892-8432-C4D2E57EDEB5}\" xmlns:x15=\"http://schemas.microsoft.com/office/spreadsheetml/2010/11/main\">" );
	std::fprintf( fpWorkbook, "<x15:workbookPr chartTrackingRefBase=\"1\"/>" );
	std::fprintf( fpWorkbook, "</ext>" );
	std::fprintf( fpWorkbook, "</extLst>" );
	
	std::fprintf( fpWorkbook, "</workbook>" );

	fclose( fpWorkbook );
}

void Excel_Workbook::CreateXlWorksheetsRels( int pageNum )
{
	TCHAR xlRelsPath[MAX_PATH + 1] = { 0 };
	wsprintf( xlRelsPath, _T("%s\\sheet%d.xml.rels"), m_xlworksheetsRelsPath, pageNum );

	FILE *fpXlRels;
	errno_t err = _wfopen_s( &fpXlRels, xlRelsPath, _T("wb") );

	if( 0 != err )
	{
		return;
	}

	std::fprintf( fpXlRels, "<?xml version = \"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" );
	std::fprintf( fpXlRels, "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">" );
	std::fprintf( fpXlRels, "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing\" " );
	std::fprintf( fpXlRels, "Target=\"../drawings/drawing%d.xml\" />", pageNum );
	std::fprintf( fpXlRels, "</Relationships>" );

	fclose( fpXlRels );
}

void Excel_Workbook::CreateXlRels()
{
	TCHAR xlRelsPath[MAX_PATH + 1] = { 0 };
	wcscat_s( xlRelsPath, m_xlrelsPath );
	wcscat_s( xlRelsPath, _T("\\workbook.xml.rels") );

	FILE *fpXlRels;
	errno_t err = _wfopen_s( &fpXlRels, xlRelsPath, _T("wb") );

	if( 0 != err )
	{
		return;
	}

	std::fprintf( fpXlRels, "<?xml version = \"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" );
	std::fprintf( fpXlRels, "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">" );
	std::fprintf( fpXlRels, "<Relationship Id=\"rId%d\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" ", m_sheetCount + 1 );
	std::fprintf( fpXlRels, "Target=\"styles.xml\" />" );

	for( int i = 0; i < m_sheetCount; i++ )
	{
		std::fprintf( fpXlRels, "<Relationship Id=\"rId%d\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" ", i + 1 );
		std::fprintf( fpXlRels, "Target=\"worksheets/sheet%d.xml\" />", i + 1 );
	}

	std::fprintf( fpXlRels, "</Relationships>" );

	fclose( fpXlRels );
}

int Excel_Workbook::AddSheet( CSSheet *sheet )
{
	if( NULL == sheet )
	{
		return 0;
	}

	m_vSheet.push_back( sheet );
	return m_vSheet.size();
}

void Excel_Workbook::CreateCustomizedSheets( vector<int> vectShownCols )
{
	// *** INIT ***
	m_sheetCount = 0;
	m_vPictureCount.clear();

	// *** SpreadSheet LOOP ***
	for( unsigned int i = 0; i < m_vSheet.size(); i++ )
	{
		short count = m_vSheet[i]->GetSheetCount();

		// *** Sheet LOOP ***
		for( int page = 0; page < count; ++page )
		{
			if( false == m_vSheet[i]->GetSheetVisible( page + 1 ) )
			{
				continue;
			}

			CSSheet *currentSpread = m_vSheet[i];

			currentSpread->SetSheet( page + 1 );
			m_vPictureCount.push_back( 0 );
			m_sheetCount++;

			// *** SHEET NAME ***
			SaveSheetName( currentSpread, page );

			TCHAR sheetPath[MAX_PATH + 1] = { 0 };
			wcscat_s( sheetPath, m_xlworksheetsPath );
			TCHAR sheetFilename[32] = { 0 };
			wsprintf( sheetFilename, _T("\\sheet%d.xml"), m_sheetCount );
			wcscat_s( sheetPath, sheetFilename );
			std::vector<std::string> mergeCell;
			mergeCell.clear();

			FILE *fpSheet;
			errno_t err = _wfopen_s( &fpSheet, sheetPath, _T("wb") );

			if( 0 != err )
			{
				return;
			}

			int maxCols = currentSpread->GetMaxCols();
			int maxRows = currentSpread->GetMaxRows();

			std::fprintf( fpSheet, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" );
			std::fprintf( fpSheet, "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" " );
			std::fprintf( fpSheet, "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" " );
			std::fprintf( fpSheet, "xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" " );
			std::fprintf( fpSheet, "mc:Ignorable=\"x14ac xr xr2 xr3\" xmlns:x14ac=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/ac\" " );
			std::fprintf( fpSheet, "xmlns:xr=\"http://schemas.microsoft.com/office/spreadsheetml/2014/revision\" " );
			std::fprintf( fpSheet, "xmlns:xr2=\"http://schemas.microsoft.com/office/spreadsheetml/2015/revision2\" " );
			std::fprintf( fpSheet, "xmlns:xr3=\"http://schemas.microsoft.com/office/spreadsheetml/2016/revision3\">" );

			std::fprintf( fpSheet, "<sheetViews>" );
			std::fprintf( fpSheet, "<sheetView tabSelected=\"0\" workbookViewId=\"0\"/>" );
			std::fprintf( fpSheet, "</sheetViews>" );

			std::fprintf( fpSheet, "<sheetFormatPr defaultRowHeight=\"15\" outlineLevelCol=\"4\" x14ac:dyDescent=\"0.25\"/>" );
			// HYS-1095: col button management only for the first sheet "Results"
			TCHAR tSheetName[30] = { 0 };
			currentSpread->GetSheetName( page + 1, tSheetName, 30 );
			if( 0 == wcscmp( tSheetName, TASApp.LoadLocalizedString( IDS_SSHEET_EXPORT_SSNAMEVALVE ) ) )
			{
				std::fprintf( fpSheet, "<cols>" );
				
				for( int cols = 1; cols <= maxCols; cols++ )
				{
					double cWidth = 0.0;
					currentSpread->GetColWidth( cols, &cWidth );
					std::fprintf( fpSheet, "<col min=\"%d\" max=\"%d\" width=\"%g\" bestFit=\"1\" customWidth=\"1\"/>", cols, cols, cWidth * 1.2 );
				}
				
				for( int cols = 1; cols <= maxCols; cols++ )
				{
					double cWidth = 0.0;
					currentSpread->GetColWidth( cols, &cWidth );
					std::vector<int>::iterator it;
					it = find( vectShownCols.begin(), vectShownCols.end(), cols );

					// 4 =  CD_Prim_Ref2
					// 10 = CD_Prim_Product_DpCDpLr
					// 22 = CD_Prim_TechInfo_DpCDpL
					// 52 = CD_Mode
					// HYS-1680: Add a group for col 14-18, column description CD_Prim_Product_Qnom, CD_Prim_Product_CableLengthToRemoteTempSensor, CD_Prim_Product_TASmartPowerSupply, 
					// CD_Prim_Product_TASmartInputSignal, CD_Prim_Product_TASmartOutputSignal	
					if( ( 4 < cols && 52 != cols ) && ( ( it == vectShownCols.end() ) || ( 10 == cols || 22 == cols ) ) )
					{
						if( 14 <= cols && 18 >= cols )
						{
							std::fprintf( fpSheet, "<col min=\"%d\" max=\"%d\" width=\"%g\" bestFit=\"1\"   customWidth=\"1\"  outlineLevel=\"3\" />", cols, cols, cWidth * 1.2 );
						}
						else if( 10 != cols && 22 != cols )
						{
							// button 2: show dpc infos col 10 & 22
							std::fprintf( fpSheet, "<col min=\"%d\" max=\"%d\" width=\"%g\" bestFit=\"1\"   customWidth=\"1\"  outlineLevel=\"2\" />", cols, cols, cWidth * 1.2 );
						}
						else
						{
							std::fprintf( fpSheet, "<col min=\"%d\" max=\"%d\" width=\"%g\" bestFit=\"1\"   customWidth=\"1\"  outlineLevel=\"1\" />", cols, cols, cWidth * 1.2 );
						}
					}
				}

				std::fprintf( fpSheet, "</cols>" );
				currentSpread->SetSheet( 0 );
				currentSpread->SetActiveSheet( 0 );
			}
			else if( 0 == wcscmp( tSheetName, TASApp.LoadLocalizedString( IDS_SSHEET_EXPORT_SSNAMEPNEUMATEX ) ) )
			{
				std::fprintf( fpSheet, "<cols>" );

				for( int cols = 1; cols <= maxCols; cols++ )
				{
					double cWidth = 0.0;
					currentSpread->GetColWidth( cols, &cWidth );
					std::fprintf( fpSheet, "<col min=\"%d\" max=\"%d\" width=\"%g\" customWidth=\"1\"/>", cols, cols, cWidth * 1.2 );
				}
				
				for( int cols = 1; cols <= maxCols; cols++ )
				{
					double cWidth = 0.0;
					currentSpread->GetColWidth( cols, &cWidth );

					// 10 = ColumnDescription_PMWQ_InputData::CD_PMWQ_IO_Sep, 
					// 60 = ColumnDescription_PMWQ_OutputData::CD_PMWQ_IO_Pointer
					// 61 = ColumnDescription_PMWQ_SeparatorTechInfos::CD_PMWQ_SepTechInfo_Sep
					// 65 = ColumnDescription_PMWQ_SafetyValveTechInfos::CD_PMWQ_SafValTechInfo_Sep
					// 72 = ColumnDescription_PMWQ_SafetyValveTechInfos::CD_PMWQ_LAST
					if( cols > 10 && cols != 60 && cols != 61 && cols != 65 && cols != 72 )
					{	
						if( cols < 72 )
						{
							// 2 button hide cols < 72
							std::fprintf( fpSheet, "<col min=\"%d\" max=\"%d\" width=\"%g\" customWidth=\"1\"  outlineLevel=\"2\" />", cols, cols, cWidth * 1.2 );
						}
						else if( cols > 72 )
						{
							std::fprintf( fpSheet, "<col min=\"%d\" max=\"%d\" width=\"%g\"  customWidth=\"1\"  outlineLevel=\"1\" />", cols, cols, cWidth * 1.2 );
						}
						// 1 button for col1 - 10
					}
				}

				std::fprintf( fpSheet, "</cols>" );
				currentSpread->SetSheet( 0 );
				currentSpread->SetActiveSheet( 0 );
			}
			else
			{
				AddColsWidth( fpSheet, maxCols, currentSpread );
			}
			
			std::fprintf( fpSheet, "<sheetData>\n" );

			// *** ROWS + CELLS ***
			CreateSheetRows( maxRows, currentSpread, fpSheet, maxCols, mergeCell );

			std::fprintf( fpSheet, "</sheetData>" );

			AddMergeCell( mergeCell, fpSheet );

			std::fprintf( fpSheet, "<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.7\" bottom=\"0.7\" header=\"0.3\" footer=\"0.3\"/>" );

			if( m_vPictureCount[m_sheetCount - 1] )
			{
				std::fprintf( fpSheet, "<drawing r:id=\"rId1\"/>" );
			}

			std::fprintf( fpSheet, "</worksheet>" );

			fclose( fpSheet );
		}

	}
}

void Excel_Workbook::CreateSheets()
{
	// *** INIT ***
	m_sheetCount = 0;
	m_vPictureCount.clear();

	// *** SpreadSheet LOOP ***
	for( unsigned int i = 0; i < m_vSheet.size(); i++ )
	{
		short count = m_vSheet[i]->GetSheetCount();

		// *** Sheet LOOP ***
		for( int page = 0; page < count; ++page )
		{
			if( false == m_vSheet[i]->GetSheetVisible( page + 1 ) )
			{
				continue;
			}

			CSSheet *currentSpread = m_vSheet[i];

			currentSpread->SetSheet( page + 1 );
			m_vPictureCount.push_back( 0 );
			m_sheetCount++;

			// *** SHEET NAME ***
			SaveSheetName( currentSpread, page );

			TCHAR sheetPath[MAX_PATH + 1] = { 0 };
			wcscat_s( sheetPath, m_xlworksheetsPath );
			TCHAR sheetFilename[32] = { 0 };
			wsprintf( sheetFilename, _T("\\sheet%d.xml"), m_sheetCount );
			wcscat_s( sheetPath, sheetFilename );
			std::vector<std::string> mergeCell;
			mergeCell.clear();

			FILE *fpSheet;
			errno_t err = _wfopen_s( &fpSheet, sheetPath, _T("wb") );

			if( 0 != err )
			{
				return;
			}

			int maxCols = currentSpread->GetMaxCols();
			int maxRows = currentSpread->GetMaxRows();

			std::fprintf( fpSheet, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" );
			std::fprintf( fpSheet, "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" " );
			std::fprintf( fpSheet, "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" " );
			std::fprintf( fpSheet, "xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" " );
			std::fprintf( fpSheet, "mc:Ignorable=\"x14ac\" xmlns:x14ac=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/ac\">" );

			std::fprintf( fpSheet, "<sheetViews>" );
			std::fprintf( fpSheet, "<sheetView tabSelected=\"1\" workbookViewId=\"0\"/>" );
			std::fprintf( fpSheet, "</sheetViews>" );

			std::fprintf( fpSheet, "<sheetFormatPr defaultRowHeight=\"15\" x14ac:dyDescent=\"0.25\"/>" );

			AddColsWidth( fpSheet, maxCols, currentSpread );

			std::fprintf( fpSheet, "<sheetData>\n" );

			// *** ROWS + CELLS ***
			CreateSheetRows( maxRows, currentSpread, fpSheet, maxCols, mergeCell );

			std::fprintf( fpSheet, "</sheetData>" );

			AddMergeCell( mergeCell, fpSheet );

			std::fprintf( fpSheet, "<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.7\" bottom=\"0.7\" header=\"0.3\" footer=\"0.3\"/>" );

			if( m_vPictureCount[m_sheetCount - 1] )
			{
				std::fprintf( fpSheet, "<drawing r:id=\"rId1\"/>" );
			}

			std::fprintf( fpSheet, "</worksheet>" );

			fclose( fpSheet );
		}

	}
}

bool Excel_Workbook::ZipOutput( const TCHAR *filePath )
{
	try
	{
		CZipArchive zip;
		zip.Open( filePath, CZipArchive::zipCreate );

		std::vector<std::wstring> fileList;
		Excel_Tools::GetFileList( m_tempPath, fileList );

		for( unsigned int i = 0; i < fileList.size(); i++ )
		{
			std::wstring fullPath = m_tempPath;
			fullPath += _T("\\");
			fullPath += fileList[i];

			zip.AddNewFile( fullPath.c_str(), fileList[i].c_str() );
		}

		zip.Close();
	}
	catch( ... )
	{
		return false;
	}

	return true;
}

void Excel_Workbook::AddMergeCell( std::vector<std::string> &mergeCell, FILE *fpSheet )
{
	if( false == mergeCell.empty() )
	{
		std::fprintf( fpSheet, "<mergeCells>" );

		for( unsigned int i = 0; i < mergeCell.size(); ++i )
		{
			std::fprintf( fpSheet, "%s\n", mergeCell[i].c_str() );
		}

		std::fprintf( fpSheet, "</mergeCells>" );
	}
}

void Excel_Workbook::ComputeMergeCell( CSSheet *currentSpread, int *cols, int *rows, std::vector<std::string> &mergeCell )
{
	SS_COORD colAnchor = 0;
	SS_COORD rowAnchor = 0;
	SS_COORD numCols = 0;
	SS_COORD numRows = 0;
	currentSpread->GetCellSpan( *cols + 1, *rows + 1, &colAnchor, &rowAnchor, &numCols, &numRows );

	bool isAnchor = ( *cols + 1 == colAnchor ) && ( *rows + 1 == rowAnchor );

	if( isAnchor && ( numCols + numRows ) > 1 )
	{
		char strMergeCell[256] = { 0 };
		std::string ref = Excel_Tools::GetRef( *rows + 1, *cols + 1 );
		sprintf_s( strMergeCell, "<mergeCell ref=\"%s:%s\"/>", ref.c_str(), Excel_Tools::GetRef( rowAnchor + numRows - 1, colAnchor + numCols - 1 ).c_str() );
		mergeCell.push_back( strMergeCell );
	}
	else if( !isAnchor && ( numCols + numRows ) > 1 )
	{
		*cols = colAnchor - 1;
		*rows = rowAnchor - 1;
	}
}

void Excel_Workbook::AddColsWidth( FILE *fpSheet, int maxCols, CSSheet *currentSpread )
{
	std::fprintf( fpSheet, "<cols>" );

	for( int cols = 0; cols < maxCols; cols++ )
	{
		double cWidth = 0.0;
		currentSpread->GetColWidth( cols + 1, &cWidth );
		/* 1.2 to approch Spread aspect ratio */
		std::fprintf( fpSheet, "<col min=\"%d\" max=\"%d\" width=\"%g\"/>", cols + 1, cols + 1, cWidth * 1.2 );
	}

	std::fprintf( fpSheet, "</cols>" );
}

void Excel_Workbook::CreateXlDrawings()
{
	for( int i = 0; i < m_sheetCount; ++i )
	{
		TCHAR fileName[MAX_PATH + 1] = { 0 };
		wsprintf( fileName, _T("%s\\drawing%d.xml"), m_xldrawingsPath, i + 1 );

		FILE *fpXlDrawing;
		errno_t err = _wfopen_s( &fpXlDrawing, fileName, _T("wb") );

		if( 0 != err )
		{
			return;
		}

		std::fprintf( fpXlDrawing, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" );
		std::fprintf( fpXlDrawing, "<xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\" " );
		std::fprintf( fpXlDrawing, "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">" );

		for( int j = PicturesStartAt( i ); j < m_vPictureCount[i] + PicturesStartAt( i ); ++j )
		{
			int idCount = j - PicturesStartAt( i ) + 1;
			std::fprintf( fpXlDrawing, "<xdr:twoCellAnchor editAs=\"oneCell\">" );
			std::fprintf( fpXlDrawing, "<xdr:from>" );

			std::fprintf( fpXlDrawing, "<xdr:col>%d</xdr:col>", m_vDrawingParam[j - 1].fromCol - 1 );
			std::fprintf( fpXlDrawing, "<xdr:colOff>12700</xdr:colOff>" );
			std::fprintf( fpXlDrawing, "<xdr:row>%d</xdr:row>", m_vDrawingParam[j - 1].fromRow - 1 );
			std::fprintf( fpXlDrawing, "<xdr:rowOff>12700</xdr:rowOff>" );

			std::fprintf( fpXlDrawing, "</xdr:from>" );
			std::fprintf( fpXlDrawing, "<xdr:to>" );

			std::fprintf( fpXlDrawing, "<xdr:col>%d</xdr:col>", m_vDrawingParam[j - 1].toCol );
			std::fprintf( fpXlDrawing, "<xdr:colOff>12700</xdr:colOff>" );
			std::fprintf( fpXlDrawing, "<xdr:row>%d</xdr:row>", m_vDrawingParam[j - 1].toRow );
			std::fprintf( fpXlDrawing, "<xdr:rowOff>12700</xdr:rowOff>" );

			std::fprintf( fpXlDrawing, "</xdr:to>" );
			std::fprintf( fpXlDrawing, "<xdr:pic>" );

			std::fprintf( fpXlDrawing, "<xdr:nvPicPr>" );
			std::fprintf( fpXlDrawing, "<xdr:cNvPr id=\"%d\" name=\"Picture 1\"/>", idCount );
			std::fprintf( fpXlDrawing, "<xdr:cNvPicPr>" );
			std::fprintf( fpXlDrawing, "<a:picLocks noChangeAspect=\"1\"/>" );
			std::fprintf( fpXlDrawing, "</xdr:cNvPicPr>" );
			std::fprintf( fpXlDrawing, "</xdr:nvPicPr>" );

			std::fprintf( fpXlDrawing, "<xdr:blipFill>" );
			std::fprintf( fpXlDrawing, "<a:blip xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" r:embed=\"rId%d\" cstate=\"print\">", idCount );
			std::fprintf( fpXlDrawing, "<a:extLst>" );
			std::fprintf( fpXlDrawing, "<a:ext uri=\"{28A0092B-C50C-407E-A947-70E740481C1C}\">" );
			std::fprintf( fpXlDrawing, "<a14:useLocalDpi xmlns:a14=\"http://schemas.microsoft.com/office/drawing/2010/main\" val=\"0\"/>" );
			std::fprintf( fpXlDrawing, "</a:ext>" );
			std::fprintf( fpXlDrawing, "</a:extLst>" );
			std::fprintf( fpXlDrawing, "</a:blip>" );
			std::fprintf( fpXlDrawing, "<a:stretch>" );
			std::fprintf( fpXlDrawing, "<a:fillRect/>" );
			std::fprintf( fpXlDrawing, "</a:stretch>" );
			std::fprintf( fpXlDrawing, "</xdr:blipFill>" );

			std::fprintf( fpXlDrawing, "<xdr:spPr>" );
			std::fprintf( fpXlDrawing, "<a:xfrm>" );
			std::fprintf( fpXlDrawing, "<a:off x=\"2187576\" y=\"2438400\"/>" );
			std::fprintf( fpXlDrawing, "<a:ext cx=\"3000375\" cy=\"1733550\"/>" );
			std::fprintf( fpXlDrawing, "</a:xfrm>" );
			std::fprintf( fpXlDrawing, "<a:prstGeom prst=\"rect\">" );
			std::fprintf( fpXlDrawing, "<a:avLst/>" );
			std::fprintf( fpXlDrawing, "</a:prstGeom>" );
			std::fprintf( fpXlDrawing, "</xdr:spPr>" );

			std::fprintf( fpXlDrawing, "</xdr:pic>" );
			std::fprintf( fpXlDrawing, "<xdr:clientData/>" );
			std::fprintf( fpXlDrawing, "</xdr:twoCellAnchor>" );
		}

		std::fprintf( fpXlDrawing, "</xdr:wsDr>" );

		fclose( fpXlDrawing );
	}
}

int Excel_Workbook::AddStyle( COLORREF cellBg, COLORREF cellFg, double fontSize, BorderParam &rBorderParam, long halign, long valign, bool fontBold )
{
	int colorId = AddFill( cellBg );
	int borderId = AddBorder( rBorderParam );
	int fontId = AddFont( fontSize, cellFg, fontBold );

	for( unsigned int i = 0; i < m_vCellXfs.size(); ++i )
	{
		if( m_vCellXfs[i].fillId == colorId
			&& m_vCellXfs[i].borderTypeId == borderId
			&& m_vCellXfs[i].fontId == fontId
			&& m_vCellXfs[i].halign == halign
			&& m_vCellXfs[i].valign == valign )
		{
			return i + 1;
		}
	}

	CellXfs tmpXfs;
	tmpXfs.fillId = colorId;
	tmpXfs.borderTypeId = borderId;
	tmpXfs.fontId = fontId;
	tmpXfs.halign = halign;
	tmpXfs.valign = valign;
	m_vCellXfs.push_back( tmpXfs );

	return m_vCellXfs.size();
}

int Excel_Workbook::AddFill( DWORD color )
{
	for( unsigned int i = 0; i < m_vColor.size(); ++i )
	{
		if( m_vColor[i] == color )
		{
			return i + 2;
		}
	}

	m_vColor.push_back( color );
	return m_vColor.size() + 1;
}

int Excel_Workbook::AddBorder( BorderParam &rBorderParam )
{
	for( unsigned int i = 0; i < m_vBorderType.size(); ++i )
	{
		if( m_vBorderType[i].wStyleLeft == rBorderParam.wStyleLeft && m_vBorderType[i].wStyleTop == rBorderParam.wStyleTop && m_vBorderType[i].wStyleRight == rBorderParam.wStyleRight 
				&& m_vBorderType[i].wStyleBottom == rBorderParam.wStyleBottom && m_vBorderType[i].clrLeft == rBorderParam.clrLeft && m_vBorderType[i].clrTop == rBorderParam.clrTop
				&& m_vBorderType[i].clrRight == rBorderParam.clrRight && m_vBorderType[i].clrBottom == rBorderParam.clrBottom )
		{
			return i + 1;
		}
	}

	m_vBorderType.push_back( rBorderParam );
	return m_vBorderType.size();
}

int Excel_Workbook::AddFont( double fontSize, COLORREF cellFg, bool bold )
{
	for( unsigned int i = 0; i < m_vFontParam.size(); ++i )
	{
		if( m_vFontParam[i].fontSize == fontSize
			&& m_vFontParam[i].fontColor == cellFg
			&& m_vFontParam[i].bold == bold )
		{
			return i + 1;
		}
	}

	FontParam fp;
	fp.fontSize = fontSize;
	fp.fontColor = cellFg;
	fp.bold = bold;
	m_vFontParam.push_back( fp );
	return m_vFontParam.size();
}

void Excel_Workbook::WriteBorders( BorderParam &rBorderParam, FILE *fpStyles )
{
	WriteBorder( SS_BORDERTYPE_LEFT, rBorderParam.wStyleLeft, rBorderParam.clrLeft, fpStyles );
	WriteBorder( SS_BORDERTYPE_RIGHT, rBorderParam.wStyleRight, rBorderParam.clrRight, fpStyles );
	WriteBorder( SS_BORDERTYPE_TOP, rBorderParam.wStyleTop, rBorderParam.clrTop, fpStyles );
	WriteBorder( SS_BORDERTYPE_BOTTOM, rBorderParam.wStyleBottom, rBorderParam.clrBottom, fpStyles );
}

void Excel_Workbook::WriteBorder( int iType, int iStyle, COLORREF rColor, FILE *fpStyles )
{
	std::string tag = "";
	std::string style = "";

	switch( iType )
	{
		case SS_BORDERTYPE_LEFT:
			tag = "left";
			break;

		case SS_BORDERTYPE_RIGHT:
			tag = "right";
			break;

		case SS_BORDERTYPE_TOP:
			tag = "top";
			break;

		case SS_BORDERTYPE_BOTTOM:
			tag = "bottom";
			break;

		default:
			return;
	}

	switch( iStyle )
	{
		case SS_BORDERSTYLE_SOLID:
			style = "thin";
			break;

		case SS_BORDERSTYLE_FINE_DASH:
			style = "dashed";
			break;

		default:
			style = "thin";
			break;
	}

	if( SS_BORDERTYPE_NONE != iStyle )
	{
		std::fprintf( fpStyles, "<%s style=\"%s\"><color rgb=\"FF%02X%02X%02X\"/></%s>", tag.c_str(), style.c_str(), GetRValue( rColor ),
				GetGValue( rColor ), GetBValue( rColor ), tag.c_str() );
	}
	else
	{
		std::fprintf( fpStyles, "<%s/>", tag.c_str() );
	}
}

string Excel_Workbook::SanitizeXmlString( string xml )
{
	if( xml.empty() == true )
	{
		return xml;
	}

	string buffer;
	for( char c : xml )
	{
		if( IsLegalXmlChar( c ) )
		{
			buffer.push_back( c );
		}
	}

	return buffer;
}

// HYS-1786: Characters not allowed by XML 1.0.
// By looking on the web we can have a list of all the authorized characters but we end up with this code misinterpreted the character '°'.
// For the moment we put the character that we do not accept as unwanted then we will complete if we meet others.
// https://seattlesoftware.wordpress.com/2008/09/11/hexadecimal-value-0-is-an-invalid-character/

bool Excel_Workbook::IsLegalXmlChar( int character )
{
	bool result = true;

	if( character == 0x1F)
	{
		result = false;
	}

	return result;
}

void Excel_Workbook::CreateSheetRows( int maxRows, CSSheet *currentSpread, FILE *fpSheet, int maxCols, std::vector<std::string> &mergeCell )
{
	bool containsPics = false;

	for( int rows = 0; rows < maxRows; rows++ )
	{
		double cHeight = 0.0;
		currentSpread->GetRowHeight( rows + 1, &cHeight );

		// x 1.08 to respect Spread aspect ratio.
		std::fprintf( fpSheet, "<row r=\"%d\" spans=\"1:%d\" ht=\"%g\" customHeight=\"1\">", rows + 1, maxRows + 1, cHeight * 1.08 );

		/************************************************************************/
		/* CELLS                                                                */
		/************************************************************************/
		for( int cols = 0; cols < maxCols; cols++ )
		{
			std::string ref = Excel_Tools::GetRef( rows + 1, cols + 1 );

			int tmpCols = cols;
			int tmpRows = rows;
			ComputeMergeCell( currentSpread, &tmpCols, &tmpRows, mergeCell );

			// *** BORDER TYPE ***
			BorderParam rBorderParam;

			currentSpread->GetBorderEx( cols + 1, rows + 1, &rBorderParam.wStyleLeft, &rBorderParam.clrLeft, &rBorderParam.wStyleTop, &rBorderParam.clrTop, 
					&rBorderParam.wStyleRight, &rBorderParam.clrRight, &rBorderParam.wStyleBottom, &rBorderParam.clrBottom );

			SS_CELLTYPE celltype;
			currentSpread->GetCellType( cols + 1, rows + 1, &celltype );

			COLORREF cellBg;
			COLORREF cellFg;
			currentSpread->GetColor( tmpCols + 1, tmpRows + 1, &cellBg, &cellFg );
			double fontSize = currentSpread->GetFontSize( tmpCols + 1, tmpRows + 1 );

			/*
			#define SS_TEXT_LEFT           0x001 >> 0x001
			#define SS_TEXT_RIGHT          0x002 >> 0x002
			#define SS_TEXT_CENTER         0x004 >> 0x004
			*/
			long halign = celltype.Style & 0x7;

			/*
			#define SS_TEXT_VCENTER        0x008 >> 0x001
			#define SS_TEXT_TOP            0x010 >> 0x002
			#define SS_TEXT_BOTTOM         0x020 >> 0x004
			*/
			long valign = ( celltype.Style >> 3 ) & 0x7;

			bool fontBold = currentSpread->GetFontBold( tmpCols + 1, tmpRows + 1 );

			int cellStyle = AddStyle( cellBg, cellFg, fontSize, rBorderParam, halign, valign, fontBold );


			switch( celltype.Type )
			{
				case SS_TYPE_PICTURE:
				{
					// *** GET PICTURE POSITION ***
					DrawingParam drawPr;
					memset( &drawPr, 0, sizeof( drawPr ) );

					WORD wRet = currentSpread->GetCellSpan( cols + 1,
															rows + 1,
															&drawPr.fromCol,
															&drawPr.fromRow,
															&drawPr.toCol,
															&drawPr.toRow );

					if( wRet == SS_SPAN_NO )
					{
						drawPr.fromCol = drawPr.toCol = cols + 1;
						drawPr.fromRow = drawPr.toRow = rows + 1;
					}
					else
					{
						drawPr.toCol += drawPr.fromCol - 1;
						drawPr.toRow += drawPr.fromRow - 1;
					}

					// *** GET CELL SIZE IN PIXELS ***
					CRect rectCell = currentSpread->GetSelectionInPixels( drawPr.fromCol, drawPr.fromRow, drawPr.toCol, drawPr.toRow );
					drawPr.cellWidth = rectCell.Width();
					drawPr.cellHeight = rectCell.Height();

					CEnBitmap bmpout;

					if( VPS_BMP == ( celltype.Style & VPS_BMP ) )
					{
						HBITMAP h = reinterpret_cast<HBITMAP>( celltype.Spec.ViewPict.hPictName );

						if( false == bmpout.CopyImage( h ) )
						{
							// Some handles are invalid !
							break;
						}
					}
					else if( VPS_ICON == ( celltype.Style & VPS_ICON ) )
					{
						HICON hIcon = reinterpret_cast<HICON>( celltype.Spec.ViewPict.hPictName );
						ICONINFO iconInfo;

						// Retrieve info on the icon.
						if( FALSE == GetIconInfo( hIcon, &iconInfo) )
						{
							break;
						}

						// Retrieve size.
						BITMAP bmp;

						if( GetObject( iconInfo.hbmMask, sizeof( bmp ), &bmp ) != sizeof( bmp ) )
						{
							break;
						}

						HDC hDC = GetDC( NULL );
						
						if( NULL == hDC ) 
						{
							break;
						}

						int nWidth = bmp.bmWidth;
						int nHeight = ( NULL != iconInfo.hbmColor ) ? bmp.bmHeight : bmp.bmHeight / 2;

						HBITMAP hBitmap = CreateCompatibleBitmap( hDC, nWidth, nHeight );

						if( NULL == hBitmap )
						{
							ReleaseDC( NULL, hDC );
							break;
						}

						HDC hMemDC = CreateCompatibleDC( hDC );
						
						if( NULL == hMemDC )
						{
							ReleaseDC( NULL, hDC );
							break;
						}

						// Select the bitmap into the memory device context.
						HBITMAP hOldBitmap = (HBITMAP)SelectObject( hMemDC, hBitmap );
						HBRUSH hBrush = CreateSolidBrush( _WHITE );
						
						if( NULL == hBrush )
						{
							DeleteDC( hMemDC );
							ReleaseDC( NULL, hDC );
							break;
						}

						// Fill the bitmap with the background color.
						HBRUSH hOldBrush = (HBRUSH)SelectObject( hMemDC, hBrush );
						PatBlt( hMemDC, 0, 0, nWidth, nHeight, PATCOPY );

						// Draw the icon.
						DrawIconEx( hMemDC, 0, 0, hIcon, nWidth, nHeight, 0, NULL, DI_NORMAL );

						HBITMAP hBitmap2 = CreateCompatibleBitmap( hDC, nWidth >> 1, nHeight >> 1 );

						if( NULL == hBitmap2 )
						{
							DeleteDC( hMemDC );
							ReleaseDC( NULL, hDC );
							break;
						}

						HDC hMemDC2 = CreateCompatibleDC( hDC );
						
						if( NULL == hMemDC2 )
						{
							DeleteDC( hMemDC );
							ReleaseDC( NULL, hDC );
							break;
						}

						HBITMAP hOldBitmap2 = (HBITMAP)SelectObject( hMemDC2, hBitmap2 );

						BitBlt( hMemDC2, 0, 0, nWidth >> 1, nHeight >> 1, hMemDC, nWidth >> 2, nHeight >> 2, SRCCOPY );

						if( false == bmpout.CopyImage( hBitmap2 ) )
						{
							// Some handles are invalid !
							break;
						}

						// Do cleanup.
						SelectObject( hMemDC, hOldBrush );
						DeleteObject( hBrush );
						SelectObject( hMemDC, hOldBitmap );
						SelectObject( hMemDC2, hOldBitmap2 );
						DeleteObject( iconInfo.hbmColor );
						DeleteObject( iconInfo.hbmMask );

						DeleteDC( hMemDC );
						DeleteDC( hMemDC2 );
						ReleaseDC(NULL, hDC);
					}
					else
					{
						// Don't assert .... just continue without drawing bitmap.
						break;
					}

					m_vPictureCount[m_sheetCount - 1]++;

					TCHAR picturePath[MAX_PATH + 1] = { 0 };
					wcscat_s( picturePath, m_xlmediaPath );
					TCHAR pictureName[32] = { 0 };
					wsprintf( pictureName, _T("\\image%d.png"), PicturesUsed() );
					wcscat_s( picturePath, pictureName );

					// *** GET PICTURE SIZE IN PIXELS ***
					CRect bmpSize = bmpout.GetSizeImage();
					drawPr.pictHeight = bmpSize.Height();
					drawPr.pictWidth = bmpSize.Width();

					// *** KEEP SIZE RATIO ***
					float cellRatio = float( rectCell.Width() ) / rectCell.Height();
					float bmpRatio = float( bmpSize.Width() ) / bmpSize.Height();
					int newWidth = bmpSize.Width() + 1;
					int newHeight = bmpSize.Height() + 1;
					
					long xOffset = 0;
					long yOffset = 0;

					if( bmpRatio > cellRatio )
					{
						newHeight = static_cast<int>( bmpSize.Height() * bmpRatio / cellRatio );
						yOffset = ( newHeight - bmpSize.Height() ) / 2;
						bmpout.ShiftImage( CSize( newWidth, newHeight ), CSize( xOffset, yOffset ) );
					}
					else if( bmpRatio < cellRatio )
					{
						newWidth = static_cast<int>( bmpSize.Width() * cellRatio / bmpRatio );
						xOffset = ( newWidth - bmpSize.Width() ) / 2;
						bmpout.ShiftImage( CSize( newWidth, newHeight ), CSize( xOffset, yOffset ) );
					}

					// *** AVOID TOO MUCH UPSCALING ***
					unsigned int cellSize = rectCell.Width() * rectCell.Height();
					unsigned int imgSize = newWidth * newHeight;

					if( imgSize * 1.1 < cellSize )
					{
						xOffset = ( rectCell.Width() - newWidth ) / 2;
						yOffset = ( rectCell.Height() - newHeight ) / 2;
						bmpout.ShiftImage( CSize( rectCell.Width(), rectCell.Height() ), CSize( xOffset, yOffset ) );
					}

					bmpout.SaveImageToFile( picturePath );

					m_vDrawingParam.push_back( drawPr );
					containsPics = true;
					break;
				}

				default:
					int dataLen = currentSpread->GetDataLen( cols + 1, rows + 1 );
					TCHAR *data = new TCHAR[dataLen + 1];
					currentSpread->GetData( cols + 1, rows + 1, data );

					if( dataLen )
					{
						char *p;
						// HYS-1786: Verify the string
						std::string xmlString = SanitizeXmlString( Excel_Tools::ws2s( data ).c_str() );
						long converted = strtol( xmlString.c_str(), &p, 10 );

						if( *p )
						{
							// Conversion failed because the input wasn't a number. 
							Excel_Tools::EncodeHtml( xmlString );
							std::fprintf( fpSheet, "<c r=\"%s\" s=\"%d\" t=\"inlineStr\"><is><t xml:space=\"preserve\">%s</t></is></c>\n", ref.c_str(), cellStyle, xmlString.c_str() );
						}
						else
						{
							// String is a signed integer.
							std::fprintf( fpSheet, "<c r=\"%s\" s=\"%d\"><v>%s</v></c>\n", ref.c_str(), cellStyle, xmlString.c_str() );
						}
					}
					else
					{
						std::fprintf( fpSheet, "<c r=\"%s\" s=\"%d\"/>\n", ref.c_str(), cellStyle );
					}

					delete[] data;
					break;
			}

		}

		std::fprintf( fpSheet, "</row>\n" );
	}

	if( containsPics )
	{
		CreateXlWorksheetsRels( m_sheetCount );
	}
}

void Excel_Workbook::SaveSheetName( CSSheet *currentSpread, int page )
{
	TCHAR tSheetName[256] = { 0 };
	currentSpread->GetSheetName( page + 1, tSheetName, 256 );

	if( tSheetName[0] )
	{
		m_vSheetName.push_back( tSheetName );
	}
	else
	{
		m_vSheetName.push_back( _T("Sheet") );
	}
}

int Excel_Workbook::PicturesUsed()
{
	short retVal = 0;

	for( unsigned int i = 0; i < m_vPictureCount.size(); ++i )
	{
		retVal += m_vPictureCount[i];
	}

	return retVal;
}

short Excel_Workbook::PicturesStartAt( unsigned short pageZC )
{
	short retVal = 1;

	for( int i = 0; i < pageZC; ++i )
	{
		if( m_vPictureCount.size() >= pageZC )
		{
			retVal += m_vPictureCount[i];
		}
	}

	return retVal;
}

void Excel_Workbook::CreateXlDrawingsRels()
{
	for( int i = 0; i < m_sheetCount; ++i )
	{
		TCHAR fileName[MAX_PATH + 1] = { 0 };
		wsprintf( fileName, _T("%s\\drawing%d.xml.rels"), m_xldrawingsRelsPath, i + 1 );

		FILE *fpXlDrawing;
		errno_t err = _wfopen_s( &fpXlDrawing, fileName, _T("wb") );

		if( 0 != err )
		{
			return;
		}

		std::fprintf( fpXlDrawing, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" );
		std::fprintf( fpXlDrawing, "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">" );

		for( int j = PicturesStartAt( i ); j < m_vPictureCount[i] + PicturesStartAt( i ); ++j )
		{
			int idCount = j + 1 - PicturesStartAt( i );
			std::fprintf( fpXlDrawing, "<Relationship Id=\"rId%d\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" ", idCount );
			std::fprintf( fpXlDrawing, "Target=\"../media/image%d.png\"/>", j );
		}

		std::fprintf( fpXlDrawing, "</Relationships>" );
		fclose( fpXlDrawing );
	}
}

void Excel_Workbook::WriteHAlign( int i, FILE *fpStyles )
{
	std::string halign = "";

	switch( m_vCellXfs[i].halign )
	{
		case 1:
			halign = "left";
			break;

		case 2:
			halign = "right";
			break;

		case 4:
			halign = "center";
			break;

		default:
			halign = "left";
			break;
	}

	std::fprintf( fpStyles, "horizontal=\"%s\" ", halign.c_str() );
}

void Excel_Workbook::WriteVAlign( int i, FILE *fpStyles )
{
	std::string valign = "";

	switch( m_vCellXfs[i].valign )
	{
		case 2:
			valign = "top";
			break;

		case 4:
			valign = "bottom";
			break;

		case 1:
			valign = "center";
			break;

		default:
			valign = "top";
			break;
	}

	std::fprintf( fpStyles, "vertical=\"%s\" ", valign.c_str() );
}
