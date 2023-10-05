/*    PortTool v2.2     FPTOOL58.C          */

// 
// FarPoint FontTable Manager
//

#include "fptools.h"
#include "string.h"

int FPLIB fpGetFontOverhang (FPFONT fpFont)
{
   LPFPFONTENTRY lpFontTable;       
   short tmOverhang = 0;
   UINT fuSize = 0;
   
   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         tmOverhang = (short)lpFontTable[fpFont].tm.tmOverhang;
      fpUnlockGlobalData (&dmFontTable, NULL);
      }

   return tmOverhang;
}

int FPLIB fpGetFontOverhangEx (HINSTANCE hInst, FPFONT fpFont)
{
   LPFPFONTENTRY lpFontTable;       
   short tmOverhang = 0;
   UINT fuSize = 0;
   
   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         tmOverhang = (short)lpFontTable[fpFont].tm.tmOverhang;
      fpUnlockGlobalData (&dmFontTable, hInst);
      }

   return tmOverhang;
}

