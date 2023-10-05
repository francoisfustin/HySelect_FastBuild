/***********
* SS_COL.H
***********/

#ifndef SS_COL_H
#define SS_COL_H

#ifdef __cplusplus
extern "C" {
#endif

#define SS_GetColCnt(lpSS) (lpSS->Col.Max < 0 ? SS_MAXCOORD : lpSS->Col.Max + lpSS->Col.HeaderCnt)

/******************************
* Declare function prototypes
******************************/

BOOL SS_ClearCol(LPSPREADSHEET lpSS, SS_COORD Col, BOOL Paint,
                 BOOL fSendChangeMsg);
BOOL SS_IsColVisible(LPSPREADSHEET lpSS, SS_COORD Col, short Visible);
BOOL SSxDelColRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Col2);
BOOL SS_DelColRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Col2);
BOOL SS_InsCol(HWND hwnd, SS_COORD Col, BOOL Paint);
void SS_SetDataColCnt(LPSPREADSHEET lpSS, SS_COORD ColCnt);
BOOL SS_SetColLock(LPSPREADSHEET lpSS, SS_COORD Col, BOOL Lock);
BOOL SS_SetColFont(LPSPREADSHEET lpSS, SS_COORD Col, HFONT hFont,
                   BOOL DeleteFont, LPBOOL lpfDeleteFontObject);
#if SS_V80
BOOL SS_SetColLogFont(LPSPREADSHEET lpSS, SS_COORD Col, LOGFONT *pLogFont);
#endif
BOOL SS_SetColColor(LPSPREADSHEET lpSS, SS_COORD Col,
                    COLORREF Background, COLORREF Foreground);
BOOL SSIsColVisible(HWND hWnd, SS_COORD Col, short Visible);
void SS_AdjustDataColCnt(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Col2);

BOOL SS_SetColCellType(LPSPREADSHEET lpSS, SS_COORD Col,
                       LPSS_CELLTYPE CellType);
BOOL SS_SetColWidthInPixels(LPSPREADSHEET lpSS, SS_COORD Col, int dWidth);
BOOL SS_SetColWidth(LPSPREADSHEET lpSS, SS_COORD Col, double dfWidth);
BOOL SSx_SetColWidth(LPSPREADSHEET lpSS, SS_COORD Col, double dfWidth, BOOL fUserAction);
BOOL SS_SetColPageBreak(LPSPREADSHEET lpSS, SS_COORD Col, BOOL fPageBreak);
BOOL SS_GetColPageBreak(LPSPREADSHEET lpSS, SS_COORD Col);
BOOL SS_InsColRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Col2);

BOOL SS_GetMaxTextColWidth(LPSPREADSHEET lpSS, SS_COORD Col,
                           LPDOUBLE lpdfColWidth);
#if defined(_WIN64) || defined(_IA64)
BOOL SS_GetColUserData(LPSPREADSHEET lpSS, SS_COORD Col, LONG_PTR* lplUserData);
BOOL SS_SetColUserData(LPSPREADSHEET lpSS, SS_COORD Col, LONG_PTR lUserData);
#else
BOOL SS_GetColUserData(LPSPREADSHEET lpSS, SS_COORD Col, LPLONG lplUserData);
BOOL SS_SetColUserData(LPSPREADSHEET lpSS, SS_COORD Col, long lUserData);
#endif
BOOL SS_SetColDataFillEvent(LPSPREADSHEET lpSS, SS_COORD Col, BYTE bDataFillEvent);
BYTE SS_GetColDataFillEvent(LPSPREADSHEET lpSS, SS_COORD Col);

#ifdef SS_V35

short SS_GetMergedColUserSortIndicator(LPSPREADSHEET lpSS, SS_COORD Col);

short SS_GetColUserSortIndicator(LPSPREADSHEET lpSS, SS_COORD Col);
void  SS_SetColUserSortIndicator(LPSPREADSHEET lpSS, SS_COORD Col, short nColUserSortIndicator);

#endif // SS_V35

#ifdef SS_V40
short SS_GetColMerge(LPSPREADSHEET lpSS, SS_COORD Col);
void SS_SetColMerge(LPSPREADSHEET lpSS, SS_COORD Col, short nColMerge);
short SS_GetColID(LPSPREADSHEET lpSS, SS_COORD Col, LPTSTR ColID);
BOOL SS_SetColID(LPSPREADSHEET lpSS, SS_COORD Col, LPCTSTR ColID);
SS_COORD SS_GetColFromID(LPSPREADSHEET lpSS, LPCTSTR lpszColID);
#endif // SS_V40
void SS_UnhideActiveCol(LPSPREADSHEET lpSS, SS_COORD Col);

#ifdef __cplusplus
}
#endif

#endif
