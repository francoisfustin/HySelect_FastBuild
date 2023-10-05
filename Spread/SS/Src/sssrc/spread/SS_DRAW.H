/************
* SS_DRAW.H
************/

#ifndef SS_DRAW_H
#define SS_DRAW_H

#ifdef __cplusplus
extern "C" {
#endif

#define SS_DFSCREEN2PRINTERX(lpPFormat, x) (!lpPFormat ? (double)(x) : ((double)(x) *\
                                       (double)(lpPFormat)->TextWidthPrinter / \
                                       (double)(lpPFormat)->TextWidthScreen *  \
                                       (lpPFormat)->dfScale *                  \
                                       (lpPFormat)->dfScalePreviewX))
#define SS_SCREEN2PRINTERX(lpPFormat, x) ((short)SS_DFSCREEN2PRINTERX(lpPFormat, x))
#define SS_DFSCREEN2PRINTERY(lpPFormat, y) (!lpPFormat ? (double)(y) : ((double)(y) *\
                                       (double)(lpPFormat)->TextHeightPrinter /\
                                       (double)(lpPFormat)->TextHeightScreen * \
                                       (lpPFormat)->dfScale *                  \
                                       (lpPFormat)->dfScalePreviewY))
#define SS_SCREEN2PRINTERY(lpPFormat, y) ((short)SS_DFSCREEN2PRINTERY(lpPFormat, y))
#define SS_PRINTER2SCREENX(lpPFormat, x) ((short)((double)(x) /                  \
                                       (double)(lpPFormat)->TextWidthPrinter * \
                                       (double)(lpPFormat)->TextWidthScreen /  \
                                       (lpPFormat)->dfScale /                  \
                                       (lpPFormat)->dfScalePreviewX))
#define SS_PRINTER2SCREENY(lpPFormat, y) ((short)((double)(y) /                  \
                                       (double)(lpPFormat)->TextHeightPrinter *\
                                       (double)(lpPFormat)->TextHeightScreen / \
                                       (lpPFormat)->dfScale /                  \
                                       (lpPFormat)->dfScalePreviewY))

#ifdef SS_V80
	#define RADIANS(d) (d * 2.0 * 3.1415926535897932384626433832795)/360.0
#endif
/******************************
* Declare function prototypes
******************************/

#define SS_DRAWTEXTSEL_DEF -1

void              SS_DrawLine(HDC hdc, int x1, int y1, int x2, int y2);
BOOL              SS_DrawText(HDC hDC, HWND hWnd, LPSPREADSHEET lpSS,
                              LPSS_COL lpCol, LPSS_ROW lpRow, LPRECT Rect,
                              SS_COORD CellCol, SS_COORD CellRow, BOOL fErase,
                              LPSSX_PRINTFORMAT lpPrintFormat, LPRECT lpRectUpdate,
                              LPRECT lpCellOverflowRectPrev, LPRECT lpCellOverflowRectNext,
                              BOOL fOverflowToLeft, BOOL fCellSelected, SS_COORD CellLeft,
                              SS_COORD CellTop, BOOL IsLastCol, BOOL IsLastRow);
BOOL              SS_GetColorTblItem(LPSPREADSHEET lpSS,
                                     SS_COORD Col, SS_COORD Row,
                                     LPSS_COLORTBLITEM BackColorTblItem,
                                     LPSS_COLORTBLITEM ForeColorTblItem);
BOOL              SSx_GetColorTblItem(LPSPREADSHEET lpSS,
                                      LPSS_COL lpCol, LPSS_ROW lpRow,
                                      LPSS_CELL lpCell, SS_COORD Col,
                                      SS_COORD Row,
                                      LPSS_COLORTBLITEM BackColorTblItem,
                                      LPSS_COLORTBLITEM ForeColorTblItem);
SS_COLORID        SS_AddColor(COLORREF Color);
LPSS_COLORTBLITEM SS_GetColorItem(LPSS_COLORTBLITEM ColorTblItem, SS_COLORID ColorId);
BOOL              SS_Paint(LPSPREADSHEET lpSS, HDC hDC);
extern BOOL       SS_DrawSelBlock(HDC hDC, LPSPREADSHEET lpSS);
extern BOOL       SS_InvertBlock(HDC hDC, LPSPREADSHEET lpSS);
BOOL              SS_CreateBlockRect(LPSPREADSHEET lpSS, LPRECT Rect,
                                     SS_COORD Left, SS_COORD Top,
                                     SS_COORD Right, SS_COORD Bottom,
                                     BOOL fCell);
