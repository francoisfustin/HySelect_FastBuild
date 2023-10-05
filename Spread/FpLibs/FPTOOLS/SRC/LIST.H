/*********
* LIST.H
*********/

#ifndef LIST_H
#define LIST_H


#if ((!(defined(SS_DLL) || defined(SS_OCX))) || defined(SS_V30))

#include "fptools.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SoftwareSecure and version defines */
#ifdef LP30
#define LP_SECURE_PRODUCT_NAME		"ProductGoku"
#define LP_SECURE_PRODUCT_PREFIX	"FLP"
#define LP_SECURE_PRODUCT_CODE		43201
#ifdef FP_ADB
//VERSION_NUMBER
#define LP_SECURE_PRODUCT_STRING	"List Pro ADO Controls version 3.0.51"
#else
#define LP_SECURE_PRODUCT_STRING	"List Pro Controls version 3.0.51"
#endif
#define LP_VER_MAJOR				3
#define LP_VER_MINOR				0
#define LP_VER_REV					51
//VERSION_NUMBER
#else			// MFC controls:
#define LP_SECURE_PRODUCT_NAME		"ProductDaedalus"
#define LP_SECURE_PRODUCT_PREFIX	"FLP"
#define LP_SECURE_PRODUCT_CODE		2344
#ifdef FP_ADB
//VERSION_NUMBER
#define LP_SECURE_PRODUCT_STRING	"List Pro ADO Controls version 2.1.20"
#else
#define LP_SECURE_PRODUCT_STRING	"List Pro Controls version 2.1.20"
#endif
#define LP_VER_MAJOR				2
#define LP_VER_MINOR				1
#define LP_VER_REV					20
//VERSION_NUMBER
#endif

#include <string.h>
#ifdef LP30
#include "texttip.h"
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif  /* RC_INVOKED */

#define IDCURSOR_RESIZE    9001
#define IDCURSOR_DRAGDROP  9002

#ifndef APSTUDIO_READONLY_SYMBOLS


#include "fpmemmgr.h"

#ifdef LB_V20
#include "lpncpnt.h"
#else
#include "fpncpnt.h"
#endif

#include "fplist.h"

// String Macros
#define StrnCmp(s1, s2, n)    _fstrncmp(s1, s2, n)
#define StrniCmp(s1, s2, n)   _fstrnicmp(s1, s2, n)
#define StrnnCpy(dst, src, n) \
   {                          \
   (dst)[0] = (char)0;        \
   _fstrnset (dst, ' ', n);   \
   (dst)[n] = (char)0;        \
   _fstrncpy (dst, src, n);   \
   }

//
// Bit Utility macros
//
#define  ISBETWEEN(x, a, b) ((x >= min(a, b)) && (x <= max(a, b)))
#define  InRange(l,a,h)     ((l) <= (a) && (a) <= (h))
#define  NotInSet(a,b)      (!((a)&(b)))
#define  InSet(a,b)         ((a)&(b))
#define  BitOn(a,b)         ((a)|=(b))
#define  BitOff(a,b)        ((a)&=~(b))

typedef BOOL FAR      *LPBOOL;
typedef short          SHORT;
typedef short FAR     *LPSHORT;
typedef HFONT FAR     *LPHFONT;
typedef float FAR     *LPFLOAT;
typedef COLORREF FAR  *LPCOLORREF;

typedef short          LBCOLINDEX, FAR *LPLBCOLINDEX;
typedef short          LBCOLPOSINDEX, FAR *LPLBCOLPOSINDEX;
typedef int            LBCOLSCROLLPOS, FAR *LPLBCOLSCROLLPOS;
typedef FPGLOBALHANDLE LBCOLID, FAR *LPLBCOLID;
typedef short          LBGRPINDEX, FAR *LPLBGRPINDEX;
typedef int            LBGRPSCROLLPOS, FAR *LPLBGRPSCROLLPOS;
typedef FPGLOBALHANDLE LBGRPID, FAR *LPLBGRPID;
typedef long           LBJOINID, FAR *LPLBJOINID;
typedef FPGLOBALHANDLE LBJOINHANDLE, FAR *LPLBJOINHANDLE;
typedef LONG           LBROWINDEX, FAR *LPLBROWINDEX;
typedef WORD           LB_COLORID;
typedef short          LB_LISTITEMID, FAR *LPLB_LISTITEMID;
typedef short          LB_LINEITEMID, FAR *LPLB_LINEITEMID;

#define LBGRPID_DEF  (LBGRPID)-1
#define LBCOLID_DEF  (LBCOLID)-1

#define DIVUP(Var1, Var2)     (((Var1) + (Var2) - 1) / (Var2))
#define LB_COLSFROZEN(lpList) (lpList->nColsFrozenActual)
#define LB_GRPSFROZEN(lpList) (lpList->nGrpsFrozenActual)
#define LB_COLCOUNT(lpList)   (InSet(lpList->lStyle, LBS_MULTICOLUMN) ? \
                               LB_GetWrapColCount(lpList) : lpList->Cols.nCnt)
#define LB_ISSINGLESEL(lpList)(NotInSet(lpList->lStyle, LBS_EXTENDEDSEL) && \
                               NotInSet(lpList->lStyle, LBS_MULTIPLESEL))

#define LB_COLORFIX(clr) ((clr != RGBCOLOR_DEFAULT && (clr & 0x80000000L)) ? GetSysColor((short)(clr & 0x00FFFFFFL)) : clr)

#define LB_IsSort(lpList) (BOOL)((InSet(lpList->lStyle, LBS_SORT) || \
                          (lpList->Cols.nCnt && lpList->hSortSeq)) && \
                          (lpList->bSortState != LB_SORTSTATE_SUSPEND) && \
                          (!lpList->Virtual.fVirtualMode))

#define LB_GetRGBColor(fpid) (fpid ? fpCM_GetItem(fpid, 0, 0) : RGBCOLOR_DEFAULT)

#define LB_ColLevelHeight(lpCol) ((lpCol)->nColLevelHeight == -1 ? \
                                  lpList->ColDefault.nColLevelHeight : \
                                  (lpCol)->nColLevelHeight)

#define LB_GetFrozenOffset(lpList) (lpList->xFrozenOffset)
#define LB_RowIsSel(lpRow) ((lpRow)->bFlags & LB_ROW_SEL)
#define LB_RowIsMerge(lpRow) ((lpRow)->bFlags & (LB_ROW_MERGE_ALWAYS | \
                              LB_ROW_MERGE_RESTRICTED))
#define LB_RowIsMergeAlways(lpRow) ((lpRow)->bFlags & LB_ROW_MERGE_ALWAYS)
#define LB_RowIsMergeRestricted(lpRow) ((lpRow)->bFlags & \
                                        LB_ROW_MERGE_RESTRICTED)
#define LB_RowIsHidden(lpRow) ((lpRow)->bFlags & LB_ROW_HIDDEN)
#define LB_ColIDIsValid(idCol) (idCol && idCol != LBCOLID_DEF)
#define LB_RowsVisible(lpList) ((lpList->lRowBottom >= lpList->lTopItem ? \
                                 lpList->lRowBottom : LB_RowGetBottom(lpList)) - \
                                 lpList->lTopItem + 1)
#define LB_RowsAllVisible(lpList) ((lpList->lRowBottomAllVis >= lpList->lTopItem ? \
                                 lpList->lRowBottomAllVis : LB_RowGetBottomAllVis(lpList)) - \
                                 lpList->lTopItem + 1)
#define LB_IsAllRowsVisible(lpList) (lpList->lTopItem == 0 && lpList->lTotalItems <= \
                                     LB_RowGetBottomAllVis(lpList) + 1)
#define LB_RowGetYOffset(lpList, lItemNumFromTop) \
           LBx_RowGetYOffset(lpList, lpList->lTopItem, lItemNumFromTop)
#define LB_IsBorderOn(NCInfo) (((NCInfo).lStyle & FP_NCBORDER) && (NCInfo).bBorderSize > 0)
#define LB_DropDownOffset(lpCombo, lpList) ((LB_IsBorderOn(lpCombo->zNCInfoEdit) && LB_IsBorderOn(lpList->zNCInfo)) ? -1 : 0)

#define LB_AllowColResize(lpList) (WORD)(lpList->fModeDesigner ? LB_ALLOWCOLRESIZE_RESIZECOL : lpList->wAllowColResize)
#define LB_AllowColDragDrop(lpList) (WORD)(lpList->fModeDesigner ? LB_ALLOWCOLDRAGDROP_ALLCOLS : lpList->wAllowColDragDrop)
#define LB_AllowGrpResize(lpList) (WORD)(lpList->fModeDesigner ? LB_ALLOWGRPRESIZE_RESIZEGRP : lpList->wAllowGrpResize)
#define LB_AllowGrpDragDrop(lpList) (WORD)(lpList->fModeDesigner ? LB_ALLOWGRPDRAGDROP_ALLGRPS : lpList->wAllowGrpDragDrop)

#define LB_MODE_COLRESIZE   1
#define LB_MODE_COLDRAGDROP 2
#define LB_MODE_GRPRESIZE   3
#define LB_MODE_GRPDRAGDROP 4

#define LB_MODE_START  0
#define LB_MODE_UPDATE 1
#define LB_MODE_END    2

#define LB_GRPMOUSEOVER_HEADERONLY      0x01
#define LB_GRPMOUSEOVER_TOPLEVELONLY    0x02
#define LB_GRPMOUSEOVER_BOTTOMLEVELONLY 0x04

#define LB_SCROLLH_DEF_PIXELS 15
#define LB_SCROLLH_DEF_TWIPS  200

#define LB_VIRT_LINEUP           0
#define LB_VIRT_LINEDOWN         1
#define LB_VIRT_PAGEUP           2
#define LB_VIRT_PAGEDOWN         3
#define LB_VIRT_THUMBPOSITION    4
#define LB_VIRT_THUMBTRACK       5
#define LB_VIRT_TOP              6
#define LB_VIRT_BOTTOM           7
#define LB_VIRT_REFRESH          10

