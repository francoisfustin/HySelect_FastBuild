/************************************************************************/
/* VBSSCNVT.H  FarPoint VBX - OCX conversion for Common Code Simplicity */
/************************************************************************/
#ifndef __VBSSCNVT_H__
#define __VBSSCNVT_H__


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

#define ssVBFormat(lObject, vtype, lpData, lpszFmt, pb, cb) \
  VBFormat(vtype, lpData, lpszFmt, pb, cb)

#define ssVBCreateHsz( lObject, lpszString) \
  VBCreateHsz( HIWORD(lObject), lpszString)
#define ssVBDestroyHsz( hszParm)            \
  VBDestroyHsz( hszParm)
#define ssVBCreateHlstr( pb, cbLen)         \
  VBCreateHlstr( pb, cbLen)
#define ssVBDestroyHlstr( hlstr)            \
  VBDestroyHlstr( hlstr)
#define ssVBGetHlstrLen( hlstr)             \
  VBGetHlstrLen( hlstr)
#define ssVBDerefHlstrLen( hlstr, pcbLen)   \
  VBDerefHlstrLen( hlstr, pcbLen)
#define ssVBDerefHlstr( hlstr)              \
  VBDerefHlstr( hlstr)
#define ssVBDerefZeroTermHlstr( hlstr)              \
  VBDerefZeroTermHlstr( hlstr)
#define ssVBDerefHsz( hsz)                  \
  VBDerefHsz( hsz)
#define ssVBGetDataSourceControl( lObject, lpbFlag) \
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
SHORT ssVBFormat(LONG lObject, SHORT vtype, LPVOID lpData, LPTSTR lpszFmt, 
        LPVOID pb, USHORT cb);
HSZ   ssVBCreateHsz(LONG lObject, LPTSTR lpszString);
HSZ   ssVBDestroyHsz(HSZ hszParm);
HLSTR ssVBCreateHlstr(LPVOID pb, UINT cbLen);
HLSTR ssVBDestroyHlstr(HLSTR hlstr);
UINT  ssVBGetHlstrLen(HLSTR hlstr);
LPTSTR ssVBDerefHlstrLen(HLSTR hlstr, USHORT FAR *pcbLen);
LPTSTR ssVBDerefHlstr(HLSTR hlstr);
LPTSTR ssVBDerefZeroTermHlstr(HLSTR hlstr);
LPTSTR ssVBDerefHsz(HSZ hsz);
LONG  ssVBGetDataSourceControl(LONG lObject, BOOL FAR *lpbFlag);
#endif //not FP_OCX

#ifdef __cplusplus
}
#endif


#endif // __VBSSCNVT_H__
 