void              SS_LineTo(LPSPREADSHEET lpSS, HDC hDC, HBRUSH hBrush,
                            HBRUSH hBrush2, HPEN hPenGrid, int x, int y,
                            BYTE LineType, BOOL fGridLines,
                            LPSSX_PRINTFORMAT lpPrintFormat);
void              SS_MoveTo(HDC hDC, int x, int y,
                            LPSSX_PRINTFORMAT lpPFormat);
void              SS_HatchRect(HDC hDC, LPSPREADSHEET lpSS,
                               LPRECT lpRect, LPSSX_PRINTFORMAT lpPrintFormat);
extern void       SS_ReverseArea(HDC hDC, int x, int y, int cx, int cy,
                                 LPRECT RclBound);
extern long       SS_CalcRowHeight(LPSPREADSHEET lpSS, SS_COORD Row);
extern short      SS_GetCellPosX(LPSPREADSHEET lpSS, SS_COORD ColLeft,
                                  SS_COORD Col);
extern short      SS_GetCellPosY(LPSPREADSHEET lpSS, SS_COORD RowTop,
                                 SS_COORD Row);
extern BOOL       SS_GetCellCoord(LPSPREADSHEET lpSS, SS_COORD ColLeft, SS_COORD RowTop,
                                  SS_COORD Col, SS_COORD Row, LPINT lpx, LPINT lpy,
                                  LPINT lpcx, LPINT lpcy);
BOOL              SS_GetCellRangeCoord(LPSPREADSHEET lpSS, SS_COORD ColLeft, SS_COORD RowTop, SS_COORD Col1,
                                       SS_COORD Row1, SS_COORD Col2, SS_COORD Row2, LPRECT lpRect, LPINT lpx, LPINT lpy,
                                       LPINT lpcx, LPINT lpcy);
BOOL              SS_GetCellActualSize(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPINT lpcx, LPINT lpcy);
extern BOOL       SS_Draw(HWND hWnd, HDC hDC, HDC hDCDev, LPSPREADSHEET lpSS, LPRECT lpRect, LPRECT lpRectUpdate,
                          SS_COORD CellLeft, SS_COORD CellTop, SS_COORD CellRight, SS_COORD CellBottom,
                          SS_COORD xCellMax, SS_COORD yCellMax, LPSSX_PRINTFORMAT lpPFormat);
BOOL              SS_InvertRow(HWND hWnd, HDC hDC, LPSPREADSHEET lpSS);
BOOL              SS_InvertRowBlock(HWND hWnd, HDC hDC, LPSPREADSHEET lpSS,
                                    SS_COORD Row, SS_COORD Row2);
//void              SS_HighlightDatePermanent(HWND hWnd);
void              SS_CalcCellMetrics(HDC hDC, LPSPREADSHEET lpSS, LPSS_COL lpCol, LPSS_ROW lpRow, LPSS_CELL Cell,
                                     LPSS_CELLTYPE CellType, LPRECT lpRect, SS_COORD CellCol, SS_COORD CellRow, BOOL fCalcWidth,
                                     BOOL fCalcHeight, LPSHORT lpJust, LPTSTR lpData, LPINT lpiWidth, LPINT lpiHeight);
#ifdef SS_V80
void              SS_DrawComboBtn(HWND hWnd, LPSPREADSHEET lpSS,
                                  HDC hDC, LPRECT lpRect, short dButtonWidth,
                                  BOOL fButtonDown, BOOL fMouseOver, BOOL fEnhanced);
long              SS_BtnDraw(HWND hWnd, HDC hDC, LPRECT lpRect,
                             LPSS_CELLTYPE lpCellType, HFONT hFont,
                             BOOL fButtonDown, BOOL fRoundCorners,
                             BOOL fUseDib, LPRECT lpRectOrig, BOOL fIsButton,
                             BOOL fIs95, BOOL fCalcSize, BOOL fMouseOver, BOOL fEnhanced);
long              SS_CheckBoxDraw(HWND hWnd, HWND hWndCtrl, HDC hDC,
                                  LPRECT lpRect, LPSS_CELLTYPE lpCellType,
                                  HFONT hFont, BOOL fButtonDown, BOOL fUseDib,
                                  LPRECT lpRectOrig, BOOL fCalcSize, BOOL fEnhanced);
#else
void              SS_DrawComboBtn(HWND hWnd, LPSPREADSHEET lpSS,
                                  HDC hDC, LPRECT lpRect, short dButtonWidth,
                                  BOOL fButtonDown);
