/*************
* WINTOOLS.H
*************/

#ifndef WINTOOLS_H
#define WINTOOLS_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef SS_V80

#if (defined(SS_W))

   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR32DU80_")
   #else
   #define SSW_PREFIX  "SPR32D80_"
   #endif

#elif (defined(FP_ADB))

   #ifdef SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPRJ32AU80_")
   #else
   #define SSW_PREFIX  "SPRJ32A80_"
   #endif
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR32AU80_")
   #else
   #define SSW_PREFIX  "SPR32A80_"
   #endif
   #endif // SPREAD_JPN

#elif (defined(SS_SPREADOCX))

   #ifdef SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPRJ32XU80_")
   #else
   #define SSW_PREFIX  "SPRJ32X80_"
   #endif
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR32XU80_")
   #else
   #define SSW_PREFIX  "SPR32X80_"
   #endif
   #endif // SPREAD_JPN

#endif // SS_SPREADOCX && SS_V70

#elif SS_V70

#if (defined(SS_W))

   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR32DU70_")
   #else
   #define SSW_PREFIX  "SPR32D70_"
   #endif

#elif (defined(FP_ADB))

   #ifdef SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPRJ32AU70_")
   #else
   #define SSW_PREFIX  "SPRJ32A70_"
   #endif
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR32AU70_")
   #else
   #define SSW_PREFIX  "SPR32A70_"
   #endif
   #endif // SPREAD_JPN

#elif (defined(SS_SPREADOCX))

   #ifdef SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPRJ32XU70_")
   #else
   #define SSW_PREFIX  "SPRJ32X70_"
   #endif
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR32XU70_")
   #else
   #define SSW_PREFIX  "SPR32X70_"
   #endif
   #endif // SPREAD_JPN

#endif // SS_SPREADOCX && SS_V70

#elif defined(SS_V40)

#if (defined(SS_W))

   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR32DU60_")
   #else
   #define SSW_PREFIX  "SPR32D60_"
   #endif

#elif (defined(FP_ADB))

   #ifdef SPREAD_JPN
   #define SSW_PREFIX  "SPRJ32A60_"
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR32AU60_")
   #else
   #define SSW_PREFIX  "SPR32A60_"
   #endif
   #endif // SPREAD_JPN

#elif (defined(SS_SPREADOCX))

   #ifdef SPREAD_JPN
   #define SSW_PREFIX  "SPRJ32X60_"
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR32XU60_")
   #else
   #define SSW_PREFIX  "SPR32X60_"
   #endif
   #endif // SPREAD_JPN

#endif // SS_SPREADOCX && SS_V40

#elif defined(SS_V35)

#if (defined(SS_W))

   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR3UD35_")
   #else
   #define SSW_PREFIX  "SPR32D35_"
   #endif

#elif (defined(FP_ADB))

   #ifdef SPREAD_JPN
   #define SSW_PREFIX  "SPRJ32A35_"
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR3UA35_")
   #else
   #define SSW_PREFIX  "SPR32A35_"
   #endif
   #endif // SPREAD_JPN

#elif (defined(SS_SPREADOCX))

   #ifdef SPREAD_JPN
   #define SSW_PREFIX  "SPRJ32X35_"
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR3UX35_")
   #else
   #define SSW_PREFIX  "SPR32X35_"
   #endif
   #endif // SPREAD_JPN

#endif // SS_SPREADOCX && SS_V35

#elif SS_V30

#if (defined(SS_BC))
   #define SSW_PREFIX  "SVBX2BC_"
#elif (defined(SS_GP))
   #define SSW_PREFIX  "GP"
#elif (defined(SS_SPREADVBXPP))
   #define SSW_PREFIX  "SVBXPP2_"
#elif (defined(SS_GRID))
   #define SSW_PREFIX  "FPGRID30_"

#elif (defined(SS_W))

   #if defined(WIN32) || defined(_WIN32)
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR3UD30_")
   #else
   #define SSW_PREFIX  "SPR32D30_"
   #endif
   #else
   #define SSW_PREFIX  "SPR16D30_"
   #endif

