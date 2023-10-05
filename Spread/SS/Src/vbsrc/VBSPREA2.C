/*********************************************************
* VBSPREA2.C
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
#include "..\sssrc\classes\wintools.h"
#include "..\ssprvw\vbssprvw.h"
#include "vbmisc.h"
#include "vbspread.h"

#include "..\vbx\stringrc.h"

//--------------------------------------------------------
// All runtime enumerated properties were removed in
// order to bypass a problem that occurs in Visual
// Basic 2.0 Japanese version. (Masanori Iwasa)
//--------------------------------------------------------

extern hDynamicInst;

//--------------------------------------------------------
// The model structure has to be within the first 10K
// so it was moved up here. (Masanori Iwasa)
//--------------------------------------------------------
#ifndef SS_VB1ONLY

MODEL ModelSpread =
   {
   VB_VERSION,              // VB version being used
   MODEL_fArrows | MODEL_fFocusOk | MODEL_fLoadMsg, // MODEL flags
   (PCTLPROC)vbSpreadProc,  // Control procedure
   CS_VREDRAW | CS_HREDRAW, // Class style
   WS_BORDER | SSS_PROTECT, // Default Window style
   sizeof(VBSPREAD),        // Size of VBSPREAD structure
   IDBMP_SPREAD,            // Palette bitmap ID
   SS_DEFCTRLNAME,          // Default control name
   SS_VBCLASSNAME,          // Visual Basic class name
#ifdef SS_NOSUPERCLASS
   NULL,
#else
   VA_SPREAD_CLASS_NAME,    // Parent class name
#endif
   SpreadProperties,        // Property information table
   SpreadEvents,            // Event information table
#ifdef IPROP_SPREAD_SPREADDESIGNER
   IPROP_SPREAD_SPREADDESIGNER,
#else
   IPROP_SPREAD_ABOUT,
#endif
   0,                       // Default Event
   IPROP_SPREAD_TEXT,
//   4                        // 7-7-93
   5                        // 10-22-93
   };

LPMODEL ModelListSpread[] =
    {
    &ModelSpread,
#ifdef SS_V30
    &PV_Model,          // Preview
#endif
    NULL
    };

MODELINFO ModelInfoSpread =
    {
    VB_VERSION,             // VB version being used
    ModelListSpread         // MODEL list
    };

MODEL ModelSpreadVB2 =
   {
   VB_VERSION,              // VB version being used
   MODEL_fArrows | MODEL_fFocusOk | MODEL_fLoadMsg, // MODEL flags
   (PCTLPROC)vbSpreadProc,  // Control procedure
   CS_VREDRAW | CS_HREDRAW, // Class style
   WS_BORDER | SSS_PROTECT, // Default Window style
   sizeof(VBSPREAD),        // Size of VBSPREAD structure
   IDBMP_SPREAD,            // Palette bitmap ID
   SS_DEFCTRLNAME,          // Default control name
   SS_VBCLASSNAME,          // Visual Basic class name
#ifdef SS_NOSUPERCLASS
   NULL,
#else
   VA_SPREAD_CLASS_NAME,    // Parent class name
#endif
   SpreadProperties2,       // Property information table
   SpreadEvents,            // Event information table
#ifdef IPROP_SPREAD_SPREADDESIGNER
   IPROP_SPREAD_SPREADDESIGNER,
#else
   IPROP_SPREAD_ABOUT,
#endif
   0,
   };

LPMODEL ModelListSpreadVB2[] =
    {
    &ModelSpreadVB2,
#ifdef SS_V30
    &PV_Model,          // Preview
#endif
    NULL
    };

MODELINFO ModelInfoSpreadVB2 =
    {
    VB200_VERSION,          // VB version being used
    ModelListSpreadVB2      // MODEL list
    };

#endif

MODEL ModelSpreadVB1 =
   {
   VB100_VERSION,           // VB version being used
   MODEL_fArrows | MODEL_fFocusOk | MODEL_fLoadMsg, // MODEL flags
   (PCTLPROC)vbSpreadProc,  // Control procedure
   CS_VREDRAW | CS_HREDRAW, // Class style
   WS_BORDER | SSS_PROTECT, // Default Window style
   sizeof(VBSPREAD),        // Size of VBSPREAD structure
   IDBMP_SPREAD,            // Palette bitmap ID
   SS_DEFCTRLNAME,          // Default control name
   SS_VBCLASSNAME,          // Visual Basic class name
#ifdef SS_NOSUPERCLASS
   NULL,
#else
   VA_SPREAD_CLASS_NAME,    // Parent class name
#endif
   SpreadProperties1,       // Property information table
   SpreadEventsVB1,         // Event information table
#ifdef IPROP_SPREAD_SPREADDESIGNER
   IPROP_SPREAD_SPREADDESIGNER,
#else
   IPROP_SPREAD_ABOUT,
#endif
   0,
   };

LPMODEL ModelListSpreadVB1[] =
    {
    &ModelSpreadVB1,
#ifdef SS_V30
    &PV_Model_VB1,          // Preview
#endif
    NULL
    };

MODELINFO ModelInfoSpreadVB1 =
    {
    VB100_VERSION,          // VB version being used
    ModelListSpreadVB1      // MODEL list
    };


/*******************************
* Visual Basic Data Structures
*******************************/

#ifdef SS_USE16BITCOORDS
#define SS_DT_ROWCOL DT_SHORT
#else
#define SS_DT_ROWCOL DT_LONG
#endif

#ifndef SSP_NO_ACTION
/*
static CHAR szActionTypes[] =
   {
   "0  - Activate Cell\0"\
   "1  - Go To\0"\
   "2  - Select Block\0"\
   "3  - Clear\0"\
   "4  - Delete Col\0"\
   "5  - Delete Row\0"\
   "6  - Insert Col\0"\
   "7  - Insert Row\0"\
   "8  - Load\0"\
   "9  - Save All\0"\
   "10 - Save Values\0"\
   "11 - Recalculate\0"\
   "12 - Clear Text\0"\
   "13 - Print\0"\
   "14 - Deselect Block\0"\
   "15 - DSave\0"\
   "16 - Set Border\0"\
   "17 - Add MultiSel Blocks\0"\
   "18 - Get MultiSel Blocks\0"\
   "19 - Copy Range\0"\
   "20 - Move Range\0"\
   "21 - Swap Range\0"\
   "22 - Clipboard Copy\0"\
   "23 - Clipboard Cut\0"\
   "24 - Clipboard Paste\0"\
   "25 - Sort\0"\
   "26 - Combo Clear\0"\
   "27 - Combo Remove String\0"\
   "28 - Reset\0"\
   "29 - Select Mode Clear\0"\
   "30 - VRefresh Buffer\0"\
   "31 - Data Refresh\0"\
   "32 - Smart Print\0"\
   ""
   };
*/
static PROPINFO PropertyAction =
   {
   "Action",
   DT_ENUM | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, "", 32
   };
#else
NOTUSED(PropertyAction, "P21");
#endif

#ifndef SSP_NO_ACTIVECELLCOL
static PROPINFO PropertyActiveCellCol =
   {
   "ActiveCol",
   DT_LONG | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyActiveCellCol, "P22");
#endif

#ifndef SSP_NO_ACTIVECELLROW
static PROPINFO PropertyActiveCellRow =
   {
   "ActiveRow",
   DT_LONG | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyActiveCellRow, "P23");
#endif

#if 0
#if (!defined(SS_GRID) || !defined(SS_BC))
static PROPINFO PropertyUtopia =
   {
#ifdef SS_UTP
   "UtopiaMode",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
#else
   "AllowResize",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
#endif
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyUtopia, "P24");
#endif
#endif

#ifndef SS_V30
#ifndef SSP_NO_ALLOWSELECTBLOCK
static PROPINFO PropertyAllowSelectBlock =
   {
   "AllowSelBlock",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyAllowSelectBlock, "P25");
#endif

#else

#ifndef SSP_NO_APPEARANCE
static CHAR szAppearance[] =
   {
   "0 - Flat\0"\
   "1 - 3D\0"\
   "2 - 3D with border\0"\
   };
static PROPINFO PropertyAppearance =
   {
   "Appearance",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, szAppearance, 2
   };
#else
NOTUSED(PropertyAppearance, "P25");
#endif

#endif

#ifndef SSP_NO_AUTOCALC
static PROPINFO PropertyAutoCalc =
   {
   "AutoCalc",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyAutoCalc, "P26");
#endif

#ifndef SSP_NO_BACKCOLOR
static PROPINFO PropertyBackColor =
   {
   "BackColor",
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyBackColor, "P27");
#endif

#ifndef SS_V30

#ifndef SSP_NO_CALCDEPENDENCIES
static PROPINFO PropertyCalcDependencies =
   {
   "CalcDependencies",
#ifdef SS_V21
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
#else
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef | PF_fNoShow,
#endif
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyCalcDependencies, "P28");
#endif

#else

#ifndef SSP_NO_TYPECOMBOBOXWIDTH
RWRT_PROP(TypeComboBoxWidth, DT_SHORT);
#else
NOTUSED(PropertyTypeComboBoxWidth, "P28");
#endif

#endif

#ifndef SSP_NO_CELLTYPE
/*
static CHAR szCellTypeTypes[] =
   {
   "0 - Date\0"\
   "1 - Edit\0"\
   "2 - Float\0"\
   "3 - Integer\0"\
   "4 - Pic\0"\
   "5 - Static Text\0"\
   "6 - Time\0"\
   "7 - Button\0"\
   "8 - Combo Box\0"\
   "9 - Picture\0"\
   "10 - Check Box\0"\
   "11 - Owner Draw\0"\
   };
*/
static PROPINFO PropertyCellType =
   {
   "CellType",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, CellType),
   0, 0, "", 11
   };
#else
NOTUSED(PropertyCellType, "P29");
#endif

#ifndef SSP_NO_CLIP
static PROPINFO PropertyClip =
   {
   "Clip",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyClip, "P30");
#endif

#ifndef SSP_NO_COL
static PROPINFO PropertyCol =
   {
   "Col",
   SS_DT_ROWCOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyCol, "P31");
#endif

#ifndef SSP_NO_COL2
static PROPINFO PropertyCol2 =
   {
   "Col2",
   SS_DT_ROWCOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyCol2, "P32");
#endif

#ifndef SSP_NO_COLWIDTH
static PROPINFO PropertyColWidth =
   {
   "ColWidth",
   DT_REAL | PF_fPropArray | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   };
#else
NOTUSED(PropertyColWidth, "P33");
#endif

#ifndef SSP_NO_DISPLAYCOLHDRS
static PROPINFO PropertyDisplayColHeaders =
   {
   "DisplayColHeaders",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyDisplayColHeaders, "P34");
#endif

#ifndef SSP_NO_DISPLAYROWHDRS
static PROPINFO PropertyDisplayRowHeaders =
   {
   "DisplayRowHeaders",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyDisplayRowHeaders, "P35");
#endif

#ifndef SSP_NO_EDITMODE
static PROPINFO PropertyEditMode =
   {
   "EditMode",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyEditMode, "P36");
#endif

#ifndef SSP_NO_FILENUM
static PROPINFO PropertyFileNum =
   {
   "FileNum",
   DT_SHORT | PF_fSetData | PF_fGetData | PF_fSaveData | PF_fNoShow | PF_fDefVal | PF_fNoInitDef,
   OFFSETIN(VBSPREAD, FileNum),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyFileNum, "P37");
#endif

#ifndef SSP_NO_FORECOLOR
static PROPINFO PropertyForeColor =
   {
   "ForeColor",
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyForeColor, "P38");
#endif

#ifndef SSP_NO_FORMULA
static PROPINFO PropertyFormula =
   {
   "Formula",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyFormula, "P39");
#endif

#ifndef SS_V30

#ifndef SSP_NO_GRIDLINES
static PROPINFO PropertyGridLines =
   {
   "GridLines",
   DT_BOOL | PF_fSetMsg | PF_fSetData | PF_fGetMsg | PF_fNoShow,
   OFFSETIN(VBSPREAD, GridLines),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyGridLines, "P40");
#endif

#else

#ifndef SSP_NO_TYPECOMBOBOXMAXDROP
RWRT_PROP(TypeComboBoxMaxDrop, DT_SHORT);
#else
NOTUSED(PropertyTypeComboBoxMaxDrop, "P40");
#endif

#endif

#ifndef SSP_NO_LEFTCOL
static PROPINFO PropertyLeftCol =
   {
   "LeftCol",
   SS_DT_ROWCOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyLeftCol, "P41");
#endif

#ifndef SSP_NO_LOCK
static PROPINFO PropertyLock =
   {
   "Lock",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyLock, "P42");
#endif

#ifndef SSP_NO_MAXCOLS
static PROPINFO PropertyMaxCols =
   {
   "MaxCols",
   SS_DT_ROWCOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 500, NULL, 0
   };
#else
NOTUSED(PropertyMaxCols, "P43");
#endif

#ifndef SSP_NO_MAXROWS
static PROPINFO PropertyMaxRows =
   {
   "MaxRows",
   SS_DT_ROWCOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 500, NULL, 0
   };
#else
NOTUSED(PropertyMaxRows, "P44");
#endif

#ifndef SSP_NO_POSITION
static CHAR szPositionTypes[145];
/*
static CHAR szPositionTypes[] =
   {
   "0 - Upper Left\0"\
   "1 - Upper Center\0"\
   "2 - Upper Right\0"\
   "3 - Center Left\0"\
   "4 - Center\0"\
   "5 - Center Right\0"\
   "6 - Bottom Left\0"\
   "7 - Bottom Center\0"\
   "8 - Bottom Right\0"\
   ""
   };
*/
static PROPINFO PropertyPosition =
   {
   "Position",
   DT_ENUM | PF_fSetData | PF_fGetData | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   OFFSETIN(VBSPREAD, Position),
   0, 0,
   szPositionTypes, 8
   };
#else
NOTUSED(PropertyPosition, "P45");
#endif

#ifndef SSP_NO_PROTECT
static PROPINFO PropertyProtect =
   {
   "Protect",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyProtect, "P46");
#endif

