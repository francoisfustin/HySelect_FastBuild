#ifdef SS_HTML_H
	#ifdef __cplusplus
		extern "C" {
	#endif
#if !defined(SS_V80) && !defined(SS_V70)
	#include "..\..\..\..\fplibs\fptools\jpeg\jpeglib.h"
#endif
	#ifdef __cplusplus
		}
	#endif
#endif

// jb 10.mar.00
//
// All of the items needed for ss_html.c for spread's html export.
//
//
#ifdef STRINGS_RESOURCE

#define	IDS_LOGS_BADINDEX				2000
#define	IDS_LOGS_COL1INVALID			2001
#define	IDS_LOGS_COL2INVALID			2002
#define	IDS_LOGS_ROW1INVALID			2003
#define	IDS_LOGS_ROW2INVALID			2004
#define	IDS_LOGS_DIRECTORYNOTFOUND		2005
#define IDS_LOGS_INVALIDFILE			2006
#define	IDS_LOGS_COLRANGEERROR			2007
#define	IDS_LOGS_ROWRANGEERROR			2008
#define	IDS_LOGS_CANTOPENFILE			2009
#define	IDS_LOGS_FILESAVED				2010
#define	IDS_LOGS_FILENOTFOUND			2011
#define	IDS_LOGS_SUCCESSEXPORT			2012
#define	IDS_LOGS_FAILEDEXPORT			2013
#define	IDS_LOGS_EXCEEDEDMAXCOL			2014
#define	IDS_LOGS_EXCEEDEDMAXROW			2015
#define	IDS_LOGS_COLHIDDEN				2016
#define	IDS_LOGS_ROWHIDDEN				2017
#define	IDS_LOGS_CHECKBOX				2018
#define	IDS_LOGS_COMBOBOX				2019
#define	IDS_LOGS_OWNERDRAWN				2020
#define	IDS_LOGS_BMP_PICTURE			2021
#define	IDS_LOGS_ICON_PICTURE			2022
#define	IDS_LOGS_JPEG_FAILURE			2023
#define	IDS_LOGS_BMP_FAILURE			2024
#define	IDS_LOGS_ICON_FAILURE			2025
#define	IDS_LOGS_INVALIDFILETYPE		2026
#define	IDS_LOGS_BUTTON					2027
#define	IDS_LOGS_UNKNOWN				2028
#define	IDS_LOGS_FORMULA				2029
#define	IDS_LOGS_HEAD1_HTML				2030
#define	IDS_LOGS_HEAD3	   				2031
#define	IDS_LOGS_HEAD4	   				2032
#define	IDS_LOGS_FOOTER_HTML			2033

#ifdef SS_V35
#define	IDS_LOGS_TEXT_NOCOLDELIMITER	2034
#define	IDS_LOGS_TEXT_NOROWDELIMITER	2035
#define	IDS_LOGS_TEXT_UNFORMATCREATE	2036
#define	IDS_LOGS_TEXT_UNFORMATAPPEND	2037
#define	IDS_LOGS_TEXT_CREATE			2038
#define	IDS_LOGS_TEXT_APPEND			2039
#define	IDS_LOGS_TEXT_UNKNOWNFLAG		2040
#define	IDS_LOGS_NODATA			 		2041
#define	IDS_LOGS_NOXMLTAGS		 		2042
#define	IDS_LOGS_CANTOPENFILE_R			2043
#define	IDS_LOGS_SUCCESSEXPORT_TEXT		2044
#define	IDS_LOGS_SUCCESSEXPORT_XML		2045
#define	IDS_LOGS_SUCCESS_LOADTEXT		2046
#define IDS_LOGS_SUCCESSBUFF			2047
#define IDS_LOGS_FAILEDBUFF				2048
#define	IDS_LOGS_FAILEDLOAD	   			2049
#define	IDS_LOGS_CANTALLOCBUFFER		2050
#define	IDS_LOGS_HEAD1_TEXT				2051
#define	IDS_LOGS_HEAD1_XML 				2052
#define	IDS_LOGS_HEAD1_XMLBUFF			2053
#define	IDS_LOGS_HEAD1_LOAD				2054
#define	IDS_LOGS_FOOTER_TEXT			2055
#define	IDS_LOGS_FOOTER_XML				2056
#define	IDS_LOGS_FOOTER_LOAD			2057
#define	IDS_LOGS_FOOTER_XMLBUFF			2058
#define IDS_LOGS_COLONCHAR        2059

#define IDS_LOGS_PRECEEDEDVTOPROW   2060
#define IDS_LOGS_EXCEEDEDVBOTTOMROW 2061
#endif
#endif

