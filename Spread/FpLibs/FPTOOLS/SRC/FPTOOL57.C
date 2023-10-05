/*    PortTool v2.2     FPTOOL57.C          */

// 
// FarPoint FontTable Manager
//

#include "fptools.h"
#include "string.h"

int FPLIB fpGetFontHeight (FPFONT fpFont)
{
   LPFPFONTENTRY lpFontTable;       
   short tmHeight = 0;
   UINT fuSize = 0;
   
   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         tmHeight = (short)lpFontTable[fpFont].tm.tmHeight;
      fpUnlockGlobalData (&dmFontTable, NULL);
      }

   return tmHeight;
}

int FPLIB fpGetFontHeightEx (HINSTANCE hInst, FPFONT fpFont)
{
   LPFPFONTENTRY lpFontTable;       
   short tmHeight = 0;
   UINT fuSize = 0;
   
   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         tmHeight = (short)lpFontTable[fpFont].tm.tmHeight;
      fpUnlockGlobalData (&dmFontTable, hInst);
      }

   return tmHeight;
}
