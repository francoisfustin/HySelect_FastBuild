/************************************************************************/
/* FPVBSUB.H  FarPoint VBAPI.H substitution file                        */
/************************************************************************/
#ifndef __FPVBSUB_H__
#define __FPVBSUB_H__

#if (defined(FP_OCX) || defined(FP_DLL))

// this should be done in the Project, if using MFC for 16-bit OCX
// this define is an extra fail-safe to make sure there are no re-definitions
#ifndef NO_VBX_SUPPORT
#define NO_VBX_SUPPORT
#endif

// following defines borrowed from VBAPI.H for common code support

//---------------------------------------------------------------------------
// Handle C++ and Pack structures on 1-byte boundaries
//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif  // ifndef RC_INVOKED


//---------------------------------------------------------------------------
// Version constants
//---------------------------------------------------------------------------
#define VB100_VERSION   0x0100
#define VB200_VERSION   0x0200
#define VB300_VERSION   0x0300
#define VB_VERSION  VB300_VERSION

//---------------------------------------------------------------------------
// General types and modifiers
//---------------------------------------------------------------------------
typedef   signed short  SHORT;      // s
typedef unsigned short  USHORT;     // u

#ifndef VC50
typedef unsigned int    FSHORT;     // fs
#endif

#ifndef WIN32
typedef   signed char   CHAR;       // ch
#endif

typedef unsigned char   ENUM;       // enum
typedef unsigned long   ULONG;      // ul
typedef unsigned long   FLONG;      // fl

typedef SHORT      FAR *LPSHORT;    // lps
typedef USHORT     FAR *LPUSHORT;   // lpu
typedef USHORT          IPROP;      // iprop
typedef DWORD           HFORMFILE;  // hformfile        // must not be HANDLE!!!

#ifndef __cplusplus
typedef USHORT          ERR;        // err  
#define ERR_DEFINED
#endif

typedef WORD        HPIC;           // hpic
#ifndef _INC_DDEMLH
typedef LPTSTR      HSZ;            // hsz
#endif

typedef HPIC   FAR *LPHPIC;         // lphpic   

typedef LPVOID      HLSTR;          // hlstr
typedef HLSTR      FAR *LPHLSTR;    // lphlstr
typedef LPVOID      HAD;            // hAD - handle to Basic array desc.
typedef LPVOID      HCTL;           // hctl
typedef LONG (FAR PASCAL *PFNWP)(HWND hwnd, USHORT msg, USHORT wp, LONG lp);            // pfnwp
typedef LONG (FAR PASCAL *PCTLPROC)(HCTL hctl, HWND hwnd, USHORT msg, USHORT wp, LONG lp);  // pctlproc


//---------------------------------------------------------------------------
// Language Variant structures
//---------------------------------------------------------------------------
#ifndef __cplusplus
#ifndef _tagCY_DEFINED
typedef struct
  {
  LONG lo;
  LONG hi;
  } CURRENCY;
#endif

typedef union
  {
  SHORT    i2;
  LONG     i4;
  float    r4;
  double   r8;
  CURRENCY cy;
  HLSTR    hlstr;
  } VALUE;

typedef VALUE FAR *LPVAL;
#else
typedef LPVOID  *LPVAL;
#endif  // __cplusplus


// The size of Variant is needed when a Variant is passed by value or in a
// structure (user-defined type).
#ifndef __cplusplus
#if (!(defined(_tagVARIANT_DEFINED) || defined(__tagVARIANT)))
typedef struct
  {
  char v[16];
  } VARIANT;
#endif
#endif // __cplusplus
//#ifndef FP_DLL
#if (defined(FP_OCX)) //not FP_DLL   -SCP
typedef VARIANT FAR *LPVAR;
#endif

//
// Variant type constants. these are the return value of GetVariantType.
// these are also defined in Constant.Txt for return value of VarType()
 // Added because wtypes.h defines these types and 
#if !defined(__wtypes_h__) && !defined(SD_V30)

			   // wtypes.h is being included for the designer
#define VT_EMPTY    0   // Empty
#define VT_NULL     1   // Null
#define VT_I2       2   // Integer
#define VT_I4       3   // Long
#define VT_R4       4   // Single
#define VT_R8       5   // Double
#define VT_DATE     7   // Date
#endif // SD_V30

#define VT_CURRENCY 6   // Currency
#define VT_STRING   8   // String

//---------------------------------------------------------------------------
// COLOR - a Windows RGB DWORD value.
//---------------------------------------------------------------------------
typedef unsigned long COLOR;
typedef COLOR    FAR *LPCOLOR;        // lpcolor

#define COLOR_DEFBITON  0x80000000  // bit set -> Win SysColor, not RGB

// Get RGB value from a color.
#define RGBCOLOR(clr) \
  (((clr)&COLOR_DEFBITON)?(GetSysColor((int)((clr)&0xFFFF))):((clr)&0xFFFFFF))


//---------------------------------------------------------------------------
// Property data structure
//---------------------------------------------------------------------------
typedef struct tagDATASTRUCT
  {
  LONG   data;          // Data for Get/Set
  USHORT cindex;        // Number of indecies
  struct
    {
    USHORT datatype;        // Type of nth index (Currently always DT_SHORT)
    LONG   data;        // Value of nth index
    } index[1];         // Currently, only 1-dim arrays supported
  } DATASTRUCT;

typedef DATASTRUCT FAR  *LPDATASTRUCT;  // pds
typedef DATASTRUCT NEAR *NPDATASTRUCT;  // npds


//---------------------------------------------------------------------------
// VISUAL BASIC variable types
//---------------------------------------------------------------------------
#define ET_I2       1   // 16 bit signed integer scalar or array variable
#define ET_I4       2   // 32 bit signed integer scalar or array variable
#define ET_R4       3
#define ET_R8       4   // 64 bit real scalar or array variable
#define ET_CY       5   // 64 bit currency scalar or array variable
#define ET_HLSTR    6   // string scalar or array variable
#define ET_SD       6   // older, unpreferred equivalent to ET_HLSTR
#define ET_FS       7   // fixed-length string variable


