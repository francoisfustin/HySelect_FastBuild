/*************
* SS_SCROL.H
*************/

#ifndef SS_SCROL_H
#define SS_SCROL_H

#ifdef __cplusplus
extern "C" {
#endif

/*
#define SS_TopRowChange(hWnd) \
        SS_SendMsg(hWnd, WM_COMMAND, GetWindowWord(hWnd, GWW_ID), \
                   MAKELONG(hWnd, SSN_TOPROWCHANGE));

#define SS_LeftColChange(hWnd) \
        SS_SendMsg(hWnd, WM_COMMAND, GetWindowWord(hWnd, GWW_ID), \
                   MAKELONG(hWnd, SSN_LEFTCOLCHANGE));

#define SS_TopLeftChange(hWnd) \
        SS_SendMsg(hWnd, WM_COMMAND, GetWindowWord(hWnd, GWW_ID), \
                   MAKELONG(hWnd, SSN_TOPLEFTCHANGE));
*/

void SS_TopRowChange(LPSPREADSHEET lpSS);
void SS_LeftColChange(LPSPREADSHEET lpSS);
void SS_TopLeftChange(LPSPREADSHEET lpSS);

short SS_GetCellPosYInRect(LPSPREADSHEET lpSS, SS_COORD RowTop, SS_COORD Row,
                           LPRECT lpRect);
short SS_GetCellPosXInRect(LPSPREADSHEET lpSS, SS_COORD ColLeft, SS_COORD Col,
                           LPRECT lpRect);

//BOOL SS_ScrollEndOfRow(HWND hWnd, BOOL MoveActiveCell, BOOL ScrollColMax);
//BOOL SS_ScrollEndOfCol(HWND hWnd, BOOL MoveActiveCell, BOOL ScrollRowMax);
BOOL SS_ScrollEnd(LPSPREADSHEET lpSS, WORD fFlags);
//BOOL SS_ScrollHomeOfRow(HWND hWnd, BOOL MoveActiveCell);
//BOOL SS_ScrollHomeOfCol(HWND hWnd, BOOL MoveActiveCell);
BOOL SS_ScrollHome(LPSPREADSHEET lpSS, WORD fFlags);
void SS_HScroll(LPSPREADSHEET lpSS, int ScrollInc);
void SS_HScrollCol(LPSPREADSHEET lpSS, SS_COORD Col, BOOL fRight,
                   BOOL fUpdateHeader);
void SS_VScroll(LPSPREADSHEET lpSS, int ScrollInc);
void SS_VScrollRow(LPSPREADSHEET lpSS, SS_COORD Row, BOOL fDown,
                   BOOL fUpdateHeader);
//void SS_ScrollLineLeft(HWND hWnd, BOOL MoveActiveCell);
//void SS_ScrollLineUp(HWND hWnd, BOOL MoveActiveCell);
void SS_ScrollLineUL(LPSPREADSHEET lpSS, WORD fFlags);
//void SS_ScrollLineRight(HWND hWnd, BOOL MoveActiveCell);
//void SS_ScrollLineDown(HWND hWnd, BOOL MoveActiveCell);
void SS_ScrollLineLR(LPSPREADSHEET lpSS, WORD fFlags);
//void SS_ScrollPageLeft(HWND hWnd, BOOL MoveActiveCell);
//void SS_ScrollPageUp(HWND hWnd, BOOL MoveActiveCell);
void SS_ScrollPageUL(LPSPREADSHEET lpSS, WORD fFlags);
//void SS_ScrollPageRight(HWND hWnd, BOOL MoveActiveCell);
//void SS_ScrollPageDown(HWND hWnd, BOOL MoveActiveCell);
void SS_ScrollPageLR(LPSPREADSHEET lpSS, WORD fFlags);
BOOL SS_LeaveCell(LPSPREADSHEET lpSS, SS_COORD ColOld, SS_COORD ColLeftOld,
                  SS_COORD RowOld, SS_COORD RowTopOld, BOOL fKillFocus);
BOOL SS_ShowActiveCell(LPSPREADSHEET lpSS, short ShowWhere);
BOOL SS_ShowCell(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow,
                 short ShowWhere);
void SS_VPos(LPSPREADSHEET lpSS, SS_COORD Pos);
void SS_HPos(LPSPREADSHEET lpSS, SS_COORD Pos);
void SS_Scroll(LPSPREADSHEET lpSS, int xScroll, int yScroll, LPRECT Rect);
void SS_ScrollNextCell(LPSPREADSHEET lpSS);
void SS_ScrollPrevCell(LPSPREADSHEET lpSS);
SS_COORD SS_GetLastPageTopRow(LPSPREADSHEET lpSS);
SS_COORD SS_GetLastPageLeftCol(LPSPREADSHEET lpSS);
void SS_InvalidateRange(LPSPREADSHEET lpSS, SS_COORD Col1, SS_COORD Row1,
                        SS_COORD Col2, SS_COORD Row2);
void SSx_InvalidateCellRange(LPSPREADSHEET lpSS, SS_COORD Col1, SS_COORD Row1,
                             SS_COORD Col2, SS_COORD Row2);
void SS_InvalidateCellRange(LPSPREADSHEET lpSS, SS_COORD Col1, SS_COORD Row1,
                            SS_COORD Col2, SS_COORD Row2);
void SS_InvalidateRowRange(LPSPREADSHEET lpSS, SS_COORD Row1, SS_COORD Row2);
void SS_InvalidateColRange(LPSPREADSHEET lpSS, SS_COORD Col1, SS_COORD Col2);
void SS_InvalidateButtons(LPSPREADSHEET lpSS,
                          SS_COORD ColOld, SS_COORD ColLeftOld,
                          SS_COORD RowOld, SS_COORD RowTopOld);
void SSx_InvalidateButtons(LPSPREADSHEET lpSS,
                           SS_COORD ColLeft, SS_COORD RowTop,
                           SS_COORD Col1, SS_COORD Row1,
                           SS_COORD Col2, SS_COORD Row2);
void SS_InvalidateRowModeButtons(LPSPREADSHEET lpSS,
                                 SS_COORD ColOld, SS_COORD ColLeftOld,
                                 SS_COORD RowOld, SS_COORD RowTopOld);
BOOL SS_GetFirstValidCol(LPSPREADSHEET lpSS, SS_COORD Row, LPSS_COORD lpCol);
BOOL SS_GetFirstValidRow(LPSPREADSHEET lpSS, SS_COORD Col, LPSS_COORD lpRow);

BOOL SS_GetFirstValidCell(LPSPREADSHEET lpSS, LPSS_COORD lpCol,
                          LPSS_COORD lpRow);
BOOL SS_GetLastValidCell(LPSPREADSHEET lpSS, LPSS_COORD lpCol,
                        LPSS_COORD lpRow);
SS_COORD SSx_GetLastPageTopRow(LPSPREADSHEET lpSS, SS_COORD MaxRows);
BOOL SS_IsCellActivatable(LPSPREADSHEET lpSS, SS_COORD ColAt, SS_COORD RowAt);

#ifdef SS_UTP
void  SS_CalcHScrollArrowArea(HWND hWnd, LPSPREADSHEET lpSS);
void  SS_CalcVScrollArrowArea(HWND hWnd, LPSPREADSHEET lpSS);
#endif

#ifdef SS_V35
short SS_GetShowScrollTips(LPSS_BOOK lpBook);
void SS_SetShowScrollTips(LPSS_BOOK lpBook, short nNewValue);
#endif

void  SS_ResetCellSizeList(LPSS_ROWCOL lpRC);

#ifdef __cplusplus
}
#endif

#endif
