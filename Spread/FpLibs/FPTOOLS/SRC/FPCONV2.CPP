/***************************************************************************/
/* FPCONV2.CPP   FarPoint Support for VBX functions in non-VB environments */
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
#include "stdafx.h"
#include "fpatl.h"
#endif

#ifdef WIN32

#ifndef NO_MFC
#include <afxpriv.h>
#endif

#if (defined( __T) && !defined(_INC_TCHAR))
#undef __T
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

DEFINE_GUID(IID_IVBGetControl, 0x40A050A0L, 0x3C31, 0x101B, 0xA8, 0x2E, 0x08, 0x00, 0x2B, 0x2B, 0x23, 0x37);

#endif

#include "fptools.h"                             

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FP_OCX

LONG FPLIB fpEnumObject(LPFPCONTROL lpObject, DWORD dwWhich)
{        
  LONG       pObjReturn = NULL;
  IVBGetControl *pGetControl; 
  LPENUMUNKNOWN   lpEnumUnknown; 
  LPDISPATCH lpDisp = (LPDISPATCH)lpObject->lpDispatch;
  LPDISPATCH lpDispReturn = NULL; 
  LPDISPATCH lpDispTemp = lpDispReturn;
//  short      nCnt = 0;
  BOOL bLoop = TRUE;
//  LPOLECONTROLSITE pOleControlSite;

  if (lpDisp)
  {
  
    if (NOERROR == lpDisp->QueryInterface(IID_IVBGetControl, (LPVOID FAR *)&pGetControl))
    {
      if (NOERROR == pGetControl->EnumControls(OLECONTF_ONLYUSER      |
                                               OLECONTF_ONLYIFRUNNING |
                                               OLECONTF_EMBEDDINGS    |
                                               OLECONTF_OTHERS,
                                               dwWhich,
                                               &lpEnumUnknown))
      {                                       
        LPUNKNOWN lpUnk;

        lpEnumUnknown->Reset();
        
        if ((dwWhich & 0xFF) == GC_WCH_CONTAINER)
            lpEnumUnknown->Skip(1);                

        while ((bLoop) &&
               (NOERROR == lpEnumUnknown->Next(1, &lpUnk, NULL)))                
        {                    

		  if (NOERROR == lpUnk->QueryInterface(IID_IDispatch, (LPVOID FAR *)&lpDispReturn))
		  {
            if (dwWhich & GC_WCH_FONLYAFTER)
            {                                     
              if (lpDispTemp == lpDisp)
              {  
                pObjReturn = (LONG)lpDispReturn;
                bLoop = FALSE;
              }
			  else
			    fpDispatchRelease((LONG)lpDispReturn);
            }
            else if (dwWhich & GC_WCH_FONLYBEFORE)
            {       
              if (lpDispReturn == lpDisp)
              {
                pObjReturn = (LONG)lpDispTemp;
                bLoop = FALSE;
              }              
			  else
			    fpDispatchRelease((LONG)lpDispReturn);
            }
            else
            { 
              // for all other cases return only first enumerated item
              if (lpDispReturn == lpDisp)
                 fpDispatchRelease((LONG)lpDispReturn);
              else
              {
                 pObjReturn = (LONG)lpDispReturn;
                 bLoop = FALSE;
              }
            }

            lpDispTemp = lpDispReturn;
              
          }
          lpUnk->Release();     
        }      
        lpEnumUnknown->Release();  
      } 
      pGetControl->Release();
    }
  }
  return (LONG)pObjReturn;
}         
             
