#include <windows.h>
#include "..\..\..\fplibs\fptools\src\fptools.h"
#include <toolbox.h>
#include <tchar.h>

#define  DRV_EDIT

#include "..\sssrc\libgen.h"
#include "..\sssrc\spread\spread.h"
#include "..\sssrc\classes\wintools.h"
#include "..\sssrc\classes\viewpict.h"
#include "..\sssrc\spread\ss_heap.h"
#ifdef SS_V30
#include "..\ssprvw\ssprvw.h"
#endif

#ifndef SS_OLDCALC
#include "..\sssrc\calc\cal_mem.h"
#endif

//#pragma message("Question: Should SmartHeap_malloc be set to zero?")
//int SmartHeap_malloc = 0;

static BOOL RegisterLibClass(HANDLE);
static BOOL DynamicInit = FALSE;

BOOL DLLENTRY UnregisterAllClasses(void);

#ifndef SS_NO_USE_SH
OMEM_POOL tbStringPool;
OMEM_POOL tbOmemPoolCol;
OMEM_POOL tbOmemPoolRow;
OMEM_POOL tbOmemPoolCell;
#endif
HINSTANCE hDynamicInst;
//GAB 10/6 - Changed from HINSTANCE to HANDLE
HANDLE fpInstance;

LPTSTR lpszSpinClassName = VA_SPINBTN_CLASS_NAME;

#ifdef WIN32
short dxPixelsPerInch;
short dyPixelsPerInch;
#else
short PASCAL dxPixelsPerInch;
short PASCAL dyPixelsPerInch;
#endif

#if defined(_WIN64) || defined(_IA64)
INT_PTR WINAPI DialogBoxProc(HINSTANCE, LPCTSTR, HWND, DLGPROC);
#else
int WINAPI DialogBoxProc(HINSTANCE, LPCTSTR, HWND, DLGPROC);
#endif

struct tagToolBoxGlobal
   {
   int ClearField;
   int CurrentDateTimeKey;
   int PopupCalendarKey;
   } ToolBoxGlobal = {0, 0, 0};
//   } ToolBoxGlobal = {VK_F2, VK_F3, VK_F4};

#ifdef WIN32
#define WM_CMD_ID(wParam)           LOWORD(wParam)
#else
#define WM_CMD_ID(wParam)           wParam
#endif

#ifdef WIN32

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
#ifdef SS_V75
  WORD wSplash = FPSPLASH_OK;
#else
  WORD wSplash = fpSplashIsExpired(hInst);
#endif
  switch( dwReason )
  {
    case DLL_PROCESS_ATTACH:
#if SS_V80
#ifdef FP_BETA
	  wSplash = fpSplashIsExpired(hInst);
#endif
#elif SS_V75
#if BETA
	  wSplash = fpSplashIsExpired(hInst);
#endif
#endif
      if (wSplash == FPSPLASH_EXPIRED)
        return (FALSE);

      {
        HDC hDC;
        hDC = GetDC(GetDesktopWindow());
        dxPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSX);
        dyPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSY);
        ReleaseDC(GetDesktopWindow(), hDC);
      }

      hDynamicInst = hInst;
      fpInstance = hInst;
      DynamicInit = TRUE;
#ifndef SS_NO_USE_SH
		/* RFW - 11/30/07 - 21744
      MemRegisterTask();
		*/
#endif
      RegisterLibClass(hInst);
      break;

    case DLL_PROCESS_DETACH:
      UnregisterAllClasses();
#ifndef SS_NO_USE_SH
		/* RFW - 11/30/07 - 21744
      MemUnregisterTask();
		*/
#endif
      break;
  }
  return TRUE;
}

#else

int PASCAL LibMain(HANDLE hInst, WORD wDataSeg, WORD wHeapSize, DWORD Ignore)
{
WORD wSplash = fpSplashIsExpired(hInst);
if (hDynamicInst)
   return (TRUE);

if (wSplash == FPSPLASH_EXPIRED)
  return (FALSE);

hDynamicInst = hInst;
fpInstance = hInst;
DynamicInit = TRUE;

{
HDC hDC;

hDC = GetDC(GetDesktopWindow());
dxPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSX);;
dyPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSY);;
ReleaseDC(GetDesktopWindow(), hDC);
}

if (wHeapSize != 0)
   UnlockData(0);

MemRegisterTask();

return (RegisterLibClass(hInst));
}

int FAR PASCAL WEP(int bSystemExit)
{
UnregisterAllClasses();
MemUnregisterTask();
return (1);
}

#endif

