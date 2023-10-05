/***********
* FPLIST.H
***********/

#ifndef FPLIST_H
#define FPLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#if ((!(defined(SS_DLL) || defined(SS_OCX))) || defined(SS_V30))

#include "fppinfo.h"

//
// Listbox Extended Control Styles
//
#define LBSX_3D             0x0001
#define LBSX_MULTICOLUMN    0x0002

//
//  Listbox Window Messages
//
#ifndef LB_PUBLIC
#define LB_PUBLIC           (FPM_USER)
#endif

#define LB_SETSELECTIONMODE (LB_PUBLIC+0)
#define LB_GETTEXTEXTENT    (LB_PUBLIC+1)
#define LB_GETDBLCLKHPOS    (LB_PUBLIC+2)
#define LB_GETDBLCLKITEM    (LB_MSGMAX+3)

#define LB_PRIVATE          (LB_PUBLIC + 25)
#define LB_WRUSER           (LB_PUBLIC + 50)

#ifndef LB_MSGMAX
#define LB_MSGMAX           LB_WRUSER
#endif                      // for backward compatibility?

//
// Listbox Notification Messages
//
#define LBN_ESCAPE        30        /* the user pressed the escape key */
#define LBN_RETURN        31        /* the user pressed the return key */

#define LBN_LBUTTONDBLCLK 2         /* our own notification */
#define LBN_TOPITEM       1000
#define LBN_RBUTTONDBLCLK 1001      /* our own notification */
#define LBN_MBUTTONDBLCLK 1002      /* our own notification */

/*****************************************************************
* CLASS Styles (additional/replacing standard WINDOWS LBS_ styles
*****************************************************************/

#define LBS_DESCENDING_ORDER      0x0020L // Sort in descending order
#define LBS_THREEDIN              0x2000L // 3D-INSIDE property: "LOWERED"
#define LBS_THREEDOUT             0x4000L // 3D-INSIDE property: "RAISED"
#define LBS_SEARCHEXACTCASE       0x8000L

#define CBS_DESCENDING_ORDER      0x1000L // Sort in descending order
#define CBS_THREEDIN              0x2000L // 3D-INSIDE property: "LOWERED"
#define CBS_THREEDOUT             0x4000L // 3D-INSIDE property: "RAISED"
#define CBS_SEARCHEXACTCASE       0x8000L


/***********
* Messages
***********/

#define LBM_SETPROPERTY    (WM_USER + 203)
#define LBM_GETPROPERTY    (WM_USER + 204)

#define CBM_SETPROPERTY    (WM_USER + 201)
#define CBM_GETPROPERTY    (WM_USER + 202)

// NOTE: Listbox & Combobox share these messages.
#define LCM_SETPROP				(WM_USER + 301)
#define LCM_GETPROP				(WM_USER + 302)
#define LCM_MOUSEICONCHANGED	(WM_USER + 303)

#define LBM_CLICK          0x0501
#define LBM_KEYDOWN        0x0502
#define LBM_KEYPRESS       0x0503
#define LBM_COLWIDTHCHANGE 0x0504
#define LBM_DRAGDROPCOL    0x0505
#define LBM_LBUTTONDOWN    0x0506
#define LBM_LBUTTONDBLCLK  0x0507
#define LBM_VIRTUALREQUEST 0x0508
#define LBM_VIRTUALSEARCH  0x0509
#define LBM_GRPWIDTHCHANGE 0x050a
#define LBM_DRAGDROPGRP    0x050b
#ifdef LP30
#define LCM_TEXTTIPFETCH   0x050c
#define CBM_BEFOREDROPDOWN 0x050d
#define LBM_DRAGDROPCOMPLETE 0x050e
#endif

#define CBM_KEYDOWN        0x0552
#define CBM_KEYPRESS       0x0553
#define CBM_MOUSE          0x0554

// Msg used by fpPropDesigner to get list of values for a prop - SCP
#define LCM_GETPROPVALLIST 0x0555

/*********
* Styles
*********/


/***********************
* Property Definitions
***********************/

#include "lcprops.h"

