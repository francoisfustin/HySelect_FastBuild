/*************************************************************************\
* FPDBMFC.H  - FarPoint Data Binding include file for ADO Binding.        *
*                                                                         *
* Copyright (C) 1991-1998 - FarPoint Technologies, Inc.                   *
* All Rights Reserved.                                                    *
*                                                                         *
* No part of this source code may be copied, modified or reproduced       *
* in any form without retaining the above copyright notice.  This         *
* source code, or source code derived from it, may not be redistributed   *
* without express written permission of FarPoint Technologies.            *
*                                                                         *
\*************************************************************************/


#ifndef FPDBMFC_H
#define FPDBMFC_H

#ifdef FP_ADB
BOOL __cdecl fpMFCGetListOfColumnNames(CfpOleBinding *pfpOleBindObject,
  CStringArray *pStringArray, CDWordArray *pCookieArray);

BOOL __cdecl fpMFCGetListOfDataMemberNames(
  CfpOleBinding *pfpOleBindObject, CStringArray *pStringArray, 
  CDWordArray *pCookieArray);
#endif

#endif
