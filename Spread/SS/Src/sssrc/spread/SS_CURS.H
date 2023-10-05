/************
* SS_CURS.H
************/

#ifndef SS_CURS_H
#define SS_CURS_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************
* Declare function prototypes
******************************/

HCURSOR SS_CreateCursorPointer(HANDLE hInstance);
HCURSOR SS_CreateCursorResizeh(HANDLE hInstance);
HCURSOR SS_CreateCursorResizev(HANDLE hInstance);

#ifdef __cplusplus
}
#endif

#endif
