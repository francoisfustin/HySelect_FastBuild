#include <windows.h>
#include "..\..\..\fplibs\fptools\src\fptools.h"
#include <toolbox.h>

#define  DRV_EDIT

#include "vbmisc.h"
#include "..\sssrc\libgen.h"
#include "..\sssrc\spread\spread.h"
#include "..\sssrc\classes\wintools.h"
#include "..\sssrc\classes\viewpict.h"
#include "..\sssrc\spread\ss_heap.h"
#include "vbspread.h" 

//#ifndef SS_OLDCALC
//#include "..\sssrc\calc\cal_mem.h"
//#endif

#ifdef SS_USEAWARE
#define  VBAPI_H 1
#include "utools.h"

#define  WININTL          "intl"
#define  WIN_THOUSAND     "sThousand"
#define  WIN_DECIMAL      "sDecimal"
#define  WIN_CURRENCY     "sCurrency"
#define  WIN_DEFCURRENCY  "$"
#define  WIN_DEFDECIMAL   "."
#define  WIN_DEFTHOUSAND  ","
#endif

#ifdef SS_QE
extern HANDLE _export FAR PASCAL xDB_GetDLLHandle(void);
#endif

static BOOL RegisterLibClass(HANDLE);
static BOOL DynamicInit = FALSE;

OMEM_POOL tbStringPool;
OMEM_POOL tbOmemPoolCol;
OMEM_POOL tbOmemPoolRow;
OMEM_POOL tbOmemPoolCell;
HANDLE    hDynamicInst;
HANDLE    fpInstance;

LPSTR     lpszSpinClassName = VA_SPINBTN_CLASS_NAME;

short PASCAL dxPixelsPerInch;
short PASCAL dyPixelsPerInch;

extern BOOL VBSSPreviewRegister(HANDLE hInstance);
int WINAPI              DialogBoxProc(HINSTANCE, LPCSTR, HWND, DLGPROC);
BOOL DLLENTRY           tbCheckBoxGetPict(HANDLE hInst, LPSTR lpDefPict);
LONG _export FAR PASCAL tbCheckBoxWndFn(HWND hWnd, WORD Msg, WORD wParam,
                                        LONG lParam);
LPSTR                   CheckBoxGetPtr(HWND hWnd, GLOBALHANDLE FAR *hGlobal);
void                    CheckBoxSetPict(HWND hWnd, HANDLE hInst,
                                        LPSTR lpPicts, WORD wParam,
                                        LONG lParam);
void                    CheckBoxPaint(HWND hWnd, LPSTR lpCheckBox,
                                      HDC hDC, LPRECT lpRect);
BOOL                    SSx_GetBorder(HWND hWnd, LPSPREADSHEET lpSpread,
                                      LPSS_COL lpCol, LPSS_ROW lpRow,
                                      LPSS_CELL lpCell, SS_COORD Col,
                                      SS_COORD Row, LPWORD lpwBorderType,
                                      LPWORD lpwBorderStyle,
                                      LPCOLORREF lpColor);
void                    DateGetDefFormat(LPDATEFORMAT lpDateFormat);
static void             DateFormatConvert(LPSTR lpszDateFormat,
                                          LPDATEFORMAT lpDateFormat);
static long             DateDMYToLong(LPTB_DATE lpDate);

struct tagToolBoxGlobal
   {
   int ClearField;
   int CurrentDateTimeKey;
   int PopupCalendarKey;
   } ToolBoxGlobal = {0, 0, 0};
//   } ToolBoxGlobal = {VK_F2, VK_F3, VK_F4};

BOOL FAR PASCAL _export VBINITCC(USHORT usVersion, BOOL fRuntime)
{
if (!SS_CheckLicense(SS_VBXNAME, SS_LICNAME, fRuntime))
   return (FALSE);

return (RegisterLibClass(hDynamicInst));
}
  