extern FP_PROPLIST LCPropList[LCPROP_PropCnt];

/**************************************************
* Identify control for Property Browser  //$PBRW
**************************************************/
#define LB_LISTBOX_CTL   1
#define LB_COMBOBOX_CTL  2


/************
* Constants
************/

#define LB_COLOR_DEFAULT (COLORREF)(0x80000000L | COLOR_INACTIVEBORDER)

#define LB_LINE_NONE                    0
#define LB_LINE_SOLID                   1
#define LB_LINE_3DLOWERED               2
#define LB_LINE_3DRAISED                3

#define LB_SELTHREED_NONE               0
#define LB_SELTHREED_LOWERED            1
#define LB_SELTHREED_RAISED             2

#define LB_THREEDTEXT_NONE              0
#define LB_THREEDTEXT_RAISEDLIGHT       1
#define LB_THREEDTEXT_LOWEREDLIGHT      2
#define LB_THREEDTEXT_RAISEDHEAVY       3
#define LB_THREEDTEXT_LOWEREDHEAVY      4
#define LB_THREEDTEXT_DEFAULT           5

#define LB_COLWIDTHSCALE_TWIPS          0
#define LB_COLWIDTHSCALE_PIXELS         1
#define LB_COLWIDTHSCALE_AVGCHAR        2
#define LB_COLWIDTHSCALE_MAXCHAR        3

#define LB_SCROLLHSCALE_TWIPS           0
#define LB_SCROLLHSCALE_PIXELS          1
#define LB_SCROLLHSCALE_AVGCHAR         2
#define LB_SCROLLHSCALE_MAXCHAR         3
#define LB_SCROLLHSCALE_COLUMN          4
#define LB_SCROLLHSCALE_GROUPTOP        5
#define LB_SCROLLHSCALE_GROUPBOTTOM     6

#define LB_ACTION_SEARCH                0
#define LB_ACTION_SELECTALL             1
#define LB_ACTION_DESELECTALL           2
#define LB_ACTION_CLEAR                 3
#define LB_ACTION_DELETEROW             4
#define LB_ACTION_FORCEUPDATE           5
#define LB_ACTION_CLEARSEARCHBUFFER     6
#define LB_ACTION_INSERTCOL             7
#define LB_ACTION_DELETECOL             8
#define LB_ACTION_VIRTUALREFRESH        9
#define LB_ACTION_INSERTGRP             10
#define LB_ACTION_DELETEGRP             11
#define LB_ACTION_CLONECOL              12

#define LB_HIGHESTPRECEDENCE_ROW        0
#define LB_HIGHESTPRECEDENCE_COL        1
#define LB_HIGHESTPRECEDENCE_COMBINED   2

#define LB_COLPICTUREALIGNH_LEFT        0
#define LB_COLPICTUREALIGNH_CENTER      1
#define LB_COLPICTUREALIGNH_RIGHT       2
#define LB_COLPICTUREALIGNH_LEFTOFTEXT  3
#define LB_COLPICTUREALIGNH_RIGHTOFTEXT 4

#define LB_ALLOWCOLRESIZE_OFF           0
#define LB_ALLOWCOLRESIZE_RESIZEHEADER  1
#define LB_ALLOWCOLRESIZE_RESIZECOL     2

#define LB_ALLOWCOLDRAGDROP_OFF         0
#define LB_ALLOWCOLDRAGDROP_ALLCOLS     1
#define LB_ALLOWCOLDRAGDROP_NONFROZEN   2

#define LB_ALLOWGRPRESIZE_OFF           0
#define LB_ALLOWGRPRESIZE_RESIZEHEADER  1
#define LB_ALLOWGRPRESIZE_RESIZEGRP     2

#define LB_ALLOWGRPDRAGDROP_OFF         0
#define LB_ALLOWGRPDRAGDROP_ALLGRPS     1
#define LB_ALLOWGRPDRAGDROP_NONFROZEN   2

#define LB_VSCROLL_NOHOMEEND            0x01
#define LB_VSCROLL_NOPAGEUPDN           0x02
#define LB_VSCROLL_NOUPDN               0x04

