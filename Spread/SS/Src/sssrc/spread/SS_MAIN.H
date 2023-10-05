/************
* SS_MAIN.H       
************/


//---------*---------*---------*---------*---------*---------*---------*-----
// 
// UPDATE LOG:
//
//    RWP01 10.18.96  <JIS3305> Can't turn off the beep sound
//  
//---------*---------*---------*---------*---------*---------*---------*-----


#ifndef SS_MAIN_H
#define SS_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#define SS_DEFAULT_HSCROLL 8
#define SS_DEFAULT_VSCROLL 20

#define SS_ALLOWSELBLOCK(lpSS) (SS_SELBLOCKOPTION(lpSS) && \
                                lpSS->wOpMode != SS_OPMODE_SINGLESEL)
/*
#define SS_ALLOWSELBLOCK(lpSS) (lpSS->lpBook->wSelBlockOption && \
                                lpSS->wOpMode != SS_OPMODE_SINGLESEL)
*/

#define SS_CMD_COPY 0
#define SS_CMD_SWAP 1
#define SS_CMD_MOVE 2

/******************************
* Declare function prototypes
******************************/

extern BOOL      SS_SetSheetCount(LPSS_BOOK lpBook, short nSheetCnt);
extern BOOL      SS_InsertSheets(LPSS_BOOK lpBook, short nSheetIndex, short nSheetCnt);
extern BOOL      SS_DeleteSheets(LPSS_BOOK lpBook, short nSheetIndex, short nSheetCnt);
extern BOOL      SS_MoveSheets(LPSS_BOOK lpBook, short nSheetSrc, short nSheetCnt, short nSheetDest);
extern BOOL      SS_InitSheetPreHwnd(LPSS_BOOK lpBook, TBGLOBALHANDLE hSS, short nSheetIndex, BOOL bInitCalcInfo);
extern void      SS_GetLargestExpandingEditCell(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType,
                                                HWND hWndCtl, SS_COORD Col, SS_COORD Row,
                                                LPTSTR lpszText, long lLen, BOOL fBegin,
                                                LPRECT lpRectOut);
extern int       SS_GetColWidthInPixels(LPSPREADSHEET lpSS, SS_COORD Col);
extern int       SS_GetColWidthInPixelsEx(LPSPREADSHEET lpSS, SS_COORD Col,
                                          LPSSX_PRINTFORMAT lpPFormat);
extern double    SS_GetColWidthInPixelsExDF(LPSPREADSHEET lpSS, SS_COORD Col,
                                            LPSSX_PRINTFORMAT lpPFormat);
extern long      SS_ColWidthToPixels(LPSPREADSHEET lpSS,
                                     double ColWidth);
//extern int       SS_ColWidthToPixels(LPSPREADSHEET lpSS,
//                                     double ColWidth);
extern int       SS_GetRowHeightInPixels(LPSPREADSHEET lpSS, SS_COORD Row);
extern int       SS_GetRowHeightInPixelsEx(LPSPREADSHEET lpSS, SS_COORD Row,
                                          LPSSX_PRINTFORMAT lpPFormat);
extern int       SS_RowHeightToPixels(LPSPREADSHEET lpSS, SS_COORD Row,
                                      double RowHeight);
extern void      SS_CalcPixelsToColWidth(LPSPREADSHEET lpSS,
                                         SS_COORD Col, int PixelWidth,
                                         LPDOUBLE lpdfColWidth);
extern void      SS_CalcPixelsToRowHeight(LPSPREADSHEET lpSS,
                                          SS_COORD Row, int PixelHeight,
                                          LPDOUBLE lpdfRowHeight);
extern BOOL      SS_HighlightCell(LPSPREADSHEET lpSS, BOOL Highlight);
extern BOOL      SSx_HighlightCell(LPSPREADSHEET lpSS, HDC hDC, BOOL Highlight);
extern void      SS_GetScrollArea(LPSPREADSHEET lpSS, LPRECT lpRect,
                                  short ScrollArea);
extern void      SS_InvalidateScrollArea(LPSPREADSHEET lpSS, short ScrollArea);
extern void      SS_InvalidateCell(LPSPREADSHEET lpSS,
                                   SS_COORD CellCol, SS_COORD CellRow);