//---------------------------------------------------------------------------
// Control Property definitions and structures.
//---------------------------------------------------------------------------
#define PF_datatype     0x000000FFL
#define PF_fPropArray       0x00000100L
#define PF_fSetData     0x00000200L
#define PF_fSetMsg      0x00000400L
#define PF_fNoShow      0x00000800L
#define PF_fNoRuntimeW      0x00001000L
#define PF_fGetData     0x00002000L
#define PF_fGetMsg      0x00004000L
#define PF_fSetCheck        0x00008000L
#define PF_fSaveData        0x00010000L
#define PF_fSaveMsg     0x00020000L
#define PF_fLoadDataOnly    0x20010000L
#define PF_fLoadMsgOnly     0x20020000L
#define PF_fGetHszMsg       0x00040000L
#define PF_fUpdateOnEdit    0x00080000L
#define PF_fEditable        0x00100000L
#define PF_fPreHwnd     0x00200000L
#define PF_fDefVal      0x00400000L
#define PF_fNoInitDef       0x00800000L
#define PF_fNoRuntimeR      0x02000000L
#define PF_fNoMultiSelect   0x04000000L

// This define is used in the structure "PROPINFO".
#ifdef WIN32
#define PTSTR  LPTSTR
#else // WIN16
#define PTSTR  PSTR
#endif

typedef struct tagPROPINFO
  {
  PTSTR  npszName;
  FLONG fl;         // PF_ flags
  BYTE  offsetData;     // Offset into static structure
  BYTE  infoData;       // 0 or _INFO value for bitfield
  LONG  dataDefault;        // 0 or _INFO value for bitfield
  PTSTR  npszEnumList;       // For type == DT_ENUM, this is
                // a near ptr to a string containing
                // all the values to be displayed
                // in the popup enumeration listbox.
                // Each value is an sz, with an
                // empty sz indicated the end of list.
  BYTE  enumMax;        // Maximum legal value for enum.
  } PROPINFO;

typedef PROPINFO  FAR  * LPPROPINFO;
typedef PROPINFO  NEAR * NPPROPINFO;
typedef PROPINFO  NEAR *  PPROPINFO;
typedef PPROPINFO FAR  * LPPROPLIST;
typedef PPROPINFO NEAR * NPPROPLIST;
typedef PPROPINFO NEAR *  PPROPLIST;


//---------------------------------------------------------------------------
// Values for PROP PF_datatype
//---------------------------------------------------------------------------
#define DT_HSZ        0x01
#define DT_SHORT      0x02
#define DT_LONG       0x03
#define DT_BOOL       0x04
#define DT_COLOR      0x05
#define DT_ENUM       0x06
#define DT_REAL       0x07
#define DT_XPOS       0x08  // Scaled from float to long twips
#define DT_XSIZE      0x09  //   _SIZE scales without origin
#define DT_YPOS       0x0A  //   _POS subtracts origin
#define DT_YSIZE      0x0B  // uses parent's scale properties
#define DT_PICTURE    0x0C
#define DT_HLSTR      0x0D
#ifdef FP_OCX
  #define DT_FP_REAL8    0x0E  // This type is a FP custom type - SCP
  #define DT_FP_DATETIME 0x0F  // This type is a FP custom type - SCP
#endif
#define DT_OBJECT     0x20


//---------------------------------------------------------------------------
// Control event definitions and structures.
//---------------------------------------------------------------------------
#define EF_fNoUnload    0x00000001L

#ifndef _CTRLOBJ_H_
typedef struct tagEVENTINFO
  {
  PTSTR     npszName;   // event procedure name suffix
  USHORT    cParms;     // number of parameters
  USHORT    cwParms;    // # words of parameters
  PWORD     npParmTypes;    // list of parameter types
  PTSTR     npszParmProf;   // event parameter profile string
  FLONG     fl;     // EF_ flags
  } EVENTINFO;

#endif	//  _CTRLOBJ_H_

typedef EVENTINFO   NEAR * PEVENTINFO;
typedef PEVENTINFO  NEAR * PPEVENTINFO;
typedef EVENTINFO   NEAR * NPEVENTINFO;
typedef EVENTINFO   FAR  * LPEVENTINFO;
typedef NPEVENTINFO FAR  * LPEVENTLIST;
typedef NPEVENTINFO NEAR * NPEVENTLIST;


//---------------------------------------------------------------------------
// Standard control event list
//---------------------------------------------------------------------------
#define IEVENT_STD_CLICK    0x0000
#define IEVENT_STD_DBLCLICK 0x0001
#define IEVENT_STD_DRAGDROP 0x0002
#define IEVENT_STD_DRAGOVER 0x0003
#define IEVENT_STD_GOTFOCUS 0x0004
#define IEVENT_STD_KEYDOWN  0x0005
#define IEVENT_STD_KEYPRESS 0x0006
#define IEVENT_STD_KEYUP    0x0007
#define IEVENT_STD_LOSTFOCUS    0x0008
#define IEVENT_STD_MOUSEDOWN    0x0009
#define IEVENT_STD_MOUSEMOVE    0x000A
#define IEVENT_STD_MOUSEUP  0x000B
#define IEVENT_STD_LINKERROR    0x000C
#define IEVENT_STD_LINKOPEN     0x000D
#define IEVENT_STD_LINKCLOSE    0x000E
#define IEVENT_STD_LINKNOTIFY   0x000F
#define IEVENT_STD_LINKCHANGE   0x0010
#define IEVENT_STD_NONE     0x0FFF
#define IEVENT_STD_LAST     0x0FFF

