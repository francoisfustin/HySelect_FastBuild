//--------------------------------------------------------------------
//
//  File: SS_SAVE4.H
//

#ifndef SS_SAVE4_H
#define SS_SAVE4_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------

#ifndef HUGE
  #define HUGE huge
#endif

#ifndef BYTE
  #define BYTE unsigned char
#endif

//--------------------------------------------------------------------
//
//  The SS_RID_xxxx constants are used in the nRecType field of the
//  SS_REC_COMMON structure.
//

#define SS_RID_AUTOSIZE              1
#define SS_RID_BORDER                2
#define SS_RID_CALCOLD               3
#define SS_RID_CALCDEPEND            4
#define SS_RID_COLOR                 5
#define SS_RID_COLWIDTH              6
#define SS_RID_DATAAWARE             7
#define SS_RID_DATAAWARECOL          8
#define SS_RID_DATAAWAREQE           9
#define SS_RID_DEFFLOATFORMAT       10
#define SS_RID_DISPLAY              11
#define SS_RID_EDIT                 12
#define SS_RID_FONT                 13
#define SS_RID_FONTTABLEOLD         14
#define SS_RID_FORMULAUNPARSED      15
#define SS_RID_FORMULA              16
#define SS_RID_FROZEN               17
#define SS_RID_GRAYAREACOLOR        18
#define SS_RID_GRID                 19
#define SS_RID_HEADER               20
#define SS_RID_ITEMDATA             21
#define SS_RID_ITERATION            22
#define SS_RID_LOCK                 23
#define SS_RID_LOCKCOLOR            24
#define SS_RID_MAXSIZE              25
#define SS_RID_CUSTNAME             26
#define SS_RID_CUSTNAMEPARSED       27
#define SS_RID_CUSTNAMETABLE        28
#define SS_RID_OPMODE               29
#define SS_RID_PRINTOLD             30
#define SS_RID_PROTECT              31
#define SS_RID_REFSTYLE             32
#define SS_RID_RESTRICT             33
#define SS_RID_ROWHEIGHT            34
#define SS_RID_SCROLLBAR            35
#define SS_RID_SELBLOCKOPTION       36
#define SS_RID_SHADOWCOLOR          37
#define SS_RID_TYPEBUTTON           38
#define SS_RID_TYPECHECK            39
#define SS_RID_TYPECOMBO            40
#define SS_RID_TYPEDATE             41
#define SS_RID_TYPEEDIT             42
#define SS_RID_TYPEFLOAT            43
#define SS_RID_TYPEINTEGER          44
#define SS_RID_TYPEPIC              45
#define SS_RID_TYPEPICTURE          46
#define SS_RID_TYPEOWNERDRAW        47
#define SS_RID_TYPESTATIC           48
#define SS_RID_TYPETIME             49
#define SS_RID_UNITTYPE             50
#define SS_RID_USERRESIZE           51
#define SS_RID_USERRESIZECOL        52
#define SS_RID_USERRESIZEROW        53
#define SS_RID_VALLONG              54
#define SS_RID_VALDBL               55
#define SS_RID_VALSTR               56
#define SS_RID_VIRTUAL              57
#define SS_RID_CUSTFUNCTABLE        58
#define SS_RID_COLPAGEBREAK         59
#define SS_RID_ROWPAGEBREAK         60
#define SS_RID_CURSOR               61
#define SS_RID_CURSORCUSTOM         62
#define SS_RID_FONTTABLE            63
#define SS_RID_CALC                 64
#define SS_RID_PRINT                65
#define SS_RID_BACKCOLORSTYLE       66
#define SS_RID_DDETABLE             67
#define SS_RID_BORDER_EXT           68
#define SS_RID_ACTIONKEY            69
#define SS_RID_ODDEVENROWCOLOR      70
#define SS_RID_APPEARANCE           71
#define SS_RID_TEXTTIP_NOTUSED      72
#define SS_RID_SCROLLBARTRACK       73
#define SS_RID_CLIPBOARDOPTIONS     74
#define SS_RID_COLHIDDEN            75
#define SS_RID_ROWHIDDEN            76
#define SS_RID_TYPECOMBOEX          77
#define SS_RID_PRINT2               78
#define SS_RID_SELCOLOR             79
#define SS_RID_TWODIGITYEARMAX      80
#define SS_RID_TEXTTIP              81
#ifdef SS_V35
#define SS_RID_CELLNOTE             82
#define SS_RID_CELLNOTEINDICATOR    83
#define SS_RID_SHOWSCROLLTIPS       84
#define SS_RID_USERCOLACTION        85
#define SS_RID_COLUSERSORTINDICATOR 86
#define SS_RID_SCRIPTENHANCED       87
#define SS_RID_CALTEXTOVERRIDE      88
#endif // SS_V35
#ifdef SS_V40
#define SS_RID_CELLSPANTABLE        89
#define SS_RID_COLMERGE             90
#define SS_RID_COLID                91
#define SS_RID_HEADER4              92
#define SS_RID_ROWMERGE             93
#define SS_RID_TYPECOMBOEX4         94
#define SS_RID_DISPLAY4             95
#define SS_RID_CELLTAG              96
#define SS_RID_TYPECURRENCY         97
#define SS_RID_TYPENUMBER           98
#define SS_RID_TYPEPERCENT          99
#endif // SS_V40
#ifdef SS_V70
#define SS_RID_SHEETINDEX           100
#define SS_RID_SHEETCOUNT           101
#define SS_RID_ACTIVESHEET          102
#define SS_RID_SHEETNAME            103
#define SS_RID_TABSTRIP             104
#define SS_RID_TYPESCIENTIFIC       105
#define SS_RID_SCROLL7              106
#define SS_RID_SHEETSETTINGS        107
#endif // SS_V70
#define SS_RID_PRINTSHEET           108
#define SS_RID_PRINTBOOK            109
#define SS_RID_ITEMDATA64           110
#ifdef SS_V80
#define SS_RID_APPEARANCESTYLE      111
#define SS_RID_TABENHANCEDSHAPE     112
#define SS_RID_CELLNOTEINDICATOR2   113
#define SS_RID_PRINTDEVMODE			114
#define SS_RID_TABSTRIPBUTTONPOLICY 115
#define SS_RID_HIGHLIGHTHEADERS     116
#define SS_RID_HIGHLIGHTSTYLE       117
#define SS_RID_HIGHLIGHTALPHABLEND  118
#define SS_RID_CUSTCELLTABLE		119
#define SS_RID_TYPECUSTOM        120
#define SS_RID_VALBUFFER         121
#define SS_RID_USEVISUALSTYLES   122
#define SS_RID_ENHANCESTATICCELLS 123
#define SS_RID_SCROLLBARSTYLE 124
#define SS_RID_ENHANCEDCOLORS 125
#endif

//--------------------------------------------------------------------
//
//  The SS_REC_xxxx structures are packed on byte boundaries.
//

#pragma pack(1)

//--------------------------------------------------------------------
//
//  The SS_REC_FILE record describes the type, version, and length
//  of a file (or buffer).  The SS_REC_FILE is always the first
//  record in a file (or buffer).
//

typedef struct tagSS_REC_FILE
{
  BYTE bFileType;
  BYTE bFileVer;
  long lFileLen;
} SS_REC_FILE, FAR* LPSS_REC_FILE;

//--------------------------------------------------------------------
//
//  The SS_REC_FILE record describes the type, version, and length
//  of a file (or buffer).  The SS_REC_FILE is always the first
//  record in a file (or buffer).
//

typedef struct tagSS_REC_BLOCKHEADER
{
  BYTE bType;
  BYTE bVer;
  long lLen;
  long lCol;
  long lRow;
  long lCol2;
  long lRow2;
} SS_REC_BLOCKHEADER, FAR* LPSS_REC_BLOCKHEADER;

//--------------------------------------------------------------------
//
//  The SS_REC_COMMON record describes the fields which are common
//  to all other record types.
//

#define REC_COMMON() short nRecType; long lRecLen

typedef struct tagSS_REC_COMMON 
{
  REC_COMMON();
} SS_REC_COMMON, FAR* LPSS_REC_COMMON;

//--------------------------------------------------------------------
//
//  The SS_REC_AUTOSIZE record stores auto size options.
//

typedef struct tagSS_REC_AUTOSIZE
{
  REC_COMMON();
  BYTE bAutoSize;
  long lVisibleCols;
  long lVisibleRows;
} SS_REC_AUTOSIZE, FAR* LPSS_REC_AUTOSIZE;

//--------------------------------------------------------------------
//
//  The SS_REC_BACKCOLORSTYLE record determines whether the backcolor
//  is paint over or under the grid lines.
//

typedef struct tagSS_REC_BACKCOLORSTYLE
{
  REC_COMMON();
  short nBackColorStyle;
} SS_REC_BACKCOLORSTYLE, FAR* LPSS_REC_BACKCOLORSTYLE;

//--------------------------------------------------------------------
//
//  The SS_REC_BORDER record describes a cell border.
//

typedef struct tagSS_REC_BORDER
{
  REC_COMMON();
  long lCol;
  long lRow;
  WORD wBorderType;
  WORD wBorderStyle;
  COLORREF crBorderColor;
} SS_REC_BORDER, FAR* LPSS_REC_BORDER;

//--------------------------------------------------------------------
//
//  The SS_REC_BORDER_EXT record describes a cell border.
//

typedef struct tagSS_REC_BORDER_EXT
{
  REC_COMMON();
  long     lCol;
  long     lRow;
  WORD     wBorderStyle[4];
  COLORREF clrBorder[4];
} SS_REC_BORDER_EXT, FAR* LPSS_REC_BORDER_EXT;

//--------------------------------------------------------------------
//
//  The SS_REC_CALC record stores calculation options.
//

typedef struct tagSS_REC_CALC
{
  REC_COMMON();
  BYTE bAllowUserFormulas;
  BYTE bAutoCalc;
  BYTE bFormulaSync;
} SS_REC_CALC, FAR* LPSS_REC_CALC;

//--------------------------------------------------------------------
//
//  The SS_REC_CALC_OLD record stores calculation options.
//

typedef struct tagSS_REC_CALC_OLD
{
  REC_COMMON();
  BYTE bAllowUserFormulas;
  BYTE bAutoCalc;
} SS_REC_CALC_OLD, FAR* LPSS_REC_CALC_OLD;

//--------------------------------------------------------------------
//
//  The SS_REC_CALCDEPEND record determines if dependency
//  tables are rebuild each time a new formula is added.  The
//  SS_REC_CALCDEPEND record is only used by OCX and VBX versions
//  of the spreadsheet.
//

typedef struct tagSS_REC_CALCDEPEND
{
  REC_COMMON();
  BYTE bCalcDepend;
} SS_REC_CALCDEPEND, FAR* LPSS_REC_CALCDEPEND;

//--------------------------------------------------------------------
//
//  The SS_REC_COLOR describes the background and foreground colors
//  of a cell, column, row, or spreadsheet.
//

typedef struct tagSS_REC_COLOR
{
  REC_COMMON();
  long lCol;
  long lRow;
  COLORREF crBackColor;
  COLORREF crForeColor;
} SS_REC_COLOR, FAR* LPSS_REC_COLOR;

