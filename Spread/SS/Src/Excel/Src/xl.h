/*********************************************************
* XL.h
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
#define TWIP_PER_PT  20 // 1440.0/72.0

#define INDEX_SIZE8  16
#define DBCELL_SIZE8  4

#define EXCEL_MAXROWS 65536
#define EXCEL_MAXCOLS 256

#define EXCEL_MAXBIFF8RECLEN 8224
#define EXCEL_MAXBIFF7RECLEN 2080

#define SS_MAX_INTEGER 32767
#define SS_MIN_INTEGER -32768

#define SS_MAX_FLOAT 99999999999999
#define SS_MIN_FLOAT -99999999999999

//#ifndef RC_INVOKED
//#if defined(_WIN64) || defined(_IA64)
//#pragma pack(8)
//#else
#pragma pack(1)
//#endif
//#endif   // ifndef RC_INVOKED

//BIFF Utility structures========================================
typedef struct tag_biffstr
{
WORD wCharCount;  //count of characters, NOT BYTES!!!
BYTE fHighByte:1; //0-all chars in the string have a high byte of 0x00
BYTE res1:1;
BYTE fExtSt:1;    //extended string follows (Far East only)
BYTE fRichSt:1;   //rich string follows
BYTE res2:4;
BYTE bString[1];  //Array of string characters and formatting runs
}BIFFSTR, FAR *LPBIFFSTR;

//Far East version of Excel's BiffString
typedef struct tag_biffstrFarEast
{
WORD wCharCount;  //count of characters, NOT BYTES!!!
BYTE fHighByte:1; //0-all chars in the string have a high byte of 0x00
BYTE res1:1;
BYTE fExtSt:1;    //extended string follows (Far East only)
BYTE fRichSt:1;   //rich string follows
BYTE res2:4;
DWORD cchExtRst;  // Length of ExtRst Data
BYTE bString[1];  //Array of string characters and formatting runs
// ExtRst -- this follows the string data for the length <cchExtRst>
}BIFFSTRFarEast, FAR *LPBIFFSTRFarEast;

//BIFF Utility structures========================================

typedef struct tag_bof
{
  WORD  wVerNum;    //0500-BIFF5/7,0600-BIFF8
  WORD  wSubStrType;//0005-Workbook globals
                    //0006-VB module
                    //0010-Worksheet or dlg sheet
                    //0020-Chart
                    //0040-Excel 4.0 macro sheet
                    //0100-Workspace file
  WORD  wBuild;     //build id 0DBB for Excel 97
  WORD  wYear;      //build year 07CC for Excel 97
  DWORD dwfh;
  DWORD dwsfo;
}zBOF, FAR *LPzBOF;

typedef struct tag_bundlesht8
{
  DWORD dwBOFPos; //Stream pos for the start of the BOF for the sheet
  WORD  hsState:2;//0-visible,1-hidden,2-very hidden(make visible only thru VB)
  WORD  res1:6;   
  WORD  dt:8;     //0-wksht or dlgsht,1-Excel4.0 macrosht,2-chart,6-VB module 
  WORD  wSheetNameLen;
  TBGLOBALHANDLE ghSheetName;
}xBUNDLESHT8, FAR *LPxBUNDLESHT8;

typedef struct tag_bundlesht
{
  DWORD dwBOFPos; //Stream pos for the start of the BOF for the sheet
  WORD  dt:8;     //0-wksht or dlgsht,1-Excel4.0 macrosht,2-chart,6-VB module 
  WORD  hsState:2;//0-visible,1-hidden,2-very hidden(make visible only thru VB)
  WORD  res1:6;   
  BYTE  bSheetNameLen;
  TBGLOBALHANDLE ghSheetName;
}xBUNDLESHT, FAR *LPxBUNDLESHT;

typedef struct tag_coutry
{
  WORD wiCountryDef;   //default country index
  WORD wiCountryWinIni;//country index from WIN.INI
}zCOUNTRY, FAR *LPzCOUNTRY;

typedef struct tag_font
{
  WORD wHeight;
  WORD res1:1;
  WORD fItalic:1;
  WORD res2:1;
  WORD fStrikeout:1;
  WORD fOutline:1;
  WORD fShadow:1;
  WORD res3:2;
  WORD res4:8;
  WORD wicv;
  WORD wBoldStyle; // a number 100-1000dec for the char weight
  WORD wSsSs;      //Superscript/Subscript:0-none,1-super,2-sub
  BYTE bUls;       //Underline style:0-none,1-single,2-double,21h-single acct,22h-dbl acct
  BYTE bFamily;    //Font family - LOGFONT
  BYTE bCharSet;   //Character set - LOGFONT
  BYTE res5;
  BYTE bFontNameLen;
  BYTE bStrType  ;   //0-1byte/char,1-2bytes/char,8-rich string
  TBGLOBALHANDLE ghFontName;  
  BOOL fGetName;  
}xFONT, FAR *LPxFONT;

typedef struct tag_xformat
{
  WORD wIndex;     //Format index code (Excel internal use only)
//  BYTE bFormatLen;
  TBGLOBALHANDLE ghFormat;
}xFORMAT, FAR *LPxFORMAT;

typedef struct tag_isstinf
{
  DWORD ib; //stream position where strings begin (stream pointer into the SST record)
  WORD  cb; //offset into SST record that points to where the bucket begins
  WORD  res1;
}ISSTINF, FAR *LPISSTINF;
typedef struct tag_zextsst
{
  WORD wSSTinfCount;  //number of strings in each bucket
  TBGLOBALHANDLE ghSSTinf;
}zEXTSST, FAR *LPzEXTSST;

typedef struct tag_name
{
  WORD fHidden:1;
  WORD fFunc:1;
  WORD fOB:1;
  WORD fProc:1;
  WORD fCalcExp:1;
  WORD fBuiltin:1;
  WORD fgrp:6;
  WORD fBig:1;
  WORD res1:3;
  BYTE chKey;
  BYTE cch;
  WORD cce;
  WORD ixals;
  WORD itab;
  BYTE cchCustMenu;
  BYTE cchDescription;
  BYTE cchHelptopic;
  BYTE cchStatustext;
  TBGLOBALHANDLE ghName;
  TBGLOBALHANDLE ghNameDef;
  TBGLOBALHANDLE ghMenuText;
  TBGLOBALHANDLE ghDescription;
  TBGLOBALHANDLE ghHelptopic;
  TBGLOBALHANDLE ghStatusBarText;
}xNAME, FAR *LPxNAME;

typedef struct tag_externname
{
  WORD  wOptions;
  DWORD fBuiltin:1;
  DWORD fWantAdvise:1;
  DWORD fWantPict:1;
  DWORD fOle:1;
  DWORD fOleLink:1;
  DWORD res1:11;
  BYTE  cch;
  TBGLOBALHANDLE ghName;
  WORD  cce;
  TBGLOBALHANDLE ghDefinition;
}xEXTERNNAME, FAR *LPxEXTERNNAME;

typedef struct tag_zsst
{
  DWORD dwStrTotal; //total strings in EXSST & SST
  DWORD dwStrCount; //number of unique strings in the SST
  TBGLOBALHANDLE ghStrings;
}zSST, FAR *LPzSST;

typedef struct tag_style
{
  WORD wixfe;        //index to style XF
                     //only uses low-order 12 bits, 12 & 13 are unused,
                     //bit 15 : 1-builtin style, 0-userdefined style 
  union 
  {
    struct tag_builtin{
      BYTE bBuiltInStyle;//0-normal,1-rowlevel,2-collevel,3-comma,4-currency,5-percent,6-comma[0],7-currency[0] 
      BYTE bLevel;       //level of outline style rowlevel or collevel
    }builtin;
    struct tag_userdef{
      BYTE bStyleNameLen;
      BYTE xxx;          //the first byte of the style name.
                         //!!!RAP remember this when writing file.
    }userdef;
  }xstyle;

  TBGLOBALHANDLE ghStyleName;
}xSTYLE, FAR *LPxSTYLE;

typedef struct tag_workbook //WINDOW1
{
  WORD   x;
  WORD   y;
  WORD   cx;
  WORD   cy;
  WORD   fHidden:1;
  WORD   fIconic:1;
  WORD   res1:1;
  WORD   fDspHScroll:1;
  WORD   fDspVScroll:1;
  WORD   fBotAdornment:1;
  WORD   res2:2;
  WORD   res3:8;
  WORD   nSelTabIndex;
  WORD   nFirstTabIndex;     //0-based
  WORD   nNumberOfSelTabs;
  WORD   nWidthToHScrollRatio; //convert to decimal/1000
}WORKBOOK, FAR *LPWORKBOOK;  //WINDOW1

typedef struct tag_xf
{
  WORD wifnt;        //font index
  WORD wifmt;        //format index
  WORD fLocked:1;    //1-locked
  WORD fHidden:1;    //1-hidden
  WORD fStyle:1;     //0-cell XF, 1-style XF
  WORD f123Pfx:1;    //Transition navigation keys? Options dlg,Transition tab.
  WORD ixfParent:12; //Index to the XF of the parent style,usually ixfeNormal=0
  WORD fAlign:3;     //0-general,1-left,2-center,3-right,4-fill,5-justify,6-center selection
  WORD fWrap:1;      //wrap text in cell
  WORD fVAlign:3;    //0-top,1-center,2-bottom,3-justify
  WORD fJustLast:1;  //(Only for Far East versions of Excel)
  WORD fTextOrient:2;//0-none,1-top2bottom,2-270deg,3-90deg
  WORD fAtrNum:1;    //0-includes number,1-the ifmt != ifmt of parent style XF
  WORD fAtrFnt:1;    //0-includes font,1-the ifnt != ifnt of parent style XF
  WORD fAtrAlc:1;    //0-includes align,1-the fWrap or the fAlign != same field of parent style XF
  WORD fAtrBdr:1;    //0-includes border,1-if any border line != same field of parent style XF
  WORD fAtrPat:1;    //0-includes patterns,1-if any pattern != same field of parent style XF
  WORD fAtrProt:1;   //0-includes protection,1-if fLocked or fHidden != same field of parent style XF
  //*** NOTE!!! The documentation has the Back & Fore Colors reversed ***
  WORD fiBackColor:6;//index to color palette for backcolor of fill pattern
  WORD fiForeColor:7;//index to color palette for forecolor of fill pattern
  WORD fSxButton:1;  //0-style XF,1-if the XF is attached to a PivotTable buton
  WORD res1:2;
  WORD fls:6;        //Fill pattern
  WORD dgBottom:3;   //border line style
  WORD icvBottom:7;  //index to color palette for bottom border color
  WORD dgTop:3;      //border line style
  WORD dgLeft:3;     //border line style
  WORD dgRight:3;    //border line style
  WORD icvTop:7;     //index to color palette for top border color
  WORD icvLeft:7;    //index to color palette for left border color
  WORD icvRight:7;   //index to color palette for right border color
  WORD res2:2;
}xXF, FAR *LPxXF;

#if defined(_WIN64) || defined(_IA64)
#pragma pack(push, 1)
#endif
typedef struct tag_xf8
{
  WORD wifnt;        //font index
  WORD wifmt;        //format index
  WORD fLocked:1;    //1-locked
  WORD fHidden:1;    //1-hidden
  WORD fStyle:1;     //0-cell XF, 1-style XF
  WORD f123Pfx:1;    //Transition navigation keys? Options dlg,Transition tab.
  WORD ixfParent:12; //Index to the XF of the parent style,usually ixfeNormal=0
  WORD fAlign:3;     //0-general,1-left,2-center,3-right,4-fill,5-justify,6-center selection
  WORD fWrap:1;      //wrap text in cell
  WORD fVAlign:3;    //0-top,1-center,2-bottom,3-justify
  WORD fJustLast:1;  //(Only for Far East versions of Excel)
  WORD trot:8;       //BIFF8:rotation:0-90->up 0-90degs, 91-180->down 1-90degs, 255->vert
  WORD cIndent:4;    //BIFF8:indent value
  WORD fShrinkToFit:1; //BIFF8:1-thrink to fit Format Cells, dlg Alignment tab
  WORD fMergeCell:1; //BIFF8:1-merg cells Format Cells dlg, Alignment tab
  WORD iReadingOrder:2; //BIFF8:Far East only.
  WORD res1:2;
  WORD fAtrNum:1;    //0-includes number,1-the ifmt != ifmt of parent style XF
  WORD fAtrFnt:1;    //0-includes font,1-the ifnt != ifnt of parent style XF
  WORD fAtrAlc:1;    //0-includes align,1-the fWrap or the fAlign != same field of parent style XF
  WORD fAtrBdr:1;    //0-includes border,1-if any border line != same field of parent style XF
  WORD fAtrPat:1;    //0-includes patterns,1-if any pattern != same field of parent style XF
  WORD fAtrProt:1;   //0-includes protection,1-if fLocked or fHidden != same field of parent style XF
  WORD dgLeft:4;     //border line style
  WORD dgRight:4;    //border line style
  WORD dgTop:4;      //border line style
  WORD dgBottom:4;   //border line style
  WORD icvLeft:7;    //index to color palette for left border color
  WORD icvRight:7;   //index to color palette for right border color
  WORD grbitDiag:2;  //BIFF8:1-diag down,2-diag up,3-both
  DWORD icvTop:7;    //index to color palette for top border color
  DWORD icvBottom:7; //index to color palette for bottom border color
  DWORD icvDiag:7;   //for diag borders
  DWORD dgDiag:4;    //diag border line style
  DWORD res2:1;
  DWORD fls:6;       //fill pattern
  //*** NOTE!!! The documentation has the Back & Fore Colors reversed ***
  WORD fiBackColor:7;//index to color palette for backcolor of fill pattern
  WORD fiForeColor:7;//index to color palette for forecolor of fill pattern
  WORD fSxButton:1;  //0-style XF,1-if the XF is attached to a PivotTable buton
  WORD res3:1;
}xXF8, FAR *LPxXF8;
#if defined(_WIN64) || defined(_IA64)
#pragma pack(pop)
#endif

#ifdef SS_V70
typedef struct tag_externref
{
  TBGLOBALHANDLE fileName;
  TBGLOBALHANDLE startSheet;
  TBGLOBALHANDLE endSheet;
}EXTERNREF, FAR *LPEXTERNREF;

typedef struct tag_externsheet
{
  TBGLOBALHANDLE ghEXTERNREFs;
  WORD           dwEXTERNREFCount;
} EXTERNSHEET, FAR *LPEXTERNSHEET;
#endif

//=================================================================
//--- SubStream structures...

typedef struct tag_cellval
{
  WORD wCellType;
  WORD wRow;
  WORD wCol;
  WORD wixfe; //index to the XF rec
  TBGLOBALHANDLE ghCell;
}yCELLVAL, FAR *LPyCELLVAL;

typedef struct tag_cvformula
{
  double dblNum;       //first 2bytes == 0xFFFF if string, BOOL or error
  WORD   fAlwaysCalc:1;//always calculate the formula  
  WORD   fCalcOnLoad:1;//calculate the formula when the file is opened
  WORD   res1:1;
  WORD   fShrFmla:1;   //1-the formula is part of shared formula group
  WORD   unused1:4;
  WORD   unused2:8;
  DWORD  chn;          //ignored on read.write 0 always!
  WORD   wFormulaLen;  //length of parsed expression
  TBGLOBALHANDLE ghFormula;  
}yyFORMULA, FAR *LPyyFORMULA;
typedef struct tag_cvlabel
{
  WORD wRow;
  WORD wCol;
  WORD wixfe; //index to the XF rec
  WORD cch;
  TBGLOBALHANDLE ghString;
}yyLABEL, FAR *LPyyLABEL;
typedef struct tag_cvlabelsst
{
DWORD dwisst; //index into the SST 
}yyLABELSST, FAR *LPyyLABELSST;
typedef struct tag_cvnumber
{
  double dblNumber;
}yyNUMBER, FAR *LPyyNUMBER;
typedef struct tag_cvrk
{
  DWORD dwRK;
}yyRK, FAR *LPyyRK;

#if defined(_WIN64) || defined(_IA64)
#pragma pack(push, 1)
#endif
typedef struct tag_mulrk
{
  WORD  wixfe;
  DWORD rk;
}RKREC, FAR *LPRKREC;
#if defined(_WIN64) || defined(_IA64)
#pragma pack(pop)
#endif

typedef struct tag_mulcell
{
  WORD wCellType;
  WORD wRow;
  WORD wFirstCol;
  WORD wLastCol;
  TBGLOBALHANDLE ghCellData;//MULBLANK - array of indices to XF recs.
                          //MULRK - array of RKRECs.
}yMULCELLVAL, FAR *LPyMULCELLVAL;
typedef struct tag_cvboolerr
{
  BYTE bFlag;
  BYTE bIsError;
}yyBOOLERR, FAR *LPyyBOOLERR;

typedef struct tag_colinfo
{
  WORD wColFirst; //First formatted column (0-based)
  WORD wColLast;  //Last formatted column (0-based)
  WORD wdxCol;    //Col width in 1/256 of a char width
  WORD wixfe;     //index to XF record that contains the default format for the col
  WORD fHidden:1; //1-col hidden
  WORD res1:7;
  WORD iOutLevel:3; //outline level of column range
  WORD res2:1;
  WORD fCollapsed:1; //1-col range is collapsed in outlining
  WORD res3:3;
  WORD res4;       //the documentation says that this is a BYTE, but Excel
                   //writes it out as a WORD.
}yCOLINFO, FAR *LPyCOLINFO;

typedef struct tab_dbcells
{
  DWORD dwRowRecOffset;
  TBGLOBALHANDLE ghStreamOffsets;
}yDBCELL, FAR *LPyDBCELL;

typedef struct tag_dimensions
{
  WORD wRow1; //first defined row on the sheet
  WORD wRow2; //last defined row on the sheet + 1
  WORD wCol1; //first defined col on the sheet
  WORD wCol2; //last defined col on the sheet + 1
  WORD res1;
}yDIM, FAR *LPyDIM;

typedef struct tag_dimensions8
{
  DWORD dwRow1; //first defined row on the sheet
  DWORD dwRow2; //last defined row on the sheet + 1
  WORD wCol1; //first defined col on the sheet
  WORD wCol2; //last defined col on the sheet + 1
  WORD res1;
}yDIM8, FAR *LPyDIM8;

typedef struct tag_defrowheight
{
  WORD fUnsynced:1; //1-all undefined rows have incompatible font height & row height
  WORD fDyZero:1;   //1-all undefined rows have 0 height
  WORD fExAsc:1;    //1-all undefined rows have an extra space above
  WORD fExDsc:1;    //1-all undefined rows have an extra space below
  WORD res1:4;
  WORD res2:8;
  WORD wDefRowHeight; 
}yDEFROWHEIGHT, FAR *LPyDEFROWHEIGHT;

typedef struct tag_guts
{
  WORD dxRowGut;       //size of the row gutter to the left of th rows
  WORD dyColGut;       //size of the col gutter above the cols.
  WORD wLevelRowMax;   //max outline for row gutter
  WORD wLevelColMax;   //max outline for col gutter
}yGUTS, FAR *LPyGUTS;

typedef struct tag_hlink
{
  WORD rwFirst;  //first row of hyperlink
  WORD rwLast;   //last row of hyperlink
  WORD colFirst; //first col of hyperlink
  WORD colLast;  //last col of hyperlink
  TBGLOBALHANDLE ghData;
}yHLINK, FAR *LPyHLINK;

typedef struct tag_index
{
  DWORD res1;          //must be 0
  WORD  wFirstRow;     //first row on the sheet
  WORD  wLastRowP1;    //last row on the sheet+1
  DWORD res2;          //must be 0
}yINDEX, FAR *LPyINDEX;
typedef struct tag_yindex8
{
  DWORD res1;          //must be 0
  DWORD dwFirstRow;    //first row on the sheet
  DWORD dwLastRowP1;   //last row on the sheet+1
  DWORD res2;          //must be 0
}yINDEX8, FAR *LPyINDEX8;

typedef struct tag_pls
{
  WORD wEnv;  //0-MS Windows,1-Apple Macintosh
  TBGLOBALHANDLE ghPrintStruct;
}yPLS, FAR *LPyPLS;

typedef struct tag_row
{
  WORD wRow;  //row number
  WORD wCol1; //first defined col in the row
  WORD wCol2; //last defined col in the row
  WORD wyRow; //row height
  WORD wiRow; //Used by MS Excel to optimize loading;if BIFF,wiRow=0
  WORD res1;
  WORD wiOutLevel:3;//index to the ouline level of the row
  WORD res2:1;
  WORD fCollapsed:1;//1-the row is collapsed in outlining
  WORD fDyZero:1;   //1-the row height is set to 0
  WORD fUnsynced:1; //1-the font height & row height are not compatible
  WORD fGhostDirty:1;//1-if the row has been formatted, even if it is all blanks
  WORD res3:8;
  WORD wixfe; //if fGhostDirty=1,this is index to XF rec for the row.
              //NOTE:wixfe uses only the low-order 12 bits. 
              //Bit12 is fExAsc, bit13 is fExDsc. fExAsc&fExDsc are set
              //to TRUE if the row has extra space above or below.
}yROW, FAR *LPyROW;

typedef struct tag_ref
{
  WORD wRow1;
  WORD wRow2;
  BYTE bCol1;
  BYTE bCol2;
}yyREF, FAR *LPyyREF;

typedef struct tag_selection
{
  BYTE bPaneNum; //number of the pane
  WORD wRow;
  WORD wCol;
  WORD wiRefNum; //ref number of the active cell
  WORD wRefs;    //number of refs in the selection
  TBGLOBALHANDLE ghRefs;
}ySEL, FAR *LPySEL;

typedef struct tag_setup
{
  WORD wPaperSize;  //Paper size (see fNoPls)
  WORD wScale;      //Scaling factor (see fNoPls)
  WORD wPageStart;  //Starting page number
  WORD wFitWidth;   //Fit to width;number of pages
  WORD wFitHeight;  //Fit to height;number of pages
  WORD fLeftToRight:1; //Print over, then down
  WORD fLandscape:1;   //0-landscape,1-protrait (see fNoPls)
  WORD fNoPls:1;    //1-wPaperSize,wScale,wRes,wVRes,wCopies,fLandscape data have NOT
                    //been obtained from the printer, so they are NOT valid.
  WORD fNoColor:1;  //1-print black & white
  WORD fDraft:1;    //1-print draft quality
  WORD fNotes:1;    //1-print notes
  WORD fNoOrient:1; //1-orientation not set
  WORD fUsePage:1;  //use custom starting page number insted of auto
  WORD res1:8;
  WORD wRes;        //Print resolution (see fNoPls)
  WORD wVRes;       //Vertical print resolution (see fNoPls)
  double numHdr;    //Header margin (IEEE number)
  double numFtr;    //Footer margin (IEEE number)
  WORD wCopies;     //Number of copies (see fNoPls)
}ySETUP, FAR *LPySETUP;

typedef struct tag_sort
{
  WORD fCol:1;     //1-sort left-to-right
  WORD fKey1Dsc:1; //1-key 1 sorts in desc order
  WORD fKey2Dsc:1; //1-key 2 sorts in desc order
  WORD fKey3Dsc:1; //1-key 3 sorts in desc order
  WORD fCaseSensitive:1; //1-sort is case sensitive 
  WORD iOrder:5;   //index to table in First Key Sort Options. Normal sort order is iOrder=0
  WORD fAltMethod:1; //Only in Far East versions of MS Excel
  WORD res1:5;
  BYTE bKey1Len;   //Length of sort key 1 string
  BYTE bKey2Len;   //Length of sort key 2 string
  BYTE bKey3Len;   //Length of sort key 3 string
  TBGLOBALHANDLE ghKey1;
  TBGLOBALHANDLE ghKey2;
  TBGLOBALHANDLE ghKey3;
}ySORT, FAR *LPySORT;

typedef struct tag_worksheet //for WINDOW2
{
  WORD fDspFmla:1;       //1-display formulas,0-display values 
  WORD fDspGrid:1;       //1-display grids
  WORD fDspRwCol:1;      //1-display row & col headings
  WORD fFrozen:1;        //1-panes in the window should be frozen
  WORD fDspZeros:1;      //1-display 0 values,0-suppress 0 values
  WORD fDefaultHdr:1;    //1-use def colors,0-use rgbHdr color
  WORD fArabic:1;        //1-use Arabic MS Excel
  WORD fDspGuts:1;       //1-outline symbols are displayed
  WORD fFrozenNoSplit:1; //1-panes in the window are frozen but no split
  WORD fSelected:1;      //1-sheet tab is selected
  WORD fPaged:1;         //1-sheet is currently being displayed in the wkbk window
  WORD res1:5;
  WORD wRowTop;          //top row visible in the window
  WORD wColLeft;         //leftmost column visible in the window
  DWORD rgbHdr;          //row/col heading and gridline color
}yWORKSHEET, FAR *LPyWORKSHEET;  //WINDOW2

typedef struct tag_worksheet8 //for WINDOW2
{
  WORD fDspFmla:1;       //1-display formulas,0-display values 
  WORD fDspGrid:1;       //1-display grids
  WORD fDspRwCol:1;      //1-display row & col headings
  WORD fFrozen:1;        //1-panes in the window should be frozen
  WORD fDspZeros:1;      //1-display 0 values,0-suppress 0 values
  WORD fDefaultHdr:1;    //1-use def colors,0-use rgbHdr color
  WORD fArabic:1;        //1-use Arabic MS Excel
  WORD fDspGuts:1;       //1-outline symbols are displayed
  WORD fFrozenNoSplit:1; //1-panes in the window are frozen but no split
  WORD fSelected:1;      //1-sheet tab is selected
  WORD fPaged:1;         //1-sheet is currently being displayed in the wkbk window
  WORD fSLV:1;           //1-sheet was saved while in page break preview
  WORD res1:4;
  WORD wRowTop;          //top row visible in the window
  WORD wColLeft;         //leftmost column visible in the window
  DWORD dwiHdr;          //index to color value for row/col headings & gridlines
  WORD wScaleSLV;        //zoom magnification in page break preview  
  WORD wScaleNormal;     //zoom magnification in normal view
  DWORD res2;
}yWORKSHEET8, FAR *LPyWORKSHEET8;  //WINDOW2

typedef struct tag_wsbool
{
  DWORD fShowAutoBreaks:1; //1-auto page breaks are visible
  DWORD res1:3;
  DWORD fDialog:1;         //1-sheet is a dlg sheet
  DWORD fApplyStyles:1;    //0-automatic styles are applied to outline
  DWORD fRowSumsBelow:1;   //1-summary rows appear below detail in outline
  DWORD fColSumsRight:1;   //1-summary cols appear right of detail in outline
  DWORD fFitToPage:1;      //1-fit options is on (Page Setup dlg, Page tab)
  DWORD res2:1;
  DWORD fDspGuts:2;        //1-outline symbols displayed
  DWORD res3:2;
  DWORD fAee:1;            //1-Alternate Expression Evalueation option is on (Options dlg, Calculation tab)
  DWORD fAfe:1;            //1-Alternate Formula Entry option is on (Options dlg, Calculation tab)
}yWSBOOL, FAR *LPyWSBOOL;

typedef struct tag_textobject
{
  WORD res1:1;
  WORD alcH:3;
  WORD alcV:3;
  WORD res2:2;
  WORD fLockText:1;
  WORD res3:6;
  WORD rotation;
  BYTE res4[6];
  WORD cchText;
  WORD cbRuns;
  DWORD res5;
}yTXO, FAR *LPyTXO;

typedef struct tag_textobjectruns
{
  WORD ichFirst;
  WORD ifnt;
  DWORD res1;
}yyTXORuns, FAR *LPyyTXORuns;

typedef struct tag_ynote
{
  WORD rw;
  WORD col;
  WORD res1:1;
  WORD fShow:1;
  WORD res2:14;
  WORD idObj;
}yNOTE, FAR *LPyNOTE;

typedef struct tag_ymerge
{
  WORD rw1;
  WORD rw2;
  WORD col1;
  WORD col2;
}yMERGE, FAR *LPyMERGE;

typedef struct tag_ypane
{
  WORD x;
  WORD y;
  WORD rwTop;
  WORD colLeft;
  WORD pnnAct;
}yPANE, FAR *LPyPANE;

typedef struct tag_ftCmo
{
  WORD ft;
  WORD cb;
  WORD ot;
  WORD id;
  WORD fLocked:1;
  WORD res1:3;
  WORD fPrint:1;
  WORD res2:8;
  WORD fAutoFill:1;
  WORD fAutoLine:1;
  WORD res3:1;
  BYTE res4[12]; //must be 0 
}ftCmo;
typedef struct tag_ftGeneric
{
  WORD ft;
  WORD cb;
}ftGeneric;

typedef struct tag_substream
{
  yINDEX     index;
  yINDEX8    index8;

  yGUTS      guts;
  yDEFROWHEIGHT DefRowHeight;
  yWSBOOL    wsbool;
  yPLS       pls;
  ySETUP     setup;
  yWORKSHEET8 worksheet;
  yDIM       dim;
  ySEL       sel;

  WORD   wDefColWidth;   //def width of cols

  WORD   wMaxIterations; //iteration count
  WORD   wCalcMode;      //0-manual,1-auto,-1-auto,except tables
  WORD   wRefMode;       //1-A1,0-R1C1
  WORD   wIteration;     //1-iteration options is on
  double dblMaxDelta;    //max iteration change
  WORD   wSaveRecalc;    //1-recalc before saving
  WORD   wPrintHeaders;  //1-print row&col headers
  WORD   wPrintGrid;     //1-print gridlines
  WORD   wGridSet;       //1-the user changed the gridlines setting in Page Setup dlg,Sheet tab
  WORD   wHCenter;       //1-center sheet between horiz margins on print
  WORD   wVCenter;       //1-center sheet between vert margins on print
  BYTE   bProtect;       //1-sheet-level protection of locked cells

  BYTE   bHeaderLen;
  TBGLOBALHANDLE ghHeader;
  BYTE   bFooterLen;
  TBGLOBALHANDLE ghFooter;

  DWORD  dwSortCount;
  TBGLOBALHANDLE ghSorts;

  DWORD  dwRowCount;
  TBGLOBALHANDLE ghRows;

  DWORD dwColInfoCount;
  TBGLOBALHANDLE ghCols;

  DWORD  dwDBCellCount;
  TBGLOBALHANDLE ghDBCells;

  DWORD  dwCellCount;
  TBGLOBALHANDLE ghCells;

  DWORD  dwMulCellCount;
  TBGLOBALHANDLE ghMulCells;

  DWORD  dwTXOLen;
  TBGLOBALHANDLE ghTXOs;

  DWORD  dwOBJCount;
  TBGLOBALHANDLE ghOBJs;

}SUBSTREAM, FAR *LPSUBSTREAM;

//=================================================================

typedef struct tag_ss
{
  TBGLOBALHANDLE ghXLFileName;
  TBGLOBALHANDLE ghXLLogFileName;
  
  WORD wVer;          //BIFF Version
  WORD wCodePage;     //code page the file is saved:01B4h-IBM PC,8000h-Apple,04E4h-ANSI(MS Win)
  WORD wProtPass;     //File protected by encrypted password
  WORD wPassword;     //Encrypted password
  WORD wBackup;       //Excel should make backups  
  WORD wHideObj;      //Options dlg, View tab
  WORD w1904;         //Excel date system
  WORD wFnGroupCount; //Count of built-in function groups in Excel
  WORD wPrecision;    //Precision options from Options dlg, Calculation tab
  WORD wBookBool;     //workbook option flag
  WORD wRevLock;      //1-sharing with Track Changes option is on (Protect Shared Workbook dlg)
  WORD wRevPass;      //Encrypted Shared Workbook Protection Password
  WORD wRefreshAll;   //1-Refresh All should be done on all external data ranges and PivotTables 
                      //when loading the workbook
  WORD wDSF;          //1-the workbook is a Double Stream File.
  WORD wUsesElfs;     //1-written by a version of Excel that can use natural-language forumla input

  BYTE bProtect;      //Protection state for the workbook
  BYTE bWdwProtect;   //Protect workbook windows (Protect Workbook dialog box)

  zBOF     bof;       //BOF info
  zCOUNTRY country;   //country info
  zSST     sst;       //shared string table
  zEXTSST  extsst;    //extended string table

  TBGLOBALHANDLE ghCodeName; //VBE Object Name
  BYTE bCodeNameLen;

  TBGLOBALHANDLE ghWriteAccess; //User name
  BYTE bWriteAccessLen; //User name length (max. 31)

  TBGLOBALHANDLE ghFonts;
  DWORD dwFontCount;

  TBGLOBALHANDLE ghFormats;
  DWORD dwFormatCount;

  TBGLOBALHANDLE ghXFs;
  DWORD dwXFCount;

  TBGLOBALHANDLE ghStyles;
  DWORD dwStyleCount;

  TBGLOBALHANDLE ghBundleShts;
  DWORD dwBundleShtCount;

  BOOL           fNamesSet;
  TBGLOBALHANDLE ghNames;
  DWORD dwNameCount;

  TBGLOBALHANDLE ghExternNames;
  DWORD dwExternNameCount;

  WORKBOOK wb;

  // internal use...
  WORD  fInUISection:1;
  
  SUBSTREAM SubStream;
  DWORD dwSubStreamCount;

  yCELLVAL ShrFmlaCell;
  BOOL     fShrFmlaFlag;
  TBGLOBALHANDLE ghShrFmla;
  DWORD dwShrFmlaCount;

  yCELLVAL StringFmlaCell;
  BOOL     fStringFmlaFlag;

  yDEFROWHEIGHT DefRowHeight;

  TBGLOBALHANDLE ghProcessData;
  BYTE           bProcessDataType;

  TBGLOBALHANDLE ghSSTData;
  DWORD          dwSSTDataLen;
  int            xlSS_HEADER; // to take care of the different definition of SS_HEADER between the ActiveX & DLL
//  SS_FONTID      defaultFontId; //15103
//  LOGFONT        defaultLogFont;
  TBGLOBALHANDLE ghDefaultLogFont;
#ifdef SS_V70
  TBGLOBALHANDLE ghSUPBOOKData;
  DWORD          dwSUPBOOKCount;

  TBGLOBALHANDLE ghEXTERNSHEETs;
  DWORD          dwEXTERNSHEETCount;
#endif
  TBGLOBALHANDLE ghRowInfos;     //19989
  DWORD          dwRowInfoCount; //19989
}SSXL, FAR *LPSSXL;


typedef struct tag_f
{
#ifdef SS_V40
  // common properties for numberfmt, currencyfmt & percentfmt.
  double Min;
  double Max;
  BYTE   Right;
	BYTE   fNegRed;
  TCHAR  szDecimal[3 + 1];
  BYTE   bNegStyle;            // 0 - 8
  // common properties for numberfmt & currency fmt
  BYTE   fShowSeparator;       // T/F
  TCHAR  szSeparator[3 + 1];
  BYTE   bLeadingZero;         // 0 - 2
 
/*
  struct tag_numberfmt
  {
    // all contained within the generic stuff above...
  }numberfmt;
*/
  struct tag_currencyfmt
	{
	  TCHAR  szCurrency[5 + 1];
	  BYTE   bPosStyle;            // 0 - 5
	  BYTE   fShowCurrencySymbol;  // T/F
	} currencyfmt;