#ifndef SSP_NO_REDRAW
static PROPINFO PropertyRedraw =
   {
   "ReDraw",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyRedraw, "P47");
#endif

#ifndef SSP_NO_RESTRICTCOLS
static PROPINFO PropertyRestrictCols =
   {
   "RestrictCols",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyRestrictCols, "P48");
#endif

#ifndef SSP_NO_RESTRICTROWS
static PROPINFO PropertyRestrictRows =
   {
   "RestrictRows",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyRestrictRows, "P49");
#endif

#ifndef SSP_NO_ROW
static PROPINFO PropertyRow =
   {
   "Row",
   SS_DT_ROWCOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyRow, "P50");
#endif

#ifndef SSP_NO_ROW2
static PROPINFO PropertyRow2 =
   {
   "Row2",
   SS_DT_ROWCOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyRow2, "P51");
#endif

#ifndef SSP_NO_ROWHEIGHT
static PROPINFO PropertyRowHeight =
   {
   "RowHeight",
   DT_REAL | PF_fPropArray | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyRowHeight, "P52");
#endif

#ifndef SSP_NO_SELECTBLOCKCOL
static PROPINFO PropertySelectBlockCol =
   {
   "SelBlockCol",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertySelectBlockCol, "P53");
#endif

#ifndef SSP_NO_SELECTBLOCKCOL2
static PROPINFO PropertySelectBlockCol2 =
   {
   "SelBlockCol2",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertySelectBlockCol2, "P54");
#endif

#ifndef SSP_NO_SELECTBLOCKROW
static PROPINFO PropertySelectBlockRow =
   {
   "SelBlockRow",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertySelectBlockRow, "P55");
#endif

#ifndef SSP_NO_SELECTBLOCKROW2
static PROPINFO PropertySelectBlockRow2 =
   {
   "SelBlockRow2",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertySelectBlockRow2, "P56");
#endif

#ifndef SSP_NO_SELLENGTH
static PROPINFO PropertySelLength =
   {
   "SelLength",
   DT_LONG | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertySelLength, "P57");
#endif

#ifndef SSP_NO_SELSTART
static PROPINFO PropertySelStart =
   {
   "SelStart",
   DT_LONG | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, SelStart),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertySelStart, "P58");
#endif

#ifndef SSP_NO_SELTEXT
static PROPINFO PropertySelText =
   {
   "SelText",
   DT_HSZ | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertySelText, "P59");
#endif

#ifndef SSP_NO_SHADOWCOLOR
static PROPINFO PropertyShadowColor =
   {
   "ShadowColor",
/*
   DT_COLOR | PF_fSetMsg | PF_fSetData | PF_fGetData | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   OFFSETIN(VBSPREAD, ShadowColor),
   0, 0xC0C0C0, NULL, 0
*/
   DT_COLOR | PF_fSetMsg | PF_fSetData | PF_fGetData | PF_fSaveData,
   OFFSETIN(VBSPREAD, ShadowColor),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyShadowColor, "P60");
#endif

#ifndef SSP_NO_SHADOWDARK
static PROPINFO PropertyShadowDark =
   {
   "ShadowDark",
/*
   DT_COLOR | PF_fSetMsg | PF_fSetData | PF_fGetData | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   OFFSETIN(VBSPREAD, ShadowDark),
   0, 0x808080, NULL, 0
*/
   DT_COLOR | PF_fSetMsg | PF_fSetData | PF_fGetData | PF_fSaveData,
   OFFSETIN(VBSPREAD, ShadowDark),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyShadowDark, "P61");
#endif

#ifndef SSP_NO_SHADOWTEXT
static PROPINFO PropertyShadowText =
   {
   "ShadowText",
//   DT_COLOR | PF_fSetMsg | PF_fSetData | PF_fGetData | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   DT_COLOR | PF_fSetMsg | PF_fSetData | PF_fGetData | PF_fSaveData,
   OFFSETIN(VBSPREAD, ShadowText),
   0, 0x0, NULL, 0
   };
#else
NOTUSED(PropertyShadowText, "P62");
#endif

#ifndef SSP_NO_TEXT
static PROPINFO PropertyText =
   {
   "Text",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyText, "P63");
#endif

#ifndef SSP_NO_TOPROW
static PROPINFO PropertyTopRow =
   {
   "TopRow",
   SS_DT_ROWCOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTopRow, "P64");
#endif

#ifndef SSP_NO_DATECENTURY
static PROPINFO PropertyTypeDateCentury =
   {
   "TypeDateCentury",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeDateFormat.bCentury),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeDateCentury, "P65");
#endif

#ifndef SSP_NO_DATEFORMAT
/*
static CHAR szTypeDateFormatTypes[] =
   {
   "0 - DDMONYY\0"\
   "1 - DDMMYY\0"\
   "2 - MMDDYY\0"\
   "3 - YYMMDD\0"\
   "4 - YYMM\0"\
   "5 - MMDD\0"\
   "6 - NYYMMDD\0"\
   "7 - NNYYMMDD\0"\
   "8 - NNNNYYMMDD\0"\
   ""
   };
*/
static PROPINFO PropertyTypeDateFormat =
   {
   "TypeDateFormat",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, "", 8
   };
#else
NOTUSED(PropertyTypeDateFormat, "P66");
#endif

#ifndef SSP_NO_DATEMAX
static PROPINFO PropertyTypeDateMax =
   {
   "TypeDateMax",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeDateMax, "P67");
#endif

#ifndef SSP_NO_DATEMIN
static PROPINFO PropertyTypeDateMin =
   {
   "TypeDateMin",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeDateMin, "P68");
#endif

#ifndef SSP_NO_DATESEPARATOR
static PROPINFO PropertyTypeDateSeparator =
   {
   "TypeDateSeparator",
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeDateFormat.cSeparator),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeDateSeparator, "P69");
#endif

#ifndef SS_V30
#ifndef SSP_NO_EDITCASE
/*
static CHAR szEditCharCaseTypes[] =
   {
   "0 - Lowercase\0"\
   "1 - No Case\0"\
   "2 - Uppercase\0"\
   ""
   };
*/
static PROPINFO PropertyTypeEditCase =
   {
   "TypeEditCase",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeEditCharCase),
   0, 0, "", 2
   };
#else
NOTUSED(PropertyTypeEditCase, "P70");
#endif

#else

#ifndef SSP_NO_TYPEVALIGN
static PROPINFO PropertyTypeVAlign =
   {
   "TypeVAlign",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, "", 2
   };
#else
NOTUSED(PropertyTypeVAlign, "P70");
#endif
#endif

#ifndef SSP_NO_EDITCHARSET
/*
static CHAR szEditCharSetTypes[] =
   {
   "0 - ASCII\0"\
   "1 - Alpha\0"\
   "2 - Alphanumeric\0"\
   "3 - Numeric\0"\
   "4 - DBCS Character"
   "5 - DBCS Character with Auto FEP"
   "6 - All Characters with Auto FEP"
   };
*/
static PROPINFO PropertyTypeEditCharSet =
   {
   "TypeEditCharSet",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeEditCharSet),
   0, 0, "", 6
   };
#else
NOTUSED(PropertyTypeEditCharSet, "P71");
#endif

#if (!defined(SSP_NO_EDITJUSTIFY) || !defined(SSP_NO_TYPEHALIGN))
/*
static CHAR szEditJustifyTypes[] =
   {
   "0 - Left\0"\
   "1 - Right\0"\
   "2 - Center\0"\
   };
*/
#endif

#ifdef SS_V30
#ifndef SSP_NO_TEXTTIP

static CHAR szTextTip[] =
   {
   "0 - Off\0"\
   "1 - Fixed\0"\
   "2 - Floating\0"\
   "3 - Fixed Focus Only\0"\
   "4 - Floating Focus Only\0"\
   };

static PROPINFO PropertyTextTip =
   {
   "TextTip",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, szTextTip, 4
   };
#else
NOTUSED(PropertyTextTip, "P72");
#endif
#else
#ifndef SSP_NO_EDITJUSTIFY
static PROPINFO PropertyTypeEditJustify =
   {
   "TypeEditAlign",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, "", 2
   };
#else
NOTUSED(PropertyTypeEditJustify, "P72");
#endif
#endif

#ifndef SSP_NO_EDITLEN
static PROPINFO PropertyTypeEditLen =
   {
#ifndef SS_V30
   "TypeEditLen",
#else
   "TypeMaxEditLen",
#endif
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeEditLen, "P73");
#endif

#ifndef SSP_NO_FLOATDECPLACES
PROPINFO PropertyTypeFloatDecimalPlaces =
   {
   "TypeFloatDecimalPlaces",
   DT_SHORT | PF_fGetMsg | PF_fSetData | PF_fSetMsg | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeFloatDecimalPlaces),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeFloatDecimalPlaces, "P74");
#endif

#ifndef SSP_NO_FLOATMAX
static PROPINFO PropertyTypeFloatMax =
   {
   "TypeFloatMax",
   DT_HSZ | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeFloatMax, "P75");
#endif

#ifndef SSP_NO_FLOATMIN
static PROPINFO PropertyTypeFloatMin =
   {
   "TypeFloatMin",
   DT_HSZ | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeFloatMin, "P76");
#endif

#ifndef SSP_NO_FLOATMONEY
PROPINFO PropertyTypeFloatMoney =
   {
   "TypeFloatMoney",
   DT_BOOL | PF_fGetMsg | PF_fSetData | PF_fSetMsg | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeFloatMoney),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeFloatMoney, "P77");
#endif

#ifndef SSP_NO_FLOATSEPARATOR
PROPINFO PropertyTypeFloatSeparator =
   {
   "TypeFloatSeparator",
   DT_BOOL | PF_fGetMsg | PF_fSetData | PF_fSetMsg | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeFloatSeparator),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeFloatSeparator, "P78");
#endif

#ifndef SSP_NO_INTEGERMAX
static PROPINFO PropertyTypeIntegerMax =
   {
   "TypeIntegerMax",
   DT_LONG | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeIntegerMax),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeIntegerMax, "P79");
#endif

#ifndef SSP_NO_INTEGERMIN
static PROPINFO PropertyTypeIntegerMin =
   {
   "TypeIntegerMin",
   DT_LONG | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeIntegerMin),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeIntegerMin, "P80");
#endif

#ifndef SSP_NO_PICMASK
static PROPINFO PropertyTypePicMask =
   {
   "TypePicMask",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypePicMask, "P81");
#endif

#ifdef SS_V30
#ifndef SSP_NO_TEXTTIPDELAY
static PROPINFO PropertyTextTipDelay =
   {
   "TextTipDelay",
   DT_LONG | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 500, NULL, 0
   };
#else
NOTUSED(PropertyTextTipDelay, "P82");
#endif
#else
#ifndef SSP_NO_TEXTJUSTIFYHORIZ
/*
static CHAR szTextJustifyHorizTypes[] =
   {
   "0 - Center\0"\
   "1 - Left\0"\
   "2 - Right\0"\
   ""
   };
*/
static PROPINFO PropertyTypeTextJustifyHoriz =
   {
   "TypeTextAlignHoriz",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, "", 2
   };
#else
NOTUSED(PropertyTypeTextJustifyHoriz, "P82");
#endif
#endif

#ifndef SSP_NO_TEXTJUSTIFYVERT
/*
static CHAR szTextJustifyVertTypes[] =
   {
   "0 - Bottom\0"\
   "1 - Center\0"\
   "2 - Top\0"\
   ""
   };
*/
static PROPINFO PropertyTypeTextJustifyVert =
   {
   "TypeTextAlignVert",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeTextJustifyVert),
   0, 0, "", 2
   };
#else
NOTUSED(PropertyTypeTextJustifyVert, "P83");
#endif

#ifndef SSP_NO_TEXTSHADOW
static PROPINFO PropertyTypeTextShadow =
   {
   "TypeTextShadow",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeTextShadow),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeTextShadow, "P84");
#endif

#ifndef SSP_NO_TEXTWORDWRAP
static PROPINFO PropertyTypeTextWordWrap =
   {
   "TypeTextWordWrap",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeTextWordWrap),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeTextWordWrap, "P85");
#endif

#ifndef SSP_NO_TIMEMAX
static PROPINFO PropertyTypeTimeMax =
   {
   "TypeTimeMax",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeTimeMax, "P86");
#endif

#ifndef SSP_NO_TIMEMIN
static PROPINFO PropertyTypeTimeMin =
   {
   "TypeTimeMin",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeTimeMin, "P87");
#endif

#ifndef SSP_NO_TIMESECONDS
static PROPINFO PropertyTypeTimeSeconds =
   {
   "TypeTimeSeconds",
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeTimeFormat.bSeconds),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeTimeSeconds, "P88");
#endif

#ifndef SSP_NO_TIMESEPARATOR
static PROPINFO PropertyTypeTimeSeparator =
   {
   "TypeTimeSeparator",
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeTimeFormat.cSeparator),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeTimeSeparator, "P89");
#endif

#ifndef SSP_NO_TIME24HOUR
/*
static CHAR szTypeTime24HourTypes[] =
   {
   "0 - 12 Hour\0"\
   "1 - 24 Hour\0"\
   "2 - 12 Hour DBCS\0"\
   "3 - DBCS 12 Hour\0"\
   ""
   };
*/
static PROPINFO PropertyTypeTime24Hour =
   {
   "TypeTime24Hour",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeTimeFormat.b24Hour),
   0, 0,
   "", 3
   };
#else
NOTUSED(PropertyTypeTime24Hour, "P90");
#endif

#ifndef SSP_NO_BLOCKMODE
static PROPINFO PropertyBlockMode =
   {
   "BlockMode",
   DT_BOOL | PF_fSetData | PF_fGetData | PF_fSaveData | PF_fNoShow | PF_fDefVal | PF_fNoInitDef,
   OFFSETIN(VBSPREAD, BlockMode),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyBlockMode, "P91");
#endif

#ifndef SSP_NO_SCROLLBARS
static CHAR szScrollBarsTypes[48];
/*
static CHAR szScrollBarsTypes[] =
   {
   "0 - None\0"\
   "1 - Horizontal\0"\
   "2 - Vertical\0"\
   "3 - Both\0"\
   ""
   };
*/
static PROPINFO PropertyScrollBars =
   {
   "ScrollBars",
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 3, szScrollBarsTypes, 3
   };
