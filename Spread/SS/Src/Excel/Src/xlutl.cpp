/*********************************************************
* XLutl.cpp
*
* Copyright (C) 1999 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
* UPDATE LOG:
* -----------
*	RAP	- 11.19.98
*********************************************************/

#if defined(_WIN64) || defined(_IA64)
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "tchar.h"
#include "ctype.h"
#include "io.h"
#include "math.h"
#include <stdio.h>
#include "windows.h"

#include "spread.h"
#include "xl.h"
#include "xlbiff.h"
#include "xlutl.h"
#include "biff.h"
#include "ss_alloc.h"
#include "resource.h"

long xl_SnapToColor(COLORREF color);


/***********************************************************************
* Name:    Alloc - Perform a GlobalAlloc or ReAlloc as needed
*
* Usage:   short Alloc(TBGLOBALHANDLE FAR *lpgh, long lSize)
*            lpgh - buffer
*            lSize - size of resulting buffer.
*
* Return:  short - Success or Failure
***********************************************************************/
short Alloc(TBGLOBALHANDLE FAR *lpgh, long lSize)
{
  if (!*lpgh)
    *lpgh = tbGlobalAlloc(GHND, lSize);
  else
    *lpgh = tbGlobalReAlloc(*lpgh, lSize, GHND);
  
  return 0;
}

/*
int CALLBACK EnumObjectsProc(LPVOID lpLogObject, LPARAM lpData)
{
  LPGLOBALHANDLE lpgh = (LPGLOBALHANDLE)lpData;
  LPBYTE         lp = (LPBYTE)GlobalLock(*lpgh);
  long           lCount = *(LPLONG)lp;

  GlobalUnlock(*lpgh);
  if (((LPLOGPEN)lpLogObject)->lopnStyle == PS_SOLID)
  {
    Alloc(lpgh, sizeof(long) + ((lCount+1)*sizeof(COLORREF)));
    lp = (LPBYTE)GlobalLock(*lpgh);
    *(LPLONG)lp = lCount + 1;
    lp += sizeof(long) + (lCount) * sizeof(COLORREF);
    memcpy(lp, &((LPLOGPEN)lpLogObject)->lopnColor, sizeof(COLORREF));
    GlobalUnlock(*lpgh);
  }
  return 1;
}

COLORREF xl_ColorFromIndex(long lIndex)
{
  COLORREF     color = RGBCOLOR_DEFAULT;
  HDC          hDC = GetDC(NULL);
  GLOBALHANDLE gh = GlobalAlloc(GHND, sizeof(long));
  LPBYTE       lp;
  short        sCount = GetDeviceCaps(hDC, NUMCOLORS);
  
  if (lIndex == (long)-1)
  {
    ReleaseDC(NULL, hDC);    
    return color;
  }

  EnumObjects(hDC, OBJ_PEN, (GOBJENUMPROC)EnumObjectsProc, (LPARAM)&gh);
  ReleaseDC(NULL, hDC);

  lp = (LPBYTE)GlobalLock(gh);
  if (lIndex <= *(LPLONG)lp)
  {
    lp += sizeof(long);
    color = ((LPCOLORREF)lp)[lIndex];
  }
  GlobalUnlock(gh);
  
  return color;
}

long xl_IndexFromColor(COLORREF color)
{
  long         lIndex = (long)-1;
  HDC          hDC = GetDC(NULL);
  GLOBALHANDLE gh = GlobalAlloc(GHND, sizeof(short));
  LPBYTE       lp;
  long         lCount;
  short        i = GetDeviceCaps(hDC, NUMCOLORS);

  EnumObjects(hDC, OBJ_PEN, (GOBJENUMPROC)EnumObjectsProc, (LPARAM)&gh);
  ReleaseDC(NULL, hDC);

  lp = (LPBYTE)GlobalLock(gh);
  lCount = *(LPLONG)lp;
  lp += sizeof(long);
  for (i=0; i<lCount; i++)
  {
    if (color == ((LPCOLORREF)lp)[i])
    {
      lIndex = i;
      break;
    }
  }
  GlobalUnlock(gh);
  
  return lIndex;
}
*/

/***********************************************************************
* Name:    xl_ColorFromIndex - Convert from an Excel BIFF color index
*                              to an RGB value.
*
* Usage:   COLORREF xl_ColorFromIndex(long lIndex)
*                     lIndex - Excel BIFF color index.  
*
* Return:  COLORREF 
***********************************************************************/
COLORREF xl_ColorFromIndex(long lIndex)
{
  COLORREF color = RGBCOLOR_DEFAULT;
  
  if (lIndex == 32767)
  {
    return color;
  }


  switch (lIndex)
  {
    case 64:
//      color = 0;
    break;
  }

  switch (lIndex)
  {
    case 8:
      color = RGB(0,0,0);
    break;
    case 9:
      color = RGB(255,255,255);
    break;
    case 10:
      color = RGB(255,0,0);
    break;
    case 11:
      color = RGB(0,255,0);
    break;
    case 12:
      color = RGB(0,0,255);
    break;
    case 13:
      color = RGB(255,255,0);
    break;
    case 14:
      color = RGB(255,0,255);
    break;
    case 15:
      color = RGB(0,255,255);
    break;
    case 16:
      color = RGB(128,0,0);
    break;
    case 17:
      color = RGB(0,128,0);
    break;
    case 18:
      color = RGB(0,0,128);
    break;
    case 19:
      color = RGB(128,128,0);
    break;
    case 20:
      color = RGB(128,0,128);
    break;
    case 21:
      color = RGB(0,128,128);
    break;
    case 22:
      color = RGB(192,192,192);
    break;
    case 23:
      color = RGB(128,128,128);
    break;
    case 24:
      color = RGB(153,153,153);
    break;
    case 25:
      color = RGB(153,51,102);
    break;
    case 26:
      color = RGB(255,255,204);
    break;
    case 27:
      color = RGB(204,255,255);
    break;
    case 28:
      color = RGB(102,0,102);
    break;
    case 29:
      color = RGB(255,128,128);
    break;
    case 30:
      color = RGB(0,102,204);
    break;
    case 31:
      color = RGB(204,204,255);
    break;
    case 32:
      color = RGB(0,0,128);
    break;
    case 33:
      color = RGB(255,0,255);
    break;
    case 34:
      color = RGB(255,255,0);
    break;
    case 35:
      color = RGB(0,255,255);
    break;
    case 36:
      color = RGB(128,0,128);
    break;
    case 37:
      color = RGB(128,0,0);
    break;
    case 38:
      color = RGB(0,128,128);
    break;
    case 39:
      color = RGB(0,0,255);
    break;
    case 40:
      color = RGB(0,204,255);
    break;
    case 41:
      color = RGB(204,255,255);
    break;
    case 42:
      color = RGB(204,255,204);
    break;
    case 43:
      color = RGB(255,255,153);
    break;
    case 44:
      color = RGB(153,204,255);
    break;
    case 45:
      color = RGB(255,153,204);
    break;
    case 46:
      color = RGB(204,153,255);
    break;
    case 47:
      color = RGB(255,204,153);
    break;
    case 48:
      color = RGB(51,102,255);
    break;
    case 49:
      color = RGB(51,204,204);
    break;
    case 50:
      color = RGB(153,204,0);
    break;
    case 51:
      color = RGB(255,204,0);
    break;
    case 52:
      color = RGB(255,153,102);
    break;
    case 53:
      color = RGB(255,102,0);
    break;
    case 54:
      color = RGB(102,102,153);
    break;
    case 55:
      color = RGB(150,150,150);
    break;
    case 56:
      color = RGB(0,51,102);
    break;
    case 57:
      color = RGB(51,153,102);
    break;
    case 58:
      color = RGB(0,51,0);
    break;
    case 59:
      color = RGB(51,51,0);
    break;
    case 60:
      color = RGB(153,51,0);
    break;
    case 61:
      color = RGB(153,51,102);
    break;
    case 62:
      color = RGB(51,51,153);
    break;
    case 63:
      color = RGB(51,51,51);
    break;
  } //switch

  return color;
}

