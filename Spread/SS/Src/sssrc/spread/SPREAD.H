
/***********
* SPREAD.H

* RAP01  Added "reserved" fields to maintain consistent structure
*        field offsets between DAO/ADO and OCX/DLL.
* RAP02  Added hFile to allow files to be loaded & saved in chunks 
*        rather than building a buffer containing the entire SS.
***********/

#ifndef SPREAD_H
#define SPREAD_H

//- Added by Masanori Iwasa at FarPoint -------
#include    "language.h"
//---------------------------------------------
#ifdef SS_V40
#include "..\edit\editnum\usernum.h"
#endif // SS_V40

#ifdef SS_V80
#include "cust_celltype.h"
#endif
#ifdef SS_OLDCALC
#define SS_OLDSAVE
#endif

#ifndef WIN32

//#define SS_BC
//#define SS_GP
//#define SS_UTP
//#define SS_MASKTEXT
//#define SS_GRID
//#define SS_SPREADVBXPP
//#define SS_SPREADVBX
//#define SS_W
//#define SS_SPREADOCX
//#define SS_GRIDOCX

#endif

/********
* SS_BC
********/

#ifdef SS_BC

#define SS_VB
#define SS_NOSAVEV1
#define SS_NOSORT
#define SS_NOPRINT
#define SS_NOSCBAR
#define SS_NOCT_COMBO
#define SS_NOCT_CHECK
#define SS_NOCT_INT
#define SS_NOCT_PIC
#define SS_NOCT_TIME
#define SS_NOCT_DATE
#define SS_NOCT_BUTTON
#define SS_NOCT_OWNERDRAW
#define SS_NOOVERFLOW
#define SS_NOCOPYMOVESWAP
#define SS_NOLISTBOX
#define SS_NOBORDERS
#define SS_NODRAGDROP
#define SS_NOVIRTUAL
#define SS_NOSUPERCLASS
#define SS_NOMAXTEXT
#define SS_NOUSERDATA
#define SS_VB1ONLY

/**********
* SS_GRID
**********/

#elif (defined(SS_GRID))

//#define SS_VB
#define SS_NOSAVEV1
#define SS_NOSORT
#define SS_NOPRINT
#define SS_NOCALC
#define SS_NOSCBAR
#define SS_NOCT_INT
#define SS_NOCT_OWNERDRAW
#define SS_NOOVERFLOW
#define SS_NOLISTBOX
#define SS_NOBORDERS
#define SS_NODRAGDROP
#define SS_NOSUPERCLASS
#define SS_BOUNDCONTROL
//#define SS_QE
#define SS_NOAUTOSIZE
#define SS_NOBUTTONDRAWMODE
#define SS_NOUSERDATA
#define SS_NOCALENDAR

/********
* SS_GP
********/

#elif (defined(SS_GP))

#define SS_VB                          // Should only be on for VB
#define SS_NOCALC                      // Dont include formula code
#define SS_NOSAVE1                     // Exclude code used for file saving
#define SS_NOSORT
#define SS_NOPRINT
#define SS_GRIDGRAY
#define SS_DEFPREFIX
#define SS_MAKETRANSPARENT
#define SS_CLIPBOARDCUTTOCOPY
//#define SS_USE16BITCOORDS
#define SS_QUERYADVANCECANCEL          // Used with VB to reverse the action
                                       // of the QueryAdvance Event
#define SS_NOCT_TIME
#define SS_NOCT_CHECK
#define SS_NOCT_OWNERDRAW
#define SS_NOOVERFLOW
#define SS_NOBORDERS
#define SS_NOMAXTEXT
#define SS_NODRAGDROP
#define SS_NOBUTTONDRAWMODE
#define SS_NOUSERDATA

/*********
* SS_UTP
*********/

#elif (defined(SS_UTP))

#define SS_USEAWARE
#define SS_BOUNDCONTROL
#define SS_VB
#define SS_NOCALC

/***************
* SS_SPREADVBX
***************/

#elif (defined(SS_SPREADVBX))

#ifndef SS_VB
#define SS_VB
#endif
#define SS_BOUNDCONTROL
//#define SS_QE
#define SS_NOSUPERCLASS

/*****************
* SS_SPREADVBXPP
*****************/

#elif (defined(SS_SPREADVBXPP))

#define SS_VB
#define SS_BOUNDCONTROL
#define SS_NOSUPERCLASS
//#define SS_QE

/***************
* SS_SPREADOCX
***************/

#elif (defined(SS_SPREADOCX))

#ifndef SS_OCX
#define SS_OCX
#endif
#ifndef SS_BOUNDCONTROL
#define SS_BOUNDCONTROL
#endif
//#define SS_QE

/***************
* SS_GRIDOCX
***************/

#elif (defined(SS_GRIDOCX))

#ifndef SS_OCX
#define SS_OCX
#endif
#define SS_NOSAVEV1
#define SS_NOSORT
#define SS_NOPRINT
#define SS_NOCALC
//#define SS_NOSCBAR
#define SS_NOCT_INT
#define SS_NOCT_OWNERDRAW
#define SS_NOOVERFLOW
#define SS_NOLISTBOX
#define SS_NOBORDERS
#define SS_NODRAGDROP
//#define SS_NOSUPERCLASS
#define SS_BOUNDCONTROL
//#define SS_QE
#define SS_NOAUTOSIZE
#define SS_NOBUTTONDRAWMODE
#define SS_NOUSERDATA
#define SS_NOCALENDAR

/***************
* SS_RSOCX
***************/

#elif (defined(SS_RSOCX))

#ifndef SS_OCX
#define SS_OCX
#endif
#ifndef SS_BOUNDCONTROL
#define SS_BOUNDCONTROL
#endif

/**************
* SS_MASKTEXT
**************/

#elif (defined(SS_MASKTEXT))

#define SS_VB

#endif


#if defined(SS_VB)
#define SS_NOPROCESSFOCUS              // Should only be on for VB
#endif

/*
#define SS_NOCHILDSCROLL
*/

#if defined(SS_NOXL_CONV)
#define SSF_NO_ExportToExcel
#define SSF_NO_IsExcelFile
#define SSF_NO_GetExcelSheetList
#define SSF_NO_ImportExcelSheet
#endif

/* Available flags
#define SS_NOSAVE
#define SS_NOSAVEV1
#define SS_NOSORT
#define SS_NOPRINT
#define SS_NOSCBAR
#define SS_NOCT_COMBO
#define SS_NOCT_CHECK
#define SS_NOCT_INT
#define SS_NOCT_PIC
#define SS_NOCT_TIME
#define SS_NOCT_DATE
#define SS_NOCT_BUTTON
#define SS_NOCT_OWNERDRAW
#define SS_VB
#define SS_NOCALC
#define SS_GRIDGRAY
#define SS_DEFPREFIX
#define SS_MAKETRANSPARENT
#define SS_CLIPBOARDCUTTOCOPY
#define SS_USE16BITCOORDS
#define SS_QUERYADVANCECANCEL
#define SS_USEAWARE
#define SS_BOUNDCONTROL
#define SS_NOSUPERCLASS
#define SS_NOOVERFLOW
#define SS_NOCOPYMOVESWAP
#define SS_NOLISTBOX
#define SS_NOBORDERS
#define SS_NODRAGDROP
#define SS_NOVIRTUAL
#define SS_NOMAXTEXT
#ifndef SS_V30
#define SS_QE
#endif
#define SS_NOCELLFONTCOLOR
#define SS_NOAUTOSIZE
#define SS_NOBUTTONDRAWMODE
#define SS_NOUSERDATA
#define SS_VB1ONLY
#define SS_NOCALENDAR
*/
//#ifdef SD40
//#include "\sd40\src\toolbox.h"
//#else
//#include "..\..\..\include\toolbox.h"
#include "toolbox.h"
//#endif
#include "ss_heap.h"

#ifndef SS_OLDCALC
#include "..\calc\calc.h"
#endif

#ifdef SS_V30
#if !defined(SS_OCX) && !defined(SS_VB)
#undef RGBCOLOR_DEFAULT
#endif

#include "..\..\..\..\fplibs\fptools\src\fptools.h"
#if !defined(SS_OCX) && !defined(SS_VB)
#undef RGBCOLOR_DEFAULT
#define RGBCOLOR_DEFAULT   (COLORREF)-1L
#endif

#ifdef SS_V35
#ifndef TT_V2
#define TT_V2
#endif
#endif
#include "..\..\..\..\fplibs\fptools\src\texttip.h"
#include "..\..\ssprvw\prvwapi.h"
#endif

#ifndef RC_INVOKED
#if defined(_WIN64) || defined(_IA64)
#pragma pack(8)
#else
#pragma pack(1)
#endif
#endif  /* RC_INVOKED */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SS_OCX) && !defined(CS_OFF)
#include "ss_csent.h"
#endif

#define SS_VERSION        3
#define SS_SAVEVERSION1   1
#define SS_SAVEVERSION2   2
#define SS_SAVEVERSION3   3
#define SS_SAVEVERSION4   4
#ifdef SS_OLDSAVE
#define SS_SAVEVERSIONMAX 3
#else
#define SS_SAVEVERSIONMAX 4
#endif

#define DIVIDEUP(Num1, Num2)   ((Num1 + Num2 - 1) / Num2)
#define SS_ISDEFCOLOR(clr) (clr == (0x80000000L | COLOR_INACTIVEBORDER) || clr == (COLORREF)-1L || clr == -1)

