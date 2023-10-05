/*    PortTool v2.2     FPTOOL53.C          */

// 
// FarPoint FontTable Manager
//

#include "fptools.h"
#include "string.h"


BOOL FPLIB fpGetFontTextMetric (FPFONT fpFont, TEXTMETRIC FAR *lptm)
{
   LPFPFONTENTRY lpFontTable;
   BOOL rValue = FALSE;
   UINT fuSize = 0;

   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         {
         _fmemcpy(lptm, &lpFontTable[fpFont].tm, sizeof(TEXTMETRIC));
         rValue = TRUE;
         }
      fpUnlockGlobalData (&dmFontTable, NULL);
      }

   return rValue;
}

BOOL FPLIB fpGetFontTextMetricEx (HINSTANCE hInst, FPFONT fpFont, TEXTMETRIC FAR *lptm)
{
   LPFPFONTENTRY lpFontTable;
   BOOL rValue = FALSE;
   UINT fuSize = 0;

   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize)))
      {
      if ((UINT)fpFont < fuSize) 
         {
         _fmemcpy(lptm, &lpFontTable[fpFont].tm, sizeof(TEXTMETRIC));
         rValue = TRUE;
         }
      fpUnlockGlobalData (&dmFontTable, hInst);
      }

   return rValue;
}