//--------------------------------------------------------------------
//
//  The SS_REC_COLPAGEBREAK record determines if a page break occurs
//  of the given column.
//

typedef struct tagSS_REC_COLPAGEBREAK
{
  REC_COMMON();
  long lCol;
  BYTE bPageBreak;
} SS_REC_COLPAGEBREAK, FAR* LPSS_REC_COLPAGEBREAK;

//--------------------------------------------------------------------
//
//  The SS_REC_COLWIDTH record specifies the width of a column.
//

typedef struct tagSS_REC_COLWIDTH
{
  REC_COMMON();
  long lCol;
  double dfColWidth;
} SS_REC_COLWIDTH, FAR* LPSS_REC_COLWIDTH;

//--------------------------------------------------------------------
//
//  The SS_REC_CURSOR record specifies one of the spreadsheet's
//  mouse cursors as a built in cursor.
//

typedef struct tagSS_REC_CURSOR
{
  REC_COMMON();
  short nCursorType;
  short nCursorStyle;
} SS_REC_CURSOR, FAR* LPSS_REC_CURSOR;

//--------------------------------------------------------------------
//
//  The SS_REC_CURSORCUSTOM record specifies one of the spreadsheet's
//  mouse cursors as a custom cursor.
//

typedef struct tagSS_REC_CURSORCUSTOM
{
  REC_COMMON();
  short nCursorType;
  long xHotspot;
  long yHotspot;
  long lMaskLen;
  long lColorLen;
  //BYTE bMask[];   // mask bitmap
  //BYTE bColor[];  // color bitmap
} SS_REC_CURSORCUSTOM, FAR* LPSS_REC_CURSORCUSTOM;

//--------------------------------------------------------------------
//
//  The SS_REC_CUSTFUNCTABLE record contains a list of all custom
//  functions used in the spreadsheet.  The SS_REC_CUSTFUNCTABLE
//  record is a variable length record containing zero or more
//  SS_SUBREC_CUSTFUNC subrecords.
//

typedef struct tagSS_REC_CUSTFUNCTABLE
{
  REC_COMMON();
  short nTableCnt;
  long lTableLen;
  //SS_CUSTOMCELLTYPE aTable[];  // variable length portion of record
} SS_REC_CUSTFUNCTABLE, FAR* LPSS_REC_CUSTFUNCTABLE;

#ifdef SS_V80
//--------------------------------------------------------------------
//
//  The SS_REC_CUSTCELLTABLE record contains a list of all custom
//  celltypes used in the spreadsheet.  The SS_REC_CUSTCELLTABLE
//  record is a variable length record containing zero or more
//  SS_SUBREC_CUSTCELL subrecords.
//

typedef struct tagSS_REC_CUSTCELLTABLE
{
  REC_COMMON();
  short nTableCnt;
  long lTableLen;
  //SS_SUBREC_CUSTCELL aTable[];  // variable length portion of record
} SS_REC_CUSTCELLTABLE, FAR* LPSS_REC_CUSTCELLTABLE;
//--------------------------------------------------------------------
//
//  The SS_SUBREC_CUSTCELL record describes a custom celltype
//  and is a subrecord of the SS_CUSTCELLTABLE record.  The
//  SS_SUBREC_CUSTCELL record is a variable length record which
//  contains NULL terminated text.
//

typedef struct tagSS_SUBREC_CUSTCELL
{
   CT_HANDLE hText;		// handle to name of custom function
//   long lRefCnt;		// number of references to custom celltype
   BOOL	bEditable;		// whether the celltype is editable
   BOOL	bCanOverflow;	// whether the celltype can overflow
   BOOL	bCanBeOverflown;// whether the celltype can be overflown
   BOOL  bUseRendererControl; // whether the celltype uses a renderer control
//   SS_CT_PROCS Procs;	// function pointers to callbacks
   long lNameLen;     // number of bytes in szName field;
  //TCHAR szName[];  // variable length portion of record;
} SS_SUBREC_CUSTCELL, FAR* LPSS_SUBREC_CUSTCELL;
#endif
//--------------------------------------------------------------------
//
//  The SS_REC_CUSTNAMETABLE record contains a list of all custom
//  names used in the spreadsheet.  The SS_REC_CUSTNAMETABLE
//  record is a variable length record containing zero or more
//  SS_SUBREC_CUSTNAME subrecords.
//

typedef struct tagSS_REC_CUSTNAMETABLE
{
  REC_COMMON();
  short nTableCnt;
  long lTableLen;
  //SS_SUBREC_CUSTNAME aTable[];  // variable length portion of record
}  SS_REC_CUSTNAMETABLE, FAR* LPSS_REC_CUSTNAMETABLE;

//--------------------------------------------------------------------
//
//  The SS_REC_DATAAWARE record contains data aware options which
//  apply to the entire spreadsheet.  The SS_REC_DATAAWARE record
//  is only used by OCX and VBX versions of the spreadsheet.
//

typedef struct tagSS_REC_DATAAWARE
{
  REC_COMMON();
  BYTE bDAutoCellTypes;
  BYTE bDAutoFill;
  BYTE bDAutoHeadings;
  BYTE bDAutoSave;
  BYTE bDAutoSizeCols;
  BYTE bDInformActiveRowChange;
} SS_REC_DATAAWARE, FAR* LPSS_REC_DATAAWARE;

//--------------------------------------------------------------------
//
//  The SS_REC_DATAAWARECOL record contains data aware options which
//  apply to an individual column.  The SS_REC_DATAAWARECOL record
//  is a variable length record which contains an option NULL
//  terminated string.  The SS_REC_DATAAWARECOL record is only used
//  by OCX and VBX versions of the spreadsheet.
//

typedef struct tagSS_REC_DATAAWARECOL
{
  REC_COMMON();
  long lCol;
  BYTE bDataFillEvent;
  long lDataFieldLen;
  //TCHAR szDataField[];
} SS_REC_DATAAWARECOL, FAR* LPSS_REC_DATAAWARECOL;

//--------------------------------------------------------------------
//
//  The SS_REC_DATAAWAREQE record contains data aware options used
//  with QE.  The SS_REC_DATAAWAREQE record is a variable length
//  record which contains optional NULL terminated strings.  The
//  SS_REC_DATAAWAREQE record is only used by VBX versions of the
//  spreadsheet.
//

typedef struct tagSS_REC_DATAAWAREQE
{
  REC_COMMON();
  long lDataConnectLen;
  long lDataSelectLen;
  //TCHAR szDataConnect[];
  //TCHAR szDataSelect[];
} SS_REC_DATAAWAREQE, FAR* LPSS_REC_DATAAWAREQE;

//--------------------------------------------------------------------
//
//  The SS_REC_DDETABLE record contains a list of all DDE links
//  used in the spreadsheet.  The SS_REC_DDETABLE record is a
//  variable length record containing zero or more SS_SUBREC_DDE
//  subrecords.
//

typedef struct tagSS_REC_DDETABLE
{
  REC_COMMON();
  short nTableCnt;
  long lTableLen;
  //SS_SUBREC_DDE aTable[];  // variable lenght portion of record
} SS_REC_DDETABLE, FAR* LPSS_REC_DDETABLE;

//--------------------------------------------------------------------
//
//  The SS_REC_DEFFLOATFORMAT record describes the default float
//  format for the spreadsheet.
//
//  Note: Unicode requires 16-bit characters.
//

typedef struct tagSS_REC_DEFFLOATFORMAT
{
  REC_COMMON();
  short nCurrChar;
  short nDecChar;
  short nSepChar;
} SS_REC_DEFFLOATFORMAT, FAR* LPSS_REC_DEFFLOATFORMAT;

//--------------------------------------------------------------------
//
//  The SS_REC_DISPLAY record stores display options.
//

typedef struct tagSS_REC_DISPLAY
{
  REC_COMMON();
  BYTE bAllowCellOverflow;
  WORD wButtonDrawMode;
  BYTE bRedraw;
  BYTE bRetainSelBlock;
  BYTE bNoBorder;
} SS_REC_DISPLAY, FAR* LPSS_REC_DISPLAY;

//--------------------------------------------------------------------
//
//  The SS_REC_EDIT record stores edit options.
//

typedef struct tagSS_REC_EDIT
{
  REC_COMMON();
  BYTE bAllowDragDrop;
  BYTE bAllowMultiBlocks;
  BYTE bArrowsExitEditMode;
  BYTE bAutoClipboard;
  WORD wEditEnterAction;
  BYTE bEditModePermanent;
  BYTE bEditModeReplace;
  BYTE bMoveActiveOnFocus;
  BYTE bNoBeep;
  BYTE bProcessTab;
} SS_REC_EDIT, FAR* LPSS_REC_EDIT;

//--------------------------------------------------------------------
//
//  The SS_REC_FONT record describes the font for a cell, column,
//  row, or spreadsheet.  The nFontId is an index (base 0) into
//  the font table which is stored in the SS_REC_FONTTABLE record.
//

typedef struct tagSS_REC_FONT
{
  REC_COMMON();
  long lCol;
  long lRow;
  short nFontId;
} SS_REC_FONT, FAR* LPSS_REC_FONT;

//--------------------------------------------------------------------
//
//  The SS_REC_FONTTABLE record contains a list of all fonts used
//  in the spreadsheet.  The SS_REC_FONTTABLE record is a variable
//  length record containing zero or more SS_SUBREC_LOGFONT
//  subrecords.
//

typedef struct tagSS_REC_FONTTABLE
{
  REC_COMMON();
  short nTableCnt;
  //SS_SUBREC_LOGFONT[];  // variable portion of record
} SS_REC_FONTTABLE, FAR* LPSS_REC_FONTTABLE;

//--------------------------------------------------------------------
//
//  The SS_REC_FONTTABLEOLD record contains a list of all fonts used
//  in the spreadsheet.  The SS_REC_FONTTABLEOLD record is a variable
//  length record containing zero or more SS_SUBREC_LOGFONTOLD
//  subrecords.
//

typedef struct tagSS_REC_FONTTABLEOLD
{
  REC_COMMON();
  short nTableCnt;
  //SS_SUBREC_LOGFONTOLD[];  // variable portion of record
} SS_REC_FONTTABLEOLD, FAR* LPSS_REC_FONTTABLEOLD;

//--------------------------------------------------------------------
//
//  The SS_REC_FORMULA record describes a formula for a cell, column,
//  or row.  The SS_REC_FORMULA record is a variable length record
//  containing a NULL terminated text string.
//

typedef struct tagSS_REC_FORMULA
{
  REC_COMMON();
  long lCol;
  long lRow;
  BYTE bAlwaysCalc;
  BYTE bNeedsEval;
  long lExprLen;
  //TCHAR lpszExpr[];  // variable portion of record
} SS_REC_FORMULA, FAR* LPSS_REC_FORMULA;

//--------------------------------------------------------------------
//
//  The SS_REC_FORMULAPARSED record describes a formula for a cell, column,
//  or row.  The SS_REC_FORMULAPARSED record is a variable length record
//  containing a NULL terminated text string.
//

typedef struct tagSS_REC_FORMULAUNPARSED
{
  REC_COMMON();
  long lCol;
  long lRow;
  //BYTE bFormula[];  // variable length parsed expression
} SS_REC_FORMULAUNPARSED, FAR* LPSS_REC_FORMULAUNPARSED;

