/************************************************************************/
/* FPCONVRT.H  FarPoint VBX - OCX conversion for Common Code Simplicity */
/************************************************************************/
#ifndef __FPCONVRT_H__
#define __FPCONVRT_H__

// Keywords/Functions NOT SUPPORTED in WIN32
//
#ifdef WIN32
#ifndef _WIN32
#define _WIN32
#endif
#endif

#ifdef DLLENTRY
#undef DLLENTRY
#endif

//--------------------------------------------------------------------
//
//  Following definitions provide generic character and text types
//  which can map to SBCS, DBCS, or Unicode.
//

#if !defined(_TCHAR_DEFINED)
   #define _TCHAR_DEFINED
   #if defined(UNICODE) || defined(_UNICODE)
      typedef wchar_t TCHAR;
   #else
      typedef char TCHAR;
   #endif
#endif

#ifdef _WIN32
#define huge
#define _export
#define _huge
//#define __pascal
#define DATA_PASCAL  //since VC 4.1, "PASCAL" no longer applies to data.
#ifndef WIN32
#define WIN32
#endif

// SCP include windowsx.h instead of the following macros
#include "windowsx.h"

/* SCP
#define _fmemset(a, b, c)     memset(a, b, c)
#define _fmemcpy(a, b, c)     memcpy(a, b, c)
#define _fmemcmp(a, b, c)     memcmp(a, b, c)
#define _fmemicmp(a, b, c)    memicmp(a, b, c)
#define _fmemmove(a, b, c)    memmove(a, b, c)

#define _fstrlen(a)           strlen(a)
#define _fstrcpy(a, b)        strcpy(a, b)
#define _fstrcat(a, b)        strcat(a, b)
#define _fstrchr(a, b)        strchr(a, b)
#define _fstrcmp(a, b)        strcmp(a, b)
#define _fstricmp(a, b)       stricmp(a, b)
#define _fstrstr(a, b)        strstr(a, b)
#define _fstrtok(a, b)        strtok(a, b)

#define _fstrncpy(a, b, c)    strncpy(a, b, c)
#define _fstrncat(a, b, c)    strncat(a, b, c)
#define _fstrnchr(a, b, c)    strnchr(a, b, c)
#define _fstrncmp(a, b, c)    strncmp(a, b, c)
#define _fstrnicmp(a, b, c)   strnicmp(a, b, c)
#define _fstrnset(a, b, c)    strnset(a, b, c)

#define GlobalPtrHandle(lp)         \
            GlobalHandle(lp)

#define GlobalLockPtr(lp)		\
            ((BOOL)GlobalLock(GlobalPtrHandle(lp)))
#define GlobalUnlockPtr(lp) 	\
            GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalAllocPtr(flags, cb)	\
            (GlobalLock(GlobalAlloc((flags), (cb))))
#define GlobalReAllocPtr(lp, cbNew, flags)	\
            (GlobalUnlockPtr(lp), GlobalLock(GlobalReAlloc(GlobalPtrHandle(lp) , (cbNew), (flags))))
#define GlobalFreePtr(lp)		\
            (GlobalUnlockPtr(lp), (BOOL)GlobalFree(GlobalPtrHandle(lp)))
*/

#ifdef FP_API
#undef FP_API
#endif

#define FP_API      __declspec(dllexport) __stdcall

#define DLLENTRY    __declspec(dllexport) __stdcall
//#define DLLENTRY    __stdcall
#define FPDLLPROC   __stdcall

#define DLLENTRY_(type)  __declspec(dllexport) type __stdcall

// Use these to keep the fptools functions from being exported - SCP 10/7/98
#define FPLIB            __stdcall
#define FPLIB_(type)     type __stdcall

#define WM_CMD_CODE(wParam, lParam) HIWORD(wParam)
#define WM_CMD_ID(wParam)           LOWORD(wParam)
#define WM_CMD_HWND(lParam)         (HWND)lParam

