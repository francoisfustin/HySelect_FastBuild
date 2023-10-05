/*    PortTool v2.2     FPTOOL56.C          */

// 
// FarPoint FontTable Manager
//

#include "fptools.h"
#include "string.h"

int FPLIB fpGetCharWidth (FPFONT fpFont, TCHAR ch)
{
   LPCHARWIDTHS lpCharWidth;
   short width = 0; 
   
   if (fpFont != NOFONT && (lpCharWidth = fpLockCharWidths(fpFont)))
      {
#ifdef _UNICODE
      width = lpCharWidth[(unsigned short)ch];
#else
      width = lpCharWidth[(unsigned char)ch];
#endif
      fpUnlockCharWidths(fpFont);
      }
   
   return width;
}                      

int FPLIB fpGetCharWidthEx (HINSTANCE hInst, FPFONT fpFont, TCHAR ch)
{
   LPCHARWIDTHS lpCharWidth;
   short width = 0; 
   
   if (fpFont != NOFONT && (lpCharWidth = fpLockCharWidthsEx(hInst, fpFont)))
      {
#ifdef _UNICODE
      width = lpCharWidth[(unsigned short)ch];
#else
      width = lpCharWidth[(unsigned char)ch];
#endif
      fpUnlockCharWidthsEx(hInst, fpFont);
      }
   
   return width;
}                      