/*  struct tag_percentfmt
	{
    // all contained within the generic stuff above...
	} percentfmt;
*/
#else
  struct tag_floatfmt
  {
    BYTE    fCurrency:1;
    BYTE    fSeparator:1;
    BYTE    fDigitLimit:1;
    BYTE    res1:5;
    int     nDigitsLeft;
    int     nDigitsRight;
    double  dblMax;
    double  dblMin;
    TCHAR   szCurrency[50];
  }floatfmt;
#endif
  struct tag_datefmt
  {
    BOOL    bCentury;
    TCHAR   cSeparator;
    int     nFormat;
    BOOL    bSpin;
  }datefmt;
  struct tag_timefmt
  {
    BOOL   b24Hour;
    BOOL   bSeconds;
    TCHAR  cSeparator;
    BOOL   bSpin;
  }timefmt;

}FMT, FAR *LPFMT;

//XF: Extended Format internally saved formats
//Index to internal format (ifmt)

#define XF00h _T("General")
#define XF01h _T("0")
#define XF02h _T("0.00")
#define XF03h _T("#,##0")
#define XF04h _T("#,##0.00")
#define XF05h _T("($#,##0_);($#,##0)")
#define XF06h _T("($#,##0_);[Red]($#,##0)")
#define XF07h _T("($#,##0.00_);($#,##0.00)")
#define XF08h _T("($#,##0.00_);[Red]($#,##0.00)")
#define XF09h _T("0%")
#define XF0ah _T("0.00%")
#define XF0bh _T("0.00E+00")
#define XF0ch _T("# ?/?")
#define XF0dh _T("# ??/??")
#define XF0eh _T("m/d/yyyy")
#define XF0fh _T("d-mmm-yyyy")
#define XF10h _T("d-mmm")
#define XF11h _T("mmm-yyyy")
#define XF12h _T("h:mm AM/PM")
#define XF13h _T("h:mm:ss AM/PM")
#define XF14h _T("h:mm")
#define XF15h _T("h:mm:ss")
#define XF16h _T("m/d/yyyy h:mm")
//#ifdef SPREAD_JPN //far east version of Excel 
#define XF19h _T("$#,##0.00_);($#,##0.00)")
#define XF1ah _T("$#,##0.00_);[Red]($#,##0.00)")
#define XF1eh _T("m/d/y")
#define XF1fh _T("yyyy\"”N\"m\"ŒŽ\"d\"“ú\"")
#define XF20h _T("h\"Žž\"mm\"•ª\"")
#define XF21h _T("h\"Žž\"mm\"•ª\"ss\"•b\"")
//#endif //SPREAD_JPN
#define XF25h _T("(#,##0_);(#,##0)")
#define XF26h _T("(#,##0_);[Red](#,##0)")
#define XF27h _T("(#,##0.00_);(#,##0.00)")
#define XF28h _T("(#,##0.00_);[Red](#,##0.00)")
#define XF29h _T("_(* #,##0_);_(* (#,##0);_(* "-"_);_(@_)")
#define XF2ah _T("_($* #,##0_);_($* (#,##0);_($* "-"_);_(@_)")
#define XF2bh _T("_(* #,##0.00_);_(* (#,##0.00);_(* "-"??_);_(@_)")
#define XF2ch _T("_($* #,##0.00_);_($* (#,##0.00);_($* "-"??_);_(@_)")
#define XF2dh _T("mm:ss")
#define XF2eh _T("[h]:mm:ss")
#define XF2fh _T("mm:ss.0")
#define XF30h _T("##0.0E+0")
#define XF31h _T("@")