#define PEVENTINFO_STD_CLICK        ((PEVENTINFO)~IEVENT_STD_CLICK)
#define PEVENTINFO_STD_DBLCLICK     ((PEVENTINFO)~IEVENT_STD_DBLCLICK)
#define PEVENTINFO_STD_DRAGDROP     ((PEVENTINFO)~IEVENT_STD_DRAGDROP)
#define PEVENTINFO_STD_DRAGOVER     ((PEVENTINFO)~IEVENT_STD_DRAGOVER)
#define PEVENTINFO_STD_GOTFOCUS     ((PEVENTINFO)~IEVENT_STD_GOTFOCUS)
#define PEVENTINFO_STD_KEYDOWN      ((PEVENTINFO)~IEVENT_STD_KEYDOWN)
#define PEVENTINFO_STD_KEYPRESS     ((PEVENTINFO)~IEVENT_STD_KEYPRESS)
#define PEVENTINFO_STD_KEYUP        ((PEVENTINFO)~IEVENT_STD_KEYUP)
#define PEVENTINFO_STD_LOSTFOCUS    ((PEVENTINFO)~IEVENT_STD_LOSTFOCUS)
#define PEVENTINFO_STD_MOUSEDOWN    ((PEVENTINFO)~IEVENT_STD_MOUSEDOWN)
#define PEVENTINFO_STD_MOUSEMOVE    ((PEVENTINFO)~IEVENT_STD_MOUSEMOVE)
#define PEVENTINFO_STD_MOUSEUP      ((PEVENTINFO)~IEVENT_STD_MOUSEUP)
#define PEVENTINFO_STD_LINKERROR    ((PEVENTINFO)~IEVENT_STD_LINKERROR)
#define PEVENTINFO_STD_LINKOPEN     ((PEVENTINFO)~IEVENT_STD_LINKOPEN)
#define PEVENTINFO_STD_LINKCLOSE    ((PEVENTINFO)~IEVENT_STD_LINKCLOSE)
#define PEVENTINFO_STD_LINKNOTIFY   ((PEVENTINFO)~IEVENT_STD_LINKNOTIFY)
#define PEVENTINFO_STD_LINKCHANGE   ((PEVENTINFO)~IEVENT_STD_LINKCHANGE)
#define PEVENTINFO_STD_NONE     ((PEVENTINFO)~IEVENT_STD_NONE)
#define PEVENTINFO_STD_LAST     ((PEVENTINFO)~IEVENT_STD_LAST)


//---------------------------------------------------------------------------
// Standard control properties
//---------------------------------------------------------------------------
#if defined(_WIN64) || defined(_IA64)
#define IPROP_STD_NAME           (LONG_PTR)0x0000
#define IPROP_STD_CTLNAME        IPROP_STD_NAME
#define IPROP_STD_INDEX          (LONG_PTR)0x0001
#define IPROP_STD_HWND           (LONG_PTR)0x0002
#define IPROP_STD_BACKCOLOR      (LONG_PTR)0x0003
#define IPROP_STD_FORECOLOR      (LONG_PTR)0x0004
#define IPROP_STD_LEFT           (LONG_PTR)0x0005
#define IPROP_STD_TOP            (LONG_PTR)0x0006
#define IPROP_STD_WIDTH          (LONG_PTR)0x0007
#define IPROP_STD_HEIGHT         (LONG_PTR)0x0008
#define IPROP_STD_ENABLED        (LONG_PTR)0x0009
#define IPROP_STD_VISIBLE        (LONG_PTR)0x000A
#define IPROP_STD_MOUSEPOINTER   (LONG_PTR)0x000B
#define IPROP_STD_CAPTION        (LONG_PTR)0x000C
#define IPROP_STD_FONTNAME       (LONG_PTR)0x000D
#define IPROP_STD_FONTBOLD       (LONG_PTR)0x000E
#define IPROP_STD_FONTITALIC     (LONG_PTR)0x000F
#define IPROP_STD_FONTSTRIKE     (LONG_PTR)0x0010
#define IPROP_STD_FONTUNDER      (LONG_PTR)0x0011
#define IPROP_STD_FONTSIZE       (LONG_PTR)0x0012
#define IPROP_STD_TABINDEX       (LONG_PTR)0x0013
#define IPROP_STD_PARENT         (LONG_PTR)0x0014
#define IPROP_STD_DRAGMODE       (LONG_PTR)0x0015
#define IPROP_STD_DRAGICON       (LONG_PTR)0x0016
#define IPROP_STD_BORDERSTYLEOFF (LONG_PTR)0x0017
#define IPROP_STD_TABSTOP        (LONG_PTR)0x0018
#define IPROP_STD_TAG            (LONG_PTR)0x0019
#define IPROP_STD_TEXT           (LONG_PTR)0x001B
#define IPROP_STD_BORDERSTYLEON  (LONG_PTR)0x001C
#define IPROP_STD_CLIPCONTROLS   (LONG_PTR)0x001D
#define IPROP_STD_NONE           (LONG_PTR)0x001E
#define IPROP_STD_HELPCONTEXTID  (LONG_PTR)0x001F
#define IPROP_STD_LINKMODE       (LONG_PTR)0x0020
#define IPROP_STD_LINKITEM       (LONG_PTR)0x0021
#define IPROP_STD_LINKTOPIC      (LONG_PTR)0x0022
#define IPROP_STD_LINKTIMEOUT    (LONG_PTR)0x0023
#define IPROP_STD_LEFTNORUN      (LONG_PTR)0x0024
#define IPROP_STD_TOPNORUN       (LONG_PTR)0x0025
#define IPROP_STD_ALIGN          (LONG_PTR)0x0026
#define IPROP_STD_IMEMODE        (LONG_PTR)0x0027
#define IPROP_STD_DATASOURCE     (LONG_PTR)0x0028
#define IPROP_STD_DATAFIELD      (LONG_PTR)0x0029
#define IPROP_STD_DATACHANGED    (LONG_PTR)0x002A
#define IPROP_STD_LAST           (LONG_PTR)0x0FFF
#else
#define IPROP_STD_NAME           (UINT)0x0000
#define IPROP_STD_CTLNAME        IPROP_STD_NAME
#define IPROP_STD_INDEX          (UINT)0x0001
#define IPROP_STD_HWND           (UINT)0x0002
#define IPROP_STD_BACKCOLOR      (UINT)0x0003
#define IPROP_STD_FORECOLOR      (UINT)0x0004
#define IPROP_STD_LEFT           (UINT)0x0005
#define IPROP_STD_TOP            (UINT)0x0006
#define IPROP_STD_WIDTH          (UINT)0x0007
#define IPROP_STD_HEIGHT         (UINT)0x0008
#define IPROP_STD_ENABLED        (UINT)0x0009
#define IPROP_STD_VISIBLE        (UINT)0x000A
#define IPROP_STD_MOUSEPOINTER   (UINT)0x000B
#define IPROP_STD_CAPTION        (UINT)0x000C
#define IPROP_STD_FONTNAME       (UINT)0x000D
#define IPROP_STD_FONTBOLD       (UINT)0x000E
#define IPROP_STD_FONTITALIC     (UINT)0x000F
#define IPROP_STD_FONTSTRIKE     (UINT)0x0010
#define IPROP_STD_FONTUNDER      (UINT)0x0011
#define IPROP_STD_FONTSIZE       (UINT)0x0012
#define IPROP_STD_TABINDEX       (UINT)0x0013
#define IPROP_STD_PARENT         (UINT)0x0014
#define IPROP_STD_DRAGMODE       (UINT)0x0015
#define IPROP_STD_DRAGICON       (UINT)0x0016
#define IPROP_STD_BORDERSTYLEOFF (UINT)0x0017
#define IPROP_STD_TABSTOP        (UINT)0x0018
#define IPROP_STD_TAG            (UINT)0x0019
#define IPROP_STD_TEXT           (UINT)0x001B
#define IPROP_STD_BORDERSTYLEON  (UINT)0x001C
#define IPROP_STD_CLIPCONTROLS   (UINT)0x001D
#define IPROP_STD_NONE           (UINT)0x001E
#define IPROP_STD_HELPCONTEXTID  (UINT)0x001F
#define IPROP_STD_LINKMODE       (UINT)0x0020
#define IPROP_STD_LINKITEM       (UINT)0x0021
#define IPROP_STD_LINKTOPIC      (UINT)0x0022
#define IPROP_STD_LINKTIMEOUT    (UINT)0x0023
#define IPROP_STD_LEFTNORUN      (UINT)0x0024
#define IPROP_STD_TOPNORUN       (UINT)0x0025
#define IPROP_STD_ALIGN          (UINT)0x0026
#define IPROP_STD_IMEMODE        (UINT)0x0027
#define IPROP_STD_DATASOURCE     (UINT)0x0028
#define IPROP_STD_DATAFIELD      (UINT)0x0029
#define IPROP_STD_DATACHANGED    (UINT)0x002A
#define IPROP_STD_LAST           (UINT)0x0FFF
#endif

