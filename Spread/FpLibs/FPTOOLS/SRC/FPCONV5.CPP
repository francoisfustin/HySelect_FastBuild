/***************************************************************************/
/* FPCONV5.CPP   FarPoint Support for VBX functions in non-VB environments */
/***************************************************************************/

#ifdef STRICT
#undef STRICT
#endif

#include <string.h>

#ifdef FP_OCX
#ifndef NO_MFC
#include <afxctl.h>                 // MFC support for OLE Custom Controls
#else
//#include "ctlcore.h"
#include <crtdbg.h>
#include "stdafx.h"
#include "fpatl.h"
#endif

#ifdef WIN32

#ifndef NO_MFC
#include <afxpriv.h>
#else
#if (defined( __T) && !defined(_INC_TCHAR))
#undef __T
#endif
#endif

#include "tchar.h"
#else

#define WM_NOTIFY                       0x004E

#define USES_CONVERSION int _convert; _convert

#define T2OLE  
#define OLE2T

#define NON_STANDARD_INDEX  12

#endif

#include "vbinterf.h"
// following lines taken from VBINTERF.H
#include <initguid.h>

#endif

#include "fptools.h"                             
#include "fptstr.h"

#ifdef __cplusplus
extern "C" {
#endif

void FPLIB fpVBRefreshBrowser(LPFPCONTROL lpObject)                          
{
#ifdef FP_OCX
#ifndef NO_MFC
  // OCX - only function
  CObject *pObject = (CObject *)lpObject->lObject;
  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(COleControl))))
      ((COleControl *)pObject)->BoundPropertyChanged(DISPID_UNKNOWN);
#else	   // NO_MFC
  // OCX - only function
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  

  if (pObject)
      pObject->fpAtlBoundPropertyChanged(DISPID_UNKNOWN);
  else
  {
      if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
      {
//		  pObject = COleControl::ControlFromHwnd(lpObject->hWnd);
          _ASSERT(0);  // No support for this case (ATL).  SCP 7/30/97
      }
  }

#endif
#endif
// there is no VB3 or prior equivalent
}            

                
USHORT FPLIB fpVBGetMode(LPFPCONTROL lpObject)
{
#ifdef FP_OCX
#ifndef NO_MFC
	
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(COleControl))))
  {
    if (((COleControl *)pObject)->AmbientUserMode() == FALSE) 
      return MODE_DESIGN;
    else
      return MODE_RUN;
  }
  else
  {
    return MODE_RUN;  //default to run-time
  }
#else    // NO_MFC
  
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  

  if (pObject)
  {
    if (pObject->fpAtlGetUserMode() == FALSE) 
      return MODE_DESIGN;
    else
      return MODE_RUN;
  }
  else
  {
    if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd)) 
    {
//	  pObject = COleControl::ControlFromHwnd(lpObject->hWnd);
      _ASSERT(0);  // No support for this case (ATL).  SCP 7/30/97
    }

    return MODE_RUN;  //default to run-time
  }

#endif
#else 
  return VBGetMode();
#endif
      
}

LPMODEL FPLIB fpVBGetControlModel(LPFPCONTROL lpObject)                         
{
#ifdef FP_OCX
  return NULL;  
#else      
  return VBGetControlModel((HCTL)lpObject->hCtl);
#endif
} 

HSZ FPLIB fpVBCreateHsz(LPFPCONTROL lpObject, LPTSTR lpszString)
{
#ifdef FP_OCX
  return (HSZ)fpStr2OLE(lpszString);
#else
  return VBCreateHsz((_segment)((HCTL)lpObject->hCtl), lpszString);
#endif
}

void FPLIB fpVBFreePic(LONG hPicture)
{       
#ifdef FP_OCX              
  CPictureHolder *pObject;

  if (hPicture)
	 {
     pObject = (CPictureHolder *)hPicture;

     delete pObject;
	 }

  return;
#else
  VBFreePic((HPIC)hPicture);
#endif  
}

HLSTR FPLIB fpVBCreateHlstr(LPVOID pb, USHORT cbLen)
{
#ifdef FP_OCX
  return (HLSTR)SysAllocStringLen((OLECHAR FAR *)pb, cbLen);
#else
  return VBCreateHlstr(pb, cbLen);
#endif
}