long              SS_BtnDraw(HWND hWnd, HDC hDC, LPRECT lpRect,
                             LPSS_CELLTYPE lpCellType, HFONT hFont,
                             BOOL fButtonDown, BOOL fRoundCorners,
                             BOOL fUseDib, LPRECT lpRectOrig, BOOL fIsButton,
                             BOOL fIs95, BOOL fCalcSize);
long              SS_CheckBoxDraw(HWND hWnd, HWND hWndCtrl, HDC hDC,
                                  LPRECT lpRect, LPSS_CELLTYPE lpCellType,
                                  HFONT hFont, BOOL fButtonDown, BOOL fUseDib,
                                  LPRECT lpRectOrig, BOOL fCalcSize);
#endif

#ifndef SS_NOOVERFLOW
BOOL              SS_CalcCellOverflow(LPSPREADSHEET lpSS, LPSS_CELL lpCell,
                                      LPSS_CELLTYPE lpCellType,
                                      SS_COORD Col, SS_COORD Row);
BOOL              SS_ResetCellOverflowNeighbor(LPSPREADSHEET lpSS, SS_COORD Col,
                                               SS_COORD Row);
BOOL              SS_ResetCellOverflow(LPSPREADSHEET lpSS,
                                       SS_COORD Col, SS_COORD Row);
BOOL              SSx_ResetCellOverflow(LPSPREADSHEET lpSS,
                                        SS_COORD Col, SS_COORD Row,
                                        LPSHORT lpWidthPrev,
                                        LPSHORT lpWidthNext);
void              SS_OverflowAdjustNeighbor(LPSPREADSHEET lpSS,
                                            SS_COORD Col, SS_COORD Row);
BOOL              SS_CalcOverflowRect(HDC hDC, LPSPREADSHEET lpSS,
                                      LPTSTR Data, SS_COORD Col, UINT Just,
                                      LPRECT lpRect,
                                      LPRECT lpCellOverflowRectPrev,
                                      LPRECT lpCellOverflowRectNext,
                                      LPRECT lpRectNew);
#else
#define SS_CalcCellOverflow(lpSS, lpCell, lpCellType, Col, Row)  FALSE
#define SS_ResetCellOverflowNeighbor(lpSS, Col, Row)             FALSE
#define SS_ResetCellOverflow(lpSS, Col, Row)                     FALSE
#define SSx_ResetCellOverflow(lpSS, Col, Row, lpWidthPrev, lpWidthNext) FALSE
#define SS_OverflowAdjustNeighbor(lpSS, Col, Row)                FALSE
#define SS_CalcOverflowRect(hDC, lpSS, Data, Col, Just, lpRect, lpCellOverflowRectPrev, lpCellOverflowRectNext, lpRectNew) FALSE
#endif

short             SS_RetrieveJust(short Justification);
HFONT             SS_CreatePrinterFont(LPSPREADSHEET lpSS, HDC hDCPrinter, LPSS_FONT lpFont,
                                       SS_FONTID FontId,
                                       LPSSX_PRINTFORMAT lpPFormat);
SS_COORD          SS_GetBottomCell(LPSPREADSHEET lpSS, SS_COORD RowTop);
BOOL              SS_OverflowInvalidateCell(LPSPREADSHEET lpSS,
                                            SS_COORD Col, SS_COORD Row);
WORD              SS_GetOverflowRects(LPSPREADSHEET lpSS,
                                      SS_COORD Col, SS_COORD Row,
                                      LPRECT lpRectClient, LPRECT lpRect,
                                      LPRECT lpRectOverflowPrev,
                                      LPRECT lpRectOverflowNext,
                                      LPBOOL lpfOverflowToLeft,
                                      LPRECT lpRectOverflowAll);
void              SS_DrawHiddenOverflowCellLeft(HWND hWnd, HDC hDC,
                                                LPSPREADSHEET lpSS,
                                                LPSS_ROW lpRow, LPRECT lpRect,
                                                SS_COORD Col, SS_COORD Row,
                                                LPSSX_PRINTFORMAT lpPFormat,
                                                LPRECT lpRectUpdate, SS_COORD CellLeft,
                                                SS_COORD CellTop, BOOL IsLastCol, BOOL IsLastRow);