#else
NOTUSED(PropertyScrollBars, "P92");
#endif

#ifndef SSP_NO_ISBLOCKSELECTED
static PROPINFO PropertyIsBlockSelected =
   {
   "IsBlockSelected",
   DT_BOOL | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyIsBlockSelected, "P93");
#endif

#ifndef SSP_NO_SPREADDESIGNER
static PROPINFO PropertySpreadDesigner =
   {
   "InterfaceDesigner",
   //"SpreadDesigner",
   DT_HSZ | PF_fSaveMsg,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertySpreadDesigner, "P94");
#endif

#ifndef SSP_NO_VALUE
static PROPINFO PropertyValue =
   {
   "Value",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyValue, "P95");
#endif

#ifndef SSP_NO_PICDEFTEXT
static PROPINFO PropertyTypePicDefText =
   {
   "TypePicDefaultText",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypePicDefText, "P96");
#endif

#ifndef SSP_NO_EDITMODEPERMANENT
static PROPINFO PropertyEditModePermanent =
   {
   "EditModePermanent",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyEditModePermanent, "P97");
#endif

#ifndef SSP_NO_AUTOSIZE
static PROPINFO PropertyAutoSize =
   {
   "AutoSize",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyAutoSize, "P98");
#endif

#ifndef SSP_NO_VISIBLEROWS
static PROPINFO PropertyVisibleRows =
   {
   "VisibleRows",
   SS_DT_ROWCOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyVisibleRows, "P99");
#endif

#ifndef SSP_NO_VISIBLECOLS
static PROPINFO PropertyVisibleCols =
   {
   "VisibleCols",
   SS_DT_ROWCOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyVisibleCols, "P100");
#endif

#ifndef SSP_NO_ROWSFROZEN
static PROPINFO PropertyRowsFrozen =
   {
   "RowsFrozen",
   SS_DT_ROWCOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyRowsFrozen, "P101");
#endif

#ifndef SSP_NO_COLSFROZEN
static PROPINFO PropertyColsFrozen =
   {
   "ColsFrozen",
   SS_DT_ROWCOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyColsFrozen, "P102");
#endif

#ifndef SSP_NO_SCROLLBAREXTMODE
static PROPINFO PropertyScrollBarExtMode =
   {
   "ScrollBarExtMode",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyScrollBarExtMode, "P103");
#endif

#ifndef SSP_NO_BUTTONALIGN
/*
static CHAR szTypeButtonAlignTypes[] =
   {
   "0 - Text Bottom/Picture Top\0"\
   "1 - Text Top/Picture Bottom\0"\
   "2 - Text Left/Picture Right\0"\
   "3 - Text Right/Picture Left\0"\
   ""
   };
*/
static PROPINFO PropertyTypeButtonAlign =
   {
   "TypeButtonAlign",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeButtonAlign),
   0, 0,
   "", 3
   };
#else
NOTUSED(PropertyTypeButtonAlign, "P104");
#endif

#ifndef SSP_NO_BUTTONTEXT
static PROPINFO PropertyTypeButtonText =
   {
   "TypeButtonText",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeButtonText, "P105");
#endif

#ifndef SSP_NO_BUTTONPICTNAME
static PROPINFO PropertyTypeButtonPictureName =
   {
   "TypeButtonPicture",
   DT_PICTURE | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeButtonPictureName, "P106");
#endif

#ifndef SSP_NO_BUTTONPICTDOWN
static PROPINFO PropertyTypeButtonPictDownName =
   {
   "TypeButtonPictureDown",
   DT_PICTURE | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeButtonPictDownName, "P107");
#endif

#ifndef SSP_NO_BUTTONTYPE
/*
static CHAR szTypeButtonType[] =
   {
   "0 - Normal\0"\
   "1 - 2 State\0"\
   ""
   };
*/
static PROPINFO PropertyTypeButtonType =
   {
   "TypeButtonType",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeButtonType),
   0, 0,
   "", 1
   };
#else
NOTUSED(PropertyTypeButtonType, "P108");
#endif

#ifndef SSP_NO_BUTTONCOLOR
static PROPINFO PropertyTypeButtonColor =
   {
   "TypeButtonColor",
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeButtonColor.Color),
   0, 0, 0, 0
   };
#else
NOTUSED(PropertyTypeButtonColor, "P109");
#endif

#ifndef SSP_NO_BUTTONBORDERCOLOR
static PROPINFO PropertyTypeButtonBorderColor =
   {
   "TypeButtonBorderColor",
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeButtonColor.ColorBorder),
   0, 0, 0, 0
   };
#else
NOTUSED(PropertyTypeButtonBorderColor, "P110");
#endif

#ifndef SSP_NO_BUTTONDARKCOLOR
static PROPINFO PropertyTypeButtonDarkColor =
   {
   "TypeButtonDarkColor",
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeButtonColor.ColorShadow),
   0, 0, 0, 0
   };
#else
NOTUSED(PropertyTypeButtonDarkColor, "P111");
#endif

#ifndef SSP_NO_BUTTONLIGHTCOLOR
static PROPINFO PropertyTypeButtonLightColor =
   {
   "TypeButtonLightColor",
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeButtonColor.ColorHighlight),
   0, 0, 0, 0
   };
#else
NOTUSED(PropertyTypeButtonLightColor, "P112");
#endif

#ifndef SSP_NO_BUTTONTEXTCOLOR
static PROPINFO PropertyTypeButtonTextColor =
   {
   "TypeButtonTextColor",
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeButtonColor.ColorText),
   0, 0, 0, 0
   };
#else
NOTUSED(PropertyTypeButtonTextColor, "P113");
#endif

#ifndef SSP_NO_PROCESSTAB
static PROPINFO PropertyProcessTab =
   {
   "ProcessTab",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyProcessTab, "P114");
#endif

#ifndef SSP_NO_BUTTONSHADOWSIZE
static PROPINFO PropertyTypeButtonShadowSize =
   {
   "TypeButtonShadowSize",
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeButtonShadowSize),
   0, 0, 0, 0
   };
#else
NOTUSED(PropertyTypeButtonShadowSize, "P115");
#endif

#ifndef SSP_NO_SCROLLBARSHOWMAX
static PROPINFO PropertyScrollBarShowMax =
   {
   "ScrollBarShowMax",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyScrollBarShowMax, "P116");
#endif

#ifndef SSP_NO_SCROLLBARMAXALIGN
static PROPINFO PropertyScrollBarMaxAlign =
   {
   "ScrollBarMaxAlign",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyScrollBarMaxAlign, "P117");
#endif

#ifndef SSP_NO_LOCKBACKCOLOR
static PROPINFO PropertyLockBackColor =
   {
   "LockBackColor",
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyLockBackColor, "P118");
#endif

#ifndef SSP_NO_LOCKFORECOLOR
static PROPINFO PropertyLockForeColor =
   {
   "LockForeColor",
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyLockForeColor, "P119");
#endif

#ifndef SSP_NO_EDITPASSWORD
static PROPINFO PropertyTypeEditPassword =
   {
   "TypeEditPassword",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeEditPassword),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeEditPassword, "P120");
#endif

#ifndef SSP_NO_COMBOLIST
static PROPINFO PropertyTypeComboList =
   {
   "TypeComboBoxList",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeComboList, "P121");
#endif

#ifndef SSP_NO_PRINTJOBNAME
static PROPINFO PropertyPrintJobName =
   {
   "PrintJobName",
   DT_HSZ | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyPrintJobName, "P122");
#endif

#ifndef SSP_NO_PRINTABORTMSG
static PROPINFO PropertyPrintAbortMsg =
   {
   "PrintAbortMsg",
   DT_HSZ | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyPrintAbortMsg, "P123");
#endif

#ifndef SSP_NO_PRINTHEADER
static PROPINFO PropertyPrintHeader =
   {
   "PrintHeader",
   DT_HSZ | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyPrintHeader, "P124");
#endif

#ifndef SSP_NO_PRINTFOOTER
static PROPINFO PropertyPrintFooter =
   {
   "PrintFooter",
   DT_HSZ | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyPrintFooter, "P125");
#endif

#ifndef SSP_NO_PRINTBORDER
static PROPINFO PropertyPrintBorder =
   {
   "PrintBorder",
   DT_BOOL | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintBorder),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyPrintBorder, "P126");
#endif

#ifndef SSP_NO_PRINTORIENTATION
static PROPINFO PropertyPrintOrientation =
   {
   "PrintOrientation",
   DT_ENUM | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintOrientation),
   0, 0, "", 2
   };
#else
NOTUSED(PropertyPrintOrientation, "P24");
#endif

#ifndef SSP_NO_PRINTSHADOWS
static PROPINFO PropertyPrintShadows =
   {
   "PrintShadows",
   DT_BOOL | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintShadows),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyPrintShadows, "P127");
#endif

#ifndef SSP_NO_PRINTGRID
static PROPINFO PropertyPrintGrid =
   {
   "PrintGrid",
   DT_BOOL | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintGrid),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyPrintGrid, "P128");
#endif

#ifndef SSP_NO_PRINTCOLHEADERS
static PROPINFO PropertyPrintColHeaders =
   {
   "PrintColHeaders",
   DT_BOOL | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintColHeaders),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyPrintColHeaders, "P129");
#endif

#ifndef SSP_NO_PRINTROWHEADERS
static PROPINFO PropertyPrintRowHeaders =
   {
   "PrintRowHeaders",
   DT_BOOL | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintRowHeaders),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyPrintRowHeaders, "P130");
#endif

#ifndef SSP_NO_PRINTUSEDATAMAX
static PROPINFO PropertyPrintUseDataMax =
   {
   "PrintUseDataMax",
   DT_BOOL | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintUseDataMax),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyPrintUseDataMax, "P131");
#endif

#ifndef SSP_NO_PRINTTYPE
/*
static CHAR szPrintType[] =
   {
   "0 - All\0"\
   "1 - Cell Range\0"\
   "2 - Current Page\0"\
   "3 - Page Range\0"\
   ""
   };
*/
static PROPINFO PropertyPrintType =
   {
   "PrintType",
   DT_ENUM | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintType),
   0, 0, "", 3
   };
#else
NOTUSED(PropertyPrintType, "P132");
#endif

#ifndef SSP_NO_PRINTPAGESTART
static PROPINFO PropertyPrintPageStart =
   {
   "PrintPageStart",
   DT_SHORT | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintPageStart),
   0, 0, 0, 0
   };
#else
NOTUSED(PropertyPrintPageStart, "P133");
#endif

#ifndef SSP_NO_PRINTPAGEEND
static PROPINFO PropertyPrintPageEnd =
   {
   "PrintPageEnd",
   DT_SHORT | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintPageEnd),
   0, 0, 0, 0
   };
#else
NOTUSED(PropertyPrintPageEnd, "P134");
#endif

#ifndef SSP_NO_PRINTMARGINLEFT
static PROPINFO PropertyPrintMarginLeft =
   {
   "PrintMarginLeft",
   DT_LONG | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintMarginLeft),
   0, 0, 0, 0
   };
#else
NOTUSED(PropertyPrintMarginLeft, "P135");
#endif

#ifndef SSP_NO_PRINTMARGINTOP
static PROPINFO PropertyPrintMarginTop =
   {
   "PrintMarginTop",
   DT_LONG | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintMarginTop),
   0, 0, 0, 0
   };
#else
NOTUSED(PropertyPrintMarginTop, "P136");
#endif

#ifndef SSP_NO_PRINTMARGINRIGHT
static PROPINFO PropertyPrintMarginRight =
   {
   "PrintMarginRight",
   DT_LONG | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintMarginRight),
   0, 0, 0, 0
   };
#else
NOTUSED(PropertyPrintMarginRight, "P137");
#endif

#ifndef SSP_NO_PRINTMARGINBOTTOM
static PROPINFO PropertyPrintMarginBottom =
   {
   "PrintMarginBottom",
   DT_LONG | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintMarginBottom),
   0, 0, 0, 0
   };
#else
NOTUSED(PropertyPrintMarginBottom, "P138");
#endif

#ifndef SSP_NO_UNITTYPE
static CHAR szUnitType[40];
/*
static CHAR szUnitType[] =
   {
   "0 - Normal  \0"\
   "1 - VGA Base \0"\
   "2 - Twips\0"\
   ""
   };
*/
static PROPINFO PropertyUnitType =
   {
   "UnitType",
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, szUnitType, 2
   };
#else
NOTUSED(PropertyUnitType, "P139");
#endif

#ifndef SSP_NO_EDITENTERACTION
static CHAR szEditEnterAction[122];
/*
static CHAR szEditEnterAction[] =
   {
   "0 - None\0"\
   "1 - Up\0"\
   "2 - Down\0"\
   "3 - Left\0"\
   "4 - Right\0"\
   "5 - Next\0"\
   "6 - Previous\0"\
   "7 - Same\0"\
   "8 - Next Row\0"\
   ""
   };
*/
static PROPINFO PropertyEditEnterAction =
   {
   "EditEnterAction",
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, szEditEnterAction, 8
   };
#else
NOTUSED(PropertyEditEnterAction, "P140");
#endif

#ifndef SSP_NO_GRAYAREABACKCOLOR
static PROPINFO PropertyGrayAreaBackColor =
   {
   "GrayAreaBackColor",
//   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fSaveData,
#ifdef SS_UTP
   0, 0, 0xFFFFFF, NULL, 0
#else
//   0, 0, 0xC0C0C0, NULL, 0
   0, 0, 0, NULL, 0
#endif
   };
#else
NOTUSED(PropertyGrayAreaBackColor, "P141");
#endif

#ifndef SS_V30

#ifndef SSP_NO_GRAYAREAFORECOLOR
static PROPINFO PropertyGrayAreaForeColor =
   {
   "GrayAreaForeColor",
   DT_COLOR | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyGrayAreaForeColor, "P142");
#endif

#else