#define LB_ROW_SEL               0x01
#define LB_ROW_SELOLD            0x02
#define LB_ROW_MERGE_ALWAYS      0x04
#define LB_ROW_MERGE_RESTRICTED  0x08
#define LB_ROW_HIDDEN            0x10
#define LB_ROW_HEIGHT_MANUAL     0x20
#define LB_ROW_TEXTTIP           0x40

#define LB_VIRT_READAHEAD_ROWINC 2

#define LB_SEARCHBUFFER_MAX 50
#ifdef WIN32
#define LB_SBMAX            2147483647L
#else
#define LB_SBMAX            32767L
#endif
#define LB_ROWALLOCINC      100
#define LB_XOFFSET          1
#define FPFALSE             0

#define LB_GRP_MAX          256
#define LB_COL_MAX          256

#define LB_GRPHEADER        1
#define LB_COLHEADER        2

#define LB_FOCUSRECT_NOLEFT  0x01
#define LB_FOCUSRECT_NORIGHT 0x02

#ifdef FP_VB
#define FPTRUE -1
#else
#define FPTRUE 1
#endif

#define LBMx_ADJUSTHEIGHT          0x0600
#define LBMx_ADJUSTROWHEIGHT       0x0601
#define LBMx_VIRTREADAHEAD         0x0602
#define LBMx_FIREDATALOADED        0x0603
#define LBMx_VIRTON                0x0604
#define LBMx_SETHSCROLL            0x0605
#define LBMx_BUILDPOSLIST          0x0606
#define LBMx_ADJUSTCOLHEADERHEIGHT 0x0607
#define LBMx_ADJUSTGRPHEADERHEIGHT 0x0608
#define LBMx_SETVSCROLLRANGE       0x0609
#define LBMx_FIREDATAROWCHANGED    0x060a
#define LPMx_RESET                 0x060b
#define LBMx_GETMAXSIZE            0x060c
#define LBMx_RESIZE                0x060d
#define LBMx_GETCELLPOS            0x060e
#define LBMx_DESIGNERHWND          0x060f // sent by designer to indicate its hWnd
#define LBMx_GETOWNERHWND          0x0610
#define LBMx_SETOWNERHWND          0x0611
#define LBMx_SIZEDROPDOWN          0x0612
#define LBMx_GETCOLTEXT            0x0613
#define LBMx_GETCOLTEXTLEN         0x0614
#define LBMx_GETACTUALHEXTENT      0x0615
#define LBMx_FPDB_POSTMSG		       0x0616
#define LBMx_SETTEXTTIPFONT        0x0617

#define LBN_COLSEL                 2000
#define LBN_GRPSEL                 2001

#define LC_GETMAXSIZE_COL_HEADER_ROW -1
#define LC_GETMAXSIZE_GRP_HEADER_ROW -2

//#ifndef FP_OCX
//extern short dxPixelsPerInch;
//extern short dyPixelsPerInch;

//#define XTwipsToPixels(lTwips)  (short)((((double)(lTwips) * (double)dxPixelsPerInch) / 1440L) + 0.5)
//#define YTwipsToPixels(lTwips)  (short)((((double)(lTwips) * (double)dyPixelsPerInch) / 1440L) + 0.5)
//#define XPixelsToTwips(nPixels) (long)(((double)(nPixels) * 1440L / (double)dxPixelsPerInch) + 0.5)
//#define YPixelsToTwips(nPixels) (long)(((double)(nPixels) * 1440L / (double)dyPixelsPerInch) + 0.5)
//#endif

#if defined(FP_OCX) || defined(FP_VB)
#ifdef FP_OCX
#define FPXTwipsToPixels(lTwips)  XTwipsToPixels(lTwips)
#define FPYTwipsToPixels(lTwips)  YTwipsToPixels(lTwips)
#define FPXPixelsToTwips(nPixels) XPixelsToTwips(nPixels)
#define FPYPixelsToTwips(nPixels) YPixelsToTwips(nPixels)
#else
#define FPXTwipsToPixels(lTwips)  VBXTwipsToPixels(lTwips)
#define FPYTwipsToPixels(lTwips)  VBYTwipsToPixels(lTwips)
#define FPXPixelsToTwips(nPixels) VBXPixelsToTwips(nPixels)
#define FPYPixelsToTwips(nPixels) VBYPixelsToTwips(nPixels)
#endif //FP_OCX

#define XINIT(x) x
#define YINIT(y) y

#else
#define FPXTwipsToPixels(lTwips)  (short)(lTwips)
#define FPYTwipsToPixels(lTwips)  (short)(lTwips)
#define FPXPixelsToTwips(nPixels) (long)(nPixels)
#define FPYPixelsToTwips(nPixels) (long)(nPixels)

#define XINIT(x) XTwipsToPixels(x)
#define YINIT(y) YTwipsToPixels(y)
#endif

typedef struct tagLB_MAXSIZE
   {
   LBCOLINDEX nCol;
   LBROWINDEX lRow;
   WORD       wMethod;
   WORD       wUnits;
   } LB_MAXSIZE, FAR *LPLB_MAXSIZE;

typedef struct tagLB_CELLPOS
   {
   LBCOLINDEX nCol;
   LBROWINDEX lRow;
   RECT       Rect;
   } LB_CELLPOS, FAR *LPLB_CELLPOS;

/***************************
* FarPoint Buffer features
***************************/

typedef struct tagFP_HBUFFER
   {
   HPSTR        hpBuffer;
   GLOBALHANDLE hBuffer;
   long         lBufferLen;
   long         lAllocLen;
   long         lAllocInc;
   } FP_HBUFFER, FAR *LPFP_HBUFFER;

void  fpHBufferInit(LPFP_HBUFFER lpAllocBuffer, long nAllocInc);
void  fpHBufferFree(LPFP_HBUFFER lpAllocBuffer);
HPSTR fpHBufferAlloc(LPFP_HBUFFER lpAllocBuffer, HPSTR hpData, long lDataLen);

typedef struct tagLB_THREEDTEXT
   {
   BYTE      bStyle;
   FPCOLORID idDark;
   FPCOLORID idLight;
   short     nOffset;
   } LB_THREEDTEXT, FAR *LPLB_THREEDTEXT;

typedef struct tagLB_LINEITEM
   {
   FPCOLORID idLineColor;
   short     nLineWidth;
   FPCOLORID id3DLight;
   FPCOLORID id3DDark;
   short     n3DWidth;
   BYTE      bStyle;
   } LB_LINEITEM, FAR *LPLB_LINEITEM;

typedef struct tagLB_LINE
   {
   LB_LINEITEMID idLineItemDef;
   LB_LINEITEMID idLineItemRows;
   LB_LINEITEMID idLineItemCols;
   LB_LINEITEMID idLineItemColsH;
   } LB_LINE, FAR *LPLB_LINE;

typedef struct tagLB_LISTITEM
   {
   LB_LINE       Line;
   FPCOLORID     idBackColor;
   FPCOLORID     idForeColor;
   BYTE          bOrientation;
   BYTE          bAlignH;
   BYTE          bAlignV;
   BYTE          bPictAlignH;
   BYTE          bPictAlignV;
   BYTE          bPictStyle;
   BYTE          bMultiLine;
   FPPICTID      idPict;
   FPPICTID      idPictSel;
   LB_THREEDTEXT Text3D;
   FPFONT        fpFont;
#ifdef LP30
// enhancement SEL8276
   BYTE			  bTextEllipses;
// enhancement SEL8276
#endif
   } LB_LISTITEM, FAR *LPLB_LISTITEM;

typedef struct tagLB_HEADER
   {
   BOOL  fShow;
   short nHeight;
   short nHeightInside;
   short nHeightOutside;
   short nHeightTotal;
   } LB_HEADER, FAR *LPLB_HEADER;

typedef struct tagFP_TABLEMAN
   {
   FPGLOBALHANDLE hItems;              // Table of elements
   BYTE           nSize;               // Size of each array element
   short          nCnt;                // Number of elements
   short          nAllocCnt;           // Number of allocated elements
   } FP_TABLEMAN, FAR *LPFP_TABLEMAN;

typedef struct tagLB_GRP
   {
   LBGRPID        idGrpParent;
   FP_TABLEMAN    ColChildren;
   FP_TABLEMAN    GrpChildren;
   LB_LISTITEMID  idListItem;
   LB_LISTITEMID  idListItemHeader;
   FPGLOBALHANDLE hHeaderText;
   float          Width;
   short          nWidthInPixels;
   char           fHide;
   char           fLockResize;
   long           xOffset;
   short          nLevel;
   short          nChildLevelCount;
   FPGLOBALHANDLE hGrpName;
   long           lGrpID;
   } LB_GRP, FAR *LPLB_GRP;

typedef struct tagLB_COL
   {
   LBGRPID        idGrpParent;
   LB_LISTITEMID  idListItem;
   LB_LISTITEMID  idListItemHeader;
   FPGLOBALHANDLE hFieldName;
   FPGLOBALHANDLE hFormat;
   FPGLOBALHANDLE hHeaderText;
   LBCOLINDEX     nColCellIndex;
   short          nFieldNum;
   BYTE           bFieldType;
   BYTE           bColSortDataType;
   BYTE           bColSorted;
   BYTE           bColMerge;
   short          nColSortSequence;
   short          nColLevel;
   short          nColLevelHeight;
   float          Width;
   short          nWidthInPixels;
   char           fHide;
   char           fLockResize;
   long           xOffset;
   FPGLOBALHANDLE hColName;
   long           lColID;
   } LB_COL, FAR *LPLB_COL;

