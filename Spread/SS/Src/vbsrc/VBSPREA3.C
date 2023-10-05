/*********************************************************
* VBSPREA3.C     
*
* Copyright (C) 1991-1993 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/

#include <windows.h>
//#include <vbapi.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "..\sssrc\spread\spread.h"
#include "..\sssrc\spread\ss_alloc.h"
#include "..\sssrc\spread\ss_cell.h"
#include "..\sssrc\spread\ss_heap.h"
#include "..\sssrc\spread\ss_save.h"
#include "vbmisc.h"
#include "vbdattim.h"
#include "vbspread.h"

LPSS_CELLTYPE SetCellTypePropDate(HWND hWnd, LPVBSPREAD lpSpread,
                                  short Property, long lParam,
                                  LPSS_CELLTYPE lpCellTypeTemp,
                                  LPSS_CELLTYPE lpCT);
LPSS_CELLTYPE SetCellTypePropEdit(HWND hWnd, LPVBSPREAD lpSpread,
                                  short Property, long lParam,
                                  LPSS_CELLTYPE lpCellTypeTemp,
                                  LPSS_CELLTYPE lpCT);
LPSS_CELLTYPE SetCellTypePropFloat(HWND hWnd, LPVBSPREAD lpSpread,
                                   short Property, long lParam,
                                   LPSS_CELLTYPE lpCellTypeTemp,
                                   LPSS_CELLTYPE lpCT);
LPSS_CELLTYPE SetCellTypePropInt(HWND hWnd, LPVBSPREAD lpSpread,
                                 short Property, long lParam,
                                 LPSS_CELLTYPE lpCellTypeTemp,
                                 LPSS_CELLTYPE lpCT);
LPSS_CELLTYPE SetCellTypePropPic(HWND hWnd, LPVBSPREAD lpSpread,
                                 short Property, long lParam,
                                 LPSS_CELLTYPE lpCellTypeTemp,
                                 LPSS_CELLTYPE lpCT);
LPSS_CELLTYPE SetCellTypePropText(HWND hWnd, LPVBSPREAD lpSpread,
                                  short Property, long lParam,
                                  LPSS_CELLTYPE lpCellTypeTemp,
                                  LPSS_CELLTYPE lpCT);
LPSS_CELLTYPE SetCellTypePropTime(HWND hWnd, LPVBSPREAD lpSpread,
                                  short Property, long lParam,
                                  LPSS_CELLTYPE lpCellTypeTemp,
                                  LPSS_CELLTYPE lpCT);
LPSS_CELLTYPE SetCellTypePropCombo(HWND hWnd, LPVBSPREAD lpSpread,
                                   short Property, long lParam,
                                   LPSS_CELLTYPE lpCellTypeTemp,
                                   LPSS_CELLTYPE lpCT);
LPSS_CELLTYPE SetCellTypePropPict(HWND hWnd, LPVBSPREAD lpSpread,
                                  short Property, long lParam,
                                  LPSS_CELLTYPE lpCellTypeTemp,
                                  LPSS_CELLTYPE lpCT);
LPSS_CELLTYPE SetCellTypePropButton(HWND hWnd, LPVBSPREAD lpSpread,
                                    short Property, long lParam,
                                    LPSS_CELLTYPE lpCellTypeTemp,
                                    LPSS_CELLTYPE lpCT);
LPSS_CELLTYPE SetCellTypePropCheck(HWND hWnd, LPVBSPREAD lpSpread,
                                   short Property, long lParam,
                                   LPSS_CELLTYPE lpCellTypeTemp,
                                   LPSS_CELLTYPE lpCT);

#define SS_VALIGNTRANSLATE(lParam) (lParam == 1 ? SSS_ALIGN_BOTTOM : (lParam == 2 ? SSS_ALIGN_VCENTER : SSS_ALIGN_TOP))
#define SS_HALIGNTRANSLATE(lParam) (lParam == 1 ? ES_RIGHT : (lParam == 2 ? ES_CENTER : ES_LEFT))


BOOL SetCellType(hWnd, lpSpread)

HWND        hWnd;
LPVBSPREAD  lpSpread;
{
SS_CELLTYPE CellType;
DATEFORMAT  DateFormat;
HANDLE      hDataTemp;
TB_DATE     DateMin;
TB_DATE     DateMax;
long        lStyle;
short       iStyle;
SS_COORD    x;
SS_COORD    y;
LPSTR       Data;
LPSTR       lpDefText;
LPSTR       lpDataTemp;
LPSTR       lpText = NULL;
LPSTR       lpPict = NULL;
LPSTR       lpPictUp = NULL;
LPSTR       lpPictDown = NULL;
LPSTR       lpPictFocusUp = NULL;
LPSTR       lpPictFocusDown = NULL;
LPSTR       lpPictGray = NULL;
LPSTR       lpPictFocusGray = NULL;
#ifdef  BUGS
// Bug-002
char        Buffer[330];
#else
char        Buffer[30];
#endif
BOOL        Ret = TRUE;

VBSSVALCOLROW(lpSpread);

switch (lpSpread->CellType)
   {
#ifndef SS_NOCT_DATE
   case 0:                 // Date
      lStyle = ES_LEFT;

      DateFormat.nFormat = lpSpread->TypeDateFormat.nFormat;
      DateFormat.bCentury = lpSpread->TypeDateFormat.bCentury;
      DateFormat.cSeparator = (char)lpSpread->TypeDateFormat.cSeparator;
      DateFormat.bSpin = lpSpread->bSpin;

      DateMin.nDay = lpSpread->TypeDateMin.nDay;
      DateMin.nMonth = lpSpread->TypeDateMin.nMonth;
      DateMin.nYear = lpSpread->TypeDateMin.nYear;

      DateMax.nDay = lpSpread->TypeDateMax.nDay;
      DateMax.nMonth = lpSpread->TypeDateMax.nMonth;
      DateMax.nYear = lpSpread->TypeDateMax.nYear;

      SSSetTypeDate(hWnd, &CellType, 0, &DateFormat, &DateMin, &DateMax);

      break;
#endif

   case 1:                 // Edit
      lStyle = ES_LEFT | ES_AUTOHSCROLL;

      if (lpSpread->TypeEditPassword)
         lStyle |= ES_PASSWORD;

      if (lpSpread->TypeEditMultiLine)
         {
         lStyle |= ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN;
         lStyle &= ~ES_AUTOHSCROLL;
         }

      SSSetTypeEdit(hWnd, &CellType, lStyle, lpSpread->nMaxEditLen,
                    lpSpread->TypeEditCharSet,
                    (char)lpSpread->TypeEditCharCase);

      break;

   case 2:                 // Float
      lStyle = ES_RIGHT;

      if (lpSpread->TypeFloatSeparator)
         lStyle |= FS_SEPARATOR;

      if (lpSpread->TypeFloatMoney)
         lStyle |= FS_MONEY;

      StrPrintf(Buffer, "%.*f", 2, lpSpread->TypeFloatMax);

      SSSetTypeFloat(hWnd, &CellType, lStyle,
                     (short)((long)StrChr(Buffer, '.') - (long)(LPSTR)Buffer),
                     lpSpread->TypeFloatDecimalPlaces,
                     lpSpread->TypeFloatMin,
                     lpSpread->TypeFloatMax);

      break;

#ifndef SS_NOCT_INT
   case 3:                 // Integer
      lStyle = ES_RIGHT;

      if (lpSpread->bSpin)
         lStyle |= IS_SPIN;

      SSSetTypeIntegerExt(hWnd, &CellType, lStyle,
                          lpSpread->TypeIntegerMin,
                          lpSpread->TypeIntegerMax,
                          lpSpread->fIntegerSpinWrap,
                          lpSpread->lIntegerSpinInc);

      break;
#endif

#ifndef SS_NOCT_PIC
   case 4:                 // Pic
      lStyle = ES_LEFT;

      if (lpSpread->hTypePicMask)
         {
         Data = (LPSTR)GlobalLock(lpSpread->hTypePicMask);

         if (lpSpread->hTypePicDefText)
            {
            lpDefText = (LPSTR)GlobalLock(lpSpread->hTypePicDefText);

            if (hDataTemp = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                        lstrlen(Data) + 1 +
                                        lstrlen(lpDefText) + 1))
               {
               lpDataTemp = (LPSTR)GlobalLock(hDataTemp);
               lstrcpy(lpDataTemp, Data);
               StrCat(lpDataTemp, "\n");
               StrCat(lpDataTemp, lpDefText);
               Data = lpDataTemp;
               }

            GlobalUnlock(lpSpread->hTypePicDefText);
            }
         }
      else
         Data = "";

      if (lpSpread->BlockMode)
         {
         LPSPREADSHEET lpSS;
         SS_COORD      Col;
         SS_COORD      Row;
         SS_COORD      Col2;
         SS_COORD      Row2;

         lpSS = SS_Lock(hWnd);
         VBSSVALCOL2ROW2(lpSpread);

         Col = lpSpread->Col;
         Col2 = lpSpread->Col2;
         Row = lpSpread->Row;
         Row2 = lpSpread->Row2;

         SS_AdjustCellRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);

         for (x = Col; x <= Col2; x++)
            for (y = Row; y <= Row2; y++)
               {
               SSSetTypePic(hWnd, &CellType, lStyle, Data);
               Ret = SSSetCellType(hWnd, x, y, &CellType);
               }

         SS_Unlock(hWnd);
         }
      else
         SSSetTypePic(hWnd, &CellType, lStyle, Data);

      if (lpSpread->hTypePicMask && lpSpread->hTypePicDefText)
         {
         GlobalUnlock(hDataTemp);
         GlobalFree(hDataTemp);
         }

      if (lpSpread->hTypePicMask)
         GlobalUnlock(lpSpread->hTypePicMask);

      if (lpSpread->BlockMode)
         return (Ret);

      break;
#endif

   case 5:                 // StaticText
      iStyle = SS_TEXT_LEFT;

      if (lpSpread->TypeTextJustifyVert == 0)
         iStyle |= SS_TEXT_BOTTOM;
      else if (lpSpread->TypeTextJustifyVert == 1)
         iStyle |= SS_TEXT_VCENTER;
      else if (lpSpread->TypeTextJustifyVert == 2)
         iStyle |= SS_TEXT_TOP;

      if (lpSpread->TypeTextWordWrap)
         iStyle |= SS_TEXT_WORDWRAP;

      if (lpSpread->TypeTextShadow)
         iStyle |= SS_TEXT_SHADOW;

      if (lpSpread->fTextPrefix)
         iStyle |= SS_TEXT_PREFIX;

      if (lpSpread->fTextShadowIn)
         iStyle |= SS_TEXT_SHADOWIN;

      SSSetTypeStaticText(hWnd, &CellType, iStyle);
      break;

#ifndef SS_NOCT_TIME
   case 6:                 // Time
      {
      TIMEFORMAT TimeFormat;
      TIME       TimeMin;
      TIME       TimeMax;

      lStyle = ES_LEFT;

      TimeFormat.bSeconds = lpSpread->TypeTimeFormat.bSeconds;
      TimeFormat.b24Hour = lpSpread->TypeTimeFormat.b24Hour;
      TimeFormat.cSeparator = (char)lpSpread->TypeTimeFormat.cSeparator;
      TimeFormat.bSpin = lpSpread->bSpin;

      TimeMin.nHour = lpSpread->TypeTimeMin.nHour;
      TimeMin.nMinute = lpSpread->TypeTimeMin.nMinute;
      TimeMin.nSecond = lpSpread->TypeTimeMin.nSecond;

      TimeMax.nHour = lpSpread->TypeTimeMax.nHour;
      TimeMax.nMinute = lpSpread->TypeTimeMax.nMinute;
      TimeMax.nSecond = lpSpread->TypeTimeMax.nSecond;

      SSSetTypeTime(hWnd, &CellType, lStyle, &TimeFormat, &TimeMin, &TimeMax);
      }

      break;
#endif

#ifndef SS_NOCT_BUTTON
   case 7:                 // Button
      lStyle = 0;

      if (lpSpread->TypeButtonAlign == 0)
         lStyle |= SBS_TEXTBOT_PICTTOP;
      else if (lpSpread->TypeButtonAlign == 1)
         lStyle |= SBS_TEXTTOP_PICTBOT;
      else if (lpSpread->TypeButtonAlign == 2)
         lStyle |= SBS_TEXTLEFT_PICTRIGHT;
      else if (lpSpread->TypeButtonAlign == 3)
         lStyle |= SBS_TEXTRIGHT_PICTLEFT;

      if (lpSpread->hTypeButtonText)
         lpText = (LPSTR)GlobalLock(lpSpread->hTypeButtonText);

      if (lpSpread->hTypeButtonPictureName)
         lpPict = (LPSTR)&lpSpread->hTypeButtonPictureName;
      else
         lpPict = 0;

      if (lpSpread->hTypeButtonPictureDownName)
         lpPictDown = (LPSTR)&lpSpread->hTypeButtonPictureDownName;
      else
         lpPictDown = 0;

      SSSetTypeButton(hWnd, &CellType, lStyle, lpText, lpPict,
                      lpSpread->TypeButtonPictureType, lpPictDown,
                      lpSpread->TypeButtonPictureDownType,
                      lpSpread->TypeButtonType, lpSpread->TypeButtonShadowSize,
                      &lpSpread->TypeButtonColor);

      if (lpSpread->hTypeButtonText)
         GlobalUnlock(lpSpread->hTypeButtonText);

      break;
#endif

#ifndef SS_NOCT_COMBO
   case 8:                 // ComboBox
      SSSetTypeComboBoxEx(hWnd, &CellType,
                          (long)(lpSpread->fComboEditable ?
                          SS_CB_DROPDOWN : 0), NULL, lpSpread->dComboMaxDrop,
                          lpSpread->dComboMaxEditLen, lpSpread->dComboWidth, 0);
      break;
#endif

   case 9:                 // Picture
      if (!lpSpread->BlockMode && SSGetCellType(hWnd, lpSpread->Col,
          lpSpread->Row, &CellType) && CellType.Type == SS_TYPE_PICTURE)
         return (TRUE);

      /*
      if (lpSpread->hTypePictPal)
         SSSetTypePictureHandle(hWnd, &CellType, lpSpread->TypePictStyle,
                                lpSpread->hTypePictPicture,
                                lpSpread->hTypePictPal, FALSE);
      else
         SSSetTypePicture(hWnd, &CellType, lpSpread->TypePictStyle,
                          (LPSTR)&lpSpread->hTypePictPicture);
      */
      SSSetTypePicture(hWnd, &CellType, 0, 0);

      break;