//--------------------------------------------------------------------
//
//  The SS_REC_FROZEN record specifies the number of frozen (i.e.
//  stationary, non-scrolling) columns and rows.
//

typedef struct tagSS_REC_FROZEN
{
  REC_COMMON();
  long lColsFrozen;
  long lRowsFrozen;
} SS_REC_FROZEN, FAR* LPSS_REC_FROZEN;

//--------------------------------------------------------------------
//
//  The SS_REC_GRAYAREACOLOR record describes the background and
//  foreground colors used in the spreadsheet's gray area.
//

typedef struct tagSS_REC_GRAYAREACOLOR
{
  REC_COMMON();
  COLORREF crBackColor;
  COLORREF crForeColor;
} SS_REC_GRAYAREACOLOR, FAR* LPSS_REC_GRAYAREACOLOR;

//--------------------------------------------------------------------
//
//  The SS_REC_GRID record stores grid lines options.
//

typedef struct tagSS_REC_GRID
{
  REC_COMMON();
  WORD wGridType;
  COLORREF crGridColor;
} SS_REC_GRID, FAR* LPSS_REC_GRID;

//--------------------------------------------------------------------
//
//  The SS_REC_HEADER record stores column and row header options.
//

typedef struct tagSS_REC_HEADER
{
  REC_COMMON();
  WORD wColHeaderDisplay;
  WORD wRowHeaderDisplay;
  long lStartColNum;
  long lStartRowNum;
} SS_REC_HEADER, FAR* LPSS_REC_HEADER;

//--------------------------------------------------------------------
//
//  The SS_REC_ITERATION record specifies iteration options used
//  when evaluating recursive formulas.
//

typedef struct tagSS_REC_ITERATION
{
  REC_COMMON();
  BYTE bIteration;
  short nMaxIterations;
  double dfMaxChange;
} SS_REC_ITERATION, FAR* LPSS_REC_ITERATION;

//--------------------------------------------------------------------
//
//  The SS_REC_ITEMDATA record stores item data for a cell, column,
//  row, or spreadsheet.
//

typedef struct tagSS_REC_ITEMDATA
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lItemData;
} SS_REC_ITEMDATA, FAR* LPSS_REC_ITEMDATA;

#if defined(_WIN64) || defined(_IA64)
//--------------------------------------------------------------------
//
//  The SS_REC_ITEMDATA64 record stores item data for a cell, column,
//  row, or spreadsheet in the 64-bit version.
//

typedef struct tagSS_REC_ITEMDATA64
{
  REC_COMMON();
  long lCol;
  long lRow;
  LONG_PTR lItemData;
} SS_REC_ITEMDATA64, FAR* LPSS_REC_ITEMDATA64;
#endif

//--------------------------------------------------------------------
//
//  The SS_REC_LOCK record specifies whether or not a cell, column,
//  row, or spreadsheet is locked.
//

typedef struct tagSS_REC_LOCK
{
  REC_COMMON();
  long lCol;
  long lRow;
  BYTE bLock;  // 0 = SS_LOCK_DEF, 1 = SS_LOCK_ON, 2 = SS_LOCK_OFF
} SS_REC_LOCK, FAR* LPSS_REC_LOCK;

//--------------------------------------------------------------------
//
//  The SS_REC_LOCKCOLOR record describes the background and
//  foreground colors for locked cells.
//

typedef struct tagSS_REC_LOCKCOLOR
{
  REC_COMMON();
  COLORREF crBackColor;
  COLORREF crForeColor;
} SS_REC_LOCKCOLOR, FAR* LPSS_REC_LOCKCOLOR;

//--------------------------------------------------------------------
//
//  The SS_REC_SELCOLOR record describes the background and
//  foreground colors for selected cells.
//

typedef struct tagSS_REC_SELCOLOR
{
  REC_COMMON();
  COLORREF crBackColor;
  COLORREF crForeColor;
} SS_REC_SELCOLOR, FAR* LPSS_REC_SELCOLOR;

//--------------------------------------------------------------------
//
//  The SS_REC_MAXSIZE record contains the maximum dimensions of
//  the spreadsheet.
//

typedef struct tagSS_REC_MAXSIZE
{
  REC_COMMON();
  long lMaxCols;
  long lMaxRows;
} SS_REC_MAXSIZE, FAR* LPSS_REC_MAXSIZE;

//--------------------------------------------------------------------
//
//  The SS_REC_OPMODE record stores the opertion mode option.
//

typedef struct tagSS_REC_OPMODE
{
  REC_COMMON();
  WORD wOpMode;
} SS_REC_OPMODE, FAR* LPSS_REC_OPMODE;

//--------------------------------------------------------------------
//
//  The SS_REC_PRINT record contains print options.  The SS_REC_PRINT
//  is a variable length record which can contain optional NULL
//  terminated strings. The SS_REC_PRINT record is only used by OCX
//  and VBX versions of the spreadsheet.
//

typedef struct tagSS_REC_PRINT
{
  REC_COMMON();
  BYTE bDrawBorder;
  BYTE bDrawColor;
  BYTE bDrawShadows;
  BYTE bShowGrid;
  BYTE bShowColHeaders;
  BYTE bShowRowHeaders;
  BYTE bUseDataMax;
  short nPrintType;
  short nPageEnd;
  short nPageStart;
  long lMarginLeft;
  long lMarginTop;
  long lMarginBottom;
  long lMarginRight;
  short nOrientation;
  long lAbortMsgLen;  // length in bytes of abort message
  long lFooterLen;    // length in bytes of footer
  long lHeaderLen;    // length in bytes of header
  long lJobNameLen;   // length in bytes of job name
  //TCHAR szAbortMsg[];
  //TCHAR szFooter[];
  //TCHAR szHeader[];
  //TCHAR szJobName[];
} SS_REC_PRINT, FAR* LPSS_REC_PRINT;

//--------------------------------------------------------------------
//
//  The SS_REC_PRINT2 record contains print options that were added
//  in version 3.0. The SS_REC_PRINT2 record is only used by OCX
//  and VBX versions of the spreadsheet.
//

typedef struct tagSS_REC_PRINT2
{
  REC_COMMON();
  BYTE bSmartPrint;
  short nPageOrder;
  long lFirstPageNumber;
} SS_REC_PRINT2, FAR* LPSS_REC_PRINT2;

//--------------------------------------------------------------------
//
//  The SS_REC_PRINTOLD record contains print options.  The SS_REC_PRINTOLD
//  is a variable length record which can contain optional NULL
//  terminated strings. The SS_REC_PRINTOLD record is only used by OCX
//  and VBX versions of the spreadsheet.
//

typedef struct tagSS_REC_PRINTOLD
{
  REC_COMMON();
  BYTE bDrawBorder;
  BYTE bDrawColor;
  BYTE bDrawShadows;
  BYTE bShowGrid;
  BYTE bShowColHeaders;
  BYTE bShowRowHeaders;
  BYTE bUseDataMax;
  short nPrintType;
  short nPageEnd;
  short nPageStart;
  long lMarginLeft;
  long lMarginTop;
  long lMarginBottom;
  long lMarginRight;
  long lAbortMsgLen;  // length in bytes of abort message
  long lFooterLen;    // length in bytes of footer
  long lHeaderLen;    // length in bytes of header
  long lJobNameLen;   // length in bytes of job name
  //TCHAR szAbortMsg[];
  //TCHAR szFooter[];
  //TCHAR szHeader[];
  //TCHAR szJobName[];
} SS_REC_PRINTOLD, FAR* LPSS_REC_PRINTOLD;

//--------------------------------------------------------------------
//
//  The SS_REC_PROTECT record determines whether or not the
//  spreadsheet is protected.
//

typedef struct tagSS_REC_PROTECT
{
  REC_COMMON();
  BYTE bProtect;
} SS_REC_PROTECT, FAR* LPSS_REC_PROTECT;

//--------------------------------------------------------------------
//
//  The SS_REC_REFSTYLE record specifies the reference style used
//  to parse and unparse formulas.
//

typedef struct tagSS_REC_REFSTYLE
{
  REC_COMMON();
  short nRefStyle;
} SS_REC_REFSTYLE, FAR* LPSS_REC_REFSTYLE;

//--------------------------------------------------------------------
//
//  The SS_REC_RESTRICT record contains options for restricting data
//  entry beyond the last column and/or row containing data.
//

typedef struct tagSS_REC_RESTRICT
{
  REC_COMMON();
  BYTE bRestrictCols;
  BYTE bRestrictRows;
} SS_REC_RESTRICT, FAR* LPSS_REC_RESTRICT;

//--------------------------------------------------------------------
//
//  The SS_REC_ROWHEIGHT record specifies the height of a row.
//

typedef struct tagSS_REC_ROWHEIGHT
{
  REC_COMMON();
  long lRow;
  double dfRowHeight;
} SS_REC_ROWHEIGHT, FAR* LPSS_REC_ROWHEIGHT;

//--------------------------------------------------------------------
//
//  The SS_REC_ROWPAGEBREAK record determines if a page break occurs
//  at the given Row.
//

typedef struct tagSS_REC_ROWPAGEBREAK
{
  REC_COMMON();
  long lRow;
  BYTE bPageBreak;
} SS_REC_ROWPAGEBREAK, FAR* LPSS_REC_ROWPAGEBREAK;

//--------------------------------------------------------------------
//
//  The SS_REC_SCROLLBAR record stores scroll bar options.
//

typedef struct tagSS_REC_SCROLLBAR
{
  REC_COMMON();
  BYTE bHorzScrollBar;
  BYTE bVertScrollBar;
  BYTE bScrollBarExtMode;
  BYTE bScrollBarMaxAlign;
  BYTE bScrollBarShowMax;
} SS_REC_SCROLLBAR, FAR* LPSS_REC_SCROLLBAR;

//--------------------------------------------------------------------
//
//  The SS_REC_SELBLOCKOPTION record specifies whether or not the
//  user can select a column, row, block of cells, or entire
//  the spreadsheet.
//

typedef struct tagSS_REC_SELBLOCKOPTION
{
  REC_COMMON();
  WORD wOption;
} SS_REC_SELBLOCKOPTION, FAR* LPSS_REC_SELBLOCKOPTION;

//--------------------------------------------------------------------
//
//  The SS_REC_SHADOWCOLOR record describes the colors used to
//  paint shadowed text cells.
//

typedef struct tagSS_REC_SHADOWCOLOR
{
  REC_COMMON();
  COLORREF crShadowColor;
  COLORREF crShadowText;
  COLORREF crShadowDark;
  COLORREF crShadowLight;
} SS_REC_SHADOWCOLOR, FAR* LPSS_REC_SHADOWCOLOR;

//--------------------------------------------------------------------
//
//  The SS_REC_TYPEBUTTON record describes a button cell type.  The
//  SS_REC_TYPEBUTTON record is a variable length record which can
//  contain optional text and/or pictures.
//

typedef struct tagSS_REC_TYPEBUTTON
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  short nPictType;      // type of picture
  short nPictDownType;  // type of down picture
  short nButtonType;    // type of button (i.e. one or two state)
  short nShadowSize;
  COLORREF crFace;
  COLORREF crText;
  COLORREF crHilight;
  COLORREF crLight;
  COLORREF crShadow;
  COLORREF crDkShadow;
  COLORREF crBorder;
  long lTextLen;       // length of text in bytes
  long lPictLen;       // length of picture in bytes
  long lPictDownLen;   // length of down picture in bytes
  //TCHAR szText[];    // optional NULL terminated text
  //BYTE bPict[];      // optional picture
  //BYTE bPictDown[];  // optional down picture
} SS_REC_TYPEBUTTON, FAR* LPSS_REC_TYPEBUTTON;

