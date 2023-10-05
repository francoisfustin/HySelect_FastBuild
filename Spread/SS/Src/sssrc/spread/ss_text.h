//GAB 01/08/02 Added for Unicode. This is the Unicode marker.
//This is what Notepad.exe uses to mark Unicode files.
#define BYTE_ORDER_MARK 0xFEFF
#define ADD_OFFSET(lpv, dw)        ((LPVOID)(lpv + dw))

#ifdef SS_V35
	#if !defined(SS_TEXT_H)
		#define SS_TEXT_H

		#define SS_BUFF_ALLOCINC 4096

		typedef struct tagSS_TEXT
		{
			TCHAR   szFile[200];
			LPCTSTR pcszCellDelim;
			LPCTSTR pcszColDelim;
			LPCTSTR pcszRowDelim;
			LPTSTR  pszLogFile;
			GLOBALHANDLE ghLogFile;
			TCHAR   tcBuf[BUFSIZ+1];

			SS_COORD lCol;
			SS_COORD lRow;
			SS_COORD lRowCnt;
			SS_COORD lColCnt;
			SS_COORD lLastCol;
			long	 lFlags;
			BOOL	bLoadText;

		} SS_TEXT;

		// Coordinates for the range.
		//
		typedef struct tagTEXTCOORD
		{
			SS_COORD	lColStart;
			SS_COORD	lRowStart;
			SS_COORD	lColEnd;
			SS_COORD	lRowEnd;
			BOOL		bEntireSheet;
			BOOL		bRowRange;
			BOOL		bColRange;
			BOOL		bFlag;

		} TEXT_COORD, *PTEXT_COORD;

	#ifdef __cplusplus
	extern "C" {
	#endif

	TCHAR *getdatetimestring (TCHAR *);
	void getothercnt (LPSPREADSHEET, SS_COORD *, SS_COORD *);
	BOOL entirespreadsheet (SS_COORD, SS_COORD);
	BOOL allcolumns (SS_COORD, SS_COORD);
	BOOL allrows (SS_COORD, SS_COORD);
	BOOL columnrange (SS_COORD, SS_COORD);
	BOOL rowrange (SS_COORD, SS_COORD);
	//void InitCoords (PWEB_COORD, SS_COORD, SS_COORD, SS_COORD, SS_COORD);
	void virtualmode (LPSPREADSHEET, SS_COORD *, SS_COORD *);
	
	BOOL SS_ExportRangeToTextFile (LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, SS_COORD lCol2, SS_COORD lRow2, LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile, BOOL fUnicode);
	BOOL SS_LoadTextFile (HWND hWnd, LPCTSTR pcszFile, LPCTSTR pcszCellDelim, LPCTSTR pcszColDelim, LPCTSTR pcszRowDelim, long lFlags, LPCTSTR pcszLogFile);

	#ifdef __cplusplus
	}
	#endif

	#endif // SS_TEXT_H
#endif // SS_V35
