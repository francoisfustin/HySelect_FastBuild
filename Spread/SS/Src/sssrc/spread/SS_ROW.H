/***********
* SS_ROW.H
***********/

#ifndef SS_ROW_H
#define SS_ROW_H

#ifdef __cplusplus
extern "C" {
#endif

#define SS_GetRowCnt(lpSS)  (lpSS->Row.Max < 0 ? SS_MAXCOORD : lpSS->Row.Max + lpSS->Row.HeaderCnt)

/******************************
* Declare function prototypes
******************************/

BOOL SS_InsRowRange(LPSPREADSHEET lpSS, SS_COORD Row, SS_COORD Row2);
BOOL SS_SetRowLock(LPSPREADSHEET lpSS, SS_COORD Row, BOOL Lock);
BOOL SS_ClearRow(LPSPREADSHEET lpSS, SS_COORD Row, BOOL Paint,
                 BOOL fSendChangeMsg);
BOOL SS_IsRowVisible(LPSPREADSHEET lpSS, SS_COORD Row, short Visible);
void SS_SetDataRowCnt(LPSPREADSHEET lpSS, SS_COORD RowCnt);
BOOL SSxDelRowRange(LPSPREADSHEET lpSS, SS_COORD Row, SS_COORD Row2);
BOOL SS_DelRowRange(LPSPREADSHEET lpSS, SS_COORD Row, SS_COORD Row2);
/*
SS_COORD SS_GetRowsMax(LPSPREADSHEET lpSS);
*/
BOOL SS_SetRowFont(LPSPREADSHEET lpSS, SS_COORD Row, HFONT hFont,
                   BOOL DeleteFont, LPBOOL lpfDeleteFontObject);
#if SS_V80
BOOL SS_SetRowLogFont(LPSPREADSHEET lpSS, SS_COORD Row, LOGFONT *pLogFont);
#endif
BOOL SS_SetRowColor(LPSPREADSHEET lpSS, SS_COORD Row,
                    COLORREF Background, COLORREF Foreground);
BOOL SSIsRowVisible(HWND hWnd, SS_COORD Row, short Visible);
void SS_AdjustDataRowCnt(LPSPREADSHEET lpSS, SS_COORD Row, SS_COORD Row2);

BOOL SS_GetMaxTextRowHeight(LPSPREADSHEET lpSS, SS_COORD Row,
                            LPDOUBLE lpdfRowHeight);

BOOL SS_SetRowCellType(LPSPREADSHEET lpSS, SS_COORD Row,
                       LPSS_CELLTYPE CellType);

BOOL SS_SetRowHeight(LPSPREADSHEET lpSS, SS_COORD Row, double dfHeight);
BOOL SSx_SetRowHeight(LPSPREADSHEET lpSS, SS_COORD Row, double dfHeight, BOOL fUserAction);

BOOL SS_GetRowPageBreak(LPSPREADSHEET lpSS, SS_COORD Row);
BOOL SS_SetRowPageBreak(LPSPREADSHEET lpSS, SS_COORD Row, BOOL fPageBreak);
#if defined(_WIN64) || defined(_IA64)
BOOL SS_GetRowUserData(LPSPREADSHEET lpSS, SS_COORD Row, LONG_PTR* lplUserData);
BOOL SS_SetRowUserData(LPSPREADSHEET lpSS, SS_COORD Row, LONG_PTR lUserData);
#else
BOOL SS_GetRowUserData(LPSPREADSHEET lpSS, SS_COORD Row, LPLONG lplUserData);
BOOL SS_SetRowUserData(LPSPREADSHEET lpSS, SS_COORD Row, long lUserData);
#endif

#ifdef SS_V40
short SS_GetRowMerge(LPSPREADSHEET lpSS, SS_COORD Row);
void SS_SetRowMerge(LPSPREADSHEET lpSS, SS_COORD Row, short nRowMerge);
#endif // SS_V40
void SS_UnhideActiveRow(LPSPREADSHEET lpSS, SS_COORD Row);

#ifdef __cplusplus
}
#endif

#endif