//--------------------------------------------------------------------
//
//  The SS_REC_TYPECHECK record describes a check box cell type.  The
//  SS_REC_TYPECHECK record is a variable length record which can
//  contain optional text and/or pictures.
//

typedef struct tagSS_REC_TYPECHECK
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  short nPictUpType;      // type of unchecked picture
  short nPictDownType;    // type of checked picture
  short nPictGrayType;    // type of grayed picture
  short nPictFocUpType;   // type of focus unchecked picture
  short nPictFocDownType; // type of focus checked picture
  short nPictFocGrayType; // type of focus grayed picture
  short nPictDisUpType;   // type of disabled unchecked picture
  short nPictDisDownType; // type of disabled checked picture
  short nPictDisGrayType; // type of disabled grayed picture
  long lTextLen;          // length of text in bytes
  long lPictUpLen;        // length of unchecked picture in bytes
  long lPictDownLen;      // length of checked picture in bytes
  long lPictGrayLen;      // length of grayed picture in bytes
  long lPictFocUpLen;     // length of focus unchecked picture in bytes
  long lPictFocDownLen;   // length of focus checked picture in bytes
  long lPictFocGrayLen;   // length of focus grayed picture in bytes
  long lPictDisUpLen;     // length of disabled unchecked picture in bytes
  long lPictDisDownLen;   // length of disabled checked picture in bytes
  long lPictDisGrayLen;   // length of disabled grayed picture in bytes
  //TCHAR szText[];       // optional NULL terminated text
  //BYTE bPictUp[];       // optional unchecked picture
  //BYTE bPictDown[];     // optional checked picture
  //BYTE bPictGray[];     // optional grayed picture
  //BYTE bPictFocUp[];    // optional focus unchecked picture
  //BYTE bPictFocDown[];  // optional focus checked picture
  //BYTE bPictFocGray[];  // optional focus grayed picture
  //BYTE bPictDisUp[];    // optional disabled unchecked picture
  //BYTE bPictDisDown[];  // optional disabled checked picture
  //BYTE bPictDisGray[];  // optional disabled grayed picture
} SS_REC_TYPECHECK, FAR* LPSS_REC_TYPECHECK;

//--------------------------------------------------------------------
//
//  The SS_REC_TYPECOMBO record describes a combo box cell type.  The
//  SS_REC_TYPECOMBO record is a variable length record which contains
//  a NULL terminated, TAB separated list of combo box items.
//

typedef struct tagSS_REC_TYPECOMBO
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  //TCHAR lpszList[];  // variable length portion of record
} SS_REC_TYPECOMBO, FAR* LPSS_REC_TYPECOMBO;

//--------------------------------------------------------------------
//
//  The SS_REC_TYPECOMBOEX record describes a combo box cell type.  The
//  SS_REC_TYPECOMBOEX record is a variable length record which contains
//  a NULL terminated, TAB separated list of combo box items.  This
//  record contains more info than the SS_REC_TYPECOMBO record.
//

typedef struct tagSS_REC_TYPECOMBOEX
{
  REC_COMMON();
  long  lCol;
  long  lRow;
  long  lStyle;
  short dMaxRows;
  short dMaxEditLen;
  short dComboWidth;
  //TCHAR lpszList[];  // variable length portion of record
} SS_REC_TYPECOMBOEX, FAR* LPSS_REC_TYPECOMBOEX;

//--------------------------------------------------------------------
//
//  The SS_REC_TYPECOMBOEX4 record appends info to the SS_REC_TYPECOMBOEX record.
//  This record assumes that an SS_REC_TYPECOMBOEX record was previously saved.
//

typedef struct tagSS_REC_TYPECOMBOEX4
{
  REC_COMMON();
  long lCol;
  long lRow;
  BYTE bAutoSearch;
} SS_REC_TYPECOMBOEX4, FAR* LPSS_REC_TYPECOMBOEX4;
//--------------------------------------------------------------------
//
//  The SS_REC_TYPEDATE record describes a date cell type.
//
//  Note: Unicode requires 16-bit character.
//

typedef struct tagSS_REC_TYPEDATE
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  BYTE bSetFormat;
  BYTE bSetRange;
  short nFormat;
  short nSepChar;
  BYTE bCentury;
  BYTE bSpin;
  short nMinYear;
  short nMinMon;
  short nMinDay;
  short nMaxYear;
  short nMaxMon;
  short nMaxDay;
} SS_REC_TYPEDATE, FAR* LPSS_REC_TYPEDATE;

//--------------------------------------------------------------------
//
// The SS_REC_TYPEEDIT record describes a edit cell type.
//

typedef struct tagSS_REC_TYPEEDIT
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  long lLen;
  short nChrSet;
  short nChrCase;
} SS_REC_TYPEEDIT, FAR* LPSS_REC_TYPEEDIT;

//--------------------------------------------------------------------
//
// The SS_REC_TYPEFORMAT record describes a float cell type.
//
//  Note: Unicode requires 16-bit characters.
//

typedef struct tagSS_REC_TYPEFLOAT
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  short nLeft;
  short nRight;
  double dfMin;
  double dfMax;
  BYTE bSetFormat;
  short nCurrChar;
  short nSepChar;
  short nDecChar;
} SS_REC_TYPEFLOAT, FAR* LPSS_REC_TYPEFLOAT;

//--------------------------------------------------------------------
//
//  The SS_REC_TYPEINTEGER record describes an integer cell type.
//

typedef struct tagSS_REC_TYPEINTEGER
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  long lMin;
  long lMax;
  long lSpinInc;
  BYTE bSpinWrap;
} SS_REC_TYPEINTEGER, FAR* LPSS_REC_TYPEINTEGER;

//--------------------------------------------------------------------
//
//  The SS_REC_TYPEOWNERDRAW record cdescribes an owner drawn cell
//  type.
//

typedef struct tagSS_REC_TYPEOWNERDRAW
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
} SS_REC_TYPEOWNERDRAW, FAR* LPSS_REC_TYPEOWNERDRAW;

//--------------------------------------------------------------------
//
// The SS_REC_TYPEPIC record describes a PIC cell type.
//

typedef struct tagSS_REC_TYPEPIC
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  long lMaskLen;     // number of bytes in szMask[] field
  //TCHAR szMask[];  // optional variable length portion of record
} SS_REC_TYPEPIC, FAR* LPSS_REC_TYPEPIC;

//--------------------------------------------------------------------
//
//  The SS_REC_TYPEPICTURE record describes a picture cell type.
//

typedef struct tagSS_REC_TYPEPICTURE
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  long lPictLen;   // number of bytes in bPict[] field
  //BYTE bPict[];  // optional variable length portion of record
} SS_REC_TYPEPICTURE, FAR* LPSS_REC_TYPEPICTURE;

//--------------------------------------------------------------------
//
//  The SS_REC_TYPESTATIC record describs a static text cell type.
//

typedef struct tagSS_REC_TYPESTATIC
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
} SS_REC_TYPESTATIC, FAR* LPSS_REC_TYPESTATIC;

//--------------------------------------------------------------------
//
//  The SS_REC_TYPETIME record describes a time cell type.
//
//  Note: Unicode requires 16-bit characters.
//

typedef struct tagSS_REC_TYPETIME
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  BYTE bSetFormat;
  BYTE bSetRange;
  BYTE b24Hour;
  BYTE bSeconds;
  short nSepChar;
  BYTE bSpin;
  short nMinHour;
  short nMinMin;
  short nMinSec;
  short nMaxHour;
  short nMaxMin;
  short nMaxSec;
} SS_REC_TYPETIME, FAR* LPSS_REC_TYPETIME;

//--------------------------------------------------------------------
//
//  The SS_REC_UNITTYPE record stores unit type option.
//

typedef struct tagSS_REC_UNITTYPE
{
  REC_COMMON();
  short nUnitType;
} SS_REC_UNITTYPE, FAR* LPSS_REC_UNITTYPE;

//--------------------------------------------------------------------
//
//  The SS_REC_USERRESIZE record determines if the user can resize
//  column and/or rows.
//

typedef struct tagSS_REC_USERRESIZE
{
  REC_COMMON();
  WORD wResize;
} SS_REC_USERRESIZE, FAR* LPSS_REC_USERRESIZE;

//--------------------------------------------------------------------
//
//  The SS_REC_USERRESIZECOL record determines if the user can resize
//  an individual column.
//

typedef struct tagSS_REC_USERRESIZECOL
{
  REC_COMMON();
  long lCol;
  short nResize;
} SS_REC_USERRESIZECOL, FAR* LPSS_REC_USERRESIZECOL;

//--------------------------------------------------------------------
//
//  The SS_REC_USERRESIZEROW record determines if the user can resize
//  an individual row.
//

typedef struct tagSS_REC_USERRESIZEROW
{
  REC_COMMON();
  long lRow;
  short nResize;
} SS_REC_USERRESIZEROW, FAR* LPSS_REC_USERRESIZEROW;

//--------------------------------------------------------------------
//
//  The SS_REC_VALBDL record stores the value (as double) of a cell,
//  column, row, or spreadsheet.
//

typedef struct tagSS_REC_VALDBL
{
  REC_COMMON();
  long lCol;
  long lRow;
  double dfVal;
} SS_REC_VALDBL, FAR* LPSS_REC_VALDBL;

//--------------------------------------------------------------------
//
//  The SS_REC_VALLONG record stores the value (as long) of a cell,
//  column, row, or spreadsheet.
//

typedef struct tagSS_REC_VALLONG
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lVal;
} SS_REC_VALLONG, FAR* LPSS_REC_VALLONG;

//--------------------------------------------------------------------
//
//  The SS_REC_VALSTR record contains the value (as string) of a cell,
//  column, row, or spreadsheet.  The SS_REC_VALSTR record is a
//  variable length record which contains NULL terminated text.
//

typedef struct tagSS_REC_VALSTR
{
  REC_COMMON();
  long lCol;
  long lRow;
  //TCHAR szValue[];  // variable length portion of record
} SS_REC_VALSTR, FAR* LPSS_REC_VALSTR;

//--------------------------------------------------------------------
//
//  The SS_REC_VIRTUAL record stores virtual mode options.
//

typedef struct tagSS_REC_VIRTUAL
{
  REC_COMMON();
  BYTE bVMode;
  BYTE bUseVScroll;
  WORD wVScrollOptions;
  long lVStyle;
  long lVMax;
  long lVTop;
  long lVSize;
  long lVOverlap;
  long lVPhysSize;
} SS_REC_VIRTUAL, FAR* LPSS_REC_VIRTUAL;

#ifdef SS_V35
//--------------------------------------------------------------------
//
//  The SS_REC_CELLNOTE record
//

typedef struct tagSS_REC_CELLNOTE
{
  REC_COMMON();
  long lCol;
  long lRow;
  //TCHAR szValue[];  // variable length portion of record
} SS_REC_CELLNOTE, FAR* LPSS_REC_CELLNOTE;

