/*    PortTool v2.2     FPSTDPRP.C          */

#include "fptools.h"
#include "fpstdprp.h"

#include <string.h>
#include <math.h>

/**************************************************************************
*
* NOTE:  This module implements (in DLL's) some of the "Standard Properties"
*        found in VBX's.  The only reason that the VBPROPPARAMS structure
*        is used is to be consistent with the Non-Client properties.
*        DO NOT use the "hCtl" field from this structure except to check
*        for NULL/not NULL.
*
**************************************************************************/
//#define PT_TO_PIXELS(hDC, pt)   (short)((double)(pt) * (double)GetDeviceCaps(hDC, LOGPIXELSY) / 72.0);
//#define PIXELS_TO_PT(hDC, Pels) (float)((double)(Pels) * 72.0 / (double)GetDeviceCaps(hDC, LOGPIXELSY))


/**************************************************************************
*  Local Function Prototypes
**************************************************************************/
static void fpChangeFont( HWND hWnd, LOGFONT FAR * lpLogFont);


long FPLIB FP_Enabled (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   if (fGetProp)
      {
      *(BOOL FAR *)lpPP->lParam =
         (IsWindowEnabled(lpPP->hWnd) ? PROP_TRUE : 0);
      }
   else
      {
      EnableWindow( lpPP->hWnd, (BOOL)(lpPP->lParam) );
      }
   return 0;
}

long FPLIB FP_Visible (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   if (fGetProp)
      {
      *(BOOL FAR *)lpPP->lParam =
         (IsWindowVisible(lpPP->hWnd) ? PROP_TRUE : 0);
      }
   else
      {
      int nCmdShow;
      
      if ((BOOL)lpPP->lParam)
        nCmdShow = SW_RESTORE;
      else
        nCmdShow = SW_HIDE;
      
      ShowWindow( lpPP->hWnd, nCmdShow);
      }
   return 0;
}

long FPLIB FP_TabStop (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   long lStyle = GetWindowLong( lpPP->hWnd, GWL_STYLE);
   
   
   if (fGetProp)
      {
      *(BOOL FAR *)lpPP->lParam =
         ((lStyle & WS_TABSTOP) ? PROP_TRUE : 0);
      }
   else
      {
      if ((BOOL)lpPP->lParam)
        lStyle |= WS_TABSTOP;               //adding WS_TABSTOP
      else
        lStyle = (lStyle & ~WS_TABSTOP);    //removing WS_TABSTOP
        
      SetWindowLong( lpPP->hWnd, GWL_STYLE, lStyle);
      }
   return 0;
}


// Size & Position....

long FPLIB FP_Height (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
  RECT rect;
  
  GetWindowRect( lpPP->hWnd, &rect);
  
   if (fGetProp)
      {
      *(long FAR *)lpPP->lParam = (long)fpWIN_HEIGHT(rect);
      }
   else
      {
      if (lpPP->lParam > 0)
         {
         LPARAM lSize;
         
         SetWindowPos( lpPP->hWnd, NULL, 0, 0,  
                       fpWIN_WIDTH(rect)-1, (int)(lpPP->lParam)-1,
                       SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);
                       
         lSize = MAKELONG( fpWIN_WIDTH(rect), fpWIN_HEIGHT(rect));
                       
         SendMessage( lpPP->hWnd, WM_SIZE, SIZE_RESTORED, lSize);
         }
      }
   return 0;
}

long FPLIB FP_Width (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
  RECT rect;
  
  GetWindowRect( lpPP->hWnd, &rect);
  
   if (fGetProp)
      {
      *(long FAR *)lpPP->lParam = (long)fpWIN_WIDTH(rect);
      }
   else
      {
      if (lpPP->lParam > 0)
         {
         LPARAM lSize;
         
         SetWindowPos( lpPP->hWnd, NULL, 0, 0,  
                       (int)(lpPP->lParam)-1, fpWIN_HEIGHT(rect)-1,
                       SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);
         
         lSize = MAKELONG( fpWIN_WIDTH(rect), fpWIN_HEIGHT(rect));
                       
         SendMessage( lpPP->hWnd, WM_SIZE, SIZE_RESTORED, lSize);
         }
      }
   return 0;
}

