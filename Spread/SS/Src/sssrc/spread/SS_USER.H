/************
* SS_USER.H
************/

#ifndef SS_USER_H
#define SS_USER_H

#ifdef __cplusplus
extern "C" {
#endif

BOOL SS_DeSelectBlock(LPSPREADSHEET lpSS);

BOOL SS_ShowRow(LPSPREADSHEET lpSS, SS_COORD Row, BOOL fShow);
BOOL SS_IsRowHidden(LPSPREADSHEET lpSS, SS_COORD Row);
BOOL SS_ShowCol(LPSPREADSHEET lpSS, SS_COORD Col, BOOL fShow);
BOOL SS_IsColHidden(LPSPREADSHEET lpSS, SS_COORD Col);

void SS_GetAutoSizeVisible(LPSPREADSHEET lpSS, LPSS_COORD lpVisibleCols,
                           LPSS_COORD lpVisibleRows);
void SS_SetAutoSizeVisible(LPSPREADSHEET lpSS, SS_COORD VisibleCols,
                           SS_COORD VisibleRows);

void SS_Reset(LPSS_BOOK lpBook);

BOOL SS_GetCellRect(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                    LPRECT lpRect);

BOOL SS_GetCellType(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                    LPSS_CELLTYPE CellType);
BOOL SS_SetCellTypeRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                         SS_COORD Col2, SS_COORD Row2, LPSS_CELLTYPE CellType);

WORD SS_GetColHeaderDisplay(LPSPREADSHEET lpSS);
WORD SS_SetColHeaderDisplay(LPSPREADSHEET lpSS, WORD wDisplay);

HCURSOR SS_GetCursor(LPSS_BOOK lpBook, WORD wType);
BOOL    SS_SetCursor(LPSS_BOOK lpBook, WORD wType, HCURSOR hCursor, BOOL fDelete);

BOOL SS_GetDataCnt(LPSPREADSHEET lpSS, LPSS_COORD lpColCnt,
                   LPSS_COORD lpRowCnt);

BOOL SSx_GetDefFloatFormat(LPSS_BOOK lpBook, LPFLOATFORMAT lpFloatFormat);
BOOL SS_GetDefFloatFormat(LPSS_BOOK lpBook, LPFLOATFORMAT lpFloatFormat);
BOOL SS_SetDefFloatFormat(LPSS_BOOK lpBook, LPFLOATFORMAT lpFloatFormat);

WORD SS_GetEditEnterAction(LPSS_BOOK lpBook);
WORD SS_SetEditEnterAction(LPSS_BOOK lpBook, WORD wAction);

BOOL SS_SetFontRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                     SS_COORD Col2, SS_COORD Row2,
                     HFONT hFont, BOOL fDeleteFont);
#if SS_V80
BOOL SS_SetLogFontRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, LOGFONT *pLogFont);
#endif
BOOL SS_GetFreeze(LPSPREADSHEET lpSS, LPSS_COORD lpColsFrozen,
                  LPSS_COORD lpRowsFrozen);
BOOL SS_SetFreeze(LPSPREADSHEET lpSS, SS_COORD ColsFrozen,
                  SS_COORD RowsFrozen);

COLORREF SS_GetGridColor(LPSPREADSHEET lpSS);
COLORREF SS_SetGridColor(LPSPREADSHEET lpSS, COLORREF GridColor);

WORD SS_GetGridType(LPSPREADSHEET lpSS);
WORD SS_SetGridType(LPSPREADSHEET lpSS, WORD wGridType);

BOOL SSxx_GetLock(LPSPREADSHEET lpSS, LPSS_COL lpCol, LPSS_ROW lpRow,
                  LPSS_CELL lpCell, SS_COORD Col, SS_COORD Row, BOOL fIgnoreMergedCells);
BOOL SS_SetLockRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                     SS_COORD Col2, SS_COORD Row2, BOOL Lock);

void SS_GetLockColor(LPSPREADSHEET lpSS, LPCOLORREF lpBackground,
                     LPCOLORREF lpForeground);
void SS_SetLockColor(LPSPREADSHEET lpSS, COLORREF Background,
                     COLORREF Foreground);

WORD SS_GetOperationMode(LPSPREADSHEET lpSS);
WORD SS_SetOperationMode(LPSPREADSHEET lpSS, WORD wMode);

WORD SS_GetRowHeaderDisplay(LPSPREADSHEET lpSS);
WORD SS_SetRowHeaderDisplay(LPSPREADSHEET lpSS, WORD wDisplay);

void SS_GetStartingNumbers(LPSPREADSHEET lpSS, LPSS_COORD lpStartingColNumber,
                           LPSS_COORD lpStartingRowNumber);
void SS_SetStartingNumbers(LPSPREADSHEET lpSS, SS_COORD StartingColNumber,
                           SS_COORD StartingRowNumber);

