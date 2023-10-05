/***************
*  SS_BORD.H
***************/

#ifndef SS_BORD_H
#define SS_BORD_H

#ifdef __cplusplus
extern "C" {
#endif

BOOL SS_GetBorder(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                  LPWORD lpwBorderType, LPWORD lpwBorderStyle,
                  LPCOLORREF lpColor);
BOOL SS_GetBorderEx(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                    LPWORD lpwStyleLeft, LPCOLORREF lpclrLeft,
                    LPWORD lpwStyleTop, LPCOLORREF lpclrTop,
                    LPWORD lpwStyleRight, LPCOLORREF lpclrRight,
                    LPWORD lpwStyleBottom, LPCOLORREF lpclrBottom);
BOOL SSx_GetBorder(LPSPREADSHEET lpSS, LPSS_COL lpCol, LPSS_ROW lpRow,
                   LPSS_CELL lpCell, SS_COORD Col, SS_COORD Row,
                   LPWORD lpwStyleLeft, LPCOLORREF lpclrLeft,
                   LPWORD lpwStyleTop, LPCOLORREF lpclrTop,
                   LPWORD lpwStyleRight, LPCOLORREF lpclrRight,
                   LPWORD lpwStyleBottom, LPCOLORREF lpclrBottom);
BOOL SS_SetBorder(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                  WORD wBorderType, WORD wBorderStyle, COLORREF Color);
BOOL SS_SetBorderItem(LPSPREADSHEET lpSS, LPTBGLOBALHANDLE lphBorder,
                      WORD wBorderType, WORD wBorderStyle, COLORREF Color);
void SS_FreeBorder(LPSPREADSHEET lpSS, LPTBGLOBALHANDLE lphBorder);

#ifdef __cplusplus
}
#endif

#endif