#if 0
VOID FAR PASCAL _export VBTERMCC()
{
#ifdef SS_QE
{
HANDLE hDLL;
if (hDLL = xDB_GetDLLHandle())
   FreeLibrary(hDLL);
}
#endif
MemUnregisterTask();
}
#endif

int PASCAL LibMain(HANDLE hInst, WORD wDataSeg, WORD wHeapSize, DWORD Ignore)
{
if (hDynamicInst)
   return (TRUE);

hDynamicInst = hInst;
DynamicInit = TRUE;
fpInstance = hInst;

{
HDC hDC = GetDC(GetDesktopWindow());
dxPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSX);;
dyPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSY);;
ReleaseDC(GetDesktopWindow(), hDC);
}

if (wHeapSize != 0)
   UnlockData(0);

MemRegisterTask();

return (TRUE);
}

int FAR PASCAL _export WEP(int bSystemExit)
{
#ifdef SS_QE
{
HANDLE hDLL;
if (hDLL = xDB_GetDLLHandle())
   FreeLibrary(hDLL);
}
#endif
MemUnregisterTask();
return (TRUE);
}


BOOL RegisterLibClass(HANDLE hInst)
{
WNDCLASS WndClass;
BOOL     fOk = TRUE;

MemSet(&WndClass, 0, sizeof(WNDCLASS));

/*
#ifdef SS_USEAWARE
WndClass.style          = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW | CS_GLOBALCLASS;
WndClass.hInstance      = hInst;
WndClass.hIcon          = NULL;
WndClass.hCursor        = LoadCursor(NULL, IDC_IBEAM);
WndClass.hbrBackground  = COLOR_WINDOW + 1;
WndClass.lpszMenuName   = (LPSTR) NULL;
WndClass.lpfnWndProc    = wrEditWndProc;
WndClass.lpszClassName  = "WREditCtl";
WndClass.cbWndExtra     = 4;
RegisterClass((LPWNDCLASS) &WndClass);
#endif
*/

#ifdef SS_USEAWARE
#ifdef SS_USEAWARE
WndClass.style         = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW | CS_GLOBALCLASS;
WndClass.lpfnWndProc   = wrTextWndProc;
WndClass.cbWndExtra    = 4;
#else
WndClass.style         = CS_GLOBALCLASS;
WndClass.lpfnWndProc   = (WNDPROC)EditGeneralWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
#endif
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor(NULL, IDC_IBEAM);
WndClass.hbrBackground = COLOR_WINDOW + 1;
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_GENERAL_CLASS_NAME;
RegisterClass( (LPWNDCLASS) &WndClass);
#endif

#ifndef SS_NOCT_DATE
#ifdef SS_USEAWARE
WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)wrDateWndProc;
WndClass.cbWndExtra    = 4;
#else
WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditDateWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
#endif
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = COLOR_WINDOW + 1;
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_DATE_CLASS_NAME;
RegisterClass( (LPWNDCLASS) &WndClass);
#endif

#ifndef SS_NOCT_TIME
WndClass.style         = CS_GLOBALCLASS;
WndClass.lpfnWndProc   = (WNDPROC)EditTimeWndProc;
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = COLOR_WINDOW + 1;
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_TIME_CLASS_NAME;
WndClass.cbWndExtra    = 2 * sizeof (HANDLE) + sizeof(FARPROC);
RegisterClass( (LPWNDCLASS) &WndClass);
#endif

#ifndef SS_NOCT_INT
WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditIntegerWndProc;
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = COLOR_WINDOW + 1;
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_INTEGER_CLASS_NAME;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
RegisterClass( (LPWNDCLASS) &WndClass);
#endif

#ifndef SS_NOCT_FLOAT
#ifdef SS_USEAWARE
WndClass.style         = CS_GLOBALCLASS;
WndClass.lpfnWndProc   = (WNDPROC)wrDoubleWndProc;
WndClass.cbWndExtra    = 4;
#else
WndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS;
WndClass.lpfnWndProc   = (WNDPROC)EditFloatWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
#endif
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = COLOR_WINDOW + 1;
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_FLOAT_CLASS_NAME;
RegisterClass( (LPWNDCLASS) &WndClass);
#endif

