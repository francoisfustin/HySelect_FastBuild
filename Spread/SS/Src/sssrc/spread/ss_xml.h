#ifdef SS_V35
	#if !defined(SS_XML_H)
		#define SS_XML_H

		#define TAB2	_T("\t\t<")
		#define	CLOSECR	_T(">\n")
		#define TABROW	_T("\t<Row>\n")
		#define SPREAD  _T("<Spread>\n")
		#define MAXLEN_TAGS	100
		#define MAXNUM_TAGS 512

		typedef struct tagXML
		{
			GLOBALHANDLE ghBuf;
			BOOL bColHeaderDisplay;
			BOOL bRowHeaderDisplay;
			LPTSTR pszBuf;
			long lFlags;
			long colcnt;
			long rowcnt;
			BOOL bFirstPass;
			FILE *fp;
			int  nBufSize;
// fix for bug 10020 -scl
//			TCHAR tcBuf[BUFSIZ +1];
			TCHAR tcBuf[BUFSIZ*10 +1];
			TCHAR root[MAXLEN_TAGS];
			TCHAR collection[MAXLEN_TAGS];
			LPTSTR pPath;
			LPTSTR pXMLName;
			LPTSTR pszLogFile;
			GLOBALHANDLE ghLogFile;
			int	iNumTags;
			BOOL bRowRange;
			BOOL bColRange;

			struct _Tags
			{	LPTSTR pDataTags;
			} Tags[MAXNUM_TAGS];

		} SS_XML;

		// Coordinates for the range.
		//
		typedef struct tagWEBCOORD
		{
			SS_COORD	lColStart;
			SS_COORD	lRowStart;
			SS_COORD	lColEnd;
			SS_COORD	lRowEnd;
			BOOL		bEntireSheet;
			BOOL		bRowRange;
			BOOL		bColRange;
			BOOL		bFlag;

		} WEB_COORD, *PWEB_COORD;

		#ifdef __cplusplus
		extern "C" {
		#endif

		TCHAR *getdatetimestring (TCHAR *);
		BOOL SS_ExportRangeToXML (LPSPREADSHEET, SS_COORD, SS_COORD, SS_COORD, SS_COORD, LPCTSTR, LPCTSTR, LPCTSTR, long, LPCTSTR);
		HGLOBAL SS_ExportRangeToXMLBuffer (LPSPREADSHEET, SS_COORD, SS_COORD, SS_COORD, SS_COORD, LPCTSTR, LPCTSTR, long, LPCTSTR);
		BOOL gettypeof (LPSPREADSHEET, SS_COORD, SS_COORD, BYTE, LONG);
		BOOL dontshowpassword (LPSPREADSHEET, SS_COORD, SS_COORD, LPTSTR);
      LPTSTR ValidateFileName (LPCTSTR lpcFileName, TCHAR *tcPath, LPCTSTR lpcExt);

		#ifdef __cplusplus
		}
		#endif
	#endif //SS_XML_H
#endif // SS_V35