#define PPROPINFO_STD_NAME      ((PPROPINFO)~IPROP_STD_NAME)
#define PPROPINFO_STD_CTLNAME       ((PPROPINFO)~IPROP_STD_NAME)
#define PPROPINFO_STD_INDEX     ((PPROPINFO)~IPROP_STD_INDEX)
#define PPROPINFO_STD_HWND      ((PPROPINFO)~IPROP_STD_HWND)
#define PPROPINFO_STD_BACKCOLOR     ((PPROPINFO)~IPROP_STD_BACKCOLOR)
#define PPROPINFO_STD_FORECOLOR     ((PPROPINFO)~IPROP_STD_FORECOLOR)
#define PPROPINFO_STD_LEFT      ((PPROPINFO)~IPROP_STD_LEFT)
#define PPROPINFO_STD_TOP       ((PPROPINFO)~IPROP_STD_TOP)
#define PPROPINFO_STD_WIDTH     ((PPROPINFO)~IPROP_STD_WIDTH)
#define PPROPINFO_STD_HEIGHT        ((PPROPINFO)~IPROP_STD_HEIGHT)
#define PPROPINFO_STD_ENABLED       ((PPROPINFO)~IPROP_STD_ENABLED)
#define PPROPINFO_STD_VISIBLE       ((PPROPINFO)~IPROP_STD_VISIBLE)
#define PPROPINFO_STD_MOUSEPOINTER  ((PPROPINFO)~IPROP_STD_MOUSEPOINTER)
#define PPROPINFO_STD_CAPTION       ((PPROPINFO)~IPROP_STD_CAPTION)
#define PPROPINFO_STD_FONTNAME      ((PPROPINFO)~IPROP_STD_FONTNAME)
#define PPROPINFO_STD_FONTBOLD      ((PPROPINFO)~IPROP_STD_FONTBOLD)
#define PPROPINFO_STD_FONTITALIC    ((PPROPINFO)~IPROP_STD_FONTITALIC)
#define PPROPINFO_STD_FONTSTRIKE    ((PPROPINFO)~IPROP_STD_FONTSTRIKE)
#define PPROPINFO_STD_FONTUNDER     ((PPROPINFO)~IPROP_STD_FONTUNDER)
#define PPROPINFO_STD_FONTSIZE      ((PPROPINFO)~IPROP_STD_FONTSIZE)
#define PPROPINFO_STD_TABINDEX      ((PPROPINFO)~IPROP_STD_TABINDEX)
#define PPROPINFO_STD_PARENT        ((PPROPINFO)~IPROP_STD_PARENT)
#define PPROPINFO_STD_DRAGMODE      ((PPROPINFO)~IPROP_STD_DRAGMODE)
#define PPROPINFO_STD_DRAGICON      ((PPROPINFO)~IPROP_STD_DRAGICON)
#define PPROPINFO_STD_BORDERSTYLEOFF ((PPROPINFO)~IPROP_STD_BORDERSTYLEOFF)
#define PPROPINFO_STD_TABSTOP       ((PPROPINFO)~IPROP_STD_TABSTOP)
#define PPROPINFO_STD_TAG       ((PPROPINFO)~IPROP_STD_TAG)
#define PPROPINFO_STD_TEXT      ((PPROPINFO)~IPROP_STD_TEXT)
#define PPROPINFO_STD_BORDERSTYLEON ((PPROPINFO)~IPROP_STD_BORDERSTYLEON)
#define PPROPINFO_STD_CLIPCONTROLS  ((PPROPINFO)~IPROP_STD_CLIPCONTROLS)
#define PPROPINFO_STD_NONE      ((PPROPINFO)~IPROP_STD_NONE)
#define PPROPINFO_STD_HELPCONTEXTID ((PPROPINFO)~IPROP_STD_HELPCONTEXTID)
#define PPROPINFO_STD_LINKMODE      ((PPROPINFO)~IPROP_STD_LINKMODE)
#define PPROPINFO_STD_LINKITEM      ((PPROPINFO)~IPROP_STD_LINKITEM)
#define PPROPINFO_STD_LINKTOPIC     ((PPROPINFO)~IPROP_STD_LINKTOPIC)
#define PPROPINFO_STD_LINKTIMEOUT   ((PPROPINFO)~IPROP_STD_LINKTIMEOUT)
#define PPROPINFO_STD_LEFTNORUN     ((PPROPINFO)~IPROP_STD_LEFTNORUN)
#define PPROPINFO_STD_TOPNORUN      ((PPROPINFO)~IPROP_STD_TOPNORUN)
#define PPROPINFO_STD_ALIGN     ((PPROPINFO)~IPROP_STD_ALIGN)
#define PPROPINFO_STD_IMEMODE       ((PPROPINFO)~IPROP_STD_IMEMODE)
#define PPROPINFO_STD_DATASOURCE    ((PPROPINFO)~IPROP_STD_DATASOURCE)
#define PPROPINFO_STD_DATAFIELD     ((PPROPINFO)~IPROP_STD_DATAFIELD)
#define PPROPINFO_STD_DATACHANGED   ((PPROPINFO)~IPROP_STD_DATACHANGED)
#define PPROPINFO_STD_LAST          ((PPROPINFO)~IPROP_STD_LAST)