#ifndef SS_NOCT_CHECK
   case 10:                // CheckBox
      lStyle = 0;

      if (lpSpread->fCheckCenter)
         lStyle |= BS_CENTER;
      
      if (0 == lpSpread->nCheckType)
         lStyle |= BS_AUTOCHECKBOX;
      else if (1 == lpSpread->nCheckType)
         lStyle |= BS_AUTO3STATE;

      if (lpSpread->hCheckText)
         lpText = (LPSTR)GlobalLock(lpSpread->hCheckText);

      lpPictUp = 0;
      lpPictDown = 0;
      lpPictFocusUp = 0;
      lpPictFocusDown = 0;
      lpPictGray = 0;
      lpPictFocusGray = 0;

      if (lpSpread->hCheckPictUp)
         lpPictUp = (LPSTR)&lpSpread->hCheckPictUp;

      if (lpSpread->hCheckPictDown)
         lpPictDown = (LPSTR)&lpSpread->hCheckPictDown;

      if (lpSpread->hCheckPictFocusUp)
         lpPictFocusUp = (LPSTR)&lpSpread->hCheckPictFocusUp;

      if (lpSpread->hCheckPictFocusDown)
         lpPictFocusDown = (LPSTR)&lpSpread->hCheckPictFocusDown;

      if (lpSpread->hCheckPictGray)
         lpPictGray = (LPSTR)&lpSpread->hCheckPictGray;

      if (lpSpread->hCheckPictFocusGray)
         lpPictFocusGray = (LPSTR)&lpSpread->hCheckPictFocusGray;

      SSSetTypeCheckBox(hWnd, &CellType, lStyle, lpText,
                        lpPictUp, lpSpread->dCheckPictUpType,
                        lpPictDown, lpSpread->dCheckPictDownType,
                        lpPictFocusUp, lpSpread->dCheckPictFocusUpType,
                        lpPictFocusDown, lpSpread->dCheckPictFocusDownType,
                        lpPictGray, lpSpread->dCheckPictGrayType,
                        lpPictFocusGray, lpSpread->dCheckPictFocusGrayType);

      if (lpSpread->hCheckText)
         GlobalUnlock(lpSpread->hCheckText);

      break;
#endif

#ifndef SS_NOCT_OWNERDRAW
   case 11:                 // OwnerDraw
      SSSetTypeOwnerDraw(hWnd, &CellType, lpSpread->lOwnerDrawStyle);
      break;
#endif

   default:
      return (1);
   }

if (lpSpread->BlockMode)
   {
   VBSSVALCOL2ROW2(lpSpread);

   Ret = SSSetCellTypeRange(hWnd, lpSpread->Col, lpSpread->Row,
                            lpSpread->Col2, lpSpread->Row2, &CellType);
   }
else
   Ret = SSSetCellType(hWnd, lpSpread->Col, lpSpread->Row, &CellType);

return (Ret);
}


BOOL SetCellTypeProperties(hWnd, lpSpread, Property, lParam)

HWND          hWnd;
LPVBSPREAD    lpSpread;
short         Property;
long          lParam;
{
LPSS_CELLTYPE lpCellType = NULL;
SS_CELLTYPE   CT;
SS_CELLTYPE   CellTypeTemp;
HANDLE        hDataTemp = 0;
BOOL          Ret = TRUE;
SS_COORD      x;
SS_COORD      x1;
SS_COORD      x2;
SS_COORD      y;
SS_COORD      y1;
SS_COORD      y2;

VBSSVALCOLROW(lpSpread);

x1 = lpSpread->Col;
x2 = lpSpread->Col;
y1 = lpSpread->Row;
y2 = lpSpread->Row;

if (lpSpread->BlockMode)
   {
   VBSSVALCOL2ROW2(lpSpread);

   x2 = lpSpread->Col2;

   if (x1 == -1 || x2 == -1)
      {
      x1 = -1;
      x2 = -1;
      }

   y2 = lpSpread->Row2;

   if (y1 == -1 || y2 == -1)
      {
      y1 = -1;
      y2 = -1;
      }
   }

x = max(x, 0);
y = max(y, 0);

for (x = x1; x <= x2 && Ret; x++)
   for (y = y1; y <= y2 && Ret; y++)
      {
      if (!SSGetCellType(hWnd, x, y, &CellTypeTemp))
         return (FALSE);

      /*
      if (x != SS_ALLCOLS && y != SS_ALLROWS)
         {
         LPSPREADSHEET lpSS;
         LPSS_CELL     lpCell;

         lpSS = SS_Lock(hWnd);

         if (!(lpCell = SS_LockCellItem(lpSS, x, y)) ||
             !lpCell->hCellType)
            {
            SS_CopyCellType(lpSS, &CT, &CellTypeTemp);
            _fmemcpy(&CellTypeTemp, &CT, sizeof(SS_CELLTYPE));
            }

         SS_Unlock(hWnd);
         }
      */

      switch (Property)
         {
         case IPROP_SPREAD_TYPEHALIGN:
#ifdef IPROP_SPREAD_TYPEVALIGN
         case IPROP_SPREAD_TYPEVALIGN:
#endif
            lParam = (long)(short)lParam;

            if (CellTypeTemp.Type == SS_TYPE_DATE)
               lpCellType = SetCellTypePropDate(hWnd, lpSpread, Property,
                                                lParam, &CellTypeTemp, &CT);

            else if (CellTypeTemp.Type == SS_TYPE_EDIT)
               lpCellType = SetCellTypePropEdit(hWnd, lpSpread, Property,
                                                lParam, &CellTypeTemp, &CT);

            else if (CellTypeTemp.Type == SS_TYPE_FLOAT)
               lpCellType = SetCellTypePropFloat(hWnd, lpSpread, Property,
                                                 lParam, &CellTypeTemp, &CT);

            else if (CellTypeTemp.Type == SS_TYPE_INTEGER)
               lpCellType = SetCellTypePropInt(hWnd, lpSpread, Property,
                                               lParam, &CellTypeTemp, &CT);

            else if (CellTypeTemp.Type == SS_TYPE_PIC)
               lpCellType = SetCellTypePropPic(hWnd, lpSpread, Property,
                                               lParam, &CellTypeTemp, &CT);

            else if (CellTypeTemp.Type == SS_TYPE_STATICTEXT)
               lpCellType = SetCellTypePropText(hWnd, lpSpread, Property,
                                                lParam, &CellTypeTemp, &CT);

            else if (CellTypeTemp.Type == SS_TYPE_TIME)
               lpCellType = SetCellTypePropTime(hWnd, lpSpread, Property,
                                                lParam, &CellTypeTemp, &CT);

            else if (CellTypeTemp.Type == SS_TYPE_PICTURE)
               lpCellType = SetCellTypePropPict(hWnd, lpSpread, Property,
                                                lParam, &CellTypeTemp, &CT);

            else if (CellTypeTemp.Type == SS_TYPE_COMBOBOX)
               lpCellType = SetCellTypePropCombo(hWnd, lpSpread, Property,
                                                 lParam, &CellTypeTemp, &CT);

            else if (CellTypeTemp.Type == SS_TYPE_BUTTON)
               lpCellType = SetCellTypePropButton(hWnd, lpSpread, Property,
                                                  lParam, &CellTypeTemp, &CT);

            else if (CellTypeTemp.Type == SS_TYPE_CHECKBOX)
               lpCellType = SetCellTypePropCheck(hWnd, lpSpread, Property,
                                                 lParam, &CellTypeTemp, &CT);

            break;

#ifdef IPROP_SPREAD_TYPESPIN
         case IPROP_SPREAD_TYPESPIN:
            lParam = (long)(short)lParam;
            lpSpread->bSpin = (short)lParam;

            if (CellTypeTemp.Type == SS_TYPE_DATE)
               lpCellType = SetCellTypePropDate(hWnd, lpSpread, Property,
                                                lParam, &CellTypeTemp, &CT);

            else if (CellTypeTemp.Type == SS_TYPE_INTEGER)
               lpCellType = SetCellTypePropInt(hWnd, lpSpread, Property,
                                               lParam, &CellTypeTemp, &CT);

            else if (CellTypeTemp.Type == SS_TYPE_TIME)
               lpCellType = SetCellTypePropTime(hWnd, lpSpread, Property,
                                                lParam, &CellTypeTemp, &CT);

            break;
#endif

#ifndef SS_NOCT_DATE
#ifdef IPROP_SPREAD_DATEMIN
         case IPROP_SPREAD_DATEMIN:
#endif
#ifdef IPROP_SPREAD_DATEMAX
         case IPROP_SPREAD_DATEMAX:
#endif
#ifdef IPROP_SPREAD_DATECENTURY
         case IPROP_SPREAD_DATECENTURY:
#endif
#ifdef IPROP_SPREAD_DATEFORMAT
         case IPROP_SPREAD_DATEFORMAT:
#endif
#ifdef IPROP_SPREAD_DATESEPARATOR
         case IPROP_SPREAD_DATESEPARATOR:
#endif
            if (CellTypeTemp.Type == SS_TYPE_DATE)
               lpCellType = SetCellTypePropDate(hWnd, lpSpread, Property,
                                                lParam, &CellTypeTemp, &CT);

            break;
#endif
         case IPROP_SPREAD_EDITLEN:
            if (CellTypeTemp.Type == SS_TYPE_EDIT)
               lpCellType = SetCellTypePropEdit(hWnd, lpSpread, Property,
                                                lParam, &CellTypeTemp, &CT);
            else if (CellTypeTemp.Type == SS_TYPE_COMBOBOX)
               lpCellType = SetCellTypePropCombo(hWnd, lpSpread, Property,
                                                 lParam, &CellTypeTemp, &CT);

            break;

#ifdef IPROP_SPREAD_EDITCASE
         case IPROP_SPREAD_EDITCASE:
#endif
#ifdef IPROP_SPREAD_EDITCHARCASE
         case IPROP_SPREAD_EDITCHARCASE:
#endif
#ifdef IPROP_SPREAD_EDITCHARSET
         case IPROP_SPREAD_EDITCHARSET:
#endif
#ifdef IPROP_SPREAD_EDITJUSTIFY
         case IPROP_SPREAD_EDITJUSTIFY:
#endif
#ifdef IPROP_SPREAD_EDITPASSWORD
         case IPROP_SPREAD_EDITPASSWORD:
#endif
#ifdef IPROP_SPREAD_EDITMULTILINE
         case IPROP_SPREAD_EDITMULTILINE:
#endif
#ifdef SS_GP
         case IPROP_SPREAD_EDITEXPWIDTH:
         case IPROP_SPREAD_EDITEXPROWCNT:
#endif
            if (CellTypeTemp.Type == SS_TYPE_EDIT)
               lpCellType = SetCellTypePropEdit(hWnd, lpSpread, Property,
                                                lParam, &CellTypeTemp, &CT);

            break;

         case IPROP_SPREAD_FLOATDECPLACES:
         case IPROP_SPREAD_FLOATMAX:
         case IPROP_SPREAD_FLOATMIN:
         case IPROP_SPREAD_FLOATMONEY:
         case IPROP_SPREAD_FLOATSEPARATOR:
#ifdef IPROP_SPREAD_TYPEFLOATCURRENCY
         case IPROP_SPREAD_TYPEFLOATCURRENCY:
#endif
#ifdef IPROP_SPREAD_TYPEFLOATDECIMAL
         case IPROP_SPREAD_TYPEFLOATDECIMAL:
#endif
#ifdef IPROP_SPREAD_TYPEFLOATSEPCHAR
         case IPROP_SPREAD_TYPEFLOATSEPCHAR:
#endif
            if (CellTypeTemp.Type == SS_TYPE_FLOAT)
               lpCellType = SetCellTypePropFloat(hWnd, lpSpread, Property,
                                                 lParam, &CellTypeTemp, &CT);

            break;

#ifdef IPROP_SPREAD_INTEGERMAX
         case IPROP_SPREAD_INTEGERMAX:
#endif
#ifdef IPROP_SPREAD_INTEGERMIN
         case IPROP_SPREAD_INTEGERMIN:
#endif
#ifdef IPROP_SPREAD_TYPEINTSPINWRAP
         case IPROP_SPREAD_TYPEINTSPINWRAP:
#endif
#ifdef IPROP_SPREAD_TYPEINTSPININC
         case IPROP_SPREAD_TYPEINTSPININC:
#endif
            if (CellTypeTemp.Type == SS_TYPE_INTEGER)
               lpCellType = SetCellTypePropInt(hWnd, lpSpread, Property,
                                               lParam, &CellTypeTemp, &CT);

            break;

#ifdef IPROP_SPREAD_PICMASK
         case IPROP_SPREAD_PICMASK:
#endif
#ifdef IPROP_SPREAD_PICDEFTEXT
         case IPROP_SPREAD_PICDEFTEXT:
#endif
            if (CellTypeTemp.Type == SS_TYPE_PIC)
               lpCellType = SetCellTypePropPic(hWnd, lpSpread, Property,
                                               lParam, &CellTypeTemp, &CT);

            break;

#ifdef IPROP_SPREAD_TEXTJUSTIFYHORIZ
         case IPROP_SPREAD_TEXTJUSTIFYHORIZ:
#endif
         case IPROP_SPREAD_TEXTJUSTIFYVERT:
         case IPROP_SPREAD_TEXTSHADOW:
         case IPROP_SPREAD_TEXTWORDWRAP:
#ifdef IPROP_SPREAD_TYPETEXTPREFIX
         case IPROP_SPREAD_TYPETEXTPREFIX:
#endif
#ifdef IPROP_SPREAD_TYPETEXTSHADOWIN
         case IPROP_SPREAD_TYPETEXTSHADOWIN:
#endif
            if (CellTypeTemp.Type == SS_TYPE_STATICTEXT)
               lpCellType = SetCellTypePropText(hWnd, lpSpread, Property,
                                                lParam, &CellTypeTemp, &CT);

            break;

#ifdef IPROP_SPREAD_TIMEMIN
         case IPROP_SPREAD_TIMEMIN:
#endif
#ifdef IPROP_SPREAD_TIMEMAX
         case IPROP_SPREAD_TIMEMAX:
#endif
#ifdef IPROP_SPREAD_TIMESECONDS
         case IPROP_SPREAD_TIMESECONDS:
#endif
#ifdef IPROP_SPREAD_TIMESEPARATOR
         case IPROP_SPREAD_TIMESEPARATOR:
#endif
#ifdef IPROP_SPREAD_TIME24HOUR
         case IPROP_SPREAD_TIME24HOUR:
#endif
            if (CellTypeTemp.Type == SS_TYPE_TIME)
               lpCellType = SetCellTypePropTime(hWnd, lpSpread, Property,
                                                lParam, &CellTypeTemp, &CT);

            break;

#ifndef SS_NOCT_BUTTON
         case IPROP_SPREAD_BUTTONALIGN:
         case IPROP_SPREAD_BUTTONTEXT:
         case IPROP_SPREAD_BUTTONPICTNAME:
         case IPROP_SPREAD_BUTTONPICTDOWN:
         case IPROP_SPREAD_BUTTONTYPE:
         case IPROP_SPREAD_BUTTONCOLOR:
#ifdef IPROP_SPREAD_BUTTONBORDERCOLOR
         case IPROP_SPREAD_BUTTONBORDERCOLOR:
#endif
#ifdef IPROP_SPREAD_BUTTONDARKCOLOR
         case IPROP_SPREAD_BUTTONDARKCOLOR:
#endif
#ifdef IPROP_SPREAD_BUTTONLIGHTCOLOR
         case IPROP_SPREAD_BUTTONLIGHTCOLOR:
#endif
#ifdef IPROP_SPREAD_BUTTONTEXTCOLOR
         case IPROP_SPREAD_BUTTONTEXTCOLOR:
#endif
#ifdef IPROP_SPREAD_BUTTONSHADOWSIZE
         case IPROP_SPREAD_BUTTONSHADOWSIZE:
#endif
            if (CellTypeTemp.Type == SS_TYPE_BUTTON)
               lpCellType = SetCellTypePropButton(hWnd, lpSpread, Property,
                                                  lParam, &CellTypeTemp, &CT);

            break;
#endif

#ifdef IPROP_SPREAD_TYPEPICTPICTURE
         case IPROP_SPREAD_TYPEPICTPICTURE:
#endif
#ifdef IPROP_SPREAD_TYPEPICTCENTER
         case IPROP_SPREAD_TYPEPICTCENTER:
#endif
#ifdef IPROP_SPREAD_TYPEPICTSTRETCH
         case IPROP_SPREAD_TYPEPICTSTRETCH:
#endif
#ifdef IPROP_SPREAD_TYPEPICTSCALE
         case IPROP_SPREAD_TYPEPICTSCALE:
#endif
            if (CellTypeTemp.Type == SS_TYPE_PICTURE)
               lpCellType = SetCellTypePropPict(hWnd, lpSpread, Property,
                                                lParam, &CellTypeTemp, &CT);

            break;

#ifndef SS_NOCT_COMBO
         case IPROP_SPREAD_COMBOLIST:
#ifdef IPROP_SPREAD_TYPECOMBOEDITABLE
         case IPROP_SPREAD_TYPECOMBOEDITABLE:
#endif
#ifdef SS_V30
         case IPROP_SPREAD_TYPECOMBOMAXDROP:
         case IPROP_SPREAD_TYPECOMBOWIDTH:
         case IPROP_SPREAD_COMBOHWND:
#endif
            if (CellTypeTemp.Type == SS_TYPE_COMBOBOX)
               lpCellType = SetCellTypePropCombo(hWnd, lpSpread, Property,
                                                 lParam, &CellTypeTemp, &CT);

            break;
#endif

#ifndef SS_NOCT_CHECK
#ifdef IPROP_SPREAD_CHECKCENTER
         case IPROP_SPREAD_CHECKCENTER:
#endif
#ifdef IPROP_SPREAD_CHECKTYPE
         case IPROP_SPREAD_CHECKTYPE:
#endif
#ifdef IPROP_SPREAD_CHECKTEXT
         case IPROP_SPREAD_CHECKTEXT:
#endif
#ifdef IPROP_SPREAD_CHECKTEXTALIGN
         case IPROP_SPREAD_CHECKTEXTALIGN:
#endif
#ifdef IPROP_SPREAD_CHECKPICTURE
         case IPROP_SPREAD_CHECKPICTURE:
#endif
         case IPROP_SPREAD_CHECKPICTUP:
         case IPROP_SPREAD_CHECKPICTDOWN:
         case IPROP_SPREAD_CHECKPICTFOCUSUP:
         case IPROP_SPREAD_CHECKPICTFOCUSDOWN:
         case IPROP_SPREAD_CHECKPICTGRAY:
         case IPROP_SPREAD_CHECKPICTFOCUSGRAY:
            if (CellTypeTemp.Type == SS_TYPE_CHECKBOX)
               lpCellType = SetCellTypePropCheck(hWnd, lpSpread, Property,
                                                 lParam, &CellTypeTemp, &CT);

            break;
#endif

#ifndef SS_NOCT_OWNERDRAW
         case IPROP_SPREAD_OWNERDRAWSTYLE:
            lpSpread->lOwnerDrawStyle = lParam;
            if (CellTypeTemp.Type == SS_TYPE_OWNERDRAW)
               lpCellType = SSSetTypeOwnerDraw(hWnd, &CT,
                                               lpSpread->lOwnerDrawStyle);

            break;
#endif

         }

      if (lpCellType)
         Ret = SSSetCellType(hWnd, x, y, lpCellType);
      else
         Ret = FALSE;
      }

return (Ret);
}


