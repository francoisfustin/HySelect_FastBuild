/************
* SS_CELL.H
************/

#ifndef SS_CELL_H
#define SS_CELL_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************
* Declare function prototypes
******************************/

extern BOOL            SS_SetCellData(LPSPREADSHEET lpSS, SS_COORD CellCol,
                                      SS_COORD CellRow, LPTSTR Data, short Len);
extern BOOL            SSx_SetCellData(LPSPREADSHEET lpSS, SS_COORD CellCol,
                                       SS_COORD CellRow, LPTSTR Data, short Len,
                                       BOOL fFireChangeEvent);
extern BOOL            SS_SetCellDataItem(LPSPREADSHEET lpSS,
                                          SS_COORD CellCol, SS_COORD CellRow,
                                          LPSS_DATA lpDataItem);
extern BOOL            SSx_SetCellDataItem(LPSPREADSHEET lpSS,
                                           SS_COORD CellCol, SS_COORD CellRow,
                                           LPSS_DATA lpDataItem, BOOL fFireChangeEvent);
extern BOOL            SSxx_SetCellDataItem(LPSPREADSHEET lpSS,
                                            SS_COORD CellCol, SS_COORD CellRow,
                                            LPSS_DATA lpDataItem);
extern BOOL            SS_IsCellVisible(LPSPREADSHEET lpSS,
                                        SS_COORD CellCol, SS_COORD CellRow,
                                        short Visible);
extern void            SS_AdjustCellCoords(LPSPREADSHEET lpSS,
                                           LPSS_COORD CellCol,
                                           LPSS_COORD CellRow);
extern void            SS_AdjustCellRangeCoords(LPSPREADSHEET lpSS,
                                                LPSS_COORD lpCellCol,
                                                LPSS_COORD lpCellRow,
                                                LPSS_COORD lpCellCol2,
                                                LPSS_COORD lpCellRow2);
extern void            SS_AdjustCellCoordsOut(LPSPREADSHEET lpSS, LPSS_COORD lpCellCol, LPSS_COORD lpCellRow);
extern void            SS_LimitRangeCoords(LPSPREADSHEET lpSS,
                                           LPSS_COORD lplCol,
                                           LPSS_COORD lplRow,
                                           LPSS_COORD lplCol2,
                                           LPSS_COORD lplRow2);
extern BOOL            SS_ClearCell(LPSPREADSHEET lpSS, SS_COORD CellCol,
                                    SS_COORD CellRow, BOOL Paint,
                                    BOOL fSendChangeMsg);
extern BOOL SS_SetCellLock(LPSPREADSHEET lpSS,
                                      SS_COORD CellCol, SS_COORD CellRow,
                                      BOOL Lock);
extern BOOL SS_SetCellFont(LPSPREADSHEET lpSS,
                                      SS_COORD CellCol, SS_COORD CellRow,
                                      HFONT hFont, BOOL DeleteFont,
                                      LPBOOL lpfDeleteFontObject);
#if SS_V80
extern BOOL SS_SetCellLogFont(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow, LOGFONT *pLogFont);
#endif
extern BOOL SSSetCellCellType(HWND hWnd, LPSPREADSHEET lpSS,
                                         SS_COORD CellCol, SS_COORD CellRow,
                                         LPSS_CELLTYPE CellType);
extern BOOL SS_SetCellColor(LPSPREADSHEET lpSS,
                                       SS_COORD CellCol, SS_COORD CellRow,
                                       COLORREF Background,
                                       COLORREF Foreground);
extern BOOL            SSIsCellVisible(HWND hWnd, SS_COORD CellCol,
                                       SS_COORD CellRow, short Visible);
extern TBGLOBALHANDLE  SSx_GetData(LPSPREADSHEET lpSS, LPSS_COL lpCol,
                                   LPSS_ROW lpRow, LPSS_CELL lpCell,
                                   LPSS_CELLTYPE lpCellType, LPSS_DATA lpData,
                                   SS_COORD Col, SS_COORD Row, BOOL fValue);
extern BOOL            SS_GetDataStruct(LPSPREADSHEET lpSS, LPSS_COL lpCol,
                                        LPSS_ROW lpRow, LPSS_CELL lpCell,
                                        LPSS_DATA lpData, SS_COORD Col,
                                        SS_COORD Row);
BOOL SS_IsCellDataEmpty(LPSPREADSHEET lpSS, LPSS_COL lpCol, LPSS_ROW lpRow, LPSS_CELL lpCell,
                        SS_COORD Col, SS_COORD Row);

BOOL SS_SetCellCellType(LPSPREADSHEET lpSS,
                        SS_COORD CellCol, SS_COORD CellRow,
                        LPSS_CELLTYPE CellType);

BOOL SS_GetMaxTextCellSize(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                           LPDOUBLE lpdfWidth, LPDOUBLE lpdfHeight);
#ifdef SS_V35
extern BOOL   SS_SetCellCellNote(LPSPREADSHEET lpSS, SS_COORD CellCol,
                                 SS_COORD CellRow, LPCTSTR Note);
extern short  SS_GetCellNote(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow,
                             LPTSTR Note);
#endif

#ifdef SS_DDE
BOOL SS_CellSetDdeLink(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow,
                       LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem,
                       long lItemCol, long lItemRow);
BOOL SS_CellSaveDdeLink(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow);
#endif

#ifdef __cplusplus
}
#endif

#endif
