/*************
* SS_FORMU.H
*************/

#ifndef SS_FORMU_H
#define SS_FORMU_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************
* Declare function prototypes
******************************/

BOOL  SS_FreeFormula(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);
int SS_GetFormulaLen(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);
TBGLOBALHANDLE SS_GetFormula(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);
BOOL SS_SetFormula(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                   LPCTSTR Formula, BOOL BuildDependencies);
BOOL SS_SetFormulaRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                        SS_COORD Col2, SS_COORD Row2, LPCTSTR Formula,
                        BOOL BuildDependencies);

int SS_QueryCustomNameLen(LPSS_BOOK lpBook, LPCTSTR lpszPrevName);
int SS_QueryCustomName(LPSS_BOOK lpBook, LPCTSTR lpszPrevName,
                       LPTSTR lpszName, int nNameLen);
int SS_GetCustomNameLen(LPSS_BOOK lpBook, LPCTSTR lpszName);
int SS_GetCustomName(LPSS_BOOK lpBook, LPCTSTR lpszName,
                          LPTSTR lpszValue, int nValueLen);
BOOL SS_SetCustomName(LPSS_BOOK lpBook, LPCTSTR lpszName,
                           LPCTSTR lpszValue);
BOOL SS_GetIteration(LPSS_BOOK lpBook, LPSHORT lpnMaxIterations,
                     LPDOUBLE lpdfMaxChange);
BOOL SS_SetIteration(LPSS_BOOK lpBook, BOOL bIteration,
                     short nMaxIterations, double dfMaxChange);
short SS_GetRefStyle(LPSS_BOOK lpBook);
short SS_SetRefStyle(LPSS_BOOK lpBook, short nStyle);

#ifdef __cplusplus
}
#endif

#endif
