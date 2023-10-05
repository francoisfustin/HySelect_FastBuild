/*
$Revision:   1.2  $
*/

/*
$Log:   N:/DRLIBS/TOOLBOX/EDIT/EDITINT/USERINT.C_V  $
 * 
 *    Rev 1.2   04 Dec 1990 13:43:42   Dirk
 * No change.
 * 
 *    Rev 1.1   12 Jun 1990 14:10:52   Dirk
 * No change.
 * 
 *    Rev 1.0   04 Jun 1990 14:55:24   Bruce
 * Initial revision.
 * 
 *    Rev 1.5   24 May 1990 17:24:22   Sonny
 * No change.
 * 
 *    Rev 1.4   23 May 1990 16:50:14   Sonny
 * 
 *    Rev 1.3   21 May 1990 13:29:12   Randall
 * 
 *    Rev 1.2   17 May 1990 17:29:36   Randall
 * Check class in SetIntRange
 * 
 *    Rev 1.1   15 May 1990 17:04:32   Sonny
 * No change.
 * 
 *    Rev 1.0   11 May 1990 16:14:28   Bruce
 * Initial revision.
 * 
 *    Rev 1.1   09 May 1990 15:16:46   Randall
 * Added Trim to remove leading space.  
 * 
 *    Rev 1.0   07 May 1990 16:16:06   Randall
 * Initial revision.
*/

#define  NOCOMM             // COMM driver routines
#define  NOKANJI            // Kanji support stuff.

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <toolbox.h>
#include <tchar.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "editint.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "editint\editint.h"
<-BORLAND*/
#endif


//--------------------------------------------------------------------------//
 
BOOL DLLENTRY IntSetRange (HWND hWnd, LONG Min, LONG Max)

{
   TCHAR szString[40 + 1];  

   if (Min > Max) return FALSE;
#if 0
   GetClassName (hWnd, szString, 40);
   if (StrCmp (szString, INTEGER_CLASS_NAME) != 0) return FALSE;
#endif
   StrPrintf (szString, _T("%ld %ld"), Min, Max);

   return
      (BOOL) SendMessage (hWnd, EM_SETRANGE, 0, (LPARAM)(LPTSTR) szString);
}
 
BOOL DLLENTRY IntGetRange (HWND hWnd, LPLONG lpMin, LPLONG lpMax)

{
   LockIntHeader(lpInfo, hInfo, hWnd);

   *lpMin = lpInfo->lMinVal;
   *lpMax = lpInfo->lMaxVal;

   UnlockIntHeader(hInfo);
   return (TRUE);
}


BOOL  DLLENTRY IntGetValue(HWND hWnd, LPLONG lpLong)
{
   TCHAR  szVal[40];
   LPTSTR szPtr = szVal;

   szPtr[0] = '\0';
   SendMessage(hWnd, WM_GETTEXT, 39, (LPARAM) (LPTSTR)szVal);
   *lpLong = StringToLong(szVal);
   return(TRUE);
}


BOOL DLLENTRY IntSetValue(HWND hWnd, long lLong)
{
   TCHAR  szVal[40];

   wsprintf(szVal, _T("%ld"), lLong);
   /*
   SendMessage(hWnd, WM_SETTEXT, 0, (LONG)(LPTSTR)szVal);
   */
   if (ProcessIntWMSetText(hWnd, 0, (LPARAM)(LPTSTR)szVal))
      CallWindowProc((WNDPROC)GetDefWndProc(hWnd), hWnd, WM_SETTEXT, 0,
                     (LPARAM)(LPTSTR)szVal);

   return (TRUE);
}


BOOL DLLENTRY IntSetMask(HWND hWnd, LPCTSTR szMask)
{
long lVal;
BOOL Ret;

LockIntHeader(lpInfo, hInfo, hWnd);

IntGetValue(hWnd, &lVal);
Ret = GetIntPicture(lpInfo, szMask);
IntSetValue(hWnd, lVal);

UnlockIntHeader(hInfo);
return (Ret);
}


BOOL DLLENTRY IntSetSpin(HWND hWnd, BOOL fSpin, BOOL fSpinWrap, long lSpinInc)
{
LockIntHeader(lpInfo, hInfo, hWnd);

if (fSpin)
   {
   lpInfo->fSpinWrap = fSpinWrap;
   lpInfo->lInc = lSpinInc;
   }

IntResetSpinBtn(hWnd, fSpin);

InvalidateRect(hWnd, NULL, TRUE);
UpdateWindow(hWnd);

UnlockIntHeader(hInfo);
return (TRUE);
}


BOOL DLLENTRY IntGetSpin(HWND hWnd, LPBOOL lpfSpinWrap, LPLONG lplSpinInc)
{
BOOL Ret;

LockIntHeader(lpInfo, hInfo, hWnd);

*lpfSpinWrap = lpInfo->fSpinWrap;
*lplSpinInc = lpInfo->lInc;

if (lpInfo->hWndSpinBtn)
   Ret = TRUE;
else
   Ret = FALSE;

UnlockIntHeader(hInfo);
return (Ret);
}


BOOL DLLENTRY IntGetMask(HWND hWnd, LPTSTR szMask)
{
short dAt = 0;
short i;
BOOL  Ret = TRUE;

LockIntHeader(lpInfo, hInfo, hWnd);

for (i = 0; i < (short)lpInfo->bMaxWidth; i++)
   szMask[dAt++] = '9';

szMask[dAt] = '\0';

UnlockIntHeader(hInfo);
return (Ret);
}