#ifndef SS_NOCT_PIC
#ifdef SS_USEAWARE
WndClass.style         = CS_GLOBALCLASS;
WndClass.lpfnWndProc   = (WNDPROC)wrMaskWndProc;
WndClass.cbWndExtra    = 4;
#else
WndClass.style         = CS_GLOBALCLASS;
WndClass.lpfnWndProc   = (WNDPROC)EditPicWndProc;
WndClass.cbWndExtra    = 2 * sizeof(HANDLE) + sizeof(FARPROC);
#endif
WndClass.hInstance     = hInst;
WndClass.hCursor       = LoadCursor (NULL, IDC_IBEAM);
WndClass.hbrBackground = COLOR_WINDOW + 1;
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_PIC_CLASS_NAME;
RegisterClass( (LPWNDCLASS) &WndClass);
#endif

// The Calendar Class:
#ifndef SS_NOCALENDAR
#ifndef SS_NOCT_DATE
#ifndef SS_USEAWARE
WndClass.style         = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
WndClass.lpfnWndProc   = (WNDPROC)CalendarWndProc;
WndClass.cbClsExtra    = 0;
WndClass.cbWndExtra    = sizeof (HANDLE);
WndClass.hInstance     = hInst;
WndClass.hIcon         = NULL;
WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
WndClass.hbrBackground = COLOR_WINDOW + 1;
WndClass.lpszMenuName  = NULL;
WndClass.lpszClassName = VA_CALENDAR_CLASS_NAME;
RegisterClass((LPWNDCLASS) &WndClass);
#endif
#endif
#endif

//RegisterViewPict(hInst, VA_VIEWPICT_CLASS_NAME);
#ifndef SSP_NO_TYPESPIN
RegisterSpinBtn(hInst, VA_SPINBTN_CLASS_NAME);
#endif
RegisterSpreadSheet(hInst, VA_SPREAD_CLASS_NAME, 0);

fOk = RegisterVBSpread(hInst);

#ifdef SS_V30
if (fOk)
   fOk = VBSSPreviewRegister(hInst);
#endif

return (fOk);
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
                       LPCSTR szGIFFile, LPBYTE GifResData,
                       long GifResDataSize)
{
return (0);
}


short ViewPcxLoadImage(HWND hWnd, LPVIEWPICT_PICTURE Picture,
                       LPCSTR FileName, LPBYTE PcxResourceData,
                       long PcxResourceDataSize)
{
return (0);
}


int WINAPI DialogBoxProc(HINSTANCE hInst, LPCSTR lpszDlg, HWND hWnd,
                         DLGPROC DlgProc)
{
//return (VBDialogBoxParam(hInst, (LPSTR)lpszDlg, DlgProc, 0L));
return DialogBox(hInst, lpszDlg, hWnd, DlgProc); // BJO 22Apr96 SEL3754
}


#if (defined(SS_NOCALENDAR) || defined(SS_NOCT_DATE))
BOOL DLLENTRY PopupCalendar(HWND hDateWnd)
{
return (0);
}
#endif


#ifdef SS_USEAWARE
BOOL DLLENTRY FloatGetValue(HWND hWnd, LPDOUBLE lpDouble)
{
if (SendMessage(hWnd, EM_GETNULL, 0, 0L))
   return (FALSE);

return (!(BOOL)SendMessage(hWnd, DM_GETVALUE, 0, (long)lpDouble));
}