#elif (defined(FP_ADB))

   #ifdef SPREAD_JPN
   #define SSW_PREFIX  "SPRJ32A30_"
   #else // SPREAD_JPN
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR3UA30_")
   #else
   #define SSW_PREFIX  "SPR32A30_"
   #endif
   #endif // SPREAD_JPN

#elif (defined(SS_SPREADOCX))

   #ifdef SPREAD_JPN
   #define SSW_PREFIX  "SPRJ32X30_"
   #else // SPREAD_JPN
   #if defined(WIN32) || defined(_WIN32)
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SPR3UX30_")
   #else
   #define SSW_PREFIX  "SPR32X30_"
   #endif
   #else
   #define SSW_PREFIX  "SPR16X30_"
   #endif
   #endif // SPREAD_JPN

#elif (defined(SS_GRIDOCX))

   #if defined(WIN32) || defined(_WIN32)
   #define SSW_PREFIX  "Grid32Ocx30_"
   #else
   #define SSW_PREFIX  "Grid16Ocx30_"
   #endif

#elif (defined(SS_RSDLL))
   #define SSW_PREFIX  "RS32D30_"
#elif (defined(SS_RSOCX))
   #define SSW_PREFIX  "RSSprd32_"
#elif (defined(SS_MASKTEXT))
   #define SSW_PREFIX  "MT_"
#else // VBX
   #define SSW_PREFIX  "SVBX30_"
#endif

#else // SS_V30

#if (defined(SS_BC))
   #define SSW_PREFIX  "SVBX2BC_"
#elif (defined(SS_GP))
   #define SSW_PREFIX  "GP"
#elif (defined(SS_SPREADVBXPP))
   #define SSW_PREFIX  "SVBXPP2_"
#elif (defined(SS_GRID))
   #define SSW_PREFIX  "FPGRID25_"

#elif (defined(SS_W))

   #if defined(WIN32) || defined(_WIN32)
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SS3UD25_")
   #else
   #define SSW_PREFIX  "SS32D25_"
   #endif
   #elif (defined(SS_V21))
   #define SSW_PREFIX  "SS16D21_"
   #else
   #define SSW_PREFIX  "SS16D25_"
   #endif

#elif (defined(SS_SPREADOCX))

   #if defined(WIN32) || defined(_WIN32)
   #if defined(UNICODE) || defined(_UNICODE)
   #define SSW_PREFIX  _T("SS3UX25_")
   #else
   #define SSW_PREFIX  "SS32X25_"
   #endif
   #else
   #define SSW_PREFIX  "SS16X25_"
   #endif

#elif (defined(SS_GRIDOCX))

   #if defined(WIN32) || defined(_WIN32)
   #define SSW_PREFIX  "Grid32Ocx25_"
   #else
   #define SSW_PREFIX  "Grid16Ocx25_"
   #endif

#elif (defined(SS_RSDLL))
   #define SSW_PREFIX  "RS32D25_"
#elif (defined(SS_RSOCX))
   #define SSW_PREFIX  "RSSprd32_"
#elif (defined(SS_MASKTEXT))
   #define SSW_PREFIX  "MT_"

#else // VBX

   #if (defined(SS_V21))
   #define SSW_PREFIX  "SVBX21_"
   #else
   #define SSW_PREFIX  "SVBX25_"
   #endif

#endif

#endif // SS_V30

#define SSW_CLASS(szClassName) SSW_PREFIX szClassName