/***********************************************************************
* Name:    xl_IndexFromColor - Convert from an RGB value to an Excel BIFF
*                              color index.
*
* Usage:   COLORREF xl_IndexFromColor(COLORREF color)
*                     color - RGB value.
*
* Return:  long - Excel BIFF color index. 
***********************************************************************/
long xl_IndexFromColor(COLORREF color)
{
  long lIndex = (long)32767;
  
  if (color == 0x8000000b || color == 0xFFFFFFFF)
//    return -1L;
    return 64;

  if (color & 0x80000000)
    color = GetSysColor(color & ~0x80000000);

  switch (color)
  {
    case 0x000000:
      lIndex = 8;
    break;
    case 0xFFFFFF://RGB(255,255,255);
      lIndex = 9;
    break;
    case 0x0000FF: //RGB(255,0,0);
      lIndex = 10;
    break;
    case 0x00FF00://RGB(0,255,0);
      lIndex = 11;
    break;
    case 0xFF0000://RGB(0,0,255);
      lIndex = 12;
    break;
    case 0x00FFFF://RGB(255,255,0);
      lIndex = 13;
    break;
    case 0xFF00FF://RGB(255,0,255);
      lIndex = 14;
    break;
    case 0xFFFF00://RGB(0,255,255);
      lIndex = 15;
    break;
    case 0x000080://RGB(128,0,0);
      lIndex = 16;
    break;
    case 0x008000://RGB(0,128,0);
      lIndex = 17;
    break;
    case 0x800000://RGB(0,0,128);
      lIndex = 18;
    break;
    case 0x008080://RGB(128,128,0);
      lIndex = 19;
    break;
    case 0x800080://RGB(128,0,128);
      lIndex = 20;
    break;
    case 0x808000://RGB(0,128,128);
      lIndex = 21;
    break;
    case 0xC0C0C0://RGB(192,192,192);
      lIndex = 22;
    break;
    case 0x808080://RGB(128,128,128);
      lIndex = 23;
    break;
    case 0xff9999://RGB(0x99,0x99,0xff);
      lIndex = 24;
    break;
/*
    case 0x663399://RGB(0x99,0x33,0x66);
      lIndex = 25;
    break;
*/
    case 0xccffff://RGB(0xff,0xff,0xcc);
      lIndex = 26;
    break;
/*    case 0xffffcc://RGB(0xcc,0xff,0xff);
      lIndex = 27;
    break;
*/
    case 0x660066://RGB(0x66,0x00,0x66);
      lIndex = 28;
    break;
    case 0x8080ff://RGB(0xff,0x80,0x80);
      lIndex = 29;
    break;
    case 0xcc6600://RGB(0x00,0x66,0xcc);
      lIndex = 30;
    break;
    case 0xffcccc://RGB(0xcc,0xcc,0xff);
      lIndex = 31;
    break;
/*dupes 
    case 0x800000://RGB(0x00,0x00,0x80);
      lIndex = 32;
    break;
    case 0xff00ff://RGB(0xff,0x00,0xff);
      lIndex = 33;
    break;
    case 0x00ffff://RGB(0xff,0xff,0x00);
      lIndex = 34;
    break;
    case 0xffff00://RGB(0x00,0xff,0xff);
      lIndex = 35;
    break;
    case 0x800080://RGB(0x80,0x00,0x80);
      lIndex = 36;
    break;
    case 0x000080://RGB(0x80,0x00,0x00);
      lIndex = 37;
    break;
    case 0x808000://RGB(0x00,0x80,0x80);
      lIndex = 38;
    break;
    case 0xff0000://RGB(0x00,0x00,0xff);
      lIndex = 39;
    break;
*/
    case 0xFFCC00://RGB(0,204,255);
      lIndex = 40;
    break;
    case 0xFFFFCC://RGB(204,255,255);
      lIndex = 41;
    break;
    case 0xCCFFCC://RGB(204,255,204);
      lIndex = 42;
    break;
    case 0x99FFFF: //RGB(255,255,153);
      lIndex = 43;
    break;
    case 0xFFCC99://RGB(153,204,255);
      lIndex = 44;
    break;
    case 0xCC99FF://RGB(255,153,204);
      lIndex = 45;
    break;
    case 0xFF99CC://RGB(204,153,255);
      lIndex = 46;
    break;
    case 0x99CCFF://RGB(255,204,153);
      lIndex = 47;
    break;
    case 0xFF6633://RGB(51,102,255);
      lIndex = 48;
    break;
    case 0xCCCC33://RGB(51,204,204);
      lIndex = 49;
    break;
    case 0x00CC99://RGB(153,204,0);
      lIndex = 50;
    break;
    case 0x00CCFF://RGB(255,204,0);
      lIndex = 51;
    break;
    case 0x6699FF://RGB(255,153,102);
      lIndex = 52;
    break;
    case 0x0066FF://RGB(255,102,0);
      lIndex = 53;
    break;
    case 0x996666://RGB(102,102,153);
      lIndex = 54;
    break;
    case 0x969696://RGB(150,150,150);
      lIndex = 55;
    break;
    case 0x663300://RGB(0,51,102);
      lIndex = 56;
    break;
    case 0x669933://RGB(51,153,102);
      lIndex = 57;
    break;
    case 0x003300://RGB(0,51,0);
      lIndex = 58;
    break;
    case 0x003333://RGB(51,51,0);
      lIndex = 59;
    break;
    case 0x003399://RGB(153,51,0);
      lIndex = 60;
    break;
    case 0x663399://RGB(153,51,102);
      lIndex = 61;
    break;
    case 0x993333://RGB(51,51,153);
      lIndex = 62;
    break;
    case 0x333333://RGB(51,51,51);
      lIndex = 63;
    break;

    default:
      lIndex = xl_SnapToColor(color);
    break;
  }
      
  return lIndex;
}

