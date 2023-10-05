/*    PortTool v2.2     FPTOOL54.C          */

// 
// FarPoint FontTable Manager
//

#include "fptools.h"
#include "string.h"


HFONT FPLIB fpGetFontHandle (FPFONT fpFont)
{
   return (fpLockFont(fpFont, NULL));
}

HFONT FPLIB fpGetFontHandleEx (HINSTANCE hInst, FPFONT fpFont)
{
   return (fpLockFontEx(hInst, fpFont, NULL));
}