BOOL SS_GetTopLeftCell(LPSPREADSHEET lpSS, LPSS_COORD lpCol,
                       LPSS_COORD lpRow);
BOOL SS_GetBottomRightCell(LPSPREADSHEET lpSS, LPSS_COORD lpCol,
                           LPSS_COORD lpRow);

WORD SS_GetUnitType(LPSS_BOOK lpBook);
WORD SS_SetUnitType(LPSS_BOOK lpBook, WORD wUnitType);

WORD SS_GetUserResize(LPSPREADSHEET lpSS);
WORD SS_SetUserResize(LPSPREADSHEET lpSS, WORD wUserResize);

short SS_GetMergedUserResizeCol(LPSPREADSHEET lpSS, SS_COORD Col);
short SS_GetUserResizeCol(LPSPREADSHEET lpSS, SS_COORD Col);
short SS_SetUserResizeCol(LPSPREADSHEET lpSS, SS_COORD Col, short dOption);

short SS_GetMergedUserResizeRow(LPSPREADSHEET lpSS, SS_COORD Row);
short SS_GetUserResizeRow(LPSPREADSHEET lpSS, SS_COORD Row);
short SS_SetUserResizeRow(LPSPREADSHEET lpSS, SS_COORD Row, short dOption);

BOOL SS_ValidateFormula(LPSPREADSHEET lpSS, LPCTSTR lpszFormula);

void DLLENTRY SS_GetCellSendingMsg(LPSPREADSHEET lpSS, LPSS_COORD lpCol,
                                   LPSS_COORD lpRow);

WORD SS_SetBackColorStyle(LPSS_BOOK lpBook, WORD wStyle);
WORD SS_GetBackColorStyle(LPSS_BOOK lpBook);

BOOL SS_IsActionKey(LPSS_BOOK lpBook, WORD wAction, WORD wKey);

BOOL SS_SetActionKey(LPSS_BOOK lpBook, WORD wAction, BOOL fShift, BOOL fCtrl, WORD wKey);
BOOL SS_GetActionKey(LPSS_BOOK lpBook, WORD wAction, LPBOOL lpfShift, LPBOOL lpfCtrl, LPWORD lpwKey);
BOOL SS_SetOddEvenRowColor(LPSPREADSHEET lpSS, COLORREF clrBackOdd, COLORREF clrForeOdd, COLORREF clrBackEven, COLORREF clrForeEven);
BOOL SS_GetOddEvenRowColor(LPSPREADSHEET lpSS, LPCOLORREF lpclrBackOdd, LPCOLORREF lpclrForeOdd, LPCOLORREF lpclrBackEven, LPCOLORREF lpclrForeEven);
WORD SS_SetAppearance(LPSS_BOOK lpBook, WORD wAppearance);
WORD SS_GetAppearance(LPSS_BOOK lpBook);
void SS_GetSelColor(LPSS_BOOK lpBook, LPCOLORREF lpclrBack, LPCOLORREF lpclrFore);
void SS_SetSelColor(LPSS_BOOK lpBook, COLORREF clrBack, COLORREF clrFore);
BOOL SS_SetArray(LPSPREADSHEET lpSS, LPVOID lpArray, SS_COORD ColLeft,
                 SS_COORD RowTop, SS_COORD ArrayColCnt,
                 SS_COORD ArrayRowCnt, WORD wDataType);
BOOL SS_GetArray(LPSPREADSHEET lpSS, LPVOID lpArray, SS_COORD ColLeft,
                 SS_COORD RowTop, SS_COORD ArrayColCnt,
                 SS_COORD ArrayRowCnt, WORD wDataType);
short SS_GetTwoDigitYearMax(LPSS_BOOK lpBook);
BOOL SS_SetTwoDigitYearMax(LPSS_BOOK lpBook, short nTwoDigitYearMax);
#if defined(_WIN64) || defined(_IA64)
void SS_SetPrintOptions(LPSPREADSHEET lpSS, LPSS_PAGEFORMAT lpPageFormat,
                        SS_PRINTFUNC lpfnPrintProc, LONG_PTR lAppData);
#else
void SS_SetPrintOptions(LPSPREADSHEET lpSS, LPSS_PAGEFORMAT lpPageFormat,
                        SS_PRINTFUNC lpfnPrintProc, LONG lAppData);
#endif
BOOL SS_GetCellColor(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                     LPCOLORREF lpBackground, LPCOLORREF lpForeground);
void SS_GetTopLeftPrev(LPSPREADSHEET lpSS, LPSS_COORD lpCol, LPSS_COORD lpRow);

HFONT SS_GetFontHandle(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);