/***********************************************************************
* Name:    xl_SnapToColor - Return the Excel BIFF color index that 
*                           represents the RGB color closest to the 
*                           specified RGB value.
*
* Usage:   long xl_SnapToColor(COLORREF color)
*                 color - RGB value.
*
* Return:  long - Excel BIFF color index. 
***********************************************************************/
long xl_SnapToColor(COLORREF color)
{
  COLORREF newcolor;
  BYTE r = GetRValue(color);
  BYTE g = GetGValue(color);
  BYTE b = GetBValue(color);
  short sVal;
  short sMinAveDiff;

  //0x000000
  sMinAveDiff = (short)((r+g+b)/3);
  newcolor = 0x000000;
  
  //0x003399 //RGB(153,51,0);
  sVal = (short)((abs(r-0x99)+abs(g-0x33)+b)/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x003399;
  }

  //0x003333 //RGB(51,51,0);
  sVal = (short)((abs(r-0x33)+abs(g-0x33)+b)/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x003333;
  }

  //0x003300://RGB(0,51,0)
  sVal = (short)((r+abs(g-0x33)+b)/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x003300;
  }

  //0x663300://RGB(0,51,102);
  sVal = (short)((r+abs(g-0x33)+abs(b-0x66))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x663300;
  }

  //0x800000://RGB(0,0,128);
  sVal = (short)((r+g+abs(b-0x80))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x800000;
  }

  //0x993333://RGB(51,51,153);
  sVal = (short)((abs(r-0x33)+abs(g-0x33)+abs(b-0x99))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x993333;
  }

  //0x333333://RGB(51,51,51);
  sVal = (short)((abs(r-0x33)+abs(g-0x33)+abs(b-0x33))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x333333;
  }

  //0x000080://RGB(128,0,0);
  sVal = (short)((abs(r-0x80)+g+b)/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x000080;
  }

  //0x0066FF://RGB(255,102,0);
  sVal = (short)((abs(r-0xFF)+abs(g-0x66)+b)/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x0066FF;
  }

  //0x008080://RGB(128,128,0);
  sVal = (short)((abs(r-0x80)+abs(g-0x80)+b)/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x008080;
  }

  //0x008000://RGB(0,128,0);
  sVal = (short)((r+abs(g-0x80)+b)/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x008000;
  }

  //0x808000://RGB(0,128,128);  
  sVal = (short)((r+abs(g-0x80)+abs(b-0x80))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x808000;
  }

  //0xFF0000://RGB(0,0,255);
  sVal = (short)((r+g+abs(b-0xFF))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0xFF0000;
  }

  //0x996666://RGB(102,102,153);
  sVal = (short)((abs(r-0x66)+abs(g-0x66)+abs(b-0x99))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x996666;
  }

  //0x808080://RGB(128,128,128);
  sVal = (short)((abs(r-0x80)+abs(g-0x80)+abs(b-0x80))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x808080;
  }

  //0x0000FF: //RGB(255,0,0);
  sVal = (short)((abs(r-0xFF)+g+b)/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x0000FF;
  }

  //0x6699FF://RGB(255,153,102);
  sVal = (short)((abs(r-0xFF)+abs(g-0x99)+abs(b-0x66))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x6699FF;
  }

  //0x00CC99://RGB(153,204,0);
  sVal = (short)((abs(r-0x99)+abs(g-0xCC)+b)/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x00CC99;
  }

  //0x669933://RGB(51,153,102);
  sVal = (short)((abs(r-0x33)+abs(g-0x99)+abs(b-0x66))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x669933;
  }

  //0xCCCC33://RGB(51,204,204);
  sVal = (short)((abs(r-0x33)+abs(g-0xCC)+abs(b-0xCC))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0xCCCC33;
  }

  //0xFF6633://RGB(51,102,255);
  sVal = (short)((abs(r-0x33)+abs(g-0x66)+abs(b-0xFF))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0xFF6633;
  }

  //0x800080://RGB(128,0,128);
  sVal = (short)((abs(r-0x80)+g+abs(b-0x80))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x800080;
  }

  //0x969696://RGB(150,150,150);
  sVal = (short)((abs(r-0x96)+abs(g-0x96)+abs(b-0x96))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x969696;
  }

  //0xFF00FF://RGB(255,0,255);
  sVal = (short)((abs(r-0xFF)+g+abs(b-0x96))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0xFF00FF;
  }

  //0x00CCFF://RGB(255,204,0);
  sVal = (short)(((abs(r-0xFF)+abs(g-0xCC)+b)/3));
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x00CCFF;
  }

  //0x00FFFF://RGB(255,255,0);
  sVal = (short)((abs(r-0xFF)+abs(g-0xFF)+b)/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x00FFFF;
  }

  //0x00FF00://RGB(0,255,0);
  sVal = (short)((r+abs(g-0xFF)+b)/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x00FF00;
  }

  //0xFFFF00://RGB(0,255,255);
  sVal = (short)((r+abs(g-0xFF)+abs(b-0xFF))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0xFFFF00;
  }

  //0xFFCC00://RGB(0,204,255);
  sVal = (short)((r+abs(g-0xCC)+abs(b-0xFF))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0xFFCC00;
  }

  //0x663399://RGB(153,51,102);
  sVal = (short)((abs(r-0x99)+abs(g-0x33)+abs(b-0x66))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x663399;
  }

  //0xC0C0C0://RGB(192,192,192);
  sVal = (short)((abs(r-0xC0)+abs(g-0xC0)+abs(b-0xC0))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0xC0C0C0;
  }

  //0xCC99FF://RGB(255,153,204);
  sVal = (short)((abs(r-0xFF)+abs(g-0x99)+abs(b-0xCC))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0xCC99FF;
  }

  //0x99CCFF://RGB(255,204,153);
  sVal = (short)((abs(r-0xFF)+abs(g-0xCC)+abs(b-0x99))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x99CCFF;
  }

  //0x99FFFF: //RGB(255,255,153);
  sVal = (short)((abs(r-0xFF)+abs(g-0xFF)+abs(b-0x99))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0x99FFFF;
  }

  //0xCCFFCC://RGB(204,255,204);
  sVal = (short)((abs(r-0xCC)+abs(g-0xFF)+abs(b-0xCC))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0xCCFFCC;
  }

  //0xFFFFCC://RGB(204,255,255);
  sVal = (short)((abs(r-0xCC)+abs(g-0xFF)+abs(b-0xFF))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0xFFFFCC;
  }

  //0xFFCC99://RGB(153,204,255);
  sVal = (short)((abs(r-0x99)+abs(g-0xCC)+abs(b-0xFF))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0xFFCC99;
  }

  //0xFF99CC://RGB(204,153,255);
  sVal = (short)((abs(r-0xCC)+abs(g-0x99)+abs(b-0xFF))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0xFF99CC;
  }

  //0xFFFFFF://RGB(255,255,255);
  sVal = (short)((abs(r-0xFF)+abs(g-0xFF)+abs(b-0xFF))/3);
  if (sVal < sMinAveDiff)
  {
    sMinAveDiff = sVal;
    newcolor = 0xFFFFFF;
  }

  return xl_IndexFromColor(newcolor);
}

