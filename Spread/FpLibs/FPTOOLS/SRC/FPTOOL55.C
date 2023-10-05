/*    PortTool v2.2     FPTOOL55.C          */

// 
// FarPoint FontTable Manager
//

#include "fptools.h"
#include "string.h"

#include "fptstr.h"

int FPLIB fpGetTextWidth (FPFONT fpFont, LPTSTR lpsz, short length)
{
   LPCHARWIDTHS lpCharWidth;                 
   short i, width = 0;
   UINT fuSize = 0;
      
   if ((lpsz != NULL) && (*lpsz != 0))
   {   
     if (fpFont != NOFONT && (lpCharWidth = fpLockCharWidths(fpFont)))
        {
        for (i=0; i<length; i++)
           {
           width += lpCharWidth[(unsigned char)*lpsz];
           lpsz = CharNext(lpsz);
           }
        fpUnlockCharWidths(fpFont);
        }
   }
   return width;
}

int FPLIB fpGetTextWidthEx (HINSTANCE hInst, FPFONT fpFont, LPTSTR lpsz, short length)
{
   LPCHARWIDTHS lpCharWidth;                 
   short i, width = 0;
   UINT fuSize = 0;
      
   if ((lpsz != NULL) && (*lpsz != 0))
   {   
     if (fpFont != NOFONT && (lpCharWidth = fpLockCharWidthsEx(hInst, fpFont)))
        {
        for (i=0; i<length; i++)
           {
#ifdef _UNICODE
           width += lpCharWidth[(unsigned)*lpsz];
           lpsz = CharNext(lpsz);
#endif
           }
        fpUnlockCharWidthsEx(hInst, fpFont);
        }
   }
   return width;
}

//A419 
BOOL fpTextEndsWithTab( LPTSTR lpsz, short length)
{
#define TAB_CHAR  0x09										 
   															 
   if (length == 0)
     return(FALSE);

   if (lpsz[length-1] == (TCHAR)TAB_CHAR)							
     return(TRUE);
   else
     return(FALSE);
}


//A419 		    
int FPLIB fpGetTabbedTextWidthForDC (HDC hdc, LPTSTR lpsz, short length,
                                        LPFPTABSTOPS lpTabstops)
{		    
   short    width = 0;
   DWORD    dwTabExtent;
   FPTABSTOPS  tmpTabstops = {0};
		    
   if ((lpsz != NULL) && (*lpsz != 0) && (length > 0))
      {      
     if (hdc)
        {   
		// Use Tabstops if they are set
		if (lpTabstops)
		   {
		   tmpTabstops.ct = lpTabstops->ct;
		   tmpTabstops.lpStops = lpTabstops->lpStops;
		   }

 		/**************************************************************/
        /* Since TabbedTextOut() was used to output text and that  SCP*/
		/* function expands a TAB to 8 "average char widths",    	  */
		/* GetTabbedTextExtent() must be used to calculate the 		  */
		/* text width.												  */
		/**************************************************************/
		dwTabExtent = 
		  GetTabbedTextExtent( hdc, (FPCTSTR)lpsz, length, 
		                       tmpTabstops.ct, tmpTabstops.lpStops);
		width = LOWORD( dwTabExtent);

        /***********************************************************/
		/* Subtract the Font overhang  	                           */
        /*                                                         */
		/* NOTE: Possible Windows bug with GetTabbedTextExtent():  */
		/*														   */
		/*   If text ends with a tab char, the width is short      */
		/*   by 1 "overhang".                                      */
        /***********************************************************/
		if (width > 0)
		   {
		   TEXTMETRIC tm;
		   if (GetTextMetrics(hdc, (TEXTMETRIC FAR *)&tm) &&
		       tm.tmOverhang &&
			  !fpTextEndsWithTab(lpsz, length) &&
		       (width >= tm.tmOverhang))
			width -= (short)tm.tmOverhang;
		   }

	    }
     }

   return width;
}


//A419 
int FPLIB fpGetTabbedTextWidth (FPFONT fpFont, LPTSTR lpsz, short length,
                                   HWND hwnd, LPFPTABSTOPS lpTabstops )
{
   short width = 0;
   HDC   hdc;   
   HFONT oldFont;

   if (fpFont != NOFONT)
      {
      hdc = GetDC( hwnd);					
      if (hdc)
         {
      										// select THIS font
         oldFont = SelectObject(hdc, fpGetFontHandle(fpFont));

         width = fpGetTabbedTextWidthForDC( hdc, lpsz, length, lpTabstops);

         SelectObject(hdc, oldFont);		// restore previous font
         ReleaseDC(hwnd, hdc);					
         }
      }	    
		    
   return width;
}

int FPLIB fpGetTabbedTextWidthEx (HINSTANCE hInst, FPFONT fpFont, LPTSTR lpsz, short length,
                                   HWND hwnd, LPFPTABSTOPS lpTabstops )
{
   short width = 0;
   HDC   hdc;   
   HFONT oldFont;

   if (fpFont != NOFONT)
      {
      hdc = GetDC( hwnd);					
      if (hdc)
         {
      										// select THIS font
         oldFont = SelectObject(hdc, fpGetFontHandleEx(hInst, fpFont));

         width = fpGetTabbedTextWidthForDC( hdc, lpsz, length, lpTabstops);

         SelectObject(hdc, oldFont);		// restore previous font
         ReleaseDC(hwnd, hdc);					
         }
      }	    
		    
   return width;
}


short FPLIB fpDBCSGetCharWidth(HDC hDC, LPTSTR lpPtr, LPCHARWIDTHS lpCharWidth, long lLastChar)
{
  short iRet = 0;

  if (lpPtr)
  {
#ifndef _UNICODE
    if (IsDBCSLeadByte((BYTE)*lpPtr))
	   {
       SIZE tmpSize;

       GetTextExtentPoint(hDC, lpPtr, 2, &tmpSize);
       iRet = (short)tmpSize.cx;
	   }
    else
#endif
      {
#ifndef _UNICODE
       int iChar = (int)(unsigned char)*lpPtr;
#else
       wint_t iChar = (wint_t)(unsigned)*lpPtr;
#endif

       /* RFW - 8/19/04 - 15057 - 14919 broke 15057
       // RFW - 7/25/04 - 14919
       if (!_istprint(iChar))
         {
         iRet = 0;
         // RFW - 8/13/04 - 15029
         if (iChar == 13) // CR
           {
           SIZE tmpSize;

           GetTextExtentPoint(hDC, _T("\r"), 1, &tmpSize);
           iRet = (short)tmpSize.cx;
           }
         }
       */
       if (iChar == 13) // CR
         {
         SIZE tmpSize;

         GetTextExtentPoint(hDC, _T("\r"), 1, &tmpSize);
         iRet = (short)tmpSize.cx;
         }
       else if (iChar > lLastChar)
	      {
         SIZE tmpSize;

			GetTextExtentPoint(hDC, lpPtr, 1, &tmpSize);
			iRet = (short)tmpSize.cx;
			}
       else if (!lpCharWidth)
         {
         int iLen;
			GetCharWidth(hDC, iChar, iChar, &iLen);
         iRet = iLen;
         }
       else
         iRet = lpCharWidth[iChar];
	   }
  }
  
  return iRet;
}