BOOL RegisterLibClass(HINSTANCE hInst)
{
WNDCLASS WndClass;
BOOL     fOk = TRUE;
ATOM atom;  // for debugging purposes

MemSet(&WndClass, 0, sizeof(WNDCLASS));

WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditGeneralWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor(NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_GENERAL_CLASS_NAME;
atom = RegisterClass( (LPWNDCLASS) &WndClass);

#ifndef SS_NOCT_DATE
WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditDateWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_DATE_CLASS_NAME;
atom = RegisterClass( (LPWNDCLASS) &WndClass);
#endif

#ifndef SS_NOCT_TIME
WndClass.style         = CS_GLOBALCLASS;
WndClass.lpfnWndProc   = (WNDPROC)EditTimeWndProc;
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_TIME_CLASS_NAME;
WndClass.cbWndExtra    = 2 * sizeof (HANDLE) + sizeof(FARPROC);
atom = RegisterClass( (LPWNDCLASS) &WndClass);
#endif

#ifndef SS_NOCT_INT
WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditIntegerWndProc;
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_INTEGER_CLASS_NAME;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
atom = RegisterClass( (LPWNDCLASS) &WndClass);
#endif

#ifndef SS_NOCT_FLOAT
WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditFloatWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_FLOAT_CLASS_NAME;
atom = RegisterClass( (LPWNDCLASS) &WndClass);
#endif

#ifndef SS_NOCT_PIC
WndClass.style         = CS_GLOBALCLASS;
WndClass.lpfnWndProc   = (WNDPROC)EditPicWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_PIC_CLASS_NAME;
atom = RegisterClass( (LPWNDCLASS) &WndClass);
#endif

#ifdef SS_V40
WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditNumWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_NUM_CLASS_NAME;
atom = RegisterClass( (LPWNDCLASS) &WndClass);
#endif

// The Calendar Class:
#ifndef SS_NOCALENDAR
#ifndef SS_NOCT_DATE
//WndClass.style         = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW | CS_GLOBALCLASS;
WndClass.style         = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
WndClass.lpfnWndProc   = (WNDPROC)CalendarWndProc;
WndClass.cbClsExtra    = 0;
WndClass.cbWndExtra    = sizeof (HANDLE);
WndClass.hInstance     = hInst;
WndClass.hIcon         = NULL;
WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_CALENDAR_CLASS_NAME;
atom = RegisterClass((LPWNDCLASS) &WndClass);
#endif
#endif

//RegisterViewPict(hInst, _T(VA_VIEWPICT_CLASS_NAME));
#ifndef SSP_NO_TYPESPIN
RegisterSpinBtn(hInst, VA_SPINBTN_CLASS_NAME);
#endif
RegisterSpreadSheet(hInst, VA_SPREAD_CLASS_NAME, 0);

#ifdef SS_V30
fOk = SSPreviewRegister(hInst);
#endif

return (fOk);
}

BOOL DLLENTRY UnregisterAllClasses()
{
  // Unregister the classes registered thru RegisterSpreadSheet()
  UnregisterClass(VA_SPREAD_CLASS_NAME, hDynamicInst);

  UnregisterClass(SSClassNames[dClassNameIndex].SSEdit, hDynamicInst);
  UnregisterClass(SSClassNames[dClassNameIndex].SSEditHScroll, hDynamicInst);

  #ifndef SS_NOCT_PIC
  UnregisterClass(SSClassNames[dClassNameIndex].SSPic, hDynamicInst);
  #endif  

  #ifndef SS_NOCT_DATE
  UnregisterClass(SSClassNames[dClassNameIndex].SSDate, hDynamicInst);
  #endif
  
  #ifndef SS_NOCT_INT
  UnregisterClass(SSClassNames[dClassNameIndex].SSInteger, hDynamicInst);
  #endif
  
  UnregisterClass(SSClassNames[dClassNameIndex].SSFloat, hDynamicInst);

#ifdef SS_V40
  UnregisterClass(SSClassNames[dClassNameIndex].SSNumber, hDynamicInst);
  UnregisterClass(SSClassNames[dClassNameIndex].SSPercent, hDynamicInst);
  UnregisterClass(SSClassNames[dClassNameIndex].SSCurrency, hDynamicInst);
#endif

#ifdef SS_V70
  UnregisterClass(SSClassNames[dClassNameIndex].SSScientific, hDynamicInst);
#endif // SS_V70

  #ifndef SS_NOCHILDSCROLL
  #ifndef SS_NOSCBAR
  UnregisterClass(SSClassNames[dClassNameIndex].TBScrlBar, hDynamicInst);
  #endif
  #endif

  #ifndef SS_NOCT_TIME
  UnregisterClass(SSClassNames[dClassNameIndex].SSTime, hDynamicInst);
  #endif

  #ifndef SS_NOCT_BUTTON
  UnregisterClass(SSClassNames[dClassNameIndex].SSSuperBtn, hDynamicInst);
  UnregisterClass(SSClassNames[dClassNameIndex].TBSuperBtn, hDynamicInst);
  #endif

  #ifndef SS_NOCT_CHECK
  UnregisterClass(SSClassNames[dClassNameIndex].SSCheckBox, hDynamicInst);
  UnregisterClass(SSClassNames[dClassNameIndex].TBCheckBox, hDynamicInst);
  #endif

  #ifndef SS_NOCT_COMBO
  UnregisterClass(SSClassNames[dClassNameIndex].SSComboBox, hDynamicInst);
  UnregisterClass(SSClassNames[dClassNameIndex].SSEditComboBox, hDynamicInst);
  UnregisterClass(SSClassNames[dClassNameIndex].TBComboBox, hDynamicInst);
  UnregisterClass(SSClassNames[dClassNameIndex].SSListBox, hDynamicInst);
  #endif

  UnregisterClass(VA_GENERAL_CLASS_NAME, hDynamicInst);
#ifndef SS_NOCT_DATE
  UnregisterClass(VA_DATE_CLASS_NAME, hDynamicInst);
#endif

#ifndef SS_NOCT_TIME
  UnregisterClass(VA_TIME_CLASS_NAME, hDynamicInst);
#endif

#ifndef SS_NOCT_INT
  UnregisterClass(VA_INTEGER_CLASS_NAME, hDynamicInst);
#endif

#ifndef SS_NOCT_FLOAT
  UnregisterClass(VA_FLOAT_CLASS_NAME, hDynamicInst);
#endif

#ifndef SS_NOCT_PIC
  UnregisterClass(VA_PIC_CLASS_NAME, hDynamicInst);
#endif

#ifdef SS_V40
  UnregisterClass(VA_NUM_CLASS_NAME, hDynamicInst);
#endif

#ifndef SS_NOCALENDAR
#ifndef SS_NOCT_DATE
  UnregisterClass(VA_CALENDAR_CLASS_NAME, hDynamicInst);
#endif
#endif
//  UnregisterClass(VA_VIEWPICT_CLASS_NAME, hDynamicInst);
#ifndef SSP_NO_TYPESPIN
  UnregisterClass(VA_SPINBTN_CLASS_NAME, hDynamicInst);
#endif

#ifdef SS_V30
SSPreviewUnregister(hDynamicInst);
#endif

  return TRUE;
}


BOOL DLLENTRY SetToolBoxGlobal(int nID, LONG lValue)
{
switch (nID)
   {
   case TBG_CLEARFIELD:
      ToolBoxGlobal.ClearField = (int) lValue;
      return TRUE;

   case TBG_CURRENTDATETIME:
      ToolBoxGlobal.CurrentDateTimeKey = (int) lValue;
      return TRUE;

   case TBG_POPUPCALENDAR:
      ToolBoxGlobal.PopupCalendarKey = (int) lValue;
      return TRUE;

   default:
      return FALSE;
   }
}

BOOL DLLENTRY GetToolBoxGlobal (int nID, LPLONG lplValue)
{
switch (nID)
   {
   case TBG_CLEARFIELD:
      *lplValue = (LONG) ToolBoxGlobal.ClearField;
      return TRUE;

   case TBG_CURRENTDATETIME:
      *lplValue = (LONG) ToolBoxGlobal.CurrentDateTimeKey;
      return TRUE;

   case TBG_POPUPCALENDAR:
      *lplValue = (LONG) ToolBoxGlobal.PopupCalendarKey;
      return TRUE;

   default:
      return FALSE;
   }
}


void SpreadInit(HWND hWnd)
{
}


short ViewGifLoadImage(HWND hWnd, LPVIEWPICT_PICTURE Picture,
                       LPCTSTR szGIFFile, LPBYTE GifResData,
                       long GifResDataSize)
{
return (0);
}


short ViewPcxLoadImage(HWND hWnd, LPVIEWPICT_PICTURE Picture,
                       LPCTSTR FileName, LPBYTE PcxResourceData,
                       long PcxResourceDataSize)
{
return (0);
}

#if defined(_WIN64) || defined(_IA64)
INT_PTR WINAPI DialogBoxProc(HINSTANCE hInst, LPCTSTR lpszDlg, HWND hWnd,
                         DLGPROC DlgProc)
#else
int WINAPI DialogBoxProc(HINSTANCE hInst, LPCTSTR lpszDlg, HWND hWnd,
                         DLGPROC DlgProc)
#endif
{
#if defined(_WIN64) || defined(_IA64)
INT_PTR iRet;
#else
int iRet;
#endif
iRet = DialogBox(hInst, lpszDlg, hWnd, DlgProc);
return iRet;
}


#if (defined(SS_NOCALENDAR) || defined(SS_NOCT_DATE))
BOOL DLLENTRY PopupCalendar(HWND hDateWnd)
{
return (0);
}
#endif