#define LB_CELLFLAGS_STRING   0x00
#define LB_CELLFLAGS_LONG     0x01
#define LB_CELLFLAGS_DOUBLE   0x02
#define LB_CELLFLAGS_DATATYPE (LB_CELLFLAGS_LONG | LB_CELLFLAGS_DOUBLE)
#define LB_CELLFLAGS_JOIN     0x10
#define LB_CELLFLAGS_TEXTTIP  0x20
#define LB_CELLFLAGS_MERGED   0x40  // Used temporarily when painting

typedef struct tagLB_CELL
   {
   BYTE           bFlags;
   FPGLOBALHANDLE hData;               // Our allocated string
   LB_LISTITEMID  idListItem;
   } LB_CELL, FAR *LPLB_CELL;

typedef struct tagLB_JOIN
   {
   LBJOINID idJoin;
   long     lRefCnt;
   LB_CELL  Cell;
   } LB_JOIN, FAR *LPLB_JOIN;

typedef struct tagLB_BLOCK
   {
   LBCOLINDEX nColFirst;
   LBCOLINDEX nColLast;
   LBROWINDEX lRowFirst;
   LBROWINDEX lRowLast;
   } LB_BLOCK, FAR *LPLB_BLOCK;

typedef struct tagLB_CELLCOORD
   {
   LBCOLINDEX nCol;
   LBROWINDEX lRow;
   } LB_CELLCOORD, FAR *LPLB_CELLCOORD;

typedef struct tagLB_ITEM
   {
   FPGLOBALHANDLE hItem;               // Our allocated string table
   short          nAllocCnt;
   } LB_ITEM, FAR *LPLB_ITEM;

typedef struct tagLB_ROW
   {
   BYTE           bFlags;
                     // LB_ROW_SEL               0x01
                     // LB_ROW_SELOLD            0x02
                     // LB_ROW_MERGE_ALWAYS      0x04
                     // LB_ROW_MERGE_RESTRICTED  0x08
                     // LB_ROW_HIDDEN            0x10
#if 0
   BYTE           bRowHeightAutoSize;
#endif
   long           lRowHeight;
   LB_LISTITEMID  idListItem;
   LB_ITEM        Item;
   DWORD          dwItemData;          // User defined item data
#if defined(FP_VB) || defined(FP_OCX)
   HLSTR          hlstrBookmark;
#endif
#ifdef LP30
   BYTE			  fHide;
#endif
   } LB_ROW, far *LPLB_ROW;

typedef struct tagROWTABLE
   {
   LBROWINDEX     lItemAllocCnt;
   LBROWINDEX     lTableItemCnt;
   short          nRowsPerTableItem;
   FPGLOBALHANDLE hRowTable;
   } ROWTABLE, far *LPROWTABLE;

typedef struct tagLB_RESIZE
   {
   long    lResizeCurrentPos;
   short   nResizeOffset;
   long    lResizeStartPos;
   long    lResizeMinPos;
   long    lResizeMaxPos;
   LBCOLID idResize;
   } LB_RESIZE, FAR *LPLB_RESIZE;

typedef struct tagLB_DRAGDROP
   {
// HWND    hWndDragDropPopup;
   LBCOLID idItem;
   short   nTop;
   short   nLevelHeight;
   short   nLevelCount;
   short   nItemWidth;
   short   xMouseOffset;
   short   xLeftCurrent;
   short   nLevelCurrent;
   } LB_DRAGDROP, FAR *LPLB_DRAGDROP;

typedef struct tagLB_ALLOCBUFFER
   {
   LPSTR          lpBuffer;
   FPGLOBALHANDLE hBuffer;
   short          nBufferLen;
   short          nAllocLen;
   } LB_ALLOCBUFFER, FAR *LPLB_ALLOCBUFFER;

typedef struct tagLB_SEARCH
   {
   BYTE  bSearchType;
   BYTE  bSorted;
   BYTE  bSortDataType;
   short nCol;
   } LB_SEARCH, FAR *LPLB_SEARCH;

typedef struct tagLB_BOUND
   {
   FPCONTROL fpCtrl;
   HCTL      hCtlDataControl;
   HCTL      hCtlDataControlConnected;
   HWND      hWndDataControl;
   BOOL      fInternalSetBookmark;
   BOOL      fBoundActive;
   BOOL      fBookmarkable;
   BOOL      fAutoFill;
   BOOL      fAutoHeadings;
   BOOL      fIgnoreMoveNext;
   BOOL      fIgnoreSetCurrentRecord;
   WORD      wDataSync;
   WORD      wDataAutoSizeCols;
   BYTE      bFieldType;
   short     nFieldCount;
   short     nFieldNum;
   BOOL      fChangeMade;
   WORD      wPropDataField;
   LPSTR     lpszPropDataField;
   HLSTR     hlstrPrevCurrentRecord;
   } LB_BOUND, FAR *LPLB_BOUND;

typedef struct tagLB_VIRTUAL
   {
   BOOL fVirtualMode;
   long lVRowCount;
   long lPageSize;
   long lPagesAhead;
   char fBOF;                          // Internal
   char fReadAheadMsgSent;             // Internal
   char ReadDirection;                 // Internal
   } LB_VIRTUAL, FAR *LPLB_VIRTUAL;

#define LB_BUFFER_ALLOC_CNT 100

typedef union tagLB_HSCROLLITEM
   {
   LBCOLPOSINDEX nCol;              // Used for columns
   LBGRPID       idGrp;             // Used for groups
   } LB_HSCROLLITEM, FAR *LPLB_HSCROLLITEM;

typedef struct tagLB_PALTABLE
   {
   RECT     Rect;
   HPALETTE hPal;
   } LB_PALTABLE, FAR *LPLB_PALTABLE;