//#define SS_AdjustThumbSize(lpSS) (lpSS->fIs95)
/* RFW - 8/11/03 - 12342
#define SS_AdjustThumbSize(lpSS) FALSE
*/
#define SS_AdjustThumbSize(lpSS) TRUE

typedef GLOBALHANDLE FAR *LPGLOBALHANDLE;

// Currently not being used
//#define IDF_MONDDYY 104

/********************
* Internal Messages
********************/

#define SSM_SETEDITMODE          (WM_USER + 0)
#define SSM_NEXTCELL             (WM_USER + 1)
#define SSM_PREVCELL             (WM_USER + 2)
#define SSM_KILLFOCUSINTERNAL    (WM_USER + 3)
#define SSM_SETFOCUSINTERNAL     (WM_USER + 4)
#define SSM_SETFOCUSPOSTPROCESS  (WM_USER + 5)
#define SSM_PROCESSTAB           (WM_USER + 6)
#define SSM_NEXTROW              (WM_USER + 7)
#define SSM_PROCESSENTERKEY      (WM_USER + 8)
#define SSM_FOCUSFROMTAB         0x0555
#define SSM_INSDEL               0x0556
#define SSM_ARROWPRESSED         0x0557
// fix for #8720 -scl
#define SSM_SETACTIVECELLINTERNAL (WM_USER + 9)
#define SSM_RESUBCLASSEDITOR      (WM_USER + 10)

#define SSM_PROCESSKILLFOCUS     1570
#define SSM_PROCESSSETFOCUS      1571
#define SSM_PROCESSLBUTTONDOWN   1572
#define SSM_PROCESSLBUTTONDBLCLK 1573
#define SSM_PROCESSMOUSEMOVE     1574
#define SSM_PROCESSRBUTTON       1575
#define SSM_PROCESSSAVEBOUND     1576

// this define is used for the OCX to AutoSize
#define SSM_SETCONTROLSIZE       1580
#define SSM_UNHIDEACTIVECOL      1581
#define SSM_UNHIDEACTIVEROW      1582

#define SS_CB_REPLACESEL         1610

#define SSN_INVALIDCOLTYPE       168      // Internal

//#define SS_TYPE_CUSTOM           9

#define SS_PRINT_ACTION_PRINT      0
#define SS_PRINT_ACTION_PAGECALC   1
#define SS_PRINT_ACTION_PREVIEW    2
#define SS_PRINT_ACTION_NEXTCOL    3
#define SS_PRINT_ACTION_NEXTROW    4
#define SS_PRINT_ACTION_OWNERPRINT 5

/**********************************
* PrintPageCalc structure
**********************************/

typedef struct ss_printpagecalc
   {
   long  lPageCnt;
   float fPageWidth;                   // In Inches
   float fPageHeight;                  // In Inches
   } SS_PRINTPAGECALC, FAR *LPSS_PRINTPAGECALC;

#define SS_FONTTABLE_MAX       50

#define SS_CONTROL_BASEID      200

#define SS_SBMAX               32767

#ifdef SS_USE16BITCOORDS
#define SS_VMAXCOORD           20000
#else
#define SS_VMAXCOORD           1000000L
#endif

#define SS_SORTKEY_MAX         256

#define SS_INSDEL_TYPE_DEL     0
#define SS_INSDEL_TYPE_INS     1

#define SS_INSDEL_OP_KEY       0
#define SS_INSDEL_OP_CLIPBOARD 1
#define SS_INSDEL_OP_UNDO      2
#define SS_INSDEL_OP_POST      3

#define SS_INSDEL_ROW          0
#define SS_INSDEL_COL          1

typedef struct ss_insdel
   {
   WORD     wType;                     // SS_INSDEL_TYPE_DEL
                       // SS_INSDEL_TYPE_INS
   WORD     wOperation;                // SS_INSDEL_OP_KEY
                       // SS_INSDEL_OP_CLIPBOARD
                       // SS_INSDEL_OP_UNDO
   WORD     wDirection;                // SS_INSDEL_ROW
                       // SS_INSDEL_COL
   SS_COORD Coord1;
   SS_COORD Coord2;
   } SS_INSDEL, far *LPSS_INSDEL;

/******************************
* Memory allocation structure
******************************/

#define SS_ALLOCITEMCNT        32
#define ARRAY1_ALLOC_INC       4
#define ARRAY2_ALLOC_INC       16
#define CALCALLOCITEM(Num1)    (((Num1 + SS_ALLOCITEMCNT) >> 5) - 1)
#define CALCALLOCITEMREM(Num1) ((short)(Num1 & 0x001FL))

typedef struct ss_allocitem
   {
   MEMSIZE        wItemCnt;
   TBGLOBALHANDLE hItems;
   } SS_ALLOCITEM, huge *LPSS_ALLOCITEM;

/**************
* Cell Border
**************/

typedef struct ss_BorderItem
   {
   BYTE       bStyle;
   SS_COLORID idColor;
   } SS_BORDERITEM, FAR *LPSS_BORDERITEM;

typedef struct ss_CellBorder
   {
   SS_BORDERITEM Border[4];
   } SS_CELLBORDER, FAR *LPSS_CELLBORDER;

/*
typedef struct ss_Border
   {
   BYTE       bType;
   BYTE       bStyle;
   SS_COLORID ColorId;
   } SS_BORDER, FAR *LPSS_BORDER;
*/

/********************
* SpreadSheet Color
********************/

typedef struct ss_coloritem
   {
   SS_COLORID BackgroundId;
   SS_COLORID ForegroundId;
   } SS_COLORITEM, FAR *LPSS_COLORITEM;

/********************
* SpreadSheet Color
********************/

typedef short SS_SAVE_COLORID;

typedef struct ss_savecolor
   {
   SS_SAVE_COLORID BackgroundId;
   SS_SAVE_COLORID ForegroundId;
   SS_SAVE_COLORID ShadowColorId;
   SS_SAVE_COLORID ShadowTextId;
   SS_SAVE_COLORID ShadowDarkId;
   SS_SAVE_COLORID ShadowLightId;
   COLORREF        GrayAreaBackground;
   COLORREF        GrayAreaForeground;
   } SS_SAVECOLOR, FAR *LPSS_SAVECOLOR;

typedef struct ss_color
   {
   SS_COLORID BackgroundId;
   SS_COLORID ForegroundId;
   SS_COLORID ShadowColorId;
   SS_COLORID ShadowTextId;
   SS_COLORID ShadowDarkId;
   SS_COLORID ShadowLightId;
   } SS_COLOR, FAR *LPSS_COLOR;

/*******************
* Color table item
*******************/

typedef struct ss_colortblitem
   {
   COLORREF Color;
   HBRUSH   hBrush;
   long     lLastUsed;
   } SS_COLORTBLITEM, FAR *LPSS_COLORTBLITEM;

/*****************************
* Define font for font table
*****************************/

typedef struct ss_font
   {
   LOGFONT LogFont;
   short   FontId;
   HFONT   hFont;
   BYTE    DeleteFont;
   short   FontHeight;
   double  FontCellHeight;
   HFONT   hFontPrinter;
   short   nFontHeightPrinter;
   TBGLOBALHANDLE hCharWidths;         // SCP 12/31/98
   TBGLOBALHANDLE hCharWidthsPrinter;  // SCP 12/31/98
	DWORD   dwFontLanguageInfo;
   } SS_FONT, FAR *LPSS_FONT;

/********************************
* Font Points - Used internally
********************************/

typedef struct ss_fontpoints
   {
   short  Size;
   double CellPoints;
   } SS_FONTPOINTS, FAR *LPSS_FONTPOINTS;

/***********************************
* Define Control - Used internally
***********************************/

typedef struct ss_contol
   {
   HWND  hWnd;
   BYTE  Type;
   short CtrlID;
   long  Style;
#if SS_V80
   CT_HANDLE  hCT; // handle to custom celltype
   BOOL       fRenderer;
   FARPROC    wndProc;
   long  ldisp;
   BOOL  fDestroy;
#endif
   } SS_CONTROL, FAR *LPSS_CONTROL;

/*******************
* Calculation List
*******************/

typedef struct ss_calclist
   {
   TBGLOBALHANDLE hItems;
   short          ItemCnt;
   short          ItemAllocCnt;
   BYTE           ListBuilt;
   } SS_CALCLIST, FAR *LPSS_CALCLIST;

/***************************************
* Calculation Record - Used internally
***************************************/

typedef struct ss_calc
   {
   SS_CELLCOORD   CellCoord;
   TBGLOBALHANDLE hFormula;
   SS_CALCLIST    Ancestors;
   } SS_CALC, FAR *LPSS_CALC;

/******************
* Data Definition
******************/

#define SS_DATATYPE_DATETIME 1 // Currently not implemented
#define SS_DATATYPE_EDIT     2
#define SS_DATATYPE_FLOAT    4
#define SS_DATATYPE_INTEGER  5
#if defined(SS_V80) 
#define SS_DATATYPE_BUFFER   9
#if defined(_WIN64) || defined(_IA64)
#define SS_DATATYPE_LONGPTR  6
#endif
//typedef void (CALLBACK* SS_FREEHANDLE_FUNC)(HANDLE hValue);
#endif

#if SS_V80
typedef struct ss_data_buffer
{
   TBGLOBALHANDLE hBuffer;
   long lSize;
} SS_DATA_BUFFER, FAR *LPSS_DATA_BUFFER;
#endif