#define VA_GENERAL_CLASS_NAME  SSW_CLASS(_T("Edit"))
#define VA_DATE_CLASS_NAME     SSW_CLASS(_T("Date"))
#define VA_TIME_CLASS_NAME     SSW_CLASS(_T("Time"))
#define VA_INTEGER_CLASS_NAME  SSW_CLASS(_T("Integer"))
#define VA_FLOAT_CLASS_NAME    SSW_CLASS(_T("Float"))
#define VA_PIC_CLASS_NAME      SSW_CLASS(_T("Pic"))
#ifdef SS_V40
#define VA_NUM_CLASS_NAME      SSW_CLASS(_T("Num"))
#endif // SS_V40
//#define VA_CALENDAR_CLASS_NAME SSW_CLASS("Calendar")
#define VA_CALENDAR_CLASS_NAME _T("fpSSCalendar")
#define VA_SPINBTN_CLASS_NAME  SSW_CLASS(_T("SpinBtn"))
#define VA_VIEWTEXT_CLASS_NAME SSW_CLASS(_T("ViewText"))
#define VA_SPREAD_CLASS_NAME   SSW_CLASS(_T("SpreadSheet"))
#define VA_CHECKBOX_CLASS_NAME SSW_CLASS(_T("CheckBox"))

#ifdef SS_W
   #undef VA_SPREAD_CLASS_NAME
   #if SS_V80
     #if defined(UNICODE) || defined(_UNICODE)
       #define VA_SPREAD_CLASS_NAME _T("fpUSpread80")
     #else
       #define VA_SPREAD_CLASS_NAME "fpSpread80"
     #endif
   #elif SS_V75
     #if defined(UNICODE) || defined(_UNICODE)
       #define VA_SPREAD_CLASS_NAME _T("fpUSpread70")
     #else
       #define VA_SPREAD_CLASS_NAME "fpSpread70"
     #endif
   #elif defined(WIN32) || defined(_WIN32)
   #if defined(SS_V70)
     #if defined(UNICODE) || defined(_UNICODE)
       #define VA_SPREAD_CLASS_NAME _T("fpUSpread70")
     #else
       #define VA_SPREAD_CLASS_NAME "fpSpread70"
     #endif
   #elif defined(SS_V40)
     #if defined(UNICODE) || defined(_UNICODE)
       #define VA_SPREAD_CLASS_NAME _T("fpUSpread60")
     #else
       #define VA_SPREAD_CLASS_NAME "fpSpread60"
     #endif
   #elif defined(SS_V35)
   #define VA_SPREAD_CLASS_NAME "fpSpread35"
   #else
   #define VA_SPREAD_CLASS_NAME "fpSpread30"
   #endif
   #else
   #define VA_SPREAD_CLASS_NAME "fp16Spread30"
   #endif
#endif

typedef struct tbwndclass
   {
   UINT    style;
#ifdef STATIC
   LONG    (_export FAR PASCAL *lpfnWndProc)();
#else
   LONG    (FAR PASCAL *lpfnWndProc)();
#endif
   int     cbClsExtra;
   int     cbWndExtra;
   HANDLE  hInstance;
   HICON   hIcon;
   HCURSOR hCursor;
   HBRUSH  hbrBackground;
   LPCTSTR lpszMenuName;
   LPCTSTR lpszClassName;
   } TBWNDCLASS, FAR *LPTBWNDCLASS;

/**********************
* Function Prototypes
**********************/

extern LRESULT tbPostMessageToParent(HWND hWnd, UINT Msg, WPARAM wParam,
                                     LPARAM lParam);
extern LRESULT tbSendMessageToParent(HWND hWnd, UINT Msg, WPARAM wParam,
                                     LPARAM lParam);
extern void    tbDrawShadows(HDC hDC, LPRECT lpRect, short iShadowSize,
                             COLORREF ColorShadow, COLORREF ColorHighlight,
                             BOOL fShadowIn, BOOL fShadowButton);
extern void    tbShrinkRect(LPRECT lpRect, short x, short y);
extern void    tbDrawIcon(HDC hDC, LPRECT lpRect, COLORREF Color, short x,
                          short y, HICON hIcon);
#if defined(_WIN64) || defined(_IA64)
extern void    tbDrawBitmap(HDC hDC, HBITMAP hBitmap, LPRECT lpRect, long x,
                            long y, DWORD dwRop);
#else
extern void    tbDrawBitmap(HDC hDC, HBITMAP hBitmap, LPRECT lpRect, short x,
                            short y, DWORD dwRop);
#endif
extern HBITMAP tbCreateStretchedIcon(HDC hDC, LPRECT lpRect, COLORREF Color,
                                     HICON hIcon, BOOL fProportionally);