#else //not WIN32

#define FP_API        FAR PASCAL _export

#define DLLENTRY      FAR PASCAL _export
#define FPDLLPROC     FAR PASCAL _export

#define DLLENTRY_(type)  type FP_API

// Use this to keep the fptools functions from being exported - SCP 10/7/98
#define FPLIB            FAR PASCAL
#define FPLIB_(type)     type FAR PASCAL

#define DATA_PASCAL   PASCAL

typedef TCHAR FAR        *LPTSTR;
typedef const TCHAR FAR  *LPCTSTR;

#define SetBrushOrgEx(a, b, c, d) SetBrushOrg(a, b, c)

#define WM_CMD_CODE(wParam, lParam) HIWORD(lParam)
#define WM_CMD_ID(wParam)           wParam
#define WM_CMD_HWND(lParam)         LOWORD(lParam)

//  accelerator and mnemonic defines
#define FVIRTKEY  TRUE
#define FNOINVERT 0x02
#define FSHIFT    0x04
#define FCONTROL  0x08
#define FALT      0x10

#ifndef RC_INVOKED
#pragma pack(1)
typedef struct tagACCEL
   {
   BYTE fVirt;
   WORD key;
   WORD cmd;
   } ACCEL, FAR *LPACCEL;
#pragma pack()    
#endif

#endif


// SCP 12/5/96
#ifndef FP_PREFIX
  #if defined(FP_OCX)         // Prefix for OCX functions
    #define FP_PREFIX(x)  X_##x
  #elif (defined(FP_DLL) && !defined(FP_OCX))           // Prefix for DLL functions
    #define FP_PREFIX(x)  x
  #elif defined(FP_VB)          // Prefix for VBX functions
    #define FP_PREFIX(x)  V_##x   
  #endif
#endif


#ifndef VBAPI_H
#if (defined(FP_OCX) || defined(FP_DLL))
#include "fpvbsub.h"
#else
#include "vbapi.h"
#define ERR_DEFINED
#endif
#endif

#ifndef ERR_DEFINED
typedef UINT ERR;
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif  // ifndef RC_INVOKED

typedef struct tagFPCONTROL
  {
    HWND hWnd;	           // handle of window, if available  (DLL, VBX)
	HCTL hCtl;			   // handle of control, if available (VBX)
	LONG lObject;		   // MFC object pointer, if available (OCX, DLL)
	LONG lpDispatch;	   // IDispatch pointer, if available (OCX only)
  } FPCONTROL, FAR *LPFPCONTROL;

typedef struct tagSCALEMODE
  {
    short nScaleMode; 
    long  lHeight;
    long  lWidth;
    float fScaleTop;
    float fScaleLeft;
    float fScaleHeight;
    float fScaleWidth;
  } SCALEMODE, FAR *LPSCALEMODE;

#ifndef RC_INVOKED
#pragma pack()
#endif  // ifndef RC_INVOKED

#define FP_CTLTYPE_DLL 0
#define FP_CTLTYPE_VBX 1
#define FP_CTLTYPE_OCX 2

#define FP_LINEORIENT_UL 0
#define FP_LINEORIENT_UR 1
#define FP_LINEORIENT_LR 2
#define FP_LINEORIENT_LL 3

#ifdef __cplusplus
extern "C" short DATA_PASCAL dxPixelsPerInch;
extern "C" short DATA_PASCAL dyPixelsPerInch;
#else
extern short DATA_PASCAL dxPixelsPerInch;
extern short DATA_PASCAL dyPixelsPerInch;
#endif

#define PT_TO_PIXELS(pt)   (short)(((double)(pt) * (double)dyPixelsPerInch / 72.0) + 0.5);
#define PIXELS_TO_PT(Pels) (float)((double)(Pels) * 72.0 / (double)dyPixelsPerInch)