typedef struct ss_data
   {
   BYTE              bDataType;
#ifndef SS_NOOVERFLOW
   BYTE              bOverflow;
#endif
   union
      {
      long           lValue;
      double         dfValue;
      TBGLOBALHANDLE hszData;
#if SS_V80
      SS_DATA_BUFFER Buffer;
#if defined(_WIN64) || defined(IA64)
      LONG_PTR       lp; // int64
#endif
#endif
//      LPVOID         pData;
//#if SS_V80
//      struct {
//        HANDLE              hValue;
//        BOOL                fOwn;
//        SS_FREEHANDLE_FUNC  pFreeFunc;
//      } Handle;
//#endif
      } Data;
   } SS_DATA, FAR *LPSS_DATA;

/******************
* Cell Definition
******************/
// RAP01: If #ifdef'ed fields are added to this structure, add reserved 
//        fields of the same type in an #else to assure structure
//        offsets are preserved.
typedef struct ss_cell
   {
#ifndef SS_NOBORDERS
   TBGLOBALHANDLE hBorder;
//   SS_BORDER      Border;
#endif
   SS_FONTID      FontId;
   SS_COLORITEM   Color;
   SS_DATA        Data;
   TBGLOBALHANDLE hCellType;
#ifndef SS_NOCALC
   TBGLOBALHANDLE hCalc;
#ifdef SS_OLDCALC
   SS_CALCLIST    Dependents;
#endif
#endif
   BYTE           CellLocked;
#ifdef SS_BOUNDCONTROL
   BYTE           fDirty;
#else //>>RAP01a
   BYTE           res1;
#endif //<<RAP01a
#ifdef SS_V35
   TBGLOBALHANDLE hCellNote;
#else
   TBGLOBALHANDLE res2;
#endif
#ifdef SS_V40
	TBGLOBALHANDLE hCellTag;
#endif // SS_V40
   } SS_CELL, FAR *LPSS_CELL;

/********************
* Column Definition
********************/

// RAP01: If #ifdef'ed fields are added to this structure, add reserved 
//        fields of the same type in an #else to assure structure
//        offsets are preserved.
typedef struct ss_col
   {
#ifndef SS_NOBORDERS
   TBGLOBALHANDLE hBorder;
//   SS_BORDER      Border;
#endif
   BYTE           fColHidden;
   BYTE           ColLocked;
   BYTE           fPageBreak;
   BYTE           bUserResize;
   BYTE           fNonSelectable;
   long           dColWidthX100;
   //short          dColWidthInPixels;
   long			  dColWidthInPixels;
   SS_COORD       RowLastNonEmpty;
   SS_FONTID      FontId;
   SS_COLORITEM   Color;
   SS_DATA        Data;
   TBGLOBALHANDLE hCellType;
#ifndef SS_NOCALC
   TBGLOBALHANDLE hCalc;
#ifdef SS_OLDCALC
   SS_CALCLIST    Dependents;
#endif
#endif
#ifndef SS_NOUSERDATA
#if defined(_WIN64) || defined(_IA64)
   LONG_PTR       lUserData;
#else
   long           lUserData;
#endif
#endif
//#if (defined(SS_BOUNDCONTROL) || defined(SS_QE))
   BYTE           bDataFillEvent; // SS_DATAFILLEVENT_DEF 0
                                  // SS_DATAFILLEVENT_YES 1
                                  // SS_DATAFILLEVENT_NO  2
   BYTE           bDataType;
   TBGLOBALHANDLE hDBFieldName;
   short          dFieldNum;
/*
#else // >>RAP01a
   BYTE				res1;
   BYTE				res2;
   TBGLOBALHANDLE res3;
   short res4;
#endif // <<RAP01a
*/
#ifdef SS_V35
   TBGLOBALHANDLE hCellNote;
   short				nSortIndicator;
#else
   TBGLOBALHANDLE res5;
   short				res6;
#endif
#ifdef SS_V40
	BYTE           bMerge;
	TBGLOBALHANDLE hColID;
	TBGLOBALHANDLE hCellTag;
#endif
   } SS_COL, FAR *LPSS_COL;

/*****************
* Row Definition
*****************/

#ifdef SS_BOUNDCONTROL
#define SS_ROWDIRTY_SOME 1
#define SS_ROWDIRTY_ALL  2
#define SS_ROWDIRTY_DEL  3
#define SS_ROWDIRTY_INS  4
#endif

// RAP01: If #ifdef'ed fields are added to this structure, add reserved 
//        fields of the same type in an #else to assure structure
//        offsets are preserved.
typedef struct ss_row
   {
#ifndef SS_NOBORDERS
   TBGLOBALHANDLE hBorder;
//   SS_BORDER      Border;
#endif
   BYTE           fRowHidden;
   BYTE           fRowSelected;
   BYTE           fPageBreak;
   BYTE           RowLocked;
   BYTE           bUserResize;
#ifdef SS_UTP
   BYTE           fNonSelectable;
#endif // SS_UTP
   long           dRowHeightX100;
   // 22393 -scl
   //short          dRowHeightInPixels;
   long           dRowHeightInPixels;
   SS_FONTID      RowMaxFontId;
   SS_FONTID      FontId;
   SS_COLORITEM   Color;
   SS_DATA        Data;
   TBGLOBALHANDLE hCellType;
#ifndef SS_NOCALC
   TBGLOBALHANDLE hCalc;
#ifdef SS_OLDCALC
   SS_CALCLIST    Dependents;
#endif
#endif
   SS_ALLOCITEM   Cells;
#ifndef SS_NOUSERDATA
#if defined(_WIN64) || defined(_IA64)
   LONG_PTR       lUserData;
#else
   long           lUserData;
#endif
#endif
#ifdef SS_BOUNDCONTROL
   LPVOID         hlstrBookMark;
   BYTE           bDirty;
#else // >>RAP01a
   LPVOID res1;
   BYTE   res2;
#endif // <<RAP01a
#ifdef SS_V35
   TBGLOBALHANDLE hCellNote;
#else
   TBGLOBALHANDLE res3;
#endif
#ifdef SS_V40
	BYTE           bMerge;
	TBGLOBALHANDLE hCellTag;
#endif
   } SS_ROW, FAR *LPSS_ROW;

typedef struct ss_lastitem
   {
   SS_CELLCOORD   Coord;
   TBGLOBALHANDLE hItem;
   } SS_LASTITEM, FAR *LPSS_LASTITEM;

/**********************************
* Multiple select block structure
**********************************/

#define SS_MULTISEL_ALLOC_CNT 5

typedef struct ss_multiselblock
   {
   GLOBALHANDLE hItemList;             // List of SS_SELBLOCK
   short        dItemCnt;              // Number of items in list
   short        dItemAllocCnt;         // Number of allocated items
   } SS_MULTISELBLOCK, FAR *LPSS_MULTISELBLOCK;

/****************************
* Master Spreadsheet Record
****************************/

typedef struct ss_virtual
   {
   long     lVStyle;
   SS_COORD VMax;
   SS_COORD VTop;
   SS_COORD VSize;
   SS_COORD VOverlap;
   SS_COORD VPhysSize;                 // Physical Size
   } SS_VIRTUAL, FAR *LPSS_VIRTUAL;

/*********************
* Pic reference list
*********************/

typedef struct ss_piclist
   {
   HANDLE hPic;
   short  dRefCnt;
   } SS_PICLIST, FAR *LPSS_PICLIST;

/**************
* Undo Buffer
**************/

#define SS_UNDOTYPE_NONE   0
#define SS_UNDOTYPE_DELCOL 1
#define SS_UNDOTYPE_DELROW 2

typedef struct ss_undocol
   {
   TBGLOBALHANDLE hCol;
   TBGLOBALHANDLE hCellList;
   } SS_UNDOCOL, FAR *LPSS_UNDOCOL;

typedef struct ss_undobuffer
   {
   BYTE           bUndoType;
   TBGLOBALHANDLE hData;
   SS_COORD       ItemAllocCnt;
   SS_COORD       Col;
   SS_COORD       Row;
   SS_COORD       Col2;
   SS_COORD       Row2;
   } SS_UNDOBUFFER, FAR *LPSS_UNDOBUFFER;

#define SS_KBS_SHIFT 0x00010000
#define SS_KBS_CTRL  0x00020000

typedef struct ss_rowcol              //Type == SSROW SSCOL?
   {
   long             dCellSizeX100;
   // 22393 -scl
   //short            dCellSizeInPixels;
   long            dCellSizeInPixels;
   SS_COORD         HeaderCnt;
   SS_COORD         Frozen;
   SS_COORD         CurAt;
   SS_COORD         CurAtCalc;
   SS_COORD         UL;
   SS_COORD         LR;
   SS_COORD         LRAllVis;
   SS_COORD         AllocCnt;
   SS_COORD         DataCnt;
   SS_COORD         Max;                   // Number of Non Header rows or cols
   SS_COORD         ULPrev;
   SS_COORD         NumStart;
   SS_COORD         MaxBeforeSort;
   SS_COORD         AutoSizeVisibleCnt;
   SS_COORD         BlockCellCurrentPos;
   SS_COORD         PrevAt;
   SS_COORD         PrevUL;
   SS_ALLOCITEM     Items;
   TBGLOBALHANDLE   hCellSizeList;
   long             lCellSizeListCnt;
   long             lCellSizeListAllocCnt;
   SS_LASTITEM      LastRefCoord;
	SS_COORD         lHeaderDisplayIndex;
   SS_COORD         EditAt;
   } SS_ROWCOL, FAR *LPSS_ROWCOL;