LPSS_CELLTYPE SetCellTypePropDate(HWND hWnd, LPVBSPREAD lpSpread,
                                  short Property, long lParam,
                                  LPSS_CELLTYPE lpCellTypeTemp,
                                  LPSS_CELLTYPE lpCT)
{
#ifndef SS_NOCT_DATE

LPDATEFORMAT lpDateFormat = NULL;
LPTB_DATE    lpDateMin = NULL;
LPTB_DATE    lpDateMax = NULL;
TB_DATE      Date;
long         lStyle = 0L;

if (Property == IPROP_SPREAD_TYPEHALIGN)
   lStyle |= SS_HALIGNTRANSLATE(lParam);
else
   lStyle |= lpCellTypeTemp->Style & (ES_LEFT | ES_RIGHT | ES_CENTER);

#ifdef IPROP_SPREAD_TYPEVALIGN
if (Property == IPROP_SPREAD_TYPEVALIGN)
   lStyle |= SS_VALIGNTRANSLATE(lParam);
else
   lStyle |= lpCellTypeTemp->Style & (SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM |
                                      SSS_ALIGN_VCENTER);
#endif

if (lpCellTypeTemp->Spec.Date.SetFormat)
   lpDateFormat = &lpCellTypeTemp->Spec.Date.Format;

if (lpCellTypeTemp->Spec.Date.SetRange ||
    Property == IPROP_SPREAD_DATEMIN ||
    Property == IPROP_SPREAD_DATEMAX)
   {
   lpDateMin = &lpCellTypeTemp->Spec.Date.Min;
   lpDateMax = &lpCellTypeTemp->Spec.Date.Max;
   }

if (Property == IPROP_SPREAD_DATEMIN)
   {
   if (DateValidate((LPSTR)lParam, &Date))
      _fmemcpy(&lpSpread->TypeDateMin, &Date, sizeof(TB_DATE));

   lpDateMin = &lpSpread->TypeDateMin;
   }

else if (Property == IPROP_SPREAD_DATEMAX)
   {
   if (DateValidate((LPSTR)lParam, &Date))
      _fmemcpy(&lpSpread->TypeDateMax, &Date, sizeof(TB_DATE));

   lpDateMax = &lpSpread->TypeDateMax;
   }

else
   {
   if (Property == IPROP_SPREAD_DATEFORMAT)
      lpSpread->TypeDateFormat.nFormat = IDF_DDMONYY + (short)lParam;

   if (!lpDateFormat)
      lpDateFormat = &lpSpread->TypeDateFormat;
   else
      {
      if (Property == IPROP_SPREAD_DATECENTURY)
         lpDateFormat->bCentury = lpSpread->TypeDateFormat.bCentury;
      else if (Property == IPROP_SPREAD_DATEFORMAT)
         lpDateFormat->nFormat = lpSpread->TypeDateFormat.nFormat;
#ifdef IPROP_SPREAD_DATESEPARATOR
      else if (Property == IPROP_SPREAD_DATESEPARATOR)
         lpDateFormat->cSeparator = lpSpread->TypeDateFormat.cSeparator;
#endif
#ifdef IPROP_SPREAD_TYPESPIN
      else if (Property == IPROP_SPREAD_TYPESPIN)
         lpDateFormat->bSpin = lpSpread->bSpin;
#endif
      }
   }

return (SSSetTypeDate(hWnd, lpCT, lStyle, lpDateFormat, lpDateMin, lpDateMax));
#else
return (NULL);
#endif
}


LPSS_CELLTYPE SetCellTypePropEdit(HWND hWnd, LPVBSPREAD lpSpread,
                                  short Property, long lParam,
                                  LPSS_CELLTYPE lpCellTypeTemp,
                                  LPSS_CELLTYPE lpCT)
{
long  lStyle = ES_AUTOHSCROLL;
short CharCase;
short CharSet;
short EditLen;

#ifdef IPROP_SPREAD_EDITJUSTIFY
if (Property == IPROP_SPREAD_TYPEHALIGN ||
    Property == IPROP_SPREAD_EDITJUSTIFY)
#else
if (Property == IPROP_SPREAD_TYPEHALIGN)
#endif
   {
   lParam = (long)(WORD)lParam;

   lStyle |= SS_HALIGNTRANSLATE(lParam);
   }

else
   lStyle |= lpCellTypeTemp->Style & (ES_LEFT | ES_RIGHT | ES_CENTER);

#ifdef SS_V30
if (Property == IPROP_SPREAD_TYPEVALIGN)
   lStyle |= SS_VALIGNTRANSLATE(lParam);
else
   lStyle |= lpCellTypeTemp->Style & (SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM |
                                      SSS_ALIGN_VCENTER);
#endif

#ifdef IPROP_SPREAD_EDITPASSWORD
if (Property == IPROP_SPREAD_EDITPASSWORD)
    {
    if (lpSpread->TypeEditPassword)
       lStyle |= ES_PASSWORD;
    }

else if (lpCellTypeTemp->Style & ES_PASSWORD)
   lStyle |= ES_PASSWORD;
#endif

#ifdef IPROP_SPREAD_EDITMULTILINE
if (Property == IPROP_SPREAD_EDITMULTILINE)
   {
   if (lpSpread->TypeEditMultiLine)
      lStyle |= ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN;
   else
      lStyle |= ES_AUTOHSCROLL;
   }

else if (lpCellTypeTemp->Style & ES_MULTILINE)
   lStyle |= ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN;
#endif

if (lStyle & ES_MULTILINE)
   lStyle &= ~ES_AUTOHSCROLL;

CharCase = lpCellTypeTemp->Spec.Edit.ChrCase;

#ifdef IPROP_SPREAD_EDITCASE
if (Property == IPROP_SPREAD_EDITCASE)
   CharCase = lpSpread->TypeEditCharCase;
#endif

#ifdef IPROP_SPREAD_EDITCHARCASE
if (Property == IPROP_SPREAD_EDITCHARCASE)
   CharCase = lpSpread->TypeEditCharCase;
#endif

#ifdef IPROP_SPREAD_EDITCHARSET
if (Property == IPROP_SPREAD_EDITCHARSET)
   CharSet = lpSpread->TypeEditCharSet;
else
#endif
   CharSet = lpCellTypeTemp->Spec.Edit.ChrSet;

if (Property == IPROP_SPREAD_EDITLEN)
   {
   lpSpread->nMaxEditLen = (short)lParam;
   EditLen = lpSpread->nMaxEditLen;
   }
else
   EditLen = lpCellTypeTemp->Spec.Edit.Len;

#ifdef SS_GP
{
double dfWidth;
short  nRowCnt;

if (Property == IPROP_SPREAD_EDITEXPWIDTH)
   dfWidth = (double)*(LPFLOAT)&lParam;
else
   dfWidth = lpCellTypeTemp->Spec.Edit.dfWidth;

if (Property == IPROP_SPREAD_EDITEXPROWCNT)
   nRowCnt = (short)lParam;
else
   nRowCnt = lpCellTypeTemp->Spec.Edit.nRowCnt;

return (SSSetTypeEditExpanding(hWnd, lpCT, lStyle, EditLen, (char)CharSet,
                               (char)CharCase, dfWidth, nRowCnt));
}
#else
return (SSSetTypeEdit(hWnd, lpCT, lStyle, EditLen, (char)CharSet,
                      (char)CharCase));
#endif
}


LPSS_CELLTYPE SetCellTypePropFloat(HWND hWnd, LPVBSPREAD lpSpread,
                                   short Property, long lParam,
                                   LPSS_CELLTYPE lpCellTypeTemp,
                                   LPSS_CELLTYPE lpCT)
{
SS_FLOATFORMAT FloatFormat;

#ifdef  BUGS
// Bug-002
char        Buffer[330];
#else
char        Buffer[30];
#endif

long        lStyle = 0;

if (Property == IPROP_SPREAD_TYPEHALIGN)
   {
   if (lParam == 0)
      lStyle |= ES_LEFTALIGN;
   else if (lParam == 1)
      lStyle |= ES_RIGHT;
   else if (lParam == 2)
      lStyle |= ES_CENTER;
   }

else
   lStyle |= lpCellTypeTemp->Style & (ES_LEFTALIGN | ES_RIGHT | ES_CENTER);

#ifdef SS_V30
if (Property == IPROP_SPREAD_TYPEVALIGN)
   lStyle |= SS_VALIGNTRANSLATE(lParam);
else
   lStyle |= lpCellTypeTemp->Style & (SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM |
                                      SSS_ALIGN_VCENTER);
#endif

lStyle |= lpCellTypeTemp->Style & (FS_SEPARATOR | FS_MONEY);

if (lpCellTypeTemp->Spec.Float.fSetFormat)
   _fmemcpy(&FloatFormat, &lpCellTypeTemp->Spec.Float.Format,
            sizeof(SS_FLOATFORMAT));
else
   _fmemset(&FloatFormat, 0, sizeof(SS_FLOATFORMAT));

switch (Property)
   {
#ifdef IPROP_SPREAD_TYPEFLOATCURRENCY
   case IPROP_SPREAD_TYPEFLOATCURRENCY:
#endif
      FloatFormat.cCurrencySign = (char)lParam;
      break;

#ifdef IPROP_SPREAD_TYPEFLOATDECIMAL
   case IPROP_SPREAD_TYPEFLOATDECIMAL:
#endif
      FloatFormat.cDecimalSign = (char)lParam;
      break;

#ifdef IPROP_SPREAD_TYPEFLOATSEPCHAR
   case IPROP_SPREAD_TYPEFLOATSEPCHAR:
#endif
      FloatFormat.cSeparator = (char)lParam;
      break;

#ifdef IPROP_SPREAD_FLOATMAX
   case IPROP_SPREAD_FLOATMAX:
#endif
      StringToFloat((LPSTR)lParam, &lpSpread->TypeFloatMax);
      lpCellTypeTemp->Spec.Float.Max = lpSpread->TypeFloatMax;
      break;

#ifdef IPROP_SPREAD_FLOATMIN
   case IPROP_SPREAD_FLOATMIN:
#endif
      StringToFloat((LPSTR)lParam, &lpSpread->TypeFloatMin);
      lpCellTypeTemp->Spec.Float.Min = lpSpread->TypeFloatMin;
      break;

#ifdef IPROP_SPREAD_FLOATDECPLACES
   case IPROP_SPREAD_FLOATDECPLACES:
#endif
      lpCellTypeTemp->Spec.Float.Right = lpSpread->TypeFloatDecimalPlaces;
      break;

#ifdef IPROP_SPREAD_FLOATSEPARATOR
   case IPROP_SPREAD_FLOATSEPARATOR:
#endif
      if (lpSpread->TypeFloatSeparator)
         lStyle |= FS_SEPARATOR;
      else
         lStyle &= ~FS_SEPARATOR;
      break;

#ifdef IPROP_SPREAD_FLOATMONEY
   case IPROP_SPREAD_FLOATMONEY:
#endif
      if (lpSpread->TypeFloatMoney)
         lStyle |= FS_MONEY;
      else
         lStyle &= ~FS_MONEY;
      break;

   default:
      break;
   }

StrPrintf(Buffer, "%.*f", 2, max(fabs(lpCellTypeTemp->Spec.Float.Min),
          fabs(lpCellTypeTemp->Spec.Float.Max)));

return (SSSetTypeFloatExt(hWnd, lpCT, lStyle,
                          (short)((long)StrChr(Buffer, '.') -
                          (long)(LPSTR)Buffer),
                          lpCellTypeTemp->Spec.Float.Right,
                          lpCellTypeTemp->Spec.Float.Min,
                          lpCellTypeTemp->Spec.Float.Max,
                          &FloatFormat));
}


