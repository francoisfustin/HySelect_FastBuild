/**************************************************************************/
/* VBSSCNVT.C   FarPoint Support for VBX functions in non-VB environments */
/**************************************************************************/

#define NO_VBX_SUPPORT

#if ((defined(FP_OCX) || defined(FP_DLL)) && defined(WIN32))
#define WIN32_LEAN_AND_MEAN   //exclude conflicts with VBAPI.H in WIN32
#endif

#include <afxctl.h>                 // MFC support for OLE Custom Controls
#include <windows.h>

#include "..\..\..\fplibs\fptools\src\fpconvrt.h"
#include "..\..\..\fplibs\fptools\src\fptstr.h"

#ifdef FP_OCX
#include "..\..\..\fplibs\fptools\src\vbinterf.h"
// following lines taken from VBINTERF.H

//IVBGetControl & IGetOleObject
DEFINE_GUID(IID_IVBGetControl, 0x40A050A0L, 0x3C31, 0x101B, 0xA8, 0x2E, 0x08, 0x00, 0x2B, 0x2B, 0x23, 0x37);
DEFINE_GUID(IID_IGetOleObject, 0x8A701DA0L, 0x4FEB, 0x101B, 0xA8, 0x2E, 0x08, 0x00, 0x2B, 0x2B, 0x23, 0x37);

//DDE
DEFINE_GUID(IID_IVBDDE, 0xCC867200L, 0x0DC1, 0x101B, 0xA3, 0xF6, 0x00, 0xAA, 0x00, 0x34, 0xE4, 0xE9);
DEFINE_GUID(IID_IVBDDESite, 0xD856BCC0L, 0x0DC1, 0x101B, 0xA3, 0xF6, 0x00, 0xAA, 0x00, 0x34, 0xE4, 0xE9);

//Format & Files
DEFINE_GUID(IID_IGetVBAObject, 0x91733A60L, 0x3F4C, 0x101B, 0xA3, 0xF6, 0x00, 0xAA, 0x00, 0x34, 0xE4, 0xE9);
DEFINE_GUID(IID_IVBFormat, 0x9849FD60L, 0x3768, 0x101B, 0x8D, 0x72, 0xAE, 0x61, 0x64, 0xFF, 0xE3, 0xCF);
DEFINE_GUID(IID_IVBFiles, 0xB53BA860L, 0x3768, 0x101B, 0x8D, 0x72, 0x00, 0xDD, 0x01, 0x0E, 0xDF, 0xAA);

#endif

#include "..\..\..\fplibs\fpdb\src\fpdbutil.h"
#include "vbsscnvt.h"

