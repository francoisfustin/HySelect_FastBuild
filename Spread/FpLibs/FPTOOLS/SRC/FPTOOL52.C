/*    PortTool v2.2     FPTOOL52.C          */

// 
// FarPoint FontTable Manager
//

#include "fptools.h"
#include "string.h"


BOOL FPLIB fpGetFontLogFont (FPFONT fpFont, LOGFONT FAR *lpLogFont)
{
   LPFPFONTENTRY lpFontTable;
   BOOL rValue = FALSE;
   UINT fuSize = 0;

   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         {
         _fmemcpy(lpLogFont, &lpFontTable[fpFont].LogFont, sizeof(LOGFONT));
         rValue = TRUE;
         }
      fpUnlockGlobalData (&dmFontTable, NULL);
      }

   return rValue;
}

BOOL FPLIB fpGetFontLogFontEx (HINSTANCE hInst, FPFONT fpFont, LOGFONT FAR *lpLogFont)
{
   LPFPFONTENTRY lpFontTable;
   BOOL rValue = FALSE;
   UINT fuSize = 0;

   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         {
         _fmemcpy(lpLogFont, &lpFontTable[fpFont].LogFont, sizeof(LOGFONT));
         rValue = TRUE;
         }
      fpUnlockGlobalData (&dmFontTable, hInst);
      }

   return rValue;
}


void FPLIB fpGetHFontLogFont (HFONT hFont, LOGFONT FAR *lpLogFont)
{
   if (!hFont)
      hFont = GetStockObject(SYSTEM_FONT);

   GetObject (hFont, sizeof(LOGFONT), lpLogFont);

   return;
}