typedef struct tagLISTBOX
   {
   LB_BOUND       Bound;
   FPCONTROL      fpCtrl;
   HANDLE         fpInst;
   BOOL           fDesignTime;
   BOOL           fCombo;
   BOOL           fIgnoreSetFont;
   BYTE           bCtlType;
   HANDLE         hInstance;           // the instance of the owner
   HWND           hWndParent;          // the owner of this list box
   HWND           hWnd;                // Window handle of this list box
   HWND           hWndOwner;           // Window handle of Owner
   HWND           hWndOwnerOrig;       // Window handle of original Owner
   WORD           wWndId;              // Window ID of this list box
   ROWTABLE       RowTable;            // Table to hold all the row items
   RECT           Rect;                // Inside rectangle of the list box
   RECT           RectClient;          // Inside client rectangle of list box
   RECT           RectOrig;            // Original Rectangle
   long           lFirstSelectedItem;  // Currently selected item
   long           lExtendedItemBase;   // Base item for extended select
   long           lRectDotPos;         // current position of the dotted rectangle
   long           lTopItem;            // The top item on the list box
   long           lTopItemOld;         // The Old top item on the list box
   LONG           lStyle;              // the style of the list box
   long           lVScrollPos;         // Vertical thumb position
   long           lVScrollMax;         // Vertical max pos
   int            iHScrollPos;         // Horizontal thumb position
   int            iHScrollMin;         // Horizontal thumb min
   int            iHScrollMax;         // Horizontal thumb max
   long           lRowsVisDef;         // Number that can be displayed
   long           lTotalItems;         // total number of items in the listbox
   long           lItemHeightDef;      // item height in logical pixels
   WORD           wHorzExtent;         // Range of Horz Scroll in Pixel
   WORD           wItemState;          // current state of this item
   WORD           wItemAction;         // current action
   WORD           wColumnWidth;        // Column width (in logical pixel)
   short          xDblClkPos;          // last position of the mouse in the X axis
   long           lDblClkItem;
   WORD           wTabStop;
   LB_LISTITEM    ListItemDef;
   LB_LISTITEM    ListItemRows;
   LB_LISTITEM    ListItemSelRows;
   LB_LISTITEM    ListItemOddRows;
   LB_LISTITEM    ListItemEvenRows;
   FPNCINFO       zNCInfo;
   char           szSearchBuffer[LB_SEARCHBUFFER_MAX + 1];
   LB_COL         ColDefault;
   LB_HEADER      ColHeaders;
   LB_GRP         GrpDefault;
   LB_HEADER      GrpHeaders;
   BOOL           fInvertSelection;
   short          nHeaderHeightInPixels;
   short          dHScrollHeight;
   short          dVScrollWidth;
   HWND           hWndHScroll;
   HWND           hWndVScroll;
   FPGLOBALHANDLE hSortSeq;
   FPGLOBALHANDLE hSearchText;
   short          nSortSeqCnt;
   short          dSeparatorChar;
   WORD           wColumnWidthScale;
   short          nAveCharWidth;
   short          nMaxCharWidth;
   short          nPixelsPerInch;
   long           lSelMax;
   long           lSelCount;
   long           lRowHeight;
   BYTE           bRowHeightAutoSize;
   BYTE           bScrollHScale;
   short          nScrollHInc;
   short          nColsFrozen;
   short          nColsFrozenActual;
   short          nGrpsFrozen;
   short          nGrpsFrozenActual;
   LBCOLID        idColCur;
   LBGRPID        idGrpCur;
   long           lRow;
   long           lNewIndex;
   long           lSearchIndex;
   BOOL           fDrawFocusRect;
   BYTE           bScrollBarV;
   BYTE           bScrollBarH;
   WORD           wHighestPrecedence;
   WORD           wAllowColResize;
   WORD           wAllowColDragDrop;
   WORD           wAllowGrpResize;
   WORD           wAllowGrpDragDrop;
   BOOL           fReadOnly;
   HCURSOR        hCursorCurrent;
   WORD           wCurrentMode;
   LB_RESIZE      Resize;
   LB_DRAGDROP    DragDrop;
   short          nRowPictureCol;
   short          nColumnSearch;
   BOOL           fUseSpecialVScroll;
   BYTE           bSpecialVScrollOptions;
   BOOL           fEnableKeyEvents;
   BOOL           fEnableMouseEvents;
   BOOL           fEnableTopChangeEvent;
   BYTE           fExtSelected;
   BYTE           bSortState;
   BYTE           bSearchMethod;
   BYTE           bAutoSearch;
   BYTE           bExtendCol;
   BYTE           bExtendRow;
   long           lWrapWidth;
   FARPROC        lpfnParentProc;      // Used by the combo box
   BOOL           fWMSizeInternal;
   short          nNCTopMargin;
   short          nNCBottomMargin;
   short          nXtraNew;
   short          nColumnLevels;
   unsigned       fVB                        : 1;
   unsigned       fMouseCaptureOn            : 1;
   unsigned       fShiftDown                 : 1;
   unsigned       fControlDown               : 1;
   unsigned       fHScrollVisible            : 1;
   unsigned       fVScrollVisible            : 1;
   unsigned       fSentFireDataLoaded        : 1;
   unsigned       fDisableSpaceBar           : 1;
   unsigned       fLButtonDownReceived       : 1;
   unsigned       fAdjustHeightSent          : 1;
   unsigned       fAdjustRowHeightSent       : 1;
   unsigned       fAdjustColHeaderHeightSent : 1;
   unsigned       fAdjustGrpHeaderHeightSent : 1;
   unsigned       fSetHScrollSent            : 1;
   unsigned       fSetVScrollRangeSent       : 1;
   unsigned       fBuildPosListSent          : 1;
   unsigned       fInvalidatedAll            : 1;
   unsigned       fIgnoreAdjustChildrenWidth : 1;
   unsigned       fColFirstDrawn             : 1; // Paint temporary variable
   unsigned       fIsVisible                 : 1;
   unsigned       fIgnoreOldSel              : 1;
   unsigned       fSetIgnoreOldSel           : 1;
   unsigned       fModeDesigner              : 1;
   unsigned       fHasPalette                : 1;
   unsigned       fNoSendClick               : 1;
   unsigned       fVirtualSearchSent         : 1;
   unsigned       fVirtOnSent                : 1;
   unsigned       fResizingComboList         : 1;
   unsigned       fIgnoreClickEvent          : 1;
   unsigned       fLoading                   : 1;
   unsigned       fLoadingProps              : 1;
   LB_VIRTUAL     Virtual;
   char           cDecimalSign;
   char           cThousandsSeparator;  // fix for GIC7719
   BYTE           bLineApplyTo;
   BYTE           bListApplyTo;
   FP_TABLEMAN    Cols;
   FP_TABLEMAN    Grps;
   FP_TABLEMAN    ColsPos;
   FP_TABLEMAN    HScrollPosList;       // List of LB_HSCROLLITEM's
   FP_TABLEMAN    JoinTable;
   FP_TABLEMAN    BlockDrawTable;
   short          nRowHeightInside;
   short          nRowHeightOutside;
   short          xFrozenOffset;
   long           lGrpTotalWidth;
   BOOL           fMergeAdjustView;
   LBCOLINDEX     nColMergeCnt;
   LBROWINDEX     lRowMergeCnt;
   BYTE           bTextTip;
   BYTE           bTextTipMultiLine;
   BYTE           bTextTipPos;
   long           lTextTipDelay;
   FPCOLORID      idClrListGrayArea;
   HWND           hWndTextTip;
   LBROWINDEX     lRowBottom;
   LBROWINDEX     lRowBottomAllVis;
   LBROWINDEX     lNextSel;
   BYTE           bRowHeightsEqual;
   BYTE           xFocusRectOrig;
   FP_HBUFFER     PalTable;
   RECT           RectPaint;
   BOOL           fOnDraw;
   SCALEMODE      scalemode;
   LB_BLOCK       PaintMerged;
   FPPICTID       idMouseIcon;

#ifdef FP_DLL
   UINT           helpContextId;
#endif   
#ifdef LP30
// enhancement GIC6646
   BOOL			  fEnableClickEvent;
   short		  nExtSelF8Mode;	// this variable will be 0 (off), 1 (show focus rect), or -1 (hide focus rect)
									// it is for the keyboard interface for extended select mode when Shift-F8 is pressed -scl
   DWORD		  dwNextScrollTickCount;	// stores tick count when next scrolling should happen when mouse is captured 
											// (used to implement delayed scrolling) -scl
   BYTE			  fNoRedraw;
   BYTE			  fResizeRowToFont;
   BOOL			  fSettingFontEmpty;
   FP_TT_INFO       TT_Info;               // TextTip
   FPFONT			fpFontTT;			// font id for text tips.  will use cell font if 0 -scl
//   BYTE				bTextTipMultiline;
   BOOL             fLoadTextTip;
   long			  lRowMouseOverLast;
   LBGRPID        idDragParentGrp;
   short			nDragPosInParent;
   BOOL       fSetTextTipFont;
   BOOL       fAnimating;
   BOOL       fCalcRectDone;
#endif
   } LISTBOX, FAR *LPLISTBOX;

#ifdef LP30
#define LB_EXTSEL_TIMER_ID				147		// id for timer used to flash focus rect for special Shift-F8 extended select keyboard interface
#define LB_AUTOSEARCHFILL_TIMER_ID		148		// id for timer used to flash focus rect for special Shift-F8 extended select keyboard interface
#define LB_EXTSEL_TIMER_DELAY			500		
#define LB_AUTOSEARCHFILL_TIMER_DELAY	500		
#endif

typedef struct tagCTL_STRUCT
   {
   LPVOID lpCtl;
   int ctlType;
   } CTL_STRUCT, FAR *LPCTL_STRUCT;

typedef enum { FP_LEFT_BUTTON, FP_MIDDLE_BUTTON, FP_RIGHT_BUTTON } FP_MOUSE_BUTTON;


#define MAX_DLGCAPTION_LENGTH     90
#define MAX_DLGITEMCLASS_LENGTH   30
#define MAX_DLGITEMTEXT_LENGTH    60

//
// Internal Constants
//
#define  ODA_NOFOCUS       0x0004
#define  MK_NOSELECT       0x1000

extern LONG      fpObjectCount;
extern OMEM_POOL fpStringPool;
extern HANDLE    hInstance;
extern PROPINFO  PropNULL;
extern HCURSOR   hCursorResize;
extern HCURSOR   hCursorDragDrop;
extern HBRUSH    hBrushFocusRect;

/**********************
* Function Prototypes
**********************/

// dragdrop.c

void LB_DragDrop(LPLISTBOX lpList, WORD wMode, long lPos);
void LB_DragDropInvert(LPLISTBOX lpList, HDC hDC);
void LB_RegisterDragDropPopup(HANDLE hInst);

// libmain.c

// SCP 10/7/98 - These are now defined by fptools.h
//extern void DLLENTRY fpDestroy();
//extern void DLLENTRY fpCreate();

BOOL                 LB_CheckRuntimeLicense(LPSTR lpszName);

WORD fpSplashIsExpired(HINSTANCE);

#define FPSPLASH_NONE    0
#define FPSPLASH_OK      1
#define FPSPLASH_EXPIRED 2

BOOL FAR PASCAL LBInit(HANDLE hInst);
BOOL FAR PASCAL UnLoadLPLib();

// props.c

long PASCAL List_SetProp(LPLISTBOX lpList, WORD wProp, long lParam);
long PASCAL List_GetProp(LPLISTBOX lpList, WORD wProp, long lParam);
long LB_ColsList(LPLISTBOX lpList, WORD wProp, long lParam, BOOL fGetProp);
long LB_Cols(LPLISTBOX lpList, WORD wProp, long lParam, BOOL fGetProp,
             LBCOLID idCol);
long LB_PropGrp(LPLISTBOX lpList, WORD wProp, long lParam, BOOL fGetProp);
long LB_ColText(LPLISTBOX lpList, long lRow, long lParam, BOOL fGetProp);
long LBx_ProcessText(long lParam, BOOL fGetProp, LPFPGLOBALHANDLE lphText);
long LB_ProcessNCProps(LPFPNCINFO lpNCInfo, WORD wProp, long lParam,
                       BOOL fGetProp);
int  TranslateSortStyle( long lStyle);                       
long LB_SaveProp(LPLISTBOX lpList, WORD wProp, long lParam);
long LB_LoadProp(LPLISTBOX lpList, WORD wProp, long lParam);

// search.c

LONG LB_FindString(LPLISTBOX lpList, LONG lStart, LPSTR lpszText,
                   BYTE bSearchType, short nColumnSearch);
void LB_StringToFloat(LPSTR nptr, LPDOUBLE lpdfVal);
LONG LB_StringToLong(LPSTR lpszStr);

// sort.c

int DLLENTRY StrCompare(LPLISTBOX lpList, LPLB_COMPAREITEMSTRUCT lpComp);
BOOL         LB_BuildSortSeq(LPLISTBOX lpList);
void         LB_ReSort(LPLISTBOX lpList);

#if defined(LP30)
// datecomp.c
int LB_SortCompareDates(LPSTR lpsz1, LPSTR lpsz2);
LPSTR LB_SysAllocBSTRFromTString(LPSTR lpszInput);
#endif

// list.c
BOOL                LB_Register(HANDLE hInstance, LPSTR lpszClassName);
BOOL                LB_Unregister(HANDLE hInstance, LPSTR lpszClassName);
LONG WINAPI _export ListProc(HWND hWnd, UINT Msg, WPARAM wParam,
                             LPARAM lParam);
LONG                ListMain(LPLISTBOX lpList, HWND hWnd, UINT Msg,
                             WPARAM wParam, LPARAM lParam, LPBOOL lpfCallDef);
LBCOLPOSINDEX       LB_GetLeftCol(LPLISTBOX lpList, LPSHORT lpxPos);
LBCOLPOSINDEX       LBx_GetLeftCol(LPLISTBOX lpList, LPSHORT lpxPos,
                                   short nColCount);
