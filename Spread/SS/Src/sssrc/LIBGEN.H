/*
$Revision:   1.61  $
*/

/*
$Log:   N:/DRLIBS/TOOLBOX/LIBGEN.H_V  $
 * 
 *    Rev 1.61   13 Aug 1990 15:16:10   Sonny
 * Modified the declaration of functions (MemAlloc) to be conditional.
 * 
 *    Rev 1.60   19 Jul 1990 10:21:50   Sonny
 * Added TB_MINVER, TB_MAJVER
 * 
 *    Rev 1.59   10 Jul 1990 19:03:50   Sonny
 * 
 *    Rev 1.58   06 Jul 1990 11:41:22   Sonny
 * 
 *    Rev 1.57   22 Jun 1990 17:27:44   Dirk
 * 
 *    Rev 1.56   18 Jun 1990 14:22:16   Steve
 * No change.
 * 
 *    Rev 1.55   18 Jun 1990 10:36:18   Dirk
 * 
 *    Rev 1.54   15 Jun 1990 12:38:02   Steve
 * changed CalendarProc to DLGENTRY, moved it out of an ifdef block
 * 
 *    Rev 1.53   14 Jun 1990 17:27:10   Sonny
 * 
 *    Rev 1.52   14 Jun 1990 15:32:56   Bruce
 * No change.
 * 
 *    Rev 1.51   13 Jun 1990 10:07:48   Bruce
 * 
 *    Rev 1.50   12 Jun 1990 14:06:50   Dirk
 * 
 *    Rev 1.49   12 Jun 1990  9:26:26   Sonny
 * 
 *    Rev 1.40   06 Jun 1990  9:51:36   Bruce
 * No change.
 * 
 *    Rev 1.39   01 Jun 1990 15:49:30   Sonny
 * 
 *    Rev 1.38   31 May 1990 16:50:36   Sonny
 * Modified Heap Manager
 * 
 *    Rev 1.37   31 May 1990 12:33:54   Bruce
 * No change.
 * 
 *    Rev 1.36   30 May 1990 10:52:04   Randall
 * Added GuageWndProc prototype
 * 
 *    Rev 1.35   30 May 1990 10:37:28   Sonny
 * 
 *    Rev 1.34   29 May 1990 17:22:34   Sonny
 * 
 *    Rev 1.33   26 May 1990 13:33:32   Bruce
 * 
 *    Rev 1.32   28 May 1990 17:38:12   Sonny
 * Added ProcessWMleftButtonUp
 * 
 *    Rev 1.31   24 May 1990 17:31:18   Randall
 * No change.
 * 
 *    Rev 1.30   24 May 1990 15:43:10   Randall
 * No change.
 * 
 *    Rev 1.29   23 May 1990 16:43:48   Sonny
 * Added hFont in EDIT struct
 * 
 *    Rev 1.28   22 May 1990 12:30:04   Sonny
 * 
 *    Rev 1.27   21 May 1990 18:20:20   Sonny
 * Added support for Windows 3.
 * 
 *    Rev 1.26   21 May 1990 13:24:40   Randall
 * Moved internal edit field stuff to header files in edit... subdirectories.
 * 
 *    Rev 1.25   21 May 1990 10:47:20   Sonny
 * Added wHorzExtent
 * 
 *    Rev 1.24   20 May 1990  3:34:02   Sonny
 * Added DrvListGetTextExtent
 * 
 *    Rev 1.23   20 May 1990  0:25:20   Sonny
 * 
 *    Rev 1.22   19 May 1990 19:45:18   Sonny
 * changed the prototype of ProcessWMVerticalScroll to returnin BOOL
 * and added fTabs in GRAYSTRUCT
 * 
 *    Rev 1.21   18 May 1990 21:20:06   Sonny
 * Added rect in GRAYSTRUCT
 * 
 *    Rev 1.20   18 May 1990 17:11:38   Randall
 * Added ClearSelection function
 * 
 *    Rev 1.19   18 May 1990 14:39:16   Sonny
 * Added ODA_ERASE
 * 
 *    Rev 1.18   18 May 1990 11:50:36   Randall
 * Changed highest year supported (HIYEAR) to 2099.
*/
#ifndef __LIBGEN_H
#define __LIBGEN_H