HSZ FPLIB fpVBDestroyHsz(HSZ hszParm)
{
#ifdef FP_OCX
  SysFreeString((BSTR)hszParm);
  return NULL;
#else   
  return VBDestroyHsz(hszParm);
#endif
}

USHORT FPLIB fpVBGetHlstrLen(HLSTR hlstr)
{
#ifdef FP_OCX
  return SysStringLen((BSTR)hlstr);
#else   
  return VBGetHlstrLen(hlstr);
#endif
}

SHORT FPLIB fpVBGetVersion()
{
#ifdef FP_OCX    
  return OLE_VERSION;
#else
  return VBGetVersion();
#endif
}

LONG FPLIB fpVBGetCapture()
{
#ifdef FP_OCX    
  return (LONG)(LPVOID)::GetCapture(); 
#else
  return (LONG)VBGetCapture();
#endif
}

LPSTR FPLIB fpVBDerefHsz(HSZ hsz)
{
#ifdef FP_OCX    
  return (LPSTR)hsz;
#else
  return VBDerefHsz(hsz);
#endif
}

LPSTR FPLIB fpVBDerefZeroTermHlstr(HLSTR hlstr)
{
#ifdef FP_OCX    
  return (LPSTR)hlstr;
#else
  return VBDerefZeroTermHlstr(hlstr);
#endif
}

LONG FPLIB fpVBDefControlProc(LPFPCONTROL lpObject, USHORT usMsg, WPARAM wp, LPARAM lp)
{
#ifdef FP_OCX    
  return DefWindowProc(lpObject->hWnd, usMsg, wp, lp);
#else
  return VBDefControlProc((HCTL)lpObject->hCtl, lpObject->hWnd, usMsg, wp, lp);    
#endif
}
      
void FPLIB fpVBUpdateControl(LPFPCONTROL lpObject)                           
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);
  
  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
    ((CWnd *)pObject)->UpdateWindow();
#else	   // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndTemp = NULL;

  if (lpObject->hWnd)
	  hWndTemp = lpObject->hWnd;
  
  if (pObject)
	 hWndTemp = pObject->fpAtlGetHWnd();

  if (hWndTemp && IsWindow(hWndTemp))
    UpdateWindow(hWndTemp);

#endif
#else
  VBUpdateControl((HCTL)lpObject->hCtl);                           
#endif
}    

LPVOID FPLIB fpVBDerefControl(LPFPCONTROL lpObject)
{
#ifdef FP_OCX
  // this function should return programmer-defined structure pointer
  // each control should override this to provide their own implementation
  // for OCX's ... therefore ASSERT
#ifndef NO_MFC
  ASSERT(FALSE);	
#else	   // NO_MFC
  _ASSERT(FALSE);	
#endif
  return (LPVOID)lpObject->lObject;
#else
  return VBDerefControl((HCTL)lpObject->hCtl);                            
#endif
}

LONG FPLIB fpVBGetHwndControl(LPFPCONTROL lpObject)                          
{
#ifdef FP_OCX
#ifndef NO_MFC
  if ((lpObject) && (lpObject->hWnd))
    return (LONG)CWnd::FromHandlePermanent(lpObject->hWnd);
//    return (LONG)CWnd::FromHandle(lpObject->hWnd);
  else
    return NULL;
#else	       // NO_MFC
  if ((lpObject) && (lpObject->hWnd))
  {
//  return (LONG)COleControl::ControlFromHwnd(lpObject->hWnd);
    _ASSERT(0);  // this feature is not supported in ATL.
    return NULL;
  }
  else
    return NULL;
#endif 
#else 
  return (LONG)VBGetHwndControl(lpObject->hWnd);
#endif
}

LONG FPLIB fpVBSendControlMsg(LPFPCONTROL lpObject, USHORT usMsg, WPARAM wp, LPARAM lp)           
{
#ifdef FP_OCX
  return fpSendMessage(lpObject, usMsg, wp, lp);
#else      
  return VBSendControlMsg((HCTL)lpObject->hCtl, usMsg, wp, lp);
#endif
}            