#ifndef SSP_NO_SCROLLBARTRACK
static CHAR szScrollBarTrack[] =
   {
   "0 - Off\0"\
   "1 - Vertical\0"\
   "2 - Horizontal\0"\
   "3 - Both\0"\
   ""
   };
static PROPINFO PropertyScrollBarTrack =
   {
   "ScrollBarTrack",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, szScrollBarTrack, 3
   };
#else
NOTUSED(PropertyScrollBarTrack, "P142");
#endif

#endif // SS_V30

#ifndef SSP_NO_ARROWSEXITEDITMOD
static PROPINFO PropertyArrowsExitEditMode =
   {
   "ArrowsExitEditMode",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyArrowsExitEditMode, "P143");
#endif

#ifndef SSP_NO_MOVEACTIVEONFOCUS
static PROPINFO PropertyMoveActiveOnFocus =
   {
   "MoveActiveOnFocus",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyMoveActiveOnFocus, "P144");
#endif

#ifndef SSP_NO_TYPEPICTPICTURE
static PROPINFO PropertyTypePictPicture =
   {
   "TypePictPicture",
   DT_PICTURE | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypePictPicture, "P145");
#endif

#ifndef SSP_NO_TYPEPICTCENTER
static PROPINFO PropertyTypePictCenter =
   {
   "TypePictCenter",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypePictCenter, "P146");
#endif

#ifndef SSP_NO_TYPEPICTSTRETCH
static PROPINFO PropertyTypePictStretch =
   {
   "TypePictStretch",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypePictStretch, "P147");
#endif

#ifndef SSP_NO_TYPEPICTSCALE
static PROPINFO PropertyTypePictMaintainScale =
   {
   "TypePictMaintainScale",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypePictMaintainScale, "P148");
#endif

#ifndef SSP_NO_RETAINSELBLOCK
static PROPINFO PropertyRetainSelBlock =
   {
   "RetainSelBlock",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyRetainSelBlock, "P149");
#endif

#ifndef SSP_NO_ALLOWUSERFORMULAS
static PROPINFO PropertyAllowUserFormulas =
   {
   "AllowUserFormulas",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyAllowUserFormulas, "P150");
#endif

#ifndef SSP_NO_NOBEEP
static PROPINFO PropertyNoBeep =
   {
   "NoBeep",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyNoBeep, "P151");
#endif

#ifndef SSP_NO_PRINTCOLOR
static PROPINFO PropertyPrintColor =
   {
   "PrintColor",
   DT_BOOL | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrintColor),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyPrintColor, "P152");
#endif

#ifndef SSP_NO_OPERATIONMODE
static CHAR szOperationMode[96];
/*
static CHAR szOperationMode[] =
   {
   "0 - Normal\0"\
   "1 - Read Only\0"\
   "2 - Row Mode\0"\
   "3 - Single Select\0"\
   "4 - Multi Select\0"\
   "5 - Extended Select\0"\
   ""
   };
*/
static PROPINFO PropertyOperationMode =
   {
   "OperationMode",
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   NULL, 0, 0, szOperationMode, 5
   };
#else
NOTUSED(PropertyOperationMode, "P153");
#endif

#ifndef SSP_NO_EDITCHARCASE
static PROPINFO PropertyTypeEditCharCase =
   {
   "TypeEditCharCase",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeEditCharCase),
   0, 0, "", 2
   };
#else
NOTUSED(PropertyTypeEditCharCase, "P154");
#endif

#ifndef SS_V30
#ifndef SSP_NO_DATACONNECT
static PROPINFO PropertyDataConnect =
   {
   "DataConnect",
//   DT_HSZ | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef | PF_fEditable,
   DT_HSZ | PF_fGetMsg | PF_fSetMsg | PF_fNoShow | PF_fLoadDataOnly,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyDataConnect, "P155");
#endif

#else

#ifndef SSP_NO_CLIPBOARDOPTIONS
/*
static CHAR szClipboardOptions[] =
   {
   "0 - Copy/Paste Row Headers\0"\
   "1 - Copy Row Headers\0"\
   "2 - Paste Row Headers\0"\
   "3 - No Row Headers\0"\
   };
*/
static PROPINFO PropertyClipboardOptions =
   {
   "ClipboardOptions",
//   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 15, NULL, 0
   };
#else
NOTUSED(PropertyClipboardOptions, "P155");
#endif

#endif / SS_V30

#ifndef SSP_NO_EDITMODEREPLACE
static PROPINFO PropertyEditModeReplace =
   {
   "EditModeReplace",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyEditModeReplace, "P156");
#endif

#ifndef SSP_NO_CLIPVALUE
static PROPINFO PropertyClipValue =
   {
   "ClipValue",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyClipValue, "P157");
#endif

#ifndef SSP_NO_EDITMULTILINE
static PROPINFO PropertyTypeEditMultiLine =
   {
   "TypeEditMultiLine",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, TypeEditMultiLine),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeEditMultiLine, "P158");
#endif

#ifndef SSP_NO_HWND
static PROPINFO PropertyhWnd =
   {
   "hWnd",
   DT_SHORT | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyhWnd, "P159");
#endif

#ifdef SS_V30

#ifndef SSP_NO_COMBOHWND
static PROPINFO PropertyTypeCombohWnd =
   {
   "TypeComboBoxhWnd",
   DT_LONG | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeCombohWnd, "P160");
#endif

#else // SS_V30

#ifndef SSP_NO_LOADTABFILE
static PROPINFO PropertyLoadTabFile =
   {
   "LoadTabFile",
   DT_HSZ | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyLoadTabFile, "P160");
#endif

#endif // SS_V30

#ifndef SSP_NO_MAXTEXTROWHEIGHT
static PROPINFO PropertyMaxTextRowHeight =
   {
   "MaxTextRowHeight",
   DT_REAL | PF_fPropArray | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyMaxTextRowHeight, "P161");
#endif

#ifndef SSP_NO_MAXTEXTCELLHEIGHT
static PROPINFO PropertyMaxTextCellHeight =
   {
   "MaxTextCellHeight",
   DT_REAL | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyMaxTextCellHeight, "P162");
#endif

#ifndef SSP_NO_MAXTEXTCOLWIDTH
static PROPINFO PropertyMaxTextColWidth =
   {
   "MaxTextColWidth",
   DT_REAL | PF_fPropArray | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyMaxTextColWidth, "P163");
#endif

#ifndef SSP_NO_MAXTEXTCELLWIDTH
static PROPINFO PropertyMaxTextCellWidth =
   {
   "MaxTextCellWidth",
   DT_REAL | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyMaxTextCellWidth, "P164");
#endif

#ifndef SSP_NO_DATACOLCNT
static PROPINFO PropertyDataColCnt =
   {
   "DataColCnt",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyDataColCnt, "P165");
#endif

#ifndef SSP_NO_DATAROWCNT
static PROPINFO PropertyDataRowCnt =
   {
   "DataRowCnt",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyDataRowCnt, "P166");
#endif

#ifndef SSP_NO_AUTOCLIPBOARD
static PROPINFO PropertyAutoClipboard =
   {
   "AutoClipboard",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyAutoClipboard, "P167");
#endif

#ifndef SSP_NO_ALLOWMULTIBLOCKS
static PROPINFO PropertyAllowMultiBlocks =
   {
   "AllowMultiBlocks",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyAllowMultiBlocks, "P168");
#endif

#ifndef SSP_NO_ALLOWCELLOVERFLOW
static PROPINFO PropertyAllowCellOverflow =
   {
   "AllowCellOverflow",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyAllowCellOverflow, "P169");
#endif

#ifndef SSP_NO_ALLOWDRAGDROP
static PROPINFO PropertyAllowDragDrop =
   {
   "AllowDragDrop",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyAllowDragDrop, "P170");
#endif

#ifndef SSP_NO_GRIDLINECOLOR
static PROPINFO PropertyGridLineColor =
   {
   "GridColor",
   DT_COLOR | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0xC0C0C0, NULL, 0
   };
#else
NOTUSED(PropertyGridLineColor, "P171");
#endif

#ifndef SSP_NO_GRIDSOLID
static PROPINFO PropertyGridSolid =
   {
   "GridSolid",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyGridSolid, "P172");
#endif

#ifndef SSP_NO_VSCROLLSPECIAL
static PROPINFO PropertyVScrollSpecial =
   {
   "VScrollSpecial",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyVScrollSpecial, "P173");
#endif

#ifndef SSP_NO_USERRESIZE
static PROPINFO PropertyUserResize =
   {
   "UserResize",
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 3, NULL, 0
   };
#else
NOTUSED(PropertyUserResize, "P174");
#endif

#ifndef SSP_NO_BUTTONDRAWMODE
static PROPINFO PropertyButtonDrawMode =
   {
   "ButtonDrawMode",
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyButtonDrawMode, "P175");
#endif

#ifndef SSP_NO_SELECTBLOCKOPTIONS
static PROPINFO PropertySelectBlockOptions =
   {
   "SelectBlockOptions",
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 15, NULL, 0
   };
#else
NOTUSED(PropertySelectBlockOptions, "P176");
#endif

#ifndef SSP_NO_TYPEFLOATDEFCUR
static PROPINFO PropertyTypeFloatDefCur =
   {
   "FloatDefCurrencyChar",
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeFloatDefCur, "P177");
#endif

#ifndef SSP_NO_TYPEFLOATDEFDECIMAL
static PROPINFO PropertyTypeFloatDefDecimal =
   {
   "FloatDefDecimalChar",
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeFloatDefDecimal, "P178");
#endif

#ifndef SSP_NO_TYPEFLOATDEFSEP
static PROPINFO PropertyTypeFloatDefSep =
   {
   "FloatDefSepChar",
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeFloatDefSep, "P179");
#endif

#ifndef SSP_NO_VIRTUALROWS
static PROPINFO PropertyVirtualRows =
   {
   "VirtualRows",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyVirtualRows, "P180");
#endif

#ifndef SSP_NO_VIRTUALOVERLAP
static PROPINFO PropertyVirtualOverlap =
   {
   "VirtualOverlap",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyVirtualOverlap, "P181");
#endif

#ifndef SSP_NO_VIRTUALMODE
static PROPINFO PropertyVirtualMode =
   {
   "VirtualMode",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyVirtualMode, "P182");
#endif

#ifndef SSP_NO_VIRTUALMAXROWS
static PROPINFO PropertyVirtualMaxRows =
   {
   "VirtualMaxRows",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, -1, NULL, 0
   };
#else
NOTUSED(PropertyVirtualMaxRows, "P183");
#endif

#ifndef SSP_NO_VIRTUALSCROLLBUFFER
static PROPINFO PropertyVirtualScrollBuffer =
   {
   "VirtualScrollBuffer",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyVirtualScrollBuffer, "P184");
#endif

#ifndef SSP_NO_NOBORDER
static PROPINFO PropertyNoBorder =
   {
   "NoBorder",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyNoBorder, "P185");
#endif

#ifndef SSP_NO_GRIDSHOWHORIZ
static PROPINFO PropertyGridShowHoriz =
   {
   "GridShowHoriz",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyGridShowHoriz, "P186");
#endif

#ifndef SSP_NO_GRIDSHOWVERT
static PROPINFO PropertyGridShowVert =
   {
   "GridShowVert",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyGridShowVert, "P187");
#endif

#ifndef SSP_NO_STARTINGCOLNUMBER
static PROPINFO PropertyStartingColNumber =
   {
   "StartingColNumber",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyStartingColNumber, "P188");
#endif

#ifndef SSP_NO_STARTINGROWNUMBER
static PROPINFO PropertyStartingRowNumber =
   {
   "StartingRowNumber",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, NULL, 0
   };
#else
NOTUSED(PropertyStartingRowNumber, "P189");
#endif

#if (!defined(SSP_NO_COLHEADERDISPLAY) || !defined(SSP_NO_ROWHEADERDISPLAY))
static CHAR szHeaderDisplay[52];
/*
static CHAR szHeaderDisplay[] =
   {
   "0 - Blank\0"\
   "1 - Display Numbers\0"\
   "2 - Display Letters\0"\
   ""
   };
*/
#endif

#ifndef SSP_NO_COLHEADERDISPLAY
static PROPINFO PropertyColHeaderDisplay =
   {
   "ColHeaderDisplay",
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 2, szHeaderDisplay, 2
   };
#else
NOTUSED(PropertyColHeaderDisplay, "P190");
#endif

#ifndef SSP_NO_ROWHEADERDISPLAY
static PROPINFO PropertyRowHeaderDisplay =
   {
   "RowHeaderDisplay",
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, 1, szHeaderDisplay, 2
   };
#else
NOTUSED(PropertyRowHeaderDisplay, "P191");
#endif

#ifndef SSP_NO_SORTKEYORDER
/*
static CHAR szSortKeyOrder[] =
   {
   "0 - None\0"\
   "1 - Ascending\0"\
   "2 - Descending\0"\
   ""
   };
*/
static PROPINFO PropertySortKeyOrder =
   {
   "SortKeyOrder",
   DT_ENUM | PF_fPropArray | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, "", 2
   };
#else
NOTUSED(PropertySortKeyOrder, "P192");
#endif

#ifndef SSP_NO_SORTKEY
static PROPINFO PropertySortKey =
   {
   "SortKey",
   SS_DT_ROWCOL | PF_fPropArray | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertySortKey, "P193");
#endif

#ifndef SSP_NO_SORTBY
/*
static CHAR szSortBy[] =
   {
   "0 - Row\0"\
   "1 - Col\0"\
   ""
   };
*/
static PROPINFO PropertySortBy =
   {
   "SortBy",
   DT_ENUM | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, dSortBy),
   0, 0, "", 1
   };
#else
NOTUSED(PropertySortBy, "P194");
#endif

#ifndef SSP_NO_CHANGEMADE
static PROPINFO PropertyChangeMade =
   {
   "ChangeMade",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyChangeMade, "P195");
#endif

#ifndef SSP_NO_VSCROLLSPECIALTYPE
static PROPINFO PropertyVScrollSpecialType =
   {
   "VScrollSpecialType",
   DT_SHORT | PF_fGetMsg | PF_fSetMsg |         // SCP-2/96: removed fNoShow
   PF_fSaveData | PF_fDefVal | PF_fNoInitDef,   // BJO-3/96: added fSaveData, fDefVal, fNoInitDef
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyVScrollSpecialType, "P196");
#endif