/***********************************************************************
* Name:    xl_CatWSToWS - Concatenate two buffers together. Append the 
*                         "source" buffer to the end of the "destination"
*                         buffer, and free the "source" buffer.
*
* Usage:   long xl_CatWSToWS(LPTBGLOBALHANDLE lpghDest, LPLONG lplDestLen,
*                            LPTBGLOBALHANDLE lpghSrc, LPLONG lplSrcLen)
*                lpghDest - destination bufer
*                lplDestLen - length of destination buffer
*                lpghSrc - source buffer
*                lplSrcLen - length of source buffer.  
*
* Return:  long - Success or failure.
***********************************************************************/
long xl_CatWSToWS(LPTBGLOBALHANDLE lpghDest, LPLONG lplDestLen, LPTBGLOBALHANDLE lpghSrc, LPLONG lplSrcLen)
{
  HPBYTE hpDest = NULL;
  HPBYTE hpSrc = NULL;
   
  
  if (!*lplSrcLen)
    return S_OK;

  hpSrc = (HPBYTE)tbGlobalLock(*lpghSrc);
  Alloc(lpghDest, *lplDestLen + *lplSrcLen);
  hpDest = (HPBYTE)tbGlobalLock(*lpghDest);
  hpDest = (HPBYTE)(hpDest + *lplDestLen);
  *lplDestLen += *lplSrcLen;
  MemHugeCpy(hpDest, hpSrc, *lplSrcLen);
  
  *lplSrcLen = 0;
  tbGlobalUnlock(*lpghDest);
  tbGlobalUnlock(*lpghSrc);
  tbGlobalFree(*lpghSrc);
  *lpghSrc = (TBGLOBALHANDLE)0;

  return S_OK;
}
  