LPLISTBOX           LB_Lock(HWND hWnd);
#define             LB_Unlock(hWnd)
void                LB_Invalidate(LPLISTBOX lpList);
void                LB_WM_ShowWindow(LPLISTBOX lpList, WPARAM wParam);
void                LB_GetMonitorRect(LPRECT lprcCtl, LPRECT lprcMonitor);

// wrlist2.c
FPGLOBALHANDLE      ProcessWMCreate(HWND hWnd, LPCREATESTRUCT lParam);
BOOL                LBx_Create(HWND hWnd, LPCREATESTRUCT lParam,
                               LPLISTBOX lpList);
void                ProcessWMDestroy(HWND hWnd, FPGLOBALHANDLE hList);
void                LBx_Destroy(HWND hWnd, LPLISTBOX lpList);
BOOL                DeleteOwnerItem(LPLISTBOX lpList, LONG wItemIndex,
                                    LONG lItem);
BOOL                LB_ResetRows(LPLISTBOX lpList, BOOL fResetScrollBars);
LONG                DrvListAddString(LPLISTBOX lpList, long lParam);
long                LB_InsertItemInList(LPLISTBOX lpList, LPLB_ITEM lpItemNew,
                                        BOOL fSort, LPVOID hlstrBookmark);
long                LB_UpdateItemInList(LPLISTBOX lpList, LPLB_ITEM lpItem,
                                        long lRow, BOOL fSort);
void                LB_SetScrollRange(LPLISTBOX lpList, short nBar, BOOL fOn,
                                      BOOL fRedraw);
void                LB_SetScrollPos(LPLISTBOX lpList, short nBar, BOOL fRedraw);
void                LB_UpdateScrollBar(LPLISTBOX lpList, BOOL fHorz,
                                       BOOL fVert, BOOL fOn, BOOL fRedraw);
void                LB_MoveScrollBar(LPLISTBOX lpList, LPRECT lpClientRect,
                                     short nBar);
long                LB_GetCaretIndex(LPLISTBOX lpList);
BOOL                LB_AllocRow(LPLISTBOX lpList, LBROWINDEX Row,
                                LBROWINDEX RowCnt);
BOOL                LB_DeAllocRow(LPLISTBOX lpList, LBROWINDEX Row,
                                  LBROWINDEX RowCnt);
LPLB_ROW            LB_LockRow(LPLISTBOX lpList, LBROWINDEX Row);
#define             LB_UnlockRow(lpList, Row) TRUE
void                LB_FreeRow(LPLISTBOX lpList, long lRow);
void                LB_FreeRowData(LPLISTBOX lpList, long lRow, LPLB_ITEM lpItem);
BOOL                LB_CreateRowItem(LPLISTBOX lpList, LPLB_ITEM lpItem,
                                     long lItem);
BOOL                LB_AllocRowItem(LPLISTBOX lpList, LPLB_ITEM lpItem,
                                    short nCnt);
FPGLOBALHANDLE      LB_BuildRowString(LPLISTBOX lpList, LPLB_ROW lpRow,
                                      BOOL fCreateString, LPBOOL lpfStringCreated,
                                      char cSeparator);
BOOL                LB_BuildRowFromString(LPLISTBOX lpList, long lRow,
                                          LPLB_ROW lpRow, long lString);
BOOL                LB_UpdateCellData(LPLISTBOX lpList, long lRow,
                                      LPLB_ROW lpRow,  LBCOLID idCol,
                                      long lString);
LPSTR               LB_AllocBuffer(LPLB_ALLOCBUFFER lpAllocBuffer, LPSTR lpData,
                                   short nDataLen);
void                LB_MoveRow(LPLISTBOX lpList, long lRowSrc, long lRowDest);
void                LB_DeleteRow(LPLISTBOX lpList, long lRow, long lRowCnt);
void                LB_InsertRow(LPLISTBOX lpList, long lRowInsert,
                                 long lRowCnt);
void                fpRefreshNC(HWND hWnd, LPFPNCINFO lpNCInfo);
void                LB_GlobalFree(LPFPGLOBALHANDLE lphData);


/* paint.c */
void  DrawCaret(LPLISTBOX lpList, BOOL fOn);
void  LB_ResetContent(LPLISTBOX lpList);
void  ProcessWMPaint(LPLISTBOX lpList, WPARAM wParam);
void  LB_DrawItem(LPLISTBOX lpList, long nItemNum, BOOL fErase);
BOOL  LB_PaintText(LPLISTBOX lpList, HDC hDC, LPRECT lpRect, LPSTR lpText,
                   short Len, LPLB_LISTITEM lpListItem);
void  fpDrawBox(HDC hDC, LPRECT lpRect, LPLB_LINEITEM lpLineItemV,
                LPLB_LINEITEM lpLineItemTop, LPLB_LINEITEM lpLineItemBottom);

/* wrlist4.c */
LONG  LB_InsertString(LPLISTBOX lpList, LONG wParam, LONG lParam);
void  ProcessWMSetFocus(LPLISTBOX lpList);
void  ProcessWMKillFocus(LPLISTBOX lpList);
int   DLLENTRY ItemCompare(LPLISTBOX lpList, LPLB_COMPAREITEMSTRUCT lpComp);
long  LB_BinaryInsert(LPLISTBOX lpList, register LONG Num, int (FAR PASCAL *lpCompare)(LPVOID, LPVOID));
void  ProcessWMEnable(LPLISTBOX lpList, WORD wParam);
/* (string.c) */
#if defined(LP30) && !defined(FP_DLL)
// enhancement CHB216 -scl
// adding support for ColFormat when control is not bound
BOOL LB_FormatString(LPLISTBOX lpList, LPFPGLOBALHANDLE hString, FPGLOBALHANDLE hFormat);
// enhancement CHB216 -scl
#endif

/* wrlist5.c */
LONG  DrvListGetCurSel(LPLISTBOX lpList);
LONG  DrvListGetSelCount(LPLISTBOX lpList, WORD wParam, LONG lParam);
BOOL  ProcessWMVerticalScroll(LPLISTBOX lpList, WPARAM wParam, LONG lParam);
BOOL  ProcessWMHorizontalScroll(LPLISTBOX lpList, WORD wScrollCode, LONG lPos);
BOOL  LB_HScrollInc(LPLISTBOX lpList, int iHScrollInc);
void  ProcessWMKeyDown(LPLISTBOX lpList, WORD wParam, LONG lParam);
BOOL  LB_ActivateItem(LPLISTBOX lpList, long lIndex, BOOL fPartial,
                      BOOL fSendClick);
short LB_GetHScrollInc(LPLISTBOX lpList);
BOOL  LB_SetRowSelState(LPLISTBOX lpList, LPLB_ROW lpRow, BOOL fSel);

/* wrlist6.c */
LONG  LB_GetText(LPLISTBOX lpList, LONG wParam, LONG lParam);
LONG  LB_GetTextLen(LPLISTBOX lpList, LONG wParam, LONG lParam);
LONG  LB_GetTopIndex(LPLISTBOX lpList);
LONG  LB_SelectString(LPLISTBOX lpList, LONG wParam, LONG lParam);
LONG  LB_SetCurSel(LPLISTBOX lpList, LONG wParam);
void  ProcessWMChar(LPLISTBOX lpList, WORD wParam, LONG lParam);
void  ProcessWMGetDlgCode(LPLISTBOX lpList, WORD wParam);
void  ProcessWMMouseMove(LPLISTBOX lpList, WORD wParam, LONG lParam);
void  ProcessWMDoubleClick(LPLISTBOX lpList, FP_MOUSE_BUTTON which, WPARAM wParam, LONG lParam);
void  LB_ClearSearchBuffer(LPLISTBOX lpList);

/* wrlist7.c */
long  LB_SetFont(LPLISTBOX lpList, HFONT hFont);
void  LB_PerformAdjustHeight(LPLISTBOX lpList);
void  LB_AdjustRowHeight(LPLISTBOX lpList);
void  LB_PerformAdjustRowHeight(LPLISTBOX lpList);
void  LB_AdjustColHeaderHeight(LPLISTBOX lpList);
void  LB_PerformAdjustColHeaderHeight(LPLISTBOX lpList);
void  LB_AdjustGrpHeaderHeight(LPLISTBOX lpList);
void  LB_PerformAdjustGrpHeaderHeight(LPLISTBOX lpList);
BOOL  LB_AdjustVisibleRows(LPLISTBOX lpList);
short LB_GetHRowOffsets(LPLISTBOX lpList, LPSHORT lpTop, LPSHORT lpBottom);
HFONT LB_GetFont(LPLISTBOX lpList);
void  CalcNewWindowSize(LPLISTBOX lpList, BOOL fDraw);
BOOL  DrvListWMSize(LPLISTBOX lpList, WORD wParam, LONG lParam);
int   DrvListWMGetText(LPLISTBOX lpList, WORD wParam, LONG lParam);
int   DrvListWMSetText(LPLISTBOX lpList, WORD wParam, LONG lParam);
LONG  LB_GetTextExtent(LPLISTBOX lpList, LONG wParam, LONG lParam);
DWORD DLLENTRY GetTabTextExtent(HDC hDC, LPSTR szStr, int iTab);
int   ProcessWMLBDIR(LPLISTBOX lpList, WORD wParam, LONG lParam);
short LB_GetFontHeight(LPLB_LISTITEM lpListItem);

