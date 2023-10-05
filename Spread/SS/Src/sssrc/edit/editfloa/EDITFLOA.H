/* editfloa.h */

/*
$Revision:   1.9  $
*/

/*
$Log:   N:/DRLIBS/TOOLBOX/EDIT/EDITFLOA/EDITFLOA.H_V  $
 * 
 *    Rev 1.9   04 Dec 1990 13:40:54   Dirk
 * No change.
 * 
 *    Rev 1.8   12 Jun 1990 14:10:30   Dirk
 * No change.
 * 
 *    Rev 1.7   28 May 1990 15:58:56   Sonny
 * 
 *    Rev 1.6   24 May 1990 16:21:40   Sonny
 * 
 *    Rev 1.5   23 May 1990 18:16:12   Sonny
 * 
 *    Rev 1.4   23 May 1990 16:51:16   Sonny
 * 
 *    Rev 1.3   22 May 1990 17:02:06   Sonny
 * 
 *    Rev 1.2   21 May 1990 13:27:56   Randall
 * No change.
 * 
 *    Rev 1.1   15 May 1990 17:05:16   Sonny
 * 
 *    Rev 1.0   11 May 1990 16:17:18   Bruce
 * Initial revision.
 * 
 *    Rev 1.3   09 May 1990 15:29:26   Randall
 * ProcessFloatWMKeyDown now returns BOOL: TRUE if no need to call FieldMgr.
 * 
 *    Rev 1.2   01 May 1990 10:38:44   Sonny
 * 
 *    Rev 1.1   30 Apr 1990 13:21:04   Bruce
 * No change.
 * 
 *    Rev 1.0   23 Apr 1990 18:41:36   Sonny
 * Initial revision.
*/
#define    IN_LINE


typedef
struct
{
   HWND     hWnd;
   int      iErrorCode;                   // the error code to send
   double   dfMinVal;                     // the range value
   double   dfMaxVal;                     // the range value
   double   dfVal;                         // the current value
   LONG     style;
   BYTE     bIntegerWidth;
   BYTE     bFractionalWidth;
   BYTE     bTotalWidth;
   BYTE     bMaxWidth;
   BYTE     bMinWidth;
   BOOL     fNegative : 1;
   BOOL     fEmpty : 1;                   // this is true if the buffer is empty
   LPARAM   iWindowID;                    // window id
   HWND     hWndParent;                   // the parent window, if any
   TUCHAR   cCurrencySign;                // the character to echo for currency e.g. '$', 'ú', 'ù' etc.
   TUCHAR   cDecimalSign;                 // decimal separator e.g. '.' or ','
   TUCHAR   cSeparator;                // thousand separator e.g. ',' or '.'
   BYTE     bPostCount;                // counter for PostMessage in EM_VALIDATE
   int      iPrevStrLen;
   WPARAM   wKeyDownVirtualKeyCode;
} EDITFLOATINFO;

typedef  EDITFLOATINFO FAR *LPEDITFLOATINFO;

#define  LockFloatHeader(lpInfo,hInfo,hWnd)  \
   HANDLE          (hInfo);\
   LPEDITFLOATINFO   (lpInfo);\
   (hInfo) = GetWindowFloatField(hWnd);\
   (lpInfo) = (LPEDITFLOATINFO)GlobalLock((hInfo))\

#define  UnlockFloatHeader(hInfo)     GlobalUnlock((hInfo))


#define  WININTL          _T("intl")
#define  WIN_THOUSAND     _T("sThousand")
#define  WIN_DECIMAL      _T("sDecimal")
#define  WIN_CURRENCY     _T("sCurrency")
#define  WIN_DEFCURRENCY  _T("$")
#define  WIN_DEFDECIMAL   _T(".")
#define  WIN_DEFTHOUSAND  _T(",")

// function prototypes

BOOL  ProcessFloatWMChar(HWND hWnd, WPARAM wParam, LPARAM lParam);
BOOL  ProcessFloatWMCreate(HWND hWnd, LPARAM lParam);
LONG  ProcessFloatEMValidate(HWND hWnd, WPARAM wParam, LPARAM lParam);
void  ProcessFloatWMDestroy(HWND hWnd);
BOOL  ProcessFloatWMSetText(HWND hWnd, WPARAM wParam, LPCTSTR lParam,
                            BOOL fPassOnOnly, BOOL fInternal);
BOOL  ProcessFloatEMRange(HWND hWnd, WPARAM wParam, LPARAM lParam);
void  ProcessFloatWMKillFocus(HWND hWnd);
void  SendFloatPictureText(HWND hWnd);
BOOL  ProcessFloatEMCaretPos(HWND hWnd, WPARAM wParam, LPARAM lParam);
BOOL  ProcessFloatWMKeyDown(HWND hWnd, WPARAM wParam);
void  ProcessFloatWMKeyUp(HWND hWnd, WPARAM wParam);
void  SetInitialCaretPosition (HWND hWnd);
void  ProcessWMIniChange(HWND hWnd, WPARAM wParam, LPARAM lParam);
BOOL  ProcessEMSetFormat(HWND hWnd, WPARAM wParam, LPARAM lParam);
BOOL  ProcessEMGetFormat(HWND hWnd, LPARAM lParam);
BOOL  FloatFormat(LPEDITFLOATINFO lpInfo, LPDOUBLE lpVal, LPTSTR szString);
BOOL  GetFloatPicture(LPEDITFLOATINFO lpInfo, LPCTSTR szRange);
void  FloatResetPrevLen(HWND hWnd);
void  FloatReplaceText(HWND hWnd, LPARAM lParam);

#ifndef  IN_LINE
int   DLLENTRY StrDeleteCh(LPTSTR szStr, TCHAR ch);
int   DLLENTRY StrReplaceCh(LPTSTR szStr, TCHAR chEnc);

#else

#define  StrDeleteCh(szOrg,ch) \
{\
   register int   iNext = 0;\
   LPTSTR   szPtr = (szOrg);\
   LPTSTR   szStr = (szOrg);\
\
   while (*szStr)\
   {\
      if (*szStr == (TCHAR)(ch))\
      {\
         ++iNext;\
         *szStr = *(szStr + iNext);\
      }\
      else\
      {\
         ++szStr;\
         *szStr = *(szStr + iNext);\
      }\
   }\
   ((int)(szStr - szPtr));\
}

#define  StrReplaceCh(szOrg,ch)  \
{\
   LPTSTR szPtr = (szOrg);\
   LPTSTR szStr = (szOrg);\
\
   while (*szStr)\
   {\
      if ((BYTE)*szStr == HIBYTE((ch)))\
         *szStr = LOBYTE((ch));\
      ++szStr;\
   }\
   ((int)(szStr - szPtr));\
}

#endif