HANDLE FPLIB fpVBGetPic(HANDLE hpic, LPPIC lppic)                           
{
#ifdef FP_OCX
  CPictureHolder *pPicture;
  short sType;
  OLE_HANDLE hHandle,hHandPal;

  if (hpic)
  {
    pPicture = (CPictureHolder *)hpic;
	if (pPicture->m_pPict)
	{
	  pPicture->m_pPict->get_Type((short FAR *)&sType);
      pPicture->m_pPict->get_Handle((OLE_HANDLE FAR *)&hHandle);
	}
    else
	 sType = PICTYPE_NONE; 
	
	lppic->picType = (BYTE)sType;
  
    switch (sType)
    {           
      case PICTYPE_BITMAP:
        lppic->picData.bmp.hbitmap = (HBITMAP)hHandle;
		if (pPicture->m_pPict)
          pPicture->m_pPict->get_hPal((OLE_HANDLE FAR *)&hHandPal);
		else hHandPal = NULL;
        lppic->picData.bmp.hpal = (HPALETTE)hHandPal;
        break;
      case PICTYPE_ICON:
        lppic->picData.icon.hicon = (HICON)hHandle;
        break;
      case PICTYPE_METAFILE:
        lppic->picData.wmf.hmeta = (HANDLE)hHandle;
		if (pPicture->m_pPict)
           {
           pPicture->m_pPict->get_Height((long FAR *)&(lppic->picData.wmf.yExt));
           pPicture->m_pPict->get_Width((long FAR *)&(lppic->picData.wmf.xExt));
           }
        break;  
    }
  }    
  return hpic;
#else
  return (LONG)VBGetPic((HPIC)hpic, lppic);
#endif
}           

HANDLE FPLIB fpVBGetPicEx(HANDLE hpic, LPPIC lppic, USHORT usVersion)              
{
#ifdef FP_OCX
  return fpVBGetPic(hpic, lppic);
#else      
  return (LONG)VBGetPicEx((HPIC)hpic, lppic, usVersion);
#endif
}    

HANDLE FPLIB fpVBRefPic(HANDLE hpic)                                  
{
#ifdef FP_OCX
  return hpic;
#else
  return (LONG)VBRefPic((HPIC)hpic);
#endif
} 

void FPLIB fpVBInvalidateRect(LPFPCONTROL lpObject, LPRECT lprect, BOOL fEraseBkGnd)     
{
#ifdef FP_OCX
  fpInvalidateRect(lpObject, lprect, fEraseBkGnd);  
#else
  VBInvalidateRect((HCTL)lpObject->hCtl, lprect, fEraseBkGnd);
#endif
} 

LONG FPLIB fpVBGetDataSourceControl(LPFPCONTROL lpObject, BOOL FAR *lpbFlag)           
{
#ifdef FP_OCX
  return NULL;
#else
  return (LONG)VBGetDataSourceControl((HCTL)lpObject->hCtl, lpbFlag);
#endif
}

BOOL FPLIB fpVBRegisterModel(HANDLE hmod, LPMODEL lpmdl)
{
#ifdef FP_OCX
  return 0;
#else 
  return VBRegisterModel(hmod, lpmdl);
#endif
}                     

LPSTR FPLIB fpVBDerefHlstr(HLSTR hlstr)
{
#ifdef FP_OCX
  return (LPSTR)hlstr;
#else 
  return VBDerefHlstr(hlstr);
#endif
}

void FPLIB fpVBDestroyHlstr(HLSTR hlstr)                           
{
#ifdef FP_OCX
  SysFreeString((BSTR)hlstr);
  return;
#else
  VBDestroyHlstr(hlstr);
#endif
} 

void FPLIB fpVBDirtyForm(LPFPCONTROL lpObject)                               
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;
  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(COleControl))))
    ((COleControl *)pObject)->SetModifiedFlag(TRUE);
  return;
#else			 // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  if (pObject)
    pObject->fpAtlSetModifiedFlag(TRUE);
  else if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
  {
//  pObject = (COleControl *)COleControl::ControlFromHwnd((HWND)lpObject->hWnd);
    _ASSERT(0);  // this feature is not supported in ATL.
  }

  return;