long FPLIB FP_Left (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
  RECT rect;
  
  GetWindowRect( lpPP->hWnd, &rect);
  
   if (fGetProp)
      {
      *(long FAR *)lpPP->lParam = (long)rect.left;
      }
   else
      {
      if (lpPP->lParam >= 0)
         {
//         WindowMove()
         }
      }
   return 0;
}


long FPLIB FP_Top (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
  RECT rect;
  
  GetWindowRect( lpPP->hWnd, &rect);
  
   if (fGetProp)
      {
      *(long FAR *)lpPP->lParam = (long)rect.top;
      }
   else
      {
      if (lpPP->lParam >= 0)
         {
//         WindowMove()
         }
      }
   return 0;
}


// -----------------------------------------------------------------------
// Font Properties...
//

long FPLIB FP_FontBold (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
  HFONT hFont;
  LOGFONT LogFont;
  
  // Get the control's font and info about the font.
  hFont = (HFONT)SendMessage( lpPP->hWnd, WM_GETFONT, 0, 0L);
  fpGetHFontLogFont( hFont, &LogFont);
  
   if (fGetProp)                                
      {
      // If the Weight is greater than NORMAL (400), consider it BOLD.
      *(BOOL FAR *)lpPP->lParam =
         ((LogFont.lfWeight > FW_NORMAL) ? PROP_TRUE : 0);
      }
   else
      {
      int lfWeight;
      
      if (lpPP->lParam)
        lfWeight = FW_BOLD;
      else
        lfWeight = FW_NORMAL;
      
      // If the attribute actually changed.  
      if (LogFont.lfWeight != lfWeight)
         {
         // Change the attribute and change to the new font with these
         // attributes.
         LogFont.lfWeight = lfWeight;
         
         fpChangeFont( lpPP->hWnd, &LogFont);
         }
      } //setprop
      
   return 0;
}


long FPLIB FP_FontItalic (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
  HFONT hFont;
  LOGFONT LogFont;
  
  // Get the control's font and info about the font.
  hFont = (HFONT)SendMessage( lpPP->hWnd, WM_GETFONT, 0, 0L);
  fpGetHFontLogFont( hFont, &LogFont);
  
   if (fGetProp)                                
      {
      *(BOOL FAR *)lpPP->lParam = ((LogFont.lfItalic) ? PROP_TRUE : 0);
      }
   else
      {
      BOOL lfItalic = (lpPP->lParam != 0);
      
      // If the attribute actually changed.  
      if ((BOOL)LogFont.lfItalic != lfItalic)
         {
         // Change the attribute and change to the new font with these
         // attributes.
         LogFont.lfItalic = (BYTE)lfItalic;
         
         fpChangeFont( lpPP->hWnd, &LogFont);
         }
      } //setprop
      
   return 0;
}


long FPLIB FP_FontStrikeThru (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
  HFONT hFont;
  LOGFONT LogFont;
  
  // Get the control's font and info about the font.
  hFont = (HFONT)SendMessage( lpPP->hWnd, WM_GETFONT, 0, 0L);
  fpGetHFontLogFont( hFont, &LogFont);
  
   if (fGetProp)                                
      {
      *(BOOL FAR *)lpPP->lParam = ((LogFont.lfStrikeOut) ? PROP_TRUE : 0);
      }
   else
      {
      BOOL lfStrikeOut = (lpPP->lParam != 0);
      
      // If the attribute actually changed.  
      if ((BOOL)LogFont.lfStrikeOut != lfStrikeOut)
         {
         // Change the attribute and change to the new font with these
         // attributes.
         LogFont.lfStrikeOut = (BYTE)lfStrikeOut;
         
         fpChangeFont( lpPP->hWnd, &LogFont);
         }
      } //setprop
      
   return 0;
}


long FPLIB FP_FontUnderline (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
  HFONT hFont;
  LOGFONT LogFont;
  
  // Get the control's font and info about the font.
  hFont = (HFONT)SendMessage( lpPP->hWnd, WM_GETFONT, 0, 0L);
  fpGetHFontLogFont( hFont, &LogFont);
  
   if (fGetProp)                                
      {
      *(BOOL FAR *)lpPP->lParam = ((LogFont.lfUnderline) ? PROP_TRUE : 0);
      }
   else
      {
      BOOL lfUnderline = (lpPP->lParam != 0);
      
      // If the attribute actually changed.  
      if ((BOOL)LogFont.lfUnderline != lfUnderline)
         {
         // Change the attribute and change to the new font with these
         // attributes.
         LogFont.lfUnderline = (BYTE)lfUnderline;
         
         fpChangeFont( lpPP->hWnd, &LogFont);
         }
      } //setprop
      
   return 0;
}