#if !defined _SS_HTML_ && !defined SS_TEXT_H && !defined SS_XML_H
	#define _SS_HTML_	1

	// WIDTHBYTES performs DWORD-aligning of DIB scanlines.  The "bits"
	// parameter is the bit count for the scanline (biWidth * biBitCount),
	// and this macro returns the number of DWORD-aligned bytes needed
	// to hold those bits.

	#define LESSTHAN		60
	#define GREATERTHAN		62
	#define AMPERSAND		38

	#define SS_NOHTMLAPPEND	0
	#define SS_HTMLAPPEND	1

	#define HTMLTAG			_T("</HTML>")
	#define TABLE			_T("</TABLE>")
	#define TAGROW			_T("\t<TR>\n")
	#define TAGROWHEADER	_T("\t<TR>\n\t\t<TH")
	#define TAGROWDATA		_T("\t<TR>\n\t\t<TD")
	#define TAGROWDATA1		_T("\t<TR>\n\t<TD")
	#define TAGDATA3TABS	_T("\t\t\t<TD")
	#define TAGDATA2TABS	_T("\t\t<TD")
	#define TAGHEADERTAB2	_T("\t\t<TH")

	typedef struct tagHTML
	{
		TCHAR tszFaceName[LF_FACESIZE];
		TCHAR tszBGColor[16];
		TCHAR tszFGColor[16];
		COLORREF clrLockBack;
		COLORREF clrLockFore;
		BOOL  fDisplayRowHeaders;
		BOOL  fDisplayColHeaders;
		BOOL  fSpreadSheet;
		BOOL  fNoTable;
		BOOL  fColHeaders;
		BOOL  fSpecifiedRow;
		BOOL  fRowHeaders;
		BOOL  fUpperLeft;
		BOOL  fHeaderRowCell;
		BOOL  fSpecifiedCol;
		BOOL  fHeaderColCell;
		BOOL  fSpecifiedCell;
		BOOL  fRange;
		BOOL  bPictureHits;

		SS_COORD	lColStart;
		SS_COORD	lRowStart;
		SS_COORD	lColEnd;
		SS_COORD	lRowEnd;
		SS_COORD	lCol;
		SS_COORD	lRow;
		TCHAR tcLogFile[FILENAME_MAX];
		TCHAR tcHtmlFile[FILENAME_MAX];
		TCHAR tcPath[_MAX_PATH];

		int	  jpegindex;

		struct _jpeg
		{	
			GLOBALHANDLE ghBuf;
			TCHAR   *tcJpegName;
			SS_COORD col;
			SS_COORD row;



// fix for bug 10020 -scl
//		} jpeg[BUFSIZ];
		} jpeg[BUFSIZ*10];

		BOOL bAppendFlag;
		BOOL bAllowCellOverFlow;
		BOOL bNoBorder;
		BOOL bHeader;
		INT iAlignment;
		INT iSpannedColumn;
		INT iType;
// fix for bug 10020 -scl
//		TCHAR tcBuf[BUFSIZ];
		TCHAR tcBuf[BUFSIZ*10];
		TCHAR tcJpegName[FILENAME_MAX];

#ifdef SS_V40
// two-dimensional array of bools
// indexed by [col,row] indicating which
// cells are part of a cell span
//    LPBOOL pbspanning;  
#endif
	} HTML, FAR *LPHTML;

	#ifdef __cplusplus
		extern "C" {
	#endif

	// SS_HTML.C functions.
	//
	long ssm_Save4Expr(LPSPREADSHEET, SS_COORD, SS_COORD, LPCALC_INFO, CALC_HANDLE, LPTBGLOBALHANDLE, LPLONG);
	void init_filenames (LPSPREADSHEET, LPCTSTR, LPCTSTR, HTML*);
	BOOL init_colandrowcoordinates (LPSPREADSHEET, SS_COORD*, SS_COORD*, SS_COORD*, SS_COORD*, HTML*, LPCTSTR);
	void init_virtualmode (LPSPREADSHEET, SS_COORD*, SS_COORD*);
	LPHTML getdefaultattributes (LPSPREADSHEET, SS_COORD, SS_COORD, SS_COORD, SS_COORD, LPCTSTR, BOOL, LPCTSTR, LPHTML);
	BOOL openfile (HWND, LPHTML, FILE**);
	BOOL startoftable (LPSPREADSHEET, FILE**, HTML*);
	BOOL startcolumnheaders (LPSPREADSHEET, SS_COORD*, SS_COORD, SS_COORD, FILE**, HTML*);
	BOOL isrowhidden (LPSPREADSHEET, SS_COORD, SS_COORD, HTML*);
	BOOL iscolhidden (LPSPREADSHEET, SS_COORD, SS_COORD, HTML*);
	BOOL allcolumnsandrows (LPSPREADSHEET, TCHAR*, SS_COORD*, SS_COORD, FILE**, LPHTML);
	BOOL justrowheaders (LPSPREADSHEET, SS_COORD, SS_COORD, FILE**, TCHAR*, HTML*);
	void displayrowheaders (LPSPREADSHEET, SS_COORD, FILE**, TCHAR*, HTML*);
	BOOL displayspecifiedrow (LPSPREADSHEET, SS_COORD, SS_COORD, SS_COORD, FILE**, TCHAR*, HTML*);
	BOOL displayrangerowheader (LPSPREADSHEET, SS_COORD, SS_COORD*, SS_COORD, FILE**, TCHAR*, HTML*);
	BOOL writehtmltag (LPSPREADSHEET, SS_COORD, SS_COORD, TCHAR*, LPHTML, FILE**);
	void endtable (FILE**, LPHTML);
	void logfilefooter (LPHTML);
	void maincleanup (LPHTML);
	TCHAR *getdatetimestring (TCHAR *timebuf);
  BOOL getheaderdata (LPSPREADSHEET lpSS, SS_COORD coord, BOOL bRowHeader, LPTSTR lptstrBuf);
  BOOL iswebformulacell (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPTSTR lptstrBuf);
  TCHAR *randomname (TCHAR *tcRndName, TCHAR *tcPath, TCHAR *tcExt);
  BOOL look4specialcharacters (LPTSTR lptstrBuf, UINT uiCellDataLen);

	#ifdef __cplusplus
		}
	#endif

#endif

