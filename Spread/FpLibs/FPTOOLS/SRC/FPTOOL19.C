/*    PortTool v2.2     FPTOOL19.C          */

#include "fptools.h"

void FPLIB fpAdjustRect (UINT uMargin, LPRECT rect)
{
   rect->left   += fpLM (uMargin);
   rect->right  -= fpRM (uMargin);
   rect->top    += fpTM (uMargin);
   rect->bottom -= fpBM (uMargin);
   return;
}


LPPOINT FPLIB fpMakePoint( LPPOINT lpPoint, DWORD dwXY)
{
  lpPoint->x = (int)LOWORD(dwXY);
  lpPoint->y = (int)HIWORD(dwXY);

  return lpPoint;
}