LPSS_CELLTYPE SetCellTypePropInt(HWND hWnd, LPVBSPREAD lpSpread,
                                 short Property, long lParam,
                                 LPSS_CELLTYPE lpCellTypeTemp,
                                 LPSS_CELLTYPE lpCT)
{
#ifndef SS_NOCT_INT

long lStyle = 0;

if (Property == IPROP_SPREAD_TYPEHALIGN)
   {
   if (lParam == 0)
      lStyle = ES_LEFTALIGN;
   else if (lParam == 1)
      lStyle = ES_RIGHT;
   else if (lParam == 2)
      lStyle = ES_CENTER;
   }

else
   lStyle = lpCellTypeTemp->Style & (ES_LEFTALIGN | ES_RIGHT | ES_CENTER);

#ifdef SS_V30
if (Property == IPROP_SPREAD_TYPEVALIGN)
   lStyle |= SS_VALIGNTRANSLATE(lParam);
else
   lStyle |= lpCellTypeTemp->Style & (SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM |
                                      SSS_ALIGN_VCENTER);
#endif

if (Property == IPROP_SPREAD_INTEGERMAX)
   lpCellTypeTemp->Spec.Integer.Max = lpSpread->TypeIntegerMax;

else if (Property == IPROP_SPREAD_INTEGERMIN)
   lpCellTypeTemp->Spec.Integer.Min = lpSpread->TypeIntegerMin;

else if (Property == IPROP_SPREAD_TYPESPIN)
   {
   if (lpSpread->bSpin)
      lpCellTypeTemp->Style |= IS_SPIN;
   else
      lpCellTypeTemp->Style &= ~IS_SPIN;
   }

else if (Property == IPROP_SPREAD_TYPEINTSPINWRAP)
   {
   lpSpread->fIntegerSpinWrap = (BOOL)lParam;
   lpCellTypeTemp->Spec.Integer.fSpinWrap = lpSpread->fIntegerSpinWrap;
   }

else if (Property == IPROP_SPREAD_TYPEINTSPININC)
   {
   lpSpread->lIntegerSpinInc = (long)lParam;
   lpCellTypeTemp->Spec.Integer.lSpinInc = lpSpread->lIntegerSpinInc;
   }

if (lpCellTypeTemp->Style & IS_SPIN)
   lStyle |= IS_SPIN;

return (SSSetTypeIntegerExt(hWnd, lpCT, lStyle,
                            lpCellTypeTemp->Spec.Integer.Min,
                            lpCellTypeTemp->Spec.Integer.Max,
                            lpCellTypeTemp->Spec.Integer.fSpinWrap,
                            lpCellTypeTemp->Spec.Integer.lSpinInc));
#else
return (NULL);
#endif
}


LPSS_CELLTYPE SetCellTypePropPic(HWND hWnd, LPVBSPREAD lpSpread,
                                 short Property, long lParam,
                                 LPSS_CELLTYPE lpCellTypeTemp,
                                 LPSS_CELLTYPE lpCT)
{
#ifndef SS_NOCT_PIC
LPSS_CELLTYPE lpCellType;
LPSTR         Data;
LPSTR         lpDataTemp;
LPSTR         lpData = NULL;
LPSTR         lpDefText = NULL;
HANDLE        hDataTemp = 0;
long          lStyle = 0;

if (Property == IPROP_SPREAD_TYPEHALIGN)
   lStyle |= SS_HALIGNTRANSLATE(lParam);
else
   lStyle |= lpCellTypeTemp->Style & (ES_LEFT | ES_RIGHT | ES_CENTER);

#ifdef SS_V30
if (Property == IPROP_SPREAD_TYPEVALIGN)
   lStyle |= SS_VALIGNTRANSLATE(lParam);
else
   lStyle |= lpCellTypeTemp->Style & (SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM |
                                      SSS_ALIGN_VCENTER);
#endif

if (Property == IPROP_SPREAD_PICMASK)
   {
   lpDefText = NULL;
   lpDataTemp = NULL;

   if (lpCellTypeTemp->Spec.Pic.hMask)
      {
      lpData = (LPSTR)tbGlobalLock(lpCellTypeTemp->Spec.Pic.hMask);
      if (lpDefText = StrpBrk(lpData, "\t\n"))
         lpDefText++;
      }

   if (lpSpread->hTypePicMask)
      {
      Data = (LPSTR)GlobalLock(lpSpread->hTypePicMask);

      if (lpDefText)
         {
         if (hDataTemp = GlobalAlloc(GMEM_MOVEABLE |
                                     GMEM_ZEROINIT,
                                     lstrlen(Data) + 1 +
                                     lstrlen(lpDefText) + 1))
            {
            lpDataTemp = (LPSTR)GlobalLock(hDataTemp);
            lstrcpy(lpDataTemp, Data);
            StrCat(lpDataTemp, "\n");
            StrCat(lpDataTemp, lpDefText);
            Data = lpDataTemp;
            }
         }
      }
   else
      Data = "";

   if (lpCellTypeTemp->Style & ES_PASSWORD)
      lStyle |= ES_PASSWORD;

   lpCellType = SSSetTypePic(hWnd, lpCT, lStyle, Data);

   if (lpCellTypeTemp->Spec.Pic.hMask)
      tbGlobalUnlock(lpCellTypeTemp->Spec.Pic.hMask);

   if (lpSpread->hTypePicMask)
      GlobalUnlock(lpSpread->hTypePicMask);

   if (hDataTemp)
      {
      GlobalUnlock(hDataTemp);
      GlobalFree(hDataTemp);
      }
   }

else if (Property == IPROP_SPREAD_PICDEFTEXT)
   {
   if (lpCellTypeTemp->Spec.Pic.hMask)
      {
      lpData = (LPSTR)tbGlobalLock(lpCellTypeTemp->Spec.Pic.hMask);
      if (lpDefText = StrpBrk(lpData, "\t\n"))
         *lpDefText = '\0';

      Data = lpData;

      if (lpSpread->hTypePicDefText)
         {
         lpDefText = (LPSTR)GlobalLock(lpSpread->hTypePicDefText);

         if (hDataTemp = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                     lstrlen(Data) + 1 +
                                     lstrlen(lpDefText) + 1))
            {
            lpDataTemp = (LPSTR)GlobalLock(hDataTemp);
            lstrcpy(lpDataTemp, Data);
            StrCat(lpDataTemp, "\n");
            StrCat(lpDataTemp, lpDefText);
            Data = lpDataTemp;
            }

         GlobalUnlock(lpSpread->hTypePicDefText);
         }
      }
   else
      Data = "";

   if (lpCellTypeTemp->Style & ES_PASSWORD)
      lStyle |= ES_PASSWORD;

   lpCellType = SSSetTypePic(hWnd, lpCT, lStyle, Data);

   if (lpCellTypeTemp->Spec.Pic.hMask)
      tbGlobalUnlock(lpCellTypeTemp->Spec.Pic.hMask);

   if (hDataTemp)
      {
      GlobalUnlock(hDataTemp);
      GlobalFree(hDataTemp);
      }
   }

else
   {
   if (lpCellTypeTemp->Spec.Pic.hMask)
      Data = (LPSTR)tbGlobalLock(lpCellTypeTemp->Spec.Pic.hMask);
   else
      Data = "";

   if (lpCellTypeTemp->Style & ES_PASSWORD)
      lStyle |= ES_PASSWORD;

   lpCellType = SSSetTypePic(hWnd, lpCT, lStyle, Data);

   if (lpCellTypeTemp->Spec.Pic.hMask)
      tbGlobalUnlock(lpCellTypeTemp->Spec.Pic.hMask);
   }

return (lpCellType);
#else
return (NULL);
#endif
}


LPSS_CELLTYPE SetCellTypePropText(HWND hWnd, LPVBSPREAD lpSpread,
                                  short Property, long lParam,
                                  LPSS_CELLTYPE lpCellTypeTemp,
                                  LPSS_CELLTYPE lpCT)
{
switch (Property)
   {
#ifdef IPROP_SPREAD_TEXTJUSTIFYHORIZ
   case IPROP_SPREAD_TEXTJUSTIFYHORIZ:
      lParam = (long)(WORD)lParam;

      lpCellTypeTemp->Style &= ~(SS_TEXT_CENTER | SS_TEXT_LEFT | SS_TEXT_RIGHT);

      if (lParam == 0)
         lpCellTypeTemp->Style |= SS_TEXT_CENTER;
      else if (lParam == 1)
         lpCellTypeTemp->Style |= SS_TEXT_LEFT;
      else if (lParam == 2)
         lpCellTypeTemp->Style |= SS_TEXT_RIGHT;
      break;
#endif

#ifdef IPROP_SPREAD_TYPEHALIGN
   case IPROP_SPREAD_TYPEHALIGN:
      lpCellTypeTemp->Style &= ~(SS_TEXT_CENTER | SS_TEXT_LEFT | SS_TEXT_RIGHT);

      if (lParam == 0)
         lpCellTypeTemp->Style |= SS_TEXT_LEFT;
      else if (lParam == 1)
         lpCellTypeTemp->Style |= SS_TEXT_RIGHT;
      else if (lParam == 2)
         lpCellTypeTemp->Style |= SS_TEXT_CENTER;
      break;
#endif

#ifdef IPROP_SPREAD_TEXTJUSTIFYVERT
   case IPROP_SPREAD_TEXTJUSTIFYVERT:
      lpCellTypeTemp->Style &= ~(SS_TEXT_VCENTER | SS_TEXT_TOP | SS_TEXT_BOTTOM);

      if (lpSpread->TypeTextJustifyVert == 0)
         lpCellTypeTemp->Style |= SS_TEXT_BOTTOM;
      else if (lpSpread->TypeTextJustifyVert == 1)
         lpCellTypeTemp->Style |= SS_TEXT_VCENTER;
      else if (lpSpread->TypeTextJustifyVert == 2)
         lpCellTypeTemp->Style |= SS_TEXT_TOP;
      break;
#endif

#ifdef IPROP_SPREAD_TYPEVALIGN
   case IPROP_SPREAD_TYPEVALIGN:
      lpCellTypeTemp->Style &= ~(SS_TEXT_VCENTER | SS_TEXT_TOP | SS_TEXT_BOTTOM);

      if (lParam == 0)
         lpCellTypeTemp->Style |= SS_TEXT_TOP;
      else if (lParam == 1)
         lpCellTypeTemp->Style |= SS_TEXT_BOTTOM;
      else if (lParam == 2)
         lpCellTypeTemp->Style |= SS_TEXT_VCENTER;
      break;
#endif

#ifdef IPROP_SPREAD_TEXTWORDWRAP
   case IPROP_SPREAD_TEXTWORDWRAP:
      lpCellTypeTemp->Style &= ~SS_TEXT_WORDWRAP;

      if (lpSpread->TypeTextWordWrap)
         lpCellTypeTemp->Style |= SS_TEXT_WORDWRAP;
      break;
#endif

#ifdef IPROP_SPREAD_TEXTSHADOW
   case IPROP_SPREAD_TEXTSHADOW:
      lpCellTypeTemp->Style &= ~SS_TEXT_SHADOW;

      if (lpSpread->TypeTextShadow)
         lpCellTypeTemp->Style |= SS_TEXT_SHADOW;
      break;
#endif

#ifdef IPROP_SPREAD_TYPETEXTPREFIX
   case IPROP_SPREAD_TYPETEXTPREFIX:
      lpSpread->fTextPrefix = (BOOL)lParam;
      lpCellTypeTemp->Style &= ~SS_TEXT_PREFIX;

      if (lpSpread->fTextPrefix)
         lpCellTypeTemp->Style |= SS_TEXT_PREFIX;
      break;
#endif

#ifdef IPROP_SPREAD_TYPETEXTSHADOWIN
   case IPROP_SPREAD_TYPETEXTSHADOWIN:
      lpSpread->fTextShadowIn = (BOOL)lParam;

      lpCellTypeTemp->Style &= ~SS_TEXT_SHADOWIN;

      if (lpSpread->fTextShadowIn)
         lpCellTypeTemp->Style |= SS_TEXT_SHADOWIN;
      break;
#endif

   default:
      break;
   }

return (SSSetTypeStaticText(hWnd, lpCT, (short)lpCellTypeTemp->Style));
}


