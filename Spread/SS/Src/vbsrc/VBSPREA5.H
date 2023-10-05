/*************
* VBSPREA5.H
*************/

#ifndef VBSPREA5_H
#define VBSPREA5_H

#define SSVB_ARRAYTYPE_SHORT   0
#define SSVB_ARRAYTYPE_LONG    1
#define SSVB_ARRAYTYPE_FLOAT   2
#define SSVB_ARRAYTYPE_DOUBLE  3
#define SSVB_ARRAYTYPE_VARIANT 4

/*****************************************
* Exported functions for calling from VB
*****************************************/

void FAR PASCAL _export     SpreadGetClientArea(HCTL hCtl, LPLONG lplWidth,
                                     LPLONG lplHeight);
void FAR PASCAL _export     SpreadGetCellFromScreenCoord(HCTL hCtl, LPLONG lpCol,
                                               LPLONG lpRow, long x, long y);
BOOL FAR PASCAL _export     SpreadGetCellPos(HCTL hCtl, long Col, long Row, LPLONG lpx,
                                   LPLONG lpy, LPLONG lpWidth, LPLONG lpHeight);
void FAR PASCAL _export     SpreadGetBottomRightCell(HCTL hCtl, LPLONG lpCol, LPLONG lpRow);
void FAR PASCAL _export     SpreadGetFirstValidCell(HCTL hCtl, LPLONG lpCol, LPLONG lpRow);
void FAR PASCAL _export     SpreadGetLastValidCell(HCTL hCtl, LPLONG lpCol, LPLONG lpRow);
void FAR PASCAL _export     SpreadGetCurrSelBlockPos(HCTL hCtl, LPLONG lpCol,
                                           LPLONG lpRow);
void FAR PASCAL _export     SpreadColWidthToTwips(HCTL hCtl, float fColWidth,
                                        LPLONG lpTwips);
void FAR PASCAL _export     SpreadRowHeightToTwips(HCTL hCtl, long Row, float fRowHeight,
                                         LPLONG lpTwips);
void FAR PASCAL _export     SpreadTwipsToColWidth(HCTL hCtl, long Twips,
                                        LPFLOAT lpfColWidth);
void FAR PASCAL _export     SpreadTwipsToRowHeight(HCTL hCtl, long Row, long Twips,
                                         LPFLOAT lpfRowHeight);
BOOL FAR PASCAL _export     SpreadIsVisible(HCTL hCtl, long Col, long Row, BOOL Partial);
BOOL FAR PASCAL _export     SpreadIsFormulaValid(HCTL hCtl, HLSTR hlstrFormula);
BOOL FAR PASCAL _export     SpreadIsCellSelected(HCTL hCtl, long Col, long Row);
void FAR PASCAL _export     SpreadSetItemData(HCTL hCtl, long lValue);
void FAR PASCAL _export     SpreadSetColItemData(HCTL hCtl, long Col, long lValue);
void FAR PASCAL _export     SpreadSetRowItemData(HCTL hCtl, long Row, long lValue);
long FAR PASCAL _export     SpreadGetItemData(HCTL hCtl);
long FAR PASCAL _export     SpreadGetColItemData(HCTL hCtl, long Col);
long FAR PASCAL _export     SpreadGetRowItemData(HCTL hCtl, long Row);
void FAR PASCAL _export     SpreadSaveDesignInfo(HWND hWndDest, HWND hWndSrc,
                                       BOOL fInit);
#ifdef SS_GRID
void FAR PASCAL _export     fpSetText(HCTL hCtl, long Col, long Row, LPVAR lpVar);
BOOL FAR PASCAL _export     fpGetText(HCTL hCtl, long Col, long Row, LPVAR lpVar);
#else
void FAR PASCAL _export     SpreadSetText(HCTL hCtl, long Col, long Row, LPVAR lpVar);
BOOL FAR PASCAL _export     SpreadGetText(HCTL hCtl, long Col, long Row, LPVAR lpVar);
#endif
BOOL FAR PASCAL _export     SpreadAddCustomFunction(HCTL hCtl, HSZ lpszFunctionName,
                                          short nParameterCnt);