/***********************************************************************
* Name:    xl_LogFile - Write a message to the logfile.
*
* Usage:   short FP_API xl_LogFile(LPSPREADSHEET lpSS, short LogId,
*                                  long lCol, long lRow, LPVOID lpData)
*                         lpSS - pointer to Spread structure.
*                         LogId - Id of message to be written.
*                         lCol - referenced column.
*                         lRow - referenced row.  
*                         lpData - additional data to be added to the message.
*
* Return:  short - Success or failure
***********************************************************************/
short FP_API xl_LogFile(LPSS_BOOK lpBook, short LogId, long lCol, long lRow, LPVOID lpData)
{
  FILE *hf = (FILE *)lpBook->lXLLogFile;

  if (hf == (FILE *)-1)
  {
    //No LogFile
    return 0L;
  }

  switch(LogId)
  {
    case LOG_OPEN:
      SYSTEMTIME SystemTime;

      if (!lpData || !lstrlen((LPTSTR)lpData))
      {
        lpBook->lXLLogFile = -1;
        return 0L;
      }

      if (lCol == 0)
      {
        TCHAR buffer[500];
        hf = _tfopen((LPTSTR)lpData, _T("w"));

        if (NULL == hf)
        {
          lpBook->lXLLogFile = -1;
          return 0L;
        }

        LoadString((HINSTANCE)fpInstance, IDS_LOGS_FP, buffer, 500);
        _ftprintf(hf, buffer);

        if (lRow == 0) // open for import
        {
          LoadString((HINSTANCE)fpInstance, IDS_LOGS_OPEN_IMPWB, buffer, 500);
          _ftprintf(hf, buffer);
        }
        else if (lRow == 1) // open for export
        {
          LoadString((HINSTANCE)fpInstance, IDS_LOGS_OPEN_EXPORT, buffer, 500);
          _ftprintf(hf, buffer);
        }
        GetLocalTime(&SystemTime);
        _ftprintf(hf, _T("%d/%d/%d - %02d:%02d\n\n"), SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear,
                                                SystemTime.wHour, SystemTime.wMinute);
        LoadString((HINSTANCE)fpInstance, IDS_LOGS_HEADERSTUFF, buffer, 500);
        _ftprintf(hf, buffer);
        _ftprintf(hf, _T("___\t___\t___________\n"));
      }
      else if (lCol == 1)
      {
        TCHAR buffer[500];
        hf = _tfopen((LPTSTR)lpData, _T("a"));

        if (NULL == hf)
        {
          lpBook->lXLLogFile = -1;
          return 0L;
        }

        LoadString((HINSTANCE)fpInstance, IDS_LOGS_OPEN_IMPSHEET, buffer, 500);
        _ftprintf(hf, buffer);
    
        LoadString((HINSTANCE)fpInstance, IDS_LOGS_LOADSHEET, buffer, 500);
        _ftprintf(hf, _T("\t\t%s %d\n"), buffer, (short)lRow);
      }  
#if defined(_WIN64) || defined(_IA64)
      lpBook->lXLLogFile = (LONG_PTR)hf;
#else
      lpBook->lXLLogFile = (long)hf;
#endif
    break;
    case LOG_LOADFILE:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_LOADFILE, buffer, 500);
      _ftprintf(hf, _T("\t\t%s%s\n"), buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_SAVEFILE:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_SAVEFILE, buffer, 500);
      _ftprintf(hf, _T("\t\t%s%s\n"), buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_CLOSE:
    {
      TCHAR buffer[500];
      TCHAR buffer2[500];
      if (lCol == 0)
      {
        LoadString((HINSTANCE)fpInstance, IDS_LOGS_CLOSE_IMPWB, buffer, 500);
        _ftprintf(hf, _T("\n%s\n"), buffer);
      }
      else if (lCol == 1)
      {
        LoadString((HINSTANCE)fpInstance, IDS_LOGS_CLOSE_IMPSHEET, buffer, 500);
        LoadString((HINSTANCE)fpInstance, IDS_LOGS_FP, buffer2, 500);
        _ftprintf(hf, _T("\n%s\n%s"), buffer, buffer2);
      }
      else if (lCol == 2)
      {
        LoadString((HINSTANCE)fpInstance, IDS_LOGS_CLOSE_EXPORT, buffer, 500);
        LoadString((HINSTANCE)fpInstance, IDS_LOGS_FP, buffer2, 500);
        _ftprintf(hf, _T("\n%s\n%s"), buffer, buffer2);
      }

      fclose(hf);
      lpBook->lXLLogFile = 0;
    }
    break;

    case LOG_INTEGERCELLTYPE:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_INTEGERCELLTYPE, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s%f\n"), lCol, lRow, LogId, buffer, *(double *)lpData);
    }
    break;
    case LOG_INVALIDTOKEN:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_INVALIDTOKEN, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%c\n"), LogId, buffer, *(LPTSTR)lpData);
    }
    break;
    case LOG_IGNOREDTOKEN:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_IGNOREDTOKEN, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%c\n"), LogId, buffer, *(LPTSTR)lpData);
    }
    break;
    case LOG_FORMATNOTSUPPORTED:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_FORMATNOTSUPPORTED, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%s\n"), LogId, buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_DATEFORMATNOTSUPPORTED:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_DATEFORMATNOTSUPPORTED, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%s\n"), LogId, buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_FORMAT0IGNORED:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_FORMAT0IGNORED, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%s\n"), LogId, buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_FORMAT0FORCED:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_FORMAT0FORCED, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%s\n"), LogId, buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_FORMATFRACTION:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_FORMATFRACTION, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%s\n"), LogId, buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_FORMATPERCENT:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_FORMATPERCENT, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%s\n"), LogId, buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_FORMATSCIENTIFIC:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_FORMATSCIENTIFIC, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%s\n"), LogId, buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_PMTFUNCTIONLOAD:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_PMTFUNCTIONLOAD, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s\n"), lCol, lRow, LogId, buffer);
    }
    break;
    case LOG_TRUNCFUNCTIONLOAD:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_TRUNCFUNCTIONLOAD, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s\n"), lCol, lRow, LogId, buffer);
    }
    break;
    case LOG_IFFUNCTIONLOAD:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_IFFUNCTIONLOAD, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s\n"), lCol, lRow, LogId, buffer);
    }
    break;
    case LOG_PMTFUNCTIONSAVE:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_PMTFUNCTIONSAVE, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s\n"), lCol, lRow, LogId, buffer);
    }
    break;
    case LOG_XROOTFUNCTIONSAVE:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_XROOTFUNCTIONSAVE, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s\n"), lCol, lRow, LogId, buffer);
    }
    break;
    case LOG_IFFUNCTIONSAVE:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_IFFUNCTIONSAVE, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s\n"), lCol, lRow, LogId, buffer);
    }
    break;
    case LOG_REGISTRYPROPS:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_REGISTRYPROPS, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s\n"), LogId, buffer);
    }
    break;
    case LOG_DATEOUTOFRANGE:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_DATEOUTOFRANGE, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s%s\n"), lCol, lRow, LogId, buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_OBJCELLTYPE:
    {
      short sVal = 0;
      sVal = (short)*(LPBYTE)lpData;
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_OBJCELLTYPE, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s%d\n"), lCol, lRow, LogId, buffer, sVal);
    }        
    break;
    case LOG_FUNCFORMCELLTYPE:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_FUNCFORMCELLTYPE, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s\n"), lCol, lRow, LogId, buffer, NULL);
    }
    break;
    case LOG_FUNCNOTSUPPORTED:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_FUNCNOTSUPPORTED, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s%d\n"), lCol, lRow, LogId, buffer, *(LPSHORT)lpData);
    }
    break;
    case LOG_FORMERROR:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_FORMERROR, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s%d\n"), lCol, lRow, LogId, buffer, *(LPSHORT)lpData);
    }
    break;
    case LOG_PICTOKENNOTSUPPORTED:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_PICTOKENNOTSUPPORTED, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%c\n"), LogId, buffer, *(TCHAR *)lpData);
    }
    break;
    case LOG_CELLOBJECTSNOTSUPPORTED:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_CELLOBJECTSNOTSUPPORTED, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s\n"), LogId, buffer);
    }
    break;
    case LOG_FORMPTGNOTSUPPORTED:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_FORMPTGNOTSUPPORTED, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s%d\n"), lCol, lRow, LogId, buffer, *(LPSHORT)lpData);
    }
    break;
    case LOG_TOOMANYCOLS:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_TOOMANYCOLS, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%d\n"), LogId, buffer, *(LPLONG)lpData);
    }
    break;
    case LOG_TOOMANYROWS:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_TOOMANYROWS, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%d\n"), LogId, buffer, *(LPLONG)lpData);
    }
    break;
    case LOG_FLOATOUTOFBOUNDS:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_FLOATOUTOFBOUNDS, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s%f\n"), lCol, lRow, LogId, buffer, *(double *)lpData);
    }
    break;
    case LOG_CUSTFUNCNOTSUPPORTED:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_CUSTFUNCNOTSUPPORTED, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s\n"), lCol, lRow, LogId, buffer);
    }
    break;
    case LOG_CURRENCYMULTIPLECHARS:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_CURRENCYMULTIPLECHARS, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%s\n"), LogId, buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_CURRENCYNOTFIRST:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_CURRENCYNOTFIRST, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%s\n"), LogId, buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_RICHTEXTNOTSUPPORTED:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_RICHTEXTNOTSUPPORTED, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%s\n"), LogId, buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_EXPORTCHECKBOX:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_EXPORTCHECKBOX, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s\n"), lCol, lRow, LogId, buffer);
    }
    break;
    case LOG_EXPORTBUTTON:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_EXPORTBUTTON, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s\n"), lCol, lRow, LogId, buffer);
    }
    break;
    case LOG_EXPORTCOMBOBOX:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_EXPORTCOMBOBOX, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s\n"), lCol, lRow, LogId, buffer);
    }
    break;
    case LOG_EXPORTSTATICTEXT:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_EXPORTSTATICTEXT, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s\n"), lCol, lRow, LogId, buffer);
    }
    break;
    case LOG_CUSTHEADERNOTSUPPORTED:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_CUSTHEADERNOTSUPPORTED, buffer, 500);
      if (lRow == 0)
        _ftprintf(hf, _T("%ld\t%-\t%d:%s\n"), lCol, LogId, buffer);
      else if (lCol == 0)
        _ftprintf(hf, _T("%-\t%ld\t%d:%s\n"), lRow, LogId, buffer);
    }
    break;
    case LOG_URLFUNCTIONNOTIFY:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_URLFUNCTIONNOTIFY, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s %s\n"), lCol, lRow, LogId, buffer, (LPTSTR)lpData);
    }
    break;
    case LOG_FEB291900:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_FEB291900, buffer, 500);
      _ftprintf(hf, _T("%ld\t%ld\t%d:%s\n"), lCol, lRow, LogId, buffer);
    }
    break;
    case LOG_PASSWORD:
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_PASSWORD, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s\n"), LogId, buffer);
    }
    break;
    case LOG_MULTIPLEFORMATS: //Spread30 Only
    {
      TCHAR buffer[500];
      LoadString((HINSTANCE)fpInstance, IDS_LOGS_MULTIPLEFORMATS, buffer, 500);
      _ftprintf(hf, _T("-\t-\t%d:%s%s\n"), LogId, buffer, (LPTSTR)lpData);
    }
    break;
  }
  
  if (LogId != LOG_CLOSE)
    fflush(hf);

  return 0L;
}