//#ifdef SPREAD_JPN //far east version of Excel
#define XF37h _T("yyyy\"”N\"m\"ŒŽ\"")
#define XF38h _T("m\"ŒŽ\"d\"“ú\"")
#define XF39h _T("ge.m.d")
#define XF3ah _T("ggge\"”N\"m\"ŒŽ\"d\"“ú\"")
//#endif

#define FLOATTYPE    1
#define NUMBERTYPE   2
#define DATETIMETYPE 3
#define DATETYPE     4
#define TIMETYPE     5
#define MASKTYPE     6
#define STRINGTYPE   7
#define UNDEFTYPE    10
#define INTEGERTYPE  11
#define PERCENTTYPE  12
#define CURRENCYTYPE 13
#define SCIENTIFICTYPE 14

//Parsed Thing (ptg) defines 
#ifdef SS_V70

typedef struct tag_cellref3d
{
  BYTE bToken;
  WORD externSheetIndex;
  WORD rw;       //the row number or offset (zero-based)
  WORD col:14;   //the col number or offset...
  WORD fColRel:1;//1-col offset is relative:A1, 0-absolute:$A$1
  WORD fRwRel:1; //1-row offset is relative:A1, 0-absolute:$A$1
}CELLREF3D, FAR *LPCELLREF3D;

