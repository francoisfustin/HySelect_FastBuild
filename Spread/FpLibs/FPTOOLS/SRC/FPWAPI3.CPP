/***************************************************************************/
/* FPWAPI3.CPP   FarPoint Support for Windows API in non-hWnd OCX's        */
/***************************************************************************/

#ifdef STRICT
#undef STRICT
#endif

#ifdef FP_OCX
#ifndef NO_MFC
#include <afxctl.h>                 // MFC support for OLE Custom Controls
#else
#include "stdafx.h"
#include "fpatl.h"
#endif
#endif

#include "fptools.h"                             

#ifdef __cplusplus
extern "C" {
#endif

LONG FPLIB fpPictureFromHolder(LONG lPict)
{      
#ifdef FP_OCX  
  CPictureHolder *pObject;
  if (lPict)
  {                    
    pObject = (CPictureHolder *)lPict;  

    if (!fpIsPictureEmpty(lPict))
       return (LONG)(pObject->GetPictureDispatch());
  }
  
  return 0L;
  
#else
  return lPict;
#endif
}


HANDLE FPLIB fpAttachPicture(LPHANDLE lPict, LPARAM lParam)
{
#ifdef FP_OCX  
  CPictureHolder *pObject;
  if (!(*lPict))
  {
    pObject = new CPictureHolder();
    (*lPict) = (HANDLE)pObject;
  }  
  else
    pObject = (CPictureHolder *)(*lPict);  
  pObject->SetPictureDispatch((LPPICTUREDISP)lParam);
  return (*lPict);
#else
  (*lPict) = (HANDLE)lParam;
  return (*lPict);  
#endif
}                           

void FPLIB fpDetachPicture(LPHANDLE lPict)
{      
#ifdef FP_OCX  
  CPictureHolder *pObject;
  if ((lPict) && (*lPict))
  {
    pObject = (CPictureHolder *)(*lPict);

// this code is not needed (GetPictureDispatch adds a ref, so this is just
// adding a ref and then releasing it again -scl
//    if (!fpIsPictureEmpty(*lPict))
//       (pObject->GetPictureDispatch())->Release();

    delete pObject;	// this releases the ref -scl
    (*lPict) = NULL;
  }  
#else
  (*lPict) = 0L;
#endif
}

BOOL FPLIB fpIsPictureEmpty(LONG lPict)
{
#ifdef FP_OCX  
  CPictureHolder *pObject;
  short sType;
  
  if (lPict)
  {                    
    pObject = (CPictureHolder *)lPict;  
    sType = pObject->GetType();
    if ((sType == PICTYPE_NONE) || (sType == PICTYPE_UNINITIALIZED))
      return TRUE;
    else
      return FALSE;
  }
  else
    return TRUE;  
#else
  return (lPict == 0L ? TRUE : FALSE);
#endif
}

#ifdef __cplusplus
}
#endif
