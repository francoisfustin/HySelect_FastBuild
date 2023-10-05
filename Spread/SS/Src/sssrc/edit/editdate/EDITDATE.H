/*
$Revision:   1.9  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/EDITDATE/EDITDATE.H_V  $
 * 
 *    Rev 1.9   12 Jun 1990 14:09:50   Dirk
 * No change.
 * 
 *    Rev 1.8   28 May 1990 15:34:20   Randall
 * 
 *    Rev 1.7   24 May 1990 17:33:40   Randall
 * Load current date
 * 
 *    Rev 1.6   24 May 1990 11:03:22   Randall
 * 
 *    Rev 1.5   23 May 1990 16:48:08   Sonny
 * 
 *    Rev 1.4   23 May 1990  9:19:42   Randall
 * No change.
 * 
 *    Rev 1.3   21 May 1990 16:22:26   Randall
 * Added date range checking
 * 
 *    Rev 1.2   21 May 1990 13:27:14   Randall
 * 
 *    Rev 1.1   14 May 1990 17:44:58   Randall
 * 
 *    Rev 1.0   11 May 1990 16:13:02   Bruce
 * Initial revision.
 * 
 *    Rev 1.3   11 May 1990 13:36:18   Randall
 * 
 *    Rev 1.2   10 May 1990 16:53:00   Randall
 * 
 *    Rev 1.1   10 May 1990 13:50:22   Bruce
 * No change.
 * 
 *    Rev 1.0   23 Apr 1990 18:40:56   Sonny
 * Initial revision.
*/

#define  DEFAULT_SEPARATOR _T("-")
#define  KANJISEPARATOR    '?'  //- Built in Japanese Feature (Masanori Iwasa)

//- For Japanese Era the length was changed (Masanori Iwasa)
//- Originally it was ( DATELENGTH = 12, LOCAL_LENGTH = 20 )
#define  DATELENGTH     20      // length of strings for Date functions
#define  LOCAL_LENGTH   24      // Length of local working string.

//#define  THISCENTURY    1900    // Used by DateDMYToString() etc.
#define TWODIGITYEARMAX_DEF 2059
#define  LOYEAR         0001
//#define  HIYEAR         2500
//#define  HIYEAR1        '2'
//#define  HIYEAR2        '5'
#define  HIYEAR         9999
#define  HIYEAR1        '9'
#define  HIYEAR2        '9'

#define  UNDEFINED      -1L

#define  DATE_DD        0
#define  DATE_YY        1
#define  DATE_MM        2
#define  DATE_MON       3
//- For Japanese Era support from Date types from here
//-               on was added. (Masanori Iwasa)
#define  DATE_NULL      4
#define  DATE_NYY       5
#define  DATE_NNYY      6
#define  DATE_NNNNYY    7

//- For Japanese Era support this User defined messages was added.
//  This messages is used to check the range of an era. (Masanori Iwasa)
#define  UM_CHECKNENGO  (WM_USER) + 400

#define DM_POSTCALENDAR 0x0555

#define  ISLEAP(nYear)  (nYear%4 == 0 && nYear%100 != 0 || nYear%400 == 0)

typedef struct tagDateEditInfo
   {
   short nChars;
   short iCurrentPos;
   } DATEEDITINFO, FAR *LPDATEEDITINFO;

typedef struct tagDateField
   {
   HWND       hParent;          // Handle of the parent window
   HWND       hWndSpinBtn;
   DATEFORMAT df;
   LONG       lRangeMin;
   LONG       lRangeMax;
   short      nTwoDigitYearMax;
   BYTE       fStrictValidate;
   BOOL		  fEnhancedCalendar;
} DATEFIELD, FAR *LPDATEFIELD;