typedef struct tag_xti
{
  WORD supBookIndex;
  WORD firstSheet;
  WORD lastSheet;
}XTI, FAR *LPXTI;
#endif
typedef struct tag_cellref8
{
  BYTE bToken;
  WORD rw;       //the row number or offset (zero-based)
  WORD col:14;   //the col number or offset...
  WORD fColRel:1;//1-col offset is relative:A1, 0-absolute:$A$1
  WORD fRwRel:1; //1-row offset is relative:A1, 0-absolute:$A$1
}CELLREF8, FAR *LPCELLREF8;
typedef struct tag_cellref7
{
  BYTE bToken;
  WORD rw:14;    //the row number or offset (zero-based)
  WORD fColRel:1;//1-col offset is relative:A1, 0-absolute:A$1
  WORD fRwRel:1; //1-row offset is relative:A1, 0-absolute:$A1
  BYTE col;      //the col number or offset...
}CELLREF7, FAR *LPCELLREF7;

typedef struct tag_funcvar
{
  BYTE bToken;
  BYTE cargs:7;  //number of args for the function
  BYTE fPrompt:1;//1-function prompts the user
  WORD iftab:15; //index to the function table
  WORD fCE:1;    //the function is a command-equivalent 
}FUNCVAR, FAR *LPFUNCVAR;