#define LB_SORT_NONE                    0
#define LB_SORT_ASCENDING               1
#define LB_SORT_DESCENDING              2

#define LB_SORTDATATYPE_TEXTNOCASE      0
#define LB_SORTDATATYPE_TEXTCASE        1
#define LB_SORTDATATYPE_INTEGER         2
#define LB_SORTDATATYPE_FLOAT           3
#define LB_SORTDATATYPE_DATE			4

#define LB_SORTSTATE_ACTIVE             0
#define LB_SORTSTATE_ACTIVE_RESORT      1
#define LB_SORTSTATE_SUSPEND            2

#define LB_SEARCHMETHOD_EXACT           0
#define LB_SEARCHMETHOD_GREATER         1
#define LB_SEARCHMETHOD_PARTIAL         2

#define LB_AUTOSEARCH_NONE              0
#define LB_AUTOSEARCH_SINGLECHAR        1
#define LB_AUTOSEARCH_MULTIPLECHAR      2
#define LB_AUTOSEARCH_SINGLECHARGREATER 3

#define LB_SCROLLBAR_SHOW               0
#define LB_SCROLLBAR_SHOWWHENNEEDED     1
#define LB_SCROLLBAR_SHOWDISABLED       2
#define LB_SCROLLBAR_HIDE               3

#define CB_STYLE_DROPDOWN_COMBO         0
#define CB_STYLE_SIMPLE_COMBO           1
#define CB_STYLE_DROPDOWN_LIST          2

#define CB_APPLYTO_BOTH                 0
#define CB_APPLYTO_LISTONLY             1
#define CB_APPLYTO_EDITONLY             2

#define CB_COMPONENT_EDIT               0
#define CB_COMPONENT_LIST               1
#define CB_COMPONENT_TEXT               2
#define CB_COMPONENT_BUTTON             3

#define LB_VIRTACTION_ISHOME            0
#define LB_VIRTACTION_ISEND             1
#define LB_VIRTACTION_DOWN              2
#define LB_VIRTACTION_UP                3
#define LB_VIRTACTION_HOME              4
#define LB_VIRTACTION_END               5
#define LB_VIRTACTION_REFRESH           6
#define LB_VIRTACTION_ROWSREMOVED       7
#define LB_VIRTACTION_POS               8

#define LB_LINESTYLE_DEF                0
#define LB_LINESTYLE_NONE               1
#define LB_LINESTYLE_FLAT               2
#define LB_LINESTYLE_LOWERED            3
#define LB_LINESTYLE_RAISED             4
#define LB_LINESTYLE_LOWERED_W_LINE     5
#define LB_LINESTYLE_RAISED_W_LINE      6

#define LB_LINEAPPLYTO_DEF              0
#define LB_LINEAPPLYTO_ROWS             1
#define LB_LINEAPPLYTO_COLS             2
#define LB_LINEAPPLYTO_COLSH            3

#define LB_LISTAPPLYTO_DEF              0
#define LB_LISTAPPLYTO_ALLROWS          1
#define LB_LISTAPPLYTO_ALLCOLS          2
#define LB_LISTAPPLYTO_ALLGROUPS        3
#define LB_LISTAPPLYTO_SELROWS          4
#define LB_LISTAPPLYTO_ODDROWS          5
#define LB_LISTAPPLYTO_EVENROWS         6
#define LB_LISTAPPLYTO_COLHEADERS       7
#define LB_LISTAPPLYTO_GROUPHEADERS     8
#define LB_LISTAPPLYTO_SINGLECOLHEADER  9
#define LB_LISTAPPLYTO_SINGLEGRPHEADER  10
#define LB_LISTAPPLYTO_SINGLEGRP        11
#define LB_LISTAPPLYTO_SINGLE           12

#define LB_TEXTELLIPSES_DEFAULT         0
#define LB_TEXTELLIPSES_NO              1
#define LB_TEXTELLIPSES_YES             2

#define LB_TEXTTIP_OFF                  0
#define LB_TEXTTIP_ALWAYS               1
#define LB_TEXTTIP_ONFOCUS              2

