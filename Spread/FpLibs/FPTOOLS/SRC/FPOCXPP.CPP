/***********************************************************************
* FPOCXPP.CPP - Farpoint's implementation of OCX Property Pages in 
*               a seperate DLL.
*
* Copyright (C) 1991-1996 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/
#ifndef NO_MFC
#include <afxctl.h>                 // MFC support for OLE Custom Controls
#else
//#include "ctlcore.h"
#include "stdafx.h"
#endif
#include "windowsx.h"
#include "fptools.h"

#include "fptstr.h"
#include "fpocxpp.h"


// NOTE: If the Clsid cannot be found, a MessageBox will pop-up only if
//       "tstrDllName" is not null.  So, if you don't want a message
//       to pop-up, don't specify the DLL name.
FPLIB_(BOOL) fpVerifyCOMClsid(GUID FAR *lpClsid, LPCTSTR tstrDllName)
{
#define FP_CLSID_ERROR_MSG  \
"Error loading Dll.  Please register this Dll with the system."

  LPCLASSFACTORY pClassFactory = NULL;
  SCODE sc;

  sc = (SCODE)CoGetClassObject( *lpClsid, CLSCTX_SERVER,
         NULL, IID_IClassFactory, (LPVOID FAR *)&pClassFactory);
    
  if (FAILED(sc) || (pClassFactory == NULL))
  {
    // If tstrDllName is not NULL, pop-up dialog: "Error loading DLL."
    if (tstrDllName != NULL)
      MessageBox(NULL, _T(FP_CLSID_ERROR_MSG), (LPCTSTR)tstrDllName, MB_OK);
    return(FALSE);
  }
  else
  {
    pClassFactory->Release();
  }

  return TRUE;
}


