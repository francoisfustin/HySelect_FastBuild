/************
* SS_WIN.H
************/

#ifndef SS_WIN_H
#define SS_WIN_H

#ifdef __cplusplus
extern "C" {
#endif

void SS_InvalidateRect(LPSS_BOOK lpBook, const RECT FAR* lprc, BOOL fErase);
void SS_UpdateWindow(LPSS_BOOK lpBook);
void SS_GetWinClientRect(LPSS_BOOK lpBook, LPRECT lpRect);
void SS_GetWindowRect(LPSS_BOOK lpBook, LPRECT lpRect);
HDC  SS_GetDC(LPSS_BOOK lpBook);
int  SS_ReleaseDC(LPSS_BOOK lpBook, HDC hDC);

#ifdef __cplusplus
}
#endif

#endif
