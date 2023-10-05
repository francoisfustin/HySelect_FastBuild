/*************
* SS_ALLOC.H
*************/

#ifndef SS_ALLOC_H
#define SS_ALLOC_H

#include "ss_heap.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
#define SS_UnlockColItem(lpSS, dCol)                TRUE
#define SS_UnlockRowItem(lpSS, dRow)                TRUE
#define SS_UnlockCellItem(lpSS, dCol, dRow)         TRUE
#define SSx_UnlockCellItem(lpSS, lpRow, dCol, dRow) TRUE
*/

#ifndef SS_NO_USE_SH
#define SS_UnlockColItem(lpSS, dCol)
#define SS_UnlockRowItem(lpSS, dRow)
#define SS_UnlockCellItem(lpSS, dCol, dRow)
#define SSx_UnlockCellItem(lpSS, lpRow, dCol, dRow)
#else
BOOL SS_UnlockColItem(LPSPREADSHEET lpSS, SS_COORD dCol);
BOOL SS_UnlockRowItem(LPSPREADSHEET lpSS, SS_COORD dRow);
BOOL SS_UnlockCellItem(LPSPREADSHEET lpSS, SS_COORD dCol, SS_COORD dRow);
BOOL SSx_UnlockCellItem(LPSPREADSHEET lpSS, LPSS_ROW lpRow, SS_COORD dCol, SS_COORD dRow);
#endif

/******************************
* Declare function prototypes
******************************/

extern BOOL           SS_Alloc(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);
extern BOOL           SS_AllocRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2);
extern TBGLOBALHANDLE SS_AllocCol(LPSPREADSHEET lpSS, SS_COORD CellCol);
extern TBGLOBALHANDLE SS_AllocRow(LPSPREADSHEET lpSS, SS_COORD CellRow);
extern TBGLOBALHANDLE SS_AllocRowCells(LPSPREADSHEET SpreadSheet,
                                       SS_COORD Row);
extern TBGLOBALHANDLE SS_AllocCell(LPSPREADSHEET SpreadSheet,
                                   SS_COORD CellCol, SS_COORD CellRow);
extern BOOL           SS_DeAllocCol(LPSPREADSHEET lpSS, SS_COORD Col,
                                    BOOL fSendChangeMsg);
extern void           SSx_FreeCol(LPSPREADSHEET lpSS, LPSS_COL lpCol);
extern BOOL           SS_DeAllocRow(LPSPREADSHEET lpSS, SS_COORD Row,
                                    BOOL fSendChangeMsg);
extern void           SSx_FreeRow(LPSPREADSHEET lpSS, LPSS_ROW lpRow);
extern BOOL           SS_DeAllocCell(LPSPREADSHEET, SS_COORD CellCol,
                                   SS_COORD CellRow, BOOL fSendChangeMsg);
extern void           SSx_FreeCell(LPSPREADSHEET lpSS, LPSS_CELL lpCell);
extern BOOL           SS_IsDestroyed(HWND hWnd);
extern LRESULT        SS_SendMsg(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam,
                                 LPARAM lParam);
