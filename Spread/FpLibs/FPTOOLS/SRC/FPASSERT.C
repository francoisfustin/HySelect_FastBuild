/*    PortTool v2.2     FPASSERT.C          */

#include "windows.h"
//#include "fpassert.h"
#include <tchar.h>

void PASCAL fp_assert (LPSTR exp, LPSTR file, unsigned line)
{
   TCHAR s[256];
   wsprintf (s, _T("%s %s %d"), exp, file, line);
   MessageBox (NULL, _T("Assert!"), s, MB_SYSTEMMODAL | MB_ICONSTOP | MB_OK);
   DebugBreak();
}