/*
#define XUnitsToPixels(Units, nUnitsPerInch)  (short)((((double)(Units) * (double)dxPixelsPerInch) / nUnitsPerInch) + 0.5)
#define YUnitsToPixels(Units, nUnitsPerInch)  (short)((((double)(Units) * (double)dyPixelsPerInch) / nUnitsPerInch) + 0.5)
#define XPixelsToUnits(nPixels, nUnitsPerInch) (((double)(nPixels) * nUnitsPerInch / (double)dxPixelsPerInch) + 0.5)
#define YPixelsToUnits(nPixels, nUnitsPerInch) (((double)(nPixels) * nUnitsPerInch / (double)dyPixelsPerInch) + 0.5)
*/
#define XUnitsToPixels(Units, nUnitsPerInch)  (short)((Units) / (long)(nUnitsPerInch / (double)dxPixelsPerInch))
#define YUnitsToPixels(Units, nUnitsPerInch)  (short)((Units) / (long)(nUnitsPerInch / (double)dyPixelsPerInch))
#define XPixelsToUnits(Units, nUnitsPerInch)  (long)((Units) * (long)(nUnitsPerInch / (double)dxPixelsPerInch))
#define YPixelsToUnits(Units, nUnitsPerInch)  (long)((Units) * (long)(nUnitsPerInch / (double)dyPixelsPerInch))

#define XPixelsToRealUnits(nPixels, nUnitsPerInch) XPixelsToUnits(nPixels, nUnitsPerInch)
#define YPixelsToRealUnits(nPixels, nUnitsPerInch) YPixelsToUnits(nPixels, nUnitsPerInch)

#define FPTWIPS_PER_INCH 1440L

#define XTwipsToPixels(lTwips) XUnitsToPixels(lTwips, FPTWIPS_PER_INCH)
#define YTwipsToPixels(lTwips) YUnitsToPixels(lTwips, FPTWIPS_PER_INCH)
#define XPixelsToTwips(nPixels) XPixelsToUnits(nPixels, FPTWIPS_PER_INCH)
#define YPixelsToTwips(nPixels) YPixelsToUnits(nPixels, FPTWIPS_PER_INCH)

#define FPHIMETRIC_PER_INCH 2540L      // Number of HIMETRIC units per inch

#define XHiMetricToPixels(lTwips) XUnitsToPixels(lTwips, FPHIMETRIC_PER_INCH)
#define YHiMetricToPixels(lTwips) YUnitsToPixels(lTwips, FPHIMETRIC_PER_INCH)
#define XPixelsToHiMetric(nPixels) XPixelsToUnits(nPixels, FPHIMETRIC_PER_INCH)
#define YPixelsToHiMetric(nPixels) YPixelsToUnits(nPixels, FPHIMETRIC_PER_INCH)

#define PTS_PER_INCH 72.0

#define XPointsToPixels(lTwips) XUnitsToPixels(lTwips, PTS_PER_INCH)
#define YPointsToPixels(lTwips) YUnitsToPixels(lTwips, PTS_PER_INCH)
#define XPixelsToPoints(nPixels) XPixelsToRealUnits(nPixels, PTS_PER_INCH)
#define YPixelsToPoints(nPixels) YPixelsToRealUnits(nPixels, PTS_PER_INCH)

//#define XCHARS_PER_INCH 120.0
//#define YCHARS_PER_INCH 240.0
#define XCHARS_PER_INCH 6.0
#define YCHARS_PER_INCH 6.0

#define XCharsToPixels(lTwips) XUnitsToPixels(lTwips, XCHARS_PER_INCH)
#define YCharsToPixels(lTwips) YUnitsToPixels(lTwips, YCHARS_PER_INCH)
#define XPixelsToChars(nPixels) XPixelsToRealUnits(nPixels, XCHARS_PER_INCH)
#define YPixelsToChars(nPixels) YPixelsToRealUnits(nPixels, YCHARS_PER_INCH)