//---------------------------------------------------------------------------
// Structure for VBM_DRAGOVER and VBM_DRAGDROP events
//---------------------------------------------------------------------------
//typedef struct tagDRAGINFO
//  {
//  HCTL   hctl;
//  POINT  pt;
//  USHORT state;     // Enter, Over, Exit; only used for VBM_DRAGOVER
//  } DRAGINFO;

//typedef DRAGINFO NEAR * NPDRAGINFO;
//typedef DRAGINFO FAR  * LPDRAGINFO;

//#define DRAG_STATE_ENTER       0x00
//#define DRAG_STATE_EXIT        0x01
//#define DRAG_STATE_OVER        0x02


//---------------------------------------------------------------------------
// Control MODEL structure
//---------------------------------------------------------------------------
typedef struct tagMODEL
  {
  USHORT		usVersion;				// VB version used by control
  FLONG			fl;						// Bitfield structure
  PCTLPROC		pctlproc;				// The control proc.
  FSHORT		fsClassStyle;			// Window class style
  FLONG			flWndStyle;				// Default window style
  USHORT		cbCtlExtra;				// # bytes alloc'd for HCTL structure
  WORD			idBmpPalette;			// BITMAP id for tool palette
  PSTR			npszDefCtlName;			// Default control name prefix
  PSTR			npszClassName;			// Visual Basic class name
  PSTR			npszParentClassName;    // Parent window class if subclassed
  NPPROPLIST    npproplist;				// Property list
  NPEVENTLIST   npeventlist;			// Event list
  BYTE			nDefProp;				// Index of default property
  BYTE			nDefEvent;				// Index of default event
  BYTE			nValueProp;				// Index of control value property
  USHORT        usCtlVersion;           // Identifies the current version of
                                        // the custom control. The values
                                        // 1 and 2 are reserved for custom
                                        // controls created with VB 1.0 and
                                        // VB 2.0.
  } MODEL;

typedef MODEL FAR  *  LPMODEL;

#define MODEL_fArrows       0x00000001L
#define MODEL_fFocusOk      0x00000002L
#define MODEL_fMnemonic     0x00000004L
#define MODEL_fChildrenOk   0x00000008L
#define MODEL_fInitMsg      0x00000010L
#define MODEL_fLoadMsg      0x00000020L
#define MODEL_fDesInteract  0x00000040L
#define MODEL_fInvisAtRun   0x00000080L
#define MODEL_fGraphical    0x00000100L


//---------------------------------------------------------------------------
// MODELINFO structure
//---------------------------------------------------------------------------
typedef struct tagMODELINFO
  {
  USHORT    usVersion;      // VB version used by control
  LPMODEL FAR  *lplpmodel;      // pointer to null-terminated
  } MODELINFO;              //    list of LPMODELS

typedef MODELINFO FAR *LPMODELINFO;


//---------------------------------------------------------------------------
// Picture structure
//---------------------------------------------------------------------------
typedef struct tagPIC
  {
  BYTE picType;
  union
    {
    struct
      {
      HBITMAP   hbitmap;        // Bitmap
      HPALETTE  hpal;           // Accompanying palette
      } bmp;
    struct
      {
      HANDLE    hmeta;          // Metafile
      int   xExt, yExt;     // Extent
      } wmf;
    struct
      {
      HICON hicon;          // Icon
      } icon;
    } picData;
  BYTE picReserved[4];
  } PIC;

typedef PIC FAR *LPPIC;

#define PICTYPE_NONE        0
#define PICTYPE_BITMAP      1
#define PICTYPE_METAFILE    2
#define PICTYPE_ICON        3

#define HPIC_INVALID        0xFFFF
#define HPIC_NULL       0
//#endif


//---------------------------------------------------------------------------
// Mode constants
//---------------------------------------------------------------------------
#define MODE_DESIGN 1
#define MODE_RUN    2
#define MODE_BREAK  3


//---------------------------------------------------------------------------
// Control flags for use with VBSetControlFlags
//---------------------------------------------------------------------------
#define CTLFLG_HASPALETTE         0x00000001L
#define CTLFLG_USESPALETTE        0x00000002L
#define CTLFLG_GRAPHICALOPAQUE        0x00000004L
#define CTLFLG_GRAPHICALTRANSLUCENT   0x00000008L
#define CTLFLG_DATACHANGED        0x00000010L
#define CTLFLG_BOUNDDATASET       0x00000020L


#ifndef __cplusplus
//---------------------------------------------------------------------------
// Data xfer structure for VBM_LINKGETDATA or VBM_LINKSETDATA
//---------------------------------------------------------------------------
typedef struct tagVBLINKDATA
  {
  WORD     wReserved;
  DWORD    cb;        // Size of data
  HANDLE   hData;     // Handle to data
  DWORD    dwReserved;
  } VBLINKDATA;

typedef VBLINKDATA FAR *LPLINKDATA;


//---------------------------------------------------------------------------
// Link modes
//---------------------------------------------------------------------------
#define LINKMODENONE            0
#define LINKMODEAUTO            1
#define LINKMODEMANUAL          2
#define LINKMODENOTIFY                  3

#define LINKMODESERVERPOKE      1
#define LINKMODESERVERNOPOKE        2

#define VBDDEDATA VBLINKDATA
#define LPDDEDATA LPLINKDATA


//---------------------------------------------------------------------------
// Misc DDE defines
//---------------------------------------------------------------------------
#define MAX_EXEC                        256
#define MAXLINKITEMNAME                 256

#define ACCEPTSDATAFORMAT                1
#define SUPPLIESDATAFORMAT               2