/***********************************************************************
* Name:    xl_NumFromRk - Convert from Excel's internal number format to
*                         a double.
*
* Usage:   double FP_API xl_NumFromRk(long rk, BOOL FAR *lpbf)
*                          rk - Excel internal formatted number
*                          lpbf - Is this a floating point value?
*
* Return:  double - return value
***********************************************************************/
double FP_API xl_NumFromRk(long rk, BOOL FAR *lpbf)
{
  double num;

  if (rk & 0x02)
  {
    //int
    num = (double)(rk >> 2);
    *lpbf = FALSE;
  }
  else
  {
    //hi words of IEEE num
    *((LPLONG)&num+1) = rk & 0xFFFFFFFC;
    *((LPLONG)&num) = 0;
    *lpbf = TRUE;
  }
  if (rk & 0x01)
  {
    //divide by 100
    num /= 100;
    *lpbf = TRUE;
  }

  return num;
}

/***********************************************************************
* Name:    xl_TimeToHMS - Convert from Excel's date/time format to Spread's
*                         time format.
*
* Usage:   BOOL FP_API xl_TimeToHMS(double dbl, LPSS_TIME lpssTime)
*                          dbl - Excel date/time formatted number
*                          lpssTime - Spread time format (return value)
*
* Return:  bool - 
***********************************************************************/
BOOL FP_API xl_TimeToHMS(double dbl, LPSS_TIME lpssTime)
{
  BOOL   Ret = FALSE;

  double d = dbl*(double)24.0;

  lpssTime->nHour = (int)d;
  d -= (double)lpssTime->nHour;

  d *= (double)60.0;
  if (d == 60)
  {
    lpssTime->nHour++;
    lpssTime->nMinute = 0;
    d -= (double)60.0F;
  }
  else
  {
    lpssTime->nMinute = (int)d;
    d -= (double)lpssTime->nMinute;
  }

  d *= (double)60.0;
  lpssTime->nSecond = (int)(d+0.5);

  if (lpssTime->nSecond == 60)
  {
    lpssTime->nSecond = 0;
    lpssTime->nMinute++;
    if (lpssTime->nMinute == 60)
    {
      lpssTime->nMinute = 0;
      lpssTime->nHour++;
      if (lpssTime->nHour == 24)
        lpssTime->nHour = 0;
    }
  }
  return Ret;
}

