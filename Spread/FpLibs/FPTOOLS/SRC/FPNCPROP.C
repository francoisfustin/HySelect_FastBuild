/*    PortTool v2.2     FPNCPROP.C          */

#include "fptools.h"
#include "fpncpnt.h"
#include "fpncprop.h"

/**************************************************************************
*
* NOTE:  This module supports VBX's and DLL's, even though the 
*        VBPROPPARAMS structure is used.  DO NOT use the "hCtl" field
*        from this structure except to check for NULL.
*
**************************************************************************/


long FPLIB FP_3DInsideStyle (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);

   if (fGetProp)
      {
      switch (NC_GET3DINSIDESTYLE (ncInfo))
         {
         case FP_NC3DINLOWER:
            *(BOOL FAR *)lpPP->lParam = 1;
            break;
         case FP_NC3DINRAISE:
            *(BOOL FAR *)lpPP->lParam = 2;
            break;
         default:
            *(BOOL FAR *)lpPP->lParam = 0;
            break;
         }
      }
   else
      {
      long lStyleOld = ncInfo.lStyle;

      switch ((int)lpPP->lParam)
         {
         case 1:
            ncInfo.lStyle = NC_SET3DINSIDESTYLE (ncInfo, FP_NC3DINLOWER);
            break;
         case 2:
            ncInfo.lStyle = NC_SET3DINSIDESTYLE (ncInfo, FP_NC3DINRAISE);
            break;
         default:
            ncInfo.lStyle = NC_SET3DINSIDESTYLE (ncInfo, 0);
            break;
         }

      if (lStyleOld != ncInfo.lStyle)
         ncInfo.bAppearance = 0;

      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

long FPLIB FP_3DOutsideStyle (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);

   if (fGetProp)
      {
      switch (NC_GET3DOUTSIDESTYLE (ncInfo))
         {
         case FP_NC3DOUTLOWER:
            *(BOOL FAR *)lpPP->lParam = 1;
            break;
         case FP_NC3DOUTRAISE:
            *(BOOL FAR *)lpPP->lParam = 2;
            break;
         default:
            *(BOOL FAR *)lpPP->lParam = 0;
            break;
         }
      }
   else
      {
      long lStyleOld = ncInfo.lStyle;

      switch ((int)lpPP->lParam)
         {
         case 1:
            ncInfo.lStyle = NC_SET3DOUTSIDESTYLE (ncInfo, FP_NC3DOUTLOWER);
            break;
         case 2:
            ncInfo.lStyle = NC_SET3DOUTSIDESTYLE (ncInfo, FP_NC3DOUTRAISE);
            break;
         default:
            ncInfo.lStyle = NC_SET3DOUTSIDESTYLE (ncInfo, 0);
            break;
         }

      if (lStyleOld != ncInfo.lStyle)
         ncInfo.bAppearance = 0;

      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}
            

long FPLIB FP_3DInsideHighlightColor (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
      *(long FAR *)lpPP->lParam = fpCM_GetItem(ncInfo.idClr3DInHiLite, NULL, NULL);
     //*(long FAR *)lpPP->lParam = ncInfo.color3DInHiLite;
      }
   else
      {
      fpCM_FreeItem(ncInfo.idClr3DInHiLite);
      ncInfo.idClr3DInHiLite = fpCM_AddItem((COLORREF)lpPP->lParam);
      ncInfo.color3DInHiLite = fpCM_GetItem(ncInfo.idClr3DInHiLite,NULL,NULL);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

long FPLIB FP_3DInsideShadowColor (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
      *(long FAR *)lpPP->lParam = fpCM_GetItem(ncInfo.idClr3DInShadow, NULL, NULL);
//	  *(long FAR *)lpPP->lParam = ncInfo.color3DInShadow;
      }
   else
      {
      fpCM_FreeItem(ncInfo.idClr3DInShadow);
      ncInfo.idClr3DInShadow = fpCM_AddItem((COLORREF)lpPP->lParam);
      ncInfo.color3DInShadow = fpCM_GetItem(ncInfo.idClr3DInShadow, NULL, NULL);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

long FPLIB FP_3DInsideShadowSize (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
      *(int FAR *)lpPP->lParam = ncInfo.bInShadowWidth;
      }
   else
      {
      ncInfo.bInShadowWidth = (BYTE)lpPP->lParam;
	  ncInfo.bAppearance=0;
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

long FPLIB FP_3DOutsideHighlightColor (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
      *(long FAR *)lpPP->lParam = fpCM_GetItem(ncInfo.idClr3DOutHiLite, NULL, NULL);
//      *(long FAR *)lpPP->lParam = ncInfo.color3DOutHiLite;
      }
   else
      {
      fpCM_FreeItem(ncInfo.idClr3DOutHiLite);
      ncInfo.idClr3DOutHiLite = fpCM_AddItem((COLORREF)lpPP->lParam);
      ncInfo.color3DOutHiLite = fpCM_GetItem(ncInfo.idClr3DOutHiLite, NULL, NULL);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

long FPLIB FP_3DOutsideShadowColor (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
      *(long FAR *)lpPP->lParam = fpCM_GetItem(ncInfo.idClr3DOutShadow, NULL, NULL);
//      *(long FAR *)lpPP->lParam = ncInfo.color3DOutShadow;
      }
   else
      {
      fpCM_FreeItem(ncInfo.idClr3DOutShadow);
      ncInfo.idClr3DOutShadow = fpCM_AddItem((COLORREF)lpPP->lParam);
      ncInfo.color3DOutShadow = fpCM_GetItem(ncInfo.idClr3DOutShadow, NULL, NULL);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

long FPLIB FP_3DOutsideShadowSize (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
      *(int FAR *)lpPP->lParam = ncInfo.bOutShadowWidth;
      }
   else
      {
      ncInfo.bOutShadowWidth = (BYTE)lpPP->lParam;
	  ncInfo.bAppearance=0;
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

long FPLIB FP_3DBevelSize (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
      *(int FAR *)lpPP->lParam = ncInfo.bFrameWidth;
      }
   else
      {
      BYTE bFrameWidth = ncInfo.bFrameWidth;

      ncInfo.bFrameWidth = (BYTE)lpPP->lParam;

      if (bFrameWidth != ncInfo.bFrameWidth)
         ncInfo.bAppearance = 0;

      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

long FPLIB FP_DropShadowColor (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
      *(long FAR *)lpPP->lParam = fpCM_GetItem(ncInfo.idColorDropShadow, NULL, NULL);
//      *(long FAR *)lpPP->lParam = ncInfo.colorDropShadow;
      }
   else
      {
      fpCM_FreeItem(ncInfo.idColorDropShadow);
      ncInfo.idColorDropShadow = fpCM_AddItem((COLORREF)lpPP->lParam);
      ncInfo.colorDropShadow = fpCM_GetItem(ncInfo.idColorDropShadow, NULL, NULL);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

long FPLIB FP_DropShadowSize (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
	   *(long FAR *)lpPP->lParam = ncInfo.bDropShadowWidth;
      }
   else
      {
      ncInfo.bDropShadowWidth = (BYTE)lpPP->lParam;
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

long FPLIB FP_BorderColor (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
      *(long FAR *)lpPP->lParam = fpCM_GetItem(ncInfo.idColorBorder, NULL, NULL);
//      *(long FAR *)lpPP->lParam = ncInfo.colorBorder;
      }
   else
      {
      fpCM_FreeItem(ncInfo.idColorBorder);
      ncInfo.idColorBorder = fpCM_AddItem((COLORREF)lpPP->lParam);
      ncInfo.colorBorder = fpCM_GetItem(ncInfo.idColorBorder, NULL, NULL);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

long FPLIB FP_NCBkColor (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
      *(long FAR *)lpPP->lParam = fpCM_GetItem(ncInfo.idColorBk, NULL, NULL);
//      *(long FAR *)lpPP->lParam = ncInfo.colorBk;
      }
   else
      {
      fpCM_FreeItem(ncInfo.idColorBk);
      ncInfo.idColorBk = fpCM_AddItem((COLORREF)lpPP->lParam);
      ncInfo.colorBk = fpCM_GetItem(ncInfo.idColorBk, NULL, NULL);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

long FPLIB FP_BorderSize (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
      *(int FAR *)lpPP->lParam = ncInfo.bBorderSize;
      }
   else
      {
	  BYTE bSize = ncInfo.bBorderSize;
      ncInfo.bBorderSize = (BYTE)lpPP->lParam;
      if (ncInfo.bBorderSize != bSize)
         ncInfo.bAppearance = 0;
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }

   return 0;
}

long FPLIB FP_BorderStyle (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
      if (ncInfo.lStyle & FP_NCBORDER)
         {
         if (ncInfo.lStyle & FP_NCROUND)
            *(int FAR *)lpPP->lParam = 2;
         else switch (ncInfo.bPenStyleOnFocus)
            {
            case PS_SOLID:
               *(int FAR *)lpPP->lParam = 1; break;
            case PS_DASH:
               *(int FAR *)lpPP->lParam = 3; break;
            case PS_DOT:
               *(int FAR *)lpPP->lParam = 4; break;
            case PS_DASHDOT:
               *(int FAR *)lpPP->lParam = 5; break;
            case PS_DASHDOTDOT:
               *(int FAR *)lpPP->lParam = 6; break;
            }
         }
      else
         *(int FAR *)lpPP->lParam = 0;
      }
   else
      {
      long lStyleOld = ncInfo.lStyle;
      BYTE bPenStyleOld = ncInfo.bPenStyleOnFocus;

      NC_SETSTYLE(ncInfo, FP_NCBORDER, (int)lpPP->lParam);
      NC_SETSTYLE(ncInfo, FP_NCROUND, ((int)lpPP->lParam == 2));
      switch ((int)lpPP->lParam)
         {
         case 1:
            ncInfo.bPenStyleOnFocus = PS_SOLID;
            ncInfo.bPenStyleNoFocus = PS_SOLID;
            break;
// JIS3607...
         case 2:
            ncInfo.bPenStyleOnFocus = PS_SOLID;
            ncInfo.bPenStyleNoFocus = PS_SOLID;
            break;
// JIS3607
         case 3:
            ncInfo.bPenStyleOnFocus = PS_DASH;       
            ncInfo.bPenStyleNoFocus = PS_DASH;       
            break;
         case 4:
            ncInfo.bPenStyleOnFocus = PS_DOT;        
            ncInfo.bPenStyleNoFocus = PS_DOT;        
            break;
         case 5:                                
            ncInfo.bPenStyleOnFocus = PS_DASHDOT;    
            ncInfo.bPenStyleNoFocus = PS_DASHDOT;    
            break;
         case 6:
            ncInfo.bPenStyleOnFocus = PS_DASHDOTDOT; 
            ncInfo.bPenStyleNoFocus = PS_DASHDOTDOT; 
            break;
         }

      if (lStyleOld != ncInfo.lStyle ||
          bPenStyleOld != ncInfo.bPenStyleOnFocus)
         ncInfo.bAppearance = 0;

      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}


// onfocus
long FPLIB FP_OnFocusInvert3D (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {               
      long lTemp = ncInfo.lStyle & FP_NC3DINVERT;                              //A262
//      *(short FAR *)lpPP->lParam = (lTemp ? -1 : 0);                              //A262
      *(long FAR *)lpPP->lParam = (lTemp ? PROP_TRUE : 0);                                 //A262
      }
   else
      {
      NC_SETSTYLE(ncInfo, FP_NC3DINVERT, lpPP->lParam);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

long FPLIB FP_OnFocusShadow (LPVBPROPPARAMS lpPP, BOOL fGetProp)           
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {    
        long lTemp = ncInfo.lStyle & FP_NCDROPSHADOWFOCUS;                   //A262
        *(long FAR *)lpPP->lParam =  (lTemp ? PROP_TRUE : 0);                             //A262
      }
   else
      {        
      NC_SETSTYLE(ncInfo, FP_NCDROPSHADOWFOCUS, lpPP->lParam);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }
   return 0;
}

// scroll bars
long FPLIB FP_ScrollBars (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   if (fGetProp)
      {
      BOOL fHScroll = 0;//VBWRTextGetStyle(lpPP->hWnd, FALSE, WS_HSCROLL);
      BOOL fVScroll = 0;//VBWRTextGetStyle(lpPP->hWnd, FALSE, WS_VSCROLL);
      if (fVScroll)
         {
         if (fHScroll)
            *(int FAR *)lpPP->lParam = 3;
         else
            *(int FAR *)lpPP->lParam = 2;
          *(int FAR *)lpPP->lParam = 1;
         }
      else if (fHScroll)
         *(int FAR *)lpPP->lParam = 1;
      else
         *(int FAR *)lpPP->lParam = 1; 
        
      }
   else
      {
      switch ((int)lpPP->lParam)
         {
         case 1:
            //VBWRTextSetStyle(lpPP->hWnd, FALSE, WS_VSCROLL, TRUE);
            //VBWRTextSetStyle(lpPP->hWnd, FALSE, WS_HSCROLL, TRUE);
            break;
         case 2:
           // VBWRTextSetStyle(lpPP->hWnd, FALSE, WS_VSCROLL, TRUE);
          //  VBWRTextSetStyle(lpPP->hWnd, FALSE, WS_HSCROLL, FALSE);
            break;
         case 3:
         //   VBWRTextSetStyle(lpPP->hWnd, FALSE, WS_VSCROLL, TRUE);
         //   VBWRTextSetStyle(lpPP->hWnd, FALSE, WS_HSCROLL, TRUE);
            break;
         default:
            //VBWRTextSetStyle(lpPP->hWnd, FALSE, WS_VSCROLL, FALSE);
            //VBWRTextSetStyle(lpPP->hWnd, FALSE, WS_HSCROLL, FALSE);
            break;
         }
      }
   return 0;
}

// Button
long FPLIB FP_ButtonStyle (LPVBPROPPARAMS lpPP, BOOL fGetProp)          
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      {
      switch (ncInfo.lStyle & FP_NCBUTTON)
         {
         case FP_NCSPIN:
            *(int FAR *)lpPP->lParam = 1; break; 
         case FP_NCPOPUP:
            *(int FAR *)lpPP->lParam = 2; break; 
         case FP_NCDROPDOWN:
            *(int FAR *)lpPP->lParam = 3; break; 
         case FP_NCSLIDE:
            *(int FAR *)lpPP->lParam = 4; break; 
         default:
            *(int FAR *)lpPP->lParam = 0; break;
         }
      }
   else
      {
      switch ((int)lpPP->lParam)
         {
         case 1:
            ncInfo.lStyle = (ncInfo.lStyle & ~FP_NCBUTTON) | FP_NCSPIN;
            break;
         case 2:
            ncInfo.lStyle = (ncInfo.lStyle & ~FP_NCBUTTON) | FP_NCPOPUP;
            break;
         case 3:
            ncInfo.lStyle = (ncInfo.lStyle & ~FP_NCBUTTON) | FP_NCDROPDOWN;
            break;
         case 4:
            ncInfo.lStyle = (ncInfo.lStyle & ~FP_NCBUTTON) | FP_NCSLIDE;
            break;
         default:
            ncInfo.lStyle = (ncInfo.lStyle & ~FP_NCBUTTON);
            break;
         }
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      } 
   return 0;
}

long FPLIB FP_ButtonMin (LPVBPROPPARAMS lpPP, BOOL fGetProp)          
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      *(int FAR *)lpPP->lParam = ncInfo.dButtonMin;
   else
      {
      ncInfo.dButtonMin = (int)lpPP->lParam;
      // if the current ButtonIndex is less than new min, set index to min
      if (ncInfo.dButtonIndex < ncInfo.dButtonMin)
        ncInfo.dButtonIndex = ncInfo.dButtonMin;
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      } 
   return 0;
}

                                                      
// NOTE:  For VBX's, use FP_VBButtonWidth(), found in VBNCPNT.C
//
long FPLIB FP_ButtonWidth (LPVBPROPPARAMS lpPP, BOOL fGetProp)          
{
   FPNCINFO ncInfo;
   
   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      *(LONG FAR *)lpPP->lParam = ncInfo.dButtonWidth;
   else
      {
      short bWidth = (int)lpPP->lParam;
	  if (bWidth >= 0)
	      ncInfo.dButtonWidth = (int)bWidth;
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      } 
   return 0;
}

long FPLIB FP_ButtonMax (LPVBPROPPARAMS lpPP, BOOL fGetProp)          
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      *(int FAR *)lpPP->lParam = ncInfo.dButtonMax;
   else
      {
      ncInfo.dButtonMax = (int)lpPP->lParam;
      // if the current ButtonIndex is greater than new max, set index to max
      if (ncInfo.dButtonIndex > ncInfo.dButtonMax)
        ncInfo.dButtonIndex = ncInfo.dButtonMax;
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      } 
   return 0;
}

long FPLIB FP_ButtonIndex (LPVBPROPPARAMS lpPP, BOOL fGetProp)          
{
#define ERR_INVALID_PROPVAL      380
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      *(int FAR *)lpPP->lParam = ncInfo.dButtonIndex;
   else
      {
      // if not within range, return error
      if (((int)lpPP->lParam > ncInfo.dButtonMax) || 
          ((int)lpPP->lParam < ncInfo.dButtonMin))
        return (LONG)ERR_INVALID_PROPVAL;  
        
      ncInfo.dButtonIndex = (int)lpPP->lParam;
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      } 
   return 0;
}

long FPLIB FP_ButtonIncrement (LPVBPROPPARAMS lpPP, BOOL fGetProp)          
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      *(int FAR *)lpPP->lParam = ncInfo.dButtonIncrement;
   else
      {
      ncInfo.dButtonIncrement = (short)lpPP->lParam;
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      } 
   return 0;
}

long FPLIB FP_ButtonWrap (LPVBPROPPARAMS lpPP, BOOL fGetProp)          
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      *(int FAR *)lpPP->lParam = (ncInfo.lStyle & FP_NCBUTTONWRAP ? PROP_TRUE : 0);
   else
      {
      if ((int)lpPP->lParam)
         ncInfo.lStyle = (ncInfo.lStyle | FP_NCBUTTONWRAP);
      else
         ncInfo.lStyle = (ncInfo.lStyle & ~FP_NCBUTTONWRAP);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      } 
   return 0;
}

long FPLIB FP_ButtonDisabled (LPVBPROPPARAMS lpPP, BOOL fGetProp)          
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      *(int FAR *)lpPP->lParam = (ncInfo.lStyle & FP_NCBUTTONDISABLED ? PROP_TRUE : 0);
   else
      {
      if ((int)lpPP->lParam)
         ncInfo.lStyle = (ncInfo.lStyle | FP_NCBUTTONDISABLED);
      else
         ncInfo.lStyle = (ncInfo.lStyle & ~FP_NCBUTTONDISABLED);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      } 
   return 0;
}

long FPLIB FP_ButtonHide (LPVBPROPPARAMS lpPP, BOOL fGetProp)          
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      *(int FAR *)lpPP->lParam = (ncInfo.lStyle & FP_NCBUTTONHIDE ? PROP_TRUE : 0);
   else
      {
      if ((int)lpPP->lParam)
         ncInfo.lStyle = (ncInfo.lStyle | FP_NCBUTTONHIDE);
      else
         ncInfo.lStyle = (ncInfo.lStyle & ~FP_NCBUTTONHIDE);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      } 
   return 0;
}

long FPLIB FP_ButtonDefaultAction (LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      *(int FAR *)lpPP->lParam = (ncInfo.lStyle & FP_NCBUTTONDEFAULTACTION ? PROP_TRUE : 0);
   else
      {
      if ((int)lpPP->lParam)
         ncInfo.lStyle = (ncInfo.lStyle | FP_NCBUTTONDEFAULTACTION);
      else
         ncInfo.lStyle = (ncInfo.lStyle & ~FP_NCBUTTONDEFAULTACTION);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      } 
   return 0;
}

long FPLIB FP_ButtonColor(LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      *(long FAR *)lpPP->lParam = fpCM_GetItem(ncInfo.idClrButton, NULL, NULL);
   else
      {
      fpCM_FreeItem(ncInfo.idClrButton);
      ncInfo.idClrButton = fpCM_AddItem((COLORREF)lpPP->lParam);
      ncInfo.clrButton = fpCM_GetItem(ncInfo.idClrButton, NULL, NULL);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }

   return 0;
}             

long FPLIB FP_ButtonAlign(LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
	   *(long FAR *)lpPP->lParam = (long)(ncInfo.fButtonLeft ? 1 : 0);
   else
      {
	   ncInfo.fButtonLeft = (lpPP->lParam ? TRUE : FALSE);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }

   return 0;
}      

long FPLIB FP_ThreeDFrameColor(LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
                               
   if (fGetProp)
      *(long FAR *)lpPP->lParam = fpCM_GetItem(ncInfo.idClr3DFrame, NULL, NULL);
//      *(long FAR *)lpPP->lParam = ncInfo.clr3DFrame;
   else
      {
      fpCM_FreeItem(ncInfo.idClr3DFrame);
      ncInfo.idClr3DFrame = fpCM_AddItem((COLORREF)lpPP->lParam);
      ncInfo.clr3DFrame = fpCM_GetItem(ncInfo.idClr3DFrame, NULL, NULL);
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }

   return 0;
}

long FPLIB FP_BorderDropShadow(LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);

   if (fGetProp)
      {
      if (ncInfo.lStyle & FP_NCDROPSHADOW)
         *(int FAR *)lpPP->lParam = 1;
      else if (ncInfo.lStyle & FP_NCDROPSHADOWFOCUS)
         *(int FAR *)lpPP->lParam = 2;
      else
         *(int FAR *)lpPP->lParam = 0;
      }
   else
      {
      ncInfo.lStyle &= ~(FP_NCDROPSHADOW | FP_NCDROPSHADOWFOCUS);
      if ((short)lpPP->lParam == 2)
         ncInfo.lStyle |= FP_NCDROPSHADOWFOCUS;
      else if ((short)lpPP->lParam == 1)
         ncInfo.lStyle |= FP_NCDROPSHADOW;

      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }

   return 0;
}

long FPLIB FP_Appearance(LPVBPROPPARAMS lpPP, BOOL fGetProp)
{
   FPNCINFO ncInfo;

   SendMessage(lpPP->hWnd, FPM_GETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);

   if (fGetProp)
      *(int FAR *)lpPP->lParam = (int)ncInfo.bAppearance;
   else
      {
      BYTE bAppearance = (BYTE)lpPP->lParam;

      switch ((short)lpPP->lParam)
         {
         case FPNC_APPEARANCE_FLAT:
            ncInfo.lStyle = NC_SET3DINSIDESTYLE (ncInfo, 0);
            ncInfo.lStyle = NC_SET3DOUTSIDESTYLE (ncInfo, 0);
            NC_SETSTYLE(ncInfo, FP_NCBORDER, 1);
            NC_SETSTYLE(ncInfo, FP_NCROUND, 0);
            ncInfo.bPenStyleOnFocus = PS_SOLID;
            ncInfo.bPenStyleNoFocus = PS_SOLID;
            ncInfo.bFrameWidth = 0;
            ncInfo.bBorderSize = 1;
            break;

         case FPNC_APPEARANCE_3D:
            ncInfo.lStyle = NC_SET3DINSIDESTYLE (ncInfo, FP_NC3DINLOWER);
            ncInfo.lStyle = NC_SET3DOUTSIDESTYLE (ncInfo, FP_NC3DOUTLOWER);
            NC_SETSTYLE(ncInfo, FP_NCBORDER, 0);
            NC_SETSTYLE(ncInfo, FP_NCROUND, 0);
            ncInfo.bInShadowWidth = 1;
            ncInfo.bOutShadowWidth = 1;
            ncInfo.bFrameWidth = 0;
            break;

         case FPNC_APPEARANCE_3DBORDER:
            ncInfo.lStyle = NC_SET3DINSIDESTYLE (ncInfo, FP_NC3DINLOWER);
            ncInfo.lStyle = NC_SET3DOUTSIDESTYLE (ncInfo, FP_NC3DOUTLOWER);
            NC_SETSTYLE(ncInfo, FP_NCBORDER, 1);
            NC_SETSTYLE(ncInfo, FP_NCROUND, 0);
            ncInfo.bPenStyleOnFocus = PS_SOLID;
            ncInfo.bPenStyleNoFocus = PS_SOLID;
            ncInfo.bInShadowWidth = 1;
            ncInfo.bOutShadowWidth = 1;
            ncInfo.bFrameWidth = 0;
            ncInfo.bBorderSize = 1;
            break;

         }

      ncInfo.bAppearance = bAppearance;
      SendMessage(lpPP->hWnd, FPM_SETNCINFO, 0, (LPARAM)(LPFPNCINFO)&ncInfo);
      }

   return 0;
}