#if (!defined(SSP_NO_USERRESIZEROW) || !defined(SSP_NO_USERRESIZECOL))
/*
static CHAR szUserResize[] =
   {
   "0 - Default\0"\
   "1 - On\0"\
   "2 - Off\0"\
   ""
   };
*/
#endif

#ifndef SSP_NO_USERRESIZEROW
static PROPINFO PropertyUserResizeRow =
   {
   "UserResizeRow",
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   NULL, 0, 0, "", 2
   };
#else
NOTUSED(PropertyUserResizeRow, "P197");
#endif

#ifndef SSP_NO_USERRESIZECOL
static PROPINFO PropertyUserResizeCol =
   {
   "UserResizeCol",
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   NULL, 0, 0, "", 2
   };
#else
NOTUSED(PropertyUserResizeCol, "P198");
#endif

#ifndef SSP_NO_ROWHIDDEN
static PROPINFO PropertyRowHidden =
   {
   "RowHidden",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyRowHidden, "P199");
#endif

#ifndef SSP_NO_COLHIDDEN
static PROPINFO PropertyColHidden =
   {
   "ColHidden",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyColHidden, "P200");
#endif

#ifndef SSP_NO_TYPEFLOATCURRENCY
static PROPINFO PropertyTypeFloatCurrency =
   {
   "TypeFloatCurrencyChar",
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeFloatCurrency, "P201");
#endif

#ifndef SSP_NO_TYPEFLOATDECIMAL
static PROPINFO PropertyTypeFloatDecimal =
   {
   "TypeFloatDecimalChar",
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeFloatDecimal, "P202");
#endif

#ifndef SSP_NO_TYPEFLOATSEPCHAR
static PROPINFO PropertyTypeFloatSepChar =
   {
   "TypeFloatSepChar",
   DT_SHORT | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeFloatSepChar, "P203");
#endif

#ifndef SSP_NO_CURSORTYPE
/*
static CHAR szCursorType[] =
   {
   "0 - Default\0"\
   "1 - Column Resize\0"\
   "2 - Row Resize\0"\
   "3 - Button\0"\
   "4 - Gray Area\0"\
   "5 - Locked Cell\0"\
   "6 - Column Header\0"\
   "7 - Row Header\0"\
#ifdef SS_V30
   "8 - Drag Drop Area\0"\
   "9 - Drag Drop\0"\
#endif
   ""
   };
*/
static PROPINFO PropertyCursorType =
   {
   "CursorType",
   DT_ENUM | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, dCursorType),
#ifdef SS_V30
   0, 0, "", 9
#else
   0, 0, "", 7
#endif
   };
#else
NOTUSED(PropertyCursorType, "P204");
#endif

#ifndef SSP_NO_CURSORSTYLE
/*
static CHAR szCursorStyle[] =
   {
   "0 - User Defined\0"\
   "1 - Default\0"\
   "2 - Arrow\0"\
   "3 - Def Col Resize\0"\
   "4 - Def Row Resize\0"\
   ""
   };
*/
static PROPINFO PropertyCursorStyle =
   {
   "CursorStyle",
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   NULL, 0, 0, "", 4
   };
#else
NOTUSED(PropertyCursorStyle, "P205");
#endif

#ifndef SSP_NO_CURSORICON
static PROPINFO PropertyCursorIcon =
   {
   "CursorIcon",
   DT_PICTURE | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyCursorIcon, "P206");
#endif

#ifndef SSP_NO_BORDERTYPE
static PROPINFO PropertyBorderType =
   {
   "CellBorderType",
   DT_SHORT | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, wBorderType),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyBorderType, "P207");
#endif

#ifndef SSP_NO_BORDERSTYLE
/*
static CHAR szBorderStyle[] =
   {
   "0 - Default\0"\
   "1 - Solid\0"\
   "2 - Dash\0"\
   "3 - Dot\0"\
   "4 - Dash Dot\0"\
   "5 - Dash Dot Dot\0"\
   "6 - Blank\0"\
   "7 - Reserved\0"\
   "8 - Reserved\0"\
   "9 - Reserved\0"\
   "10 - Reserved\0"\
   "11 - Fine Solid\0"\
   "12 - Fine Dash\0"\
   "13 - Fine Dot\0"\
   "14 - Fine Dash Dot\0"\
   "15 - Fine Dash Dot Dot\0"\
   ""
   };
*/
static PROPINFO PropertyBorderStyle =
   {
   "CellBorderStyle",
   DT_ENUM | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, wBorderStyle),
   0, 0, "", 15
   };
#else
NOTUSED(PropertyBorderStyle, "P208");
#endif

#ifndef SSP_NO_BORDERCOLOR
static PROPINFO PropertyBorderColor =
   {
   "CellBorderColor",
   DT_COLOR | PF_fSetData | PF_fGetMsg | PF_fNoShow,
   OFFSETIN(VBSPREAD, BorderColor),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyBorderColor, "P209");
#endif

#ifndef SSP_NO_MULTISELINDEX
static PROPINFO PropertyMultiSelIndex =
   {
   "MultiSelIndex",
   DT_LONG | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyMultiSelIndex, "P210");
#endif

#ifndef SSP_NO_MULTISELCOUNT
static PROPINFO PropertyMultiSelCount =
   {
   "MultiSelCount",
   DT_LONG | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyMultiSelCount, "P211");
#endif

#ifndef SSP_NO_DESTCOL
static PROPINFO PropertyDestCol =
   {
   "DestCol",
   DT_LONG | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, ColDest),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyDestCol, "P212");
#endif

#ifndef SSP_NO_DESTROW
static PROPINFO PropertyDestRow =
   {
   "DestRow",
   DT_LONG | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, RowDest),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyDestRow, "P213");
#endif

#ifndef SSP_NO_COLPAGEBREAK
static PROPINFO PropertyColPageBreak =
   {
   "ColPageBreak",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyColPageBreak, "P214");
#endif

#ifndef SSP_NO_ROWPAGEBREAK
static PROPINFO PropertyRowPageBreak =
   {
   "RowPageBreak",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyRowPageBreak, "P215");
#endif

#ifndef SSP_NO_VIRTUALCURROWCOUNT
static PROPINFO PropertyVirtualCurRowCount =
   {
   "VirtualCurRowCount",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyVirtualCurRowCount, "P216");
#endif

#ifndef SSP_NO_VIRTUALCURTOP
static PROPINFO PropertyVirtualCurTop =
   {
   "VirtualCurTop",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyVirtualCurTop, "P217");
#endif

#ifndef SSP_NO_TYPESPIN
RWRT_PROP(TypeSpin, DT_BOOL);
#else
NOTUSED(PropertyTypeSpin, "P218");
#endif

#ifndef SSP_NO_TYPEINTSPINWRAP
RWRT_PROP(TypeIntegerSpinWrap, DT_BOOL);
#else
NOTUSED(PropertyTypeIntegerSpinWrap, "P219");
#endif

#ifndef SSP_NO_TYPEINTSPININC
RWRT_PROP(TypeIntegerSpinInc, DT_LONG);
#else
NOTUSED(PropertyTypeIntegerSpinInc, "P220");
#endif

#ifndef SSP_NO_TYPETEXTPREFIX
RWRT_PROP(TypeTextPrefix, DT_BOOL);
#else
NOTUSED(PropertyTypeTextPrefix, "P221");
#endif

#ifndef SSP_NO_TYPETEXTSHADOWIN
RWRT_PROP(TypeTextShadowIn, DT_BOOL);
#else
NOTUSED(PropertyTypeTextShadowIn, "P222");
#endif

#ifndef SSP_NO_TYPECOMBOEDITABLE
RWRT_PROP(TypeComboBoxEditable, DT_BOOL);
#else
NOTUSED(PropertyTypeComboBoxEditable, "P223");
#endif

#ifndef SSP_NO_OWNERDRAWSTYLE
RWRT_PROP(TypeOwnerDrawStyle, DT_LONG);
#else
NOTUSED(PropertyTypeOwnerDrawStyle, "P224");
#endif

#ifndef SSP_NO_CHECKCENTER
RWRT_PROP(TypeCheckCenter, DT_BOOL);
#else
NOTUSED(PropertyTypeCheckCenter, "P225");
#endif

#ifndef SSP_NO_CHECKTEXT
static PROPINFO PropertyTypeCheckText =
   {
   "TypeCheckText",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeCheckText, "P226");
#endif

#ifndef SSP_NO_CHECKTYPE
static PROPINFO PropertyTypeCheckType =
   {
   "TypeCheckType",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, nCheckType),
   0, 0,
   "", 1
   };
#else
NOTUSED(PropertyTypeCheckType, "P252");
#endif

#ifndef SSP_NO_CHECKPICTURE
static PROPINFO PropertyTypeCheckPicture =
   {
   "TypeCheckPicture",
   DT_PICTURE | PF_fPropArray | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeCheckPicture, "P227");
#endif

#ifndef SSP_NO_ABOUT
static PROPINFO PropertyAbout =
   {
   "(About)",
   DT_HSZ | PF_fGetMsg | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyAbout, "P228");
#endif

#ifndef SSP_NO_CHECKTEXTALIGN
/*
static CHAR szTypeCheckTextAlignTypes[] =
   {
   "0 - Left\0"\
   "1 - Right\0"\
   ""
   };
*/
static PROPINFO PropertyTypeCheckTextAlign =
   {
   "TypeCheckTextAlign",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, "", 1
   };
#else
NOTUSED(PropertyTypeCheckTextAlign, "P229");
#endif

#ifndef SSP_NO_COMBOINDEX
static PROPINFO PropertyTypeComboIndex =
   {
   "TypeComboBoxIndex",
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeComboIndex, "P230");
#endif

#ifndef SSP_NO_COMBOSTRING
static PROPINFO PropertyTypeComboString =
   {
   "TypeComboBoxString",
   DT_HSZ | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeComboString, "P231");
#endif

#ifndef SSP_NO_COMBOCOUNT
static PROPINFO PropertyTypeComboCount =
   {
   "TypeComboBoxCount",
   DT_SHORT | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeComboCount, "P232");
#endif

#ifndef SSP_NO_TYPEHALIGN
static PROPINFO PropertyTypeHAlign =
   {
   "TypeHAlign",
   DT_ENUM | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, "", 2
   };
#else
NOTUSED(PropertyTypeHAlign, "P233");
#endif

#ifndef SSP_NO_PRINTHDCPRINTER
static PROPINFO PropertyPrinthDCPrinter =
   {
   "hDCPrinter",
   DT_SHORT | PF_fGetData | PF_fSetData | PF_fNoShow,
   OFFSETIN(VBSPREAD, Print.PrinthDCPrinter),
   0, 0, NULL, 0
   };
#else
NOTUSED(PropertyPrinthDCPrinter, "P234");
#endif

#ifndef SSP_NO_DAUTOHEADINGS
static PROPINFO PropertyDAutoHeadings =
   {
   "DAutoHeadings",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   OFFSETIN(VBSPREAD, DataOpts), 0x10, 1, NULL, 0
   };
#else
NOTUSED(PropertyDAutoHeadings, "P237");
#endif

#ifndef SSP_NO_DAUTOCELLTYPES
static PROPINFO PropertyDAutoCellTypes =
   {
   "DAutoCellTypes",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   OFFSETIN(VBSPREAD, DataOpts), 0x11, 1, NULL, 0
   };
#else
NOTUSED(PropertyDAutoCellTypes, "P238");
#endif

#ifndef SSP_NO_DAUTOFILL
static PROPINFO PropertyDAutoFill =
   {
   "DAutoFill",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   OFFSETIN(VBSPREAD, DataOpts), 0x12, 1, NULL, 0
   };
#else
NOTUSED(PropertyDAutoFill, "P239");
#endif

#ifndef SSP_NO_DAUTOSIZECOLS
static CHAR szDAutoSizeCols[77];
/*
static CHAR szDAutoSizeCols[] =
   {
   "0 - Off\0"\
   "1 - Max Col Width\0"\
   "2 - Best Guess\0"\
   ""
   };
*/
static PROPINFO PropertyDAutoSizeCols =
   {
   "DAutoSizeCols",
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   OFFSETIN(VBSPREAD, DataOpts), 0x13, 2, szDAutoSizeCols, 2
   };
#else
NOTUSED(PropertyDAutoSizeCols, "P240");
#endif

#ifndef SSP_NO_DATAFIELD
 PROPINFO PropertyDataField =
   {
   "DataField",
   DT_HSZ | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyDataField, "P241");
#endif

#ifndef SSP_NO_DATAFILLEVENT
static PROPINFO PropertyDataFillEvent =
   {
   "DataFillEvent",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyDataFillEvent, "P242");
#endif

#ifndef SSP_NO_DINFORMACTIVEROW
static PROPINFO PropertyDInformActiveRowChange =
   {
   "DInformActiveRowChange",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   OFFSETIN(VBSPREAD, DataOpts), 0x15, 1, NULL, 0
   };
#else
NOTUSED(PropertyDInformActiveRowChange, "P243");
#endif

#ifndef SSP_NO_COMBOCURSEL
static PROPINFO PropertyTypeComboCurSel =
   {
   "TypeComboBoxCurSel",
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyTypeComboCurSel, "P244");
#endif

#ifndef SSP_NO_SELMODEINDEX
static PROPINFO PropertySelModeIndex =
   {
   "SelModeIndex",
   SS_DT_ROWCOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertySelModeIndex, "P245");
#endif

#ifndef SSP_NO_SELMODESELCOUNT
static PROPINFO PropertySelModeSelCount =
   {
   "SelModeSelCount",
   SS_DT_ROWCOL | PF_fGetMsg | PF_fNoShow | PF_fNoRuntimeW,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertySelModeSelCount, "P246");
#endif

#ifndef SSP_NO_SELMODESELECTED
static PROPINFO PropertySelModeSelected =
   {
   "SelModeSelected",
   DT_BOOL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertySelModeSelected, "P247");
#endif