BOOL  DateCreateField      (HWND, LPARAM);
void  DateInitializeField  (HWND, LPARAM);
BOOL  DateSetValidRange    (HWND, LPARAM);
void  DateSetFieldFormat   (HWND, LPARAM);
BOOL  DateGetFieldFormat   (HWND, LPARAM);
void  DateDestroyField     (HWND);
LRESULT DateDynamicValidate(HWND, WPARAM, LPARAM);
BOOL  ValidateDDMMYY       (HWND, LPCTSTR, int, BOOL, TCHAR);
BOOL  ValidateMMDDYY       (HWND, LPCTSTR, int, BOOL, TCHAR);
BOOL  ValidateYYMMDD       (HWND, LPCTSTR, int, BOOL, TCHAR);
LONG  DateStaticValidate   (HWND, LPARAM*);
BOOL  DateFinalValidate    (HWND);
void  LoadCurrentDate      (HWND);
BOOL  DateIsInRange        (HWND, LPTSTR, LPDATEFORMAT);
BOOL  MatchMonthAbbrev     (LPTSTR, BOOL FAR *);
LPDATEFIELD LockDateField  (HWND);
void  UnlockDateField      (HWND);
void  DateSize             (HWND hWnd, LPARAM lParam);
void  DateResetSpinBtn     (HWND hWnd, BOOL fShowSpin);
short DaysPerMonth         (short nMonth, short nYear);
void  DateResetSpinBtn     (HWND hWnd, BOOL fShowSpin);
void  DateVScroll          (HWND hWnd, int nScrollCode);
short DateGetCursorPos     (HWND hWnd, short iPos);
//- CAUTION -------------------------------------------------------------+
//- To intergrate the Japanese Era features the third parameter was      |
//  changed from (BOOL bCentury) to (DATEFORMAT df). (Masanori Iwasa)    |
//-----------------------------------------------------------------------+-----------------
BOOL  DateGetCursorPos2    (HWND hWnd, short iPos, DATEFORMAT df,
                            short First, short Second, short Third);
//-----------------------------------------------------------------------------------------
BOOL  DatePrevious         (LPTB_DATE lpDate, short Years, short Months,
                            short Days);
BOOL  DateNext             (LPTB_DATE lpDate, short Years, short Months,
                            short Days);
//- CAUTION -------------------------------------------------------------+
//- To intergrate the Japanese separators, (char cSeparator) was added   |
//  as the first parameter. (Masanori Iwasa)                             |
//-----------------------------------------------------------------------+-----------------
short DateGetYear          (TUCHAR cSeparator, LPCTSTR lpszDate, int nFormat,
                            BOOL bCentury, short nTwoDigitYearMax);
short DateGetMonth         (TUCHAR cSeparator, LPCTSTR lpszDate, int nFormat, BOOL bCentury);
short DateGetDay           (TUCHAR cSeparator, LPCTSTR lpszDate, int nFormat, BOOL bCentury);
//-----------------------------------------------------------------------------------------
// For DateSetDay the parameter (char cSeparator) was added as the 
// second parameter. (Masanori Iwasa)
//-----------------------------------------------------------------------------------------
BOOL  DateSetDay           (HWND hWnd, TUCHAR cSeparator, LPTSTR lpszDate,
                            int nFormat, BOOL bCentury, short nDay,
                            LPDATEEDITINFO lpEditInfo);
//-----------------------------------------------------------------------------
BOOL DLLENTRY xDateStringIsValid( LPCTSTR lpszDateString, LPDATEFORMAT datefmt,
                                  BOOL fCheckYear, short nTwoDigitYearMax);
BOOL  DateProcessWMKeyDown(HWND hWnd, WPARAM wParam);
void  DateEnable(HWND hWnd, WPARAM wParam);

//- The following functions were added to intergrate Japanese Era
//  and separators etc.. (Masanori Iwasa)
void  GetNengoFromDate(LPTB_DATE lpDate, LPTSTR szTmpStr, int nNengoType);
int   GetYearFromNengo(LPCTSTR lpDate, int NengoType);
void  SetNengoInit(HWND hwnd, LPTSTR lpszDate, int NengoType, int Nengo, LPDATEEDITINFO lpEditInfo, BOOL bCurrentDate);
void  CheckNengoRange(HWND hwnd);
void  NengoScroll(LPTB_DATE lpDate, int Direction);
void  SS_Beep1(HWND hWnd, UINT uAlert);
BOOL  IsDbcsSeparator(TUCHAR cSeparator);
