/***********
* SS_DOC.H
***********/

#ifndef SS_DOC_H
#define SS_DOC_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************
* Declare function prototypes
******************************/

BOOL SS_ClearDoc(LPSPREADSHEET lpSS);
void SS_SetDocLock(LPSPREADSHEET lpSS, BOOL Lock);
BOOL SSGetDocLock(HWND hWnd);
BOOL SS_SetDocDefaultFont(LPSPREADSHEET lpSS, HFONT hFont, BOOL DeleteFont,
                          LPBOOL lpfDeleteFontObject);
#if SS_V80
BOOL SS_SetDocDefaultLogFont(LPSPREADSHEET lpSS, LOGFONT *pLogFont);
#endif
BOOL SSGetDocDefaultFont(HWND hWnd, LPSS_FONT Font);
void SSGetDocColor(HWND hWnd, LPCOLORREF Background, LPCOLORREF Foreground);
void SS_SetDocColor(LPSPREADSHEET lpSS, COLORREF Background,
                    COLORREF Foreground);
BOOL SSGetDocVisible(HWND hWnd);
HANDLE SS_GetTextFromClipboard(HWND hWnd);
BOOL SS_ValidatePasteData(HWND hWnd, LPSPREADSHEET lpSS, LPTSTR lpszText,
                          LPSS_CELLCOORD lpCellUL, LPSS_CELLCOORD lpCellLR,
                          BOOL fValidate);
void SS_ClearUndoBuffer(LPSPREADSHEET lpSS);
void SS_Undo(HWND hWnd, LPSPREADSHEET lpSS);
void SS_ClearColorItem(LPSS_COLORITEM lpColor, COLORREF Background,
                       COLORREF Foreground);
void SS_VBDelBlock(HWND hWnd, LPSPREADSHEET lpSS, WORD wInsDelOperation);

BOOL SS_GetActiveCell(LPSPREADSHEET lpSS, LPSS_COORD CellCol, LPSS_COORD CellRow);
BOOL SS_SetActiveCell(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow);

BOOL SS_GetDocDefaultCellType(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType);
BOOL SS_SetDocDefaultCellType(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType);

void SS_GetGrayAreaColor(LPSS_BOOK lpBook, LPCOLORREF lpBackground,
                         LPCOLORREF lpForeground);
void SS_SetGrayAreaColor(LPSS_BOOK lpBook, COLORREF Background,
                         COLORREF Foreground);

BOOL SS_GetEditMode(LPSS_BOOK lpBook);
BOOL SS_SetEditMode(LPSS_BOOK lpBook, BOOL fEditModeOn);

SS_COORD SS_GetMaxCols(LPSPREADSHEET lpSS);
void     SS_SetMaxCols(LPSPREADSHEET lpSS, SS_COORD MaxCols);

SS_COORD SS_GetMaxRows(LPSPREADSHEET lpSS);
void     SS_SetMaxRows(LPSPREADSHEET lpSS, SS_COORD MaxRows);

BOOL SS_GetSelectBlock(LPSPREADSHEET lpSS, LPSS_CELLCOORD CellUL,
                       LPSS_CELLCOORD CellLR);
BOOL SS_SetSelectBlock(LPSPREADSHEET lpSS, LPSS_CELLCOORD CellUL,
                       LPSS_CELLCOORD CellLR);

void SS_GetShadowColor(LPSPREADSHEET lpSS,
                       LPCOLORREF ShadowColor, LPCOLORREF ShadowText,
                       LPCOLORREF ShadowDark, LPCOLORREF ShadowLight);
void SS_SetShadowColor(LPSPREADSHEET lpSS,
                       COLORREF ShadowColor, COLORREF ShadowText,
                       COLORREF ShadowDark, COLORREF ShadowLight);

WORD SS_GetButtonDrawMode(LPSS_BOOK lpBook);
WORD SS_SetButtonDrawMode(LPSS_BOOK lpBook, WORD wMode);

#ifdef SS_V35
void SS_GetDocCellNote(LPSPREADSHEET lpSS, LPTSTR Note, short *pnLen);
void SS_SetDocCellNote(LPSPREADSHEET lpSS, LPTSTR Note);
#endif

BOOL SS_SetPrintAbortMsg(LPSS_BOOK lpBook, LPTSTR lpszText);
BOOL SS_SetPrintJobName(LPSS_BOOK lpBook, LPTSTR lpszText);

#ifdef __cplusplus
}
#endif

#endif