extern void      SS_InvalidateRow(LPSPREADSHEET lpSS, SS_COORD Row);
extern void      SS_InvalidateCol(LPSPREADSHEET lpSS, SS_COORD Col);
extern SS_COORD  SS_GetBottomVisCell(LPSPREADSHEET lpSS, SS_COORD RowTop);
extern SS_COORD  SS_GetRightVisCell(LPSPREADSHEET lpSS, SS_COORD ColLeft);
SS_COORD         SS_GetRightCell(LPSPREADSHEET lpSS, SS_COORD ColLeft);
extern void      SS_SetHScrollBar(LPSPREADSHEET lpSS);
extern void      SSx_SetHScrollBar(LPSPREADSHEET lpSS, LPRECT lpRect);
extern void      SS_SetVScrollBar(LPSPREADSHEET lpSS);
extern void      SSx_SetVScrollBar(LPSPREADSHEET lpSS, LPRECT lpRect);
extern BOOL      SS_ActivateCell(LPSPREADSHEET lpSS,
                                 SS_COORD Col, SS_COORD Row, BOOL Show);
extern BOOL      SS_ResetBlock(LPSPREADSHEET lpSS);
extern BOOL      SSx_ResetBlock(LPSPREADSHEET lpSS, BOOL fDraw);

extern BOOL      SS_RegisterEdit(HANDLE hInstance);
extern BOOL      SS_UnRegisterEdit(HANDLE hInstance);
extern BOOL      SS_RegisterEditHScroll(HANDLE hInstance);
extern BOOL      SS_RegisterPic(HANDLE hInstance);
extern BOOL      SS_UnRegisterPic(HANDLE hInstance);
extern BOOL      SS_RegisterDate(HANDLE hInstance);
extern BOOL      SS_UnRegisterDate(HANDLE hInstance);
extern BOOL      SS_RegisterTime(HANDLE hInstance);
extern BOOL      SS_UnRegisterTime(HANDLE hInstance);
extern BOOL      SS_RegisterInteger(HANDLE hInstance);
extern BOOL      SS_UnRegisterInteger(HANDLE hInstance);
extern BOOL      SS_RegisterFloat(HANDLE hInstance);
extern BOOL      SS_UnRegisterFloat(HANDLE hInstance);
#ifdef SS_V40
extern BOOL      SS_RegisterCurrency(HANDLE hInstance);
extern BOOL      SS_UnRegisterCurrency(HANDLE hInstance);
extern BOOL      SS_RegisterNumber(HANDLE hInstance);
extern BOOL      SS_UnRegisterNumber(HANDLE hInstance);
extern BOOL      SS_RegisterPercent(HANDLE hInstance);
extern BOOL      SS_UnRegisterPercent(HANDLE hInstance);
#endif // SS_V40
#ifdef SS_V70
extern BOOL      SS_RegisterScientific(HANDLE hInstance);
extern BOOL      SS_UnRegisterScientific(HANDLE hInstance);
#endif // SS_V70
extern BOOL      SS_RegisterButton(HANDLE hInstance);
extern BOOL      SS_UnRegisterButton(HANDLE hInstance);
extern BOOL      SS_RegisterCheckBox(HANDLE hInstance);
extern BOOL      SS_UnRegisterCheckBox(HANDLE hInstance);
extern BOOL      SS_RegisterComboBox(HANDLE hInstance);
extern BOOL      SS_UnRegisterComboBox(HANDLE hInstance);
extern BOOL      tbSSRegisterComboBox(HANDLE hInstance);
extern BOOL      tbSSUnRegisterComboBox(HANDLE hInstance);
extern BOOL      SS_RegisterListBox(HANDLE hInstance);
extern BOOL      RegisterScrlBar(HANDLE hInstance);
extern BOOL      UnRegisterScrlBar(HANDLE hInstance);

extern BOOL      SS_GetLock(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, BOOL fIgnoreMergedCells);
extern BOOL      SSx_GetLock(LPSPREADSHEET lpSS, LPSS_COL lpCol, LPSS_ROW lpRow,
                             LPSS_CELL lpCell, SS_COORD Col, SS_COORD Row, BOOL fIgnoreMergedCells);
extern BOOL      SS_IsBlockSelected(LPSPREADSHEET lpSS);
extern BOOL      SS_IsAllSelected(LPSPREADSHEET lpSS);
extern BOOL      SS_IsRowsSelected(LPSPREADSHEET lpSS);
extern BOOL      SS_IsColsSelected(LPSPREADSHEET lpSS);
extern void      SSx_SelectBlock(LPSPREADSHEET lpSS,
                                 SS_COORD Col, SS_COORD Row);