//--------------------------------------------------------------------
//
//  The SS_REC_CELLNOTEINDICATOR record
//

typedef struct tagSS_REC_CELLNOTEINDICATOR
{
  REC_COMMON();
  short nCellNoteIndicator;
} SS_REC_CELLNOTEINDICATOR, FAR* LPSS_REC_CELLNOTEINDICATOR;

#ifdef SS_V80
//--------------------------------------------------------------------
//
//  The SS_REC_ENHANCEDCOLORS describes the background and foreground colors
//  of a cell, column, row, or spreadsheet.
//

typedef struct tagSS_REC_ENHANCEDCOLORS
{
  REC_COMMON();
COLORREF scrollTrackColor;
COLORREF scrollArrowColor;
COLORREF scrollBorderColor;
COLORREF scrollUpperNormalStartColor;
COLORREF scrollUpperNormalEndColor;
COLORREF scrollLowerNormalStartColor;
COLORREF scrollLowerNormalEndColor;
COLORREF scrollUpperHoverStartColor;
COLORREF scrollUpperHoverEndColor;
COLORREF scrollLowerHoverStartColor;
COLORREF scrollLowerHoverEndColor;
COLORREF scrollUpperPushedStartColor;
COLORREF scrollUpperPushedEndColor;
COLORREF scrollLowerPushedStartColor;
COLORREF scrollLowerPushedEndColor;
COLORREF scrollHoverBorderColor;
COLORREF rowHeaderSelectedBorderColor;
COLORREF rowHeaderBorderColor;
COLORREF rowHoverUpperColor;
COLORREF rowHoverLowerColor;
COLORREF rowSelectedHoverUpperColor;
COLORREF rowSelectedHoverLowerColor;
COLORREF rowSelectedUpperColor;
COLORREF rowSelectedLowerColor;
COLORREF rowUpperColor;
COLORREF rowLowerColor;
COLORREF cornerBackColor;
COLORREF cornerHoverColor;
COLORREF cornerTriangleColor;
COLORREF cornerTriangleHoverColor;
COLORREF cornerTriangleBorderColor;
COLORREF cornerTriangleHoverBorderColor;
COLORREF colHeaderSelectedBorderColor;
COLORREF colHeaderBorderColor;
COLORREF colHoverUpperColor;
COLORREF colHoverLowerColor;
COLORREF colSelectedHoverUpperColor;
COLORREF colSelectedHoverLowerColor;
COLORREF colSelectedUpperColor;
COLORREF colSelectedLowerColor;
COLORREF colUpperColor;
COLORREF colLowerColor;
COLORREF sheetTabUpperNormalStartColor;
COLORREF sheetTabUpperNormalEndColor;
COLORREF sheetTabLowerNormalStartColor;
COLORREF sheetTabLowerNormalEndColor;
COLORREF sheetTabUpperHoverStartColor;
COLORREF sheetTabUpperHoverEndColor;
COLORREF sheetTabLowerHoverStartColor;
COLORREF sheetTabLowerHoverEndColor;
COLORREF sheetTabInnerBorderColor;
COLORREF sheetTabOuterBorderColor;
COLORREF sheetGrayAreaStartColor;
COLORREF sheetGrayAreaEndColor;
COLORREF sheetSplitBoxStartColor;
COLORREF sheetSplitBoxEndColor;
COLORREF sheetScrollArrowColor;
COLORREF sheetScrollUpperNormalStartColor;
COLORREF sheetScrollUpperNormalEndColor;
COLORREF sheetScrollLowerNormalStartColor;
COLORREF sheetScrollLowerNormalEndColor;
COLORREF sheetScrollUpperHoverStartColor;
COLORREF sheetScrollUpperHoverEndColor;
COLORREF sheetScrollLowerHoverStartColor;
COLORREF sheetScrollLowerHoverEndColor;
COLORREF sheetScrollUpperPushedStartColor;
COLORREF sheetScrollUpperPushedEndColor;
COLORREF sheetScrollLowerPushedStartColor;
COLORREF sheetScrollLowerPushedEndColor;
COLORREF sheetTabForeColor;
COLORREF sheetTabActiveForeColor;

} SS_REC_ENHANCEDCOLORS, FAR* LPSS_REC_ENHANCEDCOLORS;

//--------------------------------------------------------------------
//
//  The SS_REC_TABSTRIPBUTTONPOLICY record stores the current sheet index
//
typedef struct tagSS_REC_TABSTRIPBUTTONPOLICY
{
  REC_COMMON();
  WORD wTabStripButtonPolicy;
} SS_REC_TABSTRIPBUTTONPOLICY, FAR* LPSS_REC_TABSTRIPBUTTONPOLICY;

//--------------------------------------------------------------------
//
//  The SS_REC_CELLNOTEINDICATOR2 record
//

typedef struct tagSS_REC_CELLNOTEINDICATOR2
{
  REC_COMMON();
  COLORREF  clrCellNoteIndicator;
  short nCellNoteIndicatorShape;
} SS_REC_CELLNOTEINDICATOR2, FAR* LPSS_REC_CELLNOTEINDICATOR2;
#endif

//--------------------------------------------------------------------
//
//  The SS_REC_SHOWSCROLLTIPS record
//

typedef struct tagSS_REC_SHOWSCROLLTIPS
{
  REC_COMMON();
  short nShowScrollTips;
} SS_REC_SHOWSCROLLTIPS, FAR* LPSS_REC_SHOWSCROLLTIPS;


//--------------------------------------------------------------------
//
//  The SS_REC_USERCOLACTION record
//

typedef struct tagSS_REC_USERCOLACTION
{
  REC_COMMON();
  short nUserColAction;
} SS_REC_USERCOLACTION, FAR* LPSS_REC_USERCOLACTION;

//--------------------------------------------------------------------
//
//  The SS_REC_COLUSERSORTINDICATOR record
//

typedef struct tagSS_REC_COLUSERSORTINDICATOR
{
  REC_COMMON();
  long lCol;
  short nColUserSortIndicator;
} SS_REC_COLUSERSORTINDICATOR, FAR* LPSS_REC_COLUSERSORTINDICATOR;

//--------------------------------------------------------------------
//
//  The SS_REC_SCRIPTENHANCED record
//

typedef struct tagSS_REC_SCRIPTENHANCED
{
  REC_COMMON();
  BOOL bScriptEnhanced;
} SS_REC_SCRIPTENHANCED, FAR* LPSS_REC_SCRIPTENHANCED;

typedef struct tagSS_REC_CALTEXTOVERRIDE
{
  REC_COMMON();
  short nShortDays;
  short nLongDays;
  short nShortMonths;
  short nLongMonths;
  short nOkText;
  short nCancelText;
  //TCHAR szText[];    // variable length portion of record;
} SS_REC_CALTEXTOVERRIDE, FAR* LPSS_REC_CALTEXTOVERRIDE;

#endif //SS_V35

//--------------------------------------------------------------------
//
//  The SS_SUBREC_CUSTFUNC record describes a custom function
//  and is a subrecord of the SS_CUSTFUNCTABLE record.  The
//  SS_SUBREC_CUSTFUNC record is a variable length record which
//  contains NULL terminated text.
//

typedef struct tagSS_SUBREC_CUSTFUNC
{
  short nMinArgs;
  short nMaxArgs;
  long lFlags;
  long lNameLen;     // number of bytes in szName field;
  //TCHAR szName[];  // variable length portion of record;
} SS_SUBREC_CUSTFUNC, FAR* LPSS_SUBREC_CUSTFUNC;

//--------------------------------------------------------------------
//
//  The SS_SUBREC_CUSTNAME record describes a custom name
//  and is a subrecord of the SS_CUSTNAMETABLE record.  The
//  SS_SUBREC_CUSTNAME record is a variable length record which
//  contains NULL terminated text and a parsed expression.
//

typedef struct tagSS_SUBREC_CUSTNAME
{
  long lTextLen;
  long lExprLen;
  //TCHAR szText[];  // variable length NULL terminated text
  //BYTE bExpr[];    // variable length parsed expression
} SS_SUBREC_CUSTNAME, FAR* LPSS_SUBREC_CUSTNAME;

//--------------------------------------------------------------------
//
//  The SS_SUBREC_DDE record describes a DDE link and is a subrecord
//  of the SS_REC_DDETABLE record.  The SS_SUBREC_DDE record is a
//  variable length record which contains NULL terminated text.
//

typedef struct tagSS_SUBREC_DDE
{
  long lServerLen;     // number of bytes in server name
  long lTopicLen;      // number of bytes in topic name
  long lItemLen;       // number of bytes in item name
  long lReadExprLen;   // number of bytes in read expression
  long lWriteExprLen;  // number of bytes in write expression
  short nMode;
  //TCHAR szServer[];  // variable portion of record
  //TCHAR szTopic[];
  //TCHAR szItem[];
  //TCHAR szReadExpr[];
  //TCHAR szWriteExpr[];
} SS_SUBREC_DDE, FAR* LPSS_SUBREC_DDE;

//--------------------------------------------------------------------
//
//  The following records are subrecords of a parsed expression.
//

typedef struct tagSS_SUBREC_EXPRCOMMON
{
  short nType;
}  SS_SUBREC_EXPRCOMMON, FAR* LPSS_SUBREC_EXPRCOMMON;

typedef struct tagSS_SUBREC_EXPRLONG
{
  short nType;
  long lVal;
} SS_SUBREC_EXPRLONG, FAR* LPSS_SUBREC_EXPRLONG;

typedef struct tagSS_SUBREC_EXPRDOUBLE
{
  short nType;
  double dfVal;
} SS_SUBREC_EXPRDOUBLE, FAR* LPSS_SUBREC_EXPRDOUBLE;

typedef struct tagSS_SUBREC_EXPRSTR
{
  short nType;
  short nLen;
  //TCHAR szText;
} SS_SUBREC_EXPRSTR, FAR* LPSS_SUBREC_EXPRSTR;

typedef struct tagSS_SUBREC_EXPRCELL
{
  short nType;
  long lCol;
  long lRow;
} SS_SUBREC_EXPRCELL, FAR* LPSS_SUBREC_EXPRCELL;

typedef struct tagSS_SUBREC_EXPRANGE
{
  short nType;
  long lCol1;
  long lRow1;
  long lCol2;
  long lRow2;
} SS_SUBREC_EXPRRANGE, FAR* LPSS_SUBREC_EXPRRANGE;

#if defined(SS_V70)
typedef struct tagSS_SUBREC_EXPREXTERNALCELL
{
  short nType;
  short nSheet;
  long lCol;
  long lRow;
} SS_SUBREC_EXPREXTERNALCELL, FAR* LPSS_SUBREC_EXPREXTERNALCELL;
#endif

#if defined(SS_V70)
typedef struct tagSS_SUBREC_EXPREXTERNALRANGE
{
  short nType;
  short nSheet;
  long lCol1;
  long lRow1;
  long lCol2;
  long lRow2;
} SS_SUBREC_EXPREXTERNALRANGE, FAR* LPSS_SUBREC_EXPREXTERNALRANGE;
#endif

typedef struct tagSS_SUBREC_EXPRCUSTNAME
{
  short nType;
  short nId;
} SS_SUBREC_EXPRCUSTNAME, FAR* LPSS_SUBREC_EXPRCUSTNAME;

