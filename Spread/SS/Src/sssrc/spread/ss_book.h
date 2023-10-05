/************
* SS_USER.H
************/

#ifndef SS_BOOK_H
#define SS_BOOK_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SS_V70

extern short SS_GetSheetCount(LPSS_BOOK lpBook);
extern short SS_GetActiveSheet(LPSS_BOOK lpBook);
extern BOOL SS_SetActiveSheet(LPSS_BOOK lpBook, short nSheet);
extern short SS_GetTabStripLeftSheet(LPSS_BOOK lpBook);
extern BOOL SS_SetTabStripLeftSheet(LPSS_BOOK lpBook, short nSheet);
extern BOOL SS_GetTabStripRect(LPSS_BOOK lpBook, LPRECT lpRect);
extern BOOL SS_IsPointInTabStripRect(LPSS_BOOK lpBook, int x, int y);
extern BOOL SS_GetTabSplitBoxRect(LPSS_BOOK lpBook, LPRECT lpRect);
extern BOOL SS_IsPointInTabSplitBoxRect(LPSS_BOOK lpBook, int x, int y);
extern void SS_EmptyTabStripRect(LPSS_BOOK lpBook);
extern BOOL SS_IsTabStripVisible(LPSS_BOOK lpBook);
extern void SS_InvalidateTabStrip(LPSS_BOOK lpBook);
extern HFONT SS_GetTabStripFont(LPSS_BOOK lpBook);
extern BOOL SS_SetTabStripFont(LPSS_BOOK lpBook, HFONT hFont);
extern TBGLOBALHANDLE SS_GetSheetName(LPSS_BOOK lpBook, short nSheet);
extern BOOL SS_SetSheetName(LPSS_BOOK lpBook, short nSheet, LPCTSTR lpszSheetName);
extern WORD SS_GetTabStripPolicy(LPSS_BOOK lpBook);
extern WORD SS_SetTabStripPolicy(LPSS_BOOK lpBook, WORD wTabStripPolicy);
extern double SS_GetTabStripRatio(LPSS_BOOK lpBook);
extern double SS_SetTabStripRatio(LPSS_BOOK lpBook, double nTabStripRatio);
extern void SS_TabResize(LPSPREADSHEET lpSS, int iPos);
extern BOOL SS_SetSheetVisible(LPSS_BOOK lpBook, short nSheet, BOOL fVisible);
extern BOOL SS_GetSheetVisible(LPSS_BOOK lpBook, short nSheet);

extern void SS_FireSheetChanging(LPSS_BOOK lpBook, short nOldSheet, short nNewSheet, LPBOOL lpbCancel);
extern void SS_FireSheetChanged(LPSS_BOOK lpBook, short nOldSheet, short nNewSheet);
extern void SS_FireTabScrolling(LPSS_BOOK lpBook, short nOldLeftSheet, short nNewLeftSheet);
extern void SS_FireTabScrolled(LPSS_BOOK lpBook, short nOldLeftSheet, short nNewLeftSheet);

void SS_SetActiveSheetIndex(LPSS_BOOK lpBook, short nActiveSheet);

#endif // SS_V70
#ifdef SS_V80
extern WORD SS_GetTabStripButtonPolicy(LPSS_BOOK lpBook);
extern WORD SS_SetTabStripButtonPolicy(LPSS_BOOK lpBook, WORD wTabStripButtonPolicy);
#endif

#ifdef __cplusplus
}
#endif

#endif