LPSS_CELLTYPE SetCellTypePropTime(HWND hWnd, LPVBSPREAD lpSpread,
                                  short Property, long lParam,
                                  LPSS_CELLTYPE lpCellTypeTemp,
                                  LPSS_CELLTYPE lpCT)
{
#ifndef SS_NOCT_TIME
TIMEFORMAT TimeFormatTemp;
LPTIME     lpTimeMin = NULL;
LPTIME     lpTimeMax = NULL;
long       lStyle = 0;

if (Property == IPROP_SPREAD_TYPEHALIGN)
   lStyle |= SS_HALIGNTRANSLATE(lParam);
else
   lStyle |= lpCellTypeTemp->Style & (ES_LEFT | ES_RIGHT | ES_CENTER);

#ifdef IPROP_SPREAD_TYPEVALIGN
if (Property == IPROP_SPREAD_TYPEVALIGN)
   lStyle |= SS_VALIGNTRANSLATE(lParam);
else
   lStyle |= lpCellTypeTemp->Style & (SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM |
                                      SSS_ALIGN_VCENTER);
#endif

TimeFormatTemp.b24Hour = lpCellTypeTemp->Spec.Time.Format.b24Hour;
TimeFormatTemp.bSeconds = lpCellTypeTemp->Spec.Time.Format.bSeconds;
TimeFormatTemp.cSeparator = lpCellTypeTemp->Spec.Time.Format.cSeparator;
TimeFormatTemp.bSpin = lpCellTypeTemp->Spec.Time.Format.bSpin;

if (lpCellTypeTemp->Spec.Time.SetRange ||
    Property == IPROP_SPREAD_TIMEMIN ||
    Property == IPROP_SPREAD_TIMEMAX)
   {
   lpTimeMin = &lpCellTypeTemp->Spec.Time.Min;
   lpTimeMax = &lpCellTypeTemp->Spec.Time.Max;
   }

switch (Property)
   {
#ifdef IPROP_SPREAD_TIMEMIN
   case IPROP_SPREAD_TIMEMIN:
#endif
      lpTimeMin = &lpSpread->TypeTimeMin;
      break;

#ifdef IPROP_SPREAD_TIMEMAX
   case IPROP_SPREAD_TIMEMAX:
#endif
      lpTimeMax = &lpSpread->TypeTimeMax;
      break;

#ifdef IPROP_SPREAD_TIME24HOUR
   case IPROP_SPREAD_TIME24HOUR:
#endif
      TimeFormatTemp.b24Hour = lpSpread->TypeTimeFormat.b24Hour;
      break;

#ifdef IPROP_SPREAD_TIMESECONDS
   case IPROP_SPREAD_TIMESECONDS:
#endif
      TimeFormatTemp.bSeconds = lpSpread->TypeTimeFormat.bSeconds;
      break;

#ifdef IPROP_SPREAD_TIMESEPARATOR
   case IPROP_SPREAD_TIMESEPARATOR:
#endif
      TimeFormatTemp.cSeparator = lpSpread->TypeTimeFormat.cSeparator;
      break;

#ifdef IPROP_SPREAD_TYPESPIN
   case IPROP_SPREAD_TYPESPIN:
#endif
      TimeFormatTemp.bSpin = lpSpread->bSpin;
      break;

   default:
      break;
   }

return (SSSetTypeTime(hWnd, lpCT, lStyle, &TimeFormatTemp, lpTimeMin,
                      lpTimeMax));
#else
return (NULL);
#endif
}


LPSS_CELLTYPE SetCellTypePropCombo(HWND hWnd, LPVBSPREAD lpSpread,
                                   short Property, long lParam,
                                   LPSS_CELLTYPE lpCellTypeTemp,
                                   LPSS_CELLTYPE lpCT)
{
LPSS_CELLTYPE lpCellType = NULL;

#ifndef SS_NOCT_COMBO

LPSTR lpData = NULL;
LPSTR lpDataTemp;

if (Property == IPROP_SPREAD_COMBOLIST)
   {
   if (lpSpread->hTypeComboList)
      lpData = GlobalLock(lpSpread->hTypeComboList);
   }
else
   {
   if (Property == IPROP_SPREAD_TYPECOMBOEDITABLE)
      {
      lpSpread->fComboEditable = (BOOL)lParam;

      if (lpSpread->fComboEditable)
         lpCellTypeTemp->Style |= SS_CB_DROPDOWN;
      else
         lpCellTypeTemp->Style &= ~SS_CB_DROPDOWN;
      }

   else if (Property == IPROP_SPREAD_EDITLEN)
      {
      lpSpread->dComboMaxEditLen = (short)lParam;
      lpCellTypeTemp->Spec.ComboBox.dMaxEditLen = (short)lParam;
      }

#ifdef SS_V30
   else if (Property == IPROP_SPREAD_TYPECOMBOMAXDROP)
      {
      lpSpread->dComboMaxDrop = (short)lParam;
      lpCellTypeTemp->Spec.ComboBox.dMaxRows = (short)lParam;
      }

   else if (Property == IPROP_SPREAD_TYPECOMBOWIDTH)
      {
      lpSpread->dComboWidth = (short)lParam;
      if (lpSpread->dComboWidth > 1 || lpSpread->dComboWidth < -1)
         lpSpread->dComboWidth = VBXTwipsToPixels(lpSpread->dComboWidth);

      lpCellTypeTemp->Spec.ComboBox.dComboWidth = lpSpread->dComboWidth;
      }

   else if (Property == IPROP_SPREAD_COMBOHWND)
      lpCellTypeTemp->Spec.ComboBox.hWndDropDown = (HWND)lParam;
#endif

#ifdef IPROP_SPREAD_TYPEHALIGN
   if (Property == IPROP_SPREAD_TYPEHALIGN)
      {
      lpCellTypeTemp->Style &= ~(SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);

      if (lParam == 0)
         lpCellTypeTemp->Style |= SSS_ALIGN_LEFT;
      else if (lParam == 1)
         lpCellTypeTemp->Style |= SSS_ALIGN_RIGHT;
      else if (lParam == 2)
         lpCellTypeTemp->Style |= SSS_ALIGN_CENTER;
      }
#endif

#ifdef IPROP_SPREAD_TYPEVALIGN
   if (Property == IPROP_SPREAD_TYPEVALIGN)
      {
      lpCellTypeTemp->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeTemp->Style |= SS_VALIGNTRANSLATE(lParam);
      }
#endif

   if (lpCellTypeTemp->Spec.ComboBox.hItems)
      {
      short i;

      lpData = (LPSTR)tbGlobalLock(lpCellTypeTemp->Spec.ComboBox.hItems);

      lpDataTemp = lpData;
      for (i = 0; i < lpCellTypeTemp->Spec.ComboBox.dItemCnt - 1;
           i++)
         {
         lpDataTemp += lstrlen(lpDataTemp) + 1;
         *(lpDataTemp - 1) = '\t';
         }

      tbGlobalUnlock(lpCellTypeTemp->Spec.ComboBox.hItems);
      }
   }

#ifndef SS_V30
lpCellType = SSSetTypeComboBox(hWnd, lpCT, lpCellTypeTemp->Style, lpData);
#else
lpCellType = SSSetTypeComboBoxEx(hWnd, lpCT, lpCellTypeTemp->Style,
                                 lpData, lpCellTypeTemp->Spec.ComboBox.dMaxRows,
                                 lpCellTypeTemp->Spec.ComboBox.dMaxEditLen,
                                 lpCellTypeTemp->Spec.ComboBox.dComboWidth,
                                 lpCellTypeTemp->Spec.ComboBox.hWndDropDown);
#endif

if (Property == IPROP_SPREAD_COMBOLIST)
   {
   if (lpSpread->hTypeComboList)
      GlobalUnlock(lpSpread->hTypeComboList);
   }

else
   {
   if (lpCellTypeTemp->Spec.ComboBox.hItems)
      {
      lpDataTemp = lpData;
      while (lpDataTemp = _fstrchr(lpDataTemp, '\t'))
         {
         *lpDataTemp = '\0';
         lpDataTemp++;
         }
      tbGlobalUnlock(lpCellTypeTemp->Spec.ComboBox.hItems);
      }
   }
#endif

return (lpCellType);
}


LPSS_CELLTYPE SetCellTypePropPict(HWND hWnd, LPVBSPREAD lpSpread,
                                  short Property, long lParam,
                                  LPSS_CELLTYPE lpCellTypeTemp,
                                  LPSS_CELLTYPE lpCT)
{
LPSS_CELLTYPE lpCellType = NULL;

switch (Property)
   {
#ifdef IPROP_SPREAD_TYPEPICTPICTURE
   case IPROP_SPREAD_TYPEPICTPICTURE:
      lpCellTypeTemp->Spec.ViewPict.hPic = lpSpread->hPic;
      lpCellTypeTemp->Spec.ViewPict.hPictName =
         lpSpread->hTypePictPicture;
      lpCellTypeTemp->Spec.ViewPict.hPal = lpSpread->hTypePictPal;
      lpCellTypeTemp->Style &= ~(VPS_BMP | VPS_ICON | VPS_HANDLE);
      lpCellTypeTemp->Style |= lpSpread->TypePictStyle &
                            (VPS_BMP | VPS_ICON | VPS_HANDLE);

      break;
#endif

#ifdef IPROP_SPREAD_TYPEPICTCENTER
   case IPROP_SPREAD_TYPEPICTCENTER:
      if ((BOOL)lParam)
         {
         lpSpread->TypePictStyle |= VPS_CENTER;
         lpCellTypeTemp->Style |= VPS_CENTER;
         }
      else
         {
         lpSpread->TypePictStyle &= ~VPS_CENTER;
         lpCellTypeTemp->Style &= ~VPS_CENTER;
         }

      break;
#endif

#ifdef IPROP_SPREAD_TYPEPICTSTRETCH
   case IPROP_SPREAD_TYPEPICTSTRETCH:
      if ((BOOL)lParam)
         {
         lpSpread->TypePictStyle |= VPS_STRETCH;
         lpCellTypeTemp->Style |= VPS_STRETCH;
         }
      else
         {
         lpSpread->TypePictStyle &= ~VPS_STRETCH;
         lpCellTypeTemp->Style &= ~VPS_STRETCH;
         }

      break;
#endif

#ifdef IPROP_SPREAD_TYPEPICTSCALE
   case IPROP_SPREAD_TYPEPICTSCALE:
      if ((BOOL)lParam)
         {
         lpSpread->TypePictStyle |= VPS_MAINTAINSCALE;
         lpCellTypeTemp->Style |= VPS_MAINTAINSCALE;
         }
      else
         {
         lpSpread->TypePictStyle &= ~VPS_MAINTAINSCALE;
         lpCellTypeTemp->Style &= ~VPS_MAINTAINSCALE;
         }

      break;
#endif

#ifdef IPROP_SPREAD_TYPEHALIGN
   case IPROP_SPREAD_TYPEHALIGN:
      lpCellTypeTemp->Style &= ~(SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);

      if (lParam == 0)
         lpCellTypeTemp->Style |= SSS_ALIGN_LEFT;
      else if (lParam == 1)
         lpCellTypeTemp->Style |= SSS_ALIGN_RIGHT;
      else if (lParam == 2)
         lpCellTypeTemp->Style |= SSS_ALIGN_CENTER;

      break;
#endif

#ifdef IPROP_SPREAD_TYPEVALIGN
   case IPROP_SPREAD_TYPEVALIGN:
      lpCellTypeTemp->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeTemp->Style |= SS_VALIGNTRANSLATE(lParam);
      break;
#endif
   }

if (lpCellTypeTemp->Spec.ViewPict.hPal)
   lpCellType = SSSetTypePictureHandle(hWnd, lpCT,
                lpCellTypeTemp->Style,
                (short)lpCellTypeTemp->Spec.ViewPict.hPictName,
                lpCellTypeTemp->Spec.ViewPict.hPal, FALSE);
else
   lpCellType = SSSetTypePicture(hWnd, lpCT, lpCellTypeTemp->Style,
                (LPSTR)&lpCellTypeTemp->Spec.ViewPict.hPictName);

lpCT->Spec.ViewPict.hPic = lpCellTypeTemp->Spec.ViewPict.hPic;
if (lpCT->Spec.ViewPict.hPic)
   VBRefPic(lpCT->Spec.ViewPict.hPic);

return (lpCellType);
}


LPSS_CELLTYPE SetCellTypePropButton(HWND hWnd, LPVBSPREAD lpSpread,
                                    short Property, long lParam,
                                    LPSS_CELLTYPE lpCellTypeTemp,
                                    LPSS_CELLTYPE lpCT)
{
LPSS_CELLTYPE lpCellType = NULL;
LPSTR         lpText = NULL;
LPSTR         lpPict = NULL;
LPSTR         lpPictDown = 0;

switch (Property)
   {
   case IPROP_SPREAD_BUTTONALIGN:
      lpCellTypeTemp->Style &= ~(SBS_TEXTBOT_PICTTOP |
         SBS_TEXTTOP_PICTBOT | SBS_TEXTLEFT_PICTRIGHT |
         SBS_TEXTRIGHT_PICTLEFT);

      if (lpSpread->TypeButtonAlign == 0)
         lpCellTypeTemp->Style |= SBS_TEXTBOT_PICTTOP;
      else if (lpSpread->TypeButtonAlign == 1)
         lpCellTypeTemp->Style |= SBS_TEXTTOP_PICTBOT;
      else if (lpSpread->TypeButtonAlign == 2)
         lpCellTypeTemp->Style |= SBS_TEXTLEFT_PICTRIGHT;
      else if (lpSpread->TypeButtonAlign == 3)
         lpCellTypeTemp->Style |= SBS_TEXTRIGHT_PICTLEFT;

      break;

   case IPROP_SPREAD_BUTTONTEXT:
      lpCellTypeTemp->Spec.Button.hText =
         lpSpread->hTypeButtonText;
      break;

   case IPROP_SPREAD_BUTTONPICTNAME:
      lpCellTypeTemp->Spec.Button.hPic = lpSpread->hPic;
      lpCellTypeTemp->Spec.Button.hPictName =
         lpSpread->hTypeButtonPictureName;
      lpCellTypeTemp->Spec.Button.nPictureType =
         lpSpread->TypeButtonPictureType;
      break;

   case IPROP_SPREAD_BUTTONPICTDOWN:
      lpCellTypeTemp->Spec.Button.hPicDown = lpSpread->hPic;
      lpCellTypeTemp->Spec.Button.hPictDownName =
         lpSpread->hTypeButtonPictureDownName;
      lpCellTypeTemp->Spec.Button.nPictureDownType =
         lpSpread->TypeButtonPictureDownType;
      break;

   case IPROP_SPREAD_BUTTONTYPE:
      lpCellTypeTemp->Spec.Button.ButtonType =
         (char)lpSpread->TypeButtonType;
      break;

#ifdef IPROP_SPREAD_BUTTONCOLOR
   case IPROP_SPREAD_BUTTONCOLOR:
      lpCellTypeTemp->Spec.Button.Color.Color =
         lpSpread->TypeButtonColor.Color;

      break;
#endif

#ifdef IPROP_SPREAD_BUTTONBORDERCOLOR
   case IPROP_SPREAD_BUTTONBORDERCOLOR:
      lpCellTypeTemp->Spec.Button.Color.ColorBorder =
         lpSpread->TypeButtonColor.ColorBorder;

      break;
#endif

#ifdef IPROP_SPREAD_BUTTONDARKCOLOR
   case IPROP_SPREAD_BUTTONDARKCOLOR:
      lpCellTypeTemp->Spec.Button.Color.ColorShadow =
         lpSpread->TypeButtonColor.ColorShadow;

      break;
#endif

#ifdef IPROP_SPREAD_BUTTONLIGHTCOLOR
   case IPROP_SPREAD_BUTTONLIGHTCOLOR:
      lpCellTypeTemp->Spec.Button.Color.ColorHighlight =
         lpSpread->TypeButtonColor.ColorHighlight;

      break;
#endif

#ifdef IPROP_SPREAD_BUTTONTEXTCOLOR
   case IPROP_SPREAD_BUTTONTEXTCOLOR:
      lpCellTypeTemp->Spec.Button.Color.ColorText =
         lpSpread->TypeButtonColor.ColorText;

      break;
#endif

#ifdef IPROP_SPREAD_BUTTONSHADOWSIZE
   case IPROP_SPREAD_BUTTONSHADOWSIZE:
      lpCellTypeTemp->Spec.Button.ShadowSize =
         (char)lpSpread->TypeButtonShadowSize;

      break;
#endif

#ifdef IPROP_SPREAD_TYPEHALIGN
   case IPROP_SPREAD_TYPEHALIGN:
      lpCellTypeTemp->Style &= ~(SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);

      if (lParam == 0)
         lpCellTypeTemp->Style |= SSS_ALIGN_LEFT;
      else if (lParam == 1)
         lpCellTypeTemp->Style |= SSS_ALIGN_RIGHT;
      else if (lParam == 2)
         lpCellTypeTemp->Style |= SSS_ALIGN_CENTER;

      break;
#endif

#ifdef IPROP_SPREAD_TYPEVALIGN
   case IPROP_SPREAD_TYPEVALIGN:
      lpCellTypeTemp->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeTemp->Style |= SS_VALIGNTRANSLATE(lParam);
      break;
#endif
   }

if (lpCellTypeTemp->Spec.Button.hText)
   lpText = (LPSTR)GlobalLock(lpCellTypeTemp->Spec.Button.hText);

if (lpCellTypeTemp->Spec.Button.hPictName)
   lpPict = (LPSTR)&lpCellTypeTemp->Spec.Button.hPictName;
else
   lpPict = 0;

if (lpCellTypeTemp->Spec.Button.hPictDownName)
   lpPictDown = (LPSTR)&lpCellTypeTemp->Spec.Button.hPictDownName;
else
   lpPictDown = 0;

lpCellType = SSSetTypeButton(hWnd, lpCT, lpCellTypeTemp->Style,
             lpText, lpPict,
             lpCellTypeTemp->Spec.Button.nPictureType,
             lpPictDown,
             lpCellTypeTemp->Spec.Button.nPictureDownType,
             lpCellTypeTemp->Spec.Button.ButtonType,
             lpCellTypeTemp->Spec.Button.ShadowSize,
             &lpCellTypeTemp->Spec.Button.Color);

lpCT->Spec.Button.hPic = lpCellTypeTemp->Spec.Button.hPic;
if (lpCT->Spec.Button.hPic)
   VBRefPic(lpCT->Spec.Button.hPic);

lpCT->Spec.Button.hPicDown = lpCellTypeTemp->Spec.Button.hPicDown;
if (lpCT->Spec.Button.hPicDown)
   VBRefPic(lpCT->Spec.Button.hPicDown);

if (lpCellTypeTemp->Spec.Button.hText)
   GlobalUnlock(lpCellTypeTemp->Spec.Button.hText);

return (lpCellType);
}


