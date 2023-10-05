/*    PortTool v2.2     FPTOOL22.C          */

#include "fptools.h"

#define ESX_VCENTER         0x00000080L
#define ESX_VBOTTOM         0x00000100L
#define ESX_VTOP            0x00000000L  

short FPLIB fpComputeVAlign (DWORD dwExtStyle, UINT yAvgFont, UINT uMargin, LPRECT r)
{
#if defined(_WIN64) || defined(_IA64)
   ULONG y1 = r->top    + fpTM(uMargin);
   ULONG y2 = r->bottom - fpBM(uMargin);
#else
   short y1 = (short)(r->top    + fpTM(uMargin));
   short y2 = (short)(r->bottom - fpBM(uMargin));
#endif
   if (fpIS_EXTSTYLE (dwExtStyle, ESX_VCENTER))
      y1 += (y2-y1-(short)yAvgFont) / 2;
   else if (fpIS_EXTSTYLE (dwExtStyle, ESX_VBOTTOM))
      y1 = (y2-(short)yAvgFont);

#if defined(_WIN64) || defined(_IA64)
   return (short)y1;
#else
   return y1;
#endif
}
