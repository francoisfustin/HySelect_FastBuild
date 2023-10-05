#ifndef SS_HTML_H_
	#define SS_HTML_H_

	#ifdef __cplusplus
	extern "C" {
	#endif
	BOOL SS_ExportRangeToHTML (LPSPREADSHEET, SS_COORD, SS_COORD, SS_COORD, SS_COORD, LPCTSTR, BOOL, LPCTSTR, BOOL, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR);
#ifdef SS_V40
#include "..\..\web\src\web.h"
    BOOL IsEntireRowSpanned(LPSPREADSHEET lpSS, SS_COORD lRow, LPHTML lpHtml);
#endif
	#ifdef __cplusplus
	}
	#endif
#endif