typedef struct ss_printoptions
   {
   SS_PAGEFORMAT pf;
   SS_PRINTFUNC  lpfnPrintProc;
#if defined(_WIN64) || defined(_IA64)
   LONG_PTR		 lAppData;
#else
   LONG          lAppData;
#endif
   } SS_PRINTOPTIONS, FAR *LPSS_PRINTOPTIONS;

/****************************************************************************
* The SS_CELLRANGE structure contains the coordinates for a range of cells.
****************************************************************************/

typedef struct ss_cellrange
   {
   SS_CELLCOORD UL;  // Upper left cell coordinate
   SS_CELLCOORD LR;  // Lower right cell coordinate
   } SS_CELLRANGE, FAR *LPSS_CELLRANGE;

typedef struct ss_cursor
	{
   HCURSOR hCursor;
	BYTE    fDelete;
	} SS_CURSOR, FAR *LPSS_CURSOR;

/****************************
* Master Spreadsheet Record
****************************/

typedef struct ss_book
	{
   HWND             hWnd;
   unsigned         fIs95                      : 1;
   unsigned         EditModeOn                 : 1;
   unsigned         fVScrollVisible            : 1;
   unsigned         fHScrollVisible            : 1;
   unsigned         fHScrollBarTrack           : 1;
   unsigned         fVScrollBarTrack           : 1;
   unsigned         fKillFocusReceived         : 1;
   unsigned         fWindowPainted             : 1;
   unsigned         fFocusOn                   : 1;
   unsigned         fNoProcessButtonDown       : 1;
   unsigned         fInvalidated               : 1;
   unsigned         fSetFocusWhileMsgBeingSent : 1;
   unsigned         fProcessingKillFocus       : 1;
   unsigned         fProcessingLButtonDown     : 1;
   unsigned         fProcessSetFocus           : 1;
   unsigned         fNoSetScrollBars           : 1;
   unsigned         fSetFocusRecieved          : 1;
   unsigned         fNoSetCapture              : 1;
   unsigned         fNoInvalidate              : 1;
   unsigned         fSelBlockInvisible         : 1;
   unsigned         fDragDropBoxOn             : 1;
   unsigned         fFocusHighlightOn          : 1;
   unsigned         fRButtonDown               : 1;
   unsigned         fUpdateAutoSize            : 1;
   unsigned         fMsgSentToVB               : 1;
   unsigned         fBorderVisible             : 1;
   unsigned         fAllowUndo                 : 1;
   unsigned         fLeaveEditModeOn           : 1;
   unsigned         fComboKeyDown              : 1;
   unsigned         fIsActionKey               : 1;
   unsigned         fSetEditModeSent           : 1;
   unsigned         fAllowEditModePermSel      : 1; // SS_UTP
   unsigned         fNoShowActiveWithSel       : 1; // SS_UTP
   unsigned         fPrintExpandMultiLine      : 1; // SS_UTP
   unsigned         fPaintingToScreen          : 1;
   unsigned         fCheckBoxDefBitmapsLoaded  : 1;
   unsigned         fIgnoreBNClicked           : 1;
   unsigned         fDontSendEditModeMsg       : 1;
   unsigned         fIgnoreKillFocus           : 1;
   unsigned         fOnDraw                    : 1;
   unsigned         fInAutoSize                : 1;   // CTF
   unsigned         fSuspendExit               : 1;
	unsigned         fDontSendParentWMSetCursor : 1;
   unsigned         EditModeTurningOn          : 1;
   unsigned         EditModeTurningOff         : 1;
#if defined(SS_V40)
   unsigned         fDAutoCellTypesOldStyle    : 1;
#endif
   unsigned         fObjectBeingDestroyed      : 1; // SS_OCX only
   unsigned         fHasMouseMoved             : 1;
	TBGLOBALHANDLE   hSheets;
	TBGLOBALHANDLE   hActiveSheet;
	short            nSheetCnt;
	short            nSheet;
	short            nActiveSheet;
	short            nSheetSendingMsg;
   BOOL             AllowUserFormulas;
   BOOL             fAutoClipboard;
   BOOL             fAllowMultipleSelBlocks;
   BOOL             fAllowCellOverflow;
   BOOL             fAllowDragDrop;
   BOOL             fUseSpecialVScroll;
   BOOL             fEditModePermanent;
   BOOL             fArrowsExitEditMode;
   BOOL             fRetainSelBlock;
   BOOL             fEditModeReplace;
   BOOL             fNoBorder;
   BOOL             fNoBeep;
   BOOL             fAutoSize;
   BOOL             fScrollBarExtMode;
   BOOL             fProcessTab;
   BOOL             fScrollBarShowMax;
   BOOL             fScrollBarMaxAlign;
   BOOL             fMoveActiveOnFocus;
   BOOL             HorzScrollBar;
   BOOL             VertScrollBar;
   BOOL             Redraw;
   BOOL             DocumentProtected;
   BOOL             CalcAuto;
   WORD             wSpecialVScrollOptions;
   WORD             wButtonDrawMode;
   WORD             wSelBlockOption;
   WORD             wMessageBeingSent;
   WORD             wEnterAction;
   WORD             wScrollDirection;
   WORD             wMode;
   HWND             hWndVScroll;
   HWND             hWndHScroll;
   short            dVScrollWidth;
   short            dHScrollHeight;
   short            dXMargin;
   short            dYMargin;
   SS_FONTID        DefaultFontId;
   SS_CELLCOORD     CurCellSendingMsg;
   SS_CELLCOORD     CurVisCell;
   SS_CELLCOORD     CurCalcCell;
   SS_COORD         DragDropCurrentCol;
   SS_COORD         DragDropCurrentRow;
   SS_CELLTYPE      FormulaCellType;
   SS_COORD         ResizeCoord;
   short            ResizeOffset;
   short            ResizeCurrentPos;
   short            ResizeStartPos;
   short            ResizeMinPos;
   short            dxPixelsPerInch;
   short            dyPixelsPerInch;
   short            dUnitType;
   short            dClientWidth;
   short            dClientHeight;
   short            dBorderSize;
   short            dComboButtonBitmapWidth;
   HBITMAP          hBitmapComboBtn;
   HBITMAP          hBitmapLines;
   HBITMAP          hBitmapLines2;
   HBRUSH           hBrushLines;
   HBRUSH           hBrushLines2;
   short            StandardFontWidth;
   short            CalcFunctionsCnt;
   short            CalcFunctionsAllocCnt;
   TBGLOBALHANDLE   hCalcFunctions;
   short            ControlsCnt;
   short            ControlsAllocCnt;
   TBGLOBALHANDLE   hControls;
   DATEFORMAT       DefaultDateFormat;
   TIMEFORMAT       DefaultTimeFormat;
   FLOATFORMAT      DefaultFloatFormat; // from user, may be undefined
   FLOATFORMAT      WinFloatFormat;     // from Windows, always defined
   HCURSOR          hCursorPointer;
   HCURSOR          hCursorResizeV;
   HCURSOR          hCursorResizeH;
   HCURSOR          hCursorCurrent;
   BOOL             fCursorCurrentDef;
   SS_CURSOR        CursorDefault;
   SS_CURSOR        CursorColResize;
   SS_CURSOR        CursorRowResize;
   SS_CURSOR        CursorButton;
   SS_CURSOR        CursorGrayArea;
   SS_CURSOR        CursorLockedCell;
   SS_CURSOR        CursorColHeader;
   SS_CURSOR        CursorRowHeader;
   SS_CURSOR        CursorDragDropArea;
   SS_CURSOR        CursorDragDrop;
   RECT             ClientRect;
   OMEM_POOL        OmemPoolCell;
   OMEM_POOL        OmemPoolCol;
   OMEM_POOL        OmemPoolRow;
   TBGLOBALHANDLE   hBuffer;
   WNDPROC          lpfnCallBack;
   HWND             hWndOwner;
   HWND             hWndFocusPrev;
   BOOL             bDesignTime;          // determines if design mode or run mode
   short            nTwoDigitYearMax;
   GLOBALHANDLE     hXtra;     // buffer used to load/save OCX/VBX properties
   long             lXtraLen;  // length of buffer
   BYTE             bXtraVer;  // version of buffer
   WPARAM           wMsgLast;
   TBGLOBALHANDLE   hPicList;
   short            dPicListCnt;
   short            dPicListAllocCnt;
   DWORD            ActionKeys[SS_KBA_CNT];
   BYTE             bBackColorStyle;
   COLORREF         GrayAreaBackground;
   COLORREF         GrayAreaForeground;
   COLORREF         clrBackDefOrig;
   COLORREF         clrForeDefOrig;
	TBGLOBALHANDLE   hPrintAbortMsg;
	TBGLOBALHANDLE   hPrintJobName;
#ifndef SS_OLDCALC
   CALC_BOOK        CalcInfo;
#endif
//#ifdef SS_OCX // This is commented out because it messages up Excel
   LPVOID           lpOleControl;
   int              xExtent;  // pixel equivalent of COleControl::m_xExtent
   int              yExtent;  // pixel equivalent of COleControl::m_yExtent
   RECT             RectPaint;
//#endif
#ifdef SS_V30
#ifndef SS_NOXL_CONV
#if defined(_WIN64) || defined(_IA64)
   LONG_PTR         lXLLogFile;   //Excel File Conversion Log File
#else
   long             lXLLogFile;   //Excel File Conversion Log File
#endif
#endif
   BYTE             bClipboardOptions;
   BYTE             bAppearance;
   FP_TT_INFO       TT_Info;               // TextTip
   BOOL             fLoadTextTip;
   SS_COLORITEM     SelColor;
   HANDLE           hFile; //RAP02a
#endif // SS_V30
#ifdef SS_V35
   short            nCellNoteIndicator;
   short            nShowScrollTips;
   TBGLOBALHANDLE   hMonthLongNames;
   TBGLOBALHANDLE   hMonthShortNames;
   TBGLOBALHANDLE   hDayLongNames;
   TBGLOBALHANDLE   hDayShortNames;
   TBGLOBALHANDLE   hOkText;
   TBGLOBALHANDLE   hCancelText;
   BOOL             bScriptEnhanced;
   BOOL             fThumb;
   BOOL             fFetchCellNote;
	POINT            ptScrollTipsMouse;
	short            nScrollTipsWidth;
#endif // SS_V35
#ifdef SS_V40
	BYTE             fEditOverflow;
   DWORD            dwNextScrollTickCount;
#endif // SS_V40
#if SS_V70
	TBGLOBALHANDLE   hTabStrip;
	short            nTabStripLeftSheet;
	RECT             TabStripRect;
	RECT             TabSplitBoxRect;
   SS_FONTID        TabStripFontId;
	WORD             wTabStripPolicy;
	double           dfTabStripRatio;
	int              iTabResizeOffset;
   short            dVScrollWidthProp;
   short            dHScrollHeightProp;
   SS_COLORID       VScrollBarColorId;
   SS_COLORID       HScrollBarColorId;
#define SCIENTIFIC_BUFFER_SIZE 50
	TCHAR            szScientificBuffer[SCIENTIFIC_BUFFER_SIZE + 1];
	DWORD            dwScientificSel1;
	DWORD            dwScientificSel2;
#endif
#ifdef SS_V80
	BOOL			fThemesActive;
	WORD			wCellNoteIndicatorShape;
	WORD             wTabStripButtonPolicy;
    COLORREF	    CellNoteIndicatorColor;
	WORD			wAppearanceStyle;
	BOOL			bEnhanceStaticCells;
	WORD			wScrollBarStyle;
	WORD			wUseVisualStyles;
	WORD			wTabEnhancedShape;
	short			mouseTabSheet;
	short			mouseTabBtn;
	SS_COORD		mouseRow;
	SS_COORD		mouseCol;
	BOOL            MouseOver;
	COLORREF		sheetTabUpperNormalStartColor;
    COLORREF		sheetTabUpperNormalEndColor;
    COLORREF		sheetTabLowerNormalStartColor;
    COLORREF		sheetTabLowerNormalEndColor;
    COLORREF		sheetTabForeColor;
	COLORREF		sheetTabActiveForeColor;
    COLORREF		sheetTabUpperHoverStartColor;
    COLORREF		sheetTabUpperHoverEndColor;
    COLORREF		sheetTabLowerHoverStartColor;
    COLORREF		sheetTabLowerHoverEndColor;
	COLORREF        sheetTabOuterBorderColor;
	COLORREF		sheetTabInnerBorderColor;
	COLORREF        sheetScrollUpperNormalStartColor;
	COLORREF        sheetScrollUpperNormalEndColor;
	COLORREF		sheetScrollLowerNormalStartColor;
	COLORREF		sheetScrollLowerNormalEndColor;
	COLORREF        sheetScrollUpperHoverStartColor;
	COLORREF        sheetScrollUpperHoverEndColor;
	COLORREF		sheetScrollLowerHoverStartColor;
	COLORREF		sheetScrollLowerHoverEndColor;
	COLORREF        sheetScrollUpperPushedStartColor;
	COLORREF        sheetScrollUpperPushedEndColor;
	COLORREF		sheetScrollLowerPushedStartColor;
	COLORREF		sheetScrollLowerPushedEndColor;
	COLORREF		sheetScrollArrowColor;
	COLORREF		sheetGrayAreaStartColor;
	COLORREF        sheetGrayAreaEndColor;
	COLORREF		sheetSplitBoxStartColor;
	COLORREF		sheetSplitBoxEndColor;
    COLORREF		scrollTrackColor;
    COLORREF		scrollArrowColor;
    COLORREF		scrollUpperNormalStartColor;
    COLORREF		scrollUpperNormalEndColor;
    COLORREF		scrollLowerNormalStartColor;
    COLORREF		scrollLowerNormalEndColor;
    COLORREF		scrollUpperHoverStartColor;
    COLORREF		scrollUpperHoverEndColor;
    COLORREF		scrollLowerHoverStartColor;
    COLORREF		scrollLowerHoverEndColor;
    COLORREF		scrollUpperPushedStartColor;
    COLORREF		scrollUpperPushedEndColor;
    COLORREF		scrollLowerPushedStartColor;
    COLORREF		scrollLowerPushedEndColor;
    COLORREF		scrollHoverBorderColor;
    COLORREF		scrollBorderColor;
	COLORREF        colSelectedHoverUpperColor;
	COLORREF        colSelectedHoverLowerColor;
	COLORREF        colSelectedUpperColor;
	COLORREF        colSelectedLowerColor;
	COLORREF        colHoverUpperColor;
	COLORREF        colHoverLowerColor;
	COLORREF        colUpperColor;
	COLORREF        colLowerColor;
	COLORREF        rowSelectedHoverUpperColor;
	COLORREF        rowSelectedHoverLowerColor;
	COLORREF        rowSelectedUpperColor;
	COLORREF        rowSelectedLowerColor;
	COLORREF        rowHoverUpperColor;
	COLORREF        rowHoverLowerColor;
	COLORREF        rowUpperColor;
	COLORREF        rowLowerColor;
	COLORREF        colHeaderSelectedBorderColor;
	COLORREF        colHeaderBorderColor;
	COLORREF        rowHeaderSelectedBorderColor;
	COLORREF        rowHeaderBorderColor;
	COLORREF        cornerBackColor;
	COLORREF        cornerHoverColor;
	COLORREF        cornerTriangleColor;
	COLORREF        cornerTriangleHoverColor;
	COLORREF        cornerTriangleBorderColor;
	COLORREF        cornerTriangleHoverBorderColor;
	COLORREF        clrHighlightAlphaBlend;
	BYTE            bHighlightAlphaBlend;
	WORD			    wHighlightStyle;
	WORD            wHighlightHeaders;
	HBITMAP         hbmpAlpha;
	int             iAlphaBitmapWidth;
	int             iAlphaBitmapHeight;
	SS_CT_TABLE     CustCellTypes;  // user defined custom cell types
#if SS_OCX
   SS_CT_PROCS     *pCTProcs;
#endif
   BOOL            fWmPrintSkipScrollBars;
   BOOL            fWmSizeMoveScrollBars;
   OSVERSIONINFO       osver;
#endif // SS_V80
	BOOL            fLoading;
	TCHAR m_szPrinterDevice[200];
	TCHAR m_szPrinterDriver[100];
	TCHAR m_szPrinterOutput[100];
	DEVMODE m_PrinterDevMode;
   } SS_BOOK, FAR *LPSS_BOOK;

