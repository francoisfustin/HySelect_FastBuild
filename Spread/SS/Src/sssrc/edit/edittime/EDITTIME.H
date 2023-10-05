/*
$Revision:   1.10  $
*/                        

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/EDITTIME/EDITTIME.H_V  $
 *
 *    Rev 1.10   12 Jun 1990 14:08:56   Dirk
 * No change.
 *
 *    Rev 1.9   24 May 1990 17:35:06   Randall
 * Added LoadCurrentTime
 *
 *    Rev 1.8   24 May 1990 12:01:22   Randall
 * Added TimeGetFormat
 *
 *    Rev 1.7   23 May 1990 16:49:22   Sonny
 *
 *    Rev 1.6   23 May 1990 12:13:16   Randall
 *
 *    Rev 1.5   23 May 1990  9:18:00   Randall
 * No change.
 *
 *    Rev 1.4   22 May 1990 15:44:28   Randall
 * Added support for TimeSetFormat
 *
 *    Rev 1.3   22 May 1990 12:45:46   Randall
 * Added support for range checking
 *
 *    Rev 1.2   21 May 1990 13:29:46   Randall
 *
 *    Rev 1.1   16 May 1990 16:15:42   Randall
 *
 *    Rev 1.0   11 May 1990 16:12:14   Bruce
 * Initial revision.
 *
 *    Rev 1.3   10 May 1990 17:01:42   Randall
 *
 *    Rev 1.2   10 May 1990 10:38:46   Randall
 * Changed internal structure to include TIMEFORMAT structure.
 *
 *    Rev 1.1   30 Apr 1990 13:20:24   Bruce
 * No change.
 *
 *    Rev 1.0   23 Apr 1990 18:42:24   Sonny
 * Initial revision.
*/

#define  DEFAULT_SEPARATOR _T(":")
// Added for remove preprocessor Charles Feng BOC
#define MAKEINT(low, high) ((int)(((BYTE)(low)) | (((unsigned int)((BYTE)(high))) << 8)))
#define KANJISEPARATOR  '?'

//- Changed by Masanori Iwasa BOC  94' 04/23
#define  TIMELENGTH     20    // length of strings for Time functions

#define  LOCAL_LENGTH   24    // Length of local working string.

#define  UNDEFINED   -1L

#define TIME_HH 0
#define TIME_MM 1
#define TIME_SS 2
#define TIME_AM 3

typedef struct tagTimeField
{
   HWND       hParent;       // Handle of the parent window
   HWND       hWndSpinBtn;
   TIMEFORMAT tf;
   LONG       lRangeMin;
   LONG       lRangeMax;
} TIMEFIELD;
typedef TIMEFIELD FAR *LPTIMEFIELD;

BOOL        TimeCreateField      (HWND, LPARAM);
void        TimeInitializeField  (HWND, LPARAM);
BOOL        TimeSetValidRange    (HWND, LPARAM);
void        TimeSetFieldFormat   (HWND, LPARAM);
LONG        TimeGetFieldFormat   (HWND, LPARAM);
void        TimeDestroyField     (HWND);
LONG        TimeDynamicValidate  (HWND, WPARAM, LPARAM);
LONG        TimeStaticValidate   (HWND, LPARAM*);
BOOL        TimeFinalValidate    (HWND);
BOOL        TimeIsInRange        (HWND, LPCTSTR, LPTIMEFORMAT);
void        LoadCurrentTime      (HWND);
LPTIMEFIELD LockTimeField        (HWND);
void        UnlockTimeField      (HWND);
void        TimeSize             (HWND hWnd, LPARAM lParam);
void        TimeResetSpinBtn     (HWND hWnd, BOOL fShowSpin);
void        TimeVScroll          (HWND hWnd, int nScrollCode);
short       TimeGetCursorPos     (HWND hWnd, short iPos);
BOOL        TimeProcessWMKeyDown (HWND hWnd, WPARAM wParam);
void        TimeEnable           (HWND hWnd, WPARAM wParam);

