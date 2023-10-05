/*************
* SS_EMODE.H
*************/

#ifndef SS_EMODE_H
#define SS_EMODE_H

#ifdef __cplusplus
extern "C" {
#endif

BOOL SS_CellEditModeOn(LPSPREADSHEET lpSS,
                       UINT Msg, WPARAM wParam, LPARAM lParam);
void SS_SizeExpandingEdit(LPSPREADSHEET lpSS, HWND hWndCtrl,
                          RECT Rect, LPSS_CELLTYPE lpCellType,
                          short x, short y, short cx, short cy,
                          LPRECT lpRectCurrent, TBGLOBALHANDLE hData);
BOOL SS_CellEditModeOff(LPSPREADSHEET lpSS, HWND hWndFocus);
BOOL SS_SetEditModeOnData(LPSPREADSHEET lpSS);
BOOL SS_GetCellBeingEdited(LPSPREADSHEET lpSS, LPSS_COORD lpColEdit, LPSS_COORD lpRowEdit);

#ifdef __cplusplus
}
#endif

#endif