typedef struct tag_attr
{
  BYTE bToken;
  BYTE bitFAttrSemi:1; //contains a volatile function
  BYTE bitFAttrIf:1;   //optimized IF function
  BYTE bitFAttrChoose:1;//optimized CHOOSE function
  BYTE bitFAttrGoto:1; //jump to another location
  BYTE bitFAttrSum:1;  //optimized SUM function
  BYTE bitFAttrBaxcel:1;//BASIC-style assignment statement
  BYTE bitFAttrSpace:1;//spaces after the equal sign(BIFF3&BIFF4 only)
  BYTE res1:1;
  WORD wData;
}ATTR, FAR *LPATTR;

typedef struct tag_area
{
  BYTE bToken;
  WORD rwFirst;
  WORD rwLast;
  WORD colFirst:14;  
  WORD fColRelFirst:1;
  WORD fRwRelFirst:1;
  WORD colLast:14;  
  WORD fColRelLast:1;
  WORD fRwRelLast:1;
}AREA, FAR *LPAREA;

typedef struct tag_area3d
{
  BYTE bToken;
  WORD ixti;
  WORD rwFirst;
  WORD rwLast;
  WORD colFirst:8;
  WORD res1:6;
  WORD fColRelFirst:1;
  WORD fRwRelFirst:1;
  WORD colLast:8;
  WORD res2:6;
  WORD fColRelLast:1;
  WORD fRwRelLast:1;
}AREA3D, FAR *LPAREA3D;