typedef struct ss_sheet
	{
   unsigned         HighlightOn                : 1;
   unsigned         FreezeHighlight            : 1;
   unsigned         NoEraseBkgnd               : 1;
   unsigned         NoPaint                    : 1;
   unsigned         EscapeInProgress           : 1;
   unsigned         fFormulaMode               : 1;
   unsigned         fEditModeDataChange        : 1;
   unsigned         fRowModeEditing            : 1;
   unsigned         fRowModeChangeMade         : 1;
   unsigned         fSuspendInvert             : 1;
   unsigned         fSingleSelRowFound         : 1;
   unsigned         fInvertBlock               : 1;
   unsigned         fSetBorder                 : 1;
   unsigned         fMultipleBlocksSelected    : 1;
   unsigned         fNoDrawFocusRect           : 1;
   unsigned         fSetActiveCellCalled       : 1;
   unsigned         fProcessingVQueryData      : 1;
   unsigned         fShowCellCalled            : 1;
   unsigned         fChangingOperationMode     : 1;
   unsigned         fProcessingBoundMove       : 1;
	unsigned         fKeyDownBeingPerformed     : 1;
	unsigned         fComboListChanged          : 1;
	unsigned         fIgnoreEditChange          : 1;
	LPSS_BOOK        lpBook;
	short            nSheetIndex;
   SS_ROWCOL        Col;
   SS_ROWCOL        Row;
   BOOL             GridLines;
   BOOL             RestrictRows;
   BOOL             RestrictCols;
   BOOL             fVirtualMode;
   BOOL             DocumentLocked;
   BOOL             fDataFillEvent;
   BOOL             fChangeMade;
   WORD             wOpMode;
   WORD             wGridType;
   WORD             ColHeaderDisplay;
   WORD             RowHeaderDisplay;
   COLORREF         GridColor;
   SS_COLOR         Color;
   SS_COLORITEM     LockColor;
   SS_FONTID        DefaultFontId;
   SS_CELLTYPE      DefaultCellType;
   SS_DATA          DefaultData;
   SS_COORD         MultiSelCnt;
#if defined(_WIN64) || defined(_IA64)
   LONG_PTR         lUserData;
#else
   long             lUserData;
#endif
   SS_CELLCOORD     BlockCellUL;
   SS_CELLCOORD     BlockCellLR;
   SS_CELLCOORD     BlockCellULPrev;
   SS_CELLCOORD     BlockCellLRPrev;
   SS_MULTISELBLOCK MultiSelBlock;
   SS_VIRTUAL       Virtual;
   SS_LASTITEM      LastCell;
   SS_CELLCOORD     DragDropBlockUL;
   BYTE             bHPixelOffset;
   BYTE             bVPixelOffset;
   TBGLOBALHANDLE   hBorder;
   SS_COORD         ExtSelAnchorRow;
   BOOL             fExtSelHighlight;
	short            nExtSelF8Mode;
   SS_COORD         DataRowCntOld;
   SS_COORD         DataFieldNameCnt;
   TBGLOBALHANDLE   hPrintColWidthList;
   long             lPrintColWidthListCnt;
   SS_UNDOBUFFER    UndoBuffer;
   short            fPrintingInProgress;
   long             lLButtonDownlParam;
   WORD             wUserResize;
   SS_PRINTOPTIONS  PrintOptions;
//#ifdef SS_BOUNDCONTROL
   LPVOID           hlstrVRefreshBookmark;
   BOOL             fDirty;
//#endif
#ifndef SS_OLDCALC
   CALC_SHEET       CalcInfo;
#else
   BOOL             fCalcBuildDependencies;
   short            CalcTableCnt;
   short            CalcTableAllocCnt;
   TBGLOBALHANDLE   hCalcTable;
   SS_CALCLIST      CalcAllDependents;
#endif
#ifdef SS_V30
   SS_COLORITEM     OddRowColor;
   SS_COLORITEM     EvenRowColor;
   BYTE             xFocusRectOrig;
#endif // SS_V30
#ifdef SS_V35
   short            nUserColAction;
   short            nAllColsSortIndicator;
   TBGLOBALHANDLE   hCellNote;
#endif // SS_V35
#ifdef SS_V40
   TBGLOBALHANDLE   hPaintSpanList;
   short            dPaintSpanListCnt;
   short            dPaintSpanListAllocCnt;
   TBGLOBALHANDLE   hCellSpan;
	SS_COORD         lColHeadersUserSortIndex;
	TBGLOBALHANDLE   hCellTag;
#endif // SS_V40
#ifdef SS_V70
	TBGLOBALHANDLE   hSheetName;
	BYTE             bActiveCellHighlightStyle;
	BOOL             fAllowColMove;
	BOOL             fAllowRowMove;
	SS_CELLRANGE     MoveStartRange;
	SS_COORD         ColMoveCurrentCol;
	SS_COORD         RowMoveCurrentRow;
	HWND             hWndHover;
	HWND             hWndHoverArrowA;
	HWND             hWndHoverArrowB;
	BOOL             fVisible;
#ifdef FP_OCX
   LPVOID           pfpOleBinding;
   BOOL             fBoundActive;
#endif
#endif // SS_V70
#ifdef SS_V80
	short			sortKeyCnt;
	SS_SORTKEY      sortKeys[SS_SORTKEY_MAX];
#endif
   } SPREADSHEET, FAR *LPSPREADSHEET;