typedef struct tagSS_SUBREC_EXPRFUNC
{
  short nType;
  short nId;
  short nArgs;
} SS_SUBREC_EXPRFUNC, FAR* LPSS_SUBREC_EXPRFUNC;

typedef struct tagSS_SUBREC_EXPRCUSTFUNC
{
  short nType;
  short nId;
  short nArgs;
} SS_SUBREC_EXPRCUSTFUNC, FAR* LPSS_SUBREC_EXPRCUSTFUNC;

typedef struct tagSS_SUBREC_EXPROPER
{
  short nType;
  short nId;
} SS_SUBREC_EXPROPER, FAR* LPSS_SUBREC_EXPROPER;

typedef struct tagSS_SUBREC_EXPRSEP
{
  short nType;
  short nId;
} SS_SUBREC_EXPRSEP, FAR* LPSS_SUBREC_EXPRSEP;

typedef struct tagSS_SUBREC_EXPRDDE
{
  short nType;
  short nId;
  long lItemCol;
  long lItemRow;
} SS_SUBREC_EXPRDDE, FAR* LPSS_SUBREC_EXPRDDE;

typedef struct tagSS_SUBREC_EXPRERROR
{
  short nType;
  short nId;
} SS_SUBREC_EXPRERROR, FAR* LPSS_SUBREC_EXPRERROR;

//--------------------------------------------------------------------
//
//  The SS_SUBREC_LOGFONT record describes a font and is a subrecord
//  of the SS_FONTTABLE record.  The SS_SUBREC_LOGFONT record is a
//  variable length record which contains NULL terminated text.  The
//  lfHeight field is in units of 1/100 of a point.
//

typedef struct tagSS_SUBREC_LOGFONT
{
  long lfHeight;
  long lfEscapement;
  long lfOrientation;
  long lfWeight;
  BYTE lfItalic;
  BYTE lfUnderline;
  BYTE lfStrikeOut;
  BYTE lfCharSet;
  BYTE lfOutPrecision;
  BYTE lfClipPrecision;
  BYTE lfQuality;
  BYTE lfPitchAndFamily;
  long lfFaceNameLen;    // number of bytes in lfFaceName[] field
  //TCHAR lfFaceName[];  // variable length portion of record
} SS_SUBREC_LOGFONT, FAR* LPSS_SUBREC_LOGFONT;

//--------------------------------------------------------------------
//
//  The SS_SUBREC_LOGFONTOLD record describes a font and is a subrecord
//  of the SS_FONTTABLEOLD record.  The SS_SUBREC_LOGFONTOLD record is
//  a variable length record which contains NULL terminated text.  The
//  lfHeight field is in units of points.  The lfWidth field is always
//  zero.
//

typedef struct tagSS_SUBREC_LOGFONTOLD
{
  long lfHeight;
  long lfWidth;
  long lfEscapement;
  long lfOrientation;
  long lfWeight;
  BYTE lfItalic;
  BYTE lfUnderline;
  BYTE lfStrikeOut;
  BYTE lfCharSet;
  BYTE lfOutPrecision;
  BYTE lfClipPrecision;
  BYTE lfQuality;
  BYTE lfPitchAndFamily;
  long lfFaceNameLen;    // number of bytes in lfFaceName[] field
  //TCHAR lfFaceName[];  // variable length portion of record
} SS_SUBREC_LOGFONTOLD, FAR* LPSS_SUBREC_LOGFONTOLD;
//--------------------------------------------------------------------
//
//  The SS_REC_PRINTSHEET record
//

typedef struct tagSS_REC_PRINTBOOK
{
  REC_COMMON();
  long lAbortMsgLen;    // length in bytes of footer
  long lJobNameLen;    // length in bytes of header
  //TCHAR szAbortMsg[];
  //TCHAR szJobName[];
} SS_REC_PRINTBOOK, FAR* LPSS_REC_PRINTBOOK;

//--------------------------------------------------------------------
//
//  The SS_REC_PRINTSHEET record
//

typedef struct tagSS_REC_PRINTSHEET
{
  REC_COMMON();
  BYTE bDrawBorder;
  BYTE bDrawColor;
  BYTE bDrawShadows;
  BYTE bShowGrid;
  BYTE bShowColHeaders;
  BYTE bShowRowHeaders;
  BYTE bUseDataMax;
  short nPrintType;
  short nPageEnd;
  short nPageStart;
  float fMarginLeft;
  float fMarginTop;
  float fMarginBottom;
  float fMarginRight;
  short nOrientation;
  long x1CellRange;
  long y1CellRange;
  long x2CellRange;
  long y2CellRange;
  long lFooterLen;    // length in bytes of footer
  long lHeaderLen;    // length in bytes of header
  BYTE bSmartPrint;
  short nPageOrder;
  long lFirstPageNumber;
  WORD   wScalingMethod;
  double ZoomFactor;
  short  nBestFitPagesWide;
  short  nBestFitPagesTall;
  BOOL   fCenterOnPageH;
  BOOL   fCenterOnPageV;
  //TCHAR szFooter[];
  //TCHAR szHeader[];
} SS_REC_PRINTSHEET, FAR* LPSS_REC_PRINTSHEET;

#ifdef SS_V30
//--------------------------------------------------------------------
//
//  The SS_REC_ACTIONKEY record determines the function keys assiged to
//  which action.
//     0 - Clear
//     1 - Current date/time
//     2 - Popup calendar
//

typedef struct tagSS_REC_ACTIONKEY
{
  REC_COMMON();
  DWORD Keys[3];
} SS_REC_ACTIONKEY, FAR* LPSS_REC_ACTIONKEY;

//--------------------------------------------------------------------
//
//  The SS_REC_ODDEVENROWCOLOR record determines the foreground and
//  background colors of odd and even rows.
//

typedef struct tagSS_REC_ODDEVENROWCOLOR
{
  REC_COMMON();
  COLORREF clrBackOdd;
  COLORREF clrForeOdd;
  COLORREF clrBackEven;
  COLORREF clrForeEven;
} SS_REC_ODDEVENROWCOLOR, FAR* LPSS_REC_ODDEVENROWCOLOR;

//--------------------------------------------------------------------
//
//  The SS_REC_APPEARANCE record changes the non-client border
//

typedef struct tagSS_REC_APPEARANCE
{
  REC_COMMON();
  BYTE bAppearance;
} SS_REC_APPEARANCE, FAR* LPSS_REC_APPEARANCE;

//--------------------------------------------------------------------
//
//  The SS_REC_TEXTTIP record determines if Text tips should be displayed
//  and their appearance.
//

typedef struct tagSS_REC_TEXTTIP
{
  REC_COMMON();
  BYTE     bStatus;
  long     lDelay;
  COLORREF clrBack;
  COLORREF clrFore;
  //SS_SUBREC_LOGFONT[];  // variable portion of record
} SS_REC_TEXTTIP, FAR* LPSS_REC_TEXTTIP;

//--------------------------------------------------------------------
//
//  The SS_REC_SCROLLBARTRACK record determines if the spreadsheet
//  tracks when the scroll bar thumb is moved.
//

typedef struct tagSS_REC_SCROLLBARTRACK
{
  REC_COMMON();
  BYTE fVTrack;
  BYTE fHTrack;
} SS_REC_SCROLLBARTRACK, FAR* LPSS_REC_SCROLLBARTRACK;

//--------------------------------------------------------------------
//
//  The SS_REC_CLIPBOARDOPTIONS record determines how the spreadsheet
//  clips headers to and from the clipboard.
//

typedef struct tagSS_REC_CLIPBOARDOPTIONS
{
  REC_COMMON();
  BYTE bOptions;
} SS_REC_CLIPBOARDOPTIONS, FAR* LPSS_REC_CLIPBOARDOPTIONS;

//--------------------------------------------------------------------
//
//  The SS_REC_TWODIGITYEARMAX record determines how the spreadsheet
//  translates 2 digit years into 4 digits.
//

typedef struct tagSS_REC_TWODIGITYEARMAX
{
  REC_COMMON();
  short nTwoDigitYearMax;
} SS_REC_TWODIGITYEARMAX, FAR* LPSS_REC_TWODIGITYEARMAX;

//--------------------------------------------------------------------
//
//  The SS_REC_COLHIDDEN record determines if a spreadsheet
//  column is hidden.
//

typedef struct tagSS_REC_COLHIDDEN
{
  REC_COMMON();
  long lCol;
  BYTE fHidden;
} SS_REC_COLHIDDEN, FAR* LPSS_REC_COLHIDDEN;

//--------------------------------------------------------------------
//
//  The SS_REC_ROWHIDDEN record determines if a spreadsheet
//  row is hidden.
//

typedef struct tagSS_REC_ROWHIDDEN
{
  REC_COMMON();
  long lRow;
  BYTE fHidden;
} SS_REC_ROWHIDDEN, FAR* LPSS_REC_ROWHIDDEN;

#endif // SS_V30

#ifdef SS_V40

//--------------------------------------------------------------------
//
//  The SS_REC_CELLSPANTABLE record contains a list of all cell spans
//  used in the spreadsheet.  The SS_REC_CELLSPANTABLE record is a variable
//  length record containing zero or more SS_SUBREC_CELLSPAN
//  subrecords.
//

typedef struct tagSS_REC_CELLSPANTABLE
{
  REC_COMMON();
  short nTableCnt;
  //SS_SUBREC_CELLSPAN[];  // variable portion of record
} SS_REC_CELLSPANTABLE, FAR* LPSS_REC_CELLSPANTABLE;

//--------------------------------------------------------------------
//
//  The SS_SUBREC_CELLSPAN record describes a CellSpan and is a subrecord
//  of the SS_CELLSPANTABLE record.
//

typedef struct tagSS_SUBREC_CELLSPAN
{
  long lCol;
  long lRow;
  long lNumCols;
  long lNumRows;
} SS_SUBREC_CELLSPAN, FAR* LPSS_SUBREC_CELLSPAN;

//--------------------------------------------------------------------
//
//  The SS_REC_COLMERGE record specifies if a column is to be merged.
//

typedef struct tagSS_REC_COLMERGE
{
  REC_COMMON();
  long lCol;
  BYTE bMerge;
} SS_REC_COLMERGE, FAR* LPSS_REC_COLMERGE;

//--------------------------------------------------------------------
//
//  The SS_REC_COLID record contains the name of a col.
//

typedef struct tagSS_REC_COLID
{
  REC_COMMON();
  long lCol;
  //TCHAR szValue[];  // variable length portion of record
} SS_REC_COLID, FAR* LPSS_REC_COLID;

//--------------------------------------------------------------------
//
//  The SS_REC_ROWMERGE record specifies if a column is to be merged.
//

typedef struct tagSS_REC_ROWMERGE
{
  REC_COMMON();
  long lRow;
  BYTE bMerge;
} SS_REC_ROWMERGE, FAR* LPSS_REC_ROWMERGE;

//--------------------------------------------------------------------
//
//  The SS_REC_HEADER4record stores column and row header options.
//

typedef struct tagSS_REC_HEADER4
{
  REC_COMMON();
  long lRowHeaderCols;
  long lRowHeadersAutoTextIndex;
  long lColHeaderRows;
  long lColHeadersAutoTextIndex;
  long lColHeadersUserSortIndex;
} SS_REC_HEADER4, FAR* LPSS_REC_HEADER4;

