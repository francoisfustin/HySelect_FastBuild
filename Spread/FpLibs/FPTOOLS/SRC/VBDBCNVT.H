/************************************************************************/
/* VBDBCNVT.H  FarPoint VBX - OCX conversion for Common Code Simplicity */
/************************************************************************/
#ifndef VBDBCNVT_H
#define VBDBCNVT_H

#ifdef FP_OCX
#include "fpconvrt.h"
#include "fptstr.h"
#include "fpdbutil.h"
#endif

        
#ifdef __cplusplus
extern "C" {
#endif

// ************************************************************************
// * These functions either call the substitute in FPCONVRT or the
// * VB API function directly.
// ************************************************************************

#ifndef FP_OCX  //not FP_OCX

// ************************************************************************
// * The following are only implemented for OCX, otherwise the 
// * "forward declaration" in the .h file maps it to the VB function
// * (with a macro).
// ************************************************************************

#define dbVBFormat(lObject, vtype, lpData, lpszFmt, pb, cb) \
  VBFormat(vtype, lpData, lpszFmt, pb, cb)

#define dbVBCreateHsz( lObject, lpszString) \
  VBCreateHsz( HIWORD(lObject), lpszString)
#define dbVBDestroyHsz( hszParm)    \
  VBDestroyHsz( hszParm)
#define dbVBCreateHlstr( pb, cbLen) \
  VBCreateHlstr( pb, cbLen)
#define dbVBDestroyHlstr( hlstr)    \
  VBDestroyHlstr( hlstr)
#define dbVBGetHlstrLen( hlstr)     \
  VBGetHlstrLen( hlstr)
#define dbVBDerefHlstrLen( hlstr, pcbLen)   \
  VBDerefHlstrLen( hlstr, pcbLen)
#define dbVBDerefHlstr( hlstr)      \
  VBDerefHlstr( hlstr)
#define dbVBDerefHsz( hsz) \
  VBDerefHsz( hsz)
#define dbVBLockHsz(hsz)   \
  VBLockHsz(hsz)
#define dbVBUnlockHsz(hsz) \
  VBUnlockHsz(hsz)

#define dbVBGetDataSourceControl( lObject, lpbFlag) \
  VBGetDataSourceControl( (HCTL)lObject, lpbFlag)

#define fpVBAllocPicEx(lppic, usVersion)    \
   VBAllocPicEx(lppic, usVersion)
#define fpVBRefPic(hpic)                    \
   VBRefPic(hpic)
#define fpVBGetVersion()                    \
   VBGetVersion()   
#define fpVBAllocPic(lppic)                 \
   VBAllocPic(lppic)
#define fpVBSetVariantValue(lpVar, vtype, lpData)  \
   VBSetVariantValue(lpVar, vtype, lpData)
   
#else  //FP_OCX is defined
//SHORT dbVBFormat(LONG lObject, SHORT vtype, LPVOID lpData, LPSTR lpszFmt, 
//        LPVOID pb, USHORT cb);

#define dbVBCreateHsz(lObject, lpszString)   \
  (HSZ)fpSysAllocTString((FPTSTR)lpszString)
#define dbVBDestroyHsz(hszParm)              \
  fpSysFreeTString((FPTSTR)hszParm)
#define dbVBCreateHlstr(pb, cbLen)           \
  (HLSTR)LBlockAllocLen((LPBYTE)pb, (LONG)cbLen)  
#define dbVBDestroyHlstr(hlstr)              \
  LBlockFree((LBLOCK)hlstr)
#define dbVBGetHlstrLen(hlstr)               \
  (UINT)LBlockLen((LBLOCK)hlstr)
#define dbVBDerefHlstrLen(hlstr, pcbLen)     \
  (LPSTR)(hlstr ? *pcbLen = (UINT)LBlockLen((LBLOCK)hlstr), hlstr : NULL)
#define dbVBDerefHlstr(hlstr) \
  (LPSTR)hlstr
#define dbVBDerefHsz(hsz) \
  (LPSTR)hsz
#define dbVBLockHsz(hsz)  \
  (LPSTR)hsz
#define dbVBUnlockHsz(hsz)

#define dbVBGetDataSourceControl(lObject, lpbFlag) \
  (LONG)NULL

#endif //not FP_OCX

#ifdef __cplusplus
}
#endif


#endif // VBDBCNVT_H
 
