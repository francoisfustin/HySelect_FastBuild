/*
$Revision:   1.3  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/OVERMODE.C_V  $
 * 
 *    Rev 1.3   12 Jun 1990 14:08:28   Dirk
 * No change.
 * 
 *    Rev 1.2   29 May 1990  9:09:24   Randall
 * Made external objects "static".
 * 
 *    Rev 1.1   28 May 1990 13:49:54   Randall
 * 
 *    Rev 1.0   25 May 1990 15:48:48   Randall
 * Initial revision.
*/

#define  NOCOMM
#define  NOKANJI
#define  NOMINMAX

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <toolbox.h>
#include "..\libgen.h"
#include "editfld.h"

//static BOOL  bLocalOverstrike;   // True if this field is in OVERSTRIKE mode.
static BOOL  bGlobalOverstrike;  // True if the screen is in OVERSTRIKE mode.
                                 //  (This provides the default if mode is not 
                                 //      specified for a particular field).

//--------------------------------------------------------------------------//

void SetOverstrikeMode (HWND hWnd, BOOL bGlobal)

{
   LPEDITFIELD lpField = LockField (hWnd);

   lpField->bLocalOverstrike = TRUE;
   if (bGlobal) bGlobalOverstrike = TRUE;

   if (GetFocus() == hWnd)       // This field has the focus
      CreateModeCaret (hWnd, lpField);

   UnlockField (hWnd);
}

//--------------------------------------------------------------------------//

void SetInsertMode (HWND hWnd, BOOL bGlobal)

{
   LPEDITFIELD lpField = LockField (hWnd);

   lpField->bLocalOverstrike = FALSE;
   if (bGlobal) bGlobalOverstrike = FALSE;

   if (GetFocus() == hWnd)       // This field has the focus
      CreateModeCaret (hWnd, lpField);

   UnlockField (hWnd);
}

//--------------------------------------------------------------------------//

BOOL GetOverstrikeMode (HWND hWnd, BOOL bGlobal)

{
   LPEDITFIELD lpField = LockField (hWnd);
   BOOL        fRet;

   fRet = (bGlobal ? bGlobalOverstrike : lpField->bLocalOverstrike);

   UnlockField (hWnd);
   return (fRet);
}