/* wrlist8.c */
void  ToggleSingleSelection(LPLISTBOX lpList, LONG wIndex);
void  ToggleExtendedSelection(LPLISTBOX lpList, WORD wParam, LONG wIndex);
void  ToggleMultipleSelection(LPLISTBOX lpList, WORD wParam, LONG wIndex);
LONG  LB_DeleteString(LPLISTBOX lpList, register LONG wParam, LONG lParam);
BOOL  CompareOwnerItem(LPLISTBOX lpList, LONG wIndex1, LPVOID lpItem1, LONG wIndex2, LPVOID lpItem2);
LONG  ProcessGetDBLClkHPos(LPLISTBOX lpList, WORD wParam, LONG lParam);
LONG  ProcessGetDBLClkItem(LPLISTBOX lpList, WORD wParam, LONG lParam);
void  DrvListSetReDraw(LPLISTBOX lpList, WORD wParam);
void  ProcessWMLeftButtonDown(LPLISTBOX lpList, WORD wParam, LONG lParam);
int   DrvListSetColumnWidth(LPLISTBOX lpList, WORD wParam, LONG lParam);
BOOL  DrvListSetTabStops(LPLISTBOX lpList, WORD wParam, LONG lParam);
LONG  DrvListSetSel(LPLISTBOX lpList, WORD wParam, LONG lParam, LONG lParam2);
LONG  LB_GetSel(LPLISTBOX lpList, LONG wParam);
short LB_IsMouseInColResize(LPLISTBOX lpList, short xMouse, short yMouse,
                            LPSHORT lpnCol);
short LB_IsMouseInGrpResize(LPLISTBOX lpList, short xMouse, short yMouse,
                            LPLBGRPID lpidGrp);
long  LB_GetItemFromMouse(LPLISTBOX lpList, long lParam, LPSHORT lpxPos,
                          LPSHORT lpyPos);
BOOL  LB_GetItemRect(LPLISTBOX lpList, long lIndex, LPRECT lpRect);
BOOL  LB_IsItemOnScreen(LPLISTBOX lpList, long lIndex, BOOL fPartial);

/* wrlist9.c */
void  ProcessWMLeftButtonUp(LPLISTBOX lpList, WORD wParam, LONG lParam);
void  LB_Resize(LPLISTBOX lpList, WORD wMode, long lPos);
BOOL  LB_FocusRect(LPLISTBOX lpList, HDC hDC, LPRECT lpRect, BYTE bFlags);
int   DrvListSetHorzExt(LPLISTBOX lpList, WORD wParam, LONG lParam);
void  LB_SetHScroll(LPLISTBOX lpList);
void  LB_PerformSetHScroll(LPLISTBOX lpList);
int   DrvListGetHorzExt(LPLISTBOX lpList, WORD wParam, LONG lParam);
LONG  DrvListGetSelItems(LPLISTBOX lpList, WORD wParam, LONG lParam);
void  LB_SetTopIndex(LPLISTBOX lpList, LONG wParam);
void  PaintListBox(LPLISTBOX lpList, LPRECT lpRect, BOOL fErase);
long  LB_SetCaretIndex(LPLISTBOX lpList, long lIndex, BOOL fScroll);
BOOL  LB_SetScrollHScale(LPLISTBOX lpList, BYTE bScrollHScale);
BOOL  LB_GetScrollHScale(LPLISTBOX lpList, LPBYTE lpbScrollHScale);
BOOL  LB_SetScrollHInc(LPLISTBOX lpList, short nScrollHInc);
BOOL  LB_GetScrollHInc(LPLISTBOX lpList, LPSHORT lpnScrollHInc);
BOOL  LB_SetColsFrozen(LPLISTBOX lpList, short nColsFrozen);
BOOL  LB_GetColsFrozen(LPLISTBOX lpList, LPSHORT lpnColsFrozen);
BOOL  LB_SetItemData(LPLISTBOX lpList, long lRow, DWORD dwItemData);
BOOL  LB_GetItemData(LPLISTBOX lpList, long lRow, LPDWORD lpdwItemData);
BOOL  LB_VScrollSetSpecial(LPLISTBOX lpList, BOOL fUseSpecialVScroll,
                           WORD wOptions);
BOOL  LB_VScrollGetSpecial(LPLISTBOX lpList, LPWORD lpwOptions);
long  LB_SendMessage(LPLISTBOX lpList, WORD Msg, WPARAM wParam, LPARAM lParam);
long  LB_PostMessage(LPLISTBOX lpList, WORD Msg, WPARAM wParam, LPARAM lParam);
long  LB_SendCmdMsg(LPLISTBOX lpList, WORD id, HWND hwndCtl, WORD code);
long  LB_PostCmdMsg(LPLISTBOX lpList, WORD id, HWND hwndCtl, WORD code);

// fpncprop.c