#define LINKSRCASK                       0
#define LINKSRCTELL                      1

#define LINK_DATA_OK             0
#define LINK_DATA_OOM            1
#define LINK_DATA_FORMATBAD      2
#define LINK_DATA_SETFAILED      3

#endif    //  __cplusplus

#define errInitOnNonServerDesk   6  // Attempted to Initate on a channel that
                    // had linkmode set to None.
#define errAllServerChUsed       7  // The Server Channel array is filled.
#define errDataSettingFailed     8  // The Set quality failed possible string
                    // too long.
#define errOutOfMemoryForLink   11

//---------------------------------------------------------------------------
// Bound control interface
//---------------------------------------------------------------------------

// values for the sAction parameter on VBM_DATA_GET messages.

#define DATA_FIELDVALUE      0
#define DATA_FIELDNAME       1
#define DATA_FIELDTYPE       2
#define DATA_FIELDSIZE       3
#define DATA_FIELDSCOUNT     4
#define DATA_FIELDCHUNK      5
#define DATA_FIELDPOSITION   6
#define DATA_FIELDATTRIBUTES 7

#define DATA_BOF         16
#define DATA_EOF         17
#define DATA_BOOKMARK        18
#define DATA_BOOKMARKABLE    19
#define DATA_UPDATABLE       20
#define DATA_RECORDCOUNT     21
#define DATA_LASTMODIFIED    22

// sAction values during VBM_DATA_AVAILABLE messages

#define DATA_MOVEFIRST        64
#define DATA_MOVENEXT         65
#define DATA_MOVEPREV         66
#define DATA_MOVELAST         67
#define DATA_FINDFIRST        68
#define DATA_FINDNEXT         69
#define DATA_FINDPREV         70
#define DATA_FINDLAST         71
#define DATA_DELETE       72
#define DATA_ADDNEW       73
#define DATA_REFRESH          74
#define DATA_ROLLBACK         76
#define DATA_CLOSE        77
#define DATA_DATAFIELDCHANGED 78
#define DATA_SAVEDATA         79
#define DATA_READDATA         80
#define DATA_UPDATE       81
#define DATA_UNLOAD       82

//DBOCX...
// User-defined actions (DATA_U...)
#define DATA_UDATAFIELD          200       // use with VBM_DATA_SET
#define DATA_UCTLFLAGS           201       // use with VBM_DATA_GET/SET
#define DATA_UFINISHUPDATE       202       // use with VBM_DATA_SET

#define DATA_UREADENABLED        203       // if supported, lData = TRUE
#define DATA_UREADFIRST          204
#define DATA_UREADNEXT           205
#define DATA_UREADFIELDVALUE     206
#define DATA_UREADFIELDSIZE      207
#define DATA_UREADBOOKMARK       208

#define DATA_URESETREAD          209       // invalidates read buffer

// These "actions" (used with VBM_DATA_GET) can be used to get the
// bookmarks associated with rows which are Added/Deleted/Changed.
// The actions are used when processing a VBM_DATA_AVAILABLE message 
// (event) with the "action" set to DATA_ADDNEW/DELETE/UPDATE.
// 
// With DATA_DELETE, any number of rows may have been deleted and
// they may or may not include the current row. DATA_UEVENTBMKCOUNT is
// used to query the number of rows deleted.  DATA_UEVENTBMK is used
// to retrieve the bookmarks. If there is more than one bookmark, then
// the index of the one you want retrieved is passed in "sDataFieldIndex"
// of the DATAACCESS structure. Typically, you would query the count
// with DATA_UEVENTBMKCOUNT and then loop that many times, each time
// retrieving the bookmark with DATA_UEVENTBMK.
#define DATA_UEVENTBMKCOUNT      210 // Count of available bookmarks
#define DATA_UEVENTBMK           211 // specify index in ".sDataFieldIndex"

#define DATA_UEVENTCOLCOUNT      212 // Count of available bookmarks
#define DATA_UEVENTCOL           213 // specify index in ".sDataFieldIndex"

#define DATA_UCOLUPDATE          214 // Column value has been updated
#define DATA_UREPLACEBMK         215 // Replacement bookmark


//DBOCX

// return types for VBM_DATA_GET with sAction = DATA_FIELDTYPE

#define DATA_VT_BOOL       1
#define DATA_VT_VAR_BYTE   2
#define DATA_VT_INTEGER    3
#define DATA_VT_LONG       4
#define DATA_VT_CURRENCY   5
#define DATA_VT_SINGLE     6
#define DATA_VT_DOUBLE     7
#define DATA_VT_DATETIME   8
#define DATA_VT_TEXT       10
#define DATA_VT_BINARY     11
#define DATA_VT_MEMO       12

//DBOCX...
// "Extended" types, found in HIWORD(lData)
#define UDATA_VT_WMEMO    100 
#define UDATA_VT_WTEXT    101 
//DBOCX

// VBM_DATA_GET with sAction = DATA_BOOKMARK fill in ldata with one of these

#define     DATA_BOOKMARKFIRST    ((LONG)0x80000000L)
#define     DATA_BOOKMARKPREV    -1L
#define     DATA_BOOKMARKCURRENT  0L
#define     DATA_BOOKMARKNEXT     1L
#define     DATA_BOOKMARKLAST     ((LONG)0x7FFFFFFFL)



//
#define DA_fNull    0x0001
#define DA_fBOF     0x0002
#define DA_fEOF     0x0004


//---------------------------------------------------------------------------
// DA structure used for conversations with the data control
//---------------------------------------------------------------------------
typedef struct
{
  USHORT  usVersion;          // VB version of structure filled in when
                  // structure is created
  SHORT   sAction;        // on VBM_DATA_GET/SET specifies what to get/set
                  // on VBM_DATA_AVAILABLE/REQUEST tells why
  HCTL    hctlData;       // the data control providing data
  HCTL    hctlBound;          // the bound control receiving data
  HSZ     hszDataField;       // the name of the field to get value of
  SHORT   sDataFieldIndex;    // the field index used when FieldName is null
  USHORT  usDataType;         // the property datatype to convert data to
  HLSTR   hlstrBookMark;      // used when getting multirow data
  FSHORT  fs;             // Bitfield structure
  LONG    lData;          // the data
  ULONG   ulChunkOffset;      // the offset to start at for GetChunk
  ULONG   ulChunkNumBytes;    // the number of bytes for GetChunk/SetChunk
  LPVOID  lpfpOleBinding;     // ptr to fpOleBinding object
} DATAACCESS, FAR * LPDATAACCESS;