typedef struct tag_ref3d
{
  BYTE bToken;
  WORD ixti;
  WORD rw;
  WORD col:8;
  WORD res1:6;
  WORD fColRel:1;
  WORD fRwRel:1;
}REF3D, FAR *LPREF3D;

typedef struct tag_named
{
  BYTE bToken;
//  WORD ixti;
  WORD ilbl;
  WORD res1;
}NAMED, FAR *LPNAMED;

typedef struct tag_namedx
{
  BYTE bToken;
  WORD ixti;
  WORD ilbl;
  WORD res1;
}NAMEDX, FAR *LPNAMEDX;

typedef struct tag_exp
{
  BYTE bToken;
  WORD rwFirst;
  WORD colFirst;
}EXP, FAR *LPEXP;

//========================================================
//Save structures
typedef struct tag_label
{
  WORD rw;
  WORD col;
  WORD ixfe;
}LABEL, FAR *LPLABEL;

//#pragma pack(1)
typedef struct tag_labelsst
{
  WORD wRow;
  WORD wCol;
  WORD wixfe;
  DWORD isst;
}LABELSST, FAR *LPLABELSST;
//#pragma pack (8)

typedef struct tag_bistyle
{
  WORD ixfe;
  BYTE istyBuiltIn;
  BYTE iLevel;
}BISTYLE, FAR *LPBISTYLE;
typedef struct tag_number
{
  WORD wRow;
  WORD wCol;
  WORD wixfe;
  double num;
}NUMBER, FAR *LPNUMBER;

