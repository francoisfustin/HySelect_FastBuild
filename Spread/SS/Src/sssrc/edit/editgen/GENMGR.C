/*
$Revision:   1.9  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/EDITGEN/GENMGR.C_V  $
 * 
 *    Rev 1.9   12 Jun 1990 14:11:08   Dirk
 * No change.
 * 
 *    Rev 1.8   30 May 1990  9:36:24   Randall
 * 
 *    Rev 1.7   29 May 1990  9:38:30   Randall
 * No change.
 * 
 *    Rev 1.6   29 May 1990  9:24:08   Randall
 * Made external objects "static".
 * 
 *    Rev 1.5   29 May 1990  9:20:26   Sonny
 * 
 *    Rev 1.4   21 May 1990 13:28:26   Randall
 * 
 *    Rev 1.3   15 May 1990 15:26:42   Randall
 * 
 *    Rev 1.2   14 May 1990 17:41:50   Randall
 * 
 *    Rev 1.1   14 May 1990 15:03:46   Randall
 * Notify parent if Alloc fails
 * 
 *    Rev 1.0   11 May 1990 16:16:50   Bruce
 * Initial revision.
 * 
 *    Rev 1.0   07 May 1990 10:20:02   Randall
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI
#define  NOSOUND

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <toolbox.h>

#if defined(_MSC_VER)
#include "..\..\libgen.h"
#include "..\editfld.h"
#include "editgen.h"
#else                      // Borland
/*BORLAND->
//borland #include "..\libgen.h"
//borland #include "editfld.h"
//borland #include "editgen\editgen.h"
<-BORLAND*/
#endif

//--------------------------------------------------------------------------//
static LPGENFIELD  LockGenField     (HWND);
static void        UnlockGenField   (HWND);
static void        FreeGenField     (HWND);
//--------------------------------------------------------------------------//

BOOL  GenCreateField (HWND hWnd, LPARAM lParam)

{
   LPGENFIELD lpGen;
   HANDLE      hGen;
   BOOL        fRetCode = FALSE;

   if (!(hGen = GlobalAlloc (GHND, (DWORD) sizeof(GENFIELD))))
   {
      NotifyParent (hWnd, EN_ERRSPACE);
   }

   else
   {
      SetDefWndProc(hWnd, GetTBEditClassProc ());
      lpGen = (LPGENFIELD) GlobalLock (hGen);
      if (lpGen)
      {
         SetWindowGenField (hWnd, hGen);

         lpGen->hParent = ((LPCREATESTRUCT) lParam)->hwndParent;
         fRetCode = TRUE;
         GlobalUnlock (hGen);
      }
      else
         NotifyParent (hWnd, EN_ERRSPACE);
   }

   return(fRetCode);
}

//--------------------------------------------------------------------------//

void GenInitializeField (HWND hWnd, LPARAM lParam)

{
   LPGENFIELD lpGen;
   TCHAR      szGen[256];
   LPARAM     lGen;

   lpGen = LockGenField (hWnd);

   if (((LPCREATESTRUCT)lParam)->lpszName)
      StrnnCpy(szGen, (LPTSTR)((LPCREATESTRUCT)lParam)->lpszName,
               sizeof(szGen));
   else
      szGen[0] = '\0';

   UnlockGenField (hWnd);

   // Make the address of the string look line an lParam
   lGen = (LPARAM)(LPTSTR)szGen;
   SendMessage (hWnd, WM_SETTEXT, 0, lGen);
}

//--------------------------------------------------------------------------//

void  GenDestroyField (HWND hWnd)

{
   FreeGenField (hWnd);
}

//--------------------------------------------------------------------------//
//--------------------------------------------------------------------------//

LPGENFIELD LockGenField (HWND hWnd)

{
   HANDLE hField;

   if (hField = GetWindowGenField (hWnd))
      return (LPGENFIELD) GlobalLock (hField);
   else
      return NULL;
}

//--------------------------------------------------------------------------//

void UnlockGenField (HWND hWnd)

{
   HANDLE hField;

   if (hField = GetWindowGenField (hWnd))
      GlobalUnlock (hField);
}

//--------------------------------------------------------------------------//

void FreeGenField (HWND hWnd)

{
   HANDLE hField;

   if (hField = GetWindowGenField (hWnd))
      GlobalFree (hField);
}


void GenLimitText(HWND hWnd, WPARAM wParam)
{
LPEDITFIELD lpField;

lpField = LockField(hWnd);

if (wParam <= 0)
   lpField->nMaxChars = -1;
else
   lpField->nMaxChars = (short)min(wParam, MAXFIELD);

UnlockField (hWnd);
}