extern void      SS_GetClientRect(LPSS_BOOK lpBook, LPRECT lpRect);
extern BOOL      SS_AutoSize(LPSS_BOOK lpBook, BOOL fWM_Size);
extern BOOL      SS_GetCellFromPixel(LPSPREADSHEET lpSS,
                                     LPSS_COORD lpCol, LPSS_COORD lpRow,
                                     LPINT lpX, LPINT lpY,
                                     int MouseX, int MouseY);

extern void      SS_GetScientificDecimal(LPSS_CELLTYPE lpCellType, LPTSTR lpszDecimal);
extern void      SSx_GetScientificDecimal(LPTSTR lpszDecimalIn, LPTSTR lpszDecimalOut);
extern BOOL      SS_ScientificCheckRange(LPSS_CELLTYPE lpCellType, LPTSTR lpszText, BOOL fEditMode);
extern BOOL      SS_EditIsValidKey(LPSS_BOOK lpBook, LPSS_CELLTYPE CellType, short Key);
extern void      SS_ProcessEnterKey(LPSPREADSHEET lpSS);
extern void      SS_Beep(LPSS_BOOK lpBook);
LRESULT  SS_SendMessageToVB(HWND hWndSS, UINT Msg, WPARAM wParam,
                            LPARAM lParam);

#ifdef SS_V35
extern short SS_GetCellNoteIndicator(LPSS_BOOK lpBook);
extern void SS_SetCellNoteIndicator(LPSS_BOOK lpBook, short nNewValue);
extern BOOL SS_IsFetchCellNote(LPSS_BOOK lpBook);

BOOL SS_GetTypeEllipses(LPSPREADSHEET lpSS);
void SS_SetTypeEllipses(LPSPREADSHEET lpSS, BOOL bShowEllipses);
#endif

//RWP01a
void       SS_Beep2(HWND hWnd);
//RWP01a

#ifdef  BUGS
// Charles
extern void      SS_BeepLowlevel(HWND hwnd);
extern BOOL      GetSpreadAutoFill(HWND hWnd);
#endif

extern LRESULT   SS_DefWindowProc(FARPROC lpfnProc, LPSPREADSHEET lpSS, HWND hWndSS,
                                  HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

BOOL             SS_ClipIn(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                           SS_COORD Col2, SS_COORD Row2, HPCTSTR Buffer,
                           long BufferLen, BOOL fValue, BOOL fValidate,
                           BOOL fPaste, LPSS_COORD lpColCnt,
                           LPSS_COORD lpRowCnt);
GLOBALHANDLE     SS_ClipOut(LPSPREADSHEET lpSS,
                            SS_COORD Col, SS_COORD Row,
                            SS_COORD Col2, SS_COORD Row2,
                            BOOL fValue, BOOL fClipboard);
BOOL             SS_ClearDataRange(LPSPREADSHEET lpSS,
                                   SS_COORD Col, SS_COORD Row,
                                   SS_COORD Col2, SS_COORD Row2,
                                   BOOL fIgnoreLockedCells,
                                   BOOL fSendDataChange);
BOOL             SS_MultiSelBlockAddItem(LPSPREADSHEET lpSS,
                                         LPSS_CELLCOORD lpUL,
                                         LPSS_CELLCOORD lpLR);
WINENTRY SS_DeskTopProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

void             SS_SetBlock(LPSS_SELBLOCK lpBlock, SS_COORD Col1,
                             SS_COORD Row1, SS_COORD Col2, SS_COORD Row2);
BOOL             SS_IntersectBlock(LPSS_SELBLOCK lpBlock, LPSS_SELBLOCK lpSrc1,
                                   LPSS_SELBLOCK lpSrc2);
long             SS_KeyDown(LPSPREADSHEET lpSS, WPARAM wParam, LPARAM lParam, BOOL fSendKeyDownEvent);
BOOL             SS_BeginMultipleSelection(HWND hWnd, LPSPREADSHEET lpSS);
HRGN             SSxx_SelectBlock(LPSPREADSHEET lpSS, HRGN hRgnMain,
                                  LPRECT lpRectMain, short dItemCnt);
void             SSx_SetRowHeaderCols(LPSPREADSHEET lpSS, SS_COORD lHeaderCnt);
void             SSx_SetColHeaderRows(LPSPREADSHEET lpSS, SS_COORD lHeaderCnt);
BOOL             SS_GetActualCell(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPSS_COORD lpCol,
                                  LPSS_COORD lpRow, LPRECT lpRect);
BOOL             SS_GetActualCellEx(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                                    LPSS_CELLRANGE lpCellRange, LPRECT lpRect);

// SS_COPY.C

BOOL             SS_MoveRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                              SS_COORD Col2, SS_COORD Row2, SS_COORD ColDest,
                              SS_COORD RowDest);
