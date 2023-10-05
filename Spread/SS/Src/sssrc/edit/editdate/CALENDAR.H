////////////////////////////////////////////////////////////////////////////
// Include file for Calendar Class                    Steve M.   May 1990 //
////////////////////////////////////////////////////////////////////////////
// NOTE: this file is grepped by my makefile for DEFINE statements then for
// underscores for a Dialog-Editor Includeable version.

// ID's for the dialog and Calendar Class bits:
// MOVED TO \DRLIBS\TOOLBOX\DIALOG.H:
/* #define IDD_MONTH          200 */ 

#define DATENAMESZ 32

// Messages for telling the calendar class to do things:
#define CM_CHANGEMONTH     (WM_USER + 0)
#define CM_NEXTMONTH       (WM_USER + 1)
#define CM_SETDAY          (WM_USER + 2)
#define CM_GETMONTH        (WM_USER + 3)

// Values for the DrawDaysList() function CalInfo structure flag "iAction":
#define DRAW_EVERYTHING    400
#define DRAW_MONTH         410
#define DRAW_DAYS          420
#define DRAW_FOCUS         430

// Values for the DayFocusAt() function flag "iFocusAction":
#define DAY_CLEAROLDFOCUS  500
#define DAY_DRAWNEWFOCUS   510

// Values for HOME and END Focus processing:
#define FOCUS_HOME         600
#define FOCUS_END          610
// Following are the first and last chars for finding the font widths of:
#define FIRSTCHAR          ' '
#define LASTCHAR           '~'


#ifdef SS_V35
typedef struct tagCALTEXT
{
   TCHAR MonthLongNames[12][DATENAMESZ];
   TCHAR MonthShortNames[12][DATENAMESZ];
   TCHAR DayLongNames[7][DATENAMESZ];
   TCHAR DayShortNames[7][DATENAMESZ];
   TCHAR OkText[15];
   TCHAR CancelText[15];
} CALTEXT, FAR *LPCALTEXT;

#endif // SS_V35

// Structure template to hold variables for each instance of the calendar:
typedef struct tagCALINFO
{
   int     iMonth;
   int     iYear;
   short   nHScrollPos;
   int     iDayFocus;
   int     iPreviousDayFocus;
   int     iAction;
   int     iStartDofW;
   int     iDaysInThisMonth;
   int     iCalWidth;
   int     iCalHeight;
   int     iWeekRectTop;
   int     iDaysRectTop;
   int     iDaysRectBottom;
   int     iBoxW;
   int     iWhereLast;
   int     iHeaderItemHeight;
   RECT    RectMonthHeader;
   RECT    RectYearHeader;
   HWND    hWndSpinYearLeft;
   HWND    hWndSpinYearRight;
   HWND    hWndSpinMonthLeft;
   HWND    hWndSpinMonthRight;
   BOOL    fCheckMin;
   BOOL    fCheckMax;
   TB_DATE DateMin;
   TB_DATE DateMax;
   HFONT   hFont;
//#ifdef MASA
   int     nFormat;
//#endif
#ifdef SS_V35
   CALTEXT calText;
#endif
   BOOL fEnhanced;
} CALINFO, FAR *LPCALINFO;


//////////////////////////////////////////////////////////////////////////////////
typedef LPVOID PTR;
// Define the structures for the Dialog Template and Dialog Item Controls:
typedef struct
{
   int   dtilX;
   int   dtilY;
   int   dtilCX;
   int   dtilCY;
   int   dtilID;
   LONG  dtilStyle;
   TCHAR *dtilClass;
   TCHAR *dtilText;
   int   dtilInfo;
   int   dtilData;
}  CAL_DLGITEMTEMPLATE;
typedef  CAL_DLGITEMTEMPLATE FAR *LPCAL_DLGITEMTEMPLATE;

typedef struct
{
   long  dtStyle;
   BYTE  dItemCount;
   int   dtX;
   int   dtY;
   int   dtCX;
   int   dtCY;
   TCHAR *dtMenuName;
   TCHAR *dtClassName;
   TCHAR *dtCaptionText;
   LPCAL_DLGITEMTEMPLATE lpDlgItemTemplate;
}  CAL_DLGTEMPLATE;
typedef  CAL_DLGTEMPLATE FAR *LPCAL_DLGTEMPLATE;


/////////////////////////// Function Prototypes ///////////////////////////////
// THESE ARE THE PROTOTYPES THAT ARE USED FOR THE CALENDAR CLASS:

BOOL FAR PASCAL CallDialogBoxIndirect( HWND, FARPROC);
HGLOBAL  MakeDialog (LPCAL_DLGTEMPLATE);

BOOL FAR PASCAL FillCalInfoStruct(HWND hWnd);
void            DrawHeader(HWND, HDC, LPCALINFO);
void            DrawCalendar(HWND hWnd, HDC hDC);
void            DrawDayAt(HDC hDC, LPCALINFO lpCalInfo, int iDayNumber,
                          HBRUSH hBrushFace, COLORREF ColorText,
                          BOOL fButtonDown);
void            DayFocusAt(HDC, LPCALINFO);
int FAR PASCAL  DayPointXYToBoxNumber( LPCALINFO, int, int);
LPRECT          DayBoxToRECT(LPCALINFO, LPRECT, int);
int FAR PASCAL  LineLength(LPCTSTR, LPINT);
int FAR PASCAL  DateGetWeekdayInt(LPTB_DATE);
int FAR PASCAL  DateGetDaysInMonth(LPTB_DATE);
void LoadCalendarResource();   //- Added by Masanori Iwasa at FarPoint
