/***************
*  SS_BOOL.H
***************/

#ifndef SS_BOOL_H
#define SS_BOOL_H

#ifdef __cplusplus
extern "C" {
#endif

BOOL SS_GetBool(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, short nIndex);
BOOL SS_SetBool(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, short nIndex, BOOL fNewVal);

BOOL SS_BookSetRedraw(LPSS_BOOK lpBook, BOOL fRedraw);
BOOL SS_BookSetAutoCalc(LPSS_BOOK lpBook, BOOL fAutoCalc);

#ifdef __cplusplus
}
#endif

#endif