typedef struct ss_paintbitmap
   {
   CRITICAL_SECTION cs; // for Apartment model threading
   int              cxPaintBitmap;
   int              cyPaintBitmap;
   HBITMAP          hPaintBitmap;
	BOOL             csInit;
	BOOL             fInUse;
   } SS_PAINTBITMAP, FAR *LPSS_PAINTBITMAP;

/**********************************
* Internal Print format structure
**********************************/

typedef struct ssx_printformat
   {
   SS_PAGEFORMAT pf;
   WORD          wAction;
   double        dfScale;
   double        dfScalePreviewX;
   double        dfScalePreviewY;
   short         TextWidthScreen;
   short         TextWidthPrinter;
   short         TextHeightScreen;
   short         TextHeightPrinter;
   short         dHorzPrinterRes;
   short         dVertPrinterRes;
   RECT          RectPreview;
   SS_COORD      ColsMax;
   SS_COORD      RowsMax;
   SS_COORD      ColLeft;
   SS_COORD      ColRight;
   SS_COORD      RowTop;
   SS_COORD      RowBottom;
   SS_COORD      ColAt;
   SS_COORD      RowAt;
   SS_COORD      ColLeftOrig;
   SS_COORD      RowTopOrig;
   short         AppHeaderLen;
   short         AppFooterLen;
   short         AppHeaderLenOrig;
   short         AppFooterLenOrig;
   short         yPrintArea;
   short         xPrintArea;
   short         x;
   short         y;
   short         cx;
   short         cy;
   long          lPage;
   LPSPREADSHEET lpSS;
   HWND          hWnd;
   HDC           hDC;
   HDC           hDCPreview;
#if defined(_WIN64) || defined(_IA64)
   LONG_PTR      lAppData;
#else
   long          lAppData;
#endif
   BOOL          fNextBandDevice;
   BOOL          fAtVirtualEnd;
   SS_PRINTINFO  PrintInfo;
   WORD          wPrinterType;
// 99914148 -scl
//	LPSHORT       lpColWidthList;
//	LPSHORT       lpRowHeightList;
	LPINT       lpColWidthList;
	LPINT       lpRowHeightList;
	long          lColWidthListCnt;
	long          lRowHeightListCnt;
   } SSX_PRINTFORMAT, FAR *LPSSX_PRINTFORMAT;

#define SS_PRINTER_HP4000PCL6  1

//#define SS_MODERESIZE_OFF      0
//#define SS_MODERESIZE_HORIZ    1
//#define SS_MODERESIZE_VERT     2

#define SS_RESIZEMODE_START    0
#define SS_RESIZEMODE_UPDATE   1
#define SS_RESIZEMODE_END      2

#define SS_VSCROLLID           1
#define SS_HSCROLLID           2

#define SS_F_SCROLL_LEFT       0x01
#define SS_F_SCROLL_RIGHT      0x02
#define SS_F_SCROLL_UP         0x04
#define SS_F_SCROLL_DOWN       0x08
#define SS_F_SCROLL_ROW        0x01 // not used with LEFT,RIGHT,UP or DOWN
#define SS_F_SCROLL_COL        0x02 // "
#define SS_F_MOVEACTIVECELL    0x10
#define SS_F_SCROLLMAX         0x20
#define SS_F_NOLEAVECELL       0x40

#define SS_ENTRYFIELDID        3
#define SS_COMBOBOXID          4

#define SS_BORDERSIZE          1

#define SS_WIDTH_DEFAULT       -1
#define SS_HEIGHT_DEFAULT      -1

#define SS_MAXDATALEN          500

#define SS_MODE_ARROW_LEFT     1
#define SS_MODE_ARROW_RIGHT    2
#define SS_MODE_ARROW_UP       3
#define SS_MODE_ARROW_DOWN     4

#define SS_USESINGLESELBAR(lpSS)  (lpSS->wOpMode == SS_OPMODE_SINGLESEL || \
                  (lpSS->wOpMode == SS_OPMODE_ROWMODE && \
                  !lpSS->fRowModeEditing))

#define SS_USESELBAR(lpSS) (SS_USESINGLESELBAR(lpSS) || \
                lpSS->wOpMode == SS_OPMODE_MULTISEL || \
                lpSS->wOpMode == SS_OPMODE_EXTSEL)

#ifdef SS_V80
#define SS_SEL_DRAWCOLOR(lpSS) ((lpSS->lpBook->wHighlightStyle == SS_HIGHLIGHTSTYLE_DEFAULT && \
                                 (lpSS->lpBook->SelColor.BackgroundId || lpSS->lpBook->SelColor.ForegroundId)) || \
                                lpSS->lpBook->wHighlightStyle == SS_HIGHLIGHTSTYLE_COLOR)
#else
#define SS_SEL_DRAWCOLOR(lpSS) (lpSS->lpBook->SelColor.BackgroundId || lpSS->lpBook->SelColor.ForegroundId)
#endif
#ifdef SS_V80
#define SS_SEL_ALPHA(lpSS)     (lpSS->lpBook->osver.dwMajorVersion >= 5 && \
                                 (lpSS->lpBook->wHighlightStyle == SS_HIGHLIGHTSTYLE_DEFAULT && \
                                 !SS_SEL_DRAWCOLOR(lpSS) && lpSS->lpBook->wAppearanceStyle) || \
                                lpSS->lpBook->wHighlightStyle == SS_HIGHLIGHTSTYLE_ALPHABLEND)
#else
#define SS_SEL_ALPHA(lpSS)     FALSE
#endif // SS_V80
#define SS_INVERTSEL(lpSS)     (!SS_SEL_DRAWCOLOR(lpSS) && !SS_SEL_ALPHA(lpSS))

#ifdef SS_V80
#define SS_SEL_HIGHLIGHTHEADERS(lpSS) ((lpSS->lpBook->wHighlightHeaders == SS_HIGHLIGHTHEADERS_DEFAULT && lpSS->lpBook->wAppearanceStyle) || \
                                       (lpSS->lpBook->wHighlightHeaders == SS_HIGHLIGHTHEADERS_ON))