BOOL DLLENTRY FloatGetFormat(HWND hWnd, LPFLOATFORMAT lpFloatFormat)
{
char szText[10];

if (hWnd == 0)
   {
   GetProfileString(WININTL, WIN_THOUSAND, WIN_DEFTHOUSAND, szText, 2);
   lpFloatFormat->cSeparator = szText[0];
   GetProfileString(WININTL, WIN_DECIMAL, WIN_DEFDECIMAL, szText, 2);
   lpFloatFormat->cDecimalSign = szText[0];
   GetProfileString(WININTL, WIN_CURRENCY, WIN_DEFCURRENCY, szText, 2);
   lpFloatFormat->cCurrencySign = szText[0];
   }

else
   {
   lpFloatFormat->cSeparator = (char)SendMessage(hWnd, DM_GETSEPARATOR, 0, 0L);
   lpFloatFormat->cDecimalSign = (char)SendMessage(hWnd, DM_GETDECIMAL, 0, 0L);
   SendMessage(hWnd, DM_GETCURRENCYSYMB, sizeof(szText), (long)(LPVOID)szText);
   lpFloatFormat->cCurrencySign = *szText;
   }

return (TRUE);
}


BOOL DLLENTRY FloatSetFormat(HWND hWnd, LPFLOATFORMAT lpFloatFormat)
{
char Buffer[1 + 1];

SendMessage(hWnd, DM_SETSEPARATOR, (WPARAM)lpFloatFormat->cSeparator, 0L);
SendMessage(hWnd, DM_SETDECIMAL, (WPARAM)lpFloatFormat->cDecimalSign, 0L);

Buffer[0] = lpFloatFormat->cCurrencySign;
Buffer[1] = '\0';
SendMessage(hWnd, DM_SETCURRENCYSYMB, 0, (long)(LPVOID)Buffer);
return (TRUE);
}


BOOL DLLENTRY FloatSetRange(HWND hWnd, double dMin, double dMax)
{
SendMessage(hWnd, DM_SETMIN, 0, (long)(LPDOUBLE)&dMin);
SendMessage(hWnd, DM_SETMAX, 0, (long)(LPDOUBLE)&dMax);
return (TRUE);
}


LPSTR DLLENTRY DateDMYToString(LPDATE lpDate, LPSTR lpStr,
                               LPDATEFORMAT lpDateFormat)
{
FPDATE wrDate;
FPTIME wrTime;
char   Buffer[20];

wrTime = fpNOTIME;
DateFormatConvert(Buffer, lpDateFormat);
wrDate = DateDMYToLong(lpDate);
wryDateWinFormat(lpStr, wrDate, wrTime, Buffer, 0, 0, 0);
return (lpStr);
}


BOOL DLLENTRY DateStringIsValid(LPSTR lpStr, LPDATEFORMAT lpDateFormat)
{
DATE Date;

if (!DateStringToDMY(lpStr, &Date, lpDateFormat))
   return (FALSE);

return (TRUE);
}


LPDATE DLLENTRY DateStringToDMY(LPSTR lpStr, LPDATE lpDate,
                                LPDATEFORMAT lpDateFormat)
{
FPDATE  fpDate;
LPSTR   bOrder;

//fpSET_DATE(fpDate, lpDate->nYear, lpDate->nMonth, lpDate->nDay);

switch (lpDateFormat->nFormat)
   {
   case IDF_DDMONYY:
   case IDF_DDMMYY:
      bOrder = "DMY"; //ORDER_DMY;
      break;

   case IDF_MMDDYY:
   //- CAUTION : Current version doesn't support this !!!!
//   case IDF_MONDDYY:
      bOrder = "MDY"; //ORDER_MDY;
      break;

   case IDF_YYMMDD:
      bOrder = "YMD"; //ORDER_YMD;

      break;

   }

wryStringToDateTime(&fpDate, NULL, lpStr, bOrder, METHOD_50_50, fpNODATE,
                    fpNODATE, fpNODATE, fpNOTIME, fpNOTIME, fpNOTIME, FALSE,
                    NULL, NULL, lpDateFormat->bCentury ? 2 : 0);

lpDate->nDay = fpDAY(fpDate);
lpDate->nMonth = fpMONTH(fpDate);
lpDate->nYear = fpYEAR(fpDate);

return (lpDate);
}


long DLLENTRY DateDMYToJulian(LPDATE lpDate)
{
FPDATE wrDate = DateDMYToLong(lpDate);
return (fpDateToJulian(wrDate));
}