//--------------------------------------------------------------------
//
//  The SS_REC_DISPLAY4 record stores display options.
//

typedef struct tagSS_REC_DISPLAY4
{
  REC_COMMON();
  BYTE bAllowEditOverflow;
} SS_REC_DISPLAY4, FAR* LPSS_REC_DISPLAY4;

//--------------------------------------------------------------------
//
//  The SS_REC_CELLTAG record
//

typedef struct tagSS_REC_CELLTAG
{
  REC_COMMON();
  long lCol;
  long lRow;
  //TCHAR szValue[];  // variable length portion of record
} SS_REC_CELLTAG, FAR* LPSS_REC_CELLTAG;

//--------------------------------------------------------------------
//
// The SS_REC_TYPECURRENCY record describes a currency cell type.
//

typedef struct tagSS_REC_TYPECURRENCY
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  double dfMin;
  double dfMax;
  BYTE Right;
  BYTE bLeadingZero;         // 0 - 2
  BYTE bNegStyle;            // 0 - 16
  BYTE bPosStyle;            // 0 - 5
  BYTE fShowSeparator;       // T/F
  BYTE fShowCurrencySymbol;  // T/F
  BYTE fSpin;
  BYTE fSpinWrap;
  double SpinInc;
  BYTE bDecLen;
  BYTE bSepLen;
  BYTE bCurLen;
  //TCHAR  szDecimal[3 + 1];   // variable length portion of record
  //TCHAR  szSeparator[3 + 1]; // variable length portion of record
  //TCHAR  szCurrency[5 + 1];  // variable length portion of record
} SS_REC_TYPECURRENCY, FAR* LPSS_REC_TYPECURRENCY;

//--------------------------------------------------------------------
//
// The SS_REC_TYPENUMBER record describes a number cell type.
//

typedef struct tagSS_REC_TYPENUMBER
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  double dfMin;
  double dfMax;
  BYTE Right;
  BYTE bLeadingZero;         // 0 - 2
  BYTE bNegStyle;
  BYTE fShowSeparator;       // T/F
  BYTE fSpin;
  BYTE fSpinWrap;
  double SpinInc;
  BYTE bDecLen;
  BYTE bSepLen;
  //TCHAR  szDecimal[3 + 1];   // variable length portion of record
  //TCHAR  szSeparator[3 + 1]; // variable length portion of record
} SS_REC_TYPENUMBER, FAR* LPSS_REC_TYPENUMBER;

//--------------------------------------------------------------------
//
// The SS_REC_TYPEPERCENT record describes a percent cell type.
//

typedef struct tagSS_REC_TYPEPERCENT
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  double dfMin;
  double dfMax;
  BYTE Right;
  BYTE bNegStyle;            // 0 - 8
  BYTE fSpin;
  BYTE fSpinWrap;
  double SpinInc;
  BYTE bLeadingZero;         // T/F
  // The following are here in case we choose to add these features in the future
  BYTE bPosStyle;            // 0 - 5
  BYTE fShowSeparator;       // T/F
  BYTE fShowSymbol;          // T/F
  BYTE bSymbolLen;
  // End of not used
  BYTE bDecLen;
  //TCHAR  szDecimal[3 + 1];   // variable length portion of record
} SS_REC_TYPEPERCENT, FAR* LPSS_REC_TYPEPERCENT;

#endif // SS_V40

#ifdef SS_V70

//--------------------------------------------------------------------
//
// The SS_REC_TYPESCIENTIFIC record describes a number cell type.
//

typedef struct tagSS_REC_TYPESCIENTIFIC
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  double dfMin;
  double dfMax;
  BYTE Right;
  BYTE bDecLen;
  //TCHAR  szDecimal[3 + 1];   // variable length portion of record
} SS_REC_TYPESCIENTIFIC, FAR* LPSS_REC_TYPESCIENTIFIC;

//--------------------------------------------------------------------
//
//  The SS_REC_SHEETINDEX record stores the current sheet index
//
typedef struct tagSS_REC_SHEETINDEX
{
  REC_COMMON();
  short nSheet;
} SS_REC_SHEETINDEX, FAR* LPSS_REC_SHEETINDEX;

//--------------------------------------------------------------------
//
//  The SS_REC_ACTIVESHEET record stores the current sheet index
//
typedef struct tagSS_REC_ACTIVESHEET
{
  REC_COMMON();
  short nActiveSheet;
} SS_REC_ACTIVESHEET, FAR* LPSS_REC_ACTIVESHEET;

//--------------------------------------------------------------------
//
//  The SS_REC_SHEETCOUNT record stores the current sheet index
//
typedef struct tagSS_REC_SHEETCOUNT
{
  REC_COMMON();
  short nSheetCount;
} SS_REC_SHEETCOUNT, FAR* LPSS_REC_SHEETCOUNT;

#ifdef SS_V80
//--------------------------------------------------------------------
//
//  The SS_REC_PRINTDEVMODE record stores the current sheet index
//
typedef struct tagSS_REC_PRINTDEVMODE
{
  REC_COMMON();
  DEVMODE PrintDevMode;
} SS_REC_PRINTDEVMODE, FAR* LPSS_REC_PRINTDEVMODE;

// fix for 99914272/99916363 -scl
// define structs actually saved by each build
#pragma pack(push)
#pragma pack(1)
// 32-bit ANSI (1-byte backing, WINVER<5):
typedef struct _devicemodeA32 {
    BYTE   dmDeviceName[CCHDEVICENAME];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    union {
      /* printer only fields */
      struct {
        short dmOrientation;
        short dmPaperSize;
        short dmPaperLength;
        short dmPaperWidth;
        short dmScale;
        short dmCopies;
        short dmDefaultSource;
        short dmPrintQuality;
      };
      /* display only fields */
      struct {
        POINTL dmPosition;
        DWORD  dmDisplayOrientation;
        DWORD  dmDisplayFixedOutput; 
      };
    };
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    BYTE   dmFormName[CCHFORMNAME];
    WORD   dmLogPixels;
    DWORD  dmBitsPerPel;
    DWORD  dmPelsWidth;
    DWORD  dmPelsHeight;
    union {
        DWORD  dmDisplayFlags;
        DWORD  dmNup;
    };
    DWORD  dmDisplayFrequency;
#if(WINVER >= 0x0400)
    DWORD  dmICMMethod;
    DWORD  dmICMIntent;
    DWORD  dmMediaType;
    DWORD  dmDitherType;
    DWORD  dmReserved1;
    DWORD  dmReserved2;
//#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
//    DWORD  dmPanningWidth;
//    DWORD  dmPanningHeight;
//#endif
#endif /* WINVER >= 0x0400 */
} DEVMODEA32, *PDEVMODEA32, *NPDEVMODEA32, *LPDEVMODEA32;
// 32-bit Unicode (1-byte backing, WINVER<5):
typedef struct _devicemodeW32 {
    WCHAR  dmDeviceName[CCHDEVICENAME];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    union {
      /* printer only fields */
      struct {
        short dmOrientation;
        short dmPaperSize;
        short dmPaperLength;
        short dmPaperWidth;
        short dmScale;
        short dmCopies;
        short dmDefaultSource;
        short dmPrintQuality;
      };
      /* display only fields */
      struct {
        POINTL dmPosition;
        DWORD  dmDisplayOrientation;
        DWORD  dmDisplayFixedOutput; 
      };
    };
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    WCHAR  dmFormName[CCHFORMNAME];
    WORD   dmLogPixels;
    DWORD  dmBitsPerPel;
    DWORD  dmPelsWidth;
    DWORD  dmPelsHeight;
    union {
        DWORD  dmDisplayFlags;
        DWORD  dmNup;
    };
    DWORD  dmDisplayFrequency;
#if(WINVER >= 0x0400)
    DWORD  dmICMMethod;
    DWORD  dmICMIntent;
    DWORD  dmMediaType;
    DWORD  dmDitherType;
    DWORD  dmReserved1;
    DWORD  dmReserved2;
//#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
//    DWORD  dmPanningWidth;
//    DWORD  dmPanningHeight;
//#endif
#endif /* WINVER >= 0x0400 */
} DEVMODEW32, *PDEVMODEW32, *NPDEVMODEW32, *LPDEVMODEW32;

typedef struct tagSS_REC_PRINTDEVMODEA32
{
  REC_COMMON();
  DEVMODEA32 PrintDevMode;
} SS_REC_PRINTDEVMODEA32, FAR* LPSS_REC_PRINTDEVMODEA32;

typedef struct tagSS_REC_PRINTDEVMODEW1
{
  REC_COMMON();
  DEVMODEW32 PrintDevMode;
} SS_REC_PRINTDEVMODEW32, FAR* LPSS_REC_PRINTDEVMODEW32;

// 64-bit ANSI (1-byte backing, WINVER>=5):
typedef struct _devicemodeA64 {
    BYTE   dmDeviceName[CCHDEVICENAME];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    union {
      /* printer only fields */
      struct {
        short dmOrientation;
        short dmPaperSize;
        short dmPaperLength;
        short dmPaperWidth;
        short dmScale;
        short dmCopies;
        short dmDefaultSource;
        short dmPrintQuality;
      };
      /* display only fields */
      struct {
        POINTL dmPosition;
        DWORD  dmDisplayOrientation;
        DWORD  dmDisplayFixedOutput; 
      };
    };
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    BYTE   dmFormName[CCHFORMNAME];
    WORD   dmLogPixels;
    DWORD  dmBitsPerPel;
    DWORD  dmPelsWidth;
    DWORD  dmPelsHeight;
    union {
        DWORD  dmDisplayFlags;
        DWORD  dmNup;
    };
    DWORD  dmDisplayFrequency;
#if(WINVER >= 0x0400)
    DWORD  dmICMMethod;
    DWORD  dmICMIntent;
    DWORD  dmMediaType;
    DWORD  dmDitherType;
    DWORD  dmReserved1;
    DWORD  dmReserved2;
//#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
    DWORD  dmPanningWidth;
    DWORD  dmPanningHeight;
//#endif
#endif /* WINVER >= 0x0400 */
} DEVMODEA64, *PDEVMODEA64, *NPDEVMODEA64, *LPDEVMODEA64;
// 64-bit Unicode (1-byte backing, WINVER>=5):
typedef struct _devicemodeW64 {
    WCHAR  dmDeviceName[CCHDEVICENAME];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    union {
      /* printer only fields */
      struct {
        short dmOrientation;
        short dmPaperSize;
        short dmPaperLength;
        short dmPaperWidth;
        short dmScale;
        short dmCopies;
        short dmDefaultSource;
        short dmPrintQuality;
      };
      /* display only fields */
      struct {
        POINTL dmPosition;
        DWORD  dmDisplayOrientation;
        DWORD  dmDisplayFixedOutput; 
      };
    };
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    WCHAR  dmFormName[CCHFORMNAME];
    WORD   dmLogPixels;
    DWORD  dmBitsPerPel;
    DWORD  dmPelsWidth;
    DWORD  dmPelsHeight;
    union {
        DWORD  dmDisplayFlags;
        DWORD  dmNup;
    };
    DWORD  dmDisplayFrequency;
#if(WINVER >= 0x0400)
    DWORD  dmICMMethod;
    DWORD  dmICMIntent;
    DWORD  dmMediaType;
    DWORD  dmDitherType;
    DWORD  dmReserved1;
    DWORD  dmReserved2;
//#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
    DWORD  dmPanningWidth;
    DWORD  dmPanningHeight;
//#endif
#endif /* WINVER >= 0x0400 */
} DEVMODEW64, *PDEVMODEW64, *NPDEVMODEW64, *LPDEVMODEW64;