#endif
#else    
  VBDirtyForm((HCTL)lpObject->hCtl);
#endif
} 

ERR FPLIB fpVBFireEvent(LPFPCONTROL lpObject, USHORT usidEvent, LPVOID lpparams)          
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;
  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(COleControl))))
    ((COleControl *)pObject)->FireEvent(usidEvent, (unsigned char far *)lpparams);  // this may cause problems
  return 0;
#else			 // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;
  ERR lRet = 0;

  if (pObject)
     lRet = (ERR)pObject->fpAtlFireEvent(usidEvent, lpparams);
  else if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
  {
//  pObject = COleControl::ControlFromHwnd((HWND)lpObject->hWnd);
    _ASSERT(0);  // No support for this case (ATL).  SCP 7/30/97
  }

  return lRet;
#endif
#else
  return VBFireEvent((HCTL)lpObject->hCtl, usidEvent, lpparams);
#endif
}

LPSTR FPLIB fpVBDerefHlstrLen(HLSTR hlstr, USHORT FAR *pcbLen)
{
#ifdef FP_OCX 
  if (hlstr)
     *pcbLen = SysStringLen((BSTR)hlstr);
  return (LPSTR)hlstr;
#else
  return VBDerefHlstrLen(hlstr, pcbLen);
#endif
}

HANDLE  FPLIB fpVBGetHInstance(LPFPCONTROL lpObject)
{
  return (HANDLE)fpGetInstanceHandle(lpObject);
}

LONG    FPLIB fpVBSuperControlProc(LPFPCONTROL lpObject, USHORT msg, USHORT wp, LONG lp)
{
#ifdef FP_OCX                                
  // cannot retrieve SuperWndProcAddr since it is a protected class member
  // for now, just send message through the queue/dispatch system
  return fpSendMessage(lpObject, msg, wp, lp);
#else
  return VBSuperControlProc((HCTL)lpObject->hCtl, msg, wp, lp);
#endif
}

ERR     FPLIB fpVBRecreateControlHwnd(LPFPCONTROL lpObject)
{
#ifdef FP_OCX 
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;
  
  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(COleControl))))
    ((COleControl *)pObject)->RecreateControlWindow();
  return 0;
#else			 // NO_MFC
/*
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  
  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
  {
//   pObject = (COleControl *)COleControl::ControlFromHwnd((HWND)lpObject->hWnd);
     _ASSERT(0);  // No support for this case (ATL).  SCP 7/31/97
  }

  if (pObject)
     pObject->RecreateControlWindow();
*/
  _ASSERT(0);  // Currently, no support (ATL).  SCP 7/31/97

  return 0;

#endif
#else
  return VBRecreateControlHwnd((HCTL)lpObject->hCtl);
#endif
}

ERR     FPLIB fpVBSetErrorMessage(ERR err, LPSTR lpstr)
{
#ifdef FP_OCX  
  return 0;
#else
  return VBSetErrorMessage(err, lpstr);
#endif
}

VOID    FPLIB fpVBGetAppTitle(LPSTR lpstr, USHORT cbMax)
{
#ifdef FP_OCX
#ifndef NO_MFC
  lpstr = (LPSTR)AfxGetAppName();
#else			 // NO_MFC
  lpstr = NULL;
#endif
#else         
  VBGetAppTitle(lpstr, cbMax);
#endif
}

// NOTE:  If you are trying to use this function in an OCX, you cannot. 
//        There is no equivalent function, you must find another way to 
//        implement this feature (see me).  -SCP 5/29/97
#ifdef FP_VB  
ULONG FPLIB fpVBSetControlFlags(LPFPCONTROL lpObject, ULONG mask, ULONG value)
{
  return VBSetControlFlags((HCTL)lpObject->hCtl, mask, value);
}
#endif