long FPLIB FP_FontSize (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
  HFONT   hFont;
  LOGFONT LogFont;
  HDC     hdc = 0;
  
  // Get the control's font.
  hFont = (HFONT)SendMessage( lpPP->hWnd, WM_GETFONT, 0, 0L);
  
  // Set the mapping mode to TWIPS for Logical-to-Point unit conversion.
  hdc = GetDC(lpPP->hWnd);
  
  // Get the control's font and info about the font.
  fpGetHFontLogFont( hFont, &LogFont);
  
  if (fGetProp)                                
     {
     TEXTMETRIC TextMetric;
     HFONT      hFontOld;
     
     hFontOld = SelectObject(hdc, hFont);
     GetTextMetrics(hdc, &TextMetric);
     
     *(float FAR *)lpPP->lParam = 
         PIXELS_TO_PT((TextMetric.tmHeight -
                       TextMetric.tmInternalLeading));
     SelectObject(hdc, hFontOld);
     }
  else
     {
     int lfHeight = -PT_TO_PIXELS(*(float FAR *)lpPP->lParam);
     
     // If the attribute actually changed.  
     if (LogFont.lfHeight != lfHeight)
        {
        // Change the attribute and change to the new font with these
        // attributes.
        LogFont.lfHeight = lfHeight;
        LogFont.lfWidth = 0;
        
        fpChangeFont( lpPP->hWnd, &LogFont);
        }
     } //setprop
     
  // Undo above
  ReleaseDC(lpPP->hWnd, hdc);
     
  return 0;
}

long FPLIB FP_FontName (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
  HFONT hFont;
  LOGFONT LogFont;
  LPTSTR lpsz = (LPTSTR)(lpPP->lParam);
  
  // NOTE:  lpPP->lParam .... LPSTR fontName (or buffer to put name)
  //        lpPP->wParam .... int bufferSize (when getting the name)
   
  
  // Get the control's font and info about the font.
  hFont = (HFONT)SendMessage( lpPP->hWnd, WM_GETFONT, 0, 0L);
  fpGetHFontLogFont( hFont, &LogFont);
  
   if (fGetProp)                                
      {
      int len = (int)lstrlen(LogFont.lfFaceName);
      
      // if the FontName will NOT fit in the buffer, return error.
      if ((len + 1) > (int)(lpPP->wParam))
        return( -1L);
        
      if (len > 0)
        _fmemcpy( lpsz, LogFont.lfFaceName, len * sizeof(TCHAR));
        
      // null-terminate string.
      lpsz[len] = (TCHAR)'\0';
      }
   else
      {
      BOOL lfUnderline = (lpPP->lParam != 0);
      
      // If the name actually changed.  
      if ( lstrcmp(LogFont.lfFaceName, lpsz) != 0)
         {
         // Change the name and change to the new font with these
         // attributes.
         if (lstrlen(lpsz)+1 <= STRING_SIZE(LogFont.lfFaceName))
            {
            lstrcpy( LogFont.lfFaceName, lpsz);
            LogFont.lfOutPrecision = OUT_TT_PRECIS;
            
            fpChangeFont( lpPP->hWnd, &LogFont);
            }
         }
      } //setprop
      
   return 0;
}



static void fpChangeFont( HWND hWnd, LOGFONT FAR * lpLogFont)
{
  BOOL  bRedraw;
  HFONT hFont;
  
  // Change the font.
  hFont = CreateFontIndirect( lpLogFont);
  
  // Determine if we should redraw
  bRedraw = (BOOL)SendMessage( hWnd, FPM_REDRAW, REDRAW_QUERY, 0L);
    
  // Set the new font.

/*    PortTool v2.2     5/10/1995    20:40          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
  SendMessage( hWnd, WM_SETFONT, (WPARAM)hFont, (LPARAM)bRedraw); 
  
  DeleteObject(hFont);  //km 12-17-97
}