extern HBITMAP tbCreateStretchedBitmap(HDC hDC, HBITMAP hBitmap, LPRECT lpRect,
                                       BOOL fProportionally, HPALETTE hPal);
extern void    tbGetBitMapInfoHeader(HBITMAP hBitmap, LPBITMAPINFOHEADER lpBI,
                                     short nWidth, short nHeight);
extern WORD    tbSetDIBitsToDevice(HDC hDC, HANDLE hDib, LPRECT lpRect,
                                   short x, short y);

extern HDC     tbGetPrinterDC(void);

extern BOOL RegisterCheckBox(HANDLE hInstance, LPCTSTR lpszClassName);
extern BOOL RegisterDirFile(HANDLE hInstance);
extern BOOL RegisterDirList(HANDLE hInstance);
extern BOOL RegisterDriveList(HANDLE hInstance);
extern BOOL RegisterFileList(HANDLE hInstance);
extern BOOL RegisterImprint(HANDLE hInstance);
extern BOOL RegisterProgress(HANDLE hInstance);
extern BOOL RegisterRadioBtn(HANDLE hInstance);
extern BOOL RegisterSpinBtn(HANDLE hInstance, LPCTSTR lpszClassName);
extern BOOL RegisterSuperBtn(HANDLE hInstance);
extern BOOL RegisterView(HANDLE hInstance, LPCTSTR lpszClassName);
extern BOOL RegisterViewPict(HANDLE hInstance, LPCTSTR lpszClassName);
extern BOOL RegisterToolBox(HANDLE hInstance);
extern BOOL RegisterStatusBar(HANDLE hInstance);
extern BOOL RegisterImprintEdit(HANDLE hInstance);

/********************************
* Function prototypes - tbdib.c
********************************/

HBITMAP      tbBitmapFromDib(HANDLE hdib, HPALETTE hpal, BOOL fPaletteIncluded);
HANDLE       tbOpenDIB(LPCTSTR szFile, HPBYTE BmpResourceData,
                     long BmpResourceDataSize);
GLOBALHANDLE tbReadDibBitmapInfo(
                               #if defined(WIN32)
                               HANDLE fh,
                               #else
                               int fh,
                               #endif
                               HPBYTE BmpResourceData,
                               long BmpResourceDataSize,
                               long FAR *BmpResourceDataAt);
WORD         tbPaletteSize(VOID FAR *pv);
WORD         tbDibNumColors(VOID FAR *pv);
BOOL         tbDibInfo(HANDLE hbi, LPBITMAPINFOHEADER lpbi);
DWORD PASCAL tbDibLRead(
                      #if defined(WIN32)
                      HANDLE hFile,
                      #else
                      int hFile,
                      #endif
                      VOID far *pBuffer, DWORD Len,
                      HPBYTE BmpResourceData, long BmpResourceDataSize,
                      long FAR *BmpResourceDataAt);
HPALETTE     tbCreateDibPalette(HANDLE hbi);
HPALETTE     tbCreateBIPalette(LPBITMAPINFOHEADER lpbi);
HANDLE       tbDibFromBitmap(HBITMAP hbm, DWORD biStyle, WORD biBits,
                           HPALETTE hpal, LPDWORD lpdwLen,
                           LPSHORT lpdPaletteSize);
HANDLE       tbBitmapToDIB(HBITMAP hBitmap, HPALETTE hPal);
BOOL         tbStretchDibBlt(HDC hDC, int x, int y, int dx, int dy,
                           HANDLE hDib, int x0, int y0, int dx0,
                           int dy0, LONG Rop);
HPALETTE     tbGetSystemPalette(void);
GLOBALHANDLE tbSaveDIBToBuffer(GLOBALHANDLE hdib, LPLONG lplBufferSize);
HBITMAP      tbIconToBitmap(HWND hWnd, HICON hIcon, COLORREF Color);
long         tbGetIconWidth(HICON hIcon);
long         tbGetIconHeight(HICON hIcon);

#ifdef __cplusplus
}
#endif

#endif