BOOL DLLENTRY DateGetFormat(HWND hWnd, LPDATEFORMAT lpDateFormat)
{
DateGetDefFormat(lpDateFormat);
return (TRUE);
}


void DateGetDefFormat(LPDATEFORMAT lpDateFormat)
{
char  szTemp[20];
LPSTR Ptr;

switch ((BOOL)GetProfileInt("intl", "iDate", -1))
   {
   case -1:
   case 0:
      lpDateFormat->nFormat = IDF_MMDDYY;
      break;

   case 1:
      lpDateFormat->nFormat = IDF_DDMMYY;
      break;

   case 2:
      lpDateFormat->nFormat = IDF_YYMMDD;
      break;
   }

GetProfileString ("intl", "sDate", "/", szTemp, 2);
if (szTemp[0])
   lpDateFormat->cSeparator = szTemp[0];
else                                   // Windows seems to return NULL
   lpDateFormat->cSeparator = ' ';           //    if you specified a space.

GetProfileString("intl", "sShortDate", "M/d/yy", szTemp, 11);

lpDateFormat->bCentury = 0;

if ((Ptr = StrChr(szTemp, 'y')) || (Ptr = StrChr(szTemp, 'Y')))
   if (StriStr(Ptr, "yyyy"))
      lpDateFormat->bCentury = 1;

lpDateFormat->bSpin = 0;
}


BOOL DLLENTRY DateSetRange(HWND hWnd, LPDATE lpMin, LPDATE lpMax)
{
SendMessage(hWnd, DTM_SETDATA, DT_MINDATE, DateDMYToLong(lpMin));
SendMessage(hWnd, DTM_SETDATA, DT_MAXDATE, DateDMYToLong(lpMax));
return (TRUE);
}


BOOL DLLENTRY DateSetFormat(HWND hWnd, LPDATEFORMAT lpDateFormat)
{
char Buffer[20];

DateFormatConvert(Buffer, lpDateFormat);

// rdw-12/19/93
SendMessage(hWnd, DTM_SETSTYLE, DT_FORMAT, FORMAT_USERDEFINED);
// rdw-12/19/93.
SendMessage(hWnd, DTM_SETUSERDEFINED, 0, (long)(LPSTR)Buffer);

return (TRUE);
}


static void DateFormatConvert(LPSTR lpszDateFormat, LPDATEFORMAT lpDateFormat)
{
char yy[4 + 1];

if (lpDateFormat->bCentury)
   lstrcpy(yy, "yyyy");
else
   lstrcpy(yy, "yy");

switch (lpDateFormat->nFormat)
   {
   case IDF_DDMONYY:
      wsprintf(lpszDateFormat, "dd%cmmm%c%s", lpDateFormat->cSeparator,
               lpDateFormat->cSeparator, 
               (LPSTR)yy);
      break;

   case IDF_DDMMYY:
      wsprintf(lpszDateFormat, "dd%cmm%c%s", lpDateFormat->cSeparator,
               lpDateFormat->cSeparator, (LPSTR)yy);
      break;

   case IDF_MMDDYY:
      wsprintf(lpszDateFormat, "mm%cdd%c%s", lpDateFormat->cSeparator,
               lpDateFormat->cSeparator, (LPSTR)yy);
      break;

   case IDF_YYMMDD:
      wsprintf(lpszDateFormat, "%s%cmm%cdd", (LPSTR)yy,
               lpDateFormat->cSeparator, lpDateFormat->cSeparator);
      break;

   //- CAUTION : Current version doesn't support this !!!!
//   case IDF_MONDDYY:
//      wsprintf(lpszDateFormat, "mmm dd, %s", (LPSTR)yy);
//      break;

   }
}


static long DateDMYToLong(LPTB_DATE lpDate)
{
FPDATE wrDate;

fpSET_DATE(wrDate, lpDate->nYear, lpDate->nMonth, lpDate->nDay);

return (wrDate);
}

#endif