BOOL FAR PASCAL _export     SpreadCFGetParamInfo(HCTL hCtl, short dParam, LPWORD lpwType,
                                       LPWORD lpwStatus);
long FAR PASCAL _export     SpreadCFGetLongParam(HCTL hCtl, short dParam);
double FAR PASCAL _export   SpreadCFGetDoubleParam(HCTL hCtl, short dParam);
HLSTR FAR PASCAL _export    SpreadCFGetStringParam(HCTL hCtl, short dParam);
void FAR PASCAL _export     SpreadCFSetResult(HCTL hCtl, LPVAR lpVar);
HLSTR FAR PASCAL _export    SpreadColNumberToLetter(long lHeaderNumber);
BOOL FAR PASCAL _export     SpreadGetDataFillData(HCTL hCtl, LPVAR lpVar, WORD wVarType);
BOOL FAR PASCAL _export     SpreadSetDataFillData(HCTL hCtl, LPVAR lpVar);
#ifdef SS_GRID
BOOL FAR PASCAL _export     SaveTabFile(HCTL hCtl, LPCSTR lpszFileName);
BOOL FAR PASCAL _export     SetCellDirtyFlag(HCTL hCtl, SS_COORD Col,
                                             SS_COORD Row, BOOL fDirty);
BOOL FAR PASCAL _export     GetCellDirtyFlag(HCTL hCtl, SS_COORD Col,
                                             SS_COORD Row);
#else
BOOL FAR PASCAL _export     SpreadSaveTabFile(HCTL hCtl, LPCSTR lpszFileName);
BOOL FAR PASCAL _export     SpreadSetCellDirtyFlag(HCTL hCtl, SS_COORD Col,
                                                   SS_COORD Row, BOOL fDirty);
BOOL FAR PASCAL _export     SpreadGetCellDirtyFlag(HCTL hCtl, SS_COORD Col,
                                         SS_COORD Row);
#endif
SS_COORD FAR PASCAL _export SpreadGetMultiSelItem(HCTL hCtl, SS_COORD SelPrev);
BOOL FAR PASCAL _export     SpreadSaveTabFile(HCTL hCtl, LPCSTR lpszFileName);

void FAR PASCAL _export     _SpreadGetClientArea(HWND hWnd, LPLONG lplWidth,
                                      LPLONG lplHeight);
void FAR PASCAL _export     _SpreadGetCellFromScreenCoord(HWND hWnd, LPLONG lpCol,
                                                LPLONG lpRow, long x, long y);
BOOL FAR PASCAL _export     _SpreadGetCellPos(HWND hWnd, long Col, long Row, LPLONG lpx,
                                    LPLONG lpy, LPLONG lpWidth, LPLONG lpHeight);
void FAR PASCAL _export     _SpreadGetBottomRightCell(HWND hWnd, LPLONG lpCol, LPLONG lpRow);
void FAR PASCAL _export     _SpreadGetFirstValidCell(HWND hWnd, LPLONG lpCol, LPLONG lpRow);
void FAR PASCAL _export     _SpreadGetLastValidCell(HWND hWnd, LPLONG lpCol, LPLONG lpRow);
void FAR PASCAL _export     _SpreadGetCurrSelBlockPos(HWND hWnd, LPLONG lpCol,
                                            LPLONG lpRow);
void FAR PASCAL _export     _SpreadColWidthToTwips(HWND hWnd, float fColWidth,
                                         LPLONG lpTwips);
void FAR PASCAL _export     _SpreadRowHeightToTwips(HWND hWnd, long Row, float fRowHeight,
                                          LPLONG lpTwips);
void FAR PASCAL _export     _SpreadTwipsToColWidth(HWND hWnd, long Twips,
                                         LPFLOAT lpfColWidth);
void FAR PASCAL _export     _SpreadTwipsToRowHeight(HWND hWnd, long Row, long Twips,
                                          LPFLOAT lpfRowHeight);
