/************
* SS_DRAG.H
************/

#ifndef SS_DRAG_H
#define SS_DRAG_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************
* Declare function prototypes
******************************/

extern BOOL SS_IsMouseInDragDrop(HWND hWnd, LPSPREADSHEET lpSS,
                                 LPRECT RectClient, int MouseX,
                                 int MouseY, LPSS_COORD lpCol,
                                 LPSS_COORD lpRow);
extern void SS_DragDropDrawBox(HWND hWnd, LPSPREADSHEET lpSS, BOOL fBoxOn);
extern void SS_DragDropFinish(HWND hWnd, LPSPREADSHEET lpSS);
extern void SS_AddDragDropRectToBlock(LPSPREADSHEET lpSS, HRGN hRgn);
extern HRGN SS_CreateDragDropRect(LPSPREADSHEET lpSS);
extern BOOL SS_GetDragDropRect(LPSPREADSHEET lpSS, LPRECT lpRect,
                               LPRECT lpRect2, SS_COORD Col, SS_COORD Row,
                               SS_COORD Col2, SS_COORD Row2, short dOffset,
                               short dOffset2, short dOffset3);

#ifdef __cplusplus
}
#endif

#endif