extern "C"
{
// LPSTR FP_API fpStr2OLE(LPSTR lpszInput); // SCP - 10/8/98

// ************************************************************************
// * These functions either call the substitute in FPCONVRT or the
// * VB API function directly.
// ************************************************************************

SHORT ssVBFormat(LONG lObject, SHORT vtype, LPVOID lpData, LPTSTR lpszFmt, LPVOID pb, USHORT cb)        
{
#ifdef FP_OCX

  FPCONTROL fpCtl = {0};
  fpCtl.lObject = lObject;
  fpCtl.hWnd = 0;
//  fpCtl.hWnd = ((COleControl *)lObject)->m_hWnd;

  return fpVBFormat(&fpCtl, vtype, lpData, lpszFmt, pb, cb);
/*
  COleControl *pObject = (COleControl *)lObject;
  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(COleControl))))
  {

    LPOLECLIENTSITE pClientSite;
    IGetVBAObject  *lpVBAObject;
    IVBFormat      *lpFormat;
    VARIANTARG      varData;
    USHORT          retcb = (USHORT)0;
    
    VariantInit(&varData);
    varData.vt = vtype;

    switch (vtype)
    {
      case VT_I2:
        varData.iVal = *(short *)lpData;
        break;
      case VT_I4:
        varData.lVal = *(long *)lpData;
        break;
      case VT_R4:
        varData.fltVal = *(float *)lpData;
        break;
      case VT_R8:
        varData.dblVal = *(double *)lpData;
        break;
      case VT_STRING:
        // for strings, lData must be the address of BSTR or LPSTR with
        //    memory previously allocated
        varData.vt = VT_BSTR;
        varData.bstrVal = *(BSTR *)lpData;
        break;
      case VT_CURRENCY:
        varData.vt = VT_CY;
        varData.cyVal = *(CY *)lpData;
        break;
      case VT_DATE:
        varData.date = *(DATE *)lpData;
        break;
    }            

    
    pClientSite = pObject->GetClientSite();

    if (pClientSite == NULL)
      return 0;

    if (NOERROR == pClientSite->QueryInterface(IID_IGetVBAObject,
                                               (LPVOID FAR *)&lpVBAObject))
    {
      if (NOERROR == lpVBAObject->GetObject(IID_IVBFormat,
                                            (LPVOID FAR *)&lpFormat, 0L))
      {        
#ifdef WIN32
#define VBCHAR  WCHAR
#define LPVBSTR LPWSTR
        LPOLESTR bstrFmt = (LPOLESTR)fpStr2OLE(lpszFmt);
        BSTR bstrBuff = SysAllocStringByteLen(NULL, (cb + 1) * sizeof(VBCHAR));
        if (NOERROR == lpFormat->Format((VARIANTARG FAR *)&varData,
                                         bstrFmt, (LPVOID)bstrBuff, cb + 1, 0,
                                         1, 1, &retcb))
        {
          // copy buff to pb
          register int i = 0;
          while ((i < retcb) && ((LPVBSTR)bstrBuff)[i])
          {
            ((LPTSTR)pb)[i] = (TCHAR)(((LPVBSTR)bstrBuff)[i]);
            ++i;
          }
          if (i < cb)
            ((LPTSTR)pb)[i] = (TCHAR)0;
        }
        else
          retcb = 0;
        SysFreeString(bstrBuff);
        SysFreeString((BSTR)bstrFmt);
#else //not WIN32
        if (NOERROR != lpFormat->Format((VARIANTARG FAR *)&varData,
                                         lpszFmt, pb, cb, 0, 1, 1, &retcb))
           retcb = 0;
#endif
        lpFormat->Release();        
        lpVBAObject->Release();                            
      }                                       
    } 
    return retcb;
  }
  else
    return 0;
*/
#else

  return VBFormat((HCTL)lObject, vtype, lpData, lpszFmt, pb, cb);
#endif

}


// ************************************************************************
// * The following are only implemented for OCX, otherwise the 
// * "forward declaration" in the .h file maps it to the VB function
// * (with a macro).
// ************************************************************************
#ifdef FP_OCX
// C function wrappers for C++ BSTR functions

HSZ ssVBCreateHsz(LONG lObject, LPTSTR lpszString)
{
  return (HSZ)fpSysAllocTString((FPTSTR)lpszString);
}

HSZ ssVBDestroyHsz(HSZ hszParm)
{
  fpSysFreeTString((FPTSTR)hszParm);
  return NULL;
}

HLSTR ssVBCreateHlstr(LPVOID pb, UINT cbLen)
{
  return (HLSTR)LBlockAllocLen((LPBYTE)pb, (LONG)cbLen);
}

HLSTR ssVBDestroyHlstr(HLSTR hlstr)
{
  LBlockFree((LBLOCK)hlstr);
  return NULL;
}

LPTSTR ssVBDerefHlstr(HLSTR hlstr)
{
  return (LPTSTR)hlstr;
}

LPTSTR ssVBDerefZeroTermHlstr(HLSTR hlstr)
{
  return (LPTSTR)hlstr;
}

UINT ssVBGetHlstrLen(HLSTR hlstr)
{
  return (UINT)LBlockLen((LBLOCK)hlstr);
}

LPTSTR ssVBDerefHsz(HSZ hsz)
{
  return (LPTSTR)hsz;
}

LPTSTR ssVBDerefHlstrLen(HLSTR hlstr, USHORT FAR *pcbLen)
{
  if (hlstr)
     *pcbLen = (USHORT)LBlockLen((LBLOCK)hlstr);
  return (LPTSTR)hlstr;
}

LONG ssVBGetDataSourceControl(LONG lObject, BOOL FAR *lpbFlag)           
{
  return NULL;
}

#endif   //FP_OCX

}