BOOL             SS_CopySwapRange(LPSPREADSHEET lpSS,
                                  SS_COORD Col, SS_COORD Row,
                                  SS_COORD Col2, SS_COORD Row2,
                                  SS_COORD ColDest, SS_COORD RowDest,
                                  WORD wCmd, BOOL fSendChangeMsg, BOOL fAdjustDataCnt);

// SS_USER.C

BOOL             SS_ClearRange(LPSPREADSHEET lpSS,
                               SS_COORD Col, SS_COORD Row,
                               SS_COORD Col2, SS_COORD Row2,
                               BOOL fSendChangeMsg);

// SS_DATA.C

BOOL SS_SetDataRange(LPSPREADSHEET lpSS,
                     SS_COORD Col, SS_COORD Row,
                     SS_COORD Col2, SS_COORD Row2,
                     LPCTSTR lpData, BOOL fValue,
                     BOOL fIgnoreLockedCells,
                     BOOL fCheckLen);

// SS_SORT.C

void SS_SortDestroy();
BOOL SS_SortEx(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, SS_COORD Col2,
               SS_COORD Row2, WORD wSortBy, LPSS_SORTKEY lpSortKeys,
               short nSortKeyCnt);

// SS_SRCH.C
SS_COORD SS_SearchCol(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRowStart, SS_COORD lRowEnd,
                      LPCTSTR lpszText, WORD wSearchFlags);
SS_COORD SS_SearchRow(LPSPREADSHEET lpSS, SS_COORD lRow, SS_COORD lColStart, SS_COORD lColEnd,
                      LPCTSTR lpszText, WORD wSearchFlags);

TBGLOBALHANDLE SS_CreatePreHwnd(COLORREF crForegrnd, COLORREF crBackgrnd,
                                HFONT hFont);
void SS_CreateWithHwnd(HWND hWnd, TBGLOBALHANDLE hBook);

void SS_DestroyWithHwnd(HWND hWnd);
void SS_DestroyPostHwnd(TBGLOBALHANDLE hBook);

int  SS_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
void SS_OnDestroy(HWND hWnd);

#ifndef SS_OLDCALC
short CALLBACK SS_CalcGetSheetCnt(HANDLE hBook);
LPCALC_SHEET CALLBACK SS_CalcGetSheetFromIndex(HANDLE hBook, short nIndex);
LPCALC_SHEET CALLBACK SS_CalcGetSheetFromName(HANDLE hBook, LPTSTR lpszText, int nLen);
void CALLBACK SS_CalcSendCircularFormulaMsg(HANDLE hBook, short nSheet, SS_COORD lCol, SS_COORD lRow);
int CALLBACK SS_CalcGetIndex(HANDLE hSS);
int CALLBACK SS_CalcGetName(HANDLE hSS, LPTSTR lpszText, int nLen);
int CALLBACK SS_CalcGetNameLen(HANDLE hSS);
BOOL CALLBACK SS_CalcGetData(HANDLE hSS, SS_COORD lCol, SS_COORD lRow, LPCALC_VALUE lpValue);
BOOL CALLBACK SS_CalcSetData(HANDLE hSS, SS_COORD lCol, SS_COORD lRow, LPCALC_VALUE lpValue);
BOOL CALLBACK SS_CalcGetDataCnt(HANDLE hSS, LPLONG lplColCnt, LPLONG lplRowCnt);
long CALLBACK SS_CalcGetMaxCols(HANDLE hSS);
long CALLBACK SS_CalcGetMaxRows(HANDLE hSS);
LRESULT CALLBACK SS_CalcGetCustRef(HANDLE hSS);
void CALLBACK SS_CalcSetSending(HANDLE hSS, long lCol, long lRow);
#endif // SS_OLDCALC

BOOL SS_SetPrintHeader(LPSPREADSHEET lpSS, LPTSTR lpszText);
BOOL SS_SetPrintFooter(LPSPREADSHEET lpSS, LPTSTR lpszText);

#if SS_V80
int  SS_FindSortKey(SS_SORTKEY[], int keyCnt, SS_COORD Reference);
void SS_InvalidateActiveHeaders(LPSPREADSHEET lpSS);
void SS_InvalidateColHeaders(LPSPREADSHEET lpSS, SS_COORD ColLeft, SS_COORD ColRight);
void SS_InvalidateRowHeaders(LPSPREADSHEET lpSS, SS_COORD RowTop, SS_COORD RowBottom);
#endif

#ifdef __cplusplus
}
#endif

#endif