extern LRESULT        SS_SendMsgActiveSheet(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
extern LRESULT        SSx_SendMsgActiveSheet(LPSS_BOOK lpBook, UINT Msg, WPARAM wParam, LPARAM lParam);
extern LRESULT        SSx_SendMsg(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, UINT Msg,
                                  WPARAM wParam, LPARAM lParam);
extern LRESULT        SS_PostMsg(LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam,
                                 LPARAM lParam);
extern LRESULT        SS_SendMsgCommand(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, WORD wCmd, BOOL fNoMsgCnt);
extern LRESULT        SS_SendMsgCommandActiveSheet(HWND hWnd, WORD wCmd, BOOL fNoMsgCnt);
extern LRESULT        SS_SendMsgColCoordRange(LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, SS_COORD Col, SS_COORD Col2);
extern LRESULT        SS_SendMsgRowCoordRange(LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, SS_COORD Row, SS_COORD Row2);
extern LRESULT        SS_SendMsgCoords(LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam,
                                       SS_COORD Col, SS_COORD Row);
extern LRESULT        SS_SendMsgEnterRow(LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, SS_COORD Row,
                                         BOOL fNewRow);
extern LRESULT        SS_SendMsgInsDel(LPSPREADSHEET lpSS, WORD wType, WORD wOperation,
                                       WORD wDirection, SS_COORD Coord1,
                                       SS_COORD Coord2);
extern LRESULT        SS_SendMsgComboCloseUp(LPSPREADSHEET lpSS, short dSelChange,
                                             SS_COORD Col, SS_COORD Row);
extern LRESULT        SS_SendMsgBeforeEditMode(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                                               WORD wUserAction, LPWORD lpwCursorPos, LPBOOL lpfCancel);
extern LRESULT        SS_SendMsgBeforeScrollTip(LPSPREADSHEET lpSS, BOOL fIsVertical, SS_COORD Index,
                                                LPTSTR lpText, LPSHORT lpnWidth);
extern BOOL           SS_SendMsgBeforeColMove(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Col2,
                                              SS_COORD ColDest);
extern BOOL           SS_SendMsgBeforeRowMove(LPSPREADSHEET lpSS, SS_COORD Row, SS_COORD Row2,
                                              SS_COORD RowDest);

extern HPBYTE         SS_HugeBufferAlloc(HPBYTE Buffer, LPLONG BufferLen,
                                         LPVOID Value, long ValueLen,
                                         LPLONG AllocLen, LPHANDLE hBuffer);
extern void           SS_FreeWithHwnd(HWND hwnd);
extern void           SS_FreePostHwnd(TBGLOBALHANDLE hBook);
extern void           SS_ClearAll(LPSS_BOOK lpBook);
extern void           SS_ClearSheet(LPSPREADSHEET lpSS, BOOL bFreeCalcInfo);
extern void           SS_DeAllocCellType(LPSPREADSHEET SpreadSheet,
                                         TBGLOBALHANDLE hCellType);

extern LPSS_COL       SS_LockColItem(LPSPREADSHEET lpSS,
                                     SS_COORD dCol);
extern LPSS_COL       SS_AllocLockCol(LPSPREADSHEET lpSS, SS_COORD dCol);
extern LPSS_ROW       SS_LockRowItem(LPSPREADSHEET lpSS, SS_COORD dRow);
extern LPSS_ROW       SS_AllocLockRow(LPSPREADSHEET lpSS, SS_COORD dRow);
extern LPSS_CELL      SS_LockCellItem(LPSPREADSHEET lpSS,
                                      SS_COORD dCol, SS_COORD dRow);
extern LPSS_CELL      SSx_LockCellItem(LPSPREADSHEET lpSS,
                                       LPSS_ROW lpRow, SS_COORD dCol,
                                       SS_COORD dRow);
extern LPSS_CELL      SS_AllocLockCell(LPSPREADSHEET lpSS, LPSS_ROW lpRow,
                                       SS_COORD dCol, SS_COORD dRow);
extern BOOL           SS_MoveAllocItems(LPSPREADSHEET lpSS,
                                        OMEM_POOL OmemPool,
                                        LPSS_ALLOCITEM lpAllocItem,
                                        SS_COORD wItemNumSrc1,
                                        SS_COORD wItemNumSrc2,
                                        SS_COORD wItemNumDest,
                                        SS_COORD wMaxItems);
extern void           SS_FreeCalcList(LPSS_CALCLIST lpCalcList);
extern void           SSx_FreeData(LPSS_DATA lpData);
TBGLOBALHANDLE        SS_AllocItems(LPSPREADSHEET lpSS, OMEM_POOL OmemPool,
                                    LPSS_ALLOCITEM lpAllocItem,
                                    SS_COORD wItemNum, WORD wItemSize,
                                    TBGLOBALHANDLE hItem, SS_COORD wMaxItems,
                                    LPBOOL lpfIsNew);
TBGLOBALHANDLE        SS_GrabItemHandle(LPSPREADSHEET lpSS,
                                        LPSS_ALLOCITEM lpAllocItem,
                                        SS_COORD wItemNum, SS_COORD wMaxItems);
TBGLOBALHANDLE        SSx_AllocList(TBGLOBALHANDLE hList, LPSHORT lpdItemCnt,
                                    short dItemSize);
void                  SS_PicRef(LPSS_BOOK lpBook, HANDLE hPic);
void                  SS_PicDeref(LPSS_BOOK lpBook, HANDLE hPic);
BOOL                  SS_DeleteItem(LPSPREADSHEET lpSS,
                                    LPSS_ALLOCITEM lpAllocItem,
                                    SS_COORD wItemNum, SS_COORD wMaxItems);
SS_COORD              SS_GetRowAllocCnt(LPSPREADSHEET lpSS, SS_COORD Row);
TBGLOBALHANDLE        SS_GetItem(LPSPREADSHEET lpSS,
                                 LPSS_ALLOCITEM lpAllocItem,
                                 register SS_COORD wItemNum,
                                 SS_COORD wMaxItems);
void                  SS_PostSetEditMode(LPSPREADSHEET lpSS, BOOL fOn,
                                         BOOL fDontSendEditModeMsg);

#ifdef __cplusplus
}
#endif

#endif