LPSS_CELLTYPE SetCellTypePropCheck(HWND hWnd, LPVBSPREAD lpSpread,
                                   short Property, long lParam,
                                   LPSS_CELLTYPE lpCellTypeTemp,
                                   LPSS_CELLTYPE lpCT)
{
LPSS_CELLTYPE lpCellType = NULL;
LPSTR         lpText = NULL;
LPSTR         lpPictUp = 0;
LPSTR         lpPictDown = 0;
LPSTR         lpPictFocusUp = 0;
LPSTR         lpPictFocusDown = 0;
LPSTR         lpPictGray = 0;
LPSTR         lpPictFocusGray = 0;

switch (Property)
   {
#ifdef IPROP_SPREAD_CHECKCENTER
   case IPROP_SPREAD_CHECKCENTER:
      lpSpread->fCheckCenter = (BOOL)lParam;
      lpCellTypeTemp->Style &= ~(SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);
      if (lpSpread->fCheckCenter)
         lpCellTypeTemp->Style |= BS_CENTER;
      else
         lpCellTypeTemp->Style &= ~BS_CENTER;

      break;
#endif

#ifdef IPROP_SPREAD_CHECKTYPE
   case IPROP_SPREAD_CHECKTYPE:
      lpSpread->nCheckType = (short)lParam;
      lpCellTypeTemp->Style &= 0xFFFFFFF0;
      if (0 == lpSpread->nCheckType)
         lpCellTypeTemp->Style |= BS_AUTOCHECKBOX;
      else if (1 == lpSpread->nCheckType)
         lpCellTypeTemp->Style |= BS_AUTO3STATE;
      break;
#endif

#ifdef IPROP_SPREAD_CHECKTEXTALIGN
   case IPROP_SPREAD_CHECKTEXTALIGN:
      lpSpread->bCheckTextAlign = (BYTE)lParam;
      lpCellTypeTemp->Style &= ~BS_LEFTTEXT;

      if (lpSpread->bCheckTextAlign == 0)
         lpCellTypeTemp->Style |= BS_LEFTTEXT;

      break;
#endif

#ifdef IPROP_SPREAD_CHECKTEXT
   case IPROP_SPREAD_CHECKTEXT:
      lpCellTypeTemp->Spec.CheckBox.hText = lpSpread->hCheckText;
      break;
#endif

#ifdef IPROP_SPREAD_CHECKPICTUP
   case IPROP_SPREAD_CHECKPICTUP:
      lpCellTypeTemp->Spec.CheckBox.hPicUp = lpSpread->hPic;
      lpCellTypeTemp->Spec.CheckBox.hPictUpName =
         lpSpread->hCheckPictUp;
      lpCellTypeTemp->Spec.CheckBox.bPictUpType =
         (BYTE)lpSpread->dCheckPictUpType;
      break;
#endif

   case IPROP_SPREAD_CHECKPICTDOWN:
      lpCellTypeTemp->Spec.CheckBox.hPicDown = lpSpread->hPic;
      lpCellTypeTemp->Spec.CheckBox.hPictDownName =
         lpSpread->hCheckPictDown;
      lpCellTypeTemp->Spec.CheckBox.bPictDownType =
         (BYTE)lpSpread->dCheckPictDownType;
      break;

   case IPROP_SPREAD_CHECKPICTFOCUSUP:
      lpCellTypeTemp->Spec.CheckBox.hPicFocusUp = lpSpread->hPic;
      lpCellTypeTemp->Spec.CheckBox.hPictFocusUpName =
         lpSpread->hCheckPictFocusUp;
      lpCellTypeTemp->Spec.CheckBox.bPictFocusUpType =
         (BYTE)lpSpread->dCheckPictFocusUpType;
      break;

   case IPROP_SPREAD_CHECKPICTFOCUSDOWN:
      lpCellTypeTemp->Spec.CheckBox.hPicFocusDown = lpSpread->hPic;
      lpCellTypeTemp->Spec.CheckBox.hPictFocusDownName =
         lpSpread->hCheckPictFocusDown;
      lpCellTypeTemp->Spec.CheckBox.bPictFocusDownType =
         (BYTE)lpSpread->dCheckPictFocusDownType;
      break;

   case IPROP_SPREAD_CHECKPICTGRAY:
      lpCellTypeTemp->Spec.CheckBox.hPicGray =
         lpSpread->hPic;
      lpCellTypeTemp->Spec.CheckBox.hPictGrayName =
         lpSpread->hCheckPictGray;
      lpCellTypeTemp->Spec.CheckBox.bPictGrayType =
         (BYTE)lpSpread->dCheckPictGrayType;
      break;

   case IPROP_SPREAD_CHECKPICTFOCUSGRAY:
      lpCellTypeTemp->Spec.CheckBox.hPicFocusGray =
         lpSpread->hPic;
      lpCellTypeTemp->Spec.CheckBox.hPictFocusGrayName =
         lpSpread->hCheckPictFocusGray;
      lpCellTypeTemp->Spec.CheckBox.bPictFocusGrayType =
         (BYTE)lpSpread->dCheckPictFocusGrayType;
      break;

#ifdef IPROP_SPREAD_TYPEHALIGN
   case IPROP_SPREAD_TYPEHALIGN:
      lpCellTypeTemp->Style &= ~(SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);
      lpCellTypeTemp->Style &= ~BS_CENTER;

      if (lParam == 0)
         lpCellTypeTemp->Style |= SSS_ALIGN_LEFT;
      else if (lParam == 1)
         lpCellTypeTemp->Style |= SSS_ALIGN_RIGHT;
      else if (lParam == 2)
         {
         lpCellTypeTemp->Style |= SSS_ALIGN_CENTER;
         lpCellTypeTemp->Style |= BS_CENTER;
         }

      break;
#endif

#ifdef IPROP_SPREAD_TYPEVALIGN
   case IPROP_SPREAD_TYPEVALIGN:
      lpCellTypeTemp->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeTemp->Style |= SS_VALIGNTRANSLATE(lParam);
      break;
#endif
   }

if (lpCellTypeTemp->Spec.CheckBox.hText)
   lpText = (LPSTR)GlobalLock(lpCellTypeTemp->Spec.CheckBox.hText);

lpPictUp = 0;
lpPictDown = 0;
lpPictFocusUp = 0;
lpPictFocusDown = 0;
lpPictGray = 0;
lpPictFocusGray = 0;

if (lpCellTypeTemp->Spec.CheckBox.hPictUpName)
   lpPictUp = (LPSTR)&lpCellTypeTemp->Spec.CheckBox.hPictUpName;

if (lpCellTypeTemp->Spec.CheckBox.hPictDownName)
   lpPictDown =
   (LPSTR)&lpCellTypeTemp->Spec.CheckBox.hPictDownName;

if (lpCellTypeTemp->Spec.CheckBox.hPictFocusUpName)
   lpPictFocusUp =
   (LPSTR)&lpCellTypeTemp->Spec.CheckBox.hPictFocusUpName;

if (lpCellTypeTemp->Spec.CheckBox.hPictFocusDownName)
   lpPictFocusDown =
   (LPSTR)&lpCellTypeTemp->Spec.CheckBox.hPictFocusDownName;

if (lpCellTypeTemp->Spec.CheckBox.hPictGrayName)
   lpPictGray =
   (LPSTR)&lpCellTypeTemp->Spec.CheckBox.hPictGrayName;

if (lpCellTypeTemp->Spec.CheckBox.hPictFocusGrayName)
   lpPictFocusGray =
   (LPSTR)&lpCellTypeTemp->Spec.CheckBox.hPictFocusGrayName;

lpCellType = SSSetTypeCheckBox(hWnd, lpCT, lpCellTypeTemp->Style,
           lpText,
           lpPictUp, lpCellTypeTemp->Spec.CheckBox.bPictUpType,
           lpPictDown, lpCellTypeTemp->Spec.CheckBox.bPictDownType,
           lpPictFocusUp,
           lpCellTypeTemp->Spec.CheckBox.bPictFocusUpType,
           lpPictFocusDown,
           lpCellTypeTemp->Spec.CheckBox.bPictFocusDownType,
           lpPictGray,
           lpCellTypeTemp->Spec.CheckBox.bPictGrayType,
           lpPictFocusGray,
           lpCellTypeTemp->Spec.CheckBox.bPictFocusGrayType);

lpCT->Spec.CheckBox.hPicUp = lpCellTypeTemp->Spec.CheckBox.hPicUp;
if (lpCT->Spec.CheckBox.hPicUp)
   VBRefPic(lpCT->Spec.CheckBox.hPicUp);

lpCT->Spec.CheckBox.hPicDown = lpCellTypeTemp->Spec.CheckBox.hPicDown;
if (lpCT->Spec.CheckBox.hPicDown)
   VBRefPic(lpCT->Spec.CheckBox.hPicDown);

lpCT->Spec.CheckBox.hPicFocusUp =
   lpCellTypeTemp->Spec.CheckBox.hPicFocusUp;
if (lpCT->Spec.CheckBox.hPicFocusUp)
   VBRefPic(lpCT->Spec.CheckBox.hPicFocusUp);

lpCT->Spec.CheckBox.hPicFocusDown =
   lpCellTypeTemp->Spec.CheckBox.hPicFocusDown;
if (lpCT->Spec.CheckBox.hPicFocusDown)
   VBRefPic(lpCT->Spec.CheckBox.hPicFocusDown);

lpCT->Spec.CheckBox.hPicGray =
   lpCellTypeTemp->Spec.CheckBox.hPicGray;
if (lpCT->Spec.CheckBox.hPicGray)
   VBRefPic(lpCT->Spec.CheckBox.hPicGray);

lpCT->Spec.CheckBox.hPicFocusGray =
   lpCellTypeTemp->Spec.CheckBox.hPicFocusGray;
if (lpCT->Spec.CheckBox.hPicFocusGray)
   VBRefPic(lpCT->Spec.CheckBox.hPicFocusGray);

if (lpCellTypeTemp->Spec.CheckBox.hText)
   GlobalUnlock(lpCellTypeTemp->Spec.CheckBox.hText);

return (lpCellType);
}


BOOL GetCellTypeProperties(hCtl, hWnd, lpSpread, Property, lParam)