typedef struct tag_blank
{
  WORD wRow;
  WORD wCol;
  WORD wixfe;
}BLANK, FAR *LPBLANK;

typedef struct tag_formula
{
  WORD   wRow;
  WORD   wCol;
  WORD   wixfe;
  double dblNum;       //last 2bytes == 0xFFFF if BOOL or error
  WORD   fAlwaysCalc:1;//always calculate the formula  
  WORD   fCalcOnLoad:1;//calculate the formula when the file is opened
  WORD   res1:1;
  WORD   fShrFmla:1;   //1-the formula is part of shared formula group
  WORD   unused1:4;
  WORD   unused2:8;
  DWORD  chn;          //ignored on read.write 0 always!
  WORD   wFormulaLen;  //length of parsed expression
}FORMULA, FAR *LPFORMULA;

typedef struct tag_shrfmla
{
  WORD rwFirst;
  WORD rwLast;
  BYTE colFirst;
  BYTE colLast;
  WORD res1;
  WORD cce;
  TBGLOBALHANDLE ghFmla;
}yySHRFMLA, FAR *LPyySHRFMLA;

typedef struct tag_array
{
  WORD rwFirst;
  WORD rwLast;
  BYTE colFirst;
  BYTE colLast;
  WORD grbit;
  DWORD chn; //ignore
  WORD len;
  TBGLOBALHANDLE ghFmla;
}yyARRAY, FAR *LPyyARRAY;

