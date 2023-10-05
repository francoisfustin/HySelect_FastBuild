/*    PortTool v2.2     FPTOOL43.C          */

//
// FarPoint DrawText
//

#include "fptools.h"
#include "string.h"

BOOL fpx_ExtTextOut(HDC hdc, int X, int Y, UINT fuOptions, CONST RECT *lprc,
                    LPCTSTR lpString, UINT cbCount, CONST INT *lpDx);

// 3D TextOut: internal function w/o parameter validation etc.
//
 
void FPLIB fp3dTextOut (HDC hdc, int x, int y, UINT fuOpt, 
   LPRECT lprc, LPCTSTR lpsz, UINT cbString, LPINT lpDx,
   LPTEXT3DINFO lp3d)                  
{
   fpGen3dTextOut (hdc, x, y, fuOpt, lprc, lpsz, cbString, lpDx, lp3d,
                   NULL,  //No tab support
                   0);    //Tab origin (don't care)                  
   return;
}


/*************************************************************************/
/*																		 */
/* FUNCTION:  fpGen3dTextOut()  -  Generic 3-D Text Output function.     */
/*																		 */
/* NOTE:  The parameter, lpTabstops, determines if tabs are expanded and */
/*        at what "stops":												 */
/*																		 */
/*          a. If NULL ptr, tabs are not expanded						 */
/*			b. If points to structure filled with 0's, the default stops */
/*               are used.												 */
/*			c. Otherwise the tab stops in the structure are used.		 */
/*																		 */
/*************************************************************************/
void FPLIB fpGen3dTextOut (HDC hdc, int x, int y, UINT fuOpt, 
   LPRECT lprc, LPCTSTR lpsz, UINT cbString, LPINT lpDx, LPTEXT3DINFO lp3d, 
   LPFPTABSTOPS lpTabstops, int tabOrigin)                           //A419
{
   UINT fuOldBkMode = 0;
   BOOL  bTabSupp = (lpTabstops != NULL);							  //A419

   //fuOpt &= ~ETO_CLIPPED;  //km 6/26/96 for multiline text clipping

   if (lp3d && lp3d->bStyle)
      {
      UINT fuOldTA = GetTextAlign(hdc);
      COLORREF clrSave = GetTextColor(hdc);
      LONG cShadow = -1, cHiLight = -1;
               
      fuOldBkMode = SetBkMode(hdc, TRANSPARENT);
      if (fuOldTA & TA_UPDATECP)
         {
         POINT point;
         GetCurrentPositionEx (hdc, &point);
         x = point.x, y = point.y;
         SetTextAlign(hdc, TA_NOUPDATECP);
         }
      switch (lp3d->bStyle)
         {
         case TEXT3D_MOREUP:
            cHiLight = (lp3d->idClrShadow ? FPCOLOR(fpCM_GetItem(lp3d->idClrShadow, NULL, NULL)) : FPCOLOR(lp3d->colorShadow));
         case TEXT3D_UP:                          
            cShadow = (lp3d->idClrHiLite ? FPCOLOR(fpCM_GetItem(lp3d->idClrHiLite, NULL, NULL)) : FPCOLOR(lp3d->colorHilite));
            break;
         case TEXT3D_MOREDOWN:
            cShadow = (lp3d->idClrShadow ? FPCOLOR(fpCM_GetItem(lp3d->idClrShadow, NULL, NULL)) : FPCOLOR(lp3d->colorShadow));
         case TEXT3D_DOWN:
            cHiLight = (lp3d->idClrHiLite ? FPCOLOR(fpCM_GetItem(lp3d->idClrHiLite, NULL, NULL)) : FPCOLOR(lp3d->colorHilite));
            break;
         }
               
      // lower
      if (cShadow >= 0)
         {
         SetTextColor (hdc, (COLORREF)cShadow);
		 if (bTabSupp)												 //A419 
	        TabbedTextOut (hdc, x+lp3d->bHOffset, y+lp3d->bVOffset,	 //A419  
	                       lpsz, cbString, 
	                       lpTabstops->ct, lpTabstops->lpStops,	     //A419
	                       tabOrigin+lp3d->bHOffset);                //A419  
         else														 //A419 
            fpx_ExtTextOut (hdc, x+lp3d->bHOffset, y+lp3d->bVOffset,
                            fuOpt, lprc, lpsz, cbString, lpDx);
               }
      // upper 
      if (cHiLight >= 0)
         {
         SetTextColor (hdc, (COLORREF)cHiLight);
		 if (bTabSupp)												 //A419 
            TabbedTextOut (hdc, x-lp3d->bHOffset, y-lp3d->bVOffset,  //A419 
                           lpsz, cbString,      					 //A419 
	                       lpTabstops->ct, lpTabstops->lpStops,	     //A419
                           tabOrigin-lp3d->bHOffset);                //A419  
         else														 //A419 
            fpx_ExtTextOut (hdc, x-lp3d->bHOffset, y-lp3d->bVOffset, 
                            fuOpt, lprc, lpsz, cbString, lpDx);
         }
      SetTextColor (hdc, clrSave);
      SetTextAlign (hdc, fuOldTA);
      MoveToEx(hdc, x, y, NULL);
      }
 
   if (bTabSupp)													 //A419 
      TabbedTextOut (hdc, x, y, lpsz, cbString, 
	                 lpTabstops->ct, lpTabstops->lpStops, tabOrigin);//A419 
   else                                                              //A419
      fpx_ExtTextOut (hdc, x, y, fuOpt, lprc, lpsz, cbString, lpDx);

   if (fuOldBkMode)
      SetBkMode (hdc, fuOldBkMode);
   return;
}


BOOL fpx_ExtTextOut(HDC hdc, int X, int Y, UINT fuOptions, CONST RECT *lprc,
                    LPCTSTR lpString, UINT cbCount, CONST INT *lpDx)
{
#ifdef _UNICODE
RECT Rect;
UINT uFormat = 0;
int  iBkMode;

// RFW - 5/6/03 - 14192
//if (!(fuOptions & ETO_CLIPPED))
//	uFormat |= DT_NOCLIP;
uFormat |= DT_NOCLIP;
if (fuOptions & ETO_CLIPPED)
	IntersectClipRect(hdc, lprc->left, lprc->top, lprc->right, lprc->bottom);

if (!lprc)
	{
	Rect.right = X + 10;
	Rect.bottom = Y + 10;
	}
else
	Rect = *lprc;

Rect.left = X;
Rect.top = Y;

if (fuOptions & ETO_OPAQUE)
	iBkMode = SetBkMode(hdc, OPAQUE);
else
	iBkMode = SetBkMode(hdc, TRANSPARENT);

if (fuOptions & ETO_RTLREADING)
	uFormat |= DT_RTLREADING;

uFormat |= DT_LEFT | DT_TOP | DT_NOPREFIX | DT_SINGLELINE;

DrawText(hdc, lpString, cbCount, &Rect, uFormat);

SetBkMode(hdc, iBkMode);
return TRUE;
#else
return ExtTextOut(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
#endif // _UNICODE
}