/***********************************************************************
* Name:    xl_GetCoord - Convert from Excel Row/Col reference structure
*                        to Spread string format (A1 or R1C1).
*
* Usage:   BOOL FP_API xl_GetCoord(LPSPREADSHEET lpSS, LPRWCO lprc,
*                                  short sRefStyle, LPyCELLVAL lpcell,
*                                  LPTSTR lpstr)
*                        lpSS - pointer to Spread structure
*                        lprc - pointer to Excel Row/Col structure
*                        sRefStyle - Spread reference style
*                        lpcell - pointer to cell structure
*                        lpstr - resulting string.
*
* Return:  bool - 
***********************************************************************/
BOOL FP_API xl_GetCoord(LPSPREADSHEET lpSS, LPRWCO lprc, short sRefStyle, LPyCELLVAL lpcell, LPTSTR lpstr)
{
  short  i;
  TCHAR  acCol[7] = {0,0,0,0,0,0,0};

  if (sRefStyle == SS_REFSTYLE_A1 || sRefStyle == SS_REFSTYLE_DEFAULT)
  {
    SSSetRefStyle(lpSS->lpBook->hWnd, SS_REFSTYLE_A1);
    memset(acCol, 0, 7*sizeof(TCHAR));
    if (lprc->col == 0)
      acCol[0] = (TCHAR)'A';
    else
    {
      long  lCol = lprc->col;
      short nCoordPos = 0;

      for (i=0;i<7;i++)
      {
        signed char cVal;
        int         power = 6-i;
        double      lPow = pow((float)26, power);

        if (lPow > lCol && power > 0)
          continue;

        if (power == 0)
        {
          cVal = (signed char)lCol;
          lCol = 0;
        }
        else
        {
          lPow = pow((float)26, power);
          cVal = (signed char)(lCol/lPow -1);
//          cVal = power-1;
          lCol -= (cVal+1)*26;
        }

        acCol[nCoordPos++] = (TCHAR)(cVal+(TCHAR)'A');
        
/*
        else if (lPow < lCol)
        {
          cVal = (signed char)(((signed char)lCol)/(int)lPow);
          cVal--;
          lCol -= 26 * cVal;
        }
        else if (lPow == lCol)
        {
          cVal = 25;
          lCol -= 26 * ((signed char)(((signed char)lCol)/(int)lPow));
        }
        if (cVal)
          acCol[nCoordPos++] = (TCHAR)(cVal+(TCHAR)'A');
*/
      }
    }

	if (lprc->fRwRel && lprc->fColRel) //A1
      _stprintf(lpstr, _T("%#s%d"), acCol, lprc->row+1);
    else if (lprc->fRwRel && !lprc->fColRel) //$A1
      _stprintf(lpstr, _T("$%#s%d"), acCol, lprc->row+1);
    else if (!lprc->fRwRel && lprc->fColRel) //A$1
      _stprintf(lpstr, _T("%#s$%d"), acCol, lprc->row+1);
    else if (!lprc->fRwRel && !lprc->fColRel) //$A$1
      _stprintf(lpstr, _T("$%#s$%d"), acCol, lprc->row+1);
  }
  else //R1C1
  {
    
    SSSetRefStyle(lpSS->lpBook->hWnd, SS_REFSTYLE_R1C1);

    if (lprc->fRwRel && lprc->fColRel) //R[1]C[1]
      _stprintf(lpstr, _T("R[%d]C[%d]"), lprc->row-(lpcell?lpcell->wRow:0), lprc->col-(lpcell?lpcell->wCol:0));
    else if (lprc->fRwRel && !lprc->fColRel) //R1C[1]
      _stprintf(lpstr, _T("R[%d]C%d"), lprc->row-(lpcell?lpcell->wRow:0), lprc->col+1);
    else if (!lprc->fRwRel && lprc->fColRel) //R[1]C1
      _stprintf(lpstr, _T("R%dC[%d]"), lprc->row+1, lprc->col-(lpcell?lpcell->wCol:0));
    else if (!lprc->fRwRel && !lprc->fColRel) //R1C1
      _stprintf(lpstr, _T("R%dC%d"), lprc->row+1, lprc->col+1);
  } //else       

  return 0;
}

#ifdef SS_V70
BOOL FP_API xl_GetCoord3D(LPSPREADSHEET lpSS, LPSSXL lpss, BOOL fFirstSheet, LPRWCO lprc, short sRefStyle, short ixti, LPyCELLVAL lpcell, LPTSTR lpstr)
{
  TCHAR         sheetNameBuffer[100];
  TCHAR         coordBuffer[100];
  LPEXTERNSHEET lpExtSheet = (LPEXTERNSHEET)tbGlobalLock(lpss->ghEXTERNSHEETs);
  LPEXTERNREF   lpExtRef = (LPEXTERNREF)tbGlobalLock(lpExtSheet->ghEXTERNREFs);
  LPTSTR        lptstrFileName = NULL;
  LPTSTR        lptstr = NULL;
  BOOL          fCrossSheetRef = FALSE;

  if (lpExtRef != NULL && ixti < lpExtSheet->dwEXTERNREFCount)
  {
   if (lpExtRef[ixti].fileName != NULL)
      lptstrFileName = (LPTSTR)tbGlobalLock(lpExtRef[ixti].fileName);
    
    fCrossSheetRef = TRUE;
    
    if (fFirstSheet)
    {
      lptstr = (LPTSTR)tbGlobalLock(lpExtRef[ixti].startSheet);
      if (lptstr == NULL || lstrlen(lptstr) == 0)
      {
        tbGlobalUnlock(lpExtSheet->ghEXTERNREFs);
        tbGlobalUnlock(lpss->ghEXTERNSHEETs);
        return 1;
      }
      _tcscpy(sheetNameBuffer, lptstr);
      tbGlobalUnlock(lpExtRef[ixti].startSheet);
    }
    else
    {
//    lptstr = (LPTSTR)tbGlobalLock(lpExtRef[ixti].endSheet);
//    if (lptstr == NULL || lstrlen(lptstr) == 0)
//    {
//      tbGlobalUnlock(lpExtSheet->ghEXTERNREFs);
//      tbGlobalUnlock(lpss->ghEXTERNSHEETs);
//      return 1;
//    }
//    _tcscpy(sheetNameBuffer, lptstr);
//    tbGlobalUnlock(lpExtRef[ixti].endSheet);
    }
  }

  xl_GetCoord(lpSS, lprc, sRefStyle, lpcell, coordBuffer);

  if (fFirstSheet && fCrossSheetRef)
  {
    if (lptstrFileName != NULL)
    {
      _stprintf(lpstr, _T("'%s%s'!%s"), lptstrFileName, sheetNameBuffer, coordBuffer);
//      _stprintf(lpstr, _T("%s%s!%s"), lptstrFileName, sheetNameBuffer, coordBuffer);
      tbGlobalUnlock(lpExtRef[ixti].fileName);
    }
    else
      _stprintf(lpstr, _T("'%s'!%s"), sheetNameBuffer, coordBuffer);
//      _stprintf(lpstr, _T("%s!%s"), sheetNameBuffer, coordBuffer);
  }
  else
  {
    _tcscpy(lpstr, coordBuffer);
  }

  tbGlobalUnlock(lpExtSheet->ghEXTERNREFs);
  tbGlobalUnlock(lpss->ghEXTERNSHEETs);
  
  return 0;  
}
#endif

