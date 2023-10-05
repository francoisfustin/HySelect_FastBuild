/*****************************************************************************
* FarPoint Technologies  (C) Copyright 1991, 1992, 1993  All Rights Reserved
* portions (C) Copyright 1991, 1992 Applegate Sotware
*****************************************************************************/

#ifndef TOOLBOX_H
#define TOOLBOX_H

#ifdef SD40
#include "ss30to40.h"
#else
#include "ssdll.h"
#endif

#ifndef SS_DLL
#undef SS_HEADER
#define SS_HEADER              -1000
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif  /* RC_INVOKED */

#if defined( __cplusplus )
extern "C" {
#endif

//#define SS_VB
//#define SS_OCX

/***************************************
* Include for Drover Toolbox libraries
***************************************/

#ifndef RC_INVOKED

/*
#define  DRV_NOCLASS
#define  DRV_NODIALOG
#define  DRV_NOGAUGE
#define  DRV_NOPATTERN
#define  DRV_NOSORT
#define  DRV_NOSYS

#define  TB_NOSPREAD
*/

#undef  GHND
#define GHND       (GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_DDESHARE)
 
// WINDOWSX.H contains define statements to map _fmemcpy, _fmemcmp, etc
// to memcpy, memcmp, etc.  The keywords huge and _export are not needed
// or supported in WIN32.
#if defined(WIN32) || defined(_WIN32)
  #include "windowsx.h"
  #define huge
  #define _export
#endif

#ifdef WINENTRY
#undef WINENTRY
#endif

#ifdef DLGENTRY
#undef DLGENTRY
#endif

#if defined(_WIN64) || defined(_IA64)
   #if defined(SS_DLL) || defined(SS_OCX) || defined(SS_VB)
      #define WINENTRY LONG_PTR __declspec(dllexport) __stdcall
      #define DLGENTRY BOOL __declspec(dllexport) __stdcall
   #else
      #define WINENTRY LONG_PTR __stdcall
      #define DLGENTRY BOOL __stdcall
   #endif
   #define MAKELONG_PTR(a, b)      ((LONG_PTR)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((LONG_PTR)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#elif defined(WIN32) || defined(_WIN32)
   #if defined(SS_DLL) || defined(SS_OCX) || defined(SS_VB)
      #define WINENTRY LONG __declspec(dllexport) __stdcall
      #define DLGENTRY BOOL __declspec(dllexport) __stdcall
   #else
      #define WINENTRY LONG __stdcall
      #define DLGENTRY BOOL __stdcall
   #endif
#else
   #if defined(SS_DLL) || defined(SS_OCX) || defined(SS_VB)
      #define WINENTRY LONG __export FAR PASCAL
      #define DLGENTRY BOOL __export FAR PASCAL
   #else
      #define WINENTRY LONG FAR PASCAL
      #define DLGENTRY BOOL FAR PASCAL
   #endif
#endif

#ifndef WINAPI
#define WINAPI _far _pascal
#endif

#ifndef CALLBACK
#define CALLBACK _far _pascal
#endif

#if !defined(_TUCHAR)
  #if defined(UNICODE) || defined(_UNICODE)
    typedef wchar_t _TUCHAR;
  #else
    typedef unsigned char _TUCHAR;
  #endif
#endif

typedef HPALETTE FAR *LPHPALETTE;
typedef HBITMAP FAR *LPHBITMAP;
typedef HFONT FAR *LPHFONT;

#if (WINVER < 0x030a)
typedef unsigned int UINT;
typedef UINT WPARAM;
typedef LONG LPARAM;
typedef LONG LRESULT;
typedef HANDLE HINSTANCE;
typedef const char FAR* LPCSTR;
typedef FARPROC DLGPROC;
typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
#endif

#define DLL_TBPRO2 0x01
#define DLL_TBPRO3 0x02
#define DLL_TBPRO4 0x04
#define DLL_TBPRO5 0x08
#define DLL_TBPRO6 0x10

#define DRIVELEN   3                 /* max length of drive in file name */
#define DIRLEN     130               /* max length of directory */
#define NAMELEN    10                /* max length of name in file name */
#define EXTLEN     5                 /* max length of extn in file name */
#define FILELEN    (NAMELEN+EXTLEN)  /* max length of file name */
#ifndef PATHLEN
#define PATHLEN    (DRIVELEN+DIRLEN+FILELEN) /* max full file name path */
#endif

#define ERRLEN     100               /* ERROR length */
#define INTLEN     7                 /* max integer string length */
#define LONGLEN    11                /* max long string length */
#define DATELEN    12                /* Length of date string: dd-mmm-[yy]yy */
#define LIBVERSION 30                /* LIBRARY VERSION */

#define TB_INT_MAX        (32767)        /* max integer supported - rdw */
#define TB_INT_MIN       (-32767)        /* min integer supported - rdw */
#define TB_INT_OVERFLOW  (TB_INT_MIN-1)  /* StringToInt overflow  - rdw */
#define TB_LONG_MAX       (2147483647)   /* max long supported    - rdw */
#define TB_LONG_MIN      (-2147483647)   /* min long supported    - rdw */
#define TB_LONG_OVERFLOW (TB_LONG_MIN-1) /* StringToLong overflow - rdw */

#if WINVER != 0x030a
typedef GLOBALHANDLE     HGLOBAL;
typedef LOCALHANDLE      HLOCAL;
#endif

#ifndef _ttol
    #if defined(UNICODE) || defined(_UNICODE)
        #define     _ttol   _wtol
    #else
        #define     _ttol   atol
    #endif
#endif

// For handling UNICODE characters and MulitiByte characters. Charles
#if !defined(_TIsDBCSLeadByte)
  #if defined(UNICODE) || defined(_UNICODE)
    #define _TIsDBCSLeadByte(c) FALSE
  #else
    #define _TIsDBCSLeadByte(c) IsDBCSLeadByte(c)
  #endif
#endif

#if !defined(STRING_SIZE)
  #if defined(UNICODE) || defined(_UNICODE)
    #define STRING_SIZE(x) (sizeof(x)/sizeof(TCHAR))
  #else
    #define STRING_SIZE(x) sizeof(x)
  #endif
#endif


#ifndef MAKEWORD
#define MAKEWORD(a, b)  ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#endif

#define SEGMENT(lp)     (*((unsigned *)&(lp) + 1))
#define OFFSET(lp)      (*((unsigned *)&(lp)))

typedef struct tagWINDEVICE
   {
   char DeviceName[50];
   char DriverName[9];
   char Port[40];
   } WINDEVICE, FAR *LPWINDEVICE;

/* Keep backsward compatibility with older code */
#define TB_DATE       SS_DATE
#define LPTB_DATE     LPSS_DATE
#define TIME          SS_TIME
#define LPTIME        LPSS_TIME
#define DATEFORMAT    SS_DATEFORMAT
#define LPDATEFORMAT  LPSS_DATEFORMAT
#define TIMEFORMAT    SS_TIMEFORMAT
#define LPTIMEFORMAT  LPSS_TIMEFORMAT
#define FLOATFORMAT   SS_FLOATFORMAT
#define LPFLOATFORMAT LPSS_FLOATFORMAT

#define IDF_NYYMMDD    106
#define IDF_NNYYMMDD   107
#define IDF_NNNNYYMMDD 108

/*********************************************************
* User callable functions to support ToolBox edit fields
*********************************************************/
 
LPTB_DATE  DLLENTRY DateAddDays       (LPTB_DATE, LONG);
LONG    DLLENTRY DateDMYToJulian   (LPTB_DATE);
LPTSTR  DLLENTRY DateDMYToString   (LPTB_DATE, LPTSTR, LPDATEFORMAT);
BOOL    DLLENTRY DateGetFormat     (HWND, LPDATEFORMAT);
LPTSTR  DLLENTRY DateGetWeekday    (LPTB_DATE, LPTSTR);
BOOL    DLLENTRY DateIntIsValid    (LPTB_DATE);
LPTB_DATE  DLLENTRY DateJulianToDMY   (LONG, LPTB_DATE);
BOOL    DLLENTRY DateSetFormat     (HWND, LPDATEFORMAT);
BOOL    DLLENTRY DateSetRange      (HWND, LPTB_DATE, LPTB_DATE);
BOOL    DLLENTRY DateStringIsValid (LPCTSTR, LPDATEFORMAT);
BOOL    DLLENTRY DateStringIsValidEx(LPCTSTR lpszDateString, LPDATEFORMAT datefmt,
                                     short nTwoDigitYearMax);
LPTB_DATE DLLENTRY DateStringToDMY  (LPCTSTR, LPTB_DATE, LPDATEFORMAT);
LPTB_DATE DLLENTRY DateStringToDMYEx(LPCTSTR lpszDateString, LPTB_DATE da,
                                     LPDATEFORMAT datefmt, short nTwoDigitYearMax);
LONG    DLLENTRY DateSubtractDates (LPTB_DATE, LPTB_DATE);
void    DLLENTRY DateGetCurrentDate(LPTSTR lpszDate, LPDATEFORMAT lpdf);
BOOL    DLLENTRY DateSetDMY        (HWND hWnd, LPTB_DATE lpDate);
BOOL    DLLENTRY DateGetDMY        (HWND hWnd, LPTB_DATE lpDate);
#ifdef SS_V80
void DateSetSpinStyle (HWND hWnd, BOOL fEnhanced);
#endif
#define DateGetDlgItemFormat(h,i,fmt)    DateGetFormat(GetDlgItem((h),(i)),(fmt))
#define DateSetDlgItemFormat(h,i,fmt)    DateSetFormat(GetDlgItem((h),(i)),(fmt))
#define DateSetDlgItemRange(h,i,min,max) DateSetRange(GetDlgItem((h),(i)),(min),(max))
#define DateGetDlgItemDMY(h,i,d)         DateGetDMY(GetDlgItem((h), (i)), (d))
#define DateSetDlgItemDMY(h,i,d)         DateSetDMY(GetDlgItem((h), (i)), (d))

extern void DLLENTRY DateSetJulianStart(short dYear);
extern short DLLENTRY DateGetJulianStart(void);

BOOL    DLLENTRY PopupCalendar     (HWND hTBDateWnd, BOOL fEnhanced);
LONG    DLLENTRY Calendar          (HWND hWndParent, LPTSTR lpszDateText,
                                    LPDATEFORMAT lpDateFormat, LPTB_DATE lpDateMin,
                                    LPTB_DATE lpDateMax);
LONG    DLLENTRY CalendarEx        (HWND hWndParent, LPTSTR lpszDateText,
                                    LPDATEFORMAT lpDateFormat, LPTB_DATE lpDateMin,
                                    LPTB_DATE lpDateMax, short nTwoDigitYearMax);
void    DLLENTRY CalSetNames       (LPCTSTR lpszDays, LPCTSTR lpszMonths);
void    DLLENTRY DateWinFormat     (LPTSTR, LPTB_DATE, LPCTSTR, LPVOID);
void    DLLENTRY DateSetNames      (LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR);

BOOL    DLLENTRY TimeGetFormat     (HWND, LPTIMEFORMAT);
LONG    DLLENTRY TimeHMSToSeconds  (LPTIME);
LPTSTR  DLLENTRY TimeHMSToString   (LPTIME, LPTSTR, LPTIMEFORMAT);
LPTIME  DLLENTRY TimeSecondsToHMS  (LONG, LPTIME);
BOOL    DLLENTRY TimeSetFormat     (HWND, LPTIMEFORMAT);
BOOL    DLLENTRY TimeSetRange      (HWND, LPTIME, LPTIME);
BOOL    DLLENTRY TimeStringIsValid (LPCTSTR, LPTIMEFORMAT);
LPTIME  DLLENTRY TimeStringToHMS   (LPCTSTR, LPTIME, LPTIMEFORMAT);
void    DLLENTRY TimeGetCurrentTime(LPTSTR lpszTime, LPTIMEFORMAT lptf);
#ifdef SS_V80
void TimeSetSpinStyle (HWND hWnd, BOOL fEnhanced);
#endif
#define TimeGetDlgItemFormat(h,i,fmt)    TimeGetFormat(GetDlgItem((h),(i)),(fmt))
#define TimeSetDlgItemFormat(h,i,fmt)    TimeSetFormat(GetDlgItem((h),(i)),(fmt))
#define TimeSetDlgItemRange(h,i,min,max) TimeSetRange(GetDlgItem((h),(i)),(min),(max))

BOOL    DLLENTRY IntSetRange       (HWND hWnd, LONG lMin, LONG lMax);
BOOL    DLLENTRY IntGetRange       (HWND hWnd, LPLONG lpMin, LPLONG lpMax);
BOOL    DLLENTRY IntGetValue       (HWND hWnd, LPLONG lpLong);
BOOL    DLLENTRY IntSetValue       (HWND hWnd, long lLong);
BOOL    DLLENTRY IntSetMask        (HWND hWnd, LPCTSTR szMask);
BOOL    DLLENTRY IntGetMask        (HWND hWnd, LPTSTR szMask);
BOOL    DLLENTRY IntSetSpin        (HWND hWnd, BOOL fSpin, BOOL fSpinWrap,
                    long lSpinInc);
BOOL    DLLENTRY IntGetSpin        (HWND hWnd, LPBOOL lpfSpinWrap,
                    LPLONG lplSpinInc);
#ifdef SS_V80
void IntSetSpinStyle (HWND hWnd, BOOL fEnhanced);
#endif;

#define IntSetDlgItemRange(h,i,min,max) IntSetRange(GetDlgItem((h),(i)),(min),(max))
#define IntGetDlgItemRange(h,i,min,max) IntGetRange(GetDlgItem((h),(i)),(min),(max))
#define IntSetDlgItemValue(h,i,Val)     IntSetValue(GetDlgItem((h),(i)),(Val))
#define IntGetDlgItemValue(h,i,Val)     IntGetValue(GetDlgItem((h),(i)),(Val))

BOOL    DLLENTRY FloatGetValue     (HWND hWnd, LPDOUBLE lpDouble);
BOOL    DLLENTRY FloatSetValue     (HWND hWnd, double dfDouble);
BOOL    DLLENTRY FloatSetRange     (HWND hWnd, double dMin, double dMax);
BOOL    DLLENTRY FloatGetRange     (HWND hWnd, LPDOUBLE lpMin,
                    LPDOUBLE lpMax);
BOOL    DLLENTRY FloatSetFormat    (HWND hWnd, LPFLOATFORMAT ff);
BOOL    DLLENTRY FloatGetFormat    (HWND hWnd, LPFLOATFORMAT ff);
void    DLLENTRY FloatSetStyle     (HWND hWnd, long Style);
LPTSTR  DLLENTRY FloatFormatString (double Val, LPTSTR szBuffer,
                    BOOL fSeparator, BOOL fMoney,
                    short iDecimalPlaces);
LPTSTR  DLLENTRY FloatFormatStringExt(double Val, LPTSTR szBuffer,
                      LPFLOATFORMAT lpFormat,
                      short iDecimalPlaces);
BOOL    DLLENTRY FloatSetMask      (HWND hWnd, LPCTSTR szMask);
BOOL    DLLENTRY FloatGetMask      (HWND hWnd, LPTSTR szMask);
#define FloatSetDlgItemRange(h,i,min,max) FloatSetRange(GetDlgItem((h),(i)),(min),(max))
#define FloatGetDlgItemRange(h,i,min,max) FloatGetRange(GetDlgItem((h),(i)),(min),(max))
#define FloatGetDlgItemValue(h,i,val)    FloatGetValue(GetDlgItem((h),(i)),(val))
#define FloatSetDlgItemValue(h,i,val)    FloatSetValue(GetDlgItem((h),(i)),(val))

BOOL    DLLENTRY GetToolBoxGlobal  (int nID, LPLONG lplValue);
BOOL    DLLENTRY SetToolBoxGlobal  (int nID, LONG lValue);

#define DlgCoordToPixelX(x) (short)((x * LOWORD(GetDialogBaseUnits())) / 4)
#define DlgCoordToPixelY(y) (short)((y * HIWORD(GetDialogBaseUnits())) / 8)

/***************************************
* ToolBox Global Parameter Identifiers
***************************************/

#define TBG_CURRENTDATETIME   1
#define TBG_POPUPCALENDAR     2
#define TBG_CLEARFIELD        3

#define  ERR_AUTO    100

#ifndef  DRV_NOPATTERN
typedef  HGLOBAL     HPATTERN;
#define  PATTERNMAX  999
#define  PATTERNBRUSHMAX   4
typedef  enum tagPATTERNTYPE {SCREEN, PRINTER, PLOTTER} PATTERNTYPE;

VOID     DLLENTRY PatternPie        (HPATTERN hPat, HWND hWnd, HDC hDC, PATTERNTYPE Type, int nPattern, LPRECT Rect, int x1, int y1, int x2, int y2);
VOID     DLLENTRY PatternPoly       (HPATTERN hPat, HWND hWnd, HDC hDC, PATTERNTYPE Type, int nPattern, LPPOINT Poly, int Size);
VOID     DLLENTRY PatternRect       (HPATTERN hPat, HWND hWnd, HDC hDC, PATTERNTYPE Type, int nPattern, LPRECT Rect);
VOID     DLLENTRY PatternRgn        (HPATTERN hPat, HWND hWnd, HDC hDC, PATTERNTYPE Type, int nPattern, HRGN hRgn);
int      DLLENTRY PatternCreateBrush(HPATTERN hPat, HDC hDC, PATTERNTYPE Type, int nPattern, HBRUSH FAR * lphBrush);

VOID     DLLENTRY _PatternPie       (HWND hWnd, HDC hDC, PATTERNTYPE Type, LPVOID pPattern, LPRECT Rect, int x1, int y1, int x2, int y2);
VOID     DLLENTRY _PatternPoly      (HWND hWnd, HDC hDC, PATTERNTYPE Type, LPVOID pPattern, LPPOINT Poly, int Size);
VOID     DLLENTRY _PatternRect      (HWND hWnd, HDC hDC, PATTERNTYPE Type, LPVOID pPattern, LPRECT Rect);
VOID     DLLENTRY _PatternRgn       (HWND hWnd, HDC hDC, PATTERNTYPE Type, LPVOID pPattern, HRGN hRgn);
int      DLLENTRY _PatternCreateBrush(HDC hDC, PATTERNTYPE Type, LPVOID pPattern, HBRUSH FAR * lphBrush);

WORD     DLLENTRY PatternGetItemCount(HPATTERN hPat);
DWORD    DLLENTRY PatternGetScreenColor(int iIndex);
DWORD    DLLENTRY PatternGetPlotColor(int iIndex);
int      DLLENTRY PatternGetPlotPattern(int iIndex);
WORD     DLLENTRY PatternGetStructSize(PATTERNTYPE iType);

HPATTERN DLLENTRY PatternEdit       (LPCTSTR szFileName, int iColorNum);
HPATTERN DLLENTRY PatternLoad       (LPCTSTR szFileName);
BOOL     DLLENTRY PatternSave       (HPATTERN hPat);
LPVOID   DLLENTRY PatternLock       (HPATTERN hPat, PATTERNTYPE iType, int iIndex);
BOOL     DLLENTRY PatternUnlock     (HPATTERN hPat, PATTERNTYPE Type);
BOOL     DLLENTRY PatternFree       (HPATTERN hPat);
#endif   /* DRV_NOPATTERN */

#ifndef DRV_NOSYS /* no system functions */
#define FI_NORMAL       0x0000   /* Normal file - No read/write restrictions */
#define FI_RDONLY       0x0001   /* Read only file */
#define FI_HIDDEN       0x0002   /* Hidden file */
#define FI_SYSTEM       0x0004   /* System file */
#define FI_VOLID        0x0008   /* Volume ID file */
#define FI_SUBDIR       0x0010   /* Subdirectory */
#define FI_ARCH         0x0020   /* Archive file */


/* The following parameter is used by SysFindFirst() and EnumFiles() */
/* to match ONLY the file mode specified:                            */

#define FI_EXCLUSIVE    0x8000   /* only those selected by above attributes */

/* The following structure is used only by SysFindFirst() and SysFindNext() */

typedef struct tagFILEINFO
   {
   char  reserved[ 21];          /* Reserved for subsequent SysFindNext */
   char  attrib;                 /* Attribute of this file              */
   int   time;                   /* Time of last modification           */
   int   date;                   /* Date of last modification           */
   LONG  fileSize;               /* Size of File                        */
   char  fileName[ 13];          /* File name                           */
   } FILEINFO, FAR *LPFILEINFO, NEAR *NPFILEINFO, *PFILEINFO;

/*********************************************************
* The following structure is used only by SysDiskInfo():
*********************************************************/

typedef struct tagDISKINFO
   {
   WORD  bytesPerSector;         /* Bytes in a Sector (normally 512) */
   WORD  clustersPerDrive;       /* Clusters per Device              */
   WORD  sectorsPerCluster;      /* Sectors in a Cluster             */
   WORD  availableClusters;      /* Number of free Clusters          */
   } DISKINFO, FAR *LPDISKINFO, NEAR *NPDISKINFO, *PDISKINFO;

typedef struct tagPARAMBLK
   {
   WORD  wSegmentEnvBlock;       /* segment pointer to environment block */
   WORD  wOffSetCmdLine;         /* offset of command line               */
   WORD  wSegmentCmdLine;        /* segment of command line              */
   WORD  wOffSetFCB1;            /* etc.                                 */
   WORD  wSegmentFCB1;
   WORD  wOffSetFCB2;
   WORD  wSegmentFCB2;
   } PARAMBLK, FAR *LPPARAMBLK;

/**********************************************
* System and File-Related Function Prototypes
**********************************************/

int      DLLENTRY GetFileString     (LPSTR, LPSTR, LPSTR, int, LPSTR);
int      DLLENTRY SysCreateSubDir   (LPSTR szNewSubDir);
int      DLLENTRY SysDeleteFile     (LPSTR szExistingFile);
int      DLLENTRY SysDeleteSubDir   (LPSTR szExistingDir);
DWORD    DLLENTRY SysDiskBytesFree  (int iDrive);
DWORD    DLLENTRY SysDiskBytesTotal (int iDrive);
BOOL     DLLENTRY SysDiskInfo       (int iDrive, LPDISKINFO di);
BOOL     DLLENTRY SysFileSpec       (LPSTR , LPSTR);
int      DLLENTRY SysFindFirst      (int iAttrib, LPSTR szPath, LPFILEINFO fi);
int      DLLENTRY SysFindNext       (LPFILEINFO);
BOOL     DLLENTRY SysFixedDrive     (int iDrive);
int      DLLENTRY SysGetCurrentDir  (int iDrive, LPSTR szCurrentDir);
LPVOID   DLLENTRY SysGetDTA         (VOID);
VOID     DLLENTRY SysGetDate        (LPINT lpDay, LPINT lpMonth, LPINT lpYear);
int      DLLENTRY SysGetDefDrive    (VOID);
int      DLLENTRY SysGetLogicalDriveCount(VOID);
VOID     DLLENTRY SysGetTime        (LPINT lpHour, LPINT lpMinute, LPINT lpSeconds);
BOOL     DLLENTRY SysGetVolumeName  (int iDrive, LPSTR szVolumeName);
LPSTR    DLLENTRY SysMakePath       (LPSTR, LPSTR, LPSTR, LPSTR, LPSTR);
BOOL     DLLENTRY SysNetworkDrive   (int iDrive);
int      DLLENTRY SysRead           (int iHandle, LPVOID buff, unsigned size, LPWORD read);
int      DLLENTRY SysRenameFile     (LPSTR oldFile, LPSTR newFile);
int      DLLENTRY SysSetCurrentDir  (char FAR *szNewDir);
VOID     DLLENTRY SysSetDTA         (LPVOID);
BOOL     DLLENTRY SysSetDate        (int bDay, int bMonth, int bYear);
int      DLLENTRY SysSetDefDrive    (int iDrive);
BOOL     DLLENTRY SysSetTime        (int bHour, int bMinute, int bSeconds);
BOOL     DLLENTRY SysSetVolumeName  (int iDrive, LPSTR szNewVolumeName);
LPSTR    DLLENTRY SysSplitPath      (LPSTR, LPSTR, LPSTR, LPSTR, LPSTR);
BOOL     DLLENTRY SysValidDrive     (int iDrive);
int      DLLENTRY SysWrite          (int iHandle, LPVOID buff, unsigned size, LPWORD written);
int      DLLENTRY SysXCreateFile    (int attrib, LPSTR filename, LPWORD errorcode);
int      DLLENTRY SysSetFileAttr    (LPSTR path, WORD attrib);
int      DLLENTRY SysGetFileAttr    (LPSTR path, LPWORD attrib);

#ifndef O_RDONLY
#define O_RDONLY       0x0000
#define O_WRONLY       0x0001
#define O_RDWR         0x0002
#define O_APPEND       0x0008
#define O_CREAT        0x0100
#define O_TRUNC        0x0200
#define O_EXCL         0x0400
#define O_TEXT         0x4000
#define O_BINARY       0x8000
#endif

#ifndef S_IREAD
#define S_IREAD        0000400
#define S_IWRITE       0000200
#endif

BOOL DLLENTRY WriteFileString(LPSTR, LPSTR, LPSTR);

typedef enum {CPU86 = 0, CPU186 = 1, CPU286 = 2, CPU386 = 3} CPUID;

CPUID DLLENTRY GetCPUType(void);
#endif   /* DRV_NOSYS */

/* auto messagebox functions */
#define DRVMSG_BEEP  0x0001
#define DRVMSG_LOG   0x0010

#define StrPrintf STRPRINTF
#define StrScanf  STRSCANF
#define Debug     DEBUG
#define MsgDisp   MSGDISP

BOOL     DLLENTRY InitToolBox        (HANDLE);
BOOL     DLLENTRY ToolBoxLoadDlls    (short dDlls);
void     DLLENTRY ToolBoxFreeDlls    (short dDlls);
void     DLLENTRY ToolBoxFreeAllDlls (void);
WORD     DLLENTRY GetToolBoxVersion  (void);

BOOL     DLLENTRY InitTbPro1         (HANDLE hInst);
BOOL     DLLENTRY InitTbPro2         (HANDLE hInst);
BOOL     DLLENTRY InitTbPro3         (HANDLE hInst);
BOOL     DLLENTRY InitTbPro4         (HANDLE hInst);
BOOL     DLLENTRY InitTbPro5         (HANDLE hInst);
BOOL     DLLENTRY InitTbPro6         (HANDLE hInst);

/* HOUR GLASS FUNCTIONS */
WORD     DLLENTRY HourGlassCount    (VOID);
VOID     DLLENTRY HourGlassOff      (VOID);
VOID     DLLENTRY HourGlassOn       (VOID);

/* STRING FUNCTIONS */
LPTSTR   DLLENTRY StrCat            (LPTSTR, LPCTSTR);
LPTSTR   DLLENTRY StrChk            (LPTSTR);
LPTSTR   DLLENTRY StrChr            (LPCTSTR, TCHAR);
LPTSTR   DLLENTRY StriChr           (LPCTSTR, TCHAR);
int      DLLENTRY StrCmp            (LPCTSTR, LPCTSTR);
LPTSTR   DLLENTRY StrCpy            (LPTSTR, LPCTSTR);
LPTSTR   DLLENTRY StrDeleteChar     (LPTSTR lpszString, int iStart);
LPTSTR   DLLENTRY StrEnd            (LPTSTR);
LPTSTR   DLLENTRY StrFormat         (LPTSTR);
LPTSTR   DLLENTRY StrInsertChar     (TCHAR cChar, LPTSTR lpszString, int iInsertPos);
LPTSTR   DLLENTRY StrIntCat         (LPTSTR, int);
LPTSTR   DLLENTRY StrIntnCat        (LPTSTR, int, int);
LPTSTR   DLLENTRY StrIntnCpy        (LPTSTR, int, int);
LPTSTR   DLLENTRY StrLTrim          (LPTSTR);
LPTSTR   DLLENTRY StrLast           (LPTSTR);
int      DLLENTRY StrLen            (LPCTSTR);
LPTSTR   DLLENTRY StrLongCat        (LPTSTR, LONG);
LPTSTR   DLLENTRY StrLongnCat       (LPTSTR, LONG, int);
LPTSTR   DLLENTRY StrLongnCpy       (LPTSTR, LONG, int);
LPTSTR   DLLENTRY StrLwr            (LPTSTR);
LPTSTR   DLLENTRY StrPad            (LPTSTR, int);
int      FAR cdecl StrPrintf        (LPTSTR lpBuffer, LPCTSTR lpFormat,...);
LPTSTR   DLLENTRY StrRTrim          (LPTSTR);
LPTSTR   DLLENTRY StrRemove         (LPTSTR lpszString, int iFrom, int iTo);
LPTSTR   DLLENTRY StrReplace        (LPTSTR, LPTSTR);
LPTSTR   DLLENTRY StrRev            (LPTSTR);
int      FAR cdecl StrScanf          (LPTSTR lpStr, LPCTSTR lpFormat, ...);
LPTSTR   DLLENTRY StrStr            (LPCTSTR, LPCTSTR);
LPTSTR   DLLENTRY StrTok            (LPTSTR, LPCTSTR);
LPTSTR   DLLENTRY StrTrim           (LPTSTR);
LPTSTR   DLLENTRY StrUpr            (LPTSTR);
int      DLLENTRY StriCmp           (LPCTSTR, LPCTSTR);
LPTSTR   DLLENTRY StriStr           (LPCTSTR, LPCTSTR);
LPTSTR   DLLENTRY Strip             (LPTSTR, LPCTSTR);
LPTSTR   DLLENTRY StrnCat           (LPTSTR, LPCTSTR, int);
int      DLLENTRY StrnCmp           (LPCTSTR, LPCTSTR, int);
int      DLLENTRY StrnCmpi          (LPCTSTR, LPCTSTR, int);
LPTSTR   DLLENTRY StrnCpy           (LPTSTR, LPCTSTR, int);
int      DLLENTRY StrniCmp          (LPCTSTR, LPCTSTR, int);
LPTSTR   DLLENTRY StrnnCat          (LPTSTR, LPCTSTR, int);
LPTSTR   DLLENTRY StrnnCpy          (LPTSTR, LPCTSTR, int);
LPTSTR   DLLENTRY StrpBrk           (LPCTSTR, LPCTSTR);
LPTSTR   DLLENTRY StrpCpy           (LPTSTR, LPTSTR, int);
LPTSTR   DLLENTRY StrrChr           (LPCTSTR, TCHAR);
LPTSTR   DLLENTRY StrTrimAll        (LPTSTR);
#define  StrEqu(s1,s2)              (!StrCmp (s1, s2))
#define  StrSize(s)                 (StrLen (s)+1)
#define  StriEqu(s1,s2)             (!StriCmp (s1, s2))
#define  StrnEqu(s1,s2,n)           (!StrnCmp (s1, s2, n))
#define  StrniEqu(s1,s2,n)          (!StrniCmp (s1, s2, n))
#define  StrIntCpy(s,i)             IntToString (i, s)
#define  StrLongCpy(s,l)            LongToString (l, s)
                    
/* CONVERSION FUNCTIONS */
LPTSTR   DLLENTRY FloatToEString (LPTSTR szBuff, double value, int ndigit, LPINT decpt, LPINT sign);
LPTSTR   DLLENTRY FloatToFString (LPTSTR szBuff, double value, int ndec, LPINT decpt, LPINT sign);
LPTSTR   DLLENTRY IntToString       (int, LPTSTR);
LPTSTR   DLLENTRY LongToString      (LONG, LPTSTR);
#if defined(_WIN64) || defined(IA64)
LPTSTR   DLLENTRY LongPtrToString      (LONG_PTR, LPTSTR);
#endif
void     DLLENTRY StringToFloat     (LPCTSTR, LPDOUBLE);
int      DLLENTRY StringToInt       (LPCTSTR);
LONG     DLLENTRY StringToLong      (LPCTSTR);
                    
/* These macros added to assure backwards compatibility */

#define FloatToEStr(a, b, c, d, e)  FloatToEString(a, b, c, d, e)
#define FloatToFStr(a, b, c, d, e)  FloatToFString(a, b, c, d, e)
#define _SysGetDefDrive()           (SysGetDefDrive() + 1)
#define _SysSetDefDrive(drive)      (SysSetDefDrive(drive-1))
                    
/* FILE FUNCTIONS */                        
int      DLLENTRY ErrorNo           (void);
BOOL     DLLENTRY FileClose         (int File);
BOOL     DLLENTRY FileExists        (LPSTR);
BOOL     DLLENTRY FileGetc          (LPSTR, int);
BOOL     DLLENTRY FileGetf          (LPFLOAT, int);
BOOL     DLLENTRY FileGetl          (LPLONG, int);
BOOL     DLLENTRY FileGets          (LPSTR, WORD, int);
BOOL     DLLENTRY FileGetw          (LPINT, int);
BOOL     DLLENTRY FileOpen          (LPSTR Name, int OFlag, int PMode, LPINT File);
BOOL     DLLENTRY FilePutc          (char, int);
BOOL     DLLENTRY FilePutf          (double, int);
BOOL     DLLENTRY FilePutl          (LONG,int);
BOOL     DLLENTRY FilePuts          (LPSTR,int);
BOOL     DLLENTRY FilePutw          (int,int);
WORD     DLLENTRY FileRead          (LPVOID lpbuff, WORD wItemSize, WORD wItemCount, int iFH);
BOOL     DLLENTRY FileRemove        (LPSTR);
BOOL     DLLENTRY FileSeek          (int, LONG, WORD);
WORD     DLLENTRY FileWrite         (LPVOID lpbuff, WORD wItemSize, WORD wItemCount, int iFH);
BOOL     DLLENTRY FileCopy(LPSTR buff, int size, LPSTR src, LPSTR dst);
BOOL     DLLENTRY FileMultipleCopy(LPSTR src, LPSTR dst);
HANDLE   DLLENTRY FileFindFirst(LPSTR szBuff, LPINT lpAttrib, LPSTR szSpec, int iAttrib);
HANDLE   DLLENTRY FileFindNext(LPSTR szBuff, LPINT lpAttrib, HANDLE  hFInfo);
BOOL     DLLENTRY FileSetDateTime(int fh, int time, int date);
BOOL     DLLENTRY FileGetDateTime(int fh, LPINT time, LPINT date);
void     DLLENTRY FileDateConvert(int nDate, LPTB_DATE lpDate);
void     DLLENTRY FileTimeConvert(int nTime, LPTIME lpTime);

/* LIST BOX FUNCTIONS */
BOOL     DLLENTRY ListAddStr        (HWND,LPSTR);
BOOL     DLLENTRY ListDeleteStr     (HWND,int);
BOOL     DLLENTRY ListFindDeleteStr (HWND,LPSTR);
int      DLLENTRY ListFindStr       (HWND,LPSTR,int);
int      DLLENTRY ListGetCount      (HWND);
BOOL     DLLENTRY ListGetText       (HWND,LPSTR,int,int);
BOOL     DLLENTRY ListGetTextByIndex(HWND,LPSTR,int,int,int);
BOOL     DLLENTRY ListHiLiteFirstItem(HWND);
BOOL     DLLENTRY ListReset         (HWND);
BOOL     DLLENTRY ListSelectStr     (HWND,LPSTR);

/* FAR POINTER MEMORY FUNCTIONS (+ ALLOCATION ) */
int      DLLENTRY MemCmp            (const LPVOID, const LPVOID, WORD);
LPVOID   DLLENTRY MemcCpy           (LPVOID, LPVOID, char, WORD);
LPVOID   DLLENTRY MemCpy            (LPVOID, const void FAR*, WORD);
LPVOID   DLLENTRY MemMove           (LPVOID, LPVOID, WORD);
LPVOID   DLLENTRY MemSet            (LPVOID, char, WORD);
LPVOID   DLLENTRY MemChr            (LPVOID, char, WORD);
int      DLLENTRY MemiCmp           (LPVOID, LPVOID, WORD);
#define  Zero(struct)               MemSet(&struct,'\0',sizeof(struct))
#define  pZero(ptr)                 MemSet(ptr,'\0',sizeof(*ptr))

/* huge POINTER MEMORY FUNCTIONS */
int      DLLENTRY MemHugeCmp        (HPVOID,HPVOID, WORD);
HPVOID   DLLENTRY MemHugeCpy        (HPVOID,HPCVOID, LONG);
HPVOID   DLLENTRY MemHugeSet        (HPVOID,char, WORD);
HPVOID   DLLENTRY MemHugecCpy       (HPVOID,HPVOID,char, WORD);
HPVOID   DLLENTRY MemHugeChr        (HPVOID, char, WORD);

/* MESSAGE DISPLAY */
//VOID     FAR cdecl Debug             (LPSTR,...);
int      FAR cdecl MsgDisp           (HANDLE,int /* string ID format */, ...);
int      DLLENTRY MsgDoDisp         (LPSTR,LPSTR, int, int);
LPSTR    DLLENTRY MsgLoad           (HANDLE hInst, int id, LPSTR buff, LPSTR Title, LPINT mflags, LPINT iflags);

BOOL     DLLENTRY DlgFileOpenOne    (HWND,LPSTR,LPSTR,int);
BOOL     DLLENTRY DlgFileOpenTwo    (HWND,LPSTR,LPSTR,int);
BOOL     DLLENTRY DlgFontSelect     (HWND,LPLOGFONT,LPINT);
BOOL     DLLENTRY DlgPrinterSetup   (HWND);
int      DLLENTRY EnumFiles         (LPSTR, int, int (FAR PASCAL *)(LPFILEINFO, LPSTR, LPVOID), LPVOID );
VOID     DLLENTRY ExitNow           (VOID);
int      DLLENTRY FullMkDir         (LPSTR);

BOOL     DLLENTRY GaugeGetPos       (HWND,LPINT);
BOOL     DLLENTRY GaugeGetRange     (HWND,LPINT,LPINT);
BOOL     DLLENTRY GaugeSetColors    (HWND,DWORD,DWORD,DWORD,DWORD);
BOOL     DLLENTRY GaugeSetPos       (HWND,int);
BOOL     DLLENTRY GaugeSetRange     (HWND,int,int);
LPSTR    DLLENTRY GaugeSetTitle     (HWND,LPSTR);

VOID     DLLENTRY GetChildSize      (HWND, LPRECT);
VOID     DLLENTRY Line              (HDC, short, short, short, short);
int      DLLENTRY MsgBox            (LPSTR,LPSTR,WORD);
VOID     DLLENTRY MsgLineMenuHelp   (HWND,WORD);
VOID     DLLENTRY MsgLinePaint      (HWND,HDC);
VOID     DLLENTRY MsgLineTextOut    (HWND,LPSTR);
VOID     DLLENTRY OLRect            (HDC, LPRECT);
VOID       DLLENTRY OLLine            (HDC, int, int, int, int, int);
HDC      DLLENTRY PrinterGetDC      (VOID);
HDC      DLLENTRY PrinterGetIC      (VOID);
BOOL     DLLENTRY PrinterGetDetails (LPWINDEVICE);
int      DLLENTRY RmTmp             (VOID);
void     DLLENTRY TBFocusRect       (HDC,LPRECT);
LPSTR    DLLENTRY TempNam           (LPSTR, LPSTR);
VOID     DLLENTRY Tile              (HWND,HWND);
int      DLLENTRY TmpFile           (VOID);
LPSTR    DLLENTRY TmpNam            (LPSTR);
int      DLLENTRY WinExecProg       (LPSTR szPath, LPSTR lpCmdLine, int nCmdShow);

#ifndef   DRV_NOSORT
VOID     DLLENTRY QuickSort         (LPVOID lpBase, WORD wTotal, WORD wItemWidth, int (FAR PASCAL *lpfnCompare)(LPVOID lpItem1, LPVOID lpItem2));
BOOL     DLLENTRY HeapSort          (LPVOID lpBase, WORD wTotal, WORD wItemWidth, int (FAR PASCAL *lpfnCompare)(LPVOID lpItem1, LPVOID lpItem2));
LPVOID   DLLENTRY BinarySearch      (LPVOID lpKey, LPVOID lpBase, WORD wTotal, WORD wItemWidth, int (FAR PASCAL *lpfnCompare)(LPVOID lpItem1, LPVOID lpItem2));
HPVOID   DLLENTRY BinarySearchHuge  (HPVOID hpKey, HPVOID hpBase, LONG wTotal, WORD wItemWidth, int (FAR PASCAL *lpfnCompare)(HPVOID lpItem1, HPVOID lpItem2));
LPVOID   DLLENTRY LinearFind        (LPVOID lpKey, LPVOID lpBase, WORD wTotal, WORD wItemWidth, int (FAR PASCAL *lpfnCompare)(LPVOID lpItem1, LPVOID lpItem2));
HPVOID   DLLENTRY LinearFindHuge    (HPVOID hpKey, HPVOID hpBase, LONG lTotal, WORD wItemWidth, int (FAR PASCAL *lpfnCompare)(HPVOID hpItem1, HPVOID hpItem2));

extern void DLLENTRY QuickSortHuge(HPBYTE lpBase, LONG lNum, WORD Width,
                                   long lUserData,
                                   int (FAR PASCAL *lpCompare)(HPVOID,
                                   HPVOID, long));

#endif   /* DRV_NOSORT */

#ifndef  DRV_NODIALOG
HWND     DLLENTRY DialogNew         (HWND hWndParent, LPSTR szDialogName, HANDLE hInst);
int      DLLENTRY DialogShow        (HWND hWndParent, HWND hDlg);
#endif   /* DRV_NODIALOG */

/* these two functions are undocumented Windows function */
int DLLENTRY KillSystemTimer(HWND hWnd, int nIDEvent);
int DLLENTRY SetSystemTimer(HWND hWnd, int nIDEvent, WORD wElapse, FARPROC lpTimerFunc);

#endif   /* RC_INVOKED */


/******************
* Custom Controls
******************/

#ifndef DRV_NOCLASS

/****************
* Edit controls
****************/

#define LBN_ESCAPE        30       /* the user pressed the escape key */
#define LBN_RETURN        31       /* the user pressed the return key */

#define LBN_LBUTTONDBLCLK 2         /* our own notification */
#define LBN_TOPITEM       1000
#define LBN_RBUTTONDBLCLK 1001      /* our own notification */
#define LBN_MBUTTONDBLCLK 1002      /* our own notification */

/* LB_DIR attributes */

#define LBDA_NORMAL       0x0000
#define LBDA_READONLY     0x0001
#define LBDA_HIDDEN       0x0002
#define LBDA_SYSTEM       0x0004
#define LBDA_SUBDIR       0x0010
#define LBDA_ARCHIVE      0x0020

/* Windows places the messages generated by DlgDirList in the application */
/* queue; otherwise sent directly to the dialog function                  */

#define LBDA_DIRFLAG      0x2000
#define LBDA_DRIVE        0x4000
#define LBDA_EXCLUSIVE    0x8000

#ifndef LB_MSGMAX
#define LB_MSGMAX           (WM_USER+33)
#endif

/*
#define LB_SETSELECTIONMODE (LB_MSGMAX+0)
#define LB_GETTEXTEXTENT    (LB_MSGMAX+1)
#define LB_GETDBLCLKHPOS    (LB_MSGMAX+2)
#define LB_GETDBLCLKITEM    (LB_MSGMAX+3)
*/

/****************
* Edit controls
****************/

typedef struct tagEDIT3DFORMAT
   {
   WORD     wStyle;               /* 3D Styles                            */
   short    dShadowSize;          /* The size of the shadow in pixels     */
   COLORREF ColorShadow;          /* The color of the dark portion        */
   COLORREF ColorHighlight;       /* The color the light portion          */
   COLORREF ColorInsideShadow;    /* The color of the inside dark portion */
   COLORREF ColorInsideHighlight; /* The color the inside light portion   */
   } EDIT3DFORMAT, FAR *LPEDIT3DFORMAT;

#define EDIT3D_OUT          0x01  /* Project the 3D out     */
#define EDIT3D_INSIDEBORDER 0x02  /* Add a 3D inside border */

#ifndef EM_MSGMAX
#define EM_MSGMAX         (WM_USER+30)
#endif

#define EM_VALIDATE       (EM_MSGMAX + 1)
#define EM_GETCARETPOS    (EM_MSGMAX + 2)
#define EM_SETCARETPOS    (EM_MSGMAX + 3)
#define EM_CARETPOS       (EM_MSGMAX + 4)
#define EM_SETRANGE       (EM_MSGMAX + 5)
#define EM_SETFORMAT      (EM_MSGMAX + 6)
#define EM_GETFORMAT      (EM_MSGMAX + 7)
#define EPM_GETMASK       (EM_MSGMAX + 8)
#define EPM_SETMASK       (EM_MSGMAX + 9)
#define EPM_SETCALLBACK   (EM_MSGMAX + 10)
#define EM_GET3D          (EM_MSGMAX + 11)
#define EM_SET3D          (EM_MSGMAX + 12)
#define EPM_GETVALUE      (EM_MSGMAX + 13)
#define EPM_SETVALUE      (EM_MSGMAX + 14)
#define EM_TOOLBOXUSERMAX (EPM_SETVALUE)
#define DM_SETTWODIGITYEARMAX (EM_MSGMAX + 50)
#define DM_GETTWODIGITYEARMAX (EM_MSGMAX + 51)
#define DM_SETENHANCEDCALENDAR (EM_MSGMAX + 52)
#define EM_SETSPINSTYLE			(EM_MSGMAX + 53)

#define  EN_INVALIDDATA    0x1000
#define  EN_FIELDNONEMPTY  0x1001
#define  EN_FIELDEMPTY     0x1002
#define  EN_INVALIDSETTEXT 0x1003
#define  EN_INVALIDCHAR   (0x2003)

FARPROC DLLENTRY GetTBEditClassProc(VOID);

extern BOOL DLLENTRY PicFormatText(HWND hWnd, LPCTSTR lpszDefText,
                   LPCTSTR lpszMask, LPTSTR lpszText,
                   LPTSTR lpszTextFmt, BOOL fTextUnformatted);

/****************
* Gauge control
****************/

#ifndef DRV_NOGAUGE
#define GS_TITLE       0x00000001L
#define GS_VALUE       0x00000010L
#endif  /* DRV_NOGAUGE */

/*****************
* General styles
*****************/
// Edit styles used by SSSetTypeEdit

#define ES_STATIC      0x0800L
#define ES_OVERSTRIKE  0x0040L
#define ES_3D          0x0400L
#ifndef ES_UPPERCASE
#define ES_UPPERCASE   0x0008L
#endif
#ifndef ES_LOWERCASE
#define ES_LOWERCASE   0x0010L
#endif

/**************
* Time fields
**************/

//???? moved TS_ styles to SSDLL.H ????

/**************
* Date fields
**************/

//???? moved DS_ styles to SSDLL.H ????

/****************
* Integer Style
****************/

//???? moved IS_ styles to SSDLL.H ????

/**************
* Float Style
**************/

//?? moved FS_ styles to SSDLL.H ????

/********************************
* Define WM_CTLCOLOR parameters
********************************/

#define CTLCOLOR_TBEDIT     (101)
#define CTLCOLOR_TBPROGRESS (102)
#define CTLCOLOR_TBVIEWPICT (106)
#define CTLCOLOR_TBCHECKBOX (107)
#define CTLCOLOR_TBRADIOBTN (108)

/**********
* DirFile
**********/

#define DFM_GETSIZE             (WM_USER + 30)
#define DFM_SETSIZE             (WM_USER + 31)

#define DFN_SETFOCUS            30
#define DFN_KILLFOCUS           31
#define DFN_SIZECHANGE          32

#define DFS_DIRRIGHT            0x0001L
#define DFS_NOUPDATEFILELIST    0x0002L
#define DFS_CLOSEALLCHILDREN    0x0004L
#define DFS_OPENROOTCHILDREN    0x0008L
#define DFS_FOLDEROPENDIR       0x0010L
#define DFS_FILELIST_NOSHOWICON 0x0020L
#define DFS_FILELIST_VERT       0x0040L
#define DFS_FILELIST_HORZ       0x0080L
#define DFS_FILELIST_SHOW_SIZE  0x0100L
#define DFS_FILELIST_SHOW_DATE  0x0200L
#define DFS_FILELIST_SHOW_TIME  0x0400L
#define DFS_FILELIST_SHOW_ATTR  0x0800L
#define DFS_MULTISEL            0x1000L

/**********
* DirList
**********/

#define DLM_SETDRIVE              (WM_USER + 0)
#define DLM_READDRIVE             (WM_USER + 1)
#define DLM_GETSELDIRENTRY        (WM_USER + 2)
#define DLM_GETSELDIR             (WM_USER + 3)
#define DLM_GETDIRCNT             (WM_USER + 4)
#define DLM_SETSELDIR             (WM_USER + 5)
#define DLM_SETCOLOR              (WM_USER + 6)
#define DLM_OPENCHILDREN          (WM_USER + 7)
#define DLM_GETCOLOR              (WM_USER + 8)
#define DLM_SETSORTMETHOD         (WM_USER + 9)
#define DLM_SETSORTORDER          (WM_USER + 40)
#define DLM_GETCURDIR             (WM_USER + 41)
#define DLM_SETCURDIR             (WM_USER + 42)
#define DLM_GETSEL                (WM_USER + 43)
#define DLM_GETDRIVEHANDLE        (WM_USER + 44)
#define DLM_GETCURDIRENTRY        (WM_USER + 45)

#define DLN_SETFOCUS              10
#define DLN_KILLFOCUS             11
#define DLN_SELCHANGE             12
#define DLN_DBLCLK                13
#define DLN_TAG                   14

#define DLS_CLOSEALLCHILDREN      0x0001L
#define DLS_OPENROOTCHILDREN      0x0002L
#define DLS_FOLDEROPENDIR         0x0004L
#define DLS_SORT_NONE             0x0008L
#define DLS_SORT_NAME             0x0010L
#define DLS_SORT_DATE             0x0020L
#define DLS_SORT_EXT              0x0040L
#define DLS_SORT_DESC             0x0080L
#define DLS_SORT_SIZE             0x0100L
#define DLS_MULTISEL              0x0200L

#define DIRLIST_SORTORDER_ASC     0
#define DIRLIST_SORTORDER_DESC    1

#define DIRLIST_CHILDREN_OPENALL  0
#define DIRLIST_CHILDREN_OPENROOT 1
#define DIRLIST_CHILDREN_CLOSEALL 2
#define DIRLIST_CHILDREN_OPENCUR  3
#define DIRLIST_CHILDREN_CLOSECUR 4
#define DIRLIST_CHILDREN_FLIPCUR  5

typedef struct dirlistcolor
   {
   COLORREF ColorBack;
   COLORREF ColorText;
   COLORREF ColorLines;
   COLORREF ColorIconShadow;
   COLORREF ColorIconBack;
   COLORREF ColorIconFore;
   } DIRLISTCOLOR, FAR *LPDIRLISTCOLOR;

/************
* DriveList
************/

#define DVLM_GETDRIVECOUNT        (WM_USER + 0)
#define DVLM_GETSELDRIVE          (WM_USER + 1)
#define DVLM_SETSELDRIVE          (WM_USER + 2)
#define DVLM_SETCOLOR             (WM_USER + 3)
#define DVLM_GETCOLOR             (WM_USER + 4)

#define DVLN_SETFOCUS             10   /* Used Internally */
#define DVLN_KILLFOCUS            11
#define DVLN_SELCHANGE            12   /* Used Internally */
#define DVLN_DBLCLK               13
#define DVLN_DRIVECHANGE          14

#define DVLS_DRIVELIST_FIXED      0x01
#define DVLS_DRIVELIST_REMOTE     0x02
#define DVLS_DRIVELIST_REMOVABLE  0x04
#define DVLS_DRIVELIST_ALL        0x07
#define DVLS_DRIVELIST_LISTBOX    0x08
#define DVLS_DRIVELIST_COMBOBOX   0x10
#define DVLS_DRIVELIST_NOSHOWICON 0x20

typedef struct drivelistcolor
   {
   COLORREF ColorBack;
   COLORREF ColorText;
   COLORREF ColorIconShadowBright;
   COLORREF ColorIconShadowDark;
   COLORREF ColorIconBack;
   COLORREF ColorIconFore;
   COLORREF ColorIconLight;
   } DRIVELISTCOLOR, FAR *LPDRIVELISTCOLOR;

/***********
* FileList
***********/

#define FLM_SETDIRECTORY        (WM_USER + 10)   /* Used Internally */
#define FLM_READDIRECTORY       (WM_USER + 11)
#define FLM_SETVIEWTYPE         (WM_USER + 12)
#define FLM_SETSORTMETHOD       (WM_USER + 13)
#define FLM_SETSORTORDER        (WM_USER + 14)
#define FLM_SETCOLOR            (WM_USER + 15)
#define FLM_GETFILECOUNT        (WM_USER + 16)
#define FLM_GETSELFILE          (WM_USER + 17)
#define FLM_SETSELFILE          (WM_USER + 18)
#define FLM_SETFILESPEC         (WM_USER + 19)
#define FLM_GETCOLOR            (WM_USER + 20)
#define FLM_GETSELCOUNT         (WM_USER + 21)
#define FLM_SELECTALL           (WM_USER + 22)
#define FLM_GETCURFILE          (WM_USER + 23)
#define FLM_SETCURFILE          (WM_USER + 24)
#define FLM_GETSEL              (WM_USER + 25)

#define FLN_SETFOCUS            20
#define FLN_KILLFOCUS           21
#define FLN_SELCHANGE           22
#define FLN_DBLCLK              23
#define FLN_TAG                 24

#define FLS_FILELIST_SORT_NONE  0x0001L
#define FLS_FILELIST_SORT_NAME  0x0002L
#define FLS_FILELIST_SORT_DATE  0x0004L
#define FLS_FILELIST_SORT_SIZE  0x0008L
#define FLS_FILELIST_SORT_EXT   0x0010L
#define FLS_FILELIST_SORT_DESC  0x0020L
#define FLS_FILELIST_NOSHOWICON 0x0040L
#define FLS_FILELIST_VERT       0x0080L
#define FLS_FILELIST_HORZ       0x0100L
#define FLS_FILELIST_SHOW_SIZE  0x0200L
#define FLS_FILELIST_SHOW_DATE  0x0400L
#define FLS_FILELIST_SHOW_TIME  0x0800L
#define FLS_FILELIST_SHOW_ATTR  0x1000L
#define FLS_FILELIST_MULTISEL   0x2000L

#define FILELIST_SORTORDER_ASC  0
#define FILELIST_SORTORDER_DESC 1

typedef struct filelistcolor
   {
   COLORREF ColorBack;
   COLORREF ColorText;
   COLORREF ColorIconShadow;
   COLORREF ColorIconExeBack;
   COLORREF ColorIconExeFore;
   COLORREF ColorIconExeUpper;
   COLORREF ColorIconDocBack;
   COLORREF ColorIconDocFore;
   COLORREF ColorIconDocUpper;
   } FILELISTCOLOR, FAR *LPFILELISTCOLOR;

/**********
* Imprint
**********/

#define IS_IMPRINT_OUT            0x0001L
#define IS_IMPRINT_IN             0x0002L
#define IS_FRAME_OUT              0x0004L
#define IS_FRAME_IN               0x0008L
#define IS_INSIDEBORDER           0x0010L
#define IS_OUTSIDEBORDER          0x0020L
#define IS_GROUPBOX               0x0040L
#define IS_CLEAR                  0x0080L
#define IS_TEXTBOTTOM             0x0100L
#define IS_TEXTCENTER             0x0200L
#define IS_TEXTLEFT               0x0400L
#define IS_TEXTRIGHT              0x0800L
#define IS_TEXTTOP                0x1000L
#define IS_TEXTVCENTER            0x2000L
#define IS_TEXTBORDER             0x4000L
#define IS_SHADOWSIZE1            0x8000L

#define IM_GETFRAMESIZE           (WM_USER + 0)
#define IM_GETLINECNT             (WM_USER + 1)
#define IM_GETLINESIZE            (WM_USER + 2)
#define IM_GETSHADOWSIZE          (WM_USER + 3)
#define IM_SETCOLOR               (WM_USER + 4)
#define IM_SETFRAMESIZE           (WM_USER + 5)
#define IM_SETLINECNT             (WM_USER + 6)
#define IM_SETLINESIZE            (WM_USER + 7)
#define IM_SETSHADOWSIZE          (WM_USER + 8)
#define IM_GETCOLOR               (WM_USER + 9)

typedef struct imprintcolor
   {
   COLORREF Color;
   COLORREF ColorFrame;
   COLORREF ColorShadow;
   COLORREF ColorHighlight;
   COLORREF ColorInsideBorder;
   COLORREF ColorOutsideBorder;
   COLORREF ColorText;
   COLORREF ColorLines;
   } IMPRINTCOLOR, FAR *LPIMPRINTCOLOR;

/***********
* Progress
***********/

#define PS_VERTICAL       0x01
#define PS_HORIZONTAL     0x02
#define PS_HIDEPERCENTAGE 0x04

#define PM_GETRANGE       (WM_USER + 0)
#define PM_GETPOS         (WM_USER + 1)
#define PM_SETRANGE       (WM_USER + 2)
#define PM_SETPOS         (WM_USER + 3)

/**********
* SpinBtn
**********/

#define SBNS_UPARROW      0x001L
#define SBNS_DOWNARROW    0x002L
#define SBNS_UPDOWNARROW  0x003L
#define SBNS_HORIZONTAL   0x004L
#define SBNS_VERTICAL     0x008L
#define SBNS_BORDERNONE   0x010L
#define SBNS_BORDERLEFT   0x020L
#define SBNS_BORDERRIGHT  0x040L
#define SBNS_BORDERTOP    0x080L
#define SBNS_BORDERBOTTOM 0x100L

/************
* Check Box
************/

#define BM_SETPICT               (WM_USER + 51)

#define BT_PICT_UP               0x0010
#define BT_PICT_DOWN             0x0020
#define BT_PICT_FOCUSUP          0x0040
#define BT_PICT_FOCUSDOWN        0x0080
#define BT_PICT_DISABLEDUP       0x0100
#define BT_PICT_DISABLEDDOWN     0x0200
#define BT_PICT_GRAY             0x0400
#define BT_PICT_FOCUSGRAY        0x0800
#define BT_PICT_DISABLEDGRAY     0x1000

extern BOOL DLLENTRY tbCheckBoxLoadDef(HANDLE hInst, WORD wButtonType,
                       LPARAM lPict);
extern BOOL DLLENTRY tbCheckBoxUnloadDefs(HANDLE hInst);

/***********
* RadioBtn
***********/

extern BOOL DLLENTRY tbRadioBtnLoadDef(HANDLE hInst, WORD wButtonType,
                       long lPict);
extern BOOL DLLENTRY tbRadioBtnUnloadDefs(HANDLE hInst);

/***********
* SuperBtn
***********/

//???? moved SBS_ styles to SSDLL.H

#define SBM_SETSTYLE             BM_SETSTYLE
#define SBM_SETCOLOR             (WM_USER + 50)
#define SBM_SETPICT              BM_SETPICT
#define SBM_SETPICT_DISABLED     (WM_USER + 52)
#define SBM_SETPICT_BTNDOWN      (WM_USER + 53)
#define SBM_GETCOLOR             (WM_USER + 54)
#define SBM_SETBUTTONTYPE        (WM_USER + 55)
#define SBM_GETBUTTONSTATE       (WM_USER + 56)
#define SBM_SETBUTTONSTATE       (WM_USER + 57)
#define SBM_SETSHADOWSIZE        (WM_USER + 58)
#define SBM_GETSHADOWSIZE        (WM_USER + 59)
#define SBM_SETSTRETCH           (WM_USER + 60)
#define SBM_GETSTRETCH           (WM_USER + 61)

#define SBN_CLICKED              BN_CLICKED

//???? moved SUPERBTN_ flags to SSDLL.H ????

//???? moved superbtn color struct to SSDLL.H ????

/******************
* ToolBox/ToolBar
******************/

#define TBXS_SINGLESEL    0x01L
#define TBXS_MULTISEL     0x02L
#define TBXS_NOSTATE      0x04L

#define TBX_NOGROUP       0

#define TBX_NOSEL         0
#define TBX_SELECTED      1
#define TBX_DISABLED      2

#define TBX_RESIZE        -1
#define TBX_MAXSIZE       -2

#define TBXM_SETCOLOR     (WM_USER + 50)
#define TBXM_GETCOLOR     (WM_USER + 51)
#define TBXM_GETITEMSTATE (WM_USER + 52)
#define TBXM_SETITEMSTATE (WM_USER + 53)
#define TBXM_GETITEMCNT   (WM_USER + 54)
#define TBXM_ADDITEM      (WM_USER + 55)
#define TBXM_INSERTITEM   (WM_USER + 56)
#define TBXM_DELETEITEM   (WM_USER + 57)
#define TBXM_SETGROUP     (WM_USER + 58)
#define TBXM_GETITEMRECT  (WM_USER + 59)
#define TBXM_CHANGEITEM   (WM_USER + 60)

#define TBXN_CLICKED      1

#define WM_TBXBUTTON      0x0600
#define WM_TBXDESTROY     0x0601

/***************************
* Button Message Structure
***************************/

typedef struct tbx_button
   {
   short dItemID;               /* ID of clicked on item                     */
   short xPos;                  /* Mouse x coordinate                        */
   short yPos;                  /* Mouse y coordinate                        */
   WORD  wKeys;                 /* Other keys that are down                  */
                /*    MK_CONTROL                             */
                /*    MK_LBUTTON                             */
                /*    MK_MBUTTON                             */
                /*    MK_RBUTTON                             */
                /*    MK_SHIFT                               */
   WORD  wButtonType;           /* Button Type. May be one of the following: */
                /*    TBX_BUTTONTYPE_LDOWN                   */
                /*    TBX_BUTTONTYPE_LUP                     */
                /*    TBX_BUTTONTYPE_LDBLCLK                 */
                /*    TBX_BUTTONTYPE_RDOWN                   */
                /*    TBX_BUTTONTYPE_RUP                     */
                /*    TBX_BUTTONTYPE_RDBLCLK                 */
   } TBX_BUTTON, FAR *LPTBX_BUTTON;

#define TBX_BUTTONTYPE_LDOWN   1
#define TBX_BUTTONTYPE_LUP     2
#define TBX_BUTTONTYPE_LDBLCLK 3
#define TBX_BUTTONTYPE_RDOWN   4
#define TBX_BUTTONTYPE_RUP     5
#define TBX_BUTTONTYPE_RDBLCLK 6

#define TBXS_TEXTTOP_PICTBOT      SBS_TEXTTOP_PICTBOT
#define TBXS_TEXTBOT_PICTTOP      SBS_TEXTBOT_PICTTOP
#define TBXS_TEXTLEFT_PICTRIGHT   SBS_TEXTLEFT_PICTRIGHT
#define TBXS_TEXTRIGHT_PICTLEFT   SBS_TEXTRIGHT_PICTLEFT
#define TBXS_BTNFRAMEIN           SBS_BTNFRAMEIN
#define TBXS_BTNFRAMEOUT          SBS_BTNFRAMEOUT
#define TBXS_PICTFRAMEIN          SBS_PICTFRAMEIN
#define TBXS_PICTFRAMEOUT         SBS_PICTFRAMEOUT
#define TBXS_TEXTFRAMEIN          SBS_TEXTFRAMEIN
#define TBXS_TEXTFRAMEOUT         SBS_TEXTFRAMEOUT
#define TBXS_NOINVERTFRAME        SBS_NOINVERTFRAME
#define TBXS_STRETCH              0x1000

typedef struct toolboxcolor
   {
   COLORREF Background;
   COLORREF Border;
   COLORREF ButtonBorder;
   COLORREF ButtonUpColor;
   COLORREF ButtonUpShadow;
   COLORREF ButtonUpHighlight;
   COLORREF ButtonUpText;
   COLORREF ButtonDownColor;
   COLORREF ButtonDownShadow;
   COLORREF ButtonDownHighlight;
   COLORREF ButtonDownText;
   } TOOLBOXCOLOR, FAR *LPTOOLBOXCOLOR;

typedef struct toolboxitem
   {
   short Id;
   LPSTR lpPictUpName;
   short dPictUpType;
   LPSTR lpPictDownName;
   short dPictDownType;
   LPSTR lpPictDisabledName;
   short dPictDisabledType;
   LPSTR lpText;
   short dColWidth;
   short dRowHeight;
   long  lStyle;
   } TOOLBOXITEM, FAR *LPTOOLBOXITEM;

HWND FAR PASCAL tbCreateToolBox(HANDLE hInstance, HWND hWndParent,
                DWORD lStyle, short x, short y,
                short dMinWidth, short dMinHeight,
                short dColCnt, short dRowCnt,
                short dBorderSize, LPSTR lpszTitle);

/************
* StatusBar
************/

#define SBRS_TEXTLEFT         0x0001
#define SBRS_TEXTCENTER       0x0002
#define SBRS_TEXTRIGHT        0x0004
#define SBRS_DRAWTOPLINE      0x0008
#define SBRS_PROGRESS         0x0010
#define SBRS_PIXELS           0x0020
#define SBRS_PERCENTSCREEN    0x0040
#define SBRS_PERCENT          0x0080
#define SBRS_TWIPS            0x0100
#define SBRS_DLGUNITS         0x0200
#define SBRS_OWNERDRAW        0x0400
#define SBRS_NO3D             0x0800
#define SBRS_3DOUT            0x1000

#define SBRM_SETCOLOR         (WM_USER + 50)
#define SBRM_GETCOLOR         (WM_USER + 51)
#define SBRM_SETITEMS         (WM_USER + 52)
#define SBRM_SETITEMTEXT      (WM_USER + 53)
#define SBRM_GETITEMTEXT      (WM_USER + 54)
#define SBRM_GETITEMRECT      (WM_USER + 55)
#define SBRM_SETPROGRESSPOS   (WM_USER + 56)
#define SBRM_SETPROGRESSRANGE (WM_USER + 57)

#define WM_SBRBUTTON          0x0570

#define SBR_AUTOSIZE          -1

#define ODT_TBSTATUSBAR       500

/*******************************
* Status Bar Message Structure
*******************************/

typedef struct sbr_button
   {
   short dItemID;               /* ID of clicked on item                     */
   short xPos;                  /* Mouse x coordinate                        */
   short yPos;                  /* Mouse y coordinate                        */
   WORD  wKeys;                 /* Other keys that are down                  */
                /*    MK_CONTROL                             */
                /*    MK_LBUTTON                             */
                /*    MK_MBUTTON                             */
                /*    MK_RBUTTON                             */
                /*    MK_SHIFT                               */
   WORD  wButtonType;           /* Button Type. May be one of the following: */
                /*    SBR_BUTTONTYPE_LDOWN                   */
                /*    SBR_BUTTONTYPE_LUP                     */
                /*    SBR_BUTTONTYPE_LDBLCLK                 */
                /*    SBR_BUTTONTYPE_RDOWN                   */
                /*    SBR_BUTTONTYPE_RUP                     */
                /*    SBR_BUTTONTYPE_RDBLCLK                 */
   } SBR_BUTTON, FAR *LPSBR_BUTTON;

#define SBR_BUTTONTYPE_LDOWN   1
#define SBR_BUTTONTYPE_LUP     2
#define SBR_BUTTONTYPE_LDBLCLK 3
#define SBR_BUTTONTYPE_RDOWN   4
#define SBR_BUTTONTYPE_RUP     5
#define SBR_BUTTONTYPE_RDBLCLK 6

typedef struct statusbarcolor
   {
   COLORREF Line;
   COLORREF Background;
   COLORREF Shadow;
   COLORREF Highlight;
   COLORREF Text;
   COLORREF Progress;
   } STATUSBARCOLOR, FAR *LPSTATUSBARCOLOR;

typedef struct statusbarprogress
   {
   short dMin;
   short dMax;
   BOOL  fShowPercentage;
   } STATUSBARPROGRESS, FAR *LPSTATUSBARPROGRESS;

typedef struct statusbaritem
   {
   WORD  ItemID;
   long  lOffset;
   long  lWidth;
   LPSTR lpText;
   long  lItemStyle;
   } STATUSBARITEM, FAR *LPSTATUSBARITEM;

HWND FAR PASCAL tbCreateStatusBar(HWND hWndParent, DWORD lStyle, short x,
                  short y, short dWidth, short dHeight,
                  LPSTATUSBARITEM lpItems, short dItemCnt);

/***********
* ViewText
***********/

#define VTS_OWNERSUPPLYTEXT       0x01L
#define VTS_NOHSCROLLBAR          0x02L
#define VTS_NOVSCROLLBAR          0x04L

#define VTM_ADDSTRING             (WM_USER + 0)
#define VTM_CLEAR                 (WM_USER + 1)
#define VTM_DELETESTRING          (WM_USER + 2)
#define VTM_GETLINECOUNT          (WM_USER + 3)
#define VTM_GETSTRING             (WM_USER + 4)
#define VTM_GETSTRINGLEN          (WM_USER + 5)
#define VTM_GETTOPINDEX           (WM_USER + 6)
#define VTM_HIGHLIGHTTEXT         (WM_USER + 7)
#define VTM_INSERTSTRING          (WM_USER + 8)
#define VTM_REDRAW                (WM_USER + 9)
#define VTM_SEARCH                (WM_USER + 10)
#define VTM_SEARCHREPEAT          (WM_USER + 11)
#define VTM_SETCOLHEADERS         (WM_USER + 12)
#define VTM_SETCOLOR              (WM_USER + 13)
#define VTM_GETCOLOR              (WM_USER + 14)
#define VTM_SETHEADER             (WM_USER + 15)
#define VTM_SETROWHEADERS         (WM_USER + 16)
#define VTM_SETTOPINDEX           (WM_USER + 17)

#define VTM_VIEWSIZE              0x0510
#define VTM_RETRIEVETEXT          0x0511

#define VTN_KILLFOCUS             1
#define VTN_SETFOCUS              2

#define VIEWTEXT_SEARCH_CASE      0x0001
#define VIEWTEXT_SEARCH_NOCASE    0x0002

#define VIEWTEXT_SEARCH_CURRENT   0x0004
#define VIEWTEXT_SEARCH_BEGIN     0x0008
#define VIEWTEXT_SEARCH_END       0x0010

#define VIEWTEXT_SEARCH_FORWARD   0x0020
#define VIEWTEXT_SEARCH_BACKWARD  0x0040

#define VIEWTEXT_SEARCH_SUBSTRING 0x0080
#define VIEWTEXT_SEARCH_WHOLEWORD 0x0100

typedef struct viewtextcolor
   {
   COLORREF BackGround;
   COLORREF MainText;
   COLORREF Header;
   COLORREF ColTitles;
   COLORREF RowTitles;
   COLORREF HighLightText;
   COLORREF HighLightBk;
   } VIEWTEXTCOLOR, FAR *LPVIEWTEXTCOLOR;

typedef struct viewsize
   {
   short WidthMax;
   short ItemCnt;
   } VIEWSIZE, FAR *LPVIEWSIZE;

typedef struct viewretrievetext
   {
   short ItemNum;
   LPSTR ItemText;
   } VIEWRETRIEVETEXT, FAR *LPVIEWRETRIEVETEXT;

/***********
* ViewPict
***********/
//#define VPS_CENTER            0x0001L
#define VPS_SCROLL            0x0008L
#define VPS_FREEZE            0x0100L
#define VPS_SHOWFRAMEONCE     0x0200L
#define VPS_STATIC            0x0400L
#define VPS_METAFILE          0x2000L

/********************
* ViewPict Messages
********************/

#define VPN_KILLFOCUS         1
#define VPN_SETFOCUS          2
#define VPN_BUTTONDOWN        3
#define VPN_DBLCLK            4

#define VPM_SETPICTURE        (WM_USER + 0)
#define VPM_CLEAR             (WM_USER + 1)
#define VPM_SETSTYLE          (WM_USER + 2)
#define VPM_GETPOINTERPOS     (WM_USER + 3)
#define VPM_SETPALETTE        (WM_USER + 9)
#define VPM_GETPALETTE        (WM_USER + 10)

/*************************
* Messages for animation
*************************/

#define VPM_SETFRAMES         (WM_USER + 4)
#define VPM_FREEZEANIMATION   (WM_USER + 5)
#define VPM_STARTANIMATION    (WM_USER + 6)
#define VPM_SHOWNEXTFRAME     (WM_USER + 7)
#define VPM_SHOWPREVFRAME     (WM_USER + 8)

/**************************
* ViewPict Error Messages
**************************/

#define VIEWPICT_ERR_OK       0
#define VIEWPICT_ERR_NOTFOUND 1
#define VIEWPICT_ERR_MEMORY   2
#define VIEWPICT_ERR_FILEERR  3
#define VIEWPICT_ERR_BADPICT  4

#define VIEWPICT_END          -1
#define VIEWPICT_DEFDELAY     -1

typedef struct viewpict_animatehdr
   {
   short PictureCnt;
   short TimeDelay;
   } VIEWPICT_ANIMATEHDR, FAR *LPVIEWPICT_ANIMATEHDR;

typedef struct viewpict_animatepict
   {
   LPTSTR PictName;
   short  PictStyle;
   short  x;
   short  y;
   short  TimeDelay;
   } VIEWPICT_ANIMATEPICT, FAR *LPVIEWPICT_ANIMATEPICT;

typedef struct viewpict_animate
   {
   VIEWPICT_ANIMATEHDR    AnimateHdr;
   LPVIEWPICT_ANIMATEPICT AnimatePict;
   } VIEWPICT_ANIMATE, FAR *LPVIEWPICT_ANIMATE;

#endif   /* DRV_NOCLASS */

/*****************
* Play functions
*****************/

extern HANDLE DLLENTRY Play(HWND hWndParent, LPSTR lpPlayString, BYTE Speed);
extern BOOL   DLLENTRY PlayChangeSpeed(HANDLE hPlayInst, BYTE Speed);
extern BOOL   DLLENTRY PlayStop(HANDLE hPlayInst);
extern BOOL   DLLENTRY PlayFreeze(HANDLE hPlayInst);
extern BOOL   DLLENTRY PlayResume(HANDLE hPlayInst);
extern BOOL   DLLENTRY WhiteNoise(WORD FreqLow, WORD FreqHigh, WORD Duration);
extern void FAR PASCAL ToneInit(void);
extern void FAR PASCAL ToneOn(WORD);
extern void FAR PASCAL ToneOff(void);
extern long FAR PASCAL TickCount(void);

extern void FAR PASCAL       TBDelay(DWORD ms);
extern BOOL DLLENTRY _export PrintWin(HWND, LPRECT, BOOL);
extern void DLLENTRY         DlgBoxCenter(HWND hWnd);

#ifndef RC_INVOKED
#pragma pack()
#endif  /* RC_INVOKED */

#if defined( __cplusplus )
}
#endif

#endif