#define XInchesToPixels(lTwips) XUnitsToPixels(lTwips, 1)
#define YInchesToPixels(lTwips) YUnitsToPixels(lTwips, 1)
#define XPixelsToInches(nPixels) XPixelsToRealUnits(nPixels, 1)
#define YPixelsToInches(nPixels) YPixelsToRealUnits(nPixels, 1)

#define MM_PER_INCH (1440.0 / 56.7)

#define XMMToPixels(lTwips) XUnitsToPixels(lTwips, MM_PER_INCH)
#define YMMToPixels(lTwips) YUnitsToPixels(lTwips, MM_PER_INCH)
#define XPixelsToMM(nPixels) XPixelsToUnits(nPixels, MM_PER_INCH)
#define YPixelsToMM(nPixels) YPixelsToUnits(nPixels, MM_PER_INCH)

#define CM_PER_INCH (1440.0 / 567.0)

#define XCMToPixels(lTwips) XUnitsToPixels(lTwips, CM_PER_INCH)
#define YCMToPixels(lTwips) YUnitsToPixels(lTwips, CM_PER_INCH)
#define XPixelsToCM(nPixels) XPixelsToUnits(nPixels, CM_PER_INCH)
#define YPixelsToCM(nPixels) YPixelsToUnits(nPixels, CM_PER_INCH)



#define VBXTwipsToPixels(lTwips)  XTwipsToPixels(lTwips)
#define VBYTwipsToPixels(lTwips)  YTwipsToPixels(lTwips)
#define VBXPixelsToTwips(nPixels) XPixelsToTwips(nPixels)
#define VBYPixelsToTwips(nPixels) YPixelsToTwips(nPixels)

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(FP_VB) || defined(FP_OCX)) //not FP_DLL
// SCP 12/6/96 - These are functions defined in fpconv3.cpp. "FP_PREFIX" is
// declared in fptools.h.
#define fpVBGetControlRect      FP_PREFIX(fpVBGetControlRect)
#define fpGetWindowRect         FP_PREFIX(fpGetWindowRect)
#define fpGetRectFromDispatch   FP_PREFIX(fpGetRectFromDispatch)
#define fpGetRectFromDispatchEx   FP_PREFIX(fpGetRectFromDispatchEx)
#define fpSetRectFromDispatch   FP_PREFIX(fpSetRectFromDispatch)
// like fpSetRectFromDispatch, except it defaults to pixels if the container does not support ScaleMode -scl
#define fpSetRectFromDispatchEx   FP_PREFIX(fpSetRectFromDispatchEx)
#define fpSetRectPartsFromDispatch   FP_PREFIX(fpSetRectPartsFromDispatch)
// like fpSetRectFromPartsFromDispatch, except it defaults to pixels if the container does not support ScaleMode -scl
#define fpSetRectPartsFromDispatchEx   FP_PREFIX(fpSetRectPartsFromDispatchEx)
#define fpVBGetScaleMode        FP_PREFIX(fpVBGetScaleMode)
// like fpVBGetScaleMode, except it defaults to pixels if the container does not support ScaleMode -scl
#define fpVBGetScaleModeEx        FP_PREFIX(fpVBGetScaleModeEx)
#define fpVBGetRectInContainer  FP_PREFIX(fpVBGetRectInContainer)
#define fpVBMoveControl         FP_PREFIX(fpVBMoveControl)
#define fpScaleModeToUnits      FP_PREFIX(fpScaleModeToUnits) 
#define fpPixelsToScaleMode     FP_PREFIX(fpPixelsToScaleMode) 

  HANDLE  FPLIB fpVBGetPic(HANDLE hpic, LPPIC lppic);
  LONG    FPLIB fpBeginEnum(LPFPCONTROL lpObject, LPLONG lpAlt, LPLONG lpEnum, LONG lPrevDisp);
  LONG    FPLIB fpNextEnum(LONG lpEnum, LONG lPrevDisp);
  void    FPLIB fpEndEnum(LONG AltEnum, LONG lEnum);
  HANDLE  FPLIB fpVBGetHInstance(LPFPCONTROL lpObject);
  BOOL    FPLIB fpVBGetScaleMode(LPFPCONTROL lpObject, LPSCALEMODE lpScaleMode, BOOL bContainerOnly);
