/*
$Revision:   1.4  $
*/

/*
$Log:   F:/DRLIBS/TOOLBOX/EDIT/USERFUNC.C_V  $
 * 
 *    Rev 1.4   12 Jun 1990 14:08:22   Dirk
 * No change.
 * 
 *    Rev 1.3   07 Jun 1990 11:13:12   Bruce
 * 
 *    Rev 1.2   21 May 1990 13:25:32   Randall
 * 
 *    Rev 1.1   15 May 1990 17:03:46   Sonny
 * Added GetEditClassProc
 * 
 *    Rev 1.0   11 May 1990 16:10:48   Bruce
 * Initial revision.
 * 
 *    Rev 1.2   30 Apr 1990 13:20:18   Bruce
 * 
 *    Rev 1.1   23 Apr 1990 20:09:36   Sonny
 * 
 *    Rev 1.0   23 Apr 1990 18:40:24   Sonny
 * Initial revision.
*/

#define  NOCOMM             // COMM driver routines
#define  NOKANJI            // Kanji support stuff.

#define  DRV_EDIT       // to include our struct defns and func defns

#include <windows.h>
#include <toolbox.h>

#include "..\libgen.h"

FARPROC DLLENTRY GetTBEditClassProc(void)
{
return ((FARPROC)EditFieldWndProc);
}

