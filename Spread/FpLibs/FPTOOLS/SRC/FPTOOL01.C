/*    PortTool v2.2     FPTOOL01.C          */

#include "fptools.h"

void FPLIB fp3DTextInit (LPTEXT3DINFO lp3DText)
{
   lp3DText->bVOffset = 1;
   lp3DText->bHOffset = 1;
   lp3DText->colorShadow = 0x80000000 | COLOR_BTNSHADOW;
   lp3DText->colorHilite = 0x80000000 | COLOR_BTNFACE;
   lp3DText->idClrHiLite	= fpCM_AddItem(COLOR_INIT(COLOR_BTNFACE));
   lp3DText->idClrShadow	= fpCM_AddItem(COLOR_INIT(COLOR_BTNSHADOW));
   return;
}
