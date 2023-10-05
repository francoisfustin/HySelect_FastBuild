/*************
* VBPRINT2.H
*************/

#ifndef VBPRINT2_H
#define VBPRINT2_H

#ifdef __cplusplus
extern "C" {
#endif

//#define PT_TO_PIXELS_EX(hdc, pt) MulDiv(pt, GetDeviceCaps(hdc, LOGPIXELSY), 72)
#define PT_TO_PIXELS_EX(hdc, pt)   (short)(((double)(pt) * (double)GetDeviceCaps(hdc, LOGPIXELSY) / 72.0) + 0.5);

#define PRINTABORTTABLE_MAX    50
#define PRINTABORTDLG_HMARGIN  16
#define PRINTABORTDLG_VMARGIN1 10
#define PRINTABORTDLG_VMARGIN2 12
#define PRINTABORTDLG_VMARGIN3 12

typedef struct PrintAbortTable
   {
   HDC  hDC;
   HWND hWndDlg;
   HWND hWndSpread;
   BOOL fAbort;
   BOOL fDisabled;
   } PRINTABORTTABLE, FAR *LPPRINTABORTTABLE;

typedef struct PrintAbortParam
   {
   TBGLOBALHANDLE hAbortMsg;
   HWND           hWndSpread;
   } PRINTABORTPARAM, FAR *LPPRINTABORTPARAM;

extern PRINTABORTTABLE PrintAbortTable[PRINTABORTTABLE_MAX];
extern short           PrintAbortTableCnt;

DLGENTRY vbSSPrintAbortProc(HDC hPrnDC, int nCode);
DLGENTRY vbSSPrintDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
short vbSpreadPrintHeaderFooter(HDC hDC, GLOBALHANDLE hText, short nPageNum,
                                float fPrintMarginRight, BOOL fCalc,
                                LPSS_PRINTINFO lpPrintInfo);
GLOBALHANDLE vbSSCreatePrintString(LPTSTR lpStart, short Len, short nPageNum,
                                   long lPageCount, LPSS_PRINTINFO lpPrintInfo);
BOOL vbSSGetPrintAbortItem(LPPRINTABORTTABLE lpPrintAbortItem,
                           HDC hDC, HWND hWndDlg);
void vbSSCancelPrintAbortItem(HWND hWndDlg);
void vbSSDeletePrintAbortItem(HWND hWndDlg);
void vbSSReplacePrintAbortItem(HWND hWndSpread, HDC hOldDC, HDC hNewDC);
void vbSSAddPrintAbortItem(HWND hWndDlg, HDC hDC);

#ifdef __cplusplus
}
#endif

#endif