// like fpVBGetScaleMode, except it defaults to pixels if the container does not support ScaleMode -scl
  BOOL    FPLIB fpVBGetScaleModeEx(LPFPCONTROL lpObject, LPSCALEMODE lpScaleMode, BOOL bContainerOnly, BOOL fDefaultToPixels);
  void    FPLIB fpVBRefreshBrowser(LPFPCONTROL lpObject);
  void    FPLIB fpDispatchAddRef(LONG lObject);
  void    FPLIB fpDispatchRelease(LONG lObject);
  USHORT  FPLIB fpVBGetMode(LPFPCONTROL lpObject);
  HSZ     FPLIB fpVBCreateHsz(LPFPCONTROL lpObject, LPTSTR lpszString);
  void    FPLIB fpVBFreePic(LONG hPicture);
  BYTE    FPLIB fpVBGetControlRect(LPFPCONTROL lpObject, LPRECT lprect);
  HLSTR   FPLIB fpVBCreateHlstr(LPVOID pb, USHORT cbLen);
  HSZ     FPLIB fpVBDestroyHsz(HSZ hszParm);
  USHORT  FPLIB fpVBGetHlstrLen(HLSTR hlstr);
  SHORT   FPLIB fpVBGetVersion();
  LONG    FPLIB fpVBGetCapture();
  LPSTR   FPLIB fpVBDerefHsz(HSZ hsz);
  LPSTR   FPLIB fpVBDerefZeroTermHlstr(HLSTR hlstr);
  LONG    FPLIB fpVBDefControlProc(LPFPCONTROL lpObject, USHORT usMsg, 
                                    WPARAM wp, LPARAM lp);
  ERR     FPLIB fpVBSetControlProperty(LPFPCONTROL lpObject, USHORT usPropID, LONG lData, LPTSTR lpszPropName, USHORT usDataType);
  ERR     FPLIB fpVBGetControlProperty(LPFPCONTROL lpObject, USHORT usPropID, LPVOID lData, LPTSTR lpszPropName, USHORT usDataType);
  void    FPLIB fpVBUpdateControl(LPFPCONTROL lpObject);                           
  LPVOID  FPLIB fpVBDerefControl(LPFPCONTROL lpObject); 
  LONG    FPLIB fpVBGetHwndControl(LPFPCONTROL lpObject); 
#if 0
  BOOL    FPLIB fpIsWindowOLE(HWND hWnd);
  BOOL    FPLIB fpGetDispatch(LPFPCONTROL lpObject);