#define LB_TEXTTIPMULTILINE_AUTO        0
#define LB_TEXTTIPMULTILINE_SINGLE      1
#define LB_TEXTTIPMULTILINE_MULTI       2

/*	// will use ALIGNH, ALIGNV, and MULTILINE contstants
#define LC_TT_XALIGN_AUTO   0
#define LC_TT_XALIGN_LEFT   1
#define LC_TT_XALIGN_CENTER 2
#define LC_TT_XALIGN_RIGHT  3

#define LC_TT_YALIGN_AUTO   0
#define LC_TT_YALIGN_TOP    1
#define LC_TT_YALIGN_CENTER 2
#define LC_TT_YALIGN_BOTTOM 3
*/
#define LC_TT_TEXTMAX			1024
#define LC_TT_COL_HEADER_ROW	-1
#define LC_TT_GRP_HEADER_ROW	-2
#define LC_TT_NOT_OVER_ROW		-3

#define LB_PICTALIGNH_DEF               0
#define LB_PICTALIGNH_LEFT              1
#define LB_PICTALIGNH_CENTER            2
#define LB_PICTALIGNH_RIGHT             3
#define LB_PICTALIGNH_LEFTOFTEXT        4
#define LB_PICTALIGNH_RIGHTOFTEXT       5

#define LB_PICTALIGNV_DEF               0
#define LB_PICTALIGNV_TOP               1
#define LB_PICTALIGNV_CENTER            2
#define LB_PICTALIGNV_BOTTOM            3
#define LB_PICTALIGNV_TOPOFTEXT         4
#define LB_PICTALIGNV_BOTTOMOFTEXT      5

#define LB_ALIGNH_DEF                   0
#define LB_ALIGNH_LEFT                  1
#define LB_ALIGNH_CENTER                2
#define LB_ALIGNH_RIGHT                 3

#define LB_ALIGNV_DEF                   0
#define LB_ALIGNV_TOP                   1
#define LB_ALIGNV_CENTER                2
#define LB_ALIGNV_BOTTOM                3

#define LB_MULTILINE_DEF                0
#define LB_MULTILINE_SINGLE             1
#define LB_MULTILINE_MULTI              2

#define LB_ORIENT_DEFAULT               0
#define LB_ORIENT_HORIZ                 1
#define LB_ORIENT_VERT_LTR              2
#define LB_ORIENT_DOWN                  3
#define LB_ORIENT_UP                    4
#define LB_ORIENT_INVERT                5
#define LB_ORIENT_VERT_RTL              6

#define FP_PICTSTYLE_NORMAL             0
#define FP_PICTSTYLE_STRETCH            1
#define FP_PICTSTYLE_STRETCHPROP        2
#define FP_PICTSTYLE_TILE               3

#define LB_MERGE_OFF                    0
#define LB_MERGE_ALWAYS                 1
#define LB_MERGE_RESTRICTED             2

#define LB_MOUSEOVERAREA_NONE           0
#define LB_MOUSEOVERAREA_BUTTON         1
#define LB_MOUSEOVERAREA_EDIT           2
#define LB_MOUSEOVERAREA_LIST           3

#define CBX_EDITALIGNH_LEFT             0
#define CBX_EDITALIGNH_CENTER           1
#define CBX_EDITALIGNH_RIGHT            2

#define CBX_EDITALIGNV_TOP              0
#define CBX_EDITALIGNV_CENTER           1
#define CBX_EDITALIGNV_BOTTOM           2

/*
#define FP_ROWHEIGHTAUTOSIZE_DEF        0
#define FP_ROWHEIGHTAUTOSIZE_BASIC      1
#define FP_ROWHEIGHTAUTOSIZE_TEXT       2
#define FP_ROWHEIGHTAUTOSIZE_PICT       3
#define FP_ROWHEIGHTAUTOSIZE_TEXTPICT   4

#define FP_ROWHEIGHTSEQUAL_DIFFERENT    0
#define FP_ROWHEIGHTSEQUAL_EQUAL        1
*/