#endif // SS_V80


#define SS_HIGHLIGHTSTYLE_DEFAULT				0
#define SS_HIGHLIGHTSTYLE_INVERT					1
#define SS_HIGHLIGHTSTYLE_COLOR					2
#define SS_HIGHLIGHTSTYLE_ALPHABLEND			3



/****************************
* Structure for class names
****************************/

typedef struct ss_classnames
   {
   LPTSTR TBEdit;
   LPTSTR SSEdit;
   LPTSTR SSEditHScroll;
   LPTSTR SSEditComboBox;
   LPTSTR TBPic;
   LPTSTR SSPic;
   LPTSTR TBDate;
   LPTSTR SSDate;
   LPTSTR TBTime;
   LPTSTR SSTime;
   LPTSTR TBInteger;
   LPTSTR SSInteger;
   LPTSTR TBFloat;
   LPTSTR SSFloat;
#ifdef SS_V40
   LPTSTR TBCurrency;
   LPTSTR SSCurrency;
   LPTSTR TBNumber;
   LPTSTR SSNumber;
   LPTSTR TBPercent;
   LPTSTR SSPercent;
#endif // SS_V40
#ifdef SS_V70
	LPTSTR SSScientific;
#endif // SS_V70
   LPTSTR TBSuperBtn;
   LPTSTR SSSuperBtn;
   LPTSTR TBComboBox;
   LPTSTR SSComboBox;
   LPTSTR SSListBox;
//   LPTSTR TBViewPict;
   LPTSTR TBCheckBox;
   LPTSTR SSCheckBox;
   LPTSTR TBScrlBar;
   } SS_CLASSNAMES, FAR *LPSS_CLASSNAMES;

extern SS_CLASSNAMES  SSClassNames[1];
extern short          dClassNameIndex;

typedef struct ss_globalTable
   {
   TBGLOBALHANDLE   hTable;
   short            dTableCnt;
   short            dTableAllocCnt;
   short            dTableUseCnt;
#ifdef WIN32
   CRITICAL_SECTION cs; // for Apartment model threading -scl
#endif
   } SS_GLOBALTABLE, FAR *LPSS_GLOBALTABLE;

typedef struct ss_colorTable
   {
   TBGLOBALHANDLE   hTable;
   long             dTableCnt;
   long             dTableAllocCnt;
   long             dTableUseCnt;
#ifdef WIN32
   CRITICAL_SECTION cs; // for Apartment model threading -scl
#endif
   } SS_COLORTABLE, FAR *LPSS_COLORTABLE;

extern SS_COLORTABLE  ColorTable;
extern SS_GLOBALTABLE FontTable;
#if defined(SS_V30) && defined(WIN32)
extern SS_GLOBALTABLE ExcelTable;
#endif
extern short          dSpreadInstCnt;
/*
extern TBGLOBALHANDLE hColorTable;
extern short          dColorTableCnt;
extern short          dColorTableAllocCnt;
extern short          dColorTableUseCnt;
extern TBGLOBALHANDLE hFontTable;
extern short          dFontTableCnt;
extern short          dFontTableAllocCnt;
*/

#define SS_COLORTABLE_USEMAX       20

#define SS_BORDERSTYLE_SOLID2      7
#define SS_BORDERSTYLE_SOLID3      8
#define SS_BORDERSTYLE_DOUBLESOLID 9
#define SS_BORDERSTYLE_RESERVED    10

#define SS_BORDERINDEX_LEFT   0
#define SS_BORDERINDEX_TOP    1
#define SS_BORDERINDEX_RIGHT  2
#define SS_BORDERINDEX_BOTTOM 3

#define SS_OVERFLOW_LEFT  0x01
#define SS_OVERFLOW_RIGHT 0x02
#define SS_OVERFLOW_TOP   0x04

#define SS_VSCROLL_UP       1
#define SS_VSCROLL_DOWN     2
#define SS_VSCROLL_PAGEUP   3
#define SS_VSCROLL_PAGEDOWN 4
#define SS_VSCROLL_HOME     5
#define SS_VSCROLL_END      6

#define SS_LOCKED_DEF       0
#define SS_LOCKED_ON        1
#define SS_LOCKED_OFF       2

#define SS_SCROLLARROW_WIDTH  30
#define SS_SCROLLARROW_HEIGHT 25

#define SS_CBM_SETDROPDOWNHWND 0x0700

#define SS_DATAFILLEVENT_DEF 0
#define SS_DATAFILLEVENT_YES 1
#define SS_DATAFILLEVENT_NO  2

#define SS_MODE_NONE         0
#define SS_MODE_RESIZE_VERT  1
#define SS_MODE_RESIZE_HORIZ 2
#define SS_MODE_BLOCK        3
#define SS_MODE_SELECT       4
#define SS_MODE_DRAGDROP     5
#define SS_MODE_TAB          6
#define SS_MODE_TAB_RESIZE   7
#define SS_MODE_BEGINCOLMOVE 8
#define SS_MODE_COLMOVE      9
#define SS_MODE_BEGINROWMOVE 10
#define SS_MODE_ROWMOVE      11
#define SS_MODE_USERSORT     12
#define SS_MODE_IDLE         13

#define SS_DEFCURSOR_DEFAULT      SS_CURSOR_DEFAULT
#define SS_DEFCURSOR_COLRESIZE    SS_CURSOR_DEFCOLRESIZE
#define SS_DEFCURSOR_ROWRESIZE    SS_CURSOR_DEFROWRESIZE
#define SS_DEFCURSOR_BUTTON       SS_CURSOR_ARROW
#define SS_DEFCURSOR_GRAYAREA     SS_CURSOR_ARROW
#define SS_DEFCURSOR_LOCKEDCELL   SS_CURSOR_DEFAULT
#define SS_DEFCURSOR_COLHEADER    SS_CURSOR_DEFAULT
#define SS_DEFCURSOR_ROWHEADER    SS_CURSOR_DEFAULT
#define SS_DEFCURSOR_DRAGDROPAREA SS_CURSOR_ARROW
#define SS_DEFCURSOR_DRAGDROP     SS_CURSOR_ARROW

//#ifdef FPCOLOR
//#undef FPCOLOR
//#endif

//#define FPCOLOR(c)         ((c != RGBCOLOR_DEFAULT && (c &  0x80000000)) ? GetSysColor((int)(short)c) : c)

#define SS_IsCellTypeFloat(lpCellType) ((lpCellType)->Type == SS_TYPE_FLOAT)
#ifdef SS_V40
#define SS_IsCellTypeNum(lpCellType) ((lpCellType)->Type == SS_TYPE_CURRENCY || (lpCellType)->Type == SS_TYPE_NUMBER || (lpCellType)->Type == SS_TYPE_PERCENT)
#else
#define SS_IsCellTypeNum(lpCellType) FALSE
#endif // SS_V40
#ifdef SS_V70
#define SS_IsCellTypeFloatNum(lpCellType) (SS_IsCellTypeFloat(lpCellType) || SS_IsCellTypeNum(lpCellType) || (lpCellType)->Type == SS_TYPE_SCIENTIFIC)
#else
#define SS_IsCellTypeFloatNum(lpCellType) (SS_IsCellTypeFloat(lpCellType) || SS_IsCellTypeNum(lpCellType))
#endif // SS_V70
#define SS_IsCellTypeNumeric(lpCellType) (SS_IsCellTypeFloat(lpCellType) || (lpCellType)->Type == SS_TYPE_INTEGER || SS_IsCellTypeNum(lpCellType))

#ifdef SS_V40
#define SS_GetColHeadersUserSortRow(lpSS) (lpSS->lColHeadersUserSortIndex == -1 ? lpSS->Row.HeaderCnt - 1 : min(lpSS->lColHeadersUserSortIndex, lpSS->Row.HeaderCnt - 1))
#else
#define SS_GetColHeadersUserSortRow(lpSS) 0
#endif // SS_V40

/**********************
* Function Prototypes
**********************/

LRESULT DLLENTRY SpreadSheetProc(HWND hWnd, UINT Msg,
                              WPARAM wParam, LPARAM lParam);
//LPSPREADSHEET  SS_Lock(HWND hwnd);
//BOOL SSx_Unlock(HWND hwnd);
//#define SS_Unlock(hWnd)

BOOL RegisterSpreadSheet(HANDLE hInstance, LPCTSTR lpszClassName,
                         short dClassIndex);
BOOL UnRegisterSpreadSheet(HANDLE hInstance, LPCTSTR lpszClassName);
void SS_DeleteControls(void);
BOOL SS_ProcessMnemonic(LPSPREADSHEET lpSS, TCHAR cChar);
HACCEL SS_CreateAcceleratorTable(LPSPREADSHEET lpSS, USHORT FAR* lpcAccel);
void SS_DestroyAcceleratorTable(HACCEL hAccel);
BOOL SSx_ScrollNextCell(LPSPREADSHEET lpSS, BOOL fIsNext,  BOOL fNextRow);
BOOL SSx_ScrollPrevCell(LPSPREADSHEET lpSS, BOOL fIsNext);
BOOL SS_ScrollLineUpTest(LPSPREADSHEET lpSS);
BOOL SS_ScrollLineDownTest(LPSPREADSHEET lpSS);