#ifdef __cplusplus
extern "C" {
#endif

/*

   INTERNAL FUNCTIONS AND DEFINES OF LIBGEN

*/

#define     TB_MAJVER         2     /* 1.xx */
#define     TB_MINVER         0     /* .01 */

#define     TOOLBOXVER        MAKEWORD(TB_MINVER,TB_MAJVER)

// Bit Field macros
         
#define  ISBETWEEN(x, a, b) ((x >= min(a, b)) && (x <= max(a, b)))
#define  InRange(l,a,h)     ((l) <= (a) && (a) <= (h))
#define  NotInSet(a,b)      (!((a)&(b)))
#define  InSet(a,b)         ((a)&(b))
#define  BitOn(a,b)         ((a)|=(b))
#define  BitOff(a,b)        ((a)&=~(b))

#define  MAXUSAGE    20 // max application error routines
#define  MAXTMPFILES 30 // max files open

#define  LINELEN     129   // Arbitary line length
#define  STRLEN      257   // Arbitary line length

// private ToolBox messages
#define EM_EDIT_ADJUSTLENGTH       (EM_TOOLBOXUSERMAX+1)
#define EM_EDIT_POSITIONAFTERPAINT (EM_TOOLBOXUSERMAX+2)
#define EM_RESETPREVLEN            (EM_TOOLBOXUSERMAX+3)
#define EM_REPLACETEXT             (EM_TOOLBOXUSERMAX+4)

// private ToolBox notifications

#ifdef STATIC

#ifdef WINENTRY
#undef WINENTRY
#endif

#define WINENTRY     LONG _export FAR PASCAL

#ifdef DLGENTRY
#undef DLGENTRY
#endif

#define DLGENTRY     BOOL _export FAR PASCAL

#endif

typedef struct tagErrorFunc {
   HANDLE   Task;
   FARPROC  lpErrProc;
   BOOL     fActive;
   } ERRORFUNC;
typedef ERRORFUNC * NPERRORFUNC;
typedef ERRORFUNC FAR * LPERRORFUNC;

typedef struct tagTMPBLK   {
   int   fh;
   char  FileName[FILELEN];
   }  TMPBLK;
typedef TMPBLK FAR * LPTMPBLK;

#define  FILE_PART_LENGTH   8
#define  EXT_PART_LENGTH    3

/* LB_DIR File attribute bit masks */
#define  DLGDIR_READWRITE  0x0000
#define  DLGDIR_SUBDIR     0x0010
#define  DLGDIR_DRIVES     0x4000
#define  DLGDIR_EXCLUSIVE  0x8000

/* TEXT CAPABILITY BIT MASKS */
#define  DOES_VECTOR       0x4000
#define  DOES_RASTER       0x2000
#define  DOES_ITALIC       0x0400
#define  DOES_BOLD         0x0200
#define  DOES_STRIKEOUT    0x1000
#define  DOES_UNDERLINE    0x0800

typedef struct tagFontHold {
   int          FontType;
   LOGFONT      LogF;
   TEXTMETRIC   Tm;
   } FONTHOLD;
typedef FONTHOLD FAR * LPFONTHOLD;

typedef struct tagHandleInfo {
   int      Count;
   HANDLE   fhFontHold;
   } HANDLEINFO;
typedef HANDLEINFO  FAR * LPHANDLEINFO;


/* EXTERNALS                  */
extern   LPERRORFUNC    lpErr;
extern   HINSTANCE      hDynamicInst;
extern   TMPBLK         TmpFiles[];
extern   HANDLE         hbmInfo;

// in Gauge.c
WINENTRY GaugeWndProc(HWND,unsigned,WORD,LONG);
BOOL  DLLENTRY FocusRect(HDC hDC, LPRECT lpRect);

// in Calendar

DLGENTRY CalendarProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
WINENTRY CalendarWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#ifdef   DRV_EDIT    // then we include all the structure defns

#if defined(_WIN64) || defined(_IA64)
  #define GWL_EDITFIELD   0
  #define GWL_DATEFIELD   8
  #define GWL_FLOATFIELD  8
  #define GWL_GENFIELD    8
  #define GWL_INTFIELD    8
  #define GWL_PICFIELD    8
  #define GWL_TIMEFIELD   8
  #define GWL_NUMFIELD    8
  #define GWL_DEFWNDPROC  16
  #define GetDefWndProc(hWnd)     ((FARPROC)GetWindowLongPtr((hWnd), GWL_DEFWNDPROC))
  #define GetWindowEditField(hwnd)  ((HANDLE)GetWindowLongPtr(hWnd,GWL_EDITFIELD))
  #define GetWindowDateField(hwnd)  ((HANDLE)GetWindowLongPtr(hWnd,GWL_DATEFIELD))
  #define GetWindowFloatField(hwnd) ((HANDLE)GetWindowLongPtr(hWnd,GWL_FLOATFIELD))
  #define GetWindowGenField(hwnd)   ((HANDLE)GetWindowLongPtr(hWnd,GWL_GENFIELD))
  #define GetWindowIntField(hwnd)   ((HANDLE)GetWindowLongPtr(hWnd,GWL_INTFIELD))
  #define GetWindowPicField(hwnd)   ((HANDLE)GetWindowLongPtr(hWnd,GWL_PICFIELD))
  #define GetWindowTimeField(hwnd)  ((HANDLE)GetWindowLongPtr(hWnd,GWL_TIMEFIELD))
  #define GetWindowNumField(hwnd)   ((HANDLE)GetWindowLongPtr(hWnd,GWL_NUMFIELD))
  #define SetWindowEditField(hwnd,value)  SetWindowLongPtr(hWnd,GWL_EDITFIELD,(LONG_PTR)(value))
  #define SetWindowDateField(hwnd,value)  SetWindowLongPtr(hWnd,GWL_DATEFIELD,(LONG_PTR)(value))
  #define SetWindowFloatField(hwnd,value) SetWindowLongPtr(hWnd,GWL_FLOATFIELD,(LONG_PTR)(value))
  #define SetWindowGenField(hwnd,value)   SetWindowLongPtr(hWnd,GWL_GENFIELD,(LONG_PTR)(value))
  #define SetWindowIntField(hwnd,value)   SetWindowLongPtr(hWnd,GWL_INTFIELD,(LONG_PTR)(value))
  #define SetWindowPicField(hwnd,value)   SetWindowLongPtr(hWnd,GWL_PICFIELD,(LONG_PTR)(value))
  #define SetWindowTimeField(hwnd,value)  SetWindowLongPtr(hWnd,GWL_TIMEFIELD,(LONG_PTR)(value))
  #define SetWindowNumField(hwnd,value)   SetWindowLongPtr(hWnd,GWL_NUMFIELD,(LONG_PTR)(value))
  #define SetDefWndProc(hwnd,value)       SetWindowLongPtr(hWnd,GWL_DEFWNDPROC,(LONG_PTR)(value))
#elif defined(WIN32)
  #define GWL_EDITFIELD   0
  #define GWL_DATEFIELD   4
  #define GWL_FLOATFIELD  4
  #define GWL_GENFIELD    4
  #define GWL_INTFIELD    4
  #define GWL_PICFIELD    4
  #define GWL_TIMEFIELD   4
  #define GWL_NUMFIELD    4
  #define GWL_DEFWNDPROC  8
  #define GetDefWndProc(hWnd)     ((FARPROC)GetWindowLong((hWnd), GWL_DEFWNDPROC))
  #define GetWindowEditField(hwnd)  ((HANDLE)GetWindowLong(hWnd,GWL_EDITFIELD))
  #define GetWindowDateField(hwnd)  ((HANDLE)GetWindowLong(hWnd,GWL_DATEFIELD))
  #define GetWindowFloatField(hwnd) ((HANDLE)GetWindowLong(hWnd,GWL_FLOATFIELD))
  #define GetWindowGenField(hwnd)   ((HANDLE)GetWindowLong(hWnd,GWL_GENFIELD))
  #define GetWindowIntField(hwnd)   ((HANDLE)GetWindowLong(hWnd,GWL_INTFIELD))
  #define GetWindowPicField(hwnd)   ((HANDLE)GetWindowLong(hWnd,GWL_PICFIELD))
  #define GetWindowTimeField(hwnd)  ((HANDLE)GetWindowLong(hWnd,GWL_TIMEFIELD))
  #define GetWindowNumField(hwnd)   ((HANDLE)GetWindowLong(hWnd,GWL_NUMFIELD))
  #define SetWindowEditField(hwnd,value)  SetWindowLong(hWnd,GWL_EDITFIELD,(LONG)(value))
  #define SetWindowDateField(hwnd,value)  SetWindowLong(hWnd,GWL_DATEFIELD,(LONG)(value))
  #define SetWindowFloatField(hwnd,value) SetWindowLong(hWnd,GWL_FLOATFIELD,(LONG)(value))
  #define SetWindowGenField(hwnd,value)   SetWindowLong(hWnd,GWL_GENFIELD,(LONG)(value))
  #define SetWindowIntField(hwnd,value)   SetWindowLong(hWnd,GWL_INTFIELD,(LONG)(value))
  #define SetWindowPicField(hwnd,value)   SetWindowLong(hWnd,GWL_PICFIELD,(LONG)(value))
  #define SetWindowTimeField(hwnd,value)  SetWindowLong(hWnd,GWL_TIMEFIELD,(LONG)(value))
  #define SetWindowNumField(hwnd,value)   SetWindowLong(hWnd,GWL_NUMFIELD,(LONG)(value))
  #define SetDefWndProc(hwnd,value)       SetWindowLong(hWnd,GWL_DEFWNDPROC,(LONG)(value))
#else
  #define GWW_EDITFIELD   0
  #define GWW_DATEFIELD   2
  #define GWW_FLOATFIELD  2
  #define GWW_GENFIELD    2
  #define GWW_INTFIELD    2
  #define GWW_PICFIELD    2
  #define GWW_TIMEFIELD   2
  #define GWL_DEFWNDPROC  4
  #define GetDefWndProc(hWnd)     ((FARPROC)GetWindowLong((hWnd), GWL_DEFWNDPROC))
  #define GetWindowEditField(hwnd)  ((HANDLE)GetWindowWord(hWnd,GWW_EDITFIELD))
  #define GetWindowDateField(hwnd)  ((HANDLE)GetWindowWord(hWnd,GWW_DATEFIELD))
  #define GetWindowFloatField(hwnd) ((HANDLE)GetWindowWord(hWnd,GWW_FLOATFIELD))
  #define GetWindowGenField(hwnd)   ((HANDLE)GetWindowWord(hWnd,GWW_GENFIELD))
  #define GetWindowIntField(hwnd)   ((HANDLE)GetWindowWord(hWnd,GWW_INTFIELD))
  #define GetWindowPicField(hwnd)   ((HANDLE)GetWindowWord(hWnd,GWW_PICFIELD))
  #define GetWindowTimeField(hwnd)  ((HANDLE)GetWindowWord(hWnd,GWW_TIMEFIELD))
  #define SetWindowEditField(hwnd,value)  SetWindowWord(hWnd,GWW_EDITFIELD,(WORD)(value))
  #define SetWindowDateField(hwnd,value)  SetWindowWord(hWnd,GWW_DATEFIELD,(WORD)(value))
  #define SetWindowFloatField(hwnd,value) SetWindowWord(hWnd,GWW_FLOATFIELD,(WORD)(value))
  #define SetWindowGenField(hwnd,value)   SetWindowWord(hWnd,GWW_GENFIELD,(WORD)(value))
  #define SetWindowIntField(hwnd,value)   SetWindowWord(hWnd,GWW_INTFIELD,(WORD)(value))
  #define SetWindowPicField(hwnd,value)   SetWindowWord(hWnd,GWW_PICFIELD,(WORD)(value))
  #define SetWindowTimeField(hwnd,value)  SetWindowWord(hWnd,GWW_TIMEFIELD,(WORD)(value))
  #define SetDefWndProc(hwnd,value)       SetWindowLong(hWnd,GWL_DEFWNDPROC,(LONG)(value))
#endif

// Drover Class Names
#define  GENERAL_CLASS_NAME  "TBEdit"
#define  DATE_CLASS_NAME     "TBDate"
#define  TIME_CLASS_NAME     "TBTime"
#define  INTEGER_CLASS_NAME  "TBInteger"
#define  FLOAT_CLASS_NAME    "TBFloat"
#define  PIC_CLASS_NAME      "TBPic"
#define  NUM_CLASS_NAME      "TBNum"
#define  LISTBOX_CLASS_NAME  "TBListBox"
#define  CALENDAR_CLASS_NAME "TBCalendar"

// in EditFld and FieldMgr

WINENTRY EditFieldWndProc  (HWND, UINT, WPARAM, LPARAM);
BOOL     GetOverstrikeMode (HWND, BOOL);
void     SetOverstrikeMode (HWND, BOOL);
void     SetInsertMode     (HWND, BOOL);
long     NotifyParent      (HWND, int);
void     EditSuspendNotify(HWND hWnd, BOOL fSuspend);

// in EditGeneral

WINENTRY EditGeneralWndProc  (HWND, UINT, WPARAM, LPARAM);

// in EditDate

WINENTRY EditDateWndProc  (HWND, UINT, WPARAM, LPARAM);

// in EditTime

WINENTRY EditTimeWndProc  (HWND, UINT, WPARAM, LPARAM);

// in EditInt

WINENTRY EditIntegerWndProc  (HWND, UINT, WPARAM, LPARAM);

// in EditFloat

WINENTRY EditFloatWndProc  (HWND, UINT, WPARAM, LPARAM);

// in EditPic

WINENTRY EditPicWndProc  (HWND, UINT, WPARAM, LPARAM);

// in EditNum

WINENTRY EditNumWndProc  (HWND, UINT, WPARAM, LPARAM);

#endif
// drvlist*.c

#ifdef   DRV_LISTBOX

/* DRVLIST.H   definition for drover list box */

typedef  struct
tagELEMENT
{
   BYTE        fSelected;      // this will indicate whether this item is selected or not
   BYTE        fVisible;       // this will indicate whether this item is visible
   DWORD       hData;
   WORD        wIndex;         // current position. this will be filled by the sort routine
} ELEMENT;

#if 0
typedef  struct
tagELEMENT
{
   unsigned    fSelected : 1;
   unsigned    fVisible  : 1;
   unsigned    wIndex    : 14;
   DWORD       hData;
} ELEMENT;
#endif

typedef  ELEMENT  far   *LPELEMENT;
typedef  ELEMENT        *PELEMENT;
typedef  ELEMENT  near  *NPELEMENT;

#define  DRV_DEFELEMENT    200
#define  DRV_DEFSIZE       DRV_DEFELEMENT * sizeof(ELEMENT)

#define  ODS_ITEM    0x0020
#define  ODA_ERASE   0x0008

// Heap Manager

#define  DEF_HEAPSIZE   256

typedef  HANDLE    VMHANDLE;
typedef  DWORD    VMPTR;

/*********************************************************************

// the structure of the heap is as follows:
           ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿                
           ³  variable size³ 
           ³  memory blocks³ 
           ³               ³ 
           ³               ³ 
           ³               ³ 
           ³ --------------³
           ³       |       ³ 
           ³       |       ³ 
           ³   downwards   ³     When the difference between the fixed blocks
           ³               ³     and moveable blocks reaches a size where 
           ³               ³     allocating another HEAPREC structure is
           ³   upwards     ³     insufficient then the global memory pool
           ³       ^       ³     is reallocated and blocks are move towards 
           ³       |       ³     the other end to increase the difference.
           ³       |       ³ 
           ³       |       ³ 
           ³               ³
           ³               ³ 
           ³  fixed block  ³ 
           ³  where        ³ 
           ³  HEAPREC      ³ 
           ³  is allocated ³ 
           ³               ³ 
           ³               ³ 
           ³               ³ 
           ³               ³
           ³               ³
           ³               ³
           ³               ³
           ³               ³
           ³               ³
           ³               ³
           ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ



************************************************************************/

// Heap Manager functions

VMHANDLE    DLLENTRY VMInit(WORD wDefHandle, WORD wFlags, WORD wDefSize);
BOOL        DLLENTRY VMDestroy(VMHANDLE hVM);
VMPTR       DLLENTRY VMAlloc(VMHANDLE hVM, WORD wSize);
BOOL        DLLENTRY VMFree(VMHANDLE hVM, VMPTR vmPtr);
LPVOID      DLLENTRY VMLock(VMHANDLE hVM, VMPTR vmPtr);
BOOL        DLLENTRY VMUnlock(VMHANDLE hVM, VMPTR vmPtr);
BOOL        DLLENTRY VMReset(VMHANDLE hVM);

// End of Heap Manager


typedef  struct
tagDRVLIST
{
   HANDLE      hInstance;           // the instance of the owner
   HWND        hWndParent;          // the owner of this list box
   HWND        hWnd;                // Window handle of this list box
   WORD        wWndId;              // Window ID of this list box
   HANDLE      hElement;            // to the first element
   RECT        rListBoxDim;         // Dimension of the list box
   int         iFirstSelectedItem;  // Currently selected item
   int         iExtendedItemBase;   // Base item for extended select
   int         iRectDotPos;         // current position of the dotted rectangle
   WORD        wTopItem;            // The top item on the list box
   LONG        style;               // the style of the list box
   short       sVScrollPos;         // Vertical thumb position
   short       sVScrollMax;         // Vertical  max pos
   short       sHScrollPos;         // Horizontal thumb position
   short       sHScrollMax;         // Horizontal thumb position
	WORD		   wChar;					// this character pressed
	WORD		   wLastChar;				// Last character pressed
   WORD        wTotalVisibleItems;  // total number of elements that can be displayed
   WORD        wTotalItems;         // total number of items in the listbox
   WORD        wTotalElement;       // total number of elements that can be fitted
   short       sItemHeight;         // item height in logical pixels
   short       sItemWidth;          // as above
   WORD        wHorzExtent;         // Range of Horz Scroll in Pixel
   WORD        itemState;           // current state of this item
   WORD	      itemAction;          // current action
   HANDLE      hFont;               // Current Font
   WORD        wColumnWidth;        // Column width (in logical pixel)
   short       xDblClkPos;          // last position of the mouse in the X axis
   short       yLastPos;          // last position of the mouse in the X axis
   short       sDblClkItem;
   WORD        wTabStop;
   VMHANDLE    vmHandle;
   BOOL        fMouseCaptureOn;
   BOOL        fShiftDown;
   BOOL        fControlDown;
} DRVLIST;

typedef  DRVLIST  FAR   *LPDRVLIST;
typedef  DRVLIST        *PDRVLIST;
typedef  DRVLIST  NEAR  *NPDRVLIST;

#define MK_NOSELECT      0x1000


typedef struct
{
   HFONT    hFont;
   HBRUSH   hBrush;
   LPSTR    szStr;
   RECT     rect;
   BYTE     fTabs;
   WORD     wTabStops;
} GRAYSTRSTRUCT;

typedef  GRAYSTRSTRUCT FAR *LPGRAYSTRSTRUCT;

#define  LockHeader(lpList,hList,hWnd)  \
   HANDLE      (hList);\
   LPDRVLIST   (lpList);\
   (hList) = GetWindowWord((hWnd), 0);\
   (lpList) = (LPDRVLIST)tbMemLock((hList));\

#define  UnlockHeader(hList)  tbMemUnlock((hList))


// Definition for dialog structures (see Windows 3.0 manual for details)

#define		FieldOffset(type, field)	((int)&(((type *)0)->field))
#define		FieldSize(type, field)		sizeof(((type *)0)->field)/sizeof(char)


#define  MAX_DLGCAPTION_LENGTH     90
#define  MAX_DLGITEMCLASS_LENGTH   30
#define  MAX_DLGITEMTEXT_LENGTH    60

typedef
struct
tagfpDLGTEMPLATE
{
   DWORD dtStyle;
   BYTE  dtItemCount;
   int   dtX;
   int   dtY;
   int   dtCX;
   int   dtCY;
   char  dtMenuName[1];
   char  dtClassName[1];
   char  dtCaptionText[MAX_DLGCAPTION_LENGTH + 1];
} FPDLGTEMPLATE;

typedef
struct
tagDLGINFO
{
   FARPROC  lpProc;
   HANDLE   hRes;
} DLGINFO;

typedef  DLGINFO  FAR *LPDLGINFO;

typedef  FPDLGTEMPLATE FAR *LPFPDLGTEMPLATE;
typedef  FPDLGTEMPLATE     *PFPDLGTEMPLATE;

typedef
struct
tagfpDLGITEMTEMPLATE
{
   int   dtilX;
   int   dtilY;
   int   dtilCX;
   int   dtilCY;
   int   dtilID;
   DWORD dtilStyle;
   char  dtilClass[MAX_DLGITEMCLASS_LENGTH + 1];
   char  dtilText[MAX_DLGITEMTEXT_LENGTH + 1];
   char  dtilInfo;
   PSTR  dtilData;
} FPDLGITEMTEMPLATE;

typedef  DLGITEMTEMPLATE FAR *LPDLGITEMTEMPLATE;
typedef  DLGITEMTEMPLATE     *PDLGITEMTEMPLATE;

WINENTRY       DroverListWndProc(HWND hWnd, unsigned mMessage, WORD wParam, LONG lParam);
DLGENTRY       DlgManagerProc(HWND hDlg, unsigned msg, WORD wParam, LONG lParam);

typedef  enum { LEFT_BUTTON, MIDDLE_BUTTON, RIGHT_BUTTON } MOUSE_BUTTON;

BOOL  ProcessWMCreate(HWND hWnd, LONG lParam);
void  ProcessWMDestroy(HWND hWnd);
BOOL  DeleteOwnerItem(LPDRVLIST lpList, LONG hData);
BOOL  ResetDroverListBox(LPDRVLIST  lpList, BOOL fWipeAll);
LONG  DrvListAddString(HWND hWnd, WORD wParam, LONG lParam);
int   InsertElementInList(LPDRVLIST lpList, LONG hNew, BOOL fSort);
LONG  DrvListDeleteString(HWND hWnd, register WORD wParam, LONG lParam);
BOOL  CompareOwnerItem(LPDRVLIST lpList, WORD wIndex1, LPVOID lpItem1, WORD wIndex2, LPVOID lpItem2);
LONG  DrvListFindString(HWND hWnd, WORD wParam, LONG lParam);
LONG  DrvListGetCount(HWND hWnd, WORD wParam, LONG lParam);
LONG  DrvListGetCurSel(HWND hWnd, WORD wParam, LONG lParam);
LONG  DrvListGetSelCount(HWND hWnd, WORD wParam, LONG lParam);
void  DrvListResetContent(HWND hWnd, WORD wParam, LONG lParam);
void  DrvListSetReDraw(HWND hWnd, WORD wParam);
void  ProcessWMPaint(HWND hWnd);
void	ProcessWMLeftButtonDown(HWND hWnd, WORD wParam, LONG lParam);
void	ProcessWMLeftButtonUp(HWND hWnd, WORD wParam, LONG lParam);
BOOL	ProcessWMVerticalScroll(HWND hWnd, WORD wParam, LONG lParam);
BOOL	ProcessWMHorizontalScroll(HWND hWnd, WORD wParam, LONG lParam);
void	ProcessWMKeyDown(HWND hWnd, WORD wParam, LONG lParam);
LONG  DrvListGetText(HWND hWnd, WORD wParam, LONG lParam);
LONG  DrvListGetTextLen(HWND hWnd, WORD wParam, LONG lParam);
LONG  DrvListGetTopIndex(HWND hWnd, WORD  wParam, LONG lParam);
LONG  DrvListSelectString(HWND hWnd, WORD wParam, LONG lParam);
void  ProcessWMSetFocus(HWND hWnd);
void  ProcessWMKillFocus(HWND hWnd);
LONG  DrvListSetCurSel(HWND hWnd, WORD wParam, LONG lParam);
void	ProcessWMChar(HWND hWnd, WORD wParam, LONG lParam);
void	ProcessWMMouseMove(HWND hWnd, WORD wParam, LONG lParam);
LONG  DrvListGetSelItems(HWND hWnd, WORD wParam, LONG lParam);
void  DrvListSetTopIndex(HWND hWnd, WORD wParam, LONG lParam);
LONG  DrvListSetSel(HWND hWnd, WORD wParam, LONG lParam);
LONG  DrvListGetSel(HWND hWnd, WORD wParam);
void  ProcessMultiSelection(HWND hWnd, WORD wParam);
void  ProcessSort(HWND hWnd);
LONG  DrvListInsertString(HWND hWnd, WORD wParam, LONG lParam);
void  ProcessWMDoubleClick(HWND hWnd, MOUSE_BUTTON which, WORD wParam, LONG lParam);
void  ProcessWMSetFocus(HWND hWnd);
void  ProcessWMKillFocus(HWND hWnd);
void  ProcessWMErase(HWND hWnd);
void  ProcessWMEnable(HWND hWnd, WORD wParam);
void  CalcInvalidRect(LPDRVLIST lpList);
int   DLLENTRY StrCompare(LPDRVLIST lpList, LPCOMPAREITEMSTRUCT lpComp);
int   DLLENTRY ItemCompare(LPDRVLIST lpList, LPCOMPAREITEMSTRUCT lpComp);
void  ProcessWMGetDlgCode(HWND hWnd, WORD wParam);
int   ProcessWMLBDIR(HWND hWnd, WORD wParam, LONG lParam);
BOOL  DrvListWMSize(HWND hWnd, WORD wParam, LONG lParam);
HFONT DrvListWMSetFont(HWND hWnd, WORD wParam, LONG lParam);
HFONT DrvListWMGetFont(HWND hWnd);
LONG  DrvListGetTextExtent(HWND hWnd, WORD wParam, LONG lParam);
int   DrvListWMGetText(HWND hWnd, WORD wParam, LONG lParam);
int   DrvListWMSetText(HWND hWnd, WORD wParam, LONG lParam);
int   DrvListGetHorzExt(HWND hWnd, WORD wParam, LONG lParam);
int   DrvListSetHorzExt(HWND hWnd, WORD wParam, LONG lParam);
int   DrvListSetTabStops(HWND hWnd, WORD wParam, LONG lParam);
int   DrvListSetColumnWidth(HWND hWnd, WORD wParam, LONG lParam);
LONG  ProcessGetDBLClkHPos(HWND hWnd, WORD wParam, LONG lParam);
LONG  ProcessGetDBLClkItem(HWND hWnd, WORD wParam, LONG lParam);
BOOL  DLLENTRY DrvListGrayString(HDC hDC, LPGRAYSTRSTRUCT lpData, int nCount);
void  DLLENTRY DrvTextOut(HDC hDC, int x, int y, LPGRAYSTRSTRUCT lpData);
int   BinaryInsert(LPDRVLIST lpList, LPELEMENT base, LPELEMENT item, register WORD  num, FARPROC lpCompare);
void  CalcNewWindowSize(LPDRVLIST lpList, BOOL fDraw);
DWORD DLLENTRY TabTextOut(HDC hDC, int x, int y, LPSTR szStr, int nTabCount, DWORD dwTab);
DWORD DLLENTRY GetTabTextExtent(HDC hDC, LPSTR szStr, int iTab);
void  PaintListBox(HWND hWnd, LPRECT lpRect, BOOL fErase);
#endif


#ifdef   DRVPATTERN
#define	PATWID		   8					/* width of brush pattern */
#define	PATHGT		   8					/* height of pattern */

#define	NPENS			   6					/* number of plotter pen colours */
#define	NPATS			   6					/* number of plotter hash patterns */

#define	NPLANES		   4					/* number of screen graphics planes */

#define	NCOLORS	   	(1<<NPLANES)	/* number of screen colours */
#define  NPLOTTER       2

#define	MAXCOLNUM      256				/* maximum colour numbers */
#define  MAXFILEBUF     100

#define  MAJ_VERSION        3

#define  MIN_VERSION        0

typedef struct
{
	// the size of pattern will always be on 32 bit aligned i.e. DWORD
	BYTE	Pattern[PATHGT][(PATWID / 2) + (PATWID % sizeof(DWORD))];
} SCREENCOLOR;

typedef SCREENCOLOR FAR * SCREENCOLORP;

typedef struct 
{
	WORD	Pattern[NCOLORS];
} PRINTERCOLOR;
typedef PRINTERCOLOR FAR * PRINTERCOLORP;

typedef struct
{
	BYTE	Color[NPLOTTER]; 
	BYTE	Pattern[NPLOTTER];
	BYTE	Both;
} PLOTTERCOLOR;
typedef PLOTTERCOLOR FAR * PLOTTERCOLORP;

typedef  struct
{
   WORD  wVersion;
   WORD  wScreenSize;
   WORD  wPrinterSize;
   WORD  wPlotterSize;
   WORD  wLookUpSize;    // number of screen Bitmap Color table entries
   int   iColorNum;
   LONG  reserved[4];
} FILEHEADER;

typedef  struct tagPATCTLBLOCK
{
   char        szID[2];          // unique ID
   HANDLE 	   hScreenColor; 
   HANDLE  	   hPlotterColor;
   HANDLE  	   hPrinterColor;
   OFSTRUCT    ofBuff;
   FILEHEADER  fHeader;
   char        szFileName[MAXFILEBUF + 1];
} PATCTLBLOCK;
typedef  PATCTLBLOCK FAR *LPPATCTLBLOCK;

// Private function
BOOL  PatternLoadBlock(int fh, HANDLE hMem, LONG lOffset, WORD wSize);
BOOL  PatternLoadDiscardedBlock(HANDLE hMem, PATTERNTYPE iWhich);
BOOL  InitColorBitmap (void);

extern   DWORD PlotColors[];
extern   short PlotPatterns[];
extern   DWORD ScreenColors[];
#endif

#ifndef  DRV_MEM

#define  DRV_MEM

HGLOBAL  DLLENTRY tbMemAlloc    (HGLOBAL, DWORD, WORD);
BOOL     DLLENTRY tbMemFree     (HGLOBAL);
BOOL     DLLENTRY tbMemFreeLock (HGLOBAL);
BOOL     DLLENTRY tbMemFreeWire (HGLOBAL);
BOOL     DLLENTRY tbMemUnlock   (HGLOBAL);
BOOL     DLLENTRY tbMemUnWire   (HGLOBAL);
LPVOID   DLLENTRY tbMemLock     (HGLOBAL);
LPVOID   DLLENTRY tbMemWire     (HGLOBAL);

#endif

#ifdef __cplusplus
}
#endif

#endif // __LIBGEN_H