#define FP_MAXSIZEMETHOD_BASIC          0
#define FP_MAXSIZEMETHOD_TEXT           1
#define FP_MAXSIZEMETHOD_PICT           2
#define FP_MAXSIZEMETHOD_TEXTPICT       3

#define FP_MAXSIZEUNITS_PIXELS          1
#define FP_MAXSIZEUNITS_ROW             2

#define FP_CELLPOSUNITS_PIXELS          1

#define FP_TEXTTIPDELAY_DEF             500

/****************************************************
* These are used with the LBM_VIRTUALSEARCH message
****************************************************/

#define LB_SEARCH_PARTIAL               0x01
#define LB_SEARCH_GREATER               0x02

#define ERR_RANGE_COL                   30500
#define ERR_RANGE_ROW                   30501
#define ERR_RANGE_GRP                   30502
#define ERR_INVALIDVAL                  1

#define LB_VIRT_VSCROLLMAX_DEF          5000

/*************
* Structures
*************/

typedef struct tagLB_COMPAREITEMSTRUCT
   {
   UINT  CtlType;
   UINT  CtlID;
   HWND  hwndItem;
   long  itemID1;
   DWORD itemData1;
   long  itemID2;
   DWORD itemData2;
   } LB_COMPAREITEMSTRUCT, FAR *LPLB_COMPAREITEMSTRUCT;

typedef struct tagLB_KEYDOWN
   {
   WORD wKeyCode;
   WORD wShiftState;
   } LB_KEYDOWN, FAR *LPLB_KEYDOWN;

typedef struct tagCB_MOUSE
   {
   BYTE   bComponent;
   WORD   Msg;
   WPARAM wParam;
   LPARAM lParam;
   } CB_MOUSE, FAR *LPCB_MOUSE;

typedef struct tagLB_VIRTUALREQUEST
   {
   WORD wActionRequested;              // Action being requested
                                       //    LB_VIRTACTION_ISTOP
                                       //    LB_VIRTACTION_ISEND
                                       //    LB_VIRTACTION_DOWN
                                       //    LB_VIRTACTION_UP
                                       //    LB_VIRTACTION_HOME
                                       //    LB_VIRTACTION_END
                                       //    LB_VIRTACTION_REFRESH
                                       //    LB_VIRTACTION_ROWSREMOVED
                                       //    LB_VIRTACTION_POS
   long lRowFirst;                     // Virtual top row to begin reading
   long lRowCount;                     // The number of rows requested
   long lPos;                          // The scroll bar position
   } LB_VIRTUALREQUEST, FAR *LPLB_VIRTUALREQUEST;

typedef struct tagLB_VIRTUALSEARCH
   {
   LPSTR lpszSearchString;
   WORD  wSearchType;
   long  lRowFound;
   } LB_VIRTUALSEARCH, FAR *LPLB_VIRTUALSEARCH;

typedef struct lc_texttipfetch
   {
   BOOL     fShow;
   WORD     wMultiLine;
   WORD     wAlignX;
   long     lOffsetX;
   long     lOffsetY;
   long     lWidth;
   short    nColOrGrp;
   long		lRow;
   TCHAR    szText[LC_TT_TEXTMAX + 1];
   } LC_TEXTTIPFETCH, FAR *LPLC_TEXTTIPFETCH;
/**********************
* Function Prototypes
**********************/

void FP_API LP_SetModeDesigner(HWND hWnd, HWND hWndOwner);

#else    // SPREAD DEFINED

#define CBPROP_ColumnEdit                  170

#define LBPROP_ColumnSearch                70
#define LBPROP_Action                      85
#define LBPROP_SearchIndex                 87
#define LBPROP_VirtualMode                 100
#define LBPROP_VirtualPageSize             186
#define CBM_GETPROPERTY    (WM_USER + 202)
#define LBM_SETPROPERTY    (WM_USER + 203)
#define LBM_GETPROPERTY    (WM_USER + 204)

#define LB_ACTION_VIRTUALREFRESH        9

#endif

#ifdef __cplusplus
}
#endif

#endif // FPLIST_H
