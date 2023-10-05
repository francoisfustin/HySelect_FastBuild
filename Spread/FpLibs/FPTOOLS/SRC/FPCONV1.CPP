/***************************************************************************/
/* FPCONV1.CPP   FarPoint Support for VBX functions in non-VB environments */
/***************************************************************************/

#ifdef STRICT
#undef STRICT
#endif

#include <string.h>

#ifdef FP_OCX
#ifndef NO_MFC
#include <afxctl.h>                 // MFC support for OLE Custom Controls
#endif

#ifdef WIN32
#include <afxpriv.h>
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

// IGetOleObject
//DEFINE_GUID(IID_IGetOleObject, 0x8A701DA0L, 0x4FEB, 0x101B, 0xA8, 0x2E, 0x08, 0x00, 0x2B, 0x2B, 0x23, 0x37);

//DDE
DEFINE_GUID(IID_IVBDDE, 0xCC867200L, 0x0DC1, 0x101B, 0xA3, 0xF6, 0x00, 0xAA, 0x00, 0x34, 0xE4, 0xE9);
DEFINE_GUID(IID_IVBDDESite, 0xD856BCC0L, 0x0DC1, 0x101B, 0xA3, 0xF6, 0x00, 0xAA, 0x00, 0x34, 0xE4, 0xE9);

//Format & Files
DEFINE_GUID(IID_IVBFiles, 0xB53BA860L, 0x3768, 0x101B, 0x8D, 0x72, 0x00, 0xDD, 0x01, 0x0E, 0xDF, 0xAA);

// Internally Private GUID to convert IUnknown * to COleControl *
DEFINE_GUID(IID_IControlPrv, 0xD97180L, 0xFCF7, 0x11CE, 0xA0, 0x9E, 0x00, 0xAA, 0x00, 0x62, 0xBE, 0x57);

#endif

#include "fptools.h"                             

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FP_OCX

// cast the return value to a COleControl * object and use it as necessary
LONG FPLIB fpControlFromUnknown(LONG lpUnknown)
{
	LONG lReturn = (LONG)NULL;
#ifdef FP_OCX

    if (!lpUnknown) 
      return lReturn;
    
    ((LPUNKNOWN)lpUnknown)->QueryInterface(IID_IControlPrv, (LPVOID FAR *)&lReturn);

#endif
    return lReturn;
}
                                                             
void FPLIB fpEnumPropsHelper(LPFPCONTROL lpObject, LPSTR *bstrPropNames, int cPropNames)
{
  unsigned int iTypeInfo, ctNames;
  unsigned int i = 0;
  unsigned int iVar = 0;
  LPTYPEINFO lpTypeInfo;
  LPTYPEATTR lpTypeAttr;
  LPVARDESC  lpVarDesc;
  BSTR       bstrNames[256];
  int        iPropNames = 0;                                              
  LPDISPATCH lpDisp = (LPDISPATCH)lpObject->lpDispatch;
  
  if (lpDisp == NULL)
    return;
 
  if (NOERROR == lpDisp->GetTypeInfoCount(&iTypeInfo))
  {
    if (iTypeInfo == 1 && NOERROR == lpDisp->GetTypeInfo(0, 0, &lpTypeInfo))
    {  
      if (NOERROR == lpTypeInfo->GetTypeAttr(&lpTypeAttr))
      {
        for (i = 0; i < lpTypeAttr->cImplTypes; i++)
        {
          for (iVar = 0; iVar < lpTypeAttr->cVars && iPropNames < cPropNames; iVar++)
          {
            if (NOERROR == lpTypeInfo->GetVarDesc(iVar, &lpVarDesc))
            {
              lpTypeInfo->GetNames(lpVarDesc->memid, bstrNames, 256, &ctNames);
              if (iPropNames < cPropNames)                                                                                   
              {
                bstrPropNames[iPropNames] = (char *)bstrNames[0];
                iPropNames++;
              }
              lpTypeInfo->ReleaseVarDesc(lpVarDesc);
            }
          }
        }
        lpTypeInfo->ReleaseTypeAttr(lpTypeAttr);
      }
      lpTypeInfo->Release();        
    }
  }
}

void FPLIB fpEnumFuncsHelper(LPFPCONTROL lpObject, LPSTR *bstrFuncNames, int cFuncNames)
{
  unsigned int iTypeInfo, ctNames;
  unsigned int i = 0;
  unsigned int iFunc = 0;
  LPTYPEINFO lpTypeInfo;
  LPTYPEATTR lpTypeAttr;
  LPFUNCDESC lpFuncDesc;
  BSTR       bstrNames[256];
  int        iFuncNames = 0;
  LPDISPATCH lpDisp = (LPDISPATCH)lpObject->lpDispatch;

  if (lpDisp == NULL)
    return;
 
  if (NOERROR == lpDisp->GetTypeInfoCount(&iTypeInfo))
  {
    if (iTypeInfo == 1 && NOERROR == lpDisp->GetTypeInfo(0, 0, &lpTypeInfo))
    {      
      if (NOERROR == lpTypeInfo->GetTypeAttr(&lpTypeAttr))
      {
        for (i = 0; i < lpTypeAttr->cImplTypes; i++)
        {
          for (iFunc = 0; iFunc < lpTypeAttr->cFuncs &&
               iFuncNames < cFuncNames; iFunc++)
          {
            if (NOERROR == lpTypeInfo->GetFuncDesc(iFunc, &lpFuncDesc))
            {
              lpTypeInfo->GetNames(lpFuncDesc->memid, bstrNames, 
                                   256, &ctNames);
              if (iFuncNames < cFuncNames)
              {
                bstrFuncNames[iFuncNames] = (char *)bstrNames[0];
                iFuncNames++;
              }
              lpTypeInfo->ReleaseFuncDesc(lpFuncDesc);
            }
          }
          lpTypeInfo->ReleaseTypeAttr(lpTypeAttr);
        }  
      } 
      lpTypeInfo->Release();        
    }     
  }                                         
}

#endif

#ifdef __cplusplus
}
#endif