typedef struct tag_sst
{
  DWORD dwTotal;
  DWORD dwUnique;
}SST, FAR *LPSST;

typedef struct tag_extsst
{
  WORD wStringsPerBucket;
}EXTSST, FAR *LPEXTSST;

typedef struct tag_index8
{
  DWORD res1;
  DWORD dwFirstRow;    //first row on the sheet
  DWORD dwLastRowP1;   //last row on the sheet+1
  DWORD res2;
}INDEX8, FAR *LPINDEX8;

typedef struct tag_format
{
  WORD wifmt;
}FORMAT, FAR *LPFORMAT;

//========================================================
//Internal structures
typedef struct tag_RwCo
{
  BYTE col;
  WORD row;
  BOOL fRwRel;
  BOOL fColRel;
} RWCO, FAR *LPRWCO;

typedef struct tag_Break
{
  WORD index;
  WORD startingIndex;
  WORD endingIndex;
}PAGEBREAK, FAR *LPPAGEBREAK;

typedef struct tag_ApplyCelltype
{
  LPSHORT lpnFormatCount;
  LPSHORT lpnXFIndex;
  LPSHORT lpnXFCount;
  LPTBGLOBALHANDLE lpghFormat;
  LPLONG lplFormatLen;
  LPTBGLOBALHANDLE lpghXF;
  LPLONG lplXFLen;
  LPTBGLOBALHANDLE lpghFont;
  LPLONG lplFontLen;
  LPTBGLOBALHANDLE lpghCells;
  LPLONG lplCellsLen;
  LPTBGLOBALHANDLE lpghSST;
  LPLONG lplSSTLen;
  LPTBGLOBALHANDLE lpghEXTSST;
  LPLONG lplEXTSSTLen;
}APPLYCELLTYPE, FAR *LPAPPLYCELLTYPE;
//========================================================

// TA Hydronics - to export bitmap.
typedef struct tag_FtCmoStructure
{
	WORD ft;
	WORD cb;
	WORD ot;
	WORD id;
	WORD fALocked : 1;
	WORD fBReserved : 1;
	WORD fCDefaultSize : 1;
	WORD fDPublished : 1;
	WORD fEPrint : 1;
	WORD fFUnused : 1;
	WORD fGUnused : 1;
	WORD fHDisabled : 1;
	WORD fIUIObj : 1;
	WORD fJRecalcObj : 1;
	WORD fKUnused : 1;
	WORD fLUnused : 1;
	WORD fMRecalcObjAlways : 1;
	WORD fNUnused : 1;
	WORD fOUnused : 1;
	WORD fPUnused : 1;
	BYTE arbUnused[12];
}FTCMOSTRUCTURE, FAR* LPFTCMOSTRUCTURE;

typedef struct tag_FtCfStructure
{
	WORD ft;
	WORD cb;
	WORD cf;
}FTCFSTRUCTURE, FAR* LPFTCFSTRUCTURE;

typedef struct tag_FtPioGrbitStructure
{
	WORD ft;
	WORD cb;
	WORD fAAutoPict : 1;
	WORD fBDDe : 1;
	WORD fCPrintCalc : 1;
	WORD fDIcon : 1;
	WORD fECtl : 1;
	WORD fFPrstm : 1;
	WORD fGUnused : 1;
	WORD fHCamera : 1;
	WORD fIDefaultSize : 1;
	WORD fJAutoLoad : 1;
	WORD fUnused : 6;
}FTPIOGRBITSTRUCTURE, FAR* LPFTPIOGRBITSTRUCTURE;

typedef struct tag_FtPictFmlaStructure
{
	WORD ft;
	WORD cb;

}FTPICTFMLASTRUCTURE, FAR* LPFTPICTFMLASTRUCTURE;

typedef struct tag_ObjectRecord
{
	FTCMOSTRUCTURE		FtCmo;
	FTCFSTRUCTURE		FtCf;
	FTPIOGRBITSTRUCTURE	FtPioBrbit;
	FTPICTFMLASTRUCTURE	FtPictFmlaStructure;
	BYTE				arbReserved[4];
}OBJECTRECORD, FAR*LPOBJECTRECORD;

//ProcessDataTypes
#define PDT_PROCESSINGSST    1
#define PDT_PROCESSINGDBCELL 2
#define PDT_PROCESSINGTXO1   3
#define PDT_PROCESSINGTXO2   4



// Date Stuff
#define ISLEAP(y) (y%4 == 0 && y%100 != 0 || y%400 == 0)

#ifndef RC_INVOKED
#pragma pack()
#endif   // ifndef RC_INVOKED
