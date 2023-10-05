/* editint.h */

/*
$Revision:   1.7  $
*/

/*
$Log:   N:/DRLIBS/TOOLBOX/EDIT/EDITINT/EDITINT.H_V  $
 * 
 *    Rev 1.7   04 Dec 1990 13:43:50   Dirk
 * No change.
 * 
 *    Rev 1.6   12 Jun 1990 14:10:56   Dirk
 * No change.
 * 
 *    Rev 1.5   07 Jun 1990 17:21:40   Sonny
 * 
 *    Rev 1.4   04 Jun 1990 15:27:28   Randall
 * Removed obsolete MinWidth and MaxWidth.
 * 
 *    Rev 1.3   24 May 1990 17:24:30   Sonny
 * 
 *    Rev 1.2   23 May 1990 16:50:22   Sonny
 * 
 *    Rev 1.1   21 May 1990 13:29:04   Randall
 * 
 *    Rev 1.0   11 May 1990 16:14:00   Bruce
 * Initial revision.
 * 
 *    Rev 1.1   30 Apr 1990 13:21:12   Bruce
 * No change.
 * 
 *    Rev 1.0   23 Apr 1990 18:41:50   Sonny
 * Initial revision.
*/

typedef
struct
{
   int   iErrorCode;                   // the error code to send
   LONG  lMinVal;                      // the range value
   LONG  lMaxVal;                      // the range value
   LONG  lVal;                         // current value
   BOOL  fNegative;
   BOOL  fEmpty;                       // this is true if the buffer is empty
   BYTE  bPostCount;                   // use to prevent posting another invalid data when doing a dynamic check see note below
   BYTE  bMinWidth;                    // Minimum width for the field
   BYTE  bMaxWidth;                    // maximum width for the field
   LPARAM   iWindowID;                    // window id
   long  lInc;
   BOOL  fSpinWrap;
   HWND  hWndParent;
   HWND  hWndSpinBtn;
} EDITINTINFO;

typedef  EDITINTINFO FAR *LPEDITINTINFO;

/*****************
   N.B.
   The purpose of the PostCount field is to prevent this class from
   sending multiple error notification to the parent resulting in an endless
   loop. See this class checks the value of the field (i.e. integer) against
   the range in two places:
      1) EMValidate
      2) KillFocus

****************************/

#define  LockIntHeader(lpInfo,hInfo,hWnd)  \
   HANDLE          (hInfo);\
   LPEDITINTINFO   (lpInfo) = 0;\
   if ((hInfo) = GetWindowIntField(hWnd))\
      (lpInfo) = (LPEDITINTINFO)GlobalLock((hInfo))\

#define  UnlockIntHeader(hInfo)     GlobalUnlock((hInfo))

// function prototypes

void  ProcessIntWMSetFocus(HWND hWnd);
void  ProcessIntWMKillFocus(HWND hWnd, WPARAM wParam);
BOOL  ProcessIntWMChar(HWND hWnd, WPARAM wParam, LPARAM lParam);
BOOL  ProcessIntWMKeyDown(HWND hWnd, WPARAM wParam);
BOOL  ProcessIntWMNCCreate(HWND hWnd, LPARAM lParam);
void  ProcessIntWMCreate(HWND hWnd, LPARAM lParam);
LONG  ProcessIntEMValidate(HWND hWnd, WPARAM wParam, LPARAM lParam);
void  ProcessIntWMDestroy(HWND hWnd);
BOOL  ProcessIntWMSetText(HWND hWnd, WPARAM wParam, LPARAM lParam);
BOOL  ProcessIntEMRange(HWND hWnd, WPARAM wParam, LPARAM lParam);
void  ProcessIntWMKillFocus(HWND hWnd, WPARAM wParam);
BOOL  GetIntPicture(LPEDITINTINFO lpInfo, LPCTSTR szRange);
void  IntSize(HWND hWnd, LPARAM lParam);
void  IntResetSpinBtn(HWND hWnd, BOOL fShowSpin);
void  IntVScroll(HWND hWnd, int nScrollCode);
#if defined(_WIN64) || defined(_IA64)
LRESULT  IntWMSetText(HWND hWnd, LPARAM lParam);
#else
long  IntWMSetText(HWND hWnd, long lParam);
#endif
void IntEnable(HWND hWnd, WPARAM wParam);
