/*    PortTool v2.2     FPTOOL41.C          */

//
// FarPoint DrawText
//

#include "fptools.h"
#include "string.h"
#include <tchar.h>


void  FPLIB fpSelTextOut (HDC hdc, int x, int y, UINT fuOptions,
         LPRECT lprc, LPTSTR lpsz, UINT cbString, LPINT lpDx, 
         LPTEXT3DINFO lp3d, short nFirst, short nLast,
         COLORREF colorText, COLORREF colorBack)
{
  // Call generic function without Tab support
  fpGenSelTextOut (hdc, x, y, fuOptions, lprc, lpsz, cbString, lpDx, 
                   lp3d, nFirst, nLast, colorText, colorBack,
                   NULL  /*No Tab support*/ );
  return;
}

void  FPLIB fpSelTabbedTextOut (HDC hdc, int x, int y, UINT fuOptions,
         LPRECT lprc, LPTSTR lpsz, UINT cbString, LPINT lpDx, 
         LPTEXT3DINFO lp3d, short nFirst, short nLast,
         COLORREF colorText, COLORREF colorBack)
{
  FPTABSTOPS tmpTabstops = {0};

  // Call generic function with Tab support
  fpGenSelTextOut (hdc, x, y, fuOptions, lprc, lpsz, cbString, lpDx, 
                   lp3d, nFirst, nLast, colorText, colorBack,
                   &tmpTabstops  /*Tab support*/ );
  return;
}

// Generic version of fpSelTextOut (supports Text w/ & w/o tabs)
void  FPLIB fpGenSelTextOut (HDC hdc, int x, int y, UINT fuOptions,
         LPRECT lprc, LPTSTR lpsz, UINT cbString, LPINT lpDx, 
         LPTEXT3DINFO lp3d, short nFirst, short nLast,
         COLORREF colorText, COLORREF colorBack, 
         LPFPTABSTOPS lpTabstops)
{
   UINT  fuOpt = (fuOptions & (ETO_CLIPPED | ETO_OPAQUE));
   COLORREF oldBackColor, oldTextColor;
   UINT  fuOldBkMode, fuOldTA;
   TEXTMETRIC tm;
   short len, i;
   BOOL  bTabSupp = (lpTabstops != NULL);							  //A419
   
   if (!lpsz)
      return;
      
   GetTextMetrics (hdc, &tm);
   
   if (nFirst < -1 || nFirst == nLast)
      fpGen3dTextOut (hdc, x, y, fuOpt, lprc, lpsz, cbString,    //A419  
                      lpDx, lp3d, lpTabstops, x);				 //A419 
   else if (nLast < -1)
      {
      // the whole string is selected
      //
      if (colorBack & 0x80000000)
         colorBack = GetSysColor ((int)(colorBack & 0x7FFFFFFF));
      if (colorText & 0x80000000)
         colorText = GetSysColor ((int)(colorText & 0x7FFFFFFF));
      oldTextColor = SetTextColor (hdc, colorText);
      oldBackColor = SetBkColor   (hdc, colorBack);

      fuOldBkMode = SetBkMode (hdc, OPAQUE);
      fpGen3dTextOut  (hdc, x, y, fuOpt, lprc, lpsz, cbString,	 //A419 
                       lpDx, NULL, lpTabstops, x);				 //A419 
      SetBkMode (hdc, fuOldBkMode);
      
      oldTextColor = SetTextColor (hdc, oldTextColor);
      oldBackColor = SetBkColor   (hdc, oldBackColor);
      }
   else
      {
      LPTSTR lpszTemp = lpsz;
      POINT pt;
      LPTSTR lpszFirst = lpsz;                                    //A419 

      if (bTabSupp)                                   //A419  
	     {
    	 pt.y = y;								      //A419 
	     pt.x = x;	     							  //A419  
         fuOldTA = SetTextAlign(hdc, TA_NOUPDATECP);  //A419 
      	 }
	  else
	     {
         fuOldTA = SetTextAlign(hdc, TA_UPDATECP);
         MoveToEx(hdc, x, y, NULL);
      	 }

      // print text left of selection
      i = 0;
      while (*lpszTemp && cbString && i < nFirst)
         {
         i++;           
         cbString--;                                     
         lpszTemp = CharNext(lpszTemp);
         }
      len = i;
      if (len)
         {
         fpGen3dTextOut (hdc, x, y, fuOpt, lprc, lpsz,         	   //A419 
                         len, lpDx, lp3d, lpTabstops, x);          //A419 
		 if (bTabSupp)									    	   //A419 
		    {
     		pt.x = x + fpGetTabbedTextWidthForDC(hdc, lpszFirst, i, //A419
     		                                     lpTabstops);       //A419  
	        }
		 else
		    {
  			GetCurrentPositionEx (hdc, &pt);
  			//pt.x -= tm.tmOverhang; //GRB9123
  			MoveToEx(hdc, pt.x, pt.y, NULL);
            }
         }
         
      // print selection
      if (*lpszTemp && cbString)
         {
         lpsz = lpszTemp;
         while (*lpszTemp && cbString && i < nLast)
            {
            i++;           
            cbString--;                                     
            lpszTemp = CharNext (lpszTemp);
            }              
         len = (i - len);

         if (colorBack & 0x80000000)
            colorBack = GetSysColor ((int)(colorBack & 0x7FFFFFFF));
         if (colorText & 0x80000000)
            colorText = GetSysColor ((int)(colorText & 0x7FFFFFFF));
         oldTextColor = SetTextColor (hdc, colorText);
         oldBackColor = SetBkColor   (hdc, colorBack);

         fuOldBkMode = SetBkMode (hdc, OPAQUE);
         fpGen3dTextOut (hdc, pt.x, pt.y, fuOpt, lprc, lpsz, len,  //A419 
                         lpDx, NULL, lpTabstops, x); 			   //A419 

         SetBkMode (hdc, fuOldBkMode);
         
         oldTextColor = SetTextColor (hdc, oldTextColor);
         oldBackColor = SetBkColor   (hdc, oldBackColor);
         }

      // print text right of selection
      if (*lpszTemp && cbString)
         {
		 if (bTabSupp)											   //A419 
		    {
			// Calculate from beginning of all text.
     		pt.x = x + fpGetTabbedTextWidthForDC(hdc, lpszFirst, i, //A419  
     		                                     lpTabstops);       //A419  
			}
		 else
		    {
			GetCurrentPositionEx (hdc, &pt);
			//pt.x -= tm.tmOverhang;//GRB9123
			MoveToEx(hdc, pt.x, pt.y, NULL);
            }
         lpsz = lpszTemp;
         len = 0;
         while (*lpszTemp && cbString)
            {
            len++;           
            cbString--;                                      
            lpszTemp = CharNext(lpszTemp);
            }              
         fpGen3dTextOut (hdc, pt.x, pt.y, fuOpt, lprc, lpsz, len, //A419 
                         lpDx, lp3d, lpTabstops, x);	          //A419 
         }
      SetTextAlign(hdc, fuOldTA);
      }
             
   return;
}


