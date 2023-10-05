//--------------------------------------------------------------------
//
// File: ss_xl12.h
//
// Copyright (C) 2009 FarPoint Technologies.
// All rights reserved.
//
//--------------------------------------------------------------------

#if defined(SS_V80) && defined(XL12)
#include "windows.h"
#include "spread.h"
#include "fptools.h"

#ifdef __cplusplus
extern "C"
{
#endif

BOOL xl_OpenExcel2007File(LPFPCONTROL spread, LPCTSTR fileName, LPCTSTR password, short sheet, short excelsheet, LPCTSTR logfilename);
BOOL xl_SaveExcel2007File(LPFPCONTROL spread, LPCTSTR fileName, LPCTSTR password, short flags, LPCTSTR logfilename);
short xl_IsExcel2007File(LPFPCONTROL spread, LPCTSTR fileName);
BOOL xl_IsExcel2007Supported();

#ifdef __cplusplus
}
#endif

#endif // SS_V80 && XL12
