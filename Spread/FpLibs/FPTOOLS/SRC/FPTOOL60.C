/*    PortTool v2.2     FPTOOL60.C          */

// 
// FarPoint FontTable Manager
//

#include "fptools.h"
#include "string.h"


int FPLIB fpGetFontAscent (FPFONT fpFont)
{
   LPFPFONTENTRY lpFontTable;       
   int  ascent = 0;
   UINT fuSize = 0;
   
   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         ascent = lpFontTable[fpFont].tm.tmAscent;
      fpUnlockGlobalData (&dmFontTable, NULL);
      }

   return ascent;
}

int FPLIB fpGetFontAscentEx (HINSTANCE hInst, FPFONT fpFont)
{
   LPFPFONTENTRY lpFontTable;       
   int  ascent = 0;
   UINT fuSize = 0;
   
   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         ascent = lpFontTable[fpFont].tm.tmAscent;
      fpUnlockGlobalData (&dmFontTable, hInst);
      }

   return ascent;
}