HCTL        hCtl;
HWND        hWnd;
LPVBSPREAD  lpSpread;
short       Property;
long        lParam;
{
SS_CELLTYPE CellType;
FLOATFORMAT FloatFormat;
#ifdef  BUGS
// Bug-002
char        Buffer[330];
#else
char        Buffer[70 + 1];
#endif
char        cTemp;
LPSTR       lpData;
LPSTR       lpDataTemp;
LPSTR       lpDefText;
BOOL        Ret = TRUE;
short       i;

VBSSVALCOLROW(lpSpread);

if (!SSGetCellType(hWnd, lpSpread->Col, lpSpread->Row, &CellType))
   return (FALSE);

switch (Property)
   {
#ifndef SS_NOCT_DATE
   case IPROP_SPREAD_DATEMIN:
      if (CellType.Type == SS_TYPE_DATE)
         {
         if (DateToString(Buffer, &CellType.Spec.Date.Min))
            {
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), Buffer);
            return (TRUE);
            }
         }

      return (FALSE);

   case IPROP_SPREAD_DATEMAX:
      if (CellType.Type == SS_TYPE_DATE)
         {
         if (DateToString(Buffer, &CellType.Spec.Date.Max))
            {
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), Buffer);
            return (TRUE);
            }
         }

      return (FALSE);

   case IPROP_SPREAD_DATECENTURY:
      if (CellType.Type == SS_TYPE_DATE)
         {
         if (CellType.Spec.Date.SetFormat)
            *(short FAR *)lParam = (short)CellType.Spec.Date.Format.bCentury;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_DATEFORMAT:
      if (CellType.Type == SS_TYPE_DATE)
         {
         if (CellType.Spec.Date.SetFormat)
            *(short FAR *)lParam = CellType.Spec.Date.Format.nFormat -
                                   IDF_DDMONYY;
         else
            *(short FAR *)lParam = 2;

         return (TRUE);
         }

      return (FALSE);

#ifdef IPROP_SPREAD_DATESEPARATOR
   case IPROP_SPREAD_DATESEPARATOR:
      if (CellType.Type == SS_TYPE_DATE)
         {
         if (CellType.Spec.Date.SetFormat)
            *(short FAR *)lParam = (unsigned char)CellType.Spec.Date.Format.cSeparator;
         else
            *(short FAR *)lParam = '/';

         return (TRUE);
         }

      return (FALSE);
#endif
#endif

#ifdef IPROP_SPREAD_TYPESPIN
   case IPROP_SPREAD_TYPESPIN:
      if (CellType.Type == SS_TYPE_DATE)
         {
         if (CellType.Spec.Date.SetFormat)
            *(short FAR *)lParam = CellType.Spec.Date.Format.bSpin;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      else if (CellType.Type == SS_TYPE_INTEGER)
         {
         if (CellType.Style & IS_SPIN)
            *(BOOL FAR *)lParam = -1;
         else
            *(BOOL FAR *)lParam = 0;

         return (TRUE);
         }

      else if (CellType.Type == SS_TYPE_TIME)
         {
         *(short FAR *)lParam = CellType.Spec.Time.Format.bSpin;
         return (TRUE);
         }

      return (FALSE);
#endif

   case IPROP_SPREAD_TYPEHALIGN:
      if (CellType.Type == SS_TYPE_DATE ||
          CellType.Type == SS_TYPE_EDIT ||
          CellType.Type == SS_TYPE_FLOAT ||
          CellType.Type == SS_TYPE_INTEGER ||
          CellType.Type == SS_TYPE_PIC ||
          CellType.Type == SS_TYPE_TIME)
         {
         if (CellType.Style & ES_CENTER)
            *(short FAR *)lParam = 2;
         else if (CellType.Style & ES_RIGHT)
            *(short FAR *)lParam = 1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      else if (CellType.Type == SS_TYPE_STATICTEXT)
         {
         if (CellType.Style & SS_TEXT_CENTER)
            *(short FAR *)lParam = 2;
         else if (CellType.Style & SS_TEXT_RIGHT)
            *(short FAR *)lParam = 1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      else if (CellType.Type == SS_TYPE_PICTURE ||
               CellType.Type == SS_TYPE_COMBOBOX ||
               CellType.Type == SS_TYPE_CHECKBOX)
         {
         if (CellType.Style & SSS_ALIGN_CENTER)
            *(short FAR *)lParam = 2;
         else if (CellType.Style & SSS_ALIGN_RIGHT)
            *(short FAR *)lParam = 1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      else if (CellType.Type == SS_TYPE_BUTTON)
         {
         if (CellType.Style & SSS_ALIGN_LEFT)
            *(short FAR *)lParam = 0;
         else if (CellType.Style & SSS_ALIGN_RIGHT)
            *(short FAR *)lParam = 1;
         else
            *(short FAR *)lParam = 2;

         return (TRUE);
         }

      return (FALSE);

#ifdef IPROP_SPREAD_TYPEVALIGN
   case IPROP_SPREAD_TYPEVALIGN:
      if (CellType.Type == SS_TYPE_DATE ||
          CellType.Type == SS_TYPE_EDIT ||
          CellType.Type == SS_TYPE_FLOAT ||
          CellType.Type == SS_TYPE_INTEGER ||
          CellType.Type == SS_TYPE_PIC ||
          CellType.Type == SS_TYPE_TIME ||
          CellType.Type == SS_TYPE_PICTURE ||
          CellType.Type == SS_TYPE_COMBOBOX)
         {
         if (CellType.Style & SSS_ALIGN_VCENTER)
            *(short FAR *)lParam = 2;
         else if (CellType.Style & SSS_ALIGN_BOTTOM)
            *(short FAR *)lParam = 1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      else if (CellType.Type == SS_TYPE_STATICTEXT)
         {
         if (CellType.Style & SS_TEXT_VCENTER)
            *(short FAR *)lParam = 2;
         else if (CellType.Style & SS_TEXT_BOTTOM)
            *(short FAR *)lParam = 1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      else if (CellType.Type == SS_TYPE_CHECKBOX ||
               CellType.Type == SS_TYPE_BUTTON)
         {
         if (CellType.Style & SSS_ALIGN_TOP)
            *(short FAR *)lParam = 0;
         else if (CellType.Style & SSS_ALIGN_BOTTOM)
            *(short FAR *)lParam = 1;
         else
            *(short FAR *)lParam = 2;

         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_EDITCASE
   case IPROP_SPREAD_EDITCASE:
#endif
#ifdef IPROP_SPREAD_EDITCHARCASE
   case IPROP_SPREAD_EDITCHARCASE:
      if (CellType.Type == SS_TYPE_EDIT)
         {
         *(short FAR *)lParam = (short)CellType.Spec.Edit.ChrCase;
         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_EDITCHARSET
   case IPROP_SPREAD_EDITCHARSET:
      if (CellType.Type == SS_TYPE_EDIT)
         {
         *(short FAR *)lParam = (short)CellType.Spec.Edit.ChrSet;
         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_EDITJUSTIFY
   case IPROP_SPREAD_EDITJUSTIFY:
      if (CellType.Type == SS_TYPE_EDIT)
         {
         if (CellType.Style & ES_CENTER)
            *(short FAR *)lParam = 2;
         else if (CellType.Style & ES_RIGHT)
            *(short FAR *)lParam = 1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);
#endif

   case IPROP_SPREAD_EDITLEN:
      if (CellType.Type == SS_TYPE_EDIT)
         {
         *(short FAR *)lParam = CellType.Spec.Edit.Len;
         return (TRUE);
         }

      else if (CellType.Type == SS_TYPE_COMBOBOX)
         {
         *(short FAR *)lParam = CellType.Spec.ComboBox.dMaxEditLen;
         return (TRUE);
         }

      return (FALSE);

#ifdef IPROP_SPREAD_EDITPASSWORD
   case IPROP_SPREAD_EDITPASSWORD:
      if (CellType.Type == SS_TYPE_EDIT)
         {
         if (CellType.Style & ES_PASSWORD)
            *(short FAR *)lParam = -1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_EDITMULTILINE
   case IPROP_SPREAD_EDITMULTILINE:
      if (CellType.Type == SS_TYPE_EDIT)
         {
         if (CellType.Style & ES_MULTILINE)
            *(short FAR *)lParam = -1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef SS_GP
   case IPROP_SPREAD_EDITEXPWIDTH:
      if (CellType.Type == SS_TYPE_EDIT)
         {
         *(float FAR *)lParam = (float)CellType.Spec.Edit.dfWidth;
         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_EDITEXPROWCNT:
      if (CellType.Type == SS_TYPE_EDIT)
         {
         *(short FAR *)lParam = CellType.Spec.Edit.nRowCnt;
         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_TYPEFLOATCURRENCY
   case IPROP_SPREAD_TYPEFLOATCURRENCY:
   case IPROP_SPREAD_TYPEFLOATDECIMAL:
   case IPROP_SPREAD_TYPEFLOATSEPCHAR:
      if (CellType.Type == SS_TYPE_FLOAT)
         {
         if (CellType.Spec.Float.fSetFormat)
            _fmemcpy(&FloatFormat, &CellType.Spec.Float.Format,
                     sizeof(FLOATFORMAT));
         else
            SSGetDefFloatFormat(hWnd, &FloatFormat);

         if (Property == IPROP_SPREAD_TYPEFLOATCURRENCY)
            *(short FAR *)lParam = (unsigned char)FloatFormat.cCurrencySign;
         else if (Property == IPROP_SPREAD_TYPEFLOATDECIMAL)
            *(short FAR *)lParam = (unsigned char)FloatFormat.cDecimalSign;
         else if (Property == IPROP_SPREAD_TYPEFLOATSEPCHAR)
            *(short FAR *)lParam = (unsigned char)FloatFormat.cSeparator;

         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_FLOATDECPLACES
   case IPROP_SPREAD_FLOATDECPLACES:
      if (CellType.Type == SS_TYPE_FLOAT)
         {
         *(short FAR *)lParam = (short)CellType.Spec.Float.Right;
         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_FLOATMAX
   case IPROP_SPREAD_FLOATMAX:
      if (CellType.Type == SS_TYPE_FLOAT)
         {
         StrPrintf(Buffer, "%.*lf", max(CellType.Spec.Float.Right, 1),
                   CellType.Spec.Float.Max);

         if (CellType.Spec.Float.Right == 0)
            Buffer[lstrlen(Buffer) - 2] = '\0';

         *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), Buffer);

         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_FLOATMIN
   case IPROP_SPREAD_FLOATMIN:
      if (CellType.Type == SS_TYPE_FLOAT)
         {
         StrPrintf(Buffer, "%.*lf", max(CellType.Spec.Float.Right, 1),
                   CellType.Spec.Float.Min);

         if (CellType.Spec.Float.Right == 0)
            Buffer[lstrlen(Buffer) - 2] = '\0';

         *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), Buffer);

         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_FLOATMONEY
   case IPROP_SPREAD_FLOATMONEY:
      if (CellType.Type == SS_TYPE_FLOAT)
         {
         if (CellType.Style & FS_MONEY)
            *(short FAR *)lParam = -1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_FLOATSEPARATOR
   case IPROP_SPREAD_FLOATSEPARATOR:
      if (CellType.Type == SS_TYPE_FLOAT)
         {
         if (CellType.Style & FS_SEPARATOR)
            *(short FAR *)lParam = -1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);
#endif

#ifndef SS_NOCT_INT
   case IPROP_SPREAD_INTEGERMAX:
      if (CellType.Type == SS_TYPE_INTEGER)
         {
         *(long FAR *)lParam = CellType.Spec.Integer.Max;
         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_INTEGERMIN:
      if (CellType.Type == SS_TYPE_INTEGER)
         {
         *(long FAR *)lParam = CellType.Spec.Integer.Min;
         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_TYPEINTSPINWRAP:
      if (CellType.Type == SS_TYPE_INTEGER)
         {
         *(BOOL FAR *)lParam = CellType.Spec.Integer.fSpinWrap;
         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_TYPEINTSPININC:
      if (CellType.Type == SS_TYPE_INTEGER)
         {
         *(long FAR *)lParam = CellType.Spec.Integer.lSpinInc;
         return (TRUE);
         }

      return (FALSE);
#endif

#ifndef SS_NOCT_PIC
   case IPROP_SPREAD_PICMASK:
      if (CellType.Type == SS_TYPE_PIC)
         {
         if (CellType.Spec.Pic.hMask)
            {
            lpData = (LPSTR)tbGlobalLock(CellType.Spec.Pic.hMask);
            if (lpDefText = StrpBrk(lpData, "\t\n"))
               {
               cTemp = *lpDefText;
               *lpDefText = '\0';
               }

            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpData);
            if (lpDefText)
               *lpDefText = cTemp;

            tbGlobalUnlock(CellType.Spec.Pic.hMask);
            }
         else
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");

         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_PICDEFTEXT:
      if (CellType.Type == SS_TYPE_PIC)
         {
         if (CellType.Spec.Pic.hMask)
            {
            lpData = (LPSTR)tbGlobalLock(CellType.Spec.Pic.hMask);
            if (lpDefText = StrpBrk(lpData, "\t\n"))
               {
               lpData = lpDefText + 1;
               *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpData);
               }
            else
               *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");

            tbGlobalUnlock(CellType.Spec.Pic.hMask);
            }
         else
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");

         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_TEXTJUSTIFYHORIZ
   case IPROP_SPREAD_TEXTJUSTIFYHORIZ:
      if (CellType.Type == SS_TYPE_STATICTEXT)
         {
         if (CellType.Style & SS_TEXT_CENTER)
            *(short FAR *)lParam = 0;
         else if (CellType.Style & SS_TEXT_RIGHT)
            *(short FAR *)lParam = 2;
         else
            *(short FAR *)lParam = 1;

         return (TRUE);
         }

      return (FALSE);
#endif

   case IPROP_SPREAD_TEXTJUSTIFYVERT:
      if (CellType.Type == SS_TYPE_STATICTEXT)
         {
         if (CellType.Style & SS_TEXT_BOTTOM)
            *(short FAR *)lParam = 0;
         else if (CellType.Style & SS_TEXT_VCENTER)
            *(short FAR *)lParam = 1;
         else
            *(short FAR *)lParam = 2;

         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_TEXTSHADOW:
      if (CellType.Type == SS_TYPE_STATICTEXT)
         {
         if (CellType.Style & SS_TEXT_SHADOW)
            *(short FAR *)lParam = -1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);

#ifdef IPROP_SPREAD_TEXTWORDWRAP
   case IPROP_SPREAD_TEXTWORDWRAP:
      if (CellType.Type == SS_TYPE_STATICTEXT)
         {
         if (CellType.Style & SS_TEXT_WORDWRAP)
            *(short FAR *)lParam = -1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_TYPETEXTPREFIX
   case IPROP_SPREAD_TYPETEXTPREFIX:
      if (CellType.Type == SS_TYPE_STATICTEXT)
         {
         if (CellType.Style & SS_TEXT_PREFIX)
            *(short FAR *)lParam = -1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_TYPETEXTSHADOWIN
   case IPROP_SPREAD_TYPETEXTSHADOWIN:
      if (CellType.Type == SS_TYPE_STATICTEXT)
         {
         if (CellType.Style & SS_TEXT_SHADOWIN)
            *(short FAR *)lParam = -1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);
#endif

#ifndef SS_NOCT_TIME
   case IPROP_SPREAD_TIMEMIN:
      if (CellType.Type == SS_TYPE_TIME)
         {
         if (TimeToString(Buffer, &CellType.Spec.Time.Min))
            {
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), Buffer);
            return (TRUE);
            }

         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_TIMEMAX:
      if (CellType.Type == SS_TYPE_TIME)
         {
         if (TimeToString(Buffer, &CellType.Spec.Time.Max))
            {
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), Buffer);
            return (TRUE);
            }

         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_TIMESECONDS:
      if (CellType.Type == SS_TYPE_TIME)
         {
         *(short FAR *)lParam = CellType.Spec.Time.Format.bSeconds;
         return (TRUE);
         }

      return (FALSE);

#ifdef IPROP_SPREAD_TIMESEPARATOR
   case IPROP_SPREAD_TIMESEPARATOR:
      if (CellType.Type == SS_TYPE_TIME)
         {
         *(short FAR *)lParam = (unsigned char)CellType.Spec.Time.Format.cSeparator;
         return (TRUE);
         }

      return (FALSE);
#endif

   case IPROP_SPREAD_TIME24HOUR:
      if (CellType.Type == SS_TYPE_TIME)
         {
         *(short FAR *)lParam = CellType.Spec.Time.Format.b24Hour;
         return (TRUE);
         }

      return (FALSE);
#endif

#ifndef SS_NOCT_BUTTON
   case IPROP_SPREAD_BUTTONALIGN:
      if (CellType.Type == SS_TYPE_BUTTON)
         {
         if (CellType.Style & SBS_TEXTBOT_PICTTOP)
            *(short FAR *)lParam = 0;
         else if (CellType.Style & SBS_TEXTTOP_PICTBOT)
            *(short FAR *)lParam = 1;
         else if (CellType.Style & SBS_TEXTLEFT_PICTRIGHT)
            *(short FAR *)lParam = 2;
         else if (CellType.Style & SBS_TEXTRIGHT_PICTLEFT)
            *(short FAR *)lParam = 3;

         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_BUTTONTEXT:
      if (CellType.Type == SS_TYPE_BUTTON)
         {
         if (CellType.Spec.Button.hText)
            {
            lpData = (LPSTR)GlobalLock(CellType.Spec.Button.hText);
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpData);
            GlobalUnlock(CellType.Spec.Button.hText);
            }
         else
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");

         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_BUTTONPICTNAME:
      if (CellType.Type == SS_TYPE_BUTTON)
         {
         SS_TypeGetPict(lParam, CellType.Spec.Button.hPic);
         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_BUTTONPICTDOWN:
      if (CellType.Type == SS_TYPE_BUTTON)
         {
         SS_TypeGetPict(lParam, CellType.Spec.Button.hPicDown);
         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_BUTTONTYPE:
      if (CellType.Type == SS_TYPE_BUTTON)
         {
         *(short FAR *)lParam = CellType.Spec.Button.ButtonType;
         return (TRUE);
         }

      return (FALSE);

#ifdef IPROP_SPREAD_BUTTONCOLOR
   case IPROP_SPREAD_BUTTONCOLOR:
      if (CellType.Type == SS_TYPE_BUTTON)
         {
         *(ULONG FAR *)lParam = CellType.Spec.Button.Color.Color;
         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_BUTTONBORDERCOLOR
   case IPROP_SPREAD_BUTTONBORDERCOLOR:
      if (CellType.Type == SS_TYPE_BUTTON)
         {
         *(ULONG FAR *)lParam = CellType.Spec.Button.Color.ColorBorder;
         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_BUTTONDARKCOLOR
   case IPROP_SPREAD_BUTTONDARKCOLOR:
      if (CellType.Type == SS_TYPE_BUTTON)
         {
         *(ULONG FAR *)lParam = CellType.Spec.Button.Color.ColorShadow;
         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_BUTTONLIGHTCOLOR
   case IPROP_SPREAD_BUTTONLIGHTCOLOR:
      if (CellType.Type == SS_TYPE_BUTTON)
         {
         *(ULONG FAR *)lParam = CellType.Spec.Button.Color.ColorHighlight;
         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_BUTTONTEXTCOLOR
   case IPROP_SPREAD_BUTTONTEXTCOLOR:
      if (CellType.Type == SS_TYPE_BUTTON)
         {
         *(ULONG FAR *)lParam = CellType.Spec.Button.Color.ColorText;
         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_BUTTONSHADOWSIZE
   case IPROP_SPREAD_BUTTONSHADOWSIZE:
      if (CellType.Type == SS_TYPE_BUTTON)
         {
         *(short FAR *)lParam = CellType.Spec.Button.ShadowSize;
         return (TRUE);
         }

      return (FALSE);
#endif
#endif

#ifndef SS_NOCT_COMBO
   case IPROP_SPREAD_COMBOLIST:
      if (CellType.Type == SS_TYPE_COMBOBOX)
         {
         if (CellType.Spec.ComboBox.hItems)
            {
            lpData = (LPSTR)tbGlobalLock(CellType.Spec.ComboBox.hItems);
            lpDataTemp = lpData;

            for (i = 0; i < CellType.Spec.ComboBox.dItemCnt - 1; i++)
               {
               lpDataTemp += lstrlen(lpDataTemp) + 1;
               *(lpDataTemp - 1) = '\t';
               }

            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpData);

            lpDataTemp = lpData;
            for (i = 0; i < CellType.Spec.ComboBox.dItemCnt - 1; i++)
               {
               if (!(lpDataTemp = _fstrchr(lpDataTemp, '\t')))
                  break;

               *lpDataTemp = '\0';
               lpDataTemp++;
               }

            tbGlobalUnlock(CellType.Spec.ComboBox.hItems);
            }
         else
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");

         return (TRUE);
         }

      return (FALSE);

#ifdef IPROP_SPREAD_TYPECOMBOEDITABLE
   case IPROP_SPREAD_TYPECOMBOEDITABLE:
      if (CellType.Type == SS_TYPE_COMBOBOX)
         {
         if (CellType.Style & SS_CB_DROPDOWN)
            *(short FAR *)lParam = -1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef SS_V30
   case IPROP_SPREAD_TYPECOMBOMAXDROP:
      if (CellType.Type == SS_TYPE_COMBOBOX)
         {
         *(short FAR *)lParam = CellType.Spec.ComboBox.dMaxRows;
         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_TYPECOMBOWIDTH:
      if (CellType.Type == SS_TYPE_COMBOBOX)
         {
         if (CellType.Spec.ComboBox.dComboWidth > 1 ||
             CellType.Spec.ComboBox.dComboWidth < -1)
            *(short FAR *)lParam =
               (short)VBXPixelsToTwips(CellType.Spec.ComboBox.dComboWidth);
         else
            *(short FAR *)lParam = CellType.Spec.ComboBox.dComboWidth;

         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_COMBOHWND:
      if (CellType.Type == SS_TYPE_COMBOBOX)
         {
         *(long FAR *)lParam = CellType.Spec.ComboBox.hWndDropDown;
         return (TRUE);
         }

      return (FALSE);
#endif
#endif

#ifdef IPROP_SPREAD_TYPEPICTPICTURE
   case IPROP_SPREAD_TYPEPICTPICTURE:
      if (CellType.Type == SS_TYPE_PICTURE)
         {
         *(HPIC FAR *)lParam = CellType.Spec.ViewPict.hPic;
         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_TYPEPICTCENTER
   case IPROP_SPREAD_TYPEPICTCENTER:
      if (CellType.Type == SS_TYPE_PICTURE)
         {
         if (CellType.Style & VPS_CENTER)
            *(short FAR *)lParam = -1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_TYPEPICTSTRETCH
   case IPROP_SPREAD_TYPEPICTSTRETCH:
      if (CellType.Type == SS_TYPE_PICTURE)
         {
         if (CellType.Style & VPS_STRETCH)
            *(short FAR *)lParam = -1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);
#endif

#ifdef IPROP_SPREAD_TYPEPICTSCALE
   case IPROP_SPREAD_TYPEPICTSCALE:
      if (CellType.Type == SS_TYPE_PICTURE)
         {
         if (CellType.Style & VPS_MAINTAINSCALE)
            *(short FAR *)lParam = -1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);
#endif

#ifndef SS_NOCT_CHECK
   case IPROP_SPREAD_CHECKCENTER:
      if (CellType.Type == SS_TYPE_CHECKBOX)
         {
         if (CellType.Style & BS_CENTER)
            *(short FAR *)lParam = -1;
         else
            *(short FAR *)lParam = 0;

         return (TRUE);
         }

      return (FALSE);
      
#ifdef IPROP_SPREAD_CHECKTYPE
   case IPROP_SPREAD_CHECKTYPE:
      if (CellType.Type == SS_TYPE_CHECKBOX)
         {
         if ((CellType.Style & 0x0F) == BS_AUTO3STATE)
            *(short FAR *)lParam = 1;
         else
            *(short FAR *)lParam = 0;
         return TRUE;
         }
      return FALSE;
#endif

#ifdef IPROP_SPREAD_CHECKTEXTALIGN
   case IPROP_SPREAD_CHECKTEXTALIGN:
      if (CellType.Type == SS_TYPE_CHECKBOX)
         {
         if (CellType.Style & BS_LEFTTEXT)
            *(short FAR *)lParam = 0;
         else
            *(short FAR *)lParam = 1;

         return (TRUE);
         }

      return (FALSE);
#endif

   case IPROP_SPREAD_CHECKTEXT:
      if (CellType.Type == SS_TYPE_CHECKBOX)
         {
         if (CellType.Spec.CheckBox.hText)
            {
            lpData = (LPSTR)GlobalLock(CellType.Spec.CheckBox.hText);
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), lpData);
            GlobalUnlock(CellType.Spec.CheckBox.hText);
            }
         else
            *(HSZ FAR *)lParam = VBCreateHsz(HIWORD(hCtl), "");

         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_CHECKPICTUP:
      if (CellType.Type == SS_TYPE_CHECKBOX)
         {
         SS_TypeGetPict(lParam, CellType.Spec.CheckBox.hPicUp);
         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_CHECKPICTDOWN:
      if (CellType.Type == SS_TYPE_CHECKBOX)
         {
         SS_TypeGetPict(lParam, CellType.Spec.CheckBox.hPicDown);
         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_CHECKPICTFOCUSUP:
      if (CellType.Type == SS_TYPE_CHECKBOX)
         {
         SS_TypeGetPict(lParam, CellType.Spec.CheckBox.hPicFocusUp);
         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_CHECKPICTFOCUSDOWN:
      if (CellType.Type == SS_TYPE_CHECKBOX)
         {
         SS_TypeGetPict(lParam, CellType.Spec.CheckBox.hPicFocusDown);
         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_CHECKPICTGRAY:
      if (CellType.Type == SS_TYPE_CHECKBOX)
         {
         SS_TypeGetPict(lParam, CellType.Spec.CheckBox.hPicGray);
         return (TRUE);
         }

      return (FALSE);

   case IPROP_SPREAD_CHECKPICTFOCUSGRAY:
      if (CellType.Type == SS_TYPE_CHECKBOX)
         {
         SS_TypeGetPict(lParam, CellType.Spec.CheckBox.hPicFocusGray);
         return (TRUE);
         }

      return (FALSE);
#endif

#ifndef SS_NOCT_OWNERDRAW
   case IPROP_SPREAD_OWNERDRAWSTYLE:
      if (CellType.Type == SS_TYPE_OWNERDRAW)
         {
         *(long FAR *)lParam = CellType.Style;
         return (TRUE);
         }

      return (FALSE);
#endif
   }

return (FALSE);
}


void SS_TypeGetPict(lParam, hPic)

long lParam;
HPIC hPic;
{
#if 0
PIC    Pic;
HPIC   hPic;

_fmemset(&Pic, '\0', sizeof(PIC));

if (hPict)
   {
   if (dPictType & SUPERBTN_PICT_ICON)
      {
      Pic.picType = PICTYPE_ICON;
      Pic.picData.icon.hicon = hPict;
      }
   else
      {
      Pic.picType = PICTYPE_BITMAP;
      Pic.picData.bmp.hbitmap = hPict;
      }

   /*
   hPic = VBRefPic(VBAllocPic(&Pic));
   */
   hPic = VBAllocPic(&Pic);

   *(HPIC FAR *)lParam = hPic;
   }
else
   *(HPIC FAR *)lParam = 0;
#endif

*(HPIC FAR *)lParam = hPic;
}


#ifndef SS_NOSAVE

ERR FAR PASCAL SaveToFile(hWnd, FileNum, fDataOnly)

HWND       hWnd;
USHORT     FileNum;
BOOL       fDataOnly;
{
HANDLE     hBuffer;
HCURSOR    hCursor;
char huge *lpBuffer;
long       BufferLen;
long       i;
ERR        Ret;
short      Bytes;

hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
ShowCursor(TRUE);

if (!(hBuffer = SSSaveToBuffer(hWnd, &BufferLen, fDataOnly)))
   {
   ShowCursor(FALSE);
   SetCursor(hCursor);
   return (FALSE);
   }

lpBuffer = (char huge *)GlobalLock(hBuffer);

for (i = 0; i < BufferLen; i += VBSS_FILESAVESIZE)
   {
   Bytes = (short)min((long)VBSS_FILESAVESIZE, BufferLen - (long)i);

   if ((Ret = VBWriteBasicFile(FileNum, &lpBuffer[i], Bytes)) != 0)
      break;
   }

ShowCursor(FALSE);
SetCursor(hCursor);

GlobalUnlock(hBuffer);
GlobalFree(hBuffer);
return (TRUE);
}


BOOL FAR PASCAL LoadFromFile(hWnd, FileNum)

HWND       hWnd;
USHORT     FileNum;
{
HANDLE     hBuffer = 0;
HCURSOR    hCursor;
char huge *lpBuffer;
long       lBufferLen = 0;
long       lFileSize = 0;
BOOL       Ret = TRUE;
short      nReadSize = 6;

hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
ShowCursor(TRUE);

do
   {
   if (!hBuffer)
      {
      if (!(hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                  nReadSize)))
         {
         ShowCursor(FALSE);
         SetCursor(hCursor);
         return (0);
         }
      }

   else
      {
      GlobalUnlock(hBuffer);
      if (!(hBuffer = GlobalReAlloc(hBuffer, lBufferLen + nReadSize,
                                    GMEM_MOVEABLE | GMEM_ZEROINIT)))
         {
         ShowCursor(FALSE);
         SetCursor(hCursor);
         return (0);
         }
      }

   lpBuffer = (char huge *)GlobalLock(hBuffer);

   if (VBReadBasicFile(FileNum, &lpBuffer[lBufferLen], nReadSize))
      break;

   if (lBufferLen == 0 &&
       (lpBuffer[0] == SS_FILETYPE_DATA ||
        lpBuffer[0] == SS_FILETYPE_ALL ||
        lpBuffer[0] == (SS_FILETYPE_DATA | SS_FILETYPE_UNICODE) ||
        lpBuffer[0] == (SS_FILETYPE_ALL | SS_FILETYPE_UNICODE)) &&
       lpBuffer[1] <= SS_SAVEVERSIONMAX)
      _fmemcpy(&lFileSize, &lpBuffer[2], sizeof(long));

   lBufferLen += nReadSize;
   nReadSize = (short)min(SS_FILEREADSIZE, lFileSize - lBufferLen);
   } while (lBufferLen < lFileSize);

GlobalUnlock(hBuffer);

if (lBufferLen > 0)
   Ret = SSLoadFromBuffer(hWnd, hBuffer, lFileSize);

ShowCursor(FALSE);
SetCursor(hCursor);

GlobalFree(hBuffer);
return (Ret);
}

#endif