#ifdef SS_V35
void DLLENTRY SS_SetCalTextOverride(LPSS_BOOK lpBook, LPCTSTR lpszShortDays, LPCTSTR lpszLongDays,
                                   LPCTSTR lpszShortMonths, LPCTSTR lpszLongMonths,
                                   LPCTSTR lpszOkText, LPCTSTR lpszCancelText);
BOOL SS_SetCellNoteRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, LPTSTR Note);
#endif // SS_V35

#ifdef SS_V40
void SS_SetRowHeaderCols(LPSPREADSHEET lpSS, SS_COORD lRowHeaderCols);
SS_COORD SS_GetRowHeaderCols(LPSPREADSHEET lpSS);
void SS_SetRowHeadersAutoTextIndex(LPSPREADSHEET lpSS, SS_COORD lRowHeadersAutoTextIndex);
SS_COORD SS_GetRowHeadersAutoTextIndex(LPSPREADSHEET lpSS);
void SS_SetColHeaderRows(LPSPREADSHEET lpSS, SS_COORD lColHeaderRows);
SS_COORD SS_GetColHeaderRows(LPSPREADSHEET lpSS);
void SS_SetColHeadersAutoTextIndex(LPSPREADSHEET lpSS, SS_COORD lColHeadersAutoTextIndex);
SS_COORD SS_GetColHeadersAutoTextIndex(LPSPREADSHEET lpSS);
void SS_SetColHeadersUserSortIndex(LPSPREADSHEET lpSS, SS_COORD lColHeadersUserSortIndex);
SS_COORD SS_GetColHeadersUserSortIndex(LPSPREADSHEET lpSS);
short SS_GetCellTag(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPTSTR CellTag);
BOOL SS_SetCellTagRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, LPCTSTR lpszCellTag);
#endif // SS_V40
BOOL SS_ResetSheet(LPSS_BOOK lpBook, short nSheetIndex);

#ifdef SS_V70
void SS_GetScrollBarSize(LPSS_BOOK lpBook, LPSHORT lpdVScrollWidth, LPSHORT lpdHScrollHeight);
void SS_SetScrollBarSize(LPSS_BOOK lpBook, short dVScrollWidth, short dHScrollHeight);
WORD SS_GetActiveCellHighlightStyle(LPSPREADSHEET lpSS);
void SS_SetActiveCellHighlightStyle(LPSPREADSHEET lpSS, WORD wActiveCellHighlightStyle);
void SS_GetScrollBarColor(LPSS_BOOK lpBook, LPCOLORREF lpVScrollBarColor, LPCOLORREF lpHScrollBarColor);
void SS_SetScrollBarColor(LPSS_BOOK lpBook, COLORREF VScrollBarColor, COLORREF HScrollBarColor);
short SS_GetSheet(LPSS_BOOK lpBook);
void SS_SetSheet(LPSS_BOOK lpBook, short nSheet);
short SS_GetSheetSendingMsg(LPSS_BOOK lpBook);
#endif // SS_V70
void SS_PageFormat2PrintFormat(LPSS_PAGEFORMAT lpPageFormat, LPSS_PRINTFORMAT lpPrintFormat);
void SS_PrintFormat2PageFormat(LPSS_PAGEFORMAT lpPageFormat, LPSS_PRINTFORMAT lpPrintFormat);
#ifdef SS_V80
COLORREF SS_GetCellNoteIndicatorColor(LPSS_BOOK lpBook);
COLORREF SS_SetCellNoteIndicatorColor(LPSS_BOOK lpBook, COLORREF CellNoteIndicatorColor);
WORD SS_GetCellNoteIndicatorShape(LPSS_BOOK lpBook);
WORD SS_SetCellNoteIndicatorShape(LPSS_BOOK lpBook, WORD CellNoteIndicatorShape);
BOOL SS_GetEnhanceStaticCells(LPSS_BOOK lpBook);
BOOL SS_SetEnhanceStaticCells(LPSS_BOOK lpBook, BOOL wStyle);