VOID    FPLIB fpVBSetCapture(LPFPCONTROL lpObject)
{
#ifdef FP_OCX 
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
    ((CWnd *)pObject)->SetCapture();
#else			 // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndTemp = NULL;

  if (lpObject->hWnd)
	  hWndTemp = lpObject->hWnd;
  
  if (pObject)
	 hWndTemp = pObject->fpAtlGetHWnd();

  if (hWndTemp && IsWindow(hWndTemp))
	 SetCapture(hWndTemp);

#endif
#else
  VBSetCapture((HCTL)lpObject->hCtl);
#endif
}

VOID    FPLIB fpVBReleaseCapture(VOID)
{
#ifdef FP_OCX 
  ::ReleaseCapture();
#else
  VBReleaseCapture();
#endif
}

VOID    FPLIB fpVBGetClientRect(LPFPCONTROL lpObject, LPRECT lprect)
{
#ifdef FP_OCX 
  fpGetClientRect(lpObject, lprect);
#else
  VBGetClientRect((HCTL)lpObject->hCtl, lprect);
#endif
}

VOID    FPLIB fpVBClientToScreen(LPFPCONTROL lpObject, LPPOINT lppt)
{
#ifdef FP_OCX 
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
    ((CWnd *)pObject)->ClientToScreen(lppt);
#else			 // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndTemp = NULL;

  if (lpObject->hWnd)
	  hWndTemp = lpObject->hWnd;
  
  if (pObject)
	 hWndTemp = pObject->fpAtlGetHWnd();

  if (hWndTemp && IsWindow(hWndTemp))
    ClientToScreen(hWndTemp, lppt);
#endif
#else
  VBClientToScreen((HCTL)lpObject->hCtl, lppt);
#endif
}

VOID    FPLIB fpVBScreenToClient(LPFPCONTROL lpObject, LPPOINT lppt)
{
#ifdef FP_OCX 
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
    ((CWnd *)pObject)->ScreenToClient(lppt);
#else			 // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndTemp = NULL;

  if (lpObject->hWnd)
	  hWndTemp = lpObject->hWnd;
  
  if (pObject)
	 hWndTemp = pObject->fpAtlGetHWnd();

  if (hWndTemp && IsWindow(hWndTemp))
    ScreenToClient(hWndTemp, lppt);
#endif
#else
  VBScreenToClient((HCTL)lpObject->hCtl, lppt);
#endif
}

BOOL    FPLIB fpVBIsControlVisible(LPFPCONTROL lpObject)
{
#ifdef FP_OCX 
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);
  
  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
    return ((CWnd *)pObject)->IsWindowVisible();
  else
  {
    LONG lVisible = FALSE;
    fpVBGetControlProperty(lpObject, 0, &lVisible, (LPTSTR)_T("Visible"), VT_I4);
    return (BOOL)lVisible;
  }
#else			 // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndTemp = NULL;

  if (lpObject->hWnd)
	  hWndTemp = lpObject->hWnd;
  
  if (pObject)
	 hWndTemp = pObject->fpAtlGetHWnd();

  if (hWndTemp && IsWindow(hWndTemp))
    return IsWindowVisible(hWndTemp);
  else
  {
    LONG lVisible = FALSE;
    fpVBGetControlProperty(lpObject, 0, &lVisible, _T("Visible"), VT_I4);
    return (BOOL)lVisible;
  }
#endif
#else
  return VBIsControlVisible((HCTL)lpObject->hCtl);
#endif
}

BOOL    FPLIB fpVBIsControlEnabled(LPFPCONTROL lpObject)
{
#ifdef FP_OCX 
  return fpIsEnabled(lpObject);
#else
  return VBIsControlEnabled((HCTL)lpObject->hCtl);
#endif
}

