/************
* SS_SAVE.H
*
************/

#ifndef SS_SAVE_H
#define SS_SAVE_H

#ifdef __cplusplus
extern "C" {
#endif

//??????????????????????????????????????
#if defined(WIN32) || defined(_WIN32)
  //#define SS_EARLY_32BIT_BETA
#endif

#if !defined(SS_EARLY_32BIT_BETA)
#ifndef RC_INVOKED
#if defined(_WIN64) || defined(_IA64)
#pragma pack(8)
#else
#pragma pack(1)
#endif
#endif   // ifndef RC_INVOKED
#endif
//??????????????????????????????????????

#define SS_FILETYPE_DATA    0
#define SS_FILETYPE_ALL     1
#define SS_FILETYPE_UNICODE 2

#define SS_FILESAVESIZE  20000
//xxrap#define SS_FILEREADSIZE  2048
#define SS_FILEREADSIZE  4196

#define SS_SAVEPALETTETYPE_LOADDEF 1
#define SS_SAVEPALETTETYPE_USEDEF  2
#define SS_SAVEPALETTETYPE_LOAD    3

#define INTPT_TO_PIXELS(pt) MulDiv((int)pt, dyPixelsPerInch, 72)
#define PIXELS_TO_INTPT(Pixels) MulDiv(Pixels, 72, dyPixelsPerInch)

typedef short SS_COORD16;

//???? used to toggle between early beta and final release load routines
#if defined(SS_EARLY_32BIT_BETA)
  #define SS_SAVEBOOL     BOOL
  #define SS_SAVEINT      int
  #define SS_SAVECHAR     TCHAR
  #define SS_SAVEHGLOBAL  HGLOBAL
  #define SS_SAVETBHANDLE TBGLOBALHANDLE
#else
  #define SS_SAVEBOOL     short
  #define SS_SAVEINT      short
  #define SS_SAVECHAR     char
#if defined(_WIN64) || defined(_IA64)
  #define SS_SAVEHGLOBAL  LONG_PTR
  #define SS_SAVETBHANDLE LONG_PTR
#else
  #define SS_SAVEHGLOBAL  short
  #define SS_SAVETBHANDLE long
#endif
#endif
//????

typedef struct ss_savedateformat
{
  SS_SAVEBOOL bCentury;
  char        cSeparator;
  SS_SAVEINT  nFormat;
  SS_SAVEBOOL bSpin;
} SS_SAVEDATEFORMAT, FAR* LPSS_SAVEDATEFORMAT;

typedef struct ss_savetimeformat
{
  SS_SAVEBOOL b24Hour;
  SS_SAVEBOOL bSeconds;
  char        cSeparator;
  SS_SAVEBOOL bSpin;
} SS_SAVETIMEFORMAT, FAR *LPSS_SAVETIMEFORMAT;


typedef struct ss_savefloatformat
   {
   SS_SAVECHAR  cCurrencySign;
   SS_SAVECHAR  cDecimalSign;
   SS_SAVECHAR  cSeparator;
   } SS_SAVEFLOATFORMAT, FAR *LPSS_SAVEFLOATFORMAT;

typedef struct ss_savevirtual
   {
   long     lVStyle;
   SS_COORD VMax;
   SS_COORD VTop;
   SS_COORD VSize;
   SS_COORD VOverlap;
   SS_COORD VPhysSize;                 // Physical Size
   } SS_SAVEVIRTUAL, FAR *LPSS_SAVEVIRTUAL;

typedef struct ss_savecoloritem
   {
   SS_SAVE_COLORID BackgroundId;
   SS_SAVE_COLORID ForegroundId;
   } SS_SAVECOLORITEM, FAR *LPSS_SAVECOLORITEM;

typedef struct ss_saveBorder
   {
   BYTE            bType;
   BYTE            bStyle;
   SS_SAVE_COLORID ColorId;
   } SS_SAVEBORDER, FAR *LPSS_SAVEBORDER;

typedef struct tagSS_SAVEDATE
   {
   SS_SAVEINT  nDay;
   SS_SAVEINT  nMonth;
   SS_SAVEINT  nYear;
   } SS_SAVEDATE, FAR *LPSS_SAVEDATE;

typedef struct tagSS_SAVETIME
   {
   SS_SAVEINT  nHour;
   SS_SAVEINT  nMinute;
   SS_SAVEINT  nSecond;
   } SS_SAVETIME, FAR *LPSS_SAVETIME;

typedef struct ss_savesuperbtncolor
   {
   COLORREF Color;
   COLORREF ColorBorder;
   COLORREF ColorShadow;
   COLORREF ColorHighlight;
   COLORREF ColorText;
   } SS_SAVESUPERBTNCOLOR, FAR *LPSS_SAVESUPERBTNCOLOR;

#if defined(SS_EARLY_32BIT_BETA)
#pragma pack()
#endif
typedef struct ss_savelogfont
{
    SS_SAVEINT lfHeight;
    SS_SAVEINT lfWidth;
    SS_SAVEINT lfEscapement;
    SS_SAVEINT lfOrientation;
    SS_SAVEINT lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    SS_SAVECHAR lfFaceName[32];
} SS_SAVELOGFONT;

typedef struct ss_savebitmap
   {
   BYTE bPictureType;                  // 0 - Bitmap
                                       // 1 - DIB
   WORD wWidth;
   WORD wHeight;
   BYTE bPlanes;
   BYTE bBitsPerPixel;
   long lBytes;
   } SS_SAVEBITMAP, FAR *LPSS_SAVEBITMAP;

typedef struct ss_savecell
   {
   SS_SAVEBOOL  fCellTypeProvided;
   SS_SAVEBOOL  fFormulaProvided;
   SS_SAVEBOOL  fDataProvided;
   SS_SAVEBOOL  CellLocked;
   SS_FONTID    FontId;
   SS_SAVECOLORITEM Color;
   char         NotUsed[4];
   } SS_SAVECELL, FAR *LPSS_SAVECELL;

typedef struct ss_savecell2
   {
   SS_SAVEBOOL  fCellTypeProvided;
   SS_SAVEBOOL  fFormulaProvided;
   SS_SAVEBOOL  fDataProvided;
   SS_SAVEBOOL  CellLocked;
   SS_FONTID    FontId;
   SS_SAVECOLORITEM Color;

   SS_SAVEBORDER    Border;
   char         NotUsed[4];
   } SS_SAVECELL2, FAR *LPSS_SAVECELL2;

typedef struct ss_saverow
   {
   SS_SAVEBOOL  fCellTypeProvided;
   SS_SAVEBOOL  fFormulaProvided;
   SS_SAVEBOOL  fDataProvided;
   SS_SAVEBOOL  RowLocked;
   double       RowHeight;
   SS_COORD16   ColLastNonEmpty;
   SS_FONTID    RowMaxFontId;
   SS_FONTID    FontId;
   SS_SAVECOLORITEM Color;
   long         Offset;
   char         NotUsed[10];
   } SS_SAVEROW, FAR *LPSS_SAVEROW;

typedef struct ss_saverow2
   {
   SS_SAVEBOOL  fCellTypeProvided;
   SS_SAVEBOOL  fFormulaProvided;
   SS_SAVEBOOL  fDataProvided;
   SS_SAVEBOOL  RowLocked;
   double       RowHeight;
   SS_COORD     ColLastNonEmpty;
   SS_FONTID    RowMaxFontId;
   SS_FONTID    FontId;
   SS_SAVECOLORITEM Color;
   long         Offset;

   SS_SAVEBORDER    Border;
   char         fRowSelected;
   char         fPageBreak;
   BYTE         bUserResize;
   long         lUserData;
   char         NotUsed[10];
   } SS_SAVEROW2, FAR *LPSS_SAVEROW2;

typedef struct ss_savecol
   {
   SS_SAVEBOOL  fCellTypeProvided;
   SS_SAVEBOOL  fFormulaProvided;
   SS_SAVEBOOL  fDataProvided;
   SS_SAVEBOOL  ColLocked;
   double       ColWidth;
   SS_COORD16   RowLastNonEmpty;
   SS_FONTID    FontId;
   SS_SAVECOLORITEM Color;
   long         Offset;
   char         NotUsed[10];
   } SS_SAVECOL, FAR *LPSS_SAVECOL;

typedef struct ss_savecol2
   {
   SS_SAVEBOOL  fCellTypeProvided;
   SS_SAVEBOOL  fFormulaProvided;
   SS_SAVEBOOL  fDataProvided;
   SS_SAVEBOOL  ColLocked;
   double       ColWidth;
   SS_COORD16   RowLastNonEmpty;
   SS_FONTID    FontId;
   SS_SAVECOLORITEM Color;
   long         Offset;

   SS_SAVEBORDER    Border;
   BYTE         fPageBreak;
   BYTE         bUserResize;
   long         lUserData;
   char         NotUsed[10];
   } SS_SAVECOL2, FAR *LPSS_SAVECOL2;

typedef struct ss_savefonttbl
   {
   HFONT       hFont;
   SS_SAVEBOOL fNoDelete;
   } SS_SAVEFONTTBL, FAR *LPSS_SAVEFONTTBL;

typedef struct ss_celltype1
   {
   char  Type;
   short ControlID;
   long  Style;
   union
      {
      struct
         {
         short ChrSet;
         short Len;
         char  ChrCase;
         } Edit;
      struct
         {
         long  Min;
         long  Max;
         } Integer;
      struct
         {
         short  Left;
         short  Right;
         double Min;
         double Max;
         } Float;
      struct
         {
         short          dItemCnt;
         TBGLOBALHANDLE hItems;
         } ComboBox;
      struct
         {
         SS_SAVEBOOL       SetFormat;
         SS_SAVEBOOL       SetRange;
         SS_SAVETIMEFORMAT Format;
         SS_SAVETIME       Min;
         SS_SAVETIME       Max;
         } Time;
      struct
         {
         SS_SAVEBOOL       SetFormat;
         SS_SAVEBOOL       SetRange;
         SS_SAVEDATEFORMAT Format;
         SS_SAVEDATE       Min;
         SS_SAVEDATE       Max;
         } Date;
      struct
         {
         TBGLOBALHANDLE hMask;
         } Pic;
      struct
         {
         TBGLOBALHANDLE hClassName;
         } Custom;
      struct
         {
         TBGLOBALHANDLE hPictName;
         } ViewPict;
      struct
         {
         GLOBALHANDLE  hText;
         GLOBALHANDLE  hPictName;
         GLOBALHANDLE  hPictDownName;
         short         nPictureType;
         short         nPictureDownType;
         char          ButtonType;
         char          ShadowSize;
         SS_SAVEBOOL   fSetColor;
         SS_SAVESUPERBTNCOLOR Color;
         } Button;

      char Filler[33];
      } Spec;
   } SS_CELLTYPE1, FAR *LPSS_CELLTYPE1;

typedef struct ss_savecelltype3
   {
   char  Type;
   short ControlID;
   long  Style;
   union
      {
      struct
   {
   short  ChrSet;
   short  Len;
   char   ChrCase;
   double dfWidth;
   short  nRowCnt;
   } Edit;
      struct
   {
   long  Min;
   long  Max;
   SS_SAVEBOOL  fSpinWrap;
   long  lSpinInc;
   } Integer;
      struct
   {
   short       Left;
   short       Right;
   double      Min;
   double      Max;
   SS_SAVEBOOL fSetFormat;
   SS_SAVEFLOATFORMAT Format;
   } Float;
      struct
   {
   short          dItemCnt;
   SS_SAVETBHANDLE hItems;
   } ComboBox;
      struct
   {
   SS_SAVEBOOL       SetFormat;
   SS_SAVEBOOL       SetRange;
   SS_SAVETIMEFORMAT Format;
   SS_SAVETIME       Min;
   SS_SAVETIME       Max;
   } Time;
      struct
   {
   SS_SAVEBOOL       SetFormat;
   SS_SAVEBOOL       SetRange;
   SS_SAVEDATEFORMAT Format;
   SS_SAVEDATE       Min;
   SS_SAVEDATE       Max;
   } Date;
      struct
   {
   SS_SAVETBHANDLE hMask;
   } Pic;
      struct
   {
   SS_SAVETBHANDLE hPictName;
   HPALETTE       hPal;
#if defined(SS_VB) || (defined(SS_OCX) && !defined(SS_EARLY_32BIT_BETA))
   short          hPic;
#endif
   SS_SAVEBOOL           fDeleteHandle;
   } ViewPict;
      struct
   {
   SS_SAVEHGLOBAL hText;
   SS_SAVEHGLOBAL hPictName;
   SS_SAVEHGLOBAL hPictDownName;
   short         nPictureType;
   short         nPictureDownType;
   char          ButtonType;
   char          ShadowSize;
   char          fDelHandle;
   char          fDelDownHandle;
   SS_SAVESUPERBTNCOLOR Color;
#if defined(SS_VB) || (defined(SS_OCX) && !defined(SS_EARLY_32BIT_BETA))
   short          hPic;
   short          hPicDown;
#endif
   } Button;
      struct
   {
   SS_SAVEHGLOBAL hText;
   SS_SAVETBHANDLE hPictUpName;
   SS_SAVETBHANDLE hPictDownName;
   SS_SAVETBHANDLE hPictFocusUpName;
   SS_SAVETBHANDLE hPictFocusDownName;
   SS_SAVETBHANDLE hPictGrayName;
   SS_SAVETBHANDLE hPictFocusGrayName;
   BYTE           bPictUpType;
   BYTE           bPictDownType;
   BYTE           bPictFocusUpType;
   BYTE           bPictFocusDownType;
   BYTE           bPictGrayType;
   BYTE           bPictFocusGrayType;
   char           fDelUpHandle;
   char           fDelDownHandle;
   char           fDelFocusUpHandle;
   char           fDelFocusDownHandle;
   char           fDelGrayHandle;
   char           fDelFocusGrayHandle;
#if defined(SS_VB) || (defined(SS_OCX) && !defined(SS_EARLY_32BIT_BETA))
   short          hPicUp;
   short          hPicDown;
   short          hPicFocusUp;
   short          hPicFocusDown;
   short          hPicGray;
   short          hPicFocusGray;
#endif
   } CheckBox;

      char Dummy[34];
      /*
      char Dummy[45];
      */
      } Spec;
   } SS_SAVECELLTYPE3, FAR *LPSS_SAVECELLTYPE3;

typedef struct ss_savespread
   {
   unsigned        fDataProvided     : 1;
   unsigned        DocumentProtected : 1;
   unsigned        DocumentLocked    : 1;
   unsigned        AllowResize       : 1;
   unsigned        AllowSelectBlock  : 1;
   unsigned        RestrictRows      : 1;
   unsigned        RestrictCols      : 1;
   unsigned        CalcAuto          : 1;
   unsigned        Redraw            : 1;
   unsigned        GridLines         : 1;
   unsigned        HorzScrollBar     : 1;
   unsigned        VertScrollBar     : 1;
   SS_SAVECOLOR    Color;
   SS_FONTID       DefaultFontId;
   SS_CELLTYPE1    DefaultCellType;
   SS_COORD16      HeaderRows;
   SS_COORD16      HeaderCols;
   SS_COORD16      RowsFrozen;
   SS_COORD16      ColsFrozen;
   SS_COORD16      RowsMax;
   SS_COORD16      ColsMax;
   SS_COORD16      RowCnt;
   SS_COORD16      ColCnt;
   SS_COORD16      DataRowCnt;
   SS_COORD16      DataColCnt;
   double          ColWidth;
   double          RowHeight;
   short           FontTableCnt;
   short           ColorTableCnt;
   SS_SAVEDATEFORMAT DefaultDateFormat;
   SS_SAVETIMEFORMAT DefaultTimeFormat;
   char            fUseFontPointSize;
   char            NotUsed[49];
   } SS_SAVESPREAD, FAR *LPSS_SAVESPREAD;

typedef struct savespread2
   {
   unsigned        fDataProvided           : 1;
   unsigned        DocumentProtected       : 1;
   unsigned        DocumentLocked          : 1;
   unsigned        AllowResize             : 1;
   unsigned        AllowSelectBlock        : 1;
   unsigned        RestrictRows            : 1;
   unsigned        RestrictCols            : 1;
   unsigned        CalcAuto                : 1;
   unsigned        Redraw                  : 1;
   unsigned        GridLines               : 1;
   unsigned        HorzScrollBar           : 1;
   unsigned        VertScrollBar           : 1;
   unsigned        AllowUserFormulas       : 1;
   unsigned        fEditModePermanent      : 1;
   unsigned        fAutoSize               : 1;
   unsigned        fScrollBarExtMode       : 1;
   unsigned        fProcessTab             : 1;
   unsigned        fScrollBarShowMax       : 1;
   unsigned        fScrollBarMaxAlign      : 1;
   unsigned        fArrowsExitEditMode     : 1;
   unsigned        fMoveActiveOnFocus      : 1;
   unsigned        fRetainSelBlock         : 1;
   unsigned        fEditModeReplace        : 1;
   unsigned        fNoBorder               : 1;
   unsigned        fNoBeep                 : 1;
   unsigned        fAutoClipboard          : 1;
   unsigned        fAllowMultipleSelBlocks : 1;
   unsigned        fAllowCellOverflow      : 1;
   unsigned        fVirtualMode            : 1;
   unsigned        fAllowDragDrop          : 1;
   unsigned        fUseSpecialVScroll      : 1;
   unsigned        fSetBorder              : 1;

   SS_SAVECOLOR    Color;
   SS_FONTID       DefaultFontId;
   SS_SAVECELLTYPE3 DefaultCellType;
   SS_COORD        HeaderRows;
   SS_COORD        HeaderCols;
   SS_COORD        RowsFrozen;
   SS_COORD        ColsFrozen;
   SS_COORD        RowsMax;
   SS_COORD        ColsMax;
   SS_COORD        RowCnt;
   SS_COORD        ColCnt;
   SS_COORD        DataRowCnt;
   SS_COORD        DataColCnt;
   double          ColWidth;
   double          RowHeight;
   short           FontTableCnt;
   short           ColorTableCnt;
   SS_SAVEDATEFORMAT DefaultDateFormat;
   SS_SAVETIMEFORMAT DefaultTimeFormat;
   char            fUseFontPointSize;

   WORD            wButtonDrawMode;
   WORD            wEnterAction;
   WORD            wOpMode;
   WORD            wGridType;
   WORD            wUserResize;
   WORD            wSelBlockOption;
   WORD            wSpecialVScrollOptions;
   COLORREF        GridColor;
   SS_COLOR        LockColor;
   BYTE            dUnitType;
   BYTE            bMiscFlags;
   SS_COORD        ColNumStart;
   SS_COORD        RowNumStart;
   SS_SAVEFLOATFORMAT     DefaultFloatFormat;
   SS_COORD        AutoSizeVisibleCols;
   SS_COORD        AutoSizeVisibleRows;
   SS_SAVEVIRTUAL  Virtual;
   WORD            ColHeaderDisplay;
   WORD            RowHeaderDisplay;
   } SS_SAVESPREAD2, FAR *LPSS_SAVESPREAD2;

typedef struct savespread3
   {
   unsigned        fDataProvided           : 1;
   unsigned        DocumentProtected       : 1;
   unsigned        DocumentLocked          : 1;
   unsigned        AllowResize             : 1;
   unsigned        AllowSelectBlock        : 1;
   unsigned        RestrictRows            : 1;
   unsigned        RestrictCols            : 1;
   unsigned        CalcAuto                : 1;
   unsigned        Redraw                  : 1;
   unsigned        GridLines               : 1;
   unsigned        HorzScrollBar           : 1;
   unsigned        VertScrollBar           : 1;
   unsigned        AllowUserFormulas       : 1;
   unsigned        fEditModePermanent      : 1;
   unsigned        fAutoSize               : 1;
   unsigned        fScrollBarExtMode       : 1;
   unsigned        fProcessTab             : 1;
   unsigned        fScrollBarShowMax       : 1;
   unsigned        fScrollBarMaxAlign      : 1;
   unsigned        fArrowsExitEditMode     : 1;
   unsigned        fMoveActiveOnFocus      : 1;
   unsigned        fRetainSelBlock         : 1;
   unsigned        fEditModeReplace        : 1;
   unsigned        fNoBorder               : 1;
   unsigned        fNoBeep                 : 1;
   unsigned        fAutoClipboard          : 1;
   unsigned        fAllowMultipleSelBlocks : 1;
   unsigned        fAllowCellOverflow      : 1;
   unsigned        fVirtualMode            : 1;
   unsigned        fAllowDragDrop          : 1;
   unsigned        fUseSpecialVScroll      : 1;
   unsigned        fSetBorder              : 1;

   SS_SAVECOLOR    Color;
   SS_FONTID       DefaultFontId;
   SS_COORD        HeaderRows;
   SS_COORD        HeaderCols;
   SS_COORD        RowsFrozen;
   SS_COORD        ColsFrozen;
   SS_COORD        RowsMax;
   SS_COORD        ColsMax;
   SS_COORD        RowCnt;
   SS_COORD        ColCnt;
   SS_COORD        DataRowCnt;
   SS_COORD        DataColCnt;
   double          ColWidth;
   double          RowHeight;
   short           FontTableCnt;
   short           ColorTableCnt;
   SS_SAVEDATEFORMAT DefaultDateFormat;
   SS_SAVETIMEFORMAT DefaultTimeFormat;
   char            fUseFontPointSize;

   WORD            wButtonDrawMode;
   WORD            wEnterAction;
   WORD            wOpMode;
   WORD            wGridType;
   WORD            wUserResize;
   WORD            wSelBlockOption;
   WORD            wSpecialVScrollOptions;
   COLORREF        GridColor;
   SS_SAVECOLOR    LockColor;
   BYTE            dUnitType;
   BYTE            bMiscFlags;
   SS_COORD        ColNumStart;
   SS_COORD        RowNumStart;
   SS_SAVEFLOATFORMAT     DefaultFloatFormat;
   SS_COORD        AutoSizeVisibleCols;
   SS_COORD        AutoSizeVisibleRows;
   SS_SAVEVIRTUAL  Virtual;
   WORD            ColHeaderDisplay;
   WORD            RowHeaderDisplay;
   } SS_SAVESPREAD3, FAR *LPSS_SAVESPREAD3;

#define SS_SAVEMISC_XTRA      0x01
#define SS_SAVEMISC_DATAAWARE 0x02

/******************************
* Declare function prototypes
******************************/

BOOL SS_SaveToFile(LPSS_BOOK lpBook, LPCTSTR lpFileName, BOOL fDataOnly);
BOOL SS_LoadFromFile(LPSS_BOOK lpBook, LPCTSTR lpFileName);

GLOBALHANDLE    SS_SaveData(LPSPREADSHEET lpSS, LPLONG lpBufferLen);
GLOBALHANDLE    SS_SaveAll(LPSPREADSHEET lpSS, LPLONG lpBufferLen);
HPBYTE          SS_SaveCellType(LPSPREADSHEET lpSS, HPBYTE Buffer,
                                LPLONG BufferLen, LPLONG AllocLen,
                                LPHANDLE hBuffer, TBGLOBALHANDLE hCellType,
                                SS_COORD Col, SS_COORD Row,
                                LPBOOL lpfPaletteSaved);
HPBYTE          SS_SaveCellTypeExt(LPSPREADSHEET lpSS, HPBYTE Buffer,
                                   LPLONG BufferLen, LPLONG AllocLen,
                                   LPHANDLE hBuffer,
                                   LPSS_CELLTYPE lpCellType,
                                   SS_COORD Col, SS_COORD Row,
                                   LPBOOL lpfPaletteSaved);
BOOL SS_LoadDataFromBuffer(LPSPREADSHEET lpSS,
                                      HPBYTE lpBuffer,
                                      long lBufferLen);
BOOL SS_LoadTabDataFromBuffer(LPSPREADSHEET lpSS,
                                         HPBYTE lpBuffer,
                                         long lBufferLen);
BOOL SS_LoadAllFromBuffer(LPSPREADSHEET lpSS,
                                     HPBYTE lpBuffer,
                                     long lBufferLen);
HPBYTE          SS_LoadCellType(LPSPREADSHEET lpSS,
                             HPBYTE lpBuffer, long lBufferLen,
                             SS_COORD Col, SS_COORD Row,
                             LPHPALETTE lphPalDefault,
                             LPSHORT lpdPaletteSize,
                             LPGLOBALHANDLE lphGlobalPalette,
                             short dCellTypeSize);
HPBYTE          SS_SaveAlloc(HPBYTE Buffer, LPLONG BufferLen,
                             LPVOID Value, long ValueLen,
                             LPLONG AllocLen, LPHANDLE hBuffer);

BOOL SS_LoadFromBuffer(LPSS_BOOK lpBook, HPBYTE lpBuffer, long lBufferLen);
GLOBALHANDLE SS_SaveToBuffer(LPSS_BOOK lpBook, LPLONG lpBufferLen,
                             BOOL fDataOnly, LPBOOL lpfRet);
//GAB 01/10/02 Changed prototype to include new Unicode parameter
BOOL SS_SaveTabFile(LPSPREADSHEET lpSS, LPCTSTR lpFileName, BOOL fUnicode);

#ifdef SS_VB
HANDLE CreateVbPict(HANDLE hPicture, BOOL bIsBitmap, BOOL bIsIcon);
#endif

//??????????????????????????????????????
#if !defined(SS_EARLY_32BIT_BETA)
#pragma pack()
#endif
//??????????????????????????????????????

#ifdef __cplusplus
}
#endif

#endif