BOOL FAR PASCAL _export     _SpreadIsVisible(HWND hWnd, long Col, long Row, BOOL Partial);
BOOL FAR PASCAL _export     _SpreadIsFormulaValid(HWND hWnd, LPTSTR lpszFormula);
BOOL FAR PASCAL _export     _SpreadIsCellSelected(HWND hWnd, long Col, long Row);
void FAR PASCAL _export     _SpreadSetItemData(HWND hWnd, long lValue);
void FAR PASCAL _export     _SpreadSetColItemData(HWND hWnd, long Col, long lValue);
void FAR PASCAL _export     _SpreadSetRowItemData(HWND hWnd, long Row, long lValue);
long FAR PASCAL _export     _SpreadGetItemData(HWND hWnd);
long FAR PASCAL _export     _SpreadGetColItemData(HWND hWnd, long Col);
long FAR PASCAL _export     _SpreadGetRowItemData(HWND hWnd, long Row);
void FAR PASCAL _export     _SpreadSaveDesignInfo(HWND hWndDest, HWND hWndSrc,
                                                  BOOL fInit);
void FAR PASCAL _export     _SpreadSetText(HWND hWnd, long Col, long Row, LPTSTR lpText);
LPTSTR FAR PASCAL _export    _SpreadGetText(HWND hWnd, long Col, long Row);
BOOL FAR PASCAL _export     _SpreadAddCustomFunction(HWND hWnd, HSZ lpszFunctionName,
                                           short nParameterCnt);
BOOL FAR PASCAL _export _SpreadAddCustomFunctionExt(HWND hWnd,
                                                    HSZ lpszFunctionName,
                                                    short nMinParamCnt,
                                                    short nMaxParamCnt,
                                                    long lFlags);
BOOL FAR PASCAL _export     _SpreadCFGetParamInfo(HWND hWnd, short dParam,
                                        LPWORD lpwType, LPWORD lpwStatus);
long FAR PASCAL _export     _SpreadCFGetLongParam(HWND hWnd, short dParam);
double FAR PASCAL _export   _SpreadCFGetDoubleParam(HWND hWnd, short dParam);
LPTSTR FAR PASCAL _export    _SpreadCFGetStringParam(HWND hWnd, short dParam);
void FAR PASCAL _export _SpreadCFGetCellParam(HWND hWnd, short dParam,
                                              LPLONG lplCol, LPLONG lplRow);
void FAR PASCAL _export _SpreadCFGetRangeParam(HWND hWnd, short dParam,
                                               LPLONG lplCol, LPLONG lplRow,
                                               LPLONG lplCol2, LPLONG lplRow2);
void FAR PASCAL _export     _SpreadCFSetResult(HWND hWnd, LPTSTR lpText);
HLSTR FAR PASCAL _export    _SpreadColNumberToLetter(long lHeaderNumber);
BOOL FAR PASCAL _export     _SpreadLoadFromFile(HWND hWnd, LPCSTR lpszFileName);
BOOL FAR PASCAL _export     _SpreadLoadTabFile(HWND hWnd, LPCSTR lpszFileName);
BOOL FAR PASCAL _export     _SpreadSaveToFile(HWND hWnd, LPCSTR lpszFileName,
                                              BOOL bDataOnly);
BOOL FAR PASCAL _export     _SpreadSaveTabFile(HWND hWnd, LPCSTR lpszFileName);
BOOL FAR PASCAL _export     _SpreadSetCellDirtyFlag(HWND hWnd, SS_COORD Col,
                                          SS_COORD Row, BOOL fDirty);
BOOL FAR PASCAL _export     _SpreadGetCellDirtyFlag(HWND hWnd, SS_COORD Col,
                                          SS_COORD Row);
SS_COORD FAR PASCAL _export _SpreadGetMultiSelItem(HWND hWnd, SS_COORD SelPrev);
BOOL FAR PASCAL _export     _SpreadSaveTabFile(HWND hWnd, LPCSTR lpszFileName);
BOOL FAR PASCAL _export     _SpreadSetDataFillData(HWND hWnd, LPTSTR lpszText);
BOOL FAR PASCAL _export     SpreadGetDataFillData(HCTL hCtl, LPVAR lpVar,
                                                  WORD wVarType);
BOOL FAR PASCAL _export     _SpreadGetDataFillData(HWND hWnd, LPTSTR lpszText);