VOID    FPLIB fpVBZOrder(LPFPCONTROL lpObject, WORD zorder)
{
#ifdef FP_OCX 
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
    ::SetWindowPos(((CWnd *)pObject)->GetSafeHwnd(),
                   zorder == ZORDER_BACK ? HWND_BOTTOM : HWND_TOP,
                   0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
#else			 // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndTemp = NULL;

  if (lpObject->hWnd)
	  hWndTemp = lpObject->hWnd;
  
  if (pObject)
	 hWndTemp = pObject->fpAtlGetHWnd();

  if (hWndTemp && IsWindow(hWndTemp))
     SetWindowPos(hWndTemp,
                  zorder == ZORDER_BACK ? HWND_BOTTOM : HWND_TOP,
                  0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
#endif
#else
  VBZOrder((HCTL)lpObject->hCtl, zorder);
#endif
}

LPSTR   FPLIB fpVBLockHsz(HSZ hsz)
{
#ifdef FP_OCX     
  return (LPSTR)hsz;
#else
  return VBLockHsz(hsz);
#endif
}

VOID    FPLIB fpVBUnlockHsz(HSZ hsz)
{
#ifdef FP_OCX
  return; 
#else
  VBUnlockHsz(hsz);
#endif
}

HLSTR   FPLIB fpVBCreateTempHlstr(LPVOID pb, USHORT cbLen)
{
#ifdef FP_OCX  
  // OCX will need to free this string, it is not temporary
  return (HLSTR)SysAllocStringLen((OLECHAR FAR *)pb, cbLen);  
#else
  return VBCreateTempHlstr(pb, cbLen);
#endif
}

USHORT  FPLIB fpVBGetHlstr(HLSTR hlstr, LPVOID pb, USHORT cbLen)
{
#ifdef FP_OCX  
  UINT uiSize;
  uiSize = SysStringLen((BSTR)hlstr);
  uiSize = min(uiSize, cbLen);
  memcpy(pb, hlstr, uiSize); 
  return uiSize;  
#else
  return VBGetHlstr(hlstr, pb, cbLen);
#endif
}

ERR     FPLIB fpVBSetHlstr(LPHLSTR lphlstr, LPVOID pb, USHORT cbLen)
{
#ifdef FP_OCX 
  return SysReAllocStringLen((BSTR FAR *)lphlstr, (OLECHAR FAR *)pb, cbLen);
#else
  return VBSetHlstr(lphlstr, pb, cbLen);
#endif
}

ERR     FPLIB fpVBResizeHlstr(HLSTR hlstr, USHORT newCbLen)
{
#ifdef FP_OCX 
  return SysReAllocStringLen((BSTR FAR *)&hlstr, (OLECHAR FAR *)hlstr, newCbLen);
#else
  return VBResizeHlstr(hlstr, newCbLen);
#endif
}

ERR     FPLIB fpVBCoerceVariant(LPVAR lpVar, SHORT vtype, LPVOID lpData)
{
#ifdef FP_OCX
  VARIANT tmpVal;
  VariantInit(&tmpVal);
  ERR rc = (ERR)GetScode(VariantChangeType(&tmpVal, lpVar, 0, vtype));
  if (!rc)
  {
    // if change was OK, move value into "lpData"
    switch(vtype)
    {
      case VT_I2:
         *(short FAR *)lpData = tmpVal.iVal;   break;
  
      case VT_I4:
         *(long FAR *)lpData = tmpVal.lVal;   break;
  
      case VT_R4:
         *(float FAR *)lpData = tmpVal.fltVal;   break;
  
      case VT_R8:
         *(double FAR *)lpData = tmpVal.dblVal;   break;

      case VT_DATE:
         *(double FAR *)lpData = tmpVal.dblVal;   break;
  
      case VT_STRING:
      default:
         *(BSTR FAR *)lpData = tmpVal.bstrVal;   break;
    }
  }
  return rc;
#else
  return VBCoerceVariant(lpVar, vtype, lpData);
#endif
}

/* +++++++++++++++++  below f(n)s still to convert +++++++++++++++++++++++  */
SHORT   FPLIB fpVBGetVariantType(LPVAR lpVar)
{
#ifdef FP_OCX 
  return 0;
#else
  return VBGetVariantType(lpVar);
#endif
}

SHORT   FPLIB fpVBGetVariantValue(LPVAR lpVar, LPVAL lpVal)
{
#ifdef FP_OCX              
  return 0;
#else
  return VBGetVariantValue(lpVar, lpVal);
#endif
}

// NOTE:  This is typedef-ed in fpvbsub.h, but only when NOT __cplusplus.
typedef union
  {
  SHORT    i2;
  LONG     i4;
  float    r4;
  double   r8;
  CURRENCY cy;
  HLSTR    hlstr;
  } fpVALUE;

ERR FPLIB fpVBSetVariantValue(LPVAR lpVar, SHORT vtype, LPVOID lpData)
{
#ifdef FP_OCX 
  ERR ret = 0;

  switch(vtype)
  {
    case VT_I2:
       lpVar->iVal = ((fpVALUE FAR *)lpData)->i2;
       lpVar->vt   = VT_I2;
       break;

    case VT_I4:
       lpVar->lVal = ((fpVALUE FAR *)lpData)->i4;
       lpVar->vt   = VT_I4;
       break;

    case VT_R4:
       lpVar->fltVal = ((fpVALUE FAR *)lpData)->r4;
       lpVar->vt     = VT_R4;
       break;

    case VT_R8:
       lpVar->dblVal = ((fpVALUE FAR *)lpData)->r8;
       lpVar->vt     = VT_R8;
       break;

    case VT_EMPTY:
       lpVar->vt      = VT_EMPTY;
       break;

    case VT_STRING:
    default:
       lpVar->bstrVal = (BSTR)(((fpVALUE FAR *)lpData)->hlstr);
       lpVar->vt      = VT_BSTR;
       break;
  }
  return ret;
#else      
  return VBSetVariantValue(lpVar, vtype, lpData);
#endif
}

LPVOID  FPLIB fpVBArrayElement(HAD hAD, SHORT cIndex, LPSHORT lpi)
{
#ifdef FP_OCX                             
  return NULL;
#else
  return VBArrayElement(hAD, cIndex, lpi);
#endif
}

LONG    FPLIB fpVBArrayBounds(HAD hAD, SHORT index)
{
#ifdef FP_OCX 
  return 0L;
#else
  return VBArrayBounds(hAD, index);
#endif
}

USHORT  FPLIB fpVBArrayElemSize(HAD hAD)
{
#ifdef FP_OCX            
  return 0;
#else
  return VBArrayElemSize(hAD);
#endif
}

LPVOID  FPLIB fpVBArrayFirstElem(HAD hAD)
{
#ifdef FP_OCX                  
  return NULL;
#else
  return VBArrayFirstElem(hAD);
#endif
}

SHORT   FPLIB fpVBArrayIndexCount(HAD hAD)
{
#ifdef FP_OCX 
  return 0;
#else
  return VBArrayIndexCount(hAD);
#endif
}

VOID    FPLIB fpVBRuntimeError(ERR err)
{
#ifdef FP_OCX    
  return;
#else
  VBRuntimeError(err);
#endif
}

USHORT  FPLIB fpVBCbSaveFPState(LPVOID pb, USHORT cb)
{
#ifdef FP_OCX 
  return 0;
#else
  return VBCbSaveFPState(pb, cb);
#endif
}

VOID    FPLIB fpVBRestoreFPState(LPVOID pb)
{
#ifdef FP_OCX      
  return;
#else
  VBRestoreFPState(pb);
#endif
}

LONG FPLIB fpVBAllocPic(LPPIC lppic)
{
#ifdef FP_OCX       
  return 0L;
#else
  return (LONG)VBAllocPic(lppic);
#endif
}

HANDLE FPLIB fpVBAllocPicEx(LPPIC lppic, USHORT usVersion)
{
#ifdef FP_OCX 
   CPictureHolder *pObject;

   pObject = new CPictureHolder();
   pObject->CreateEmpty();

   switch (lppic->picType)
      {
      case PICTYPE_BITMAP:
         pObject->CreateFromBitmap(lppic->picData.bmp.hbitmap,
                                   lppic->picData.bmp.hpal, FALSE);	// changed last param to FALSE -scl 7/22/99
         break;

      case PICTYPE_ICON:
        pObject->CreateFromIcon(lppic->picData.icon.hicon, FALSE); // changed last param to FALSE -scl 7/22/99
        break;

      case PICTYPE_METAFILE:
        pObject->CreateFromMetafile((HMETAFILE)lppic->picData.wmf.hmeta,
                                    lppic->picData.wmf.xExt,
                                    lppic->picData.wmf.yExt, FALSE); // changed last param to FALSE -scl 7/22/99
        break;  
      }				// see code in fpPictLoad() (in fppict.c)

   return ((HANDLE)pObject);
#else
   if (usVersion < VB200_VERSION)
      return (LONG)VBAllocPic(lppic);
   else
      return (LONG)VBAllocPicEx(lppic, usVersion);
#endif
}

ERR FPLIB fpVBPicFromCF(LPHPIC lphpic, HANDLE hData, WORD wFormat)
{
#ifdef FP_OCX                       
  return 0;
#else
  return VBPicFromCF(lphpic, hData, wFormat);
#endif
}

VOID    FPLIB fpVBPaletteChanged(LPFPCONTROL lpObject)
{
#ifdef FP_OCX            
  return;
#else
  VBPaletteChanged((HCTL)lpObject->hCtl);
#endif
}

COLOR   FPLIB fpVBTranslateColor(LPFPCONTROL lpObject, COLOR clr)
{
#ifdef FP_OCX                            
  return 0;
#else
  return VBTranslateColor((HCTL)lpObject->hCtl, clr);
#endif
}

ERR     FPLIB fpVBReadFormFile(HFORMFILE hformfile, LPVOID pb, WORD cb)
{
#ifdef FP_OCX                          
  return 0;
#else
  return VBReadFormFile(hformfile, pb, cb);
#endif
}

ERR     FPLIB fpVBWriteFormFile(HFORMFILE hformfile, LPVOID pb, WORD cb)
{
#ifdef FP_OCX                           
  return 0;
#else
  return VBWriteFormFile(hformfile, pb, cb);
#endif
}

LONG    FPLIB fpVBSeekFormFile(HFORMFILE hformfile, LONG offset)
{
#ifdef FP_OCX                       
  return 0;
#else
  return VBSeekFormFile(hformfile, offset);
#endif
}

LONG    FPLIB fpVBRelSeekFormFile(HFORMFILE hformfile, LONG offset)
{
#ifdef FP_OCX 
  return 0L;
#else
  return VBRelSeekFormFile(hformfile, offset);
#endif
}

ERR     FPLIB fpVBReadBasicFile(USHORT usFileNo, LPVOID pb, WORD cb)
{
#ifdef FP_OCX                          
  return 0;
#else
  return VBReadBasicFile(usFileNo, pb, cb);
#endif
}

ERR     FPLIB fpVBWriteBasicFile(USHORT usFileNo, LPVOID pb, WORD cb)
{
#ifdef FP_OCX                           
  return 0;
#else
  return VBWriteBasicFile(usFileNo, pb, cb);
#endif
}

LONG    FPLIB fpVBSeekBasicFile(USHORT usFileNo, LONG offset)
{
#ifdef FP_OCX                        
  return 0L;
#else
  return VBSeekBasicFile(usFileNo, offset);
#endif
}

LONG    FPLIB fpVBRelSeekBasicFile(USHORT usFileNo, LONG offset)
{
#ifdef FP_OCX                         
  return 0L;
#else
  return VBRelSeekBasicFile(usFileNo, offset);
#endif
}

ERR     FPLIB fpVBLinkPostAdvise(LPFPCONTROL lpObject)
{
#ifdef FP_OCX                   
  return 0;
#else
  return VBLinkPostAdvise((HCTL)lpObject->hCtl);
#endif
}

BOOL    FPLIB fpVBPasteLinkOk(HANDLE FAR *phTriplet, LPFPCONTROL lpObject)
{
#ifdef FP_OCX                           
  return FALSE;
#else
  return VBPasteLinkOk(phTriplet, (HCTL)lpObject->hCtl);
#endif
}

VOID    FPLIB fpVBLinkMakeItemName(LPFPCONTROL lpObject, LPSTR lpszBuf)
{
#ifdef FP_OCX                       
  return;
#else
  VBLinkMakeItemName((HCTL)lpObject->hCtl, lpszBuf);
#endif
}
/* +++++++++++++++++   above f(n)s still to convert +++++++++++++++++++++++  */
#ifdef __cplusplus
}
#endif