LONG FPLIB fpBeginEnum(LPFPCONTROL lpObject, LPLONG lpAlt, LPLONG lpEnum, LONG lPrevDisp)
{        
  LONG       pObjReturn = NULL;
  IOleContainer *pAltGetControl; 
  IVBGetControl *pGetControl; 
  LPENUMUNKNOWN   lpEnumUnknown; 
  LPDISPATCH lpDisp = (LPDISPATCH)lpObject->lpDispatch;
  LPDISPATCH lpDispReturn = NULL;
  long       lEnumRet = 1;
  BOOL       bAltMethod = FALSE;

  if (lpDisp)
  {

    if (NOERROR == lpDisp->QueryInterface(IID_IVBGetControl, (LPVOID FAR *)&pGetControl))
    {
      *lpAlt = (LONG)pGetControl;

      lEnumRet = (long)pGetControl->EnumControls(OLECONTF_ONLYUSER      |
                                                 OLECONTF_ONLYIFRUNNING |
                                                 OLECONTF_EMBEDDINGS    |
                                                 OLECONTF_OTHERS,
                                                 GC_WCH_CONTAINED,
                                                 &lpEnumUnknown);
      bAltMethod = FALSE;
    }
	else if (NOERROR == lpDisp->QueryInterface(IID_IOleContainer, (LPVOID FAR *)&pAltGetControl))
    {
      *lpAlt = (LONG)pAltGetControl;
      lEnumRet = (long)pAltGetControl->EnumObjects(OLECONTF_ONLYUSER      |
                                                   OLECONTF_ONLYIFRUNNING |
                                                   OLECONTF_EMBEDDINGS    |
                                                   OLECONTF_OTHERS,
                                                   &lpEnumUnknown);
      bAltMethod = TRUE;
    }

    if (lEnumRet == NOERROR)
    {                                       
        *lpEnum = (LONG)lpEnumUnknown;
        lpEnumUnknown->Reset();
		// if alternate method skip first item - it is yourself
		if (bAltMethod)
            lpEnumUnknown->Skip(1);                
	
		pObjReturn = fpNextEnum((LONG)lpEnumUnknown, lPrevDisp);  

    }
  }
  return (LONG)pObjReturn;
}         

LONG FPLIB fpNextEnum(LONG lpEnum, LONG lPrevDisp)
{        
  LONG             pObjReturn = NULL;
  LPENUMUNKNOWN    lpEnumUnknown = (LPENUMUNKNOWN)lpEnum;
  LPDISPATCH       lpDispReturn = NULL;
  LPOLEOBJECT      pOleObject;
  LPOLECLIENTSITE  pOleClientSite;
  LPOLECONTROLSITE pOleControlSite;

  if (lpEnum)
  {
    LPUNKNOWN lpUnk;

    if (NOERROR == lpEnumUnknown->Next(1, &lpUnk, NULL))
    {
	  if (NOERROR == lpUnk->QueryInterface(IID_IDispatch, 
                                           (LPVOID FAR *)&lpDispReturn))
	  {
        if ((lPrevDisp) && (lPrevDisp == (LONG)lpDispReturn))
        {
          lpEnumUnknown->Skip(1);                		  
		}
	  
        if (NOERROR == lpDispReturn->QueryInterface(IID_IOleObject, (LPVOID FAR *)&pOleObject))
	    {
	    	pOleObject->GetClientSite(&pOleClientSite);
        	if (NOERROR == pOleClientSite->QueryInterface(IID_IOleControlSite, (LPVOID FAR *)&pOleControlSite))
	    	{
		  		if (lpDispReturn)
		    		fpDispatchRelease((LONG)lpDispReturn);
	      		pOleControlSite->GetExtendedControl(&lpDispReturn);
				pOleControlSite->Release();
	        }
			pOleClientSite->Release();
        	pOleObject->Release();
		}
		pObjReturn = (LONG)lpDispReturn;             
	  }
 	  else 
	  {
	    if (NOERROR == lpUnk->QueryInterface(IID_IOleObject, (LPVOID FAR *)&pOleObject))
	    {
	    	pOleObject->GetClientSite(&pOleClientSite);
        	if (NOERROR == pOleClientSite->QueryInterface(IID_IOleControlSite, (LPVOID FAR *)&pOleControlSite))
	    	{
	      		pOleControlSite->GetExtendedControl(&lpDispReturn);
				pOleControlSite->Release();
        		pObjReturn = (LONG)lpDispReturn;             
	        }
			pOleClientSite->Release();
        	pOleObject->Release();
	    }
      }
      lpUnk->Release();
    }
  }
  return (LONG)pObjReturn;
}         

void FPLIB fpEndEnum(LONG AltEnum, LONG lEnum)
{
 fpDispatchRelease(lEnum);
 fpDispatchRelease(AltEnum);
}