#ifndef SSP_NO_DAUTOSAVE
static PROPINFO PropertyDAutoSave =
   {
   "DAutoSave",
   DT_BOOL | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   OFFSETIN(VBSPREAD, DataOpts), 0x09, 1, NULL, 0
   };
#else
NOTUSED(PropertyDAutoSave, "P249");
#endif

#ifndef SS_V30
#ifndef SSP_NO_DATASELECT
static PROPINFO PropertyDataSelect =
   {
   "DataSelect",
//   DT_HSZ | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef | PF_fEditable,
   DT_HSZ | PF_fGetMsg | PF_fSetMsg | PF_fNoShow | PF_fLoadDataOnly,
   0, 0, 0, NULL, 0
   };
#else
NOTUSED(PropertyDataSelect, "P250");
#endif
#else

#ifndef SSP_NO_SELBACKCOLOR
static PROPINFO PropertySelBackColor =
   {
   "SelBackColor",
   DT_COLOR | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, RGBCOLOR_DEFAULT, NULL, 0
   };
#else
NOTUSED(PropertySelBackColor, "P250");
#endif
#endif // !SS_V30

#ifdef SS_GP
static PROPINFO PropertyTypeEditExpWidth =
   {
   "TypeEditExpWidth",
   DT_REAL | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };

static PROPINFO PropertyTypeEditExpRowCnt =
   {
   "TypeEditExpRowCnt",
   DT_SHORT | PF_fSetMsg | PF_fGetMsg | PF_fNoShow,
   0, 0, 0, NULL, 0
   };
#endif

#ifndef SSP_NO_BACKCOLORSTYLE
static CHAR szBackColorStyle[93];
/*
static CHAR szBackColorStyle[] =
   {
   "0 - Over Grid\0"\
   "1 - Under Grid\0"\
   "2 - Over Horizontal Grid Only\0"\
   "3 - Over Vertical Grid Only\0"\
   ""
   };
*/
static PROPINFO PropertyBackColorStyle =
   {
   "BackColorStyle",
   DT_ENUM | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   NULL, 0, 0, szBackColorStyle, 3
   };
#else
NOTUSED(PropertyBackColorStyle, "P251");
#endif

#ifndef SSP_NO_SELFORECOLOR
static PROPINFO PropertySelForeColor =
   {
   "SelForeColor",
   DT_COLOR | PF_fGetMsg | PF_fSetMsg | PF_fSaveData | PF_fDefVal | PF_fNoInitDef,
   0, 0, RGBCOLOR_DEFAULT, NULL, 0
   };
#else
NOTUSED(PropertySelForeColor, "P253");
#endif

PROPINFO PI_NU235 = {"P235", DT_SHORT | PF_fNoShow};
PROPINFO PI_NU236 = {"P236", DT_SHORT | PF_fNoShow};

#ifdef SS_BOUNDCONTROL
#ifndef SS_QE
PROPINFO PI_NU237 = {"P237", DT_SHORT | PF_fNoShow};
PROPINFO PI_NU238 = {"P238", DT_SHORT | PF_fNoShow};
PROPINFO PI_NU239 = {"P239", DT_SHORT | PF_fNoShow};
PROPINFO PI_NU240 = {"P240", DT_SHORT | PF_fNoShow};
PROPINFO PI_NU241 = {"P241", DT_SHORT | PF_fNoShow};
PROPINFO PI_NU242 = {"P242", DT_SHORT | PF_fNoShow};
#endif
#endif

PROPINFO PI_NU248 = {"P248", DT_SHORT | PF_fNoShow};

PPROPINFO SpreadProperties[IPROP_SPREAD_CNT + 1] =
   {
   PPROPINFO_STD_BORDERSTYLEON,
   PPROPINFO_STD_CTLNAME,
   PPROPINFO_STD_DRAGICON,
   PPROPINFO_STD_DRAGMODE,
   PPROPINFO_STD_ENABLED,
   PPROPINFO_STD_FONTBOLD,
   PPROPINFO_STD_FONTITALIC,
   PPROPINFO_STD_FONTNAME,
   PPROPINFO_STD_FONTSIZE,
   PPROPINFO_STD_FONTSTRIKE,
   PPROPINFO_STD_FONTUNDER,
   PPROPINFO_STD_HEIGHT,
   PPROPINFO_STD_INDEX,
   PPROPINFO_STD_LEFT,
   PPROPINFO_STD_PARENT,
   PPROPINFO_STD_TABINDEX,
   PPROPINFO_STD_TABSTOP,
   PPROPINFO_STD_TAG,
   PPROPINFO_STD_TOP,
   PPROPINFO_STD_VISIBLE,
   PPROPINFO_STD_WIDTH,

   &PropertyAction,
   &PropertyActiveCellCol,
   &PropertyActiveCellRow,
   &PropertyPrintOrientation,
#ifndef SS_V30
   &PropertyAllowSelectBlock,          // Old
#else
   &PropertyAppearance,
#endif
   &PropertyAutoCalc,
   &PropertyBackColor,
#ifndef SS_V30
   &PropertyCalcDependencies,          // Old
#else
   &PropertyTypeComboBoxWidth,
#endif
   &PropertyCellType,
   &PropertyClip,
   &PropertyCol,
   &PropertyCol2,
   &PropertyColWidth,
   &PropertyDisplayColHeaders,
   &PropertyDisplayRowHeaders,
   &PropertyEditMode,
   &PropertyFileNum,
   &PropertyForeColor,
   &PropertyFormula,
#ifndef SS_V30
   &PropertyGridLines,                 // Old
#else
   &PropertyTypeComboBoxMaxDrop,
#endif
   &PropertyLeftCol,
   &PropertyLock,
   &PropertyMaxCols,
   &PropertyMaxRows,
   &PropertyPosition,
   &PropertyProtect,
   &PropertyRedraw,
   &PropertyRestrictCols,
   &PropertyRestrictRows,
   &PropertyRow,
   &PropertyRow2,
   &PropertyRowHeight,
   &PropertySelectBlockCol,
   &PropertySelectBlockCol2,
   &PropertySelectBlockRow,
   &PropertySelectBlockRow2,
   &PropertySelLength,
   &PropertySelStart,
   &PropertySelText,
   &PropertyShadowColor,
   &PropertyShadowDark,
   &PropertyShadowText,
   &PropertyText,
   &PropertyTopRow,

   &PropertyTypeDateCentury,
   &PropertyTypeDateFormat,
   &PropertyTypeDateMax,
   &PropertyTypeDateMin,
   &PropertyTypeDateSeparator,
#ifndef SS_V30
   &PropertyTypeEditCase,              // Old
#else
   &PropertyTypeVAlign,
#endif
   &PropertyTypeEditCharSet,
#ifndef SS_V30
   &PropertyTypeEditJustify,           // Old
#else
   &PropertyTextTip,
#endif
   &PropertyTypeEditLen,
   &PropertyTypeFloatDecimalPlaces,
   &PropertyTypeFloatMax,
   &PropertyTypeFloatMin,
   &PropertyTypeFloatMoney,
   &PropertyTypeFloatSeparator,
   &PropertyTypeIntegerMax,
   &PropertyTypeIntegerMin,
   &PropertyTypePicMask,
#ifndef SS_V30
   &PropertyTypeTextJustifyHoriz,      // Old
#else
   &PropertyTextTipDelay,
#endif
   &PropertyTypeTextJustifyVert,
   &PropertyTypeTextShadow,
   &PropertyTypeTextWordWrap,
   &PropertyTypeTimeMax,
   &PropertyTypeTimeMin,
   &PropertyTypeTimeSeconds,
   &PropertyTypeTimeSeparator,
   &PropertyTypeTime24Hour,
   &PropertyBlockMode,
   &PropertyScrollBars,
   &PropertyIsBlockSelected,
   &PropertySpreadDesigner,
   &PropertyValue,
   &PropertyTypePicDefText,
   &PropertyEditModePermanent,
   &PropertyAutoSize,
   &PropertyVisibleRows,
   &PropertyVisibleCols,
   &PropertyRowsFrozen,
   &PropertyColsFrozen,
   &PropertyScrollBarExtMode,
   &PropertyTypeButtonAlign,
   &PropertyTypeButtonText,
   &PropertyTypeButtonPictureName,
   &PropertyTypeButtonPictDownName,
   &PropertyTypeButtonType,
   &PropertyTypeButtonColor,
   &PropertyTypeButtonBorderColor,
   &PropertyTypeButtonDarkColor,
   &PropertyTypeButtonLightColor,
   &PropertyTypeButtonTextColor,
   &PropertyProcessTab,
   &PropertyTypeButtonShadowSize,
   &PropertyScrollBarShowMax,
   &PropertyScrollBarMaxAlign,
   &PropertyLockBackColor,
   &PropertyLockForeColor,
   &PropertyTypeEditPassword,
   &PropertyTypeComboList,

#ifdef SS_GP
   &PropertyTypeEditExpWidth,
   &PropertyTypeEditExpRowCnt,
#else
   &PropertyPrintJobName,
   &PropertyPrintAbortMsg,
#endif
   &PropertyPrintHeader,
   &PropertyPrintFooter,
   &PropertyPrintBorder,
   &PropertyPrintShadows,
   &PropertyPrintGrid,
   &PropertyPrintColHeaders,
   &PropertyPrintRowHeaders,
   &PropertyPrintUseDataMax,
   &PropertyPrintType,
   &PropertyPrintPageStart,
   &PropertyPrintPageEnd,
   &PropertyPrintMarginLeft,
   &PropertyPrintMarginTop,
   &PropertyPrintMarginRight,
   &PropertyPrintMarginBottom,

   &PropertyUnitType,
   &PropertyEditEnterAction,
   &PropertyGrayAreaBackColor,
#ifndef SS_V30
   &PropertyGrayAreaForeColor,         // Old
#else
   &PropertyScrollBarTrack,
#endif
   &PropertyArrowsExitEditMode,
   &PropertyMoveActiveOnFocus,

   &PropertyTypePictPicture,
   &PropertyTypePictCenter,
   &PropertyTypePictStretch,
   &PropertyTypePictMaintainScale,

///////////////////////////////////////////////
   &PropertyRetainSelBlock,
///////////////////////////////////////////////
   &PropertyAllowUserFormulas,
   &PropertyNoBeep,

   &PropertyPrintColor,
   &PropertyOperationMode,

   &PropertyTypeEditCharCase,

#ifndef SS_V30
   &PropertyDataConnect,               // Old
#else
   &PropertyClipboardOptions,
#endif

   &PropertyEditModeReplace,
   &PropertyClipValue,
   &PropertyTypeEditMultiLine,
   &PropertyhWnd,
#ifndef SS_V30
   &PropertyLoadTabFile,               // Old
#else
   &PropertyTypeCombohWnd,
#endif
   &PropertyMaxTextRowHeight,
   &PropertyMaxTextCellHeight,
   &PropertyMaxTextColWidth,
   &PropertyMaxTextCellWidth,
   &PropertyDataColCnt,
   &PropertyDataRowCnt,

   // New for Spread VBX 2.0

   &PropertyAutoClipboard,
   &PropertyAllowMultiBlocks,
   &PropertyAllowCellOverflow,
   &PropertyAllowDragDrop,
   &PropertyGridLineColor,
   &PropertyGridSolid,
   &PropertyVScrollSpecial,
   &PropertyUserResize,
   &PropertyButtonDrawMode,
   &PropertySelectBlockOptions,
   &PropertyTypeFloatDefCur,
   &PropertyTypeFloatDefDecimal,
   &PropertyTypeFloatDefSep,
   &PropertyVirtualRows,
   &PropertyVirtualOverlap,
   &PropertyVirtualMode,
   &PropertyVirtualMaxRows,
   &PropertyVirtualScrollBuffer,
   &PropertyNoBorder,
   &PropertyGridShowHoriz,
   &PropertyGridShowVert,
   &PropertyStartingColNumber,
   &PropertyStartingRowNumber,
   &PropertyColHeaderDisplay,
   &PropertyRowHeaderDisplay,

   &PropertySortKeyOrder,
   &PropertySortKey,
   &PropertySortBy,
   &PropertyChangeMade,
   &PropertyVScrollSpecialType,
   &PropertyUserResizeRow,
   &PropertyUserResizeCol,
   &PropertyRowHidden,
   &PropertyColHidden,
   &PropertyTypeFloatCurrency,
   &PropertyTypeFloatDecimal,
   &PropertyTypeFloatSepChar,
   &PropertyCursorType,
   &PropertyCursorStyle,
   &PropertyCursorIcon,
   &PropertyBorderType,
   &PropertyBorderStyle,
   &PropertyBorderColor,
   &PropertyMultiSelIndex,
   &PropertyMultiSelCount,
   &PropertyDestCol,
   &PropertyDestRow,
   &PropertyColPageBreak,
   &PropertyRowPageBreak,
   &PropertyVirtualCurRowCount,
   &PropertyVirtualCurTop,
   &PropertyTypeSpin,
   &PropertyTypeIntegerSpinWrap,
   &PropertyTypeIntegerSpinInc,
   &PropertyTypeTextPrefix,
   &PropertyTypeTextShadowIn,
   &PropertyTypeComboBoxEditable,
   &PropertyTypeOwnerDrawStyle,
   &PropertyTypeCheckCenter,
   &PropertyTypeCheckText,
   &PropertyTypeCheckPicture,
   &PropertyAbout,
   &PropertyTypeCheckTextAlign,
   &PropertyTypeComboIndex,
   &PropertyTypeComboString,
   &PropertyTypeComboCount,
   &PropertyTypeHAlign,
   &PropertyPrinthDCPrinter,
   
#ifndef SSP_NO_DATASOURCE
   PPROPINFO_STD_DATASOURCE,
#else
   &PI_NU235,
#endif
#ifndef SSP_NO_DATACHANGED
   PPROPINFO_STD_DATACHANGED,
#else
   &PI_NU236,
#endif
   &PropertyDAutoHeadings,
   &PropertyDAutoCellTypes,
   &PropertyDAutoFill,
   &PropertyDAutoSizeCols,
   &PropertyDataField,
   &PropertyDataFillEvent,
   &PropertyDInformActiveRowChange,

   &PropertyTypeComboCurSel,
   &PropertySelModeIndex,
   &PropertySelModeSelCount,
   &PropertySelModeSelected,
#ifndef SSP_NO_HELPCONTEXTID
   PPROPINFO_STD_HELPCONTEXTID,
#else
   &PI_NU248,
#endif
   &PropertyDAutoSave,
#ifndef SS_V30
   &PropertyDataSelect,                // Old
#else
   &PropertySelBackColor,
#endif
   &PropertyBackColorStyle,
   &PropertyTypeCheckType,
   &PropertySelForeColor,

   NULL
   };

