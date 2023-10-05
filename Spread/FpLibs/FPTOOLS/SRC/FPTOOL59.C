/*    PortTool v2.2     FPTOOL59.C          */

// 
// FarPoint FontTable Manager
//

#include "fptools.h"
#include "string.h"

int FPLIB fpGetFontLineHeight (FPFONT fpFont)
{
   LPFPFONTENTRY lpFontTable;       
   int  lineHeight = 0;
   UINT fuSize = 0;
   
   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         lineHeight = lpFontTable[fpFont].tm.tmHeight + 
            lpFontTable[fpFont].tm.tmExternalLeading;
      fpUnlockGlobalData (&dmFontTable, NULL);
      }

   return lineHeight;
}

int FPLIB fpGetFontLineHeightEx (HINSTANCE hInst, FPFONT fpFont)
{
   LPFPFONTENTRY lpFontTable;       
   int  lineHeight = 0;
   UINT fuSize = 0;
   
   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         lineHeight = lpFontTable[fpFont].tm.tmHeight + 
            lpFontTable[fpFont].tm.tmExternalLeading;
      fpUnlockGlobalData (&dmFontTable, hInst);
      }

   return lineHeight;
}
