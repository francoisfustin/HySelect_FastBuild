/************
 * FPSAVE.H *
 ************/

#ifndef FPSAVE_H
#define FPSAVE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RC_INVOKED
#pragma pack(1)
#endif  /* RC_INVOKED */

#define POINTS_PER_INCH        72
#define HEIGHT_UNITS_PER_POINT 100
#define HEIGHT_UNITS_PER_INCH  (HEIGHT_UNITS_PER_POINT * POINTS_PER_INCH)

typedef COLORREF FAR  *LPCOLORREF;

typedef struct tagFP_REC_LOGFONT
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
   char lfFaceName[LF_FACESIZE];
   } FP_REC_LOGFONT, FAR* LPFP_REC_LOGFONT;

typedef struct tagFP_REC_PICT
   {
   HGLOBAL hBuffer;
   long    lLen;
   } FP_REC_PICT, FAR *LPFP_REC_PICT;

typedef struct tagFP_REC_TABLE
   {
   short nCnt;
   short nCntSaved;
   } FP_REC_TABLE, FAR *LPFP_REC_TABLE;

typedef struct tagFP_LOAD_TABLEINDEX
   {
   short nSavedID;
   short nNewID;
   } FP_LOAD_TABLEINDEX, FAR *LPFP_LOAD_TABLEINDEX;

typedef struct tagFP_LOAD_TABLE
   {
   short          nCnt;
   short          nCntSaved;
   FPGLOBALHANDLE hData;
   FPGLOBALHANDLE hIndex;
   } FP_LOAD_TABLE, FAR *LPFP_LOAD_TABLE;

typedef struct tagFP_LOAD_TABLES
   {
   FP_LOAD_TABLE Font;
   FP_LOAD_TABLE Color;
   FP_LOAD_TABLE Pict;
   } FP_LOAD_TABLES, FAR *LPFP_LOAD_TABLES;

typedef struct tagFP_IO
   {
   BOOL fVB;
   union
      {
#ifdef FP_VB
      HFORMFILE hf;
#endif
      FP_HUGEBUFFER Buffer;
      } IO;
   } FP_IO, FAR *LPFP_IO;

/**********************
* Function Prototypes
**********************/
short FPLIB FP_LoadTables(LPFP_IO lpIO, LPFP_LOAD_TABLES lpTables);
short FPLIB FP_SaveTables(HANDLE hInst, LPFP_IO lpIO);
short FPLIB FP_SaveTablesEx(HANDLE hInst, LPFP_IO lpIO);

short FPLIB fpIO_Write(LPFP_IO lpfpIO, HPVOID lpData, long lSize);

short FPLIB FP_SaveValue(LPFP_IO lpIO, BYTE bTag, LPVOID lpValue, long lValSize);
short FPLIB FP_SaveData(LPFP_IO lpIO, LPVOID lpValue, long lValSize);
short FPLIB FP_SaveByte(LPFP_IO lpIO, BYTE bValue);
short FPLIB FP_SaveWord(LPFP_IO lpIO, WORD wValue);
short FPLIB FP_SaveLong(LPFP_IO lpIO, LONG lValue);
short FPLIB FP_SaveText(LPFP_IO lpIO, BYTE bTag, FPGLOBALHANDLE hText);
short FPLIB FP_SaveColorTable(LPFP_IO lpIO);
short FPLIB FP_SaveColorTableEx(HINSTANCE hInst, LPFP_IO lpIO);
short FPLIB FP_SaveFontTable(LPFP_IO lpIO);
short FPLIB FP_SaveFontTableEx(HINSTANCE hInst, LPFP_IO lpIO);
short FPLIB FP_SavePictTable(HANDLE hInst, LPFP_IO lpIO);

#define FP_SaveTag(lpIO, bTag) FP_SaveByte(lpIO, bTag)

short FPLIB fpIO_Read(LPFP_IO lpfpIO, HPVOID hpData, long lSize);

short FPLIB FP_LoadFontTable(LPFP_IO lpIO, LPFP_LOAD_TABLES lpTables);
short FPLIB FP_LoadColorTable(LPFP_IO lpIO, LPFP_LOAD_TABLES lpTables);
short FPLIB FP_LoadPictTable(LPFP_IO lpIO, LPFP_LOAD_TABLES lpTables);
void  FPLIB FP_LoadFreeTable(LPFP_LOAD_TABLE lpTable);
void  FPLIB FP_LoadFreeTables(LPFP_LOAD_TABLES lpTables);

FPCOLORID FPLIB FP_LoadConvertColorID(LPFP_LOAD_TABLES lpTables, FPCOLORID idColorOld);
FPCOLORID FPLIB FP_LoadConvertColorIDEx(HINSTANCE hInst, LPFP_LOAD_TABLES lpTables, FPCOLORID idColorOld);
FPFONT    FPLIB FP_LoadConvertFontID(LPFP_LOAD_TABLES lpTables, FPFONT idFontOld);
FPFONT    FPLIB FP_LoadConvertFontIDEx(HINSTANCE hInst, LPFP_LOAD_TABLES lpTables, FPFONT idFontOld);
FPPICTID  FPLIB FP_LoadConvertPictID(HANDLE hInst, LPFP_LOAD_TABLES lpTables, FPPICTID idPictOld, BYTE bCtlType);

#ifndef RC_INVOKED
#pragma pack()
#endif  /* RC_INVOKED */

#ifdef __cplusplus
}
#endif    

#endif