#endif // 0
  LONG    FPLIB fpVBGetControl(LPFPCONTROL lpObject, WORD wGC); 
  LONG    FPLIB fpVBSendControlMsg(LPFPCONTROL lpObject, USHORT usMsg, 
                                      WPARAM wp, LPARAM lp);
  HANDLE  FPLIB fpVBGetPicEx(HANDLE hpic, LPPIC lppic, USHORT usVersion);
  HANDLE  FPLIB fpVBRefPic(HANDLE hpic);                                  
  LPSTR   FPLIB fpVBGetControlName(LPFPCONTROL lpObject, LPSTR lpszName);                
  void    FPLIB fpVBInvalidateRect(LPFPCONTROL lpObject, LPRECT lprect, BOOL fEraseBkGnd);     
  HWND    FPLIB fpVBGetControlHwnd(LPFPCONTROL lpObject);
  LPMODEL FPLIB fpVBGetControlModel(LPFPCONTROL lpObject);
  void    FPLIB fpVBMoveControl(LPFPCONTROL lpObject, LPRECT lprect, BOOL fRepaint,
                                 BYTE bLineOrient);
  BYTE    FPLIB fpVBGetRectInContainer(LPFPCONTROL lpObject, LPRECT lprect);
  LONG    FPLIB fpVBGetDataSourceControl(LPFPCONTROL lpObject, BOOL FAR *lpbFlag);           
  int     FPLIB fpVBDialogBoxParam(LPFPCONTROL lpObject, HANDLE hinst, LPSTR lpszTemplateName, 
                                    FARPROC lpDialogFunc, LONG lp);
  SHORT   FPLIB fpVBFormat(LPFPCONTROL lpObject, SHORT vtype, LPVOID lpData, 
                                        LPTSTR lpszFmt, LPVOID pb, USHORT cb);
  BOOL    FPLIB fpVBRegisterModel(HANDLE hmod, LPMODEL lpmdl);
  LPSTR   FPLIB fpVBDerefHlstr(HLSTR hlstr);
  void    FPLIB fpVBDestroyHlstr(HLSTR hlstr);                           
  void    FPLIB fpVBDirtyForm(LPFPCONTROL lpObject);                               
  ERR     FPLIB fpVBFireEvent(LPFPCONTROL lpObject, USHORT usidEvent, LPVOID lpparams);
  ERR     FPLIB fpVBSetVariantValue(LPVAR lpVar, SHORT vtype, LPVOID lpData);
  ERR     FPLIB fpVBCoerceVariant(LPVAR lpVar, SHORT vtype, LPVOID lpData);
  LPSTR   FPLIB fpVBDerefHlstrLen(HLSTR hlstr, USHORT FAR *pcbLen);

  LONG    FPLIB fpVBSuperControlProc(LPFPCONTROL lpObject, USHORT msg, USHORT wp, LONG lp);
  ERR     FPLIB fpVBRecreateControlHwnd(LPFPCONTROL lpObject);
  ERR     FPLIB fpVBSetErrorMessage(ERR err, LPSTR lpstr);
  VOID    FPLIB fpVBGetAppTitle(LPSTR lpstr, USHORT cbMax);
  ULONG   FPLIB fpVBSetControlFlags(LPFPCONTROL lpObject, ULONG mask, ULONG value);
  VOID    FPLIB fpVBSetCapture(LPFPCONTROL lpObject);
  VOID    FPLIB fpVBReleaseCapture(VOID);
  VOID    FPLIB fpVBGetClientRect(LPFPCONTROL lpObject, LPRECT lprect);
  VOID    FPLIB fpVBClientToScreen(LPFPCONTROL lpObject, LPPOINT lppt);
  VOID    FPLIB fpVBScreenToClient(LPFPCONTROL lpObject, LPPOINT lppt);
  BOOL    FPLIB fpVBIsControlVisible(LPFPCONTROL lpObject);
  BOOL    FPLIB fpVBIsControlEnabled(LPFPCONTROL lpObject);
  VOID    FPLIB fpVBZOrder(LPFPCONTROL lpObject, WORD zorder);

  LPSTR   FPLIB fpVBLockHsz(HSZ hsz);
  VOID    FPLIB fpVBUnlockHsz(HSZ hsz);
  HLSTR   FPLIB fpVBCreateTempHlstr(LPVOID pb, USHORT cbLen);
  USHORT  FPLIB fpVBGetHlstr(HLSTR hlstr, LPVOID pb, USHORT cbLen);
  ERR     FPLIB fpVBSetHlstr(LPHLSTR lphlstr, LPVOID pb, USHORT cbLen);
  ERR     FPLIB fpVBResizeHlstr(HLSTR hlstr, USHORT newCbLen);
  ERR     FPLIB fpVBCoerceVariant(LPVAR lpVar, SHORT vtype, LPVOID lpData);
  SHORT   FPLIB fpVBGetVariantType(LPVAR lpVar);
  SHORT   FPLIB fpVBGetVariantValue(LPVAR lpVar, LPVAL lpVal);
  ERR     FPLIB fpVBSetVariantValue(LPVAR lpVar, SHORT vtype, LPVOID lpData);
  LPVOID  FPLIB fpVBArrayElement(HAD hAD, SHORT cIndex, LPSHORT lpi);
  LONG    FPLIB fpVBArrayBounds(HAD hAD, SHORT index);
  USHORT  FPLIB fpVBArrayElemSize(HAD hAD);
  LPVOID  FPLIB fpVBArrayFirstElem(HAD hAD);
  SHORT   FPLIB fpVBArrayIndexCount(HAD hAD);

  VOID    FPLIB fpVBRuntimeError(ERR err);
  USHORT  FPLIB fpVBCbSaveFPState(LPVOID pb, USHORT cb);
  VOID    FPLIB fpVBRestoreFPState(LPVOID pb);
  LONG    FPLIB fpVBAllocPic(LPPIC lppic);
  HANDLE  FPLIB fpVBAllocPicEx(LPPIC lppic, USHORT usVersion);
  ERR     FPLIB fpVBPicFromCF(LPHPIC lphpic, HANDLE hData, WORD wFormat);
  VOID    FPLIB fpVBPaletteChanged(LPFPCONTROL lpObject);
  COLOR   FPLIB fpVBTranslateColor(LPFPCONTROL lpObject, COLOR clr);
  ERR     FPLIB fpVBReadFormFile(HFORMFILE hformfile, LPVOID pb, WORD cb);
  ERR     FPLIB fpVBWriteFormFile(HFORMFILE hformfile, LPVOID pb, WORD cb);
  LONG    FPLIB fpVBSeekFormFile(HFORMFILE hformfile, LONG offset);
  LONG    FPLIB fpVBRelSeekFormFile(HFORMFILE hformfile, LONG offset);
  ERR     FPLIB fpVBReadBasicFile(USHORT usFileNo, LPVOID pb, WORD cb);
  ERR     FPLIB fpVBWriteBasicFile(USHORT usFileNo, LPVOID pb, WORD cb);
  LONG    FPLIB fpVBSeekBasicFile(USHORT usFileNo, LONG offset);
  LONG    FPLIB fpVBRelSeekBasicFile(USHORT usFileNo, LONG offset);
  ERR     FPLIB fpVBLinkPostAdvise(LPFPCONTROL lpObject);
  BOOL    FPLIB fpVBPasteLinkOk(HANDLE FAR *phTriplet, LPFPCONTROL lpObject);
  VOID    FPLIB fpVBLinkMakeItemName(LPFPCONTROL lpObject, LPSTR lpszBuf);
  VOID    FPLIB fpScaleModeToUnits(LPSCALEMODE lpScaleMode, float flX, float flY,
                        LPLONG lpXPixels, LPLONG lpYPixels, BOOL fSize);
  VOID    FPLIB fpPixelsToScaleMode(LPSCALEMODE lpScaleMode, long lX, long lY,
                        float far * lpXUnits, float far * lpYUnits, BOOL fSize);