WORD SS_GetTabEnhancedShape(LPSS_BOOK lpBook);
WORD SS_SetTabEnhancedShape(LPSS_BOOK lpBook, WORD wStyle);
WORD SS_GetAppearanceStyle(LPSS_BOOK lpBook);
WORD SS_SetAppearanceStyle(LPSS_BOOK lpBook, WORD nAppear);
WORD SS_GetScrollBarStyle(LPSS_BOOK lpBook);
WORD SS_SetScrollBarStyle(LPSS_BOOK lpBook, WORD nAppear);
WORD SS_GetUseVisualStyles(LPSS_BOOK lpBook);
WORD SS_SetUseVisualStyles(LPSS_BOOK lpBook, WORD nAppear);
BOOL SS_SetEnhancedSheetTabColors(LPSS_BOOK lpBook, COLORREF clrNormalUpperStartColor, COLORREF clrNormalUpperEndColor,
								   COLORREF clrNormalLowerStartColor, COLORREF clrNormalLowerEndColor, COLORREF clrHoverUpperStartColor,
								   COLORREF clrHoverUpperEndColor, COLORREF clrHoverLowerStartColor, COLORREF clrHoverLowerEndColor,
								   COLORREF clrTabOuterBorderColor, COLORREF clrTabInnerBorderColor, COLORREF clrButtonUpperNormalStartColor,
								   COLORREF clrButtonUpperNormalEndColor, COLORREF clrButtonLowerNormalStartColor, COLORREF clrButtonLowerNormalEndColor,
								   COLORREF clrButtonUpperHoverStartColor, COLORREF clrButtonUpperHoverEndColor, COLORREF clrButtonLowerHoverStartColor,
								   COLORREF clrButtonLowerHoverEndColor,COLORREF clrButtonUpperPushedStartColor, COLORREF clrButtonUpperPushedEndColor,
								   COLORREF clrButtonLowerPushedStartColor, COLORREF clrButtonLowerPushedEndColor, COLORREF clrArrowColor,
								   COLORREF clrBackColorStartColor, COLORREF clrBackColorEndColor, COLORREF clrSplitBoxStartColor,
								   COLORREF clrSplitBoxEndColor, COLORREF clrForeColor, COLORREF clrActiveForeColor);
BOOL SS_SetEnhancedScrollBarColors(LPSS_BOOK lpBook, COLORREF clrTrackColor, COLORREF clrArrowColor, COLORREF clrUpperNormalStartColor, COLORREF clrUpperNormalEndColor,
										   COLORREF clrLowerNormalStartColor, COLORREF clrLowerNormalEndColor, COLORREF clrUpperPushedStartColor, COLORREF clrUpperPushedEndColor,
										   COLORREF clrLowerPushedStartColor, COLORREF clrLowerPushedEndColor, COLORREF clrUpperHoverStartColor, COLORREF clrUpperHoverEndColor,
										   COLORREF clrLowerHoverStartColor, COLORREF clrLowerHoverEndColor, COLORREF clrHoverButtonBorderColor, COLORREF clrButtonBorderColor);
BOOL SS_SetEnhancedCornerColors(LPSS_BOOK lpBook, COLORREF clrBackColor, COLORREF clrHoverColor, COLORREF clrTriangleColor, COLORREF clrTriangleHover, COLORREF clrTriangleBorderColor, COLORREF clrTriangleHoverBorderColor);
BOOL SS_SetEnhancedColumnHeaderColors(LPSS_BOOK lpBook, COLORREF clrSelectedHoverUpperColor, COLORREF clrSelectedHoverLowerColor, COLORREF clrSelectedUpperColor,
											  COLORREF clrSelectedLowerColor, COLORREF clrHoverUpperColor, COLORREF clrHoverLowerColor, COLORREF clrUpperColor,
											  COLORREF clrLowerColor, COLORREF clrSelectedBorderColor, COLORREF clrBorderColor);
BOOL SS_SetEnhancedRowHeaderColors(LPSS_BOOK lpBook, COLORREF clrSelectedHoverUpperColor, COLORREF clrSelectedHoverLowerColor, COLORREF clrSelectedUpperColor,
											  COLORREF clrSelectedLowerColor, COLORREF clrHoverUpperColor, COLORREF clrHoverLowerColor, COLORREF clrUpperColor,
											  COLORREF clrLowerColor, COLORREF clrSelectedBorderColor, COLORREF clrBorderColor);

BOOL SS_IsVerticalScrollBarVisible(LPSS_BOOK lpBook);
BOOL SS_IsHorizontalScrollBarVisible(LPSS_BOOK lpBook);

WORD SS_SetHighlightHeaders(LPSS_BOOK lpBook, WORD wOption);
WORD SS_GetHighlightHeaders(LPSS_BOOK lpBook);
WORD SS_SetHighlightStyle(LPSS_BOOK lpBook, WORD wStyle);
WORD SS_GetHighlightStyle(LPSS_BOOK lpBook);
BOOL SS_SetHighlightAlphaBlend(LPSS_BOOK lpBook, COLORREF clrAlphaBlend, short nAlphaBlend);
BOOL SS_GetHighlightAlphaBlend(LPSS_BOOK lpBook, LPCOLORREF lpclrAlphaBlend, LPSHORT lpnAlphaBlend);
#endif

SS_COORD SS_GetNextNonEmptyRow(LPSPREADSHEET lpSS, SS_COORD Row);
SS_COORD SS_GetNextNonEmptyColumnInRow(LPSPREADSHEET lpSS, SS_COORD Row, SS_COORD Col);
void SS_ClearCheckBoxPictures(LPSS_BOOK lpBook);

#ifdef __cplusplus
}
#endif

#endif
