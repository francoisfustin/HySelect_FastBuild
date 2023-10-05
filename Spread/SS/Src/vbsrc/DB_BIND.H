/************
* DB_BIND.H
************/

#ifndef DB_BIND_H
#define DB_BIND_H

#include "vbspread.h"

#define SSD_AUTOSAVE      1
#define SSD_AUTOHEADINGS  2
#define SSD_AUTOCELLTYPES 3
#define SSD_AUTOFILL      4
#define SSD_AUTOSIZECOLS  5

/*********************
* external functions
*********************/

long DBSS_GetRecs(HWND hWndSS, LPVBSPREAD lpSpread, LPDBINFO lpDBInfo,
                  long lStart, long lNumRecsNeeded, LPBOOL lpfAtTop,
                  LPBOOL lpfAtBottom);
int  DBSS_BindDatabase(HWND hWndSS, LPVBSPREAD lpSpread, LPDBINFO lpDBInfo,
                       LPSTR connection, LPSTR Query);
void DBSS_EditQuery(HWND hWndSS, LPVBSPREAD lpSpread, LPDBINFO lpDBInfo);
void DBSS_EndBind(HWND hWndSS, LPDBINFO lpDBInfo);
void DBSS_InsertRecord(LPDBINFO lpDBInfo, long Row);
void DBSS_MarkRecAsUpdated(HWND hWndSS, LPVBSPREAD lpSpread, LPDBINFO lpDBInfo,
                           long Col, long Row);
BOOL DBSS_PromptQueryString(HWND hWndSS, LPDBINFO lpDBInfo);
BOOL DBSS_ManualSave(HCTL hCtl, HWND hWnd, LPVBSPREAD lpSpread,
                     LPSPREADSHEET lpSS, SS_COORD Top, SS_COORD End);
BOOL DBSS_BoundDel(HWND hWndSS, LPVBSPREAD lpSpread, SS_COORD Row1,
                   SS_COORD Row2);
BOOL DBSS_BoundIns(HWND hWndSS, LPVBSPREAD lpSpread, SS_COORD Row1,
                   SS_COORD Row2);
BOOL DBSS_QueryConnect(HWND hWndSS);
BOOL DBSS_GetDataFillData(HCTL hCtl, LPVAR lpVar, LPSTR lpszText,
                          WORD wVarType);

#endif