//---------------------------------------------------------------------------
#define CONTROLDEREF(type, hctl)    ((P##type)(fpVBDerefControl(hctl)))
//#define FPLIB       FAR PASCAL _export

#define GC_FIRSTSIBLING       0x0000
#define GC_LASTSIBLING        0x0001
#define GC_NEXTSIBLING        0x0002
#define GC_PREVSIBLING        0x0003
#define GC_CHILD          0x0005
#define GC_CONTAINER          0x1000
#define GC_FORM           0x1001
#define GC_FIRSTCONTROL       0x1002
#define GC_NEXTCONTROL        0x1003
#define GC_FIRSTSELECTED      0x1004
#define GC_NEXTSELECTED       0x1005

#define ZORDER_FRONT    0
#define ZORDER_BACK 1

#define AB_INVALIDINDEX   1           // hAD or index is bad
#define LOBOUND(x)    ((SHORT)((x) & 0xFFFF))
#define HIBOUND(x)    ((SHORT)((x) >> 16))


//------------------------------
// Firing Basic event procedures
//------------------------------
#define EVENTNOTFIRED   (0x7FFF)



//------------------------
// Exports from .VBX files
//------------------------
//#ifndef NOCCEXPORTS
/*
BOOL FAR PASCAL _export VBINITCC(USHORT usVersion, BOOL fRuntime);
VOID FAR PASCAL _export VBTERMCC(VOID);
LPMODELINFO FAR PASCAL _export VBGetModelInfo(USHORT usVersion);
*/
//#endif // !NOCCEXPORTS



//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Visual Basic messages
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define VBM__BASE       ( WM_USER + 0x0C00 )

// All properties have been loaded.  Must set MODEL_fLoadMsg to receive.
#define VBM_CREATED     ( VBM__BASE + 0x00 )

// Form load is complete or dynamic control is fully loaded.  Must set
// MODEL_fLoadMsg to receive.
#define VBM_LOADED      ( VBM__BASE + 0x01 )

// Pre-hwnd properties are not yet loaded (no hwnd exists for control yet).
// Must set MODEL_fInitMsg to receive.
#define VBM_INITIALIZE      ( VBM__BASE + 0x02 )

// Get a property value.
// [wp=iprop, lp=pdata], RetVal=ERR
#define VBM_GETPROPERTY     ( VBM__BASE + 0x03 )

// Verify a property value.
// [wp=iprop, lp=data], RetVal=ERR
#define VBM_CHECKPROPERTY   ( VBM__BASE + 0x04 )

// Set a property value.
// [wp=iprop, lp=data], RetVal=ERR
#define VBM_SETPROPERTY     ( VBM__BASE + 0x05 )

// Write a property.
// [wp=iprop, lp=pfileref], RetVal=ERR
#define VBM_SAVEPROPERTY    ( VBM__BASE + 0x06 )

// Read a property.
// [wp=iprop, lp=pfileref], RetVal=ERR
#define VBM_LOADPROPERTY    ( VBM__BASE + 0x07 )

// Get the string representation of a property.
// [wp=iprop, lp=HSZ FAR *], RetVal=ERR
#define VBM_GETPROPERTYHSZ  ( VBM__BASE + 0x08 )

// Create popup window used to change a property.
// [wp=iprop, lp=listbox hwnd], RetVal=popup hwnd
#define VBM_INITPROPPOPUP   ( VBM__BASE + 0x09 )

// Determine if Paste/PasteLink is allowed.
// [wp=PASTETYPE], RetVal=BOOL
#define VBM_QPASTEOK        ( VBM__BASE + 0x0A )

#define PT_PASTE    0
#define PT_PASTELINK    1

// Paste control-specific data from the clipboard.
// [wp=PASTETYPE], RetVal=ERR
#define VBM_PASTE       ( VBM__BASE + 0x0B )

// Copy control specific data to the clipboard.
#define VBM_COPY        ( VBM__BASE + 0x0C )

// Control activated upon receipt of a unique mnemonic key.
#define VBM_MNEMONIC        ( VBM__BASE + 0x0D )

// Posted to fire a deferred event.
// [wp=event]
#define VBM_FIREEVENT       ( VBM__BASE + 0x0E )

// Another control in drag mode drug over the control.
// [lp=pdraginfo]
#define VBM_DRAGOVER        ( VBM__BASE + 0x0F )

// This message is sent to a control that has just had another control
// in drag mode dropped on it.
// [lp=pdraginfo]
#define VBM_DRAGDROP        ( VBM__BASE + 0x10 )

// This message is sent when certain methods are invoked on controls
// [wp=METH_, lp=far *alParams], RetVal=ERR
#define VBM_METHOD      ( VBM__BASE + 0x11 )

#define METH_ADDITEM    0x0001  // args are: cArgs, hszItem, [index]
#define METH_REMOVEITEM 0x0002  // args are: cArgs, index
#define METH_REFRESH    0x0003  // no args (lp == NULL)
#define METH_MOVE   0x0004  // args are: cArgs, left, [top, [width, [height]]]
#define METH_DRAG   0x0005  // args are: cArgs, [cmd]
#define METH_LINKSEND   0x0006  // no args (lp == NULL)
#define METH_ZORDER 0x0007  // args are: cArgs, pos
#define METH_CLEAR  0x000A  // no args (lp == NULL)

// Clear capture and internal state.
#define VBM_CANCELMODE      ( VBM__BASE + 0x12 )

// Paint notification for graphical controls.
// [wp=hdc, lp=lprect]
#define VBM_PAINT       ( VBM__BASE + 0x14 )

// Hit test for graphical controls.
// [lp=lphittest], RetVal = HT_
#define VBM_HITTEST     ( VBM__BASE + 0x15 )

#define HT_ON         7
#define HT_SOLID_NEAR     6
#define HT_PATTERN_NEAR   5
#define HT_HOLLOW_NEAR    4
#define HT_SOLID      3
#define HT_PATTERN    2
#define HT_HOLLOW     1
#define HT_MISS       0
typedef struct tagHITTEST
  {
  POINT pt;     // Point to test (in parent's coordinates)
  RECT  rect;       // Rect of self (in parent's coordinates)
  } HITTEST;