#ifdef SS_GRID
HANDLE FAR PASCAL _export GetDataConnectHandle(HCTL hCtl);
HANDLE FAR PASCAL _export GetDataSelectHandle(HCTL hCtl);
#else
HANDLE FAR PASCAL _export SpreadGetDataConnectHandle(HCTL hCtl);
HANDLE FAR PASCAL _export SpreadGetDataSelectHandle(HCTL hCtl);
#endif

HANDLE FAR PASCAL _export _SpreadGetDataConnectHandle(HWND hWnd);
HANDLE FAR PASCAL _export _SpreadGetDataSelectHandle(HWND hWnd);

LPTSTR VBAPI _export       _SSDerefHlstrLen(HLSTR hlstr, LPUSHORT lpcbLen);
VOID VBAPI _export        _SSFreeString(LPTSTR lpStr);

short FAR PASCAL _export SpreadGetRefStyle(HCTL hCtl);
void FAR PASCAL _export SpreadSetRefStyle(HCTL hCtl, short nRefStyle);

BOOL FAR PASCAL _export SpreadGetIteration(HCTL hCtl, short* lpnMaxIterations,
                                           double* lpdfMaxChange);
void FAR PASCAL _export SpreadSetIteration(HCTL hCtl, BOOL bIteration,
                                           short nMaxIterations,
                                           double dfMaxChange);

HLSTR FAR PASCAL _export SpreadQueryCustomName(HCTL hCtl, HSZ lpszName);
HLSTR FAR PASCAL _export SpreadGetCustomName(HCTL hCtl, HSZ lpszName);
BOOL FAR PASCAL _export SpreadSetCustomName(HCTL hCtl, HSZ lpszName, HSZ lpszValue);
BOOL FAR PASCAL _export SpreadEnumCustomFunction(HCTL hCtl, HSZ lpszPrevFuncName, HLSTR hlstrFuncName);
BOOL FAR PASCAL _export SpreadGetCustomFunction(HCTL hCtl, HSZ lpszFuncName, short FAR* lpnMinArgs, short FAR* lpnMaxArgs, long FAR *lplFlags);
BOOL FAR PASCAL _export SpreadRemoveCustomFunction(HCTL hCtl, HSZ lpszFuncName);
BOOL FAR PASCAL _export SpreadReCalcCell(HCTL hCtl, long lCol, long lRow);
void FAR PASCAL _export SpreadSetCalText(LPCTSTR lpszShortDays, LPCTSTR lpszLongDays,
                                         LPCTSTR lpszShortMonths, LPCTSTR lpszLongMonths,
                                         LPCTSTR lpszOkText, LPCTSTR lpszCancelText);
BOOL FAR PASCAL _export SpreadSetTextTipAppearance(HCTL hCtl, HLSTR hlstrFontName,
                        short nFontSize, BOOL fFontBold, BOOL fFontItalic,
                        COLORREF clrBack, COLORREF clrFore);
BOOL FAR PASCAL _export SpreadGetTextTipAppearance(HCTL hCtl, HLSTR hlstrFontName,
                        LPSHORT lpnFontSize, LPBOOL lpfFontBold, LPBOOL lpfFontItalic,
                        LPCOLORREF lpclrBack, LPCOLORREF lpclrFore);
BOOL FP_API SpreadSetPrintOptions(HCTL hCtl, short bSmartPrint, short nPageOrder, long lFirstPageNumber);
BOOL FP_API SpreadGetPrintOptions(HCTL hCtl, LPSHORT lpbSmartPrint, LPSHORT lpnPageOrder, LPLONG lplFirstPageNumber);
long FP_API SpreadGetPrintPageCount(HCTL hCtl);
long FP_API SpreadGetNextPageBreakCol(HCTL hCtl, long lPrevCol);
long FP_API SpreadGetNextPageBreakRow(HCTL hCtl, long lPrevRow);
short FP_API SpreadGetTwoDigitYearMax(HCTL hCtl);
BOOL FP_API SpreadSetTwoDigitYearMax(HCTL hCtl, short nTwoDigitYearMax);
BOOL FP_API SpreadSetArray(HCTL hCtl, SS_COORD ColLeft, SS_COORD RowTop, HAD hAD);
BOOL FP_API SpreadGetArray(HCTL hCtl, SS_COORD ColLeft, SS_COORD RowTop, HAD hAD);

#endif