PPROPINFO SpreadProperties1[IPROP_SPREAD_CNT + 1];
PPROPINFO SpreadProperties2[IPROP_SPREAD_CNT + 1];

/*********
* Events
*********/

#ifdef SS_USE16BITCOORDS
#define SS_ET_ROWCOL ET_I2
#else
#define SS_ET_ROWCOL ET_I4
#endif

#ifndef SSE_NO_ADVANCE
WORD ParamTypesAdvance[] = {ET_I2};

static EVENTINFO EventAdvance =
   {
   "Advance",
   1,
   2,
   ParamTypesAdvance,
   "AdvanceNext As Integer"
   };
#endif

#ifndef SSE_NO_BLOCKSELECTED
WORD ParamTypesBlockSelected[] = {SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL};

static EVENTINFO EventBlockSelected =
   {
   "BlockSelected",
   4,
   8,
   ParamTypesBlockSelected,
#ifdef SS_USE16BITCOORDS
   "BlockCol As Integer, BlockRow As Integer, BlockCol2 As Integer, BlockRow2 As Integer"
#else
   "BlockCol As Long, BlockRow As Long, BlockCol2 As Long, BlockRow2 As Long"
#endif
   };
#endif

#ifndef SSE_NO_BUTTONCLICKED
WORD ParamTypesButtonClicked[] = {SS_ET_ROWCOL, SS_ET_ROWCOL, ET_I2};

static EVENTINFO EventButtonClicked =
   {
   "ButtonClicked",
   3,
   6,
   ParamTypesButtonClicked,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer, ButtonDown As Integer"
#else
   "Col As Long, Row As Long, ButtonDown As Integer"
#endif
   };
#endif

#ifndef SSE_NO_CHANGE
WORD ParamTypesChange[] = {SS_ET_ROWCOL, SS_ET_ROWCOL};

static EVENTINFO EventChange =
   {
   "Change",
   2,
   4,
   ParamTypesChange,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer"
#else
   "Col As Long, Row As Long"
#endif
   };
#endif

#ifndef SSE_NO_CLICK
WORD ParamTypesClick[] = {SS_ET_ROWCOL, SS_ET_ROWCOL};

static EVENTINFO EventClick =
   {
   "Click",
   2,
   4,
   ParamTypesClick,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer"
#else
   "Col As Long, Row As Long"
#endif
   };
#endif

#ifndef SSE_NO_DBLCLICK
WORD ParamTypesDblClick[] = {SS_ET_ROWCOL, SS_ET_ROWCOL};

static EVENTINFO EventDblClick =
   {
   "DblClick",
   2,
   4,
   ParamTypesDblClick,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer"
#else
   "Col As Long, Row As Long"
#endif
   };
#endif

#ifndef SSE_NO_EDITERROR
WORD ParamTypesEditError[] = {SS_ET_ROWCOL, SS_ET_ROWCOL, ET_I2};

static EVENTINFO EventEditError =
   {
   "EditError",
   3,
   6,
   ParamTypesEditError,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer, EditError As Integer"
#else
   "Col As Long, Row As Long, EditError As Integer"
#endif
   };
#endif

#ifndef SSE_NO_EDITMODE
WORD ParamTypesEditMode[] = {SS_ET_ROWCOL, SS_ET_ROWCOL, ET_I2, ET_I2};

static EVENTINFO EventEditMode =
   {
   "EditMode",
   4,
   8,
   ParamTypesEditMode,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer, Mode As Integer, ChangeMade As Integer"
#else
   "Col As Long, Row As Long, Mode As Integer, ChangeMade As Integer"
#endif
   };
#endif

#ifndef SSE_NO_LEAVECELL
WORD ParamTypesLeaveCell[] = {SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, ET_I2};

static EVENTINFO EventLeaveCell =
   {
   "LeaveCell",
   5,
   10,
   ParamTypesLeaveCell,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer, NewCol As Integer, NewRow As Integer, Cancel As Integer"
#else
   "Col As Long, Row As Long, NewCol As Long, NewRow As Long, Cancel As Integer"
#endif
   };
#endif

#ifndef SSE_NO_SELCHANGE
WORD ParamTypesSelChange[] = {SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL};

static EVENTINFO EventSelChange =
   {
   "SelChange",
   6,
   12,
   ParamTypesSelChange,
#ifdef SS_USE16BITCOORDS
   "BlockCol As Integer, BlockRow As Integer, BlockCol2 As Integer, BlockRow2 As Integer, CurCol As Integer, CurRow As Integer"
#else
   "BlockCol As Long, BlockRow As Long, BlockCol2 As Long, BlockRow2 As Long, CurCol As Long, CurRow As Long"
#endif
   };
#endif

#ifndef SSE_NO_QUERYADVANCE
WORD ParamTypesQueryAdvance[] = {ET_I2, ET_I2};

static EVENTINFO EventQueryAdvance =
   {
   "QueryAdvance",
   2,
   4,
   ParamTypesQueryAdvance,
   "AdvanceNext As Integer, Cancel As Integer"
   };
#endif

#ifndef SSE_NO_QUERYDATA
WORD ParamTypesQueryData[] = {SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, ET_I2, ET_I2, ET_I2};

static EVENTINFO EventQueryData =
   {
   "QueryData",
   6,
   12,
   ParamTypesQueryData,
#ifdef SS_USE16BITCOORDS
   "Row As Integer, RowsNeeded As Integer, RowsLoaded As Integer, Direction As Integer, AtTop As Integer, AtBottom As Integer"
#else
   "Row As Long, RowsNeeded As Long, RowsLoaded As Long, Direction As Integer, AtTop As Integer, AtBottom As Integer"
#endif
   };
#endif

#ifndef SSE_NO_COLWIDTHCHANGE
WORD ParamColWidthChange[] = {SS_ET_ROWCOL, SS_ET_ROWCOL};

static EVENTINFO EventColWidthChange =
   {
   "ColWidthChange",
   2,
   4,
   ParamColWidthChange,
#ifdef SS_USE16BITCOORDS
   "Col1 As Integer, Col2 As Integer"
#else
   "Col1 As Long, Col2 As Long"
#endif
   };
#endif

#ifndef SSE_NO_ROWHEIGHTCHANGE
WORD ParamRowHeightChange[] = {SS_ET_ROWCOL, SS_ET_ROWCOL};

static EVENTINFO EventRowHeightChange =
   {
   "RowHeightChange",
   2,
   4,
   ParamRowHeightChange,
#ifdef SS_USE16BITCOORDS
   "Row1 As Integer, Row2 As Integer"
#else
   "Row1 As Long, Row2 As Long"
#endif
   };
#endif

#ifndef SSE_NO_ENTERROW
WORD ParamEnterRow[] = {SS_ET_ROWCOL, ET_I2};

static EVENTINFO EventEnterRow =
   {
   "EnterRow",
   2,
   4,
   ParamEnterRow,
#ifdef SS_USE16BITCOORDS
   "Row As Integer, RowIsLast As Integer"
#else
   "Row As Long, RowIsLast As Integer"
#endif
   };
#endif

#ifndef SSE_NO_USERFORMULAENTERED
WORD ParamUserFormulaEntered[] = {SS_ET_ROWCOL, SS_ET_ROWCOL};

static EVENTINFO EventUserFormulaEntered =
   {
   "UserFormulaEntered",
   2,
   4,
   ParamUserFormulaEntered,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer"
#else
   "Col As Long, Row As Long"
#endif
   };
#endif

#ifndef SSE_NO_RIGHTCLICK
WORD ParamRightClick[] = {ET_I2, SS_ET_ROWCOL, SS_ET_ROWCOL, ET_I4, ET_I4};

static EVENTINFO EventRightClick =
   {
   "RightClick",
   5,
   10,
   ParamRightClick,
#ifdef SS_USE16BITCOORDS
   "ClickType As Integer, Col As Integer, Row As Integer, MouseX As Long, MouseY As Long"
#else
   "ClickType As Integer, Col As Long, Row As Long, MouseX As Long, MouseY As Long"
#endif
   };
#endif

#ifndef SSE_NO_LEAVEROW
WORD ParamTypesLeaveRow[] = {SS_ET_ROWCOL, ET_I2, ET_I2, ET_I2, SS_ET_ROWCOL, ET_I2, ET_I2};

static EVENTINFO EventLeaveRow =
   {
   "LeaveRow",
   7,
   14,
   ParamTypesLeaveRow,
#ifdef SS_USE16BITCOORDS
   "Row As Integer, RowWasLast As Integer, RowChanged As Integer, AllCellsHaveData As Integer, NewRow As Integer, NewRowIsLast As Integer, Cancel As Integer"
#else
   "Row As Long, RowWasLast As Integer, RowChanged As Integer, AllCellsHaveData As Integer, NewRow As Long, NewRowIsLast As Integer, Cancel As Integer"
#endif
   };
#endif

#ifndef SSE_NO_DRAGDROPBLOCK
WORD ParamTypesDragDropBlock[] = {SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, ET_I2, ET_I2, ET_I2, ET_I2};

static EVENTINFO EventDragDropBlock =
   {
   "DragDropBlock",
   12,
   24,
   ParamTypesDragDropBlock,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer, Col2 As Integer, Row2 As Integer, NewCol As Integer, NewRow As Integer, NewCol2 As Integer, NewRow2 As Integer, Overwrite As Integer, Action As Integer, DataOnly As Integer, Cancel As Integer"
#else
   "Col As Long, Row As Long, Col2 As Long, Row2 As Long, NewCol As Long, NewRow As Long, NewCol2 As Long, NewRow2 As Long, Overwrite As Integer, Action As Integer, DataOnly As Integer, Cancel As Integer"
#endif
   };
#endif

#ifndef SSE_NO_TOPLEFTCHANGE
WORD ParamTopLeftChange[] = {SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL, SS_ET_ROWCOL};

static EVENTINFO EventTopLeftChange =
   {
   "TopLeftChange",
   4,
   8,
   ParamTopLeftChange,
#ifdef SS_USE16BITCOORDS
   "OldLeft As Integer, OldTop As Integer, NewLeft As Integer, NewTop As Integer"
#else
   "OldLeft As Long, OldTop As Long, NewLeft As Long, NewTop As Long"
#endif
   };
#endif

#ifndef SSE_NO_DRAWITEM
WORD ParamTypesDrawItem[] = {SS_ET_ROWCOL, SS_ET_ROWCOL, ET_I2, ET_I4, ET_I4, ET_I4, ET_I4, ET_I4};

static EVENTINFO EventDrawItem =
   {
   "DrawItem",
   8,
   16,
   ParamTypesDrawItem,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer, hDC As Integer, x As Long, y As Long, x2 As Long, y2 As Long, Style As Long"
#else
   "Col As Long, Row As Long, hDC As Integer, x As Long, y As Long, x2 As Long, y2 As Long, Style As Long"
#endif
   };
#endif

#ifndef SSE_NO_DATACOLCONFIG
WORD ParamTypesI4HLI2[] = {SS_ET_ROWCOL, ET_HLSTR, ET_I2};

static EVENTINFO EventDataColConfig =
   {
   "DataColConfig",
   3,
   6,
   ParamTypesI4HLI2,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, DataField As String, DataType As Integer"
#else
   "Col As Long, DataField As String, DataType As Integer"
#endif
   };
#endif

#ifndef SSE_NO_DATAFILL
WORD ParamTypesI4I4I2I2I2[] = {SS_ET_ROWCOL, SS_ET_ROWCOL, ET_I2, ET_I2, ET_I2};

static EVENTINFO EventDataFill =
   {
   "DataFill",
   5,
   10,
   ParamTypesI4I4I2I2I2,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer, DataType As Integer, fGetData As Integer, Cancel As Integer"
#else
   "Col As Long, Row As Long, DataType As Integer, fGetData As Integer, Cancel As Integer"
#endif
   };
#endif

#ifndef SSE_NO_PRINTABORT
WORD ParamPrintAbort[] = {ET_I2};

static EVENTINFO EventPrintAbort =
   {
   "PrintAbort",
   1,
   2,
   ParamPrintAbort,
   "Abort As Integer"
   };

WORD ParamPrintMsgBox[] = {ET_HLSTR, ET_I2, ET_I2, ET_I2};

static EVENTINFO EventPrintMsgBox =
   {
   "PrintMsgBox",
   4,
   8,
   ParamPrintMsgBox,
   "Text As String, PrintID As Integer, OpenMsgBox As Integer, Processed As Integer"
   };
#endif

#ifndef SSE_NO_CUSTOMFUNCTION
WORD ParamCustomFunction[] = {ET_HLSTR, ET_I2, SS_ET_ROWCOL, SS_ET_ROWCOL, ET_I2};

static EVENTINFO EventCustomFunction =
   {
   "CustomFunction",
   5,
   10,
   ParamCustomFunction,
#ifdef SS_USE16BITCOORDS
   "FunctionName As String, ParameterCnt As Integer, Col As Integer, Row As Integer, Status As Integer"
#else
   "FunctionName As String, ParameterCnt As Integer, Col As Long, Row As Long, Status As Integer"
#endif
   };
#endif

#ifndef SSE_NO_VIRTUALCLEARDATA
WORD ParamTypesVirtualClearData[] = {SS_ET_ROWCOL, SS_ET_ROWCOL};

static EVENTINFO EventVirtualClearData =
   {
   "VirtualClearData",
   2,
   4,
   ParamTypesVirtualClearData,
#ifdef SS_USE16BITCOORDS
   "Row As Integer, RowsBeingCleared As Integer"
#else
   "Row As Long, RowsBeingCleared As Long"
#endif
   };