LONG FPLIB fpGetControlDispatch(LPFPCONTROL lpObject, WORD wGC)                         
{
#ifdef FP_OCX
  LPDISPATCH lpDispObject = (LPDISPATCH)lpObject->lpDispatch;
  LONG lReturn = NULL;
  
  if (lpDispObject)
  {                                                      
    DWORD flag = 0;

    switch (wGC)
    { 
      case GC_CHILD:
        flag = GC_WCH_CONTAINED;        
        break;
      case GC_CONTAINER:
        flag = GC_WCH_CONTAINER;        
        break;
      case GC_NEXTSIBLING:
        flag = GC_WCH_SIBLING | GC_WCH_FONLYAFTER;        
        break;
      case GC_PREVSIBLING:
        flag = GC_WCH_SIBLING | GC_WCH_FONLYBEFORE;        
        break;  
      case GC_FORM:
        flag = GC_WCH_ALL;
        break;
      case GC_LASTSIBLING:
        flag = GC_WCH_SIBLING | GC_WCH_FREVERSEDIR;        
        break;
      case GC_FIRSTSIBLING:
        flag = GC_WCH_SIBLING;
        break;
    
    }  
    lReturn = (LONG)fpEnumObject(lpObject, flag);
  
  }      
  return lReturn;
#else
  return 0L;
#endif
}

#endif

LONG FPLIB fpVBGetControl(LPFPCONTROL lpObject, WORD wGC)                         
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;  
  CWnd *pWndTemp = NULL;
  LONG lReturn = NULL;
  
  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))) &&
      (((CWnd *)pObject)->GetSafeHwnd()))
  {                                                      
    UINT flag = 0;
    switch (wGC)
    { 
      case GC_CHILD:
        flag = GW_CHILD;
        break;
      case GC_CONTAINER:
        flag = GW_OWNER;
        break;
      case GC_NEXTSIBLING:
        flag = GW_HWNDNEXT;
        break;
      case GC_PREVSIBLING:
        flag = GW_HWNDPREV;
        break;  
      case GC_FORM:
        break;
      case GC_LASTSIBLING:
        flag = GW_HWNDLAST;
        break;
      case GC_FIRSTSIBLING:
        flag = GW_HWNDFIRST;
        break;
    
    }  
	pWndTemp = ((CWnd *)pObject)->GetWindow(flag);
	if (pWndTemp)
    {
      if (!(lReturn = (LONG)CWnd::FromHandlePermanent(pWndTemp->GetSafeHwnd())))
	      lReturn = (LONG)pWndTemp;
	}
	else
	  lReturn = 0L;
  }      
  return lReturn;

#else	 // NO_MFC

  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndTemp = NULL;
  LONG lReturn = NULL;
  HWND hWndBase = NULL;
  
  if (lpObject->hWnd)
	 hWndBase = lpObject->hWnd;
  
  if (pObject)
	 hWndBase = pObject->fpAtlGetHWnd();
  
  if ((hWndBase) && IsWindow(hWndBase))
  {                                                      
    UINT flag = 0;
    switch (wGC)
    { 
      case GC_CHILD:
        flag = GW_CHILD;
        break;
      case GC_CONTAINER:
        flag = GW_OWNER;
        break;
      case GC_NEXTSIBLING:
        flag = GW_HWNDNEXT;
        break;
      case GC_PREVSIBLING:
        flag = GW_HWNDPREV;
        break;  
      case GC_FORM:
        break;
      case GC_LASTSIBLING:
        flag = GW_HWNDLAST;
        break;
      case GC_FIRSTSIBLING:
        flag = GW_HWNDFIRST;
        break;
    }  
	hWndTemp = GetWindow(hWndBase, flag);
	if (hWndTemp)
    {
// non-MFC returns window handle, not window object
       lReturn = (LONG)hWndTemp;
	}
	else
	  lReturn = 0L;
  }      
  return lReturn;

#endif // NO_MFC
#else  // VBX
  return (LONG)VBGetControl((HCTL)lpObject->hCtl, wGC);
#endif
}


#ifdef __cplusplus
}
#endif