typedef struct tagSS_REC_PRINTDEVMODEA64
{
  REC_COMMON();
  DEVMODEA64 PrintDevMode;
} SS_REC_PRINTDEVMODEA64, FAR* LPSS_REC_PRINTDEVMODEA64;

typedef struct tagSS_REC_PRINTDEVMODEW64
{
  REC_COMMON();
  DEVMODEW64 PrintDevMode;
} SS_REC_PRINTDEVMODEW64, FAR* LPSS_REC_PRINTDEVMODEW64;
#pragma pack(pop)

//--------------------------------------------------------------------
//
//  The SS_REC_APPEARANCESTYLE record stores the current sheet index
//
typedef struct tagSS_REC_APPEARANCESTYLE
{
  REC_COMMON();
  WORD wAppearanceStyle;
} SS_REC_APPEARANCESTYLE, FAR* LPSS_REC_APPEARANCESTYLE;

//--------------------------------------------------------------------
//
//  The SS_REC_SCROLLBARSTYLE record stores the current sheet index
//
typedef struct tagSS_REC_SCROLLBARSTYLE
{
  REC_COMMON();
  WORD wScrollBarStyle;
} SS_REC_SCROLLBARSTYLE, FAR* LPSS_REC_SCROLLBARSTYLE;


//--------------------------------------------------------------------
//
//  The SS_REC_APPEARANCESTYLE record stores the current sheet index
//
typedef struct tagSS_REC_USEVISUALSTYLES
{
  REC_COMMON();
  WORD wUseVisualStyles;
} SS_REC_USEVISUALSTYLES, FAR* LPSS_REC_USEVISUALSTYLES;

//--------------------------------------------------------------------
//
//  The SS_REC_APPEARANCESTYLE record stores the current sheet index
//
typedef struct tagSS_REC_ENHANCESTATICCELLS
{
  REC_COMMON();
  BOOL bEnhanceStaticCells;
} SS_REC_ENHANCESTATICCELLS, FAR* LPSS_REC_ENHANCESTATICCELLS;
//--------------------------------------------------------------------
//
//  The SS_REC_TABENHANCESSHAPE record stores the current sheet index
//
typedef struct tagSS_REC_TABENHANCEDSHAPE
{
  REC_COMMON();
  WORD wTabEnhancedShape;
} SS_REC_TABENHANCEDSHAPE, FAR* LPSS_REC_TABENHANCEDSHAPE;

//--------------------------------------------------------------------
//
//  The SS_REC_HIGHLIGHTHEADERS record stores the Highlight Headers
//
typedef struct tagSS_REC_HIGHLIGHTHEADERS
{
  REC_COMMON();
  WORD wHighlightHeaders;
} SS_REC_HIGHLIGHTHEADERS, FAR* LPSS_REC_HIGHLIGHTHEADERS;

//--------------------------------------------------------------------
//
//  The SS_REC_HIGHLIGHTSTYLE record stores the Highlight Style
//
typedef struct tagSS_REC_HIGHLIGHTSTYLE
{
  REC_COMMON();
  WORD wHighlightStyle;
} SS_REC_HIGHLIGHTSTYLE, FAR* LPSS_REC_HIGHLIGHTSTYLE;

//--------------------------------------------------------------------
//
//  The SS_REC_HIGHLIGHTALPHABLEND record stores the Highlight AlphaBlend
//
typedef struct tagSS_REC_HIGHLIGHTALPHABLEND
{
  REC_COMMON();
  COLORREF clrHighlightAlphaBlend;
  short nHighlightAlphaBlend;
} SS_REC_HIGHLIGHTALPHABLEND, FAR* LPSS_REC_HIGHLIGHTALPHABLEND;

//--------------------------------------------------------------------
//
// The SS_REC_TYPECUSTOM record describes a custom cell type.
//

typedef struct tagSS_REC_TYPECUSTOM
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lStyle;
  long lNameLen;
  long lItemDataType;
  long lItemDataLen;
  //TCHAR  szDecimal[3 + 1];   // variable length portion of record
} SS_REC_TYPECUSTOM, FAR* LPSS_REC_TYPECUSTOM;

//--------------------------------------------------------------------
//
//  The SS_REC_VALBUFFER record contains the value (as buffer) of a cell,
//  column, row, or spreadsheet.  The SS_REC_VALBUFFER record is a
//  variable length record which contains lSize bytes (not NULL terminated).
//

typedef struct tagSS_REC_VALBUFFER
{
  REC_COMMON();
  long lCol;
  long lRow;
  long lSize;
  //TCHAR szValue[];  // variable length portion of record
} SS_REC_VALBUFFER, FAR* LPSS_REC_VALBUFFER;

#endif // SS_V80

//--------------------------------------------------------------------
//
//  The SS_REC_TABSTRIP record stores the current sheet index
//
typedef struct tagSS_REC_TABSTRIP
{
  REC_COMMON();
  WORD wTabStripPolicy;
  double dfTabStripRatio;
  short nFontId;
} SS_REC_TABSTRIP, FAR* LPSS_REC_TABSTRIP;

//--------------------------------------------------------------------
//
//  The SS_REC_SHEETNAME record
//

typedef struct tagSS_REC_SHEETNAME
{
  REC_COMMON();
  //TCHAR szValue[];  // variable length portion of record
} SS_REC_SHEETNAME, FAR* LPSS_REC_SHEETNAME;

//--------------------------------------------------------------------
//
//  The SS_REC_SCROLL7 record
//

typedef struct tagSS_REC_SCROLL7
{
  REC_COMMON();
  short dVScrollWidth;
  short dHScrollHeight;
  COLORREF clrVScrollBar;
  COLORREF clrHScrollBar;
} SS_REC_SCROLL7, FAR* LPSS_REC_SCROLL7;

//--------------------------------------------------------------------
//
//  The SS_REC_SHEETSETTINGS record
//

typedef struct tagSS_REC_SHEETSETTINGS
{
  REC_COMMON();
  BYTE bActiveCellHighlightStyle;
  BYTE fAllowColMove;
  BYTE fAllowRowMove;
  BYTE fSheetVisible;
} SS_REC_SHEETSETTINGS, FAR* LPSS_REC_SHEETSETTINGS;

#endif // SS_V70

//--------------------------------------------------------------------

#pragma pack()

//--------------------------------------------------------------------

/* RAP01d
typedef struct tagSS_BUFF
{
  GLOBALHANDLE hMem;
  BYTE HUGE* pMem;
  long lLen;
  long lAlloc;
} SS_BUFF, FAR* LPSS_BUFF;
*/
// RAP01a >>
typedef struct tagSS_BUFF
{
#ifdef WIN32
  TBGLOBALHANDLE hMem;
#else
  GLOBALHANDLE hMem;
#endif
  BYTE HUGE* pMem;
  long lLen;
  long lAlloc;
#if SS_V80
  FILE *fp;
#endif
} SS_BUFF, FAR* LPSS_BUFF;
// << RAP01a

BOOL SS_BuffInit(LPSS_BUFF pBuff);
#if SS_V80
BOOL SS_BuffInitFP(LPSS_BUFF pBuff, FILE *fp);
#endif
BOOL SS_BuffFree(LPSS_BUFF pBuff);
BOOL SS_BuffCopy(LPSS_BUFF pBuff, HPVOID pMem, long lLen);
BOOL SS_BuffAppend(LPSS_BUFF pBuff, HPVOID pMem, long lLen);
BOOL SS_BuffCat(LPSS_BUFF pBuff, HPVOID pMem);

//--------------------------------------------------------------------

BOOL SS_LoadFromBuffer4(LPSS_BOOK lpBook, HPBYTE lpBuff, long lBuffLen);
GLOBALHANDLE SS_SaveToBuffer4(LPSS_BOOK lpBook, LPLONG lplBuffLen,
                              BOOL bDataOnly, LPBOOL lpfRet);

//--------------------------------------------------------------------

BOOL DLLENTRY SSLoadBlockFromBuffer(HWND hWnd,
                                   SS_COORD lCol, SS_COORD lRow,
                                   SS_COORD lCol2, SS_COORD lRow2,
                                   HPBYTE lpBuff, long lBuffLen);
GLOBALHANDLE DLLENTRY SSSaveBlockToBuffer(HWND hWnd,
                                          SS_COORD lCol, SS_COORD lRow,
                                          SS_COORD lCol2, SS_COORD lRow2,
                                          LPLONG lplBuffLen, BOOL  bDataOnly);
GLOBALHANDLE DLLENTRY SSSaveBlockToBufferExt(HWND hWnd, SS_COORD lCol, SS_COORD lRow,
                                             SS_COORD lCol2, SS_COORD lRow2,
                                             LPLONG lplBuffLen, BOOL  bDataOnly, BOOL fSaveColInfo, BOOL fSaveRowInfo);

HGLOBAL DLLENTRY SSGetXtra(HWND hwndSS, LPLONG lplXtraLen, LPBYTE lpbXtraVer);
BOOL DLLENTRY SSSetXtra(HWND hwndSS, HGLOBAL hXtra, long lXtraLen, BYTE bXtraVer);
HICON SS_Load4Icon(HPBYTE lpMem, long lMemLen);

//--------------------------------------------------------------------

#if defined(SS_OCX)
BOOL SS_OcxResetOnLoad(LPSS_BOOK lpBook);
BOOL SS_OcxLoad4CalcDepend(LPSS_BOOK lpBook, LPSS_REC_CALCDEPEND lpRec);
BOOL SS_OcxSave4CalcDepend(LPSS_BOOK lpBook, LPSS_BUFF lpBuff);
BOOL SS_OcxLoad4Print(LPSS_BOOK lpBook, LPSS_REC_PRINT lpRec);
BOOL SS_OcxSave4Print(LPSS_BOOK lpBook, LPSS_BUFF lpBuff);
BOOL SS_OcxLoad4Print2(LPSS_BOOK lpBook, LPSS_REC_PRINT2 lpRec);
BOOL SS_OcxSave4Print2(LPSS_BOOK lpBook, LPSS_BUFF lpBuff);
#elif defined(SS_VB)
BOOL SS_VbxResetOnLoad(LPSPREADSHEET lpSS);
BOOL SS_VbxLoad4CalcDepend(LPSPREADSHEET lpSS, LPSS_REC_CALCDEPEND lpRec);
BOOL SS_VbxSave4CalcDepend(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff);
BOOL SS_VbxLoad4Print(LPSPREADSHEET lpSS, LPSS_REC_PRINT lpRec);
BOOL SS_VbxSave4Print(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff);
BOOL SS_VbxLoad4Print2(LPSPREADSHEET lpSS, LPSS_REC_PRINT2 lpRec);
BOOL SS_VbxSave4Print2(LPSPREADSHEET lpSS, LPSS_BUFF lpBuff);
#endif
//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
