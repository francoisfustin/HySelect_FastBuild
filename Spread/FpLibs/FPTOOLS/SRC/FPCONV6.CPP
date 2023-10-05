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

#endif

#include "fptools.h"                             

#ifdef __cplusplus
extern "C" {
#endif


void FPLIB fpDispatchAddRef(LONG lObject)
{
#ifdef FP_OCX
  // OCX - only function
  if (lObject)
    ((LPDISPATCH)lObject)->AddRef();
#endif
// there is no VB3 or prior equivalent
}

void FPLIB fpDispatchRelease(LONG lObject)
{
#ifdef FP_OCX
  // OCX - only function
  if (lObject)
    ((LPDISPATCH)lObject)->Release();
#endif
// there is no VB3 or prior equivalent
}

#ifdef __cplusplus
}
#endif
