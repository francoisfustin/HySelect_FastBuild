/*********************************************************
* VBSPREA6.C
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
#include <math.h>
#include <string.h>
#include "vbspread.h"
#include "vbsprea4.h"

extern LONG (*CallBack[IPROP_SPREAD_CNT])();


/**********************************************************************************/
void InitFunctionArray()
{
static fInitialized = FALSE;

if (fInitialized)
   return;

fInitialized = TRUE;

#ifndef SSP_NO_FONTBOLD
   CallBack[IPROP_SPREAD_FONTBOLD]             = FP_Font;
#endif
#ifndef SSP_NO_FONTITALIC
   CallBack[IPROP_SPREAD_FONTITALIC]           = FP_Font;
#endif
#ifndef SSP_NO_FONTNAME
   CallBack[IPROP_SPREAD_FONTNAME]             = FP_Font;
#endif
#ifndef SSP_NO_FONTSIZE
   CallBack[IPROP_SPREAD_FONTSIZE]             = FP_Font;
#endif
#ifndef SSP_NO_FONTSTRIKE
   CallBack[IPROP_SPREAD_FONTSTRIKE]           = FP_Font;
#endif
#ifndef SSP_NO_FONTUNDER
   CallBack[IPROP_SPREAD_FONTUNDER]            = FP_Font;
#endif
#ifndef SSP_NO_ACTION
   CallBack[IPROP_SPREAD_ACTION]               = FP_Action;
#endif
#ifndef SSP_NO_ACTIVECELLCOL
   CallBack[IPROP_SPREAD_ACTIVECELLCOL]        = FP_ActiveCol;
#endif
#ifndef SSP_NO_ACTIVECELLROW
   CallBack[IPROP_SPREAD_ACTIVECELLROW]        = FP_ActiveRow;
#endif
#ifndef SSP_NO_PRINTORIENTATION
   CallBack[IPROP_SPREAD_PRINTORIENTATION]     = FP_Print;
#endif
#ifndef SS_V30
#ifndef SSP_NO_ALLOWSELECTBLOCK
   CallBack[IPROP_SPREAD_ALLOWSELECTBLOCK]     = FP_Bool;
#endif
#else
#ifndef SSP_NO_APPEARANCE
   CallBack[IPROP_SPREAD_APPEARANCE]           = FP_Appearance;
#endif
#endif
#ifndef SSP_NO_AUTOCALC
   CallBack[IPROP_SPREAD_AUTOCALC]             = FP_Bool;
#endif
#ifndef SSP_NO_BACKCOLOR
   CallBack[IPROP_SPREAD_BACKCOLOR]            = FP_BackColor;
#endif
#ifndef SS_V30
#ifndef SSP_NO_CALCDEPENDENCIES
   CallBack[IPROP_SPREAD_CALCDEPENDENCIES]     = FP_Calc_Dept;
#endif
#else
#ifndef SSP_NO_TYPECOMBOBOXWIDTH
   CallBack[IPROP_SPREAD_TYPECOMBOWIDTH]       = FP_CellType2;
#endif
#endif
#ifndef SSP_NO_CELLTYPE
   CallBack[IPROP_SPREAD_CELLTYPE]             = FP_CellType1;
#endif
#ifndef SSP_NO_CLIP
   CallBack[IPROP_SPREAD_CLIP]                 = FP_Clip;
#endif
#ifndef SSP_NO_COL
   CallBack[IPROP_SPREAD_COL]                  = FP_Col;
#endif
#ifndef SSP_NO_COL2
   CallBack[IPROP_SPREAD_COL2]                 = FP_Col2;
#endif
#ifndef SSP_NO_COLWIDTH
   CallBack[IPROP_SPREAD_COLWIDTH]             = FP_ColWidth;
#endif
#ifndef SSP_NO_DISPLAYCOLHDRS
   CallBack[IPROP_SPREAD_DISPLAYCOLHDRS]       = FP_Bool;
#endif
#ifndef SSP_NO_DISPLAYROWHDRS
   CallBack[IPROP_SPREAD_DISPLAYROWHDRS]       = FP_Bool;
#endif
#ifndef SSP_NO_EDITMODE
   CallBack[IPROP_SPREAD_EDITMODE]             = FP_EditMode;
#endif
#ifndef SSP_NO_FORECOLOR
   CallBack[IPROP_SPREAD_FORECOLOR]            = FP_ForeColor;
#endif
#ifndef SSP_NO_FORMULA
   CallBack[IPROP_SPREAD_FORMULA]              = FP_Formula;
#endif
#ifndef SS_V30
#ifndef SSP_NO_GRIDLINES
   CallBack[IPROP_SPREAD_GRIDLINES]            = FP_Bool;
#endif
#else
#ifndef SSP_NO_TYPECOMBOBOXMAXDROP
   CallBack[IPROP_SPREAD_TYPECOMBOMAXDROP]     = FP_CellType2;
#endif
#endif
#ifndef SSP_NO_LEFTCOL
   CallBack[IPROP_SPREAD_LEFTCOL]              = FP_LeftCol;
#endif
#ifndef SSP_NO_LOCK
   CallBack[IPROP_SPREAD_LOCK]                 = FP_Lock;
#endif
#ifndef SSP_NO_MAXCOLS
   CallBack[IPROP_SPREAD_MAXCOLS]              = FP_MaxCols;
#endif
#ifndef SSP_NO_MAXROWS
   CallBack[IPROP_SPREAD_MAXROWS]              = FP_MaxRows;
#endif
#ifndef SSP_NO_PROTECT
   CallBack[IPROP_SPREAD_PROTECT]              = FP_Bool;
#endif
#ifndef SSP_NO_REDRAW
   CallBack[IPROP_SPREAD_REDRAW]               = FP_Bool;
#endif
#ifndef SSP_NO_RESTRICTCOLS
   CallBack[IPROP_SPREAD_RESTRICTCOLS]         = FP_Bool;
#endif
#ifndef SSP_NO_RESTRICTROWS
   CallBack[IPROP_SPREAD_RESTRICTROWS]         = FP_Bool;
#endif
#ifndef SSP_NO_ROW
   CallBack[IPROP_SPREAD_ROW]                  = FP_Row;
#endif
#ifndef SSP_NO_ROW2
   CallBack[IPROP_SPREAD_ROW2]                 = FP_Row2;
#endif
#ifndef SSP_NO_ROWHEIGHT
   CallBack[IPROP_SPREAD_ROWHEIGHT]            = FP_RowHeight;
#endif
#ifndef SSP_NO_SELECTBLOCKCOL
   CallBack[IPROP_SPREAD_SELECTBLOCKCOL]       = FP_SelectBlockCol;
#endif
#ifndef SSP_NO_SELECTBLOCKCOL2
   CallBack[IPROP_SPREAD_SELECTBLOCKCOL2]      = FP_SelectBlockCol2;
#endif
#ifndef SSP_NO_SELECTBLOCKROW
   CallBack[IPROP_SPREAD_SELECTBLOCKROW]       = FP_SelectBlockRow;
#endif
#ifndef SSP_NO_SELECTBLOCKROW2
   CallBack[IPROP_SPREAD_SELECTBLOCKROW2]      = FP_SelectBlockRow2;
#endif
#ifndef SSP_NO_SELLENGTH
   CallBack[IPROP_SPREAD_SELLENGTH]            = FP_SelLength;
#endif
#ifndef SSP_NO_SELSTART
   CallBack[IPROP_SPREAD_SELSTART]             = FP_SelStart;
#endif
#ifndef SSP_NO_SELTEXT
   CallBack[IPROP_SPREAD_SELTEXT]              = FP_SelText;
#endif
#ifndef SSP_NO_SHADOWCOLOR
   CallBack[IPROP_SPREAD_SHADOWCOLOR]          = FP_Shadow;
#endif
#ifndef SSP_NO_SHADOWDARK
   CallBack[IPROP_SPREAD_SHADOWDARK]           = FP_Shadow;
#endif
#ifndef SSP_NO_SHADOWTEXT
   CallBack[IPROP_SPREAD_SHADOWTEXT]           = FP_Shadow;
#endif
#ifndef SSP_NO_TEXT
   CallBack[IPROP_SPREAD_TEXT]                 = FP_Text;
#endif
#ifndef SSP_NO_TOPROW
   CallBack[IPROP_SPREAD_TOPROW]               = FP_TopRow;
#endif
#ifndef SSP_NO_DATECENTURY
   CallBack[IPROP_SPREAD_DATECENTURY]          = FP_CellType2;
#endif
#ifndef SSP_NO_DATEFORMAT
   CallBack[IPROP_SPREAD_DATEFORMAT]           = FP_CellType2;
#endif
#ifndef SSP_NO_DATEMAX
   CallBack[IPROP_SPREAD_DATEMAX]              = FP_CellType2;
#endif
#ifndef SSP_NO_DATEMIN
   CallBack[IPROP_SPREAD_DATEMIN]              = FP_CellType2;
#endif
#ifndef SSP_NO_DATESEPARATOR
   CallBack[IPROP_SPREAD_DATESEPARATOR]        = FP_CellType2;
#endif
#ifndef SS_V30
#ifndef SSP_NO_EDITCASE
   CallBack[IPROP_SPREAD_EDITCASE]             = FP_CellType2;
#endif
#else
#ifndef SSP_NO_TYPEVALIGN
   CallBack[IPROP_SPREAD_TYPEVALIGN]           = FP_CellType2;
#endif
#endif
#ifndef SSP_NO_EDITCHARSET
   CallBack[IPROP_SPREAD_EDITCHARSET]          = FP_CellType2;
#endif
#ifdef SS_V30
#ifndef SSP_NO_TEXTTIP
   CallBack[IPROP_SPREAD_TEXTTIP]              = FP_TextTip;
#endif
#else
#ifndef SSP_NO_EDITJUSTIFY
   CallBack[IPROP_SPREAD_EDITJUSTIFY]          = FP_CellType2;
#endif
#endif
#ifndef SSP_NO_EDITLEN
   CallBack[IPROP_SPREAD_EDITLEN]              = FP_CellType2;
#endif
#ifndef SSP_NO_FLOATDECPLACES
   CallBack[IPROP_SPREAD_FLOATDECPLACES]       = FP_CellType2;
#endif
#ifndef SSP_NO_FLOATMAX
   CallBack[IPROP_SPREAD_FLOATMAX]             = FP_CellType2;
#endif
#ifndef SSP_NO_FLOATMIN
   CallBack[IPROP_SPREAD_FLOATMIN]             = FP_CellType2;
#endif
#ifndef SSP_NO_FLOATMONEY
   CallBack[IPROP_SPREAD_FLOATMONEY]           = FP_CellType2;
#endif
#ifndef SSP_NO_FLOATSEPARATOR
   CallBack[IPROP_SPREAD_FLOATSEPARATOR]       = FP_CellType2;
#endif
#ifndef SSP_NO_INTEGERMAX
   CallBack[IPROP_SPREAD_INTEGERMAX]           = FP_CellType2;
#endif
#ifndef SSP_NO_INTEGERMIN
   CallBack[IPROP_SPREAD_INTEGERMIN]           = FP_CellType2;
#endif
#ifndef SSP_NO_PICMASK
   CallBack[IPROP_SPREAD_PICMASK]              = FP_PicMask;
#endif
#ifdef SS_V30
#ifndef SSP_NO_TEXTTIP
   CallBack[IPROP_SPREAD_TEXTTIPDELAY]         = FP_TextTipDelay;
#endif
#else
#ifndef SSP_NO_TEXTJUSTIFYHORIZ
   CallBack[IPROP_SPREAD_TEXTJUSTIFYHORIZ]     = FP_CellType2;
#endif
#endif
#ifndef SSP_NO_TEXTJUSTIFYVERT
   CallBack[IPROP_SPREAD_TEXTJUSTIFYVERT]      = FP_CellType2;
#endif
#ifndef SSP_NO_TEXTSHADOW
   CallBack[IPROP_SPREAD_TEXTSHADOW]           = FP_CellType2;
#endif
#ifndef SSP_NO_TEXTWORDWRAP
   CallBack[IPROP_SPREAD_TEXTWORDWRAP]         = FP_CellType2;
#endif
#ifndef SSP_NO_TIMEMAX
   CallBack[IPROP_SPREAD_TIMEMAX]              = FP_TimeMax;
#endif
#ifndef SSP_NO_TIMEMIN
   CallBack[IPROP_SPREAD_TIMEMIN]              = FP_TimeMin;
#endif
#ifndef SSP_NO_TIMESECONDS
   CallBack[IPROP_SPREAD_TIMESECONDS]          = FP_CellType2;
#endif
#ifndef SSP_NO_TIMESEPARATOR
   CallBack[IPROP_SPREAD_TIMESEPARATOR]        = FP_CellType2;
#endif
#ifndef SSP_NO_TIME24HOUR
   CallBack[IPROP_SPREAD_TIME24HOUR]           = FP_CellType2;
#endif
#ifndef SSP_NO_SCROLLBARS
   CallBack[IPROP_SPREAD_SCROLLBARS]           = FP_ScrollBars;
#endif
#ifndef SSP_NO_ISBLOCKSELECTED
   CallBack[IPROP_SPREAD_ISBLOCKSELECTED]      = FP_IsBlockSelected;
#endif
#ifndef SSP_NO_VALUE
   CallBack[IPROP_SPREAD_VALUE]                = FP_Value;
#endif
#ifndef SSP_NO_PICDEFTEXT
   CallBack[IPROP_SPREAD_PICDEFTEXT]           = FP_PicDefText;
#endif
#ifndef SSP_NO_EDITMODEPERMANENT
   CallBack[IPROP_SPREAD_EDITMODEPERMANENT]    = FP_Bool;
#endif
#ifndef SSP_NO_AUTOSIZE
   CallBack[IPROP_SPREAD_AUTOSIZE]             = FP_Bool;
#endif
#ifndef SSP_NO_VISIBLEROWS
   CallBack[IPROP_SPREAD_VISIBLEROWS]          = FP_VisibleRows;
#endif
#ifndef SSP_NO_VISIBLECOLS
   CallBack[IPROP_SPREAD_VISIBLECOLS]          = FP_VisibleCols;
#endif
#ifndef SSP_NO_ROWSFROZEN
   CallBack[IPROP_SPREAD_ROWSFROZEN]           = FP_RowsFrozen;
#endif
#ifndef SSP_NO_COLSFROZEN
   CallBack[IPROP_SPREAD_COLSFROZEN]           = FP_ColsFrozen;
#endif
#ifndef SSP_NO_SCROLLBAREXTMODE
   CallBack[IPROP_SPREAD_SCROLLBAREXTMODE]     = FP_Bool;
#endif
#ifndef SSP_NO_BUTTONALIGN
   CallBack[IPROP_SPREAD_BUTTONALIGN]          = FP_CellType2;
#endif
#ifndef SSP_NO_BUTTONTEXT
   CallBack[IPROP_SPREAD_BUTTONTEXT]           = FP_ButtonText;
#endif
#ifndef SSP_NO_BUTTONPICTNAME
   CallBack[IPROP_SPREAD_BUTTONPICTNAME]       = FP_ButtonPicName;
#endif
#ifndef SSP_NO_BUTTONPICTDOWN
   CallBack[IPROP_SPREAD_BUTTONPICTDOWN]       = FP_ButtonPicDown;
#endif
#ifndef SSP_NO_BUTTONTYPE
   CallBack[IPROP_SPREAD_BUTTONTYPE]           = FP_CellType2;
#endif
#ifndef SSP_NO_BUTTONCOLOR
   CallBack[IPROP_SPREAD_BUTTONCOLOR]          = FP_CellType2;
#endif
#ifndef SSP_NO_BUTTONBORDERCOLOR
   CallBack[IPROP_SPREAD_BUTTONBORDERCOLOR]    = FP_CellType2;
#endif
#ifndef SSP_NO_BUTTONDARKCOLOR
   CallBack[IPROP_SPREAD_BUTTONDARKCOLOR]      = FP_CellType2;
#endif
#ifndef SSP_NO_BUTTONLIGHTCOLOR
   CallBack[IPROP_SPREAD_BUTTONLIGHTCOLOR]     = FP_CellType2;
#endif
#ifndef SSP_NO_BUTTONTEXTCOLOR
   CallBack[IPROP_SPREAD_BUTTONTEXTCOLOR]      = FP_CellType2;
#endif
#ifndef SSP_NO_PROCESSTAB
   CallBack[IPROP_SPREAD_PROCESSTAB]           = FP_Bool;
#endif
#ifndef SSP_NO_BUTTONSHADOWSIZE
   CallBack[IPROP_SPREAD_BUTTONSHADOWSIZE]     = FP_CellType2;
#endif
#ifndef SSP_NO_SCROLLBARSHOWMAX
   CallBack[IPROP_SPREAD_SCROLLBARSHOWMAX]     = FP_Bool;
#endif
#ifndef SSP_NO_SCROLLBARMAXALIGN
   CallBack[IPROP_SPREAD_SCROLLBARMAXALIGN]    = FP_Bool;
#endif
#ifndef SSP_NO_LOCKBACKCOLOR
   CallBack[IPROP_SPREAD_LOCKBACKCOLOR]        = FP_LockBackColor;
#endif
#ifndef SSP_NO_LOCKFORECOLOR
   CallBack[IPROP_SPREAD_LOCKFORECOLOR]        = FP_LockForeColor;
#endif
#ifndef SSP_NO_EDITPASSWORD
   CallBack[IPROP_SPREAD_EDITPASSWORD]         = FP_CellType2;
#endif
#ifndef SSP_NO_COMBOLIST
   CallBack[IPROP_SPREAD_COMBOLIST]            = FP_ComboList;
#endif
#ifndef SSP_NO_PRINTJOBNAME
   CallBack[IPROP_SPREAD_PRINTJOBNAME]         = FP_Print;
#endif
#ifndef SSP_NO_PRINTABORTMSG
   CallBack[IPROP_SPREAD_PRINTABORTMSG]        = FP_Print;
#endif
#ifndef SSP_NO_PRINTHEADER
   CallBack[IPROP_SPREAD_PRINTHEADER]          = FP_Print;
#endif
#ifndef SSP_NO_PRINTFOOTER
   CallBack[IPROP_SPREAD_PRINTFOOTER]          = FP_Print;
#endif
#ifndef SSP_NO_UNITTYPE
   CallBack[IPROP_SPREAD_UNITTYPE]             = FP_UnitType;
#endif
#ifndef SSP_NO_EDITENTERACTION
   CallBack[IPROP_SPREAD_EDITENTERACTION]      = FP_EditEnterAction;
#endif
#ifndef SSP_NO_GRAYAREABACKCOLOR
   CallBack[IPROP_SPREAD_GRAYAREABACKCOLOR]    = FP_GrayAreaBackColor;
#endif

#ifndef SS_V30
#ifndef SSP_NO_GRAYAREAFORECOLOR
   CallBack[IPROP_SPREAD_GRAYAREAFORECOLOR]    = FP_GrayAreaForeColor;
#endif
#else
#ifndef SSP_NO_SCROLLBARTRACK
   CallBack[IPROP_SPREAD_SCROLLBARTRACK]       = FP_ScrollBarTrack;
#endif
#endif

#ifndef SSP_NO_ARROWSEXITEDITMOD
   CallBack[IPROP_SPREAD_ARROWSEXITEDITMOD]    = FP_Bool;
#endif
#ifndef SSP_NO_MOVEACTIVEONFOCUS
   CallBack[IPROP_SPREAD_MOVEACTIVEONFOCUS]    = FP_Bool;
#endif
#ifndef SSP_NO_TYPEPICTPICTURE
   CallBack[IPROP_SPREAD_TYPEPICTPICTURE]      = FP_TypePictPicture;
#endif
#ifndef SSP_NO_TYPEPICTCENTER
   CallBack[IPROP_SPREAD_TYPEPICTCENTER]       = FP_CellType2;
#endif
#ifndef SSP_NO_TYPEPICTSTRETCH
   CallBack[IPROP_SPREAD_TYPEPICTSTRETCH]      = FP_CellType2;
#endif
#ifndef SSP_NO_TYPEPICTSCALE
   CallBack[IPROP_SPREAD_TYPEPICTSCALE]        = FP_CellType2;
#endif
#ifndef SSP_NO_RETAINSELBLOCK
   CallBack[IPROP_SPREAD_RETAINSELBLOCK]       = FP_Bool;
#endif
#ifndef SSP_NO_ALLOWUSERFORMULAS
   CallBack[IPROP_SPREAD_ALLOWUSERFORMULAS]    = FP_Bool;
#endif
#ifndef SSP_NO_NOBEEP
   CallBack[IPROP_SPREAD_NOBEEP]               = FP_Bool;
#endif
#ifndef SSP_NO_OPERATIONMODE
   CallBack[IPROP_SPREAD_OPERATIONMODE]        = FP_OperationMode;
#endif
#ifndef SSP_NO_EDITCHARCASE
   CallBack[IPROP_SPREAD_EDITCHARCASE]         = FP_CellType2;
#endif

#ifndef SS_V30
#ifndef SSP_NO_DATACONNECT
   CallBack[IPROP_SPREAD_DATACONNECT]          = FP_DataConnect;
#endif
#else
#ifndef SSP_NO_CLIPBOARDOPTIONS
   CallBack[IPROP_SPREAD_CLIPBOARDOPTIONS]     = FP_ClipboardOptions;
#endif
#endif

#ifndef SSP_NO_EDITMODEREPLACE
   CallBack[IPROP_SPREAD_EDITMODEREPLACE]      = FP_Bool;
#endif
#ifndef SSP_NO_CLIPVALUE
   CallBack[IPROP_SPREAD_CLIPVALUE]            = FP_ClipValue;
#endif
#ifndef SSP_NO_EDITMULTILINE
   CallBack[IPROP_SPREAD_EDITMULTILINE]        = FP_CellType2;
#endif
#ifndef SSP_NO_HWND
   CallBack[IPROP_SPREAD_HWND]                 = FP_Spread_hWnd;
#endif
#ifndef SS_V30
#ifndef SSP_NO_LOADTABFILE
   CallBack[IPROP_SPREAD_LOADTABFILE]          = FP_LoadTabFile;
#endif
#else
#ifndef SSP_NO_COMBOHWND
   CallBack[IPROP_SPREAD_COMBOHWND]            = FP_CellType2;
#endif
#endif // SS_V30
#ifndef SSP_NO_MAXTEXTROWHEIGHT
   CallBack[IPROP_SPREAD_MAXTEXTROWHEIGHT]     = FP_MaxTextRowHeight;
#endif
#ifndef SSP_NO_MAXTEXTCELLHEIGHT
   CallBack[IPROP_SPREAD_MAXTEXTCELLHEIGHT]    = FP_MaxTextCellHeight;
#endif
#ifndef SSP_NO_MAXTEXTCOLWIDTH
   CallBack[IPROP_SPREAD_MAXTEXTCOLWIDTH]      = FP_MaxTextColWidth;
#endif
#ifndef SSP_NO_MAXTEXTCELLWIDTH
   CallBack[IPROP_SPREAD_MAXTEXTCELLWIDTH]     = FP_MaxTextCellWidth;
#endif
#ifndef SSP_NO_DATACOLCNT
   CallBack[IPROP_SPREAD_DATACOLCNT]           = FP_DataColCnt;
#endif
#ifndef SSP_NO_DATAROWCNT
   CallBack[IPROP_SPREAD_DATAROWCNT]           = FP_DataRowCnt;
#endif
#ifndef SSP_NO_AUTOCLIPBOARD
   CallBack[IPROP_SPREAD_AUTOCLIPBOARD]        = FP_Bool;
#endif
#ifndef SSP_NO_ALLOWMULTIBLOCKS
   CallBack[IPROP_SPREAD_ALLOWMULTIBLOCKS]     = FP_Bool;
#endif
#ifndef SSP_NO_ALLOWCELLOVERFLOW
   CallBack[IPROP_SPREAD_ALLOWCELLOVERFLOW]    = FP_Bool;
#endif
#ifndef SSP_NO_ALLOWDRAGDROP
   CallBack[IPROP_SPREAD_ALLOWDRAGDROP]        = FP_Bool;
#endif
#ifndef SSP_NO_GRIDLINECOLOR
   CallBack[IPROP_SPREAD_GRIDLINECOLOR]        = FP_GridLineColor;
#endif
#ifndef SSP_NO_GRIDSOLID
   CallBack[IPROP_SPREAD_GRIDSOLID]            = FP_GridSolid;
#endif
#ifndef SSP_NO_VSCROLLSPECIAL
   CallBack[IPROP_SPREAD_VSCROLLSPECIAL]       = FP_VScrollSpecial;
#endif
#ifndef SSP_NO_USERRESIZE
   CallBack[IPROP_SPREAD_USERRESIZE]           = FP_UserResize;
#endif
#ifndef SSP_NO_BUTTONDRAWMODE
   CallBack[IPROP_SPREAD_BUTTONDRAWMODE]       = FP_ButtonDrawMode;
#endif
#ifndef SSP_NO_SELECTBLOCKOPTIONS
   CallBack[IPROP_SPREAD_SELECTBLOCKOPTIONS]   = FP_SelectBlockOptions;
#endif
#ifndef SSP_NO_TYPEFLOATDEFCUR
   CallBack[IPROP_SPREAD_TYPEFLOATDEFCUR]      = FP_TypeFloatDefCur;
#endif
#ifndef SSP_NO_TYPEFLOATDEFDECIMAL
   CallBack[IPROP_SPREAD_TYPEFLOATDEFDECIMAL]  = FP_TypeFloatDefDecimal;
#endif
#ifndef SSP_NO_TYPEFLOATDEFSEP
   CallBack[IPROP_SPREAD_TYPEFLOATDEFSEP]      = FP_TypeFloatDefSep;
#endif
#ifndef SSP_NO_VIRTUALROWS
   CallBack[IPROP_SPREAD_VIRTUALROWS]          = FP_VirtualRows;
#endif
#ifndef SSP_NO_VIRTUALOVERLAP
   CallBack[IPROP_SPREAD_VIRTUALOVERLAP]       = FP_VirtualOverlap;
#endif
#ifndef SSP_NO_VIRTUALMODE
   CallBack[IPROP_SPREAD_VIRTUALMODE]          = FP_VirtualMode;
#endif
#ifndef SSP_NO_VIRTUALMAXROWS
   CallBack[IPROP_SPREAD_VIRTUALMAXROWS]       = FP_VirtualMaxRows;
#endif
#ifndef SSP_NO_VIRTUALSCROLLBUFFER
   CallBack[IPROP_SPREAD_VIRTUALSCROLLBUFFER]  = FP_VirtualScrollBuffer;
#endif
#ifndef SSP_NO_NOBORDER
   CallBack[IPROP_SPREAD_NOBORDER]             = FP_Bool;
#endif
#ifndef SSP_NO_GRIDSHOWHORIZ
   CallBack[IPROP_SPREAD_GRIDSHOWHORIZ]        = FP_GridShowHoriz;
#endif
#ifndef SSP_NO_GRIDSHOWVERT
   CallBack[IPROP_SPREAD_GRIDSHOWVERT]         = FP_GridShowVert;
#endif
#ifndef SSP_NO_STARTINGCOLNUMBER
   CallBack[IPROP_SPREAD_STARTINGCOLNUMBER]    = FP_StartingColNumber;
#endif
#ifndef SSP_NO_STARTINGROWNUMBER
   CallBack[IPROP_SPREAD_STARTINGROWNUMBER]    = FP_StartingRowNumber;
#endif
#ifndef SSP_NO_COLHEADERDISPLAY
   CallBack[IPROP_SPREAD_COLHEADERDISPLAY]     = FP_ColHeaderDisplay;
#endif
#ifndef SSP_NO_ROWHEADERDISPLAY
   CallBack[IPROP_SPREAD_ROWHEADERDISPLAY]     = FP_RowHeaderDisplay;
#endif
#ifndef SSP_NO_SORTKEYORDER
   CallBack[IPROP_SPREAD_SORTKEYORDER]         = FP_SortKeyOrder;
#endif
#ifndef SSP_NO_SORTKEY
   CallBack[IPROP_SPREAD_SORTKEY]              = FP_SortKey;
#endif
#ifndef SSP_NO_CHANGEMADE
   CallBack[IPROP_SPREAD_CHANGEMADE]           = FP_Bool;
#endif
#ifndef SSP_NO_VSCROLLSPECIALTYPE
   CallBack[IPROP_SPREAD_VSCROLLSPECIALTYPE]   = FP_VScrollSpecialType;
#endif
#ifndef SSP_NO_USERRESIZEROW
   CallBack[IPROP_SPREAD_USERRESIZEROW]        = FP_UserResizeRow;
#endif
#ifndef SSP_NO_USERRESIZECOL
   CallBack[IPROP_SPREAD_USERRESIZECOL]        = FP_UserResizeCol;
#endif
#ifndef SSP_NO_ROWHIDDEN
   CallBack[IPROP_SPREAD_ROWHIDDEN]            = FP_RowHidden;
#endif
#ifndef SSP_NO_COLHIDDEN
   CallBack[IPROP_SPREAD_COLHIDDEN]            = FP_ColHidden;
#endif
#ifndef SSP_NO_TYPEFLOATCURRENCY
   CallBack[IPROP_SPREAD_TYPEFLOATCURRENCY]    = FP_CellType2;
#endif
#ifndef SSP_NO_TYPEFLOATDECIMAL
   CallBack[IPROP_SPREAD_TYPEFLOATDECIMAL]     = FP_CellType2;
#endif
#ifndef SSP_NO_TYPEFLOATSEPCHAR
   CallBack[IPROP_SPREAD_TYPEFLOATSEPCHAR]     = FP_CellType2;
#endif
#ifndef SSP_NO_CURSORSTYLE
   CallBack[IPROP_SPREAD_CURSORSTYLE]          = FP_CursorStyle;
#endif
#ifndef SSP_NO_CURSORICON
   CallBack[IPROP_SPREAD_CURSORICON]           = FP_CursorIcon;
#endif
#ifndef SSP_NO_BORDERTYPE
   CallBack[IPROP_SPREAD_BORDERTYPE]           = FP_BorderType;
#endif
#ifndef SSP_NO_BORDERSTYLE
   CallBack[IPROP_SPREAD_BORDERSTYLE]          = FP_BorderStyle;
#endif
#ifndef SSP_NO_BORDERCOLOR
   CallBack[IPROP_SPREAD_BORDERCOLOR]          = FP_BorderColor;
#endif
#ifndef SSP_NO_MULTISELINDEX
   CallBack[IPROP_SPREAD_MULTISELINDEX]        = FP_MultiSelIndex;
#endif
#ifndef SSP_NO_MULTISELCOUNT
   CallBack[IPROP_SPREAD_MULTISELCOUNT]        = FP_MultiSelCount;
#endif
#ifndef SSP_NO_COLPAGEBREAK
   CallBack[IPROP_SPREAD_COLPAGEBREAK]         = FP_ColPageBreak;
#endif
#ifndef SSP_NO_ROWPAGEBREAK
   CallBack[IPROP_SPREAD_ROWPAGEBREAK]         = FP_RowPageBreak;
#endif
#ifndef SSP_NO_VIRTUALCURROWCOUNT
   CallBack[IPROP_SPREAD_VIRTUALCURROWCOUNT]   = FP_VirtualCurRowCount;
#endif
#ifndef SSP_NO_VIRTUALCURTOP
   CallBack[IPROP_SPREAD_VIRTUALCURTOP]        = FP_VirtualCurTop;
#endif
#ifndef SSP_NO_TYPESPIN
   CallBack[IPROP_SPREAD_TYPESPIN]             = FP_CellType2;
#endif
#ifndef SSP_NO_TYPEINTSPINWRAP
   CallBack[IPROP_SPREAD_TYPEINTSPINWRAP]      = FP_CellType2;
#endif
#ifndef SSP_NO_TYPEINTSPININC
   CallBack[IPROP_SPREAD_TYPEINTSPININC]       = FP_CellType2;
#endif
#ifndef SSP_NO_TYPETEXTPREFIX
   CallBack[IPROP_SPREAD_TYPETEXTPREFIX]       = FP_CellType2;
#endif
#ifndef SSP_NO_TYPETEXTSHADOWIN
   CallBack[IPROP_SPREAD_TYPETEXTSHADOWIN]     = FP_CellType2;
#endif
#ifndef SSP_NO_TYPECOMBOEDITABLE
   CallBack[IPROP_SPREAD_TYPECOMBOEDITABLE]    = FP_CellType2;
#endif
#ifndef SSP_NO_OWNERDRAWSTYLE
   CallBack[IPROP_SPREAD_OWNERDRAWSTYLE]       = FP_CellType2;
#endif
#ifndef SSP_NO_CHECKCENTER
   CallBack[IPROP_SPREAD_CHECKCENTER]          = FP_CellType2;
#endif
#ifndef SSP_NO_CHECKTYPE
   CallBack[IPROP_SPREAD_CHECKTYPE]            = FP_CellType2;
#endif
#ifndef SSP_NO_CHECKTEXT
   CallBack[IPROP_SPREAD_CHECKTEXT]            = FP_CheckText;
#endif
#ifndef SSP_NO_CHECKPICTURE
   CallBack[IPROP_SPREAD_CHECKPICTURE]         = FP_CheckPicture;
#endif
#ifndef SSP_NO_ABOUT
   CallBack[IPROP_SPREAD_ABOUT]                = FPSS_About;
#endif
#ifndef SSP_NO_CHECKTEXTALIGN
   CallBack[IPROP_SPREAD_CHECKTEXTALIGN]       = FP_CellType2;
#endif
#ifndef SSP_NO_COMBOINDEX
   CallBack[IPROP_SPREAD_COMBOINDEX]           = FP_TypeComboBox;
#endif
#ifndef SSP_NO_COMBOSTRING
   CallBack[IPROP_SPREAD_COMBOSTRING]          = FP_TypeComboBox;
#endif
#ifndef SSP_NO_COMBOCOUNT
   CallBack[IPROP_SPREAD_COMBOCOUNT]           = FP_TypeComboBox;
#endif
#ifndef SSP_NO_TYPEHALIGN
   CallBack[IPROP_SPREAD_TYPEHALIGN]           = FP_CellType2;
#endif
#ifndef SSP_NO_DAUTOHEADINGS
   CallBack[IPROP_SPREAD_DAUTOHEADINGS]        = FP_Bound;
#endif
#ifndef SSP_NO_DAUTOCELLTYPES
   CallBack[IPROP_SPREAD_DAUTOCELLTYPES]       = FP_Bound;
#endif
#ifndef SSP_NO_DAUTOFILL
   CallBack[IPROP_SPREAD_DAUTOFILL]            = FP_Bound;
#endif
#ifndef SSP_NO_DAUTOSIZECOLS
   CallBack[IPROP_SPREAD_DAUTOSIZECOLS]        = FP_Bound;
#endif
#ifndef SSP_NO_DATAFIELD
   CallBack[IPROP_SPREAD_DATAFIELD]            = FP_DataFieldName;
#endif
#ifndef SSP_NO_DATAFILLEVENT
   CallBack[IPROP_SPREAD_DATAFILLEVENT]        = FP_DataFillEvent;
#endif
#ifndef SSP_NO_DINFORMACTIVEROW
   CallBack[IPROP_SPREAD_DINFORMACTIVEROW]     = FP_Bound;
#endif
#ifndef SSP_NO_COMBOCURSEL
   CallBack[IPROP_SPREAD_COMBOCURSEL]          = FP_TypeComboBox;
#endif
#ifndef SSP_NO_SELMODEINDEX
   CallBack[IPROP_SPREAD_SELMODEINDEX]         = FP_TypeSelMode;
#endif
#ifndef SSP_NO_SELMODESELCOUNT
   CallBack[IPROP_SPREAD_SELMODESELCOUNT]      = FP_TypeSelMode;
#endif
#ifndef SSP_NO_SELMODESELECTED
   CallBack[IPROP_SPREAD_SELMODESELECTED]      = FP_TypeSelMode;
#endif
#ifndef SSP_NO_DAUTOSAVE
   CallBack[IPROP_SPREAD_DAUTOSAVE]            = FP_Bound;
#endif
#ifndef SS_V30
#ifdef IPROP_SPREAD_DATASELECT
   CallBack[IPROP_SPREAD_DATASELECT]           = FP_DataSelect;
#endif
#else // !SS_V30
#ifdef IPROP_SPREAD_SELBACKCOLOR
   CallBack[IPROP_SPREAD_SELBACKCOLOR]         = FP_SelBackColor;
#endif
#endif // !SS_V30
#ifdef SS_GP
   CallBack[IPROP_SPREAD_EDITEXPWIDTH]         = FP_CellType2;
   CallBack[IPROP_SPREAD_EDITEXPROWCNT]        = FP_CellType2;
#endif
#ifndef SSP_NO_BACKCOLORSTYLE
   CallBack[IPROP_SPREAD_BACKCOLORSTYLE]       = FP_BackColorStyle;
#endif
#ifdef SS_V30
#ifdef IPROP_SPREAD_SELFORECOLOR
   CallBack[IPROP_SPREAD_SELFORECOLOR]         = FP_SelForeColor;
#endif
#endif // SS_V30
}
