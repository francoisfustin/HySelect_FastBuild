/*    PortTool v2.2     FPTOOL47.C          */

//
// FarPoint DrawText						  
//

#include "fptools.h"
#include "string.h"

void FPLIB fpDrawFocusRect (HDC hdc, LPRECT lprc)
{
   static  WORD FAR Bits[] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};
   HBITMAP hBitmap = CreateBitmap(8, 8, 1, 1, (LPSTR)Bits);
   HBRUSH  hBrush  = CreatePatternBrush(hBitmap);
   RECT rc;
   
   CopyRect(&rc, lprc);
   /*
   rc.left--;
   rc.right++;
   rc.top;
   rc.bottom++;
   */
   
   FrameRect(hdc, &rc, hBrush);

   DeleteObject (hBitmap);
   DeleteObject (hBrush);

   return;
}

/*
// moved into Spread code -scl
void FPLIB fpGradientFill(HDC hdc, RECT rect, HBRUSH hBrushTop, HBRUSH hBrushBottom, int direction)
{

	GRADIENT_RECT gRect;
	TRIVERTEX vertex[2] ;
	LOGBRUSH logbrush1, logbrush2;
	GetObject(hBrushTop, sizeof(LOGBRUSH), (LPVOID)&logbrush1);
	GetObject(hBrushBottom, sizeof(LOGBRUSH), (LPVOID)&logbrush2);
	vertex[0].x     = rect.left;
	vertex[0].y     = rect.top;
	vertex[0].Red   = (GetRValue(logbrush1.lbColor) << 8);
	vertex[0].Green = (GetGValue(logbrush1.lbColor) << 8);
	vertex[0].Blue  = (GetBValue(logbrush1.lbColor) << 8);
	vertex[0].Alpha = 0x0000;

	vertex[1].x     = rect.right;
	vertex[1].y     = rect.bottom; 
	vertex[1].Red   = (GetRValue(logbrush2.lbColor) << 8);
	vertex[1].Green = (GetGValue(logbrush2.lbColor) << 8);
	vertex[1].Blue  = (GetBValue(logbrush2.lbColor) << 8);
	vertex[1].Alpha = 0x0000;

	// Create a GRADIENT_RECT structure that
	// references the TRIVERTEX vertices.
	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;

	// Draw a shaded rectangle.
	GradientFill(hdc, vertex, 2, &gRect, 1, direction);

}


void FPLIB fpAlphaFill(HDC hdc, RECT rect, HBRUSH hBrush, COLORREF clrHighlight, BYTE alpha)
{
	GRADIENT_RECT gRect;
	TRIVERTEX vertex[2] ;
	LOGBRUSH logbrush;
	int r1, g1, b1;
	int r2, g2, b2;
	int rNew, gNew, bNew;

	GetObject(hBrush, sizeof(LOGBRUSH), (LPVOID)&logbrush);

	r1 = GetRValue(logbrush.lbColor);
	g1 = GetGValue(logbrush.lbColor);
	b1 = GetBValue(logbrush.lbColor);

	r2 = GetRValue(clrHighlight);
	g2 = GetGValue(clrHighlight);
	b2 = GetBValue(clrHighlight);

	rNew = (r2 * alpha + r1 * (255 - alpha)) / 255;
	gNew = (g2 * alpha + g1 * (255 - alpha)) / 255;
	bNew = (b2 * alpha + b1 * (255 - alpha)) / 255;

	vertex[0].x     = rect.left;
	vertex[0].y     = rect.top;
	vertex[0].Red   = (rNew << 8);
	vertex[0].Green = (gNew << 8);
	vertex[0].Blue  = (bNew << 8);
	vertex[0].Alpha = 0;

	vertex[1].x     = rect.right;
	vertex[1].y     = rect.bottom; 
	vertex[1].Red   = vertex[0].Red;
	vertex[1].Green = vertex[0].Green;
	vertex[1].Blue  = vertex[0].Blue;
	vertex[1].Alpha = vertex[0].Alpha;

	// Create a GRADIENT_RECT structure that
	// references the TRIVERTEX vertices.
	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;

	// Draw a shaded rectangle.
	GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
}
*/