#ifdef FP_OCX
  LPTSTR    FPLIB fpStr2OLE(LPTSTR lpszInput);
  LPTSTR    FPLIB fpStr2OLEByteLen(LPTSTR lpszInput, int iCharCt);
  LPTSTR    FPLIB fpOLE2Str(LPTSTR lpOleInput);  //LPOLECHAR lpOleInput
  void      FPLIB fpPatchVBVariant(VARIANT FAR *lpVariant);
  void      FPLIB fpEnumPropsHelper(LPFPCONTROL lpObject, LPSTR *bstrPropNames, int cPropNames);
  void      FPLIB fpEnumFuncsHelper(LPFPCONTROL lpObject, LPSTR *bstrFuncNames, int cFuncNames);
  LONG      FPLIB fpEnumObject(LPFPCONTROL lpObject, DWORD dwWhich);

  #define FP_IDC_ABOUTBOX_PROPS_BTN   2501
  void      FPLIB fpOcxAboutBox(LPVOID lpOleControl, UINT nIDTemplate, 
                                 BOOL fTrialVersion);  // fpconv8.cpp  SCP 2/27/97
#endif //FP_OCX

#endif  //not FP_DLL

  // window helper functions
  LONG      FPLIB fpGetControlDispatch(LPFPCONTROL lpObject, WORD wGC);
  HANDLE    FPLIB fpAttachPicture(LPHANDLE lPict, LPARAM lParam);                           
  void      FPLIB fpDetachPicture(LPHANDLE lPict);                           
  LONG      FPLIB fpPictureFromHolder(LONG lPict);
  BOOL      FPLIB fpIsPictureEmpty(LONG lPict);                           
  BOOL      FPLIB fpPostMessage(LPFPCONTROL lpObject, UINT Msg, 
                                   WPARAM wParam, LPARAM lParam);						
  LRESULT   FPLIB fpSendMessage(LPFPCONTROL lpObject, UINT Msg, 
                                   WPARAM wParam, LPARAM lParam);
  HWND      FPLIB fpGetParent(LPFPCONTROL lpObject);
  HWND      FPLIB fpGetFocus(LPFPCONTROL lpObject);
  HWND      FPLIB fpSetFocus(LPFPCONTROL lpObject);                           
  void      FPLIB fpUpdateWindow(LPFPCONTROL lpObject);                           
  void      FPLIB fpGetClientRect(LPFPCONTROL lpObject, LPRECT lprect); 
  HINSTANCE FPLIB fpGetInstanceHandle(LPFPCONTROL lpObject);
  void      FPLIB fpInvalidateRect(LPFPCONTROL lpObject, LPRECT lprect, BOOL bErase);
  BOOL      FPLIB fpMoveWindow(LPFPCONTROL lpObject, int x, int y, int cx, int cy,
                                BOOL fRepaint, BYTE bLineOrient);
  BYTE      FPLIB fpGetWindowRect(LPFPCONTROL lpObject, LPRECT lprect);
  BOOL      FPLIB fpIsWindow(LPFPCONTROL lpObject);
  BOOL      FPLIB fpIsEnabled(LPFPCONTROL lpObject);
  BYTE      FPLIB fpGetRectFromDispatch(LPFPCONTROL lpObject, LPRECT lprect);
  BYTE      FPLIB fpGetRectFromDispatchEx(LPFPCONTROL lpObject, LPRECT lprect);
  void      FPLIB fpSetRectFromDispatch(LPFPCONTROL lpObject, LPRECT lprect,
                                         BYTE bLineOrient);
// like fpSetRectFromDispatch, except it defaults to pixels if the container does not support ScaleMode -scl
  void      FPLIB fpSetRectFromDispatchEx(LPFPCONTROL lpObject, LPRECT lprect,
                                         BYTE bLineOrient, BOOL fDefaultToPixels);
  void      FPLIB fpSetRectPartsFromDispatch(LPFPCONTROL lpObject, LPRECT lprect,
                                         BYTE bLineOrient, BOOL bTop, BOOL bLeft, BOOL bHeight, BOOL bWidth);
// like fpSetRectFromPartsFromDispatch, except it defaults to pixels if the container does not support ScaleMode -scl
  void      FPLIB fpSetRectPartsFromDispatchEx(LPFPCONTROL lpObject, LPRECT lprect,
                                         BYTE bLineOrient, BOOL bTop, BOOL bLeft, BOOL bHeight, BOOL bWidth, BOOL fDefaultToPixels);
  short     FPLIB fpCountControls(LPFPCONTROL lpObject);
  BOOL      FPLIB fpRenderWindow(HWND hWnd, HDC hDC, LPRECT lpRect, BOOL fClientOnly);
#ifdef __cplusplus
}
 #endif


#endif // __FPCONVRT_H__
 