typedef HITTEST FAR *LPHITTEST;

// Paint the multi-select handles
// [wp=hdc, lp=rect(in hdc coords)]
#define VBM_PAINTMULTISEL   ( VBM__BASE + 0x16 )

// Paint the "gray rect" or ctl outline while moving/sizing ctl.
// [wp=hdc, lp=rect(in hdc coords)]
#define VBM_PAINTOUTLINE    ( VBM__BASE + 0x17 )

// Palette change notification.
// [wp=fPalBack(for SelectPalette)], RetVal = return from RealizePalette
#define VBM_PALETTECHANGED  ( VBM__BASE + 0x1B )

// Get control palette.
// RetVal = HPALETTE.
#define VBM_GETPALETTE      ( VBM__BASE + 0x1C )

// Enumerate available formats.
// [wp=SUPPLIESDATAFORMAT | ACCEPTSDATAFORMAT]
// [LOWORD(lp)=ennumeration(0-x)]
#define VBM_LINKENUMFORMATS ( VBM__BASE + 0x1E )

// Ask a control for it's LINK item name.  Return item name as lpsz in lp.
// [wp=LINKSRCASK | LINKSRCTELL]
// [lp=lpszItemName]
#define VBM_LINKGETITEMNAME ( VBM__BASE + 0x1F )

// For Server items and client pokes.
// [wp=wFormat lp=LPLINKDATA]
#define VBM_LINKGETDATA     ( VBM__BASE + 0x20 )

// Used in client requests, advises, and server pokes.
// [wp=wFormat lp=LPLINKDATA]
#define VBM_LINKSETDATA     ( VBM__BASE + 0x21 )

// Get help on property or event name.
// [LOBYTE(wp)=VBHELP_, HIBYTE(wp)=iprop/ievent, lp=lpmodel]
#define VBM_HELP        ( VBM__BASE + 0x22 )

#define VBHELP_PROP 0x0001
#define VBHELP_EVT  0x0002
#define VBHELP_CTL      0x0003

// Get default size of control
// LOWORD(RetVal)=cx, HIWORD(RetVal)=cy
#define VBM_GETDEFSIZE      ( VBM__BASE + 0x23 )

// Just like VBM_SAVE(LOAD)PROPERTY, but saves (loads) as text.
#define VBM_SAVETEXTPROPERTY    ( VBM__BASE + 0x24 )
#define VBM_LOADTEXTPROPERTY    ( VBM__BASE + 0x25 )

// Is char a mnemonic for this control?
// [wp=char] RetVal=TRUE/FALSE
#define VBM_ISMNEMONIC      ( VBM__BASE + 0x26 )

// Does control want to see WM_KEYUP/WM_KEYDOWN for vk?  Sent only
// for keys which are normally trapped by VB.  E.g. Tab, Enter, etc.
// [wp=vk] RetVal=TRUE/FALSE
#define VBM_WANTSPECIALKEY  ( VBM__BASE + 0x27 )

//Notifies a control that it has been selected in the property window.
//[wp=multiselect ON (TRUE) or OFF (FALSE)]
//[LOWORD(lp)=multiselect count (0 first, n last)]
#define VBM_SELECTED        ( VBM__BASE + 0x28 )

// Bound control interface
#define VBM_DATA_INITIATE   (VBM__BASE + 0x29)
#define VBM_DATA_TERMINATE  (VBM__BASE + 0x2A)

#define VBM_DATA_AVAILABLE  (VBM__BASE + 0x2B)
#define VBM_DATA_GET        (VBM__BASE + 0x2C)
#define VBM_DATA_REQUEST    (VBM__BASE + 0x2D)
#define VBM_DATA_SET        (VBM__BASE + 0x2E)
#define VBM_DATA_METHOD     (VBM__BASE + 0x2F)

// The following message is sent back to the control by the FPDB lib
// if an error occurs while writing data to the Data Source.
//
//   wParam is the number of entries in the array.    
//   lParam is a ptr to an array of the structure:
//      struct {
//      long lErrCode;     // see fpdbole.h for list of codes (FPDB_ERR...)
//      long lExtErrCode;  // Return code from ICursor API function
//      };                 // which generated the error (if applicable) 
#define VBM_DATA_UERROR      (VBM__BASE + 0x30)          //DBOCX
#define VBM_DATA_QUERYUSERMODE (VBM__BASE + 0x31)        //DBOCX
#define VBM_DATA_POSTMSGTOSELF (VBM__BASE + 0x32)        //DBOCX


// Notifications reflected back to control from parent.
#define VBN__BASE       (VBM__BASE + 0x1000)

#define VBN_COMMAND     (VBN__BASE + WM_COMMAND)
#define VBN_CTLCOLOR        (VBN__BASE + WM_CTLCOLOR)
#define VBN_DRAWITEM        (VBN__BASE + WM_DRAWITEM)
#define VBN_MEASUREITEM     (VBN__BASE + WM_MEASUREITEM)
#define VBN_DELETEITEM      (VBN__BASE + WM_DELETEITEM)
#define VBN_VKEYTOITEM      (VBN__BASE + WM_VKEYTOITEM)
#define VBN_CHARTOITEM      (VBN__BASE + WM_CHARTOITEM)
#define VBN_COMPAREITEM     (VBN__BASE + WM_COMPAREITEM)
#define VBN_HSCROLL     (VBN__BASE + WM_HSCROLL)
#define VBN_VSCROLL     (VBN__BASE + WM_VSCROLL)
#define VBN_PARENTNOTIFY    (VBN__BASE + WM_PARENTNOTIFY)


//---------------------------------------------------------------------------
// END: Handle C++ and Pack structures on 1-byte boundaries
//---------------------------------------------------------------------------
#ifndef RC_INVOKED
#pragma pack()
#endif  // ifndef RC_INVOKED

#ifdef __cplusplus
}
#endif

//-- End Of File ------------------------------------------------------------
// previous defines borrowed from VBAPI.H for common code support

#define OLE_VERSION     VB_VERSION                                           

//#ifdef __cplusplus
//#include "vbinterf.h"
//#endif

#endif // FP_OCX || FP_DLL

#endif // __FPVBSUB_H__
 