/***********************************************************************
* Name:    xlGetDigitsLeft - Return the number of digits to the left of the
*                            decimal in a floating point number.
*
* Usage:   short FP_API xlGetDigitsLeft(double dbl)
*                         dbl - floating point number.
*
* Return:  short - number of digits to the left of the number (dbl)
***********************************************************************/
short FP_API xlGetDigitsLeft(double dbl)
{
  short sCount = 0;
  
  while (dbl >= 1)
  {
    dbl /= 10;
    sCount++;
  }
  
  return sCount;
}

/***********************************************************************
* Name:    xlGetDigitsRight - Return the number of digits to the right of the
*                             decimal in a floating point number.
*
* Usage:   short FP_API xlGetDigitsRight(double dbl)
*                         dbl - floating point number.
*
* Return:  short - number of digits to the right of the number (dbl)
***********************************************************************/
short FP_API xlGetDigitsRight(double dbl)
{
  short sCount = 0;
  short sInteger = (short)dbl;
  double dFractional = dbl - (double)sInteger;
  
  while (dFractional > 0)
  {
    dFractional = 0;
    sCount++;
    dFractional = dFractional * 10;
  }
  
  return sCount;
}

/***********************************************************************
* Name:    xl_FindShrFmla - Go through the shared formula list and determine
*                           if the specified cell's formula contains a 
*                           shared formula. If it does, then append the shared
*                           formula to the cell's formula.
*
* Usage:   short FP_API xl_FindShrFmla(LPSSXL lpss, LPyCELLVAL lpcell)
*                         lpss - pointer to the Excel structure.
*                         lpcell - the cell to investigate.
*
* Return:  short - Success or failure.
***********************************************************************/
short FP_API xl_FindShrFmla(LPSSXL lpss, LPyCELLVAL lpcell)
{
  LPyySHRFMLA lpShrFmla = NULL;
  short       sRet = 1;

  if (lpss->ghShrFmla)
  {
    DWORD dw;
    lpShrFmla = (LPyySHRFMLA)tbGlobalLock(lpss->ghShrFmla);
    for (dw=0; dw<lpss->dwShrFmlaCount; dw++)
    {
      if (lpcell->wCol >= lpShrFmla[dw].colFirst && lpcell->wCol <= lpShrFmla[dw].colLast &&
          lpcell->wRow >= lpShrFmla[dw].rwFirst && lpcell->wRow <= lpShrFmla[dw].rwLast)
      {
        LPyyFORMULA lpFmla = (LPyyFORMULA)tbGlobalLock(lpcell->ghCell);
        lpFmla->wFormulaLen = lpShrFmla[dw].cce;
        lpFmla->ghFormula = lpShrFmla[dw].ghFmla;
        tbGlobalUnlock(lpcell->ghCell);
        sRet = 0;
        break;
      }
    }
    tbGlobalUnlock(lpss->ghShrFmla);
  }
  
  return sRet;
}

/***********************************************************************
* Name:    xl_ParsePicFormat - Parse the Spread Pic format and build an
*                              Excel number format string that matches.
*                              NOTE: Excel does not support any non-numberic
*                                    formatting
*
* Usage:   short FP_API xl_ParsePicFormat(LPSPREADSHEET lpSS, LPSTR lpstrSrc, LPSTR lpstrDest)
*                         lpSS - Pointer to the Spread structure
*                         lpstrSrc - Spread Pic format string
*                         lpstrDest - resulting Excel number format string.
*
* Return:  short - Success or failure.
***********************************************************************/
short FP_API xl_ParsePicFormat(LPSPREADSHEET lpSS, LPTSTR lpstrSrc, LPTSTR lpstrDest)
{
  int   i;
  short sBufferPos = 0;
  short sLen = (short)lstrlen(lpstrSrc);
  BOOL  fIgnore = FALSE;

  for (i=0; i<sLen; i++)
  {
    switch(lpstrSrc[i])
    {
      case '9':
        lpstrDest[sBufferPos++] = '0';
      break;
      case '\\':
        lpstrDest[sBufferPos++] = '\\';
        lpstrDest[sBufferPos++] = '\\';
      break;
      case 'X':
      case 'x':
      case 'A':
      case 'a':
      case 'N':
      case 'n':
      case 'U':
      case 'u':
      case 'L':
      case 'l':
      case 'H':
      case 'h':
        xl_LogFile(lpSS->lpBook, LOG_PICTOKENNOTSUPPORTED, 0, 0, (TCHAR *)&(lpstrSrc[i]));
        fIgnore = TRUE;
      break;
      default:
        fIgnore = TRUE;
      break;
    } //switch
    if (fIgnore)
      return 1;
  } //for

  lpstrDest[sBufferPos] = 0;

  return 0;
}