void              SS_DrawHiddenOverflowCellRight(HWND hWnd, HDC hDC,
                                                 LPSPREADSHEET lpSS,
                                                 LPSS_ROW lpRow, LPRECT lpRect,
                                                 SS_COORD Col, SS_COORD Row,
                                                 LPSSX_PRINTFORMAT lpPFormat,
                                                 LPRECT lpRectUpdate, SS_COORD CellLeft,
                                                 SS_COORD CellTop, BOOL IsLastCol, BOOL IsLastRow);
void SS_DrawSortIndicator(HDC hdc, RECT *prc, BOOL bAscending);

short             SS_GetCellPosYExt(LPSPREADSHEET lpSS, SS_COORD RowTop,
                                    SS_COORD Row, LPRECT lpRect);
short             SS_GetCellPosXExt(LPSPREADSHEET lpSS, SS_COORD ColLeft,
                                    SS_COORD Col, LPRECT lpRect);
SS_COORD          SS_ColLetterToNumber(LPCTSTR lpBuffer);
void              SS_DrawFormatHeaderLetter(LPTSTR lpBuffer,
                                            long lHeaderNumber);
void              SS_DrawFormatHeaderNumber(LPTSTR lpBuffer,
                                            long lHeaderNumber);
BOOL              SS_InvertMultiSelBlock(HDC hDC, LPSPREADSHEET lpSS);
void              SS_PaintEditModeShadow(HWND hWnd, LPSPREADSHEET lpSS,
                                         LPRECT lpRectClient, short x, short y,
                                         short cx, short cy);
void              SS_ClearEditModeShadow(LPSPREADSHEET lpSS);
#ifdef SS_UTP
void              SS_GetScrollArrowRects(HWND hWnd, LPSPREADSHEET lpSS,
                                         LPRECT lpRectRight, LPRECT lpRectDown,
                                         LPRECT lpRectLeft, LPRECT lpRectUp);
void              SS_DrawArrow(HWND hWnd, HDC hDC, LPSPREADSHEET lpSS,
                               BYTE bArrow, BOOL fButtonDown);
#endif

#ifdef SS_V35
BOOL              SS_DrawCellNote(LPSPREADSHEET lpSS, HDC hdc, LPRECT lpRect, SS_COORD Col, SS_COORD Row);
#endif

#ifdef SS_V80
void		         SS_RotateBitmap(HDC bitmapDC, RECT bmpRect, HDC destDC, RECT rc, float lRadians);
void			      SS_FlipBitmap(HDC destDC, RECT rc, BOOL bLateral);
void              SS_AlphaBlendFree(LPSS_BOOK lpBook);
#endif // SS_V80

BOOL              SS_ColorTableAlloc(void);
void              SS_ColorTableDelete(void);
void              SS_ColorTableDeleteBrushes(void);
LPSS_COLORTBLITEM SS_ColorTableLock(void);
void              SS_ColorTableUnlock(void);

//#ifdef SPREAD_JPN
//- This structure is used to draw the border styles
//        provided in the Japanese version (Masanori Iwasa)
typedef struct TagLINEDATA {
   HDC          hDC;            //- Screen / Printer hDC
   COLORREF     Color[2];       //- 0- ForeColor, 1- BackColor
   BOOL         fIsLeft2Right;  //- Check if drawing line
   BOOL         fStart;         //- Is the first time
   int          nWidth;         //- For printer usage
   int          n1PointDots;    //- For printer usage
   LPBYTE       lpLineType;     //- Line pattern array
} LINEDATA, FAR * LPLINEDATA;

void DrawBorderEx(HDC hDC, int X1, int Y1, int X2, int Y2, COLORREF ForeColor, 
                    COLORREF BackColor, LPBYTE LineType, LPSSX_PRINTFORMAT lpPFormat);

//#endif

void SS_CopyCellRect(LPSPREADSHEET lpSS, LPRECT lpRectNew, LPRECT lpRect,
                     LPSSX_PRINTFORMAT lpPFormat, BOOL fOverGrid, BOOL IsLastCol, BOOL IsLastRow);
BOOL SS_ShouldCellBeDrawnSelected(LPSPREADSHEET lpSS, SS_COORD Col,
                                  SS_COORD Row, LPSS_ROW lpRow);
BOOL SS_ShouldButtonBeDrawn(LPSPREADSHEET lpSS, SS_COORD Col,
                            SS_COORD Row, WORD wCellType);
#ifdef SS_V80
BOOL SS_CustCellTypeGetAppearance(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, SS_COORD Col, SS_COORD Row, HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPSS_CT_APPEARANCE app);
#endif

#ifdef __cplusplus
}
#endif

#endif