#endif

#ifndef SSE_NO_DATAADDNEW
static EVENTINFO EventDataAddNew =
   {
   "DataAddNew",
   0,
   0,
   NULL,
   ""
   };
#endif

#ifndef SSE_NO_INSDEL
WORD ParamTypesInsDel[] = {ET_I2, ET_I2, ET_I2, SS_ET_ROWCOL, SS_ET_ROWCOL, ET_I2};

static EVENTINFO EventInsDel =
   {
   "InsDel",
   6,
   12,
   ParamTypesInsDel,
#ifdef SS_USE16BITCOORDS
   "EventType As Integer, Operation As Integer, Direction As Integer, Coord1 As Integer, Coord2 As Integer, Cancel As Integer"
#else
   "EventType As Integer, Operation As Integer, Direction As Integer, Coord1 As Long, Coord2 As Long, Cancel As Integer"
#endif
   };
#endif

#ifndef SSE_NO_ARROWPRESSED
WORD ParamTypesArrowPressed[] = {ET_I2};

static EVENTINFO EventArrowPressed =
   {
   "ArrowPressed",
   1,
   2,
   ParamTypesArrowPressed,
   "Arrow As Integer"
   };
#endif

#ifdef SS_V30
#ifndef SSE_NO_COMBOCLOSEUP
WORD ParamComboCloseUp[] = {SS_ET_ROWCOL, SS_ET_ROWCOL, ET_I2};

static EVENTINFO EventComboCloseUp =
   {
   "ComboCloseUp",
   3,
   6,
   ParamComboCloseUp,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer, SelChange As Integer"
#else
   "Col As Long, Row As Long, SelChange As Integer"
#endif
   };
#endif

#ifndef SSE_NO_COMBODROPDOWN
WORD ParamComboDropDown[] = {SS_ET_ROWCOL, SS_ET_ROWCOL};

static EVENTINFO EventComboDropDown =
   {
   "ComboDropDown",
   2,
   4,
   ParamComboDropDown,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer"
#else
   "Col As Long, Row As Long"
#endif
   };
#endif

#ifndef SSE_NO_COMBOSELCHANGE
WORD ParamComboSelChange[] = {SS_ET_ROWCOL, SS_ET_ROWCOL};

static EVENTINFO EventComboSelChange =
   {
   "ComboSelChange",
   2,
   4,
   ParamComboSelChange,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer"
#else
   "Col As Long, Row As Long"
#endif
   };
#endif

#ifndef SSE_NO_TEXTTIPFETCH
WORD ParamTextTipFetch[] = {SS_ET_ROWCOL, SS_ET_ROWCOL, ET_I2, ET_I4, ET_HLSTR, ET_I2};

static EVENTINFO EventTextTipFetch =
   {
   "TextTipFetch",
   6,
   12,
   ParamTextTipFetch,
   "Col As Long, Row As Long, MultiLine As Integer, TipWidth As Long, TipText As String, ShowTip As Integer"
   };
#endif

#ifndef SSE_NO_EDITCHANGE
WORD ParamTypesEditChange[] = {SS_ET_ROWCOL, SS_ET_ROWCOL};

static EVENTINFO EventEditChange =
   {
   "EditChange",
   2,
   4,
   ParamTypesEditChange,
#ifdef SS_USE16BITCOORDS
   "Col As Integer, Row As Integer"
#else
   "Col As Long, Row As Long"
#endif
   };
#endif
#endif // SS_V30

PEVENTINFO SpreadEventsVB1[IEVENTVB1_SPREAD_CNT + 1] =
   {
#ifndef SSE_NO_ADVANCE
   &EventAdvance,
#endif
#ifndef SSE_NO_ARROWPRESSED
   &EventArrowPressed,
#endif
#ifndef SSE_NO_BLOCKSELECTED
   &EventBlockSelected,
#endif
#ifndef SSE_NO_BUTTONCLICKED
   &EventButtonClicked,
#endif
#ifndef SSE_NO_CHANGE
   &EventChange,
#endif
#ifndef SSE_NO_CLICK
   &EventClick,
#endif
#ifndef SSE_NO_COLWIDTHCHANGE
   &EventColWidthChange,
#endif
#ifdef SS_V30
#ifndef SSE_NO_COMBOCLOSEUP
   &EventComboCloseUp,
#endif
#ifndef SSE_NO_COMBODROPDOWN
   &EventComboDropDown,
#endif
#ifndef SSE_NO_COMBOSELCHANGE
   &EventComboSelChange,
#endif
#endif
#ifndef SSE_NO_CUSTOMFUNCTION
   &EventCustomFunction,
#endif
#ifndef SSE_NO_DATAADDNEW
   &EventDataAddNew,
#endif
#ifndef SSE_NO_DATACOLCONFIG
   &EventDataColConfig,
#endif
#ifndef SSE_NO_DATAFILL
   &EventDataFill,
#endif
#ifndef SSE_NO_DBLCLICK
   &EventDblClick,
#endif
#ifndef SSE_NO_DRAGDROP
   PEVENTINFO_STD_DRAGDROP,
#endif
#ifndef SSE_NO_DRAGDROPBLOCK
   &EventDragDropBlock,
#endif
#ifndef SSE_NO_DRAGOVER
   PEVENTINFO_STD_DRAGOVER,
#endif
#ifndef SSE_NO_DRAWITEM
   &EventDrawItem,
#endif
#ifdef SS_V30
#ifndef SSE_NO_EDITCHANGE
   &EventEditChange,
#endif
#endif
#ifndef SSE_NO_EDITERROR
   &EventEditError,
#endif
#ifndef SSE_NO_EDITMODE
   &EventEditMode,
#endif
#ifndef SSE_NO_ENTERROW
   &EventEnterRow,
#endif
#ifndef SSE_NO_GOTFOCUS
   PEVENTINFO_STD_GOTFOCUS,
#endif
#ifndef SSE_NO_INSDEL
   &EventInsDel,
#endif
#ifndef SSE_NO_KEYDOWN
   PEVENTINFO_STD_KEYDOWN,
#endif
#ifndef SSE_NO_KEYPRESS
   PEVENTINFO_STD_KEYPRESS,
#endif
#ifndef SSE_NO_KEYUP
   PEVENTINFO_STD_KEYUP,
#endif
#ifndef SSE_NO_LEAVECELL
   &EventLeaveCell,
#endif
#ifndef SSE_NO_LEAVEROW
   &EventLeaveRow,
#endif
#ifndef SSE_NO_LOSTFOCUS
   PEVENTINFO_STD_LOSTFOCUS,
#endif
#ifndef SSE_NO_MOUSEDOWN
   PEVENTINFO_STD_MOUSEDOWN,
#endif
#ifndef SSE_NO_MOUSEMOVE
   PEVENTINFO_STD_MOUSEMOVE,
#endif
#ifndef SSE_NO_MOUSEUP
   PEVENTINFO_STD_MOUSEUP,
#endif
#ifndef SSE_NO_PRINTABORT
   &EventPrintAbort,
#endif
#ifndef SSE_NO_PRINTMSGBOX
   &EventPrintMsgBox,
#endif
#ifndef SSE_NO_QUERYADVANCE
   &EventQueryAdvance,
#endif
#ifndef SSE_NO_QUERYDATA
   &EventQueryData,
#endif
#ifndef SSE_NO_RIGHTCLICK
   &EventRightClick,
#endif
#ifndef SSE_NO_ROWHEIGHTCHANGE
   &EventRowHeightChange,
#endif
#ifndef SSE_NO_SELCHANGE
   &EventSelChange,
#endif
#ifdef SS_V30
#ifndef SSE_NO_TEXTTIPFETCH
   &EventTextTipFetch,
#endif
#endif
#ifndef SSE_NO_TOPLEFTCHANGE
   &EventTopLeftChange,
#endif
#ifndef SSE_NO_USERFORMULAENTERED
   &EventUserFormulaEntered,
#endif
#ifndef SSE_NO_VIRTUALCLEARDATA
   &EventVirtualClearData,
#endif
   NULL
   };

static PEVENTINFO SpreadEvents[IEVENT_SPREAD_CNT + 1] =
   {
#ifndef SSE_NO_ADVANCE
   &EventAdvance,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_BLOCKSELECTED
   &EventBlockSelected,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_BUTTONCLICKED
   &EventButtonClicked,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_CHANGE
   &EventChange,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_CLICK
   &EventClick,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_DBLCLICK
   &EventDblClick,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_DRAGDROP
   PEVENTINFO_STD_DRAGDROP,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_DRAGOVER
   PEVENTINFO_STD_DRAGOVER,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_EDITERROR
   &EventEditError,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_EDITMODE
   &EventEditMode,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_GOTFOCUS
   PEVENTINFO_STD_GOTFOCUS,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_KEYDOWN
   PEVENTINFO_STD_KEYDOWN,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_KEYPRESS
   PEVENTINFO_STD_KEYPRESS,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_KEYUP
   PEVENTINFO_STD_KEYUP,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_LEAVECELL
   &EventLeaveCell,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_LOSTFOCUS
   PEVENTINFO_STD_LOSTFOCUS,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_MOUSEDOWN
   PEVENTINFO_STD_MOUSEDOWN,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_MOUSEMOVE
   PEVENTINFO_STD_MOUSEMOVE,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_MOUSEUP
   PEVENTINFO_STD_MOUSEUP,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_SELCHANGE
   &EventSelChange,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_QUERYADVANCE
   &EventQueryAdvance,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_QUERYDATA
   &EventQueryData,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_COLWIDTHCHANGE
   &EventColWidthChange,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_ROWHEIGHTCHANGE
   &EventRowHeightChange,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_ENTERROW
   &EventEnterRow,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_USERFORMULAENTERED
   &EventUserFormulaEntered,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_RIGHTCLICK
   &EventRightClick,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_LEAVEROW
   &EventLeaveRow,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_DRAGDROPBLOCK
   &EventDragDropBlock,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_TOPLEFTCHANGE
   &EventTopLeftChange,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_DRAWITEM
   &EventDrawItem,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_DATACOLCONFIG
   &EventDataColConfig,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_DATAFILL
   &EventDataFill,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_PRINTABORT
   &EventPrintAbort,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_CUSTOMFUNCTION
   &EventCustomFunction,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_VIRTUALCLEARDATA
   &EventVirtualClearData,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_DATAADDNEW
   &EventDataAddNew,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_INSDEL
   &EventInsDel,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_PRINTMSGBOX
   &EventPrintMsgBox,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_ARROWPRESSED
   &EventArrowPressed,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifdef SS_V30
#ifndef SSE_NO_COMBOCLOSEUP
   &EventComboCloseUp,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_COMBODROPDOWN
   &EventComboDropDown,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_COMBOSELCHANGE
   &EventComboSelChange,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_TEXTTIPFETCH
   &EventTextTipFetch,
#else
   PEVENTINFO_STD_NONE,
#endif
#ifndef SSE_NO_EDITCHANGE
   &EventEditChange,
#else
   PEVENTINFO_STD_NONE,
#endif
#endif // SS_V30
   NULL
   };


//-----------------------------------------------------------------------
// This function loads enumerated property strings from the
// resource file. (Masanori Iwasa)
//-----------------------------------------------------------------------
void LoadEnumStrings(HCTL hCtl)
{
    PSTR        szTmpPtr;
    MODEL       FAR *lpModel;
    PPROPINFO   FAR *pPropInfo;
    int         i = 0, j;
    int         iProp[] =
                {
#ifndef SSP_NO_COLHEADERDISPLAY
                IPROP_SPREAD_COLHEADERDISPLAY, IDS_HEADERDISP_BLANK,
#endif
#ifndef SSP_NO_ROWHEADERDISPLAY
                IPROP_SPREAD_ROWHEADERDISPLAY, IDS_HEADERDISP_BLANK,
#endif
#ifndef SSP_NO_DAUTOSIZECOLS
                IPROP_SPREAD_DAUTOSIZECOLS,    IDS_DAUTOSIZE_OFF,
#endif
#ifndef SSP_NO_EDITENTERACTION
                IPROP_SPREAD_EDITENTERACTION,  IDS_EDITENTER_NONE,
#endif
#ifndef SSP_NO_OPERATIONMODE
                IPROP_SPREAD_OPERATIONMODE,    IDS_OPERATION_NORMAL,
#endif
#ifndef SSP_NO_POSITION
                IPROP_SPREAD_POSITION,         IDS_POSITION_UL,
#endif
#ifndef SSP_NO_SCROLLBARS
                IPROP_SPREAD_SCROLLBARS,       IDS_SCROLLBARS_NONE,
#endif
#ifndef SSP_NO_UNITTYPE
                IPROP_SPREAD_UNITTYPE,         IDS_UNITTYPE_NORMAL,
#endif
#ifndef SSP_NO_BACKCOLORSTYLE
                IPROP_SPREAD_BACKCOLORSTYLE,   IDS_BACKCOLORSTYLE_OVERGRID,
#endif
                0                
                };

    lpModel = VBGetControlModel(hCtl);

    while (iProp[i*2])
        {
        //- Get first property position.
        pPropInfo = (LPVOID)MAKELONG(lpModel->npproplist, (_segment)lpModel);
        pPropInfo = pPropInfo + iProp[i*2];
        
        if ( ((*((*pPropInfo)))).npszEnumList)
           {
           //- Pointer to static memory allocated from before.
           szTmpPtr = ((*((*pPropInfo)))).npszEnumList;
           szTmpPtr[0] = 0;

           //- Load resource string directly into static memory space.
           for(j = iProp[i*2+1]; j < iProp[i*2+1] + ((*((*pPropInfo)))).enumMax + 1; j++)
               {
               szTmpPtr = szTmpPtr + lstrlen(szTmpPtr) + ((j == iProp[i*2+1]) ? 0 : 1);
               LoadString(hDynamicInst, j + LANGUAGE_BASE, szTmpPtr, 48);
               }
           //- Two terminator in a row means end of list.
           szTmpPtr[lstrlen(szTmpPtr) + 1] = 0;
           }

        i++;
        }
}