LPSS_CELLTYPE SS_RetrieveCellType(LPSPREADSHEET lpSS,
                                  LPSS_CELLTYPE CellType,
                                  LPSS_CELL Cell, SS_COORD CellCol,
                                  SS_COORD CellRow);
TBGLOBALHANDLE SS_GetEditModeOnData(HWND hWnd, LPSPREADSHEET lpSS,
                                    BOOL fValue);
TBGLOBALHANDLE SS_GetData(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType,
                          SS_COORD Col, SS_COORD Row, BOOL fValue);

BOOL SS_SetColFieldName(LPSPREADSHEET lpSS, SS_COORD Col, LPCTSTR lpText);
TBGLOBALHANDLE SS_GetColFieldName(LPSPREADSHEET lpSS, SS_COORD Col);

BOOL SS_ClipData(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                 LPTSTR lpszData, BOOL fValue, BOOL fSendDataChange);
#if defined(_WIN64) || defined(_IA64)
BOOL SS_Print(LPSPREADSHEET lpSS, LPCTSTR lpszPrintJobName, LPSS_PAGEFORMAT lpPageFormat,
              SS_PRINTFUNC lpfnPrintProc, LONG_PTR lAppData, ABORTPROC lpfnAbortProc,
              WORD wAction, HDC hDCPreview, LPRECT lpRectPreview,
              LPSS_PRINTPAGECALC lpPrintPageCalc, LPDEVMODE lpDevModeIn, LPCTSTR lpszDriver,
              LPCTSTR lpszDevice, LPCTSTR lpszOutput);
#else
BOOL SS_Print(LPSPREADSHEET lpSS, LPCTSTR lpszPrintJobName, LPSS_PAGEFORMAT lpPageFormat,
              SS_PRINTFUNC lpfnPrintProc, LONG lAppData, ABORTPROC lpfnAbortProc,
              WORD wAction, HDC hDCPreview, LPRECT lpRectPreview,
              LPSS_PRINTPAGECALC lpPrintPageCalc, LPDEVMODE lpDevModeIn, LPCTSTR lpszDriver,
              LPCTSTR lpszDevice, LPCTSTR lpszOutput);
#endif
#if defined(SS_V30)
void SS_PrintPreviewGetInfo(LPSPREADSHEET lpSS, LPPRVW_INFO lpPrvwInfo);
void SS_PrintPreviewPrintPage(LPSPREADSHEET lpSS, LPPRVW_PAGE lpPrvwPage);
#endif

#ifdef SS_GP
LPSS_CELLTYPE DLLENTRY SSSetTypeEditExpanding(HWND hWnd,
                                              LPSS_CELLTYPE CellType,
                                              long Style, short Len,
                                              short ChrSet, short ChrCase,
                                              double dfWidth, short nRowCnt);
#endif

BOOL SS_InitSpreadPreHwnd(LPSS_BOOK lpBook, COLORREF crForegrnd, COLORREF crBackgrnd);
BOOL SS_InitSpreadWithHwnd(HWND hWnd, LPSS_BOOK lpBook);

BOOL SS_GetColor(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                 LPCOLORREF lpBackground, LPCOLORREF lpForeground);
BOOL SS_SetColorRange(LPSPREADSHEET lpSS,
                      SS_COORD Col, SS_COORD Row,
                      SS_COORD Col2, SS_COORD Row2,
                      COLORREF Background, COLORREF Foreground);

BOOL SS_SetBorderRange(LPSPREADSHEET lpSS,
                       SS_COORD Col, SS_COORD Row,
                       SS_COORD Col2, SS_COORD Row2,
                       WORD wBorderType, WORD wBorderStyle,
                       COLORREF Color);

BOOL SS_GetColWidth(LPSPREADSHEET lpSS, SS_COORD Col, LPDOUBLE lpdfColWidth);
BOOL SS_GetRowHeight(LPSPREADSHEET lpSS, SS_COORD Row, LPDOUBLE lpdfRowHeight);

BOOL SS_CopyCellType(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellTypeDest,
                     LPSS_CELLTYPE lpCellTypeSrc);

WORD fpSplashIsExpired(HINSTANCE);
BOOL fpIsTrialVersion();

#ifdef SS_V35
#define BEFOREUSERSORT_DEFAULTACTION_CANCEL     0
#define BEFOREUSERSORT_DEFAULTACTION_AUTOSORT   1
#define BEFOREUSERSORT_DEFAULTACTION_MANUALSORT 2

short SS_GetUserColAction(LPSPREADSHEET lpSS);
void  SS_SetUserColAction(LPSPREADSHEET lpSS, short nUserColAction);

BOOL  SS_GetScriptEnhanced(LPSS_BOOK lpBook);
void  SS_SetScriptEnhanced(LPSS_BOOK lpBook, BOOL bScriptEnhanced);
#endif

#define FPSPLASH_NONE    0
#define FPSPLASH_OK      1
#define FPSPLASH_EXPIRED 2

void SS_PaintPicture(LPSPREADSHEET lpSS, HDC hDC, HDC hDCDev,
                     LPRECT lpRect, double dfScaleX, double dfScaleY,
                     HPALETTE hPal, HANDLE hPict, long lStyle,
                     COLORREF clrBack, HBRUSH hBrush, BOOL bBackDiff,
                     BOOL IsLastCol, BOOL IsLastRow, BOOL fSelAlpha);
HANDLE SS_LoadPicture(HWND hWnd, LPCTSTR lpszFileName, long lStyle,
                      HPALETTE FAR *hPal);
HBITMAP SS_PictGetBitmap(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                         LPBOOL lpfDeletePict);
HBITMAP SSx_PictGetBitmap(HDC hDC, LPRECT lpRect, HPALETTE hPal, HANDLE hPict,
                          long lStyle, COLORREF clrBack, BOOL fCrop,
                          LPBOOL lpfDeletePict);

BOOL SS_CheckLicense(LPTSTR lpszFileName, LPTSTR lpszLicName, BOOL fRuntime);
BOOL SS_IsWindowsV95(void);

#define SS_EncodeSysColor(i) ((COLORREF)(i | 0x80000000L))
#define SS_IsEncodedSysColor(cr) ((cr & 0xff000000L) == 0x80000000L && cr != RGBCOLOR_DEFAULT)
#define SS_DecodeSysColor(cr) ((int)(cr & 0xff))
#define SS_TranslateColor(cr) ((COLORREF)(SS_IsEncodedSysColor(cr) ? GetSysColor(SS_DecodeSysColor(cr)) : cr))

int SS_Compare(LPSPREADSHEET lpSS, LPSS_COL lpCol, SS_COORD Col, LPSS_ROW lpRow, SS_COORD Row,
               LPSS_COL lpCol2, SS_COORD Col2, LPSS_ROW lpRow2, SS_COORD Row2, WORD wSortOrder, BOOL fUseButtonText);

#ifdef SS_OCX
// ssocxctl.cpp
BOOL SS_IsDesignTime(LPSS_BOOK lpBook);
#endif

LPSS_BOOK SS_BookLock(HWND hWnd);
TBGLOBALHANDLE SS_BookGetSheetHandleFromIndex(LPSS_BOOK lpBook, short nIndex);
LPSPREADSHEET SS_BookLockSheetIndex(LPSS_BOOK lpBook, short nIndex);
LPSPREADSHEET SS_BookLockActiveSheet(LPSS_BOOK lpBook);
LPSPREADSHEET SS_BookLockSheet(LPSS_BOOK lpBook);
LPSPREADSHEET SS_SheetLockIndex(HWND hWnd, short nIndex);
LPSPREADSHEET SS_SheetLock(HWND hWnd);
LPSPREADSHEET SS_SheetLockActive(HWND hWnd);

#ifndef SS_NO_USE_SH
#define SS_BookUnlock(hWnd) TRUE
#define SS_BookUnlockSheetIndex(lpBook, nIndex) TRUE
#define SS_BookUnlockSheet(lpBook) TRUE
#define SS_BookUnlockActiveSheet(lpBook) TRUE
#define SS_SheetUnlock(hWnd) TRUE
#define SS_SheetUnlockIndex(hWnd, nIndex) TRUE
#define SS_SheetUnlockActive(hWnd) TRUE
#else
BOOL SS_BookUnlock(HWND hWnd);
BOOL SS_BookUnlockSheetIndex(LPSS_BOOK lpBook, short nIndex);
BOOL SS_BookUnlockSheet(LPSS_BOOK lpBook);
BOOL SS_BookUnlockActiveSheet(LPSS_BOOK lpBook);
BOOL SS_SheetUnlock(HWND hWnd);
BOOL SS_SheetUnlockIndex(HWND hWnd, short nIndex);
BOOL SS_SheetUnlockActive(HWND hWnd);
#endif

#define SS_IsActiveSheet(lpSS) (lpSS->nSheetIndex == lpSS->lpBook->nActiveSheet)

#define SS_SELBLOCKOPTION(lpSS) ((lpSS->lpBook->fEditModePermanent || \
                         lpSS->wOpMode == SS_OPMODE_ROWMODE || \
                         lpSS->wOpMode == SS_OPMODE_SINGLESEL || \
                         lpSS->wOpMode == SS_OPMODE_MULTISEL || \
                         lpSS->wOpMode == SS_OPMODE_EXTSEL) ? 0 : lpSS->lpBook->wSelBlockOption)

#ifndef RC_INVOKED
#pragma pack()
#endif  /* RC_INVOKED */

#ifdef __cplusplus
}
#endif

#endif
