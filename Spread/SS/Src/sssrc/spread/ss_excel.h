/************
* SS_EXCEL.H
************/
#ifndef SS_EXCEL_H
#define SS_EXCEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\excel\src\xl.h"
#include "..\..\excel\src\xlbiff.h"
#include "..\..\excel\src\xlutl.h"
//#include "..\..\excel\src\biff.h"
#include "xlcall.h"  //Excel include file

BOOL    SS_ExcelTableAlloc(void);
void    SS_ExcelTableDelete();
LPSSXL  SS_ExcelTableLock(void);
void    SS_ExcelTableUnlock(void);

BOOL SS_FreeXLWSMemory(LPVOID lpXL);
BOOL SS_FreeXLWBMemory(LPVOID lpXL);

#ifdef SS_V70
BOOL DLLENTRY SS_LoadXLBook(LPSS_BOOK lpBook, LPCTSTR fileName, LPCTSTR logFileName);
#endif
#ifdef SS_V80
#ifdef XL12
//BOOL SS_LoadXL12File(LPFPCONTROL spread, LPCTSTR fileName);
//BOOL SS_SaveXL12File(LPFPCONTROL spread, LPCTSTR fileName);
BOOL SS_OpenExcel2007File(LPFPCONTROL spread, LPCTSTR fileName, LPCTSTR password, short sheet, short excelsheet, LPCTSTR logfilename);
BOOL SS_SaveExcel2007File(LPFPCONTROL spread, LPCTSTR fileName, LPCTSTR password, short flags, LPCTSTR logfilename);
short SS_IsExcel2007File(LPFPCONTROL spread, LPCTSTR fileName);
#endif // XL12
#endif // SS_V80
BOOL DLLENTRY SS_LoadXLSheet(LPSS_BOOK lpBook, short sWBHandle, short sSheetIndex, LPCTSTR lpszSheetName);
BOOL DLLENTRY SS_GetExcelSheetList(LPSS_BOOK lpBook, LPCTSTR lpszFileName, GLOBALHANDLE *lpghList, LPSHORT lpsListCount, LPCTSTR lpszLogFileName, LPSHORT lpWorkbookId, BOOL fReplace);
BOOL SS_XL2Spread(LPCTSTR lpszFileName, LPVOID lp, LPSS_BOOK lpBook, short sSheetNum);
BOOL SS_SaveXLFile(LPSS_BOOK lpBook, LPCTSTR lpFileName, LPCTSTR lpSheetName, LPCTSTR lpLogFileName, LPTBGLOBALHANDLE lpghBuffer, LPLONG lplBufferLen, BOOL fToFile, SHORT sFlags);
BOOL SS_RetrieveBiffBuffer(LPSS_BOOK lpBook, LPCTSTR lpszSheetName, LPCTSTR lpszLogFileName, LPGLOBALHANDLE lpghBuffer, LPLONG lplBufferLen);
BOOL SS_Spread2XL(LPVOID lp, LPSPREADSHEET lpSS, short sSheetNum);
BOOL SS_GetXLSheetList(LPVOID lp, GLOBALHANDLE FAR *lpghList, LPSHORT lpsCount);

#ifdef __cplusplus
}
#endif

#endif