long FP_Prop3DInsideStyle(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_Prop3DOutsideStyle(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_Prop3DInsideHighLightColor(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_Prop3DInsideShadowColor(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_Prop3DInsideShadowSize(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_Prop3DOutsideHighLightColor(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_Prop3DOutsideShadowColor(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_Prop3DOutsideShadowSize(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_Prop3DBevelSize(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_PropDropShadowColor(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_PropDropShadowSize(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_PropBorderColor(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_PropNCBkColor(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_PropBorderSize(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_PropBorderStyle(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_PropOnFocusInvert3D(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_PropOnFocusShadow(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);

long FP_PropThreeDFrameColor(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_PropBorderDropShadow(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);
long FP_PropAppearance(LPFPNCINFO lpncInfo, LPARAM lParam, BOOL fGetProp);

// scroll.c

BOOL LB_ScrollRequest(LPLISTBOX lpList, WORD wParam, LPARAM lParam,
                      BOOL fKeyPressed);
BOOL LB_ScrollInc(LPLISTBOX lpList, long lVScrollInc, WORD wParam,
                  BOOL fKeyPressed);
void LB_ScrollWindow(LPLISTBOX lpList, short xScroll, short yScroll,
                     LPRECT lpRect);
long LB_SendVirtReq(LPLISTBOX lpList, WORD wAction, long lRowFirst,
                    long lRowsRequested, long lPos);
void LB_VirtReadAhead(LPLISTBOX lpList, long lVScrollInc);

// scrolbar.c

void LB_SetVScrollRange(LPLISTBOX lpList);
long LB_GetActualHExtent(LPLISTBOX lpList);

//
//  Utility macros
//

#define LB_ITEMVPOS(lpList, nItemNum) (short)(lpList->Rect.top + \
                                       LB_RowGetYOffset(lpList, nItemNum))

void DLLENTRY LB_WMNCPaint(HWND hWnd, HDC hDC, LPFPNCINFO info, short redraw,
                          LPRECT lpRect, BOOL fFocus);
short         fpCalcTopMargin(LPFPNCINFO lpInfo);
short         fpCalcBottomMargin(LPFPNCINFO lpInfo);
short         fpCalcLeftMargin(LPFPNCINFO lpInfo, short dButtonWidth);
short         fpCalcRightMargin(LPFPNCINFO lpInfo, short dButtonWidth);

// cell.c

void           fpCellManInit(void);
void           fpCellManFree(void);
void           LB_ListItemRef(HANDLE fpInst, LPLB_LISTITEM lpItem);
void           LB_LineItemRef(LPLB_LINEITEM lpItem);
LPLB_LINEITEM  LB_GetCurrentLineItem(LPLISTBOX lpList, LPLB_LISTITEM lpListItem,
                                     LPLB_LINEITEM lpLineItem);
LPLB_LINEITEM  LB_GetLineItem(LPLISTBOX lpList, LPLB_LISTITEM lpListItem,
                              LPLB_LINEITEM lpLineItem, BYTE bListApplyTo,
                              BYTE bLineApplyTo);
BOOL           LB_SetCurrentLineItem(LPLISTBOX lpList, LPLB_LISTITEM lpListItem,
                                     LPLB_LINEITEM lpLineItem);
BOOL           LB_SetLineItem(LPLISTBOX lpList, LPLB_LISTITEM lpListItem,
                              LPLB_LINEITEM lpLineItem, BYTE bListApplyTo,
                              BYTE bLineApplyTo);
LPLB_LISTITEM  LB_GetCurrentListItem(LPLISTBOX lpList, LPLB_LISTITEM lpListItem);
LPLB_LISTITEM  LB_GetListItem(LPLISTBOX lpList, LPLB_LISTITEM lpListItem,
                              BYTE bListApplyTo);
BOOL           LB_SetCurrentListItem(LPLISTBOX lpList, LPLB_LISTITEM lpListItem);
BOOL           LB_SetListItem(LPLISTBOX lpList, LPLB_LISTITEM lpListItem,
                              BYTE bListApplyTo);
LPLB_LISTITEM  LB_SetDefListItem(LPLISTBOX lpList, LPLB_LISTITEM lpListItem);
LPLB_LINEITEM  LB_SetDefLineItem(LPLISTBOX lpList, LPLB_LINEITEM lpLineItem);
LPLB_LISTITEM  LB_ListItemInit(LPLISTBOX lpList, LPLB_LISTITEM lpListItem);
LPLB_LINEITEM  LB_LineItemInit(LPLISTBOX lpList, LPLB_LINEITEM lpLineItem);
void           LB_GetAttr(LPLISTBOX lpList, LPLB_LISTITEM lpListItem,
                          LPLB_LINEITEM lpLineItemRows,
                          LPLB_LINEITEM lpLineItemCols,
                          LPLB_LINEITEM lpLineItemColsH, LBCOLID idCol,
                          long lRow, LPLB_ROW lpRow, LBGRPID idGrp,
                          WORD wHeader, BOOL fIsSel);
void           LB_CellFree(LPLISTBOX lpList, LPLB_CELL lpCell);
void           LB_CellFreeData(LPLISTBOX lpList, LPLB_CELL lpCell);
LPLB_CELL      LB_CellLock(LPLISTBOX lpList, LPLB_ITEM lpItem, long lRow,
                           LBCOLINDEX nCol, LBCOLID idCol);
#define        LB_CellUnlock(lpList, lpItem, lRow, nCol, idCol) 1
LPLB_CELL      LB_CellLockOrig(LPLISTBOX lpList, LPLB_ITEM lpItem,
                                   long lRow, LBCOLINDEX nCol, LBCOLID idCol);
#define        LB_CellUnlockOrig(lpList, lpItem, lRow, nCol, idCol) 1
BOOL           LB_CellGet(LPLISTBOX lpList, LPLB_ITEM lpItem, LPLB_CELL lpCell,
                          LBCOLINDEX nCol, LBCOLID idCol);
BOOL           LB_CellGetOrig(LPLISTBOX lpList, LPLB_ITEM lpItem,
                              LPLB_CELL lpCell, LBCOLINDEX nCol, LBCOLID idCol);
FPGLOBALHANDLE LB_CellGetString(LPLISTBOX lpList, LPLB_ITEM lpItem, short nCol);
FPGLOBALHANDLE LB_CellItemGetString(LPLISTBOX lpList, LPLB_ITEM lpItem,
                                    LBCOLID idCol);
BOOL           LB_CellDup(LPLISTBOX lpList, LPLB_CELL lpCellOld,
                          LPLB_CELL lpCellNew);
short          LB_CalcLineSize(LPLB_LINEITEM lpLineItem);

// col.c

FPGLOBALHANDLE LB_DupTextHandle(FPGLOBALHANDLE hText);
BOOL           LB_ColItemSet(LPLISTBOX lpList, LBCOLID idCol, LPLB_COL lpCol);
#define        LB_ColSet(lpList, nCol, lpCol) \
                  LB_ColItemSet(lpList, LB_ColIDFromIndex(lpList, nCol), lpCol)
BOOL           LB_ColItemGet(LPLISTBOX lpList, LBCOLID idCol, LPLB_COL lpCol);
#define        LB_ColGet(lpList, nCol, lpCol) \
                  LB_ColItemGet(lpList, LB_ColIDFromIndex(lpList, nCol), lpCol)
#define        LB_ColPosGet(lpList, nCol, lpCol) \
                  LB_ColItemGet(lpList, LB_ColIDFromPosIndex(lpList, nCol), lpCol)
LPLB_COL       LB_ColItemLock(LPLISTBOX lpList, LBCOLID idCol);
#define        LB_ColLock(lpList, nCol) \
                  LB_ColItemLock(lpList, LB_ColIDFromIndex(lpList, nCol))
#define        LB_ColItemUnlock(lpList, idCol) (TRUE)
#define        LB_ColUnlock(lpList, nCol) (TRUE)
LBCOLID        LB_ColIDFromIndex(LPLISTBOX lpList, LBCOLINDEX nCol);
LBCOLID        LB_ColIDFromPosIndex(LPLISTBOX lpList, LBCOLPOSINDEX nCol);
LBCOLINDEX     LB_ColIndexFromID(LPLISTBOX lpList, LBCOLID idCol);
LBCOLPOSINDEX  LB_ColPosIndexFromID(LPLISTBOX lpList, LBCOLID idCol);
LBCOLINDEX     LB_ColParentIndexFromID(LPLISTBOX lpList, LBCOLID idCol);
LBCOLPOSINDEX  LB_ColPosIndexFromScrollPos(LPLISTBOX lpList,
                                           LBCOLSCROLLPOS nScrollPos);
#define        LB_ColIDFromScrollPos(lpList, nScrollPos) \
                  LB_ColIDFromPosIndex(lpList, \
                  LB_ColPosIndexFromScrollPos(lpList, nScrollPos)
BOOL           LB_ColSetCount(LPLISTBOX lpList, LBCOLINDEX nColCount);
BOOL           LB_ColClone(LPLISTBOX lpList, LBCOLINDEX nCol);
BOOL           LB_ColMove(LPLISTBOX lpList, LBCOLID idCol,
                          LBGRPID idGrpParentOld, LBGRPID idGrpParentNew,
                          LBCOLINDEX nColIndexNew);
BOOL           LB_ColItemFree(LPLISTBOX lpList, LBCOLID idCol);
#define        LB_ColFree(lpList, nCol) \
                  LB_ColItemFree(lpList, LB_ColIDFromIndex(lpList, nCol))
LBCOLID        LB_ColIns(LPLISTBOX lpList, LBCOLINDEX nCol);
BOOL           LB_ColDel(LPLISTBOX lpList, LBCOLINDEX nCol);
BOOL           LB_ColReBuildOrderList(LPLISTBOX lpList);
void           LB_ColInvalidate(LPLISTBOX lpList, LBCOLID idCol,
                                BOOL fHeaderOnly, BOOL fInvalidateRight);
BOOL           LB_ColAdjustChildrenWidth(LPLISTBOX lpList,
                                         LBGRPID idGrpParent,
                                         LBCOLID idCol);
long           LB_ColItemGetWidth(LPLISTBOX lpList, LBCOLID idCol);
#define        LB_ColGetWidth(lpList, nCol) \
                  LB_ColItemGetWidth(lpList, LB_ColIDFromIndex(lpList, nCol))
#define        LB_ColPosGetWidth(lpList, nCol) \
                  LB_ColItemGetWidth(lpList, LB_ColIDFromPosIndex(lpList, nCol))
void           LB_ColItemSetWidthInPixels(LPLISTBOX lpList, LBCOLID idCol,
                                          short nWidth);
#define        LB_ColSetWidthInPixels(lpList, nCol, nWidth) \
                  LB_ColItemSetWidthInPixels(lpList, LB_ColIDFromIndex(lpList, nCol), nWidth)
short          LB_GetWrapColCount(LPLISTBOX lpList);
short          LB_ColWidthToPixels(LPLISTBOX lpList, float flWidth, BOOL fHide);
float          LB_PixelsToColWidth(LPLISTBOX lpList, short nColWidth);
LBCOLPOSINDEX  LB_ColGetLeft(LPLISTBOX lpList, LPLONG lpxColOffsetFromBeginning,
                             LPSHORT lpxOffset);
long           LB_ColGetxOffset(LPLISTBOX lpList, LPLB_COL lpCol);
long           LB_ColPosGetxOffset(LPLISTBOX lpList, LBCOLPOSINDEX nCol,
                                   LPLONG lplWidth);
LBCOLPOSINDEX  LB_ColPosGetRight(LPLISTBOX lpList, LPSHORT lpxPos);
LBCOLPOSINDEX  LB_ColPosMouseOver(LPLISTBOX lpList, short xMouse, short yMouse,
                                  BOOL fHeaderOnly, LPSHORT lpX);
#define        LB_ColItemMouseOver(lpList, xMouse, yMouse, fHeaderOnly, lpX) \
                  LB_ColIDFromPosIndex(lpList, LB_ColPosMouseOver(lpList, xMouse, yMouse, fHeaderOnly, lpX))
#define        LB_ColMouseOver(lpList, xMouse, yMouse, fHeaderOnly, lpX) \
                  LB_ColIndexFromID(lpList, LB_ColItemMouseOver(lpList, xMouse, yMouse, fHeaderOnly, lpX))
LBCOLID        LB_ColIsMouseInDragDrop(LPLISTBOX lpList, short xMouse,
                                       short yMouse, LPSHORT lpX);
long           LB_ColGetMaxWidth(LPLISTBOX lpList, LBCOLINDEX nCol,
                                 LBROWINDEX lRow, WORD wMethod);
BOOL           LB_ColIsExtended(LPLISTBOX lpList);

// grp.c

BOOL        LB_GrpCopy(LPLISTBOX lpList, LBGRPINDEX nGrp, LPLB_GRP lpGrp);
BOOL        LB_GrpItemSet(LPLISTBOX lpList, LBGRPID idGrp, LPLB_GRP lpGrp);
#define     LB_GrpSet(lpList, nGrp, lpGrp) \
               LB_GrpItemSet(lpList, LB_GrpIDFromIndex(lpList, nGrp), lpGrp)
BOOL        LB_GrpItemGet(LPLISTBOX lpList, LBGRPID idGrp, LPLB_GRP lpGrp);
#define     LB_GrpGet(lpList, nGrp, lpGrp) \
               LB_GrpItemGet(lpList, LB_GrpIDFromIndex(lpList, nGrp), lpGrp)
LPLB_GRP    LB_GrpItemLock(LPLISTBOX lpList, LBGRPID idGrp);
#define     LB_GrpLock(lpList, nGrp) \
               LB_GrpItemLock(lpList, LB_GrpIDFromIndex(lpList, nGrp))
#define     LB_GrpItemUnlock(lpList, idGrp) (TRUE)
#define     LB_GrpUnlock(lpList, nGrp) (TRUE)
LBGRPID     LB_GrpIDFromIndex(LPLISTBOX lpList, LBGRPINDEX nGrp);
LBGRPID     LB_GrpIDFromScrollPos(LPLISTBOX lpList, LBGRPSCROLLPOS nScrollPos);
LBGRPINDEX  LB_GrpIndexFromID(LPLISTBOX lpList, LBGRPID idGrp);
LBGRPINDEX  LB_GrpParentIndexFromID(LPLISTBOX lpList, LBGRPID idGrp);
BOOL        LB_GrpSetCount(LPLISTBOX lpList, LBGRPINDEX nGrpCount);
BOOL        LB_GrpItemFree(LPLISTBOX lpList, LBGRPID idGrp);
#define     LB_GrpFree(lpList, nGrp) \
               LB_GrpItemFree(lpList, LB_GrpIDFromIndex(lpList, nGrp))
LBGRPID     LB_GrpIns(LPLISTBOX lpList, LBGRPINDEX nGrp);
BOOL        LB_GrpDel(LPLISTBOX lpList, LBGRPINDEX nGrp);
BOOL        LB_GrpInsChild(LPLISTBOX lpList, LBGRPID idGrpParent,
                           LBGRPID idGrp, LBCOLID idCol, short nIndex);
BOOL        LB_GrpRemoveChild(LPLISTBOX lpList, LBGRPID idGrpParent,
                              LBGRPID idGrp, LBCOLID idCol);
void        LB_GrpInvalidate(LPLISTBOX lpList, LBGRPID idGrp,
                             BOOL fHeaderOnly, BOOL fInvalidateRight);
BOOL        LB_GrpAdjustChildrenWidth(LPLISTBOX lpList,
                                      LBGRPID idGrpParent,
                                      LBGRPID idGrp);
long        LB_GrpItemGetWidth(LPLISTBOX lpList, LBGRPID idGrp);
#define     LB_GrpGetWidth(lpList, nGrp) \
               LB_GrpItemGetWidth(lpList, LB_GrpIDFromIndex(lpList, nGrp))
void        LB_GrpItemSetWidthInPixels(LPLISTBOX lpList, LBGRPID idGrp,
                                       short nWidth);
#define     LB_GrpSetWidthInPixels(lpList, nGrp) \
               LB_GrpItemSetWidthInPixels(lpList, LB_GrpIDFromIndex(lpList, nGrp))
void        LB_GrpBuildPosList(LPLISTBOX lpList);
void        LB_GrpPerformBuildPosList(LPLISTBOX lpList);
LBGRPID     LB_GrpGetLeft(LPLISTBOX lpList, LPLONG lpxGrpOffsetFromBeginning,
                          LPSHORT lpxOffset);
LBGRPID     LB_GrpGetLeftTopLevel(LPLISTBOX lpList,
                                  LPLONG lpxGrpOffsetFromBeginning,
                                  LPSHORT lpxOffset);
long        LB_GrpItemGetxOffset(LPLISTBOX lpList, LBGRPID idGrp);
void        LB_CalcFrozenActual(LPLISTBOX lpList);
LBGRPINDEX  LB_GrpGetGrpDescendentCnt(LPLISTBOX lpList, LBGRPID idGrp,
                                      LPLBCOLINDEX lpColCnt);
LBGRPID     LB_GrpMouseOver(LPLISTBOX lpList, short xMouse, short yMouse,
                            BYTE bFlags, LPSHORT lpX, LPBOOL lpfIsInFrozen);
LBGRPID     LB_GrpIsMouseInDragDrop(LPLISTBOX lpList, short xMouse,
                                    short yMouse, LPSHORT lpX);
BOOL        LB_GrpMove(LPLISTBOX lpList, LBGRPID idGrp, LBGRPID idGrpParentOld,
                       LBGRPID idGrpParentNew, LBGRPINDEX nGrpIndexNew);
BOOL        LB_GrpIsAncestor(LPLISTBOX lpList, LBGRPID idGrp,
                             LBGRPID idGrpAncestor);
LBGRPINDEX  LB_GrpGetBottomLevelNext(LPLISTBOX lpList, LBGRPINDEX nGrp);

BOOL        fpTM_Ins(LPFP_TABLEMAN lpTable, short nIndex, LPVOID lpItem);
BOOL        fpTM_Del(LPFP_TABLEMAN lpTable, short nIndex, LPVOID lpItem);
BOOL        fpTM_Move(LPFP_TABLEMAN lpTable, short nIndex, short nIndexDest);
BOOL        fpTM_SetCount(LPFP_TABLEMAN lpTable, short nCount);
long LBx_GrpGetMaxWidth(LPLISTBOX lpList, LBGRPID idGrp, WORD wMethod);

// row.c

LBROWINDEX LB_RowGetBottom(LPLISTBOX lpList);
LBROWINDEX LB_RowGetBottomAllVis(LPLISTBOX lpList);
LBROWINDEX LBx_RowGetBottomAllVis(LPLISTBOX lpList, LBROWINDEX lRowTop);
LBROWINDEX LB_RowGetLastPageAllVis(LPLISTBOX lpList);
LBROWINDEX LB_RowGetTopFromLastAllVis(LPLISTBOX lpList, LBROWINDEX lRow);
LBROWINDEX LBx_RowGetTopFromLastAllVis(LPLISTBOX lpList, LBROWINDEX lRow);
long       LB_RowGetHeight(LPLISTBOX lpList, LBROWINDEX lRow);
long       LB_RowGetHeightActual(LPLISTBOX lpList, LBROWINDEX lRow);
BOOL       LB_RowSetHeight(LPLISTBOX lpList, LBROWINDEX lRow, long lRowHeight);
long       LBx_RowGetYOffset(LPLISTBOX lpList, LBROWINDEX lRowTop,
                             LBROWINDEX lItemNumFromTop);
long       LB_RowGetMaxHeight(LPLISTBOX lpList, LBCOLINDEX nCol,
                              LBROWINDEX lRow, WORD wMethod);
void       LB_RowInvalidate(LPLISTBOX lpList, LBROWINDEX lRow,
                            BOOL fInalidateDown);
BOOL       LB_RowIsExtended(LPLISTBOX lpList);

// pict.c

short        LB_PaintPicture(HDC hDC, HDC hDCDev, LPLISTBOX lpList,
                             LPLB_LISTITEM lpListItem, LPRECT lpRectCell,
                             double dfScaleX, double dfScaleY, BOOL fSelected);

// join.c

BOOL     LB_JoinChange(LPLISTBOX lpList, LBCOLID idCol, LBROWINDEX lRow,
                       LBJOINID idJoin);
BOOL     LB_JoinDel(LPLISTBOX lpList, LBJOINHANDLE hJoin);
LBJOINID LB_JoinGetID(LPLISTBOX lpList, LBCOLINDEX nCol, LBCOLID idCol,
                      LBROWINDEX lRow, LPLB_ITEM lpItem);
BOOL     LB_JoinCalcBlock(LPLISTBOX lpList, LBCOLINDEX nCol, LBCOLID idCol,
                          LBROWINDEX lRow, LPLB_ITEM lpItem, LPLB_BLOCK lpBlock);

// texttip.c

BOOL LB_TextTipRegister(HANDLE hInstance);
BOOL LB_TextTipUnregister(HANDLE hInstance);
BOOL LB_TextTipShow(LPLISTBOX lpList, short xMouse, short yMouse);

// user.c

long  _LP_GetMaxSize(HWND hWnd, LBCOLINDEX nCol, LBROWINDEX lRow,
                     short wMethod, short wUnits);
long  LBx_GetMaxSize(LPLISTBOX lpList, LPLB_MAXSIZE lpMaxSize);
short _LP_GetCellPos(HWND hWnd, LBCOLINDEX Col, LBROWINDEX Row,
                     LPLONG lpx, LPLONG lpy, LPLONG lpWidth,
                     LPLONG lpHeight, short wUnits);
BOOL  LBx_GetCellRect(LPLISTBOX lpList, LBCOLINDEX Col, LBROWINDEX Row,
                      LPRECT lpRect);
#ifdef LP30
long  _LP_GetMaxSizeHeader(HWND hWnd, LBCOLINDEX nCol,
                     short wMethod, short wUnits, BOOL fGrpHeader);
long FP_API LP_GetMaxHeight(HWND hWnd, LBCOLINDEX nCol, LBROWINDEX lRow,
                          LBGRPINDEX nGrp, BOOL fHeader, short wMethod, short wUnits);
long FP_API LP_GetMaxWidth(HWND hWnd, LBCOLINDEX nCol, LBROWINDEX lRow,
                          LBGRPINDEX nGrp, BOOL fHeader, short wMethod, short wUnits);
#endif // LP30

//$PBRW...
// lbpbrw.c - Listbox Property Browser
BOOL InvokePropBrowserDialog(/*LPPBRW_CTLINFO*/ LPVOID lpCtlInfo);
#ifdef FP_VB
BOOL LB_RegisterPropBrowser(HINSTANCE Instance);
BOOL LB_UnregisterPropBrowser(HINSTANCE Instance);
BOOL LB_InitPropBrowser(LPVOID lpCtl, int ctlType);
BOOL LB_VBXInvokePropBrowser(LPCTL_STRUCT lpCtlStruct);
long LB_VBXGetListOfValues(LPVOID lpCtl, int ctlType, WORD wProp, HWND hWndList);

#else // OCX

void fpDesignerForceClose(void FAR *lpfpDesignerParams);
void __cdecl LP_FpPropDesigner(void FAR *lpFpDesignerParams);

#endif
//$PBRW.

// misc.c

void LB_CalcDefFontWidth(LPLISTBOX lpList, LPLB_LISTITEM lpListItem);
void LB_GetHeightInOut(LPLISTBOX lpList, LPLB_ROW lpRow, LBROWINDEX lRow,
                       BOOL fHeader, LPSHORT lpnRowHeightOutside,
                       LPSHORT lpnRowHeightInside);

BOOL DLLENTRY LB_OCXGetMessageReflect(LPLISTBOX lpList);

#ifdef LP30
/***************************
* Support for the Text Tip
***************************/

BOOL DLLENTRY LC_SetTextTip(HWND hWnd, WORD wStatus, LONG lDelay,
                           LPLOGFONT lpLogFont, COLORREF clrBack,
                           COLORREF clrFore);
BOOL DLLENTRY LC_GetTextTip(HWND hWnd, LPWORD lpwStatus, LPLONG lplDelay,
                           LPLOGFONT lpLogFont, LPCOLORREF lpclrBack,
                           LPCOLORREF lpclrFore);

#define LC_TT_STATUS_OFF               0
#define LC_TT_STATUS_FIXED             1
#define LC_TT_STATUS_FLOATING          2
#define LC_TT_STATUS_FIXEDFOCUSONLY    3
#define LC_TT_STATUS_FLOATINGFOCUSONLY 4

#endif // LP30

#endif // APSTUDIO_READONLY_SYMBOLS

#ifndef RC_INVOKED
#pragma pack()
#endif  /* RC_INVOKED */

#ifdef __cplusplus
}
#endif    

#else      // SPREAD DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#include "fplist.h"
#define LBMx_SETOWNERHWND          0x0611
#define LBMx_SIZEDROPDOWN          0x0612
#define LBMx_GETCOLTEXT            0x0613
#define LBMx_GETCOLTEXTLEN         0x0614
#define LBMx_GETACTUALHEXTENT      0x0615

#ifdef __cplusplus
}
#endif    

#endif


#endif
