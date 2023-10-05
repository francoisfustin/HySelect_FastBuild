/*    PortTool v2.2     FPTOOL51.C          */

// 
// FarPoint FontTable Manager
//

#include "fptools.h"
#include "string.h"


LPCHARWIDTHS FPLIB fpLockCharWidths (FPFONT fpFont)
{
   int FAR *lpCharWidths = NULL;
   LPFPFONTENTRY lpFontTable;
   UINT fuSize = 0;

   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         lpCharWidths = (int FAR *)GlobalLock(lpFontTable[fpFont].hCharWidths);
      fpUnlockGlobalData (&dmFontTable, NULL);
      }

   return lpCharWidths;
}

LPCHARWIDTHS FPLIB fpLockCharWidthsEx (HINSTANCE hInst, FPFONT fpFont)
{
   int FAR *lpCharWidths = NULL;
   LPFPFONTENTRY lpFontTable;
   UINT fuSize = 0;

   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         lpCharWidths = (int FAR *)GlobalLock(lpFontTable[fpFont].hCharWidths);
      fpUnlockGlobalData (&dmFontTable, hInst);
      }

   return lpCharWidths;
}

void FPLIB fpUnlockCharWidths (FPFONT fpFont)
{
   int FAR *lpCharWidths = NULL;
   LPFPFONTENTRY lpFontTable;
   UINT fuSize = 0;

   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         GlobalUnlock(lpFontTable[fpFont].hCharWidths);
      fpUnlockGlobalData (&dmFontTable, NULL);
      }
}

void FPLIB fpUnlockCharWidthsEx (HINSTANCE hInst, FPFONT fpFont)
{
   int FAR *lpCharWidths = NULL;
   LPFPFONTENTRY lpFontTable;
   UINT fuSize = 0;

   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         GlobalUnlock(lpFontTable[fpFont].hCharWidths);
      fpUnlockGlobalData (&dmFontTable, hInst);
      }
}

