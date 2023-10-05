/*********************************************************
* XLbiff.h
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

#ifndef RC_INVOKED
#pragma pack(1)
#endif   // ifndef RC_INVOKED

#ifndef FP_API
#ifdef _MSC_VER
#ifdef WIN32
#define FP_API      __declspec(dllexport) __stdcall
#else //not WIN32
#define FP_API        FAR PASCAL _export
#endif
#else	  // OWL
#define FP_API        FAR PASCAL _export
#endif
#endif

//BIFF Tokens
#define xl1904            0x22     //1904 Excel Date System
#define xlADDIN           0x87     //Workbook is an Add-in Macro
#define xlADDMENU         0xC2     //Menu Addition
#define xlBACKUP          0x40     //Save Backup Version of the File

#define xlBOF_BIFF2       0x0009   //Beginning of File BIFF2
#define xlBOF_BIFF3       0x0209   //Beginning of File BIFF3
#define xlBOF_BIFF4       0x0409   //Beginning of File BIFF4
#define xlBOF_BIFF5_7_8   0x0809   //Beginning of File BIFF5/BIFF7/BIFF8

//cell values
#define xlBLANK           0x201    //Cell Value, Blank Cell
#define xlBOOLERR         0x205    //Cell Value, Boolean or Error
#define xlFORMULA         0x406    //Cell Formula
#define xlFORMULA2        0x06     //Cell Formula
#define xlLABEL           0x204    //Cell Value, String Constant
#define xlNUMBER          0x203    //Cell Value, Floating-Point Number
#define xlRK              0x27E    //Cell Value, RK Number !!!RAP docs say 0x7E
#define xlMULBLANK        0xBE     //Multiple Blank Cells
#define xlMULRK           0xBD     //Multiple RK Cells

#define xlARRAY           0x221    //workbook options flag
#define xlBOOKBOOL        0xDA     //workbook options flag
#define xlBOTTOMMARGIN    0x29     //Bottom Margin Measurement
#define xlBUNDLESHEET     0x85     //Sheet Info - aka BOUNDSHEET
#define xlCALCCOUNT       0x0C     //Iteration Count
#define xlCALCMODE        0x0D     //Calculation Mode
#define xlCF              0x1B1    // (new for BIFF8)
#define xlCODENAME        0x1BA    //VBE Object Name
#define xlCODEPAGE        0x42     //Default Code Page
#define xlCOLINFO         0x7D     //Column Formatting Info
#define xlCONDFMT         0x1B0    // (new for BIFF8)
#define xlCONTINUE        0x3C     //Continues Long Records
#define xlCOORDLIST       0xA9     //Polygon Object Vertex Coordinates
#define xlCOUNTRY         0x8C     //Default Country & WIN.INI Country
#define xlCRN             0x5A     //Nonresident Operands
#define xlDBCELL          0xD7     //Stream Offsets
#define xlDCON            0x50     //Data Consolidation Info
#define xlDCONNAME        0x52     //Data Consolidation Named References
#define xlDCONREF         0x51     //Data Consolidation References
#define xlDEFAULTROWHEIGHT 0x225   //Default Row Height
#define xlDEFCOLWIDTH     0x55     //Default Col Width
#define xlDELMENU         0xC3     //Menu Deletion
#define xlDELTA           0x10     //Iteration Increment
#define xlDIMENSIONS      0x200    //Cell Table Size
#define xlDOCROUTE        0xB8     //Routing Slip Info
#define xlDSF             0x161    //Double Stream File
#define xlDV              0x1BE    // (new for BIFF8)
#define xlDVAL            0x1B2    // (new for BIFF8)
#define xlEDG             0x88     //Edition Globals
#define xlEOF_BIFF        0x0A     //End of File
#define xlEXTERNCOUNT     0x16     //Number of External References
#define xlEXTERNNAME      0x23    //Externally Referenced Name(DDE Link)
#define xlEXTERNSHEET     0x17     //External Reference
#define xlEXTSST          0xFF     //Extended Shared String Table(New for BIFF8)
#define xlFILEPASS        0x2F     //File Is Password-Protected
#define xlFILESHARING     0x5B     //File-Sharing Info
#define xlFILESHARING2    0x1A4    //File-Sharing Info for Shared Lists(New for BIFF7)
#define xlFILTERMODE      0x9B     //Sheet Contains Filtered List
#define xlFNGROUPCOUNT    0x9C     //Built-in Function Group Count
#define xlFONT            0x31     //Font Description !!!RAP - doc says 0x231
#define xlFOOTER          0x15     // Print FOoter on Each Page
#define xlFORMAT          0x41E    //Number Format
#define xlGCW             0xAB     //Global Column-Width Flags
#define xlGRIDSET         0x82     //State Change of Gridlines Option
#define xlGUTS            0x80     //Size of Row and Column Gutters
#define xlHCENTER         0x83     //Center Between Horiz Margins
#define xlHEADER          0x14     //Print Header on Each Page
#define xlHIDEOBJ         0x8D     //Object Display Options
#define xlHLINK           0x1B8    // (new for BIFF8)
#define xlHORIZONTALPAGEBREAKS 0x1B //Explicit Row Page Breaks
#define xlIMDATA          0x7F     //Image Data
#define xlINDEX           0x20B    //Index Record
#define xlINTERFACEEND    0xE2     //End of User Interface Records
#define xlINTERFACEHDR    0xE1     //Beginning of User Interface Records
#define xlITERATION       0x11     //Iteration Mode
#define xlLABELSST        0xFD     //Cell Value, String Constant/SST(New for BIFF8)
#define xlLEFTMARGIN      0x26     //Left Margin Measurement
#define xlLHNGRAPH        0x95     //Named Graph Info
#define xlLHRECORD        0x94     //.WK? File Conversion Info
#define xlLPR             0x98     //Sheet was printed using LINE.PRINT()
#define xlMERGECELLS      0xE5     // undoc...
#define xlMERGE_DONTKNOW  0xEF     // undoc. I don't know what this record is, but it always follows xlMERGECELLS
#define xlMMS             0xC1     //ADDMENU/DELMENU Record Group Count
#define xlMSODRAWING      0xEC     // (new for BIFF8)
#define xlMSODRAWINGGROUP 0xEB     // (new for BIFF8)
#define xlMSODRAWINGSELECTION 0xED // (new for BIFF8)
#define xlNAME            0x18    //Defined Name
#define xlNAME2           0x218    //Defined Name
#define xlNOTE            0x1C     //Note Associated with a Cell
#define xlOBJ             0x5D     //Describes a Graphic Object
#define xlOBJPROTECT      0x63     //Objects are Protected
#define xlOBPROJ          0xD3     //VB Project (reserved)
#define xlOLESIZE         0xDE     //Size of OLE Object - for embedded OLE Object
#define xlPALETTE         0x92     //Color Palette Definition
#define xlPANE            0x41     //Number of Panes and Their Position
#define xlPARAMQRY        0xDC     // (new for BIFF8)
#define xlPASSWORD        0x13     //Protection Password - encrypted
#define xlPLS             0x4D     //Environment-Specific Print Record
#define xlPRECISION       0x0E     //Precision options from Options dlg, Calculation tab
#define xlPRINTGRIDLINES  0x2B     //Print Gridlines
#define xlPRINTHEADERS    0x2A     //Print Row/Col Labels
#define xlPROT4REV        0x1AF    //Shared Workbook Protection Flag(new for BIFF8)
#define xlPROT4REVPASS    0x1BC    //Shared Workbook Protection Password(Encrypted)(new for BIFF8)
#define xlPROTECT         0x12     //Protection Flag
#define xlPUB             0x89     //Publisher
#define xlQSI             0x1AD    // (new for BIFF8)
#define xlRECIPNAME       0xB9     //Recipient Name
#define xlREFMODE         0x0F     //Reference Mode
#define xlREFRESHALL      0x1B7    //Refresh Flag(new for BIFF8)
#define xlRIGHTMARGIN     0x27     //Right Margin Measurement
#define xlROW             0x208    //Describes a Row
#define xlRSTRING         0xD6     //Cell with Character Formatting
#define xlSAVERECALC      0x5F     //Recalculate Before Save
#define xlSCENARIO        0xAF     //Scenario Data
#define xlSCENMAN         0xAE     //Scenario Output Data
#define xlSCENPROTECT     0xDD     //Scenario Protection
#define xlSELECTION       0x1D     //Current Selection
#define xlSETUP           0xA1     //Page Setup
#define xlSHRFMLA         0xBC     //Shared Formula
#define xlSHRFMLA2        0x4BC    //Shared Formula
#define xlSORT            0x90     //Sorting Options
#define xlSOUND           0x96     //Sound Note
#define xlSST             0xFC     //Shared String Table(new for BIFF8)
#define xlSTANDARDWIDTH   0x99     //Standard Column Width
#define xlSTRING          0x207    //Sting Value of a Formula
#define xlSTYLE           0x293    //Style Information
#define xlSUB             0x91     //Subscriber
#define xlSUPBOOK         0x1AE    //(new for BIFF8)
#define xlSXDB            0xC6     //(new for BIFF8)
#define xlSXDBEX          0x122    //(new for BIFF8)
#define xlSXDI            0xC5     //Data Item
#define xlSXEX            0xF1     //(new for BIFF8)
#define xlSXEXT           0xDC     //External Source Info
#define xlSXIDSTM         0xD5     //Stream ID
#define xlSXFDBTYPE       0x1BB    //(new for BIFF8)
#define xlSXFILT          0xF2     //(new for BIFF8)
#define xlSXFMLA          0xF9     //(new for BIFF8)
#define xlSXFORMAT        0xFB     //(new for BIFF8)
#define xlSXFORMULA       0x103    //(new for BIFF8)
#define xlSXIVD           0xB4     //Row/Col Field IDs
#define xlSXLI            0xB5     //Line Item Array
#define xlSXNAME          0xF6     //(new for BIFF8)
#define xlSXPAIR          0xF8     //(new for BIFF8)
#define xlSXPI            0xB6     //Page Item
#define xlSXRULE          0xF0     //(new for BIFF8)
#define xlSXSELECT        0xF7     //(new for BIFF8)
#define xlSXSTRING        0xCD     //String
#define xlSXTBL           0xD0     //Multiple consolidation Source Info
#define xlSXTBPG          0xD2     //Page Item Indices
#define xlSXTBRGIITM      0xD1     //Page Item Name Count
#define xlSXVD            0xB1     //View Fields
#define xlSXVDEX          0x100    //(new for BIFF8)
#define xlSXVI            0xB2     //View Item
#define xlSXVIEW          0xB0     //View Definition
#define xlSXVS            0xE3     //View Source
#define xlTABID           0x13D    //Sheet Tab Index Array (New for BIFF7)
#define xlTABIDCONF       0xEA     //Sheet Tab ID of Conflict History (New for BIFF7)
#define xlTABLE           0x236    //Data Table
#define xlTEMPLATE        0x60     //Workbook is a Template
#define xlTOPMARGIN       0x28     //Top Margin Measurement
#define xlTXO             0x1B6    //(new for BIFF8)
#define xlUDDESC          0xDF     //Description String for Chart Autoformat
#define xlUNCALCED        0x5E     //Recalculation Status
#define xlUSERBVIEW       0x1A9    //(new for BIFF8)
#define xlUSERSVIEWBEGIN  0x1AA    //(new for BIFF8)
#define xlUSERSVIEWEND    0x1AB    //(new for BIFF8)
#define xlUSESELFS        0x160    //Natural Language Formulas Flag(new for BIFF8)
#define xlVCENTER         0x84     //Center Between Vert Margins
#define xlVERTICALPAGEBREAKS 0x1A  //Explicit Column Page Breaks
#define xlWINDOW1         0x3D     //Window Information
#define xlWINDOW2         0x23E    //Sheet Window Information
#define xlWINDOWPROTECT   0x19     //Windows Are Protected
#define xlWRITEACCESS     0x5C     //Write Access User Name
#define xlWRITEPROT       0x86     //Workbook Is Write-Protected
#define xlWSBOOL          0x81     //Additional Workspace Info
#define xlXCT             0x59     //CRN Record Count
#define xlXF              0xE0     //Extended Format (0x43 for previous versions)
#define xlXF_OLD          0x43
#define xlXL5MODIFY       0x162    //(new for BIFF8)

// Excel Parsed Thing(ptg) Tokens
#define ptgExp        0x01 //control
#define ptgTbl        0x02 //control
#define ptgAdd        0x03 //operator
#define ptgSub        0x04 //operator
#define ptgMul        0x05 //operator
#define ptgDiv        0x06 //operator
#define ptgPower      0x07 //operator
#define ptgConcat     0x08 //operator
#define ptgLT         0x09 //operator
#define ptgLE         0x0A //operator
#define ptgEQ         0x0B //operator
#define ptgGE         0x0C //operator
#define ptgGT         0x0D //operator
#define ptgNE         0x0E //operator
#define ptgIsect      0x0F //operator
#define ptgUnion      0x10 //operator
#define ptgRange      0x11 //operator
#define ptgUplus      0x12 //operator
#define ptgUminus     0x13 //operator
#define ptgPercent    0x14 //operator
#define ptgParen      0x15 //control
#define ptgMissArg    0x16 //operand
#define ptgStr        0x17 //operand
#define ptgAttr       0x19 //control
#define ptgSheet      0x1A //(ptg DELETED)
#define ptgEndSheet   0x1B //(ptg DELETED)
#define ptgErr        0x1C //operand
#define ptgBool       0x1D //operand
#define ptgInt        0x1E //operand
#define ptgNum        0x1F //operand
#define ptgArray      0x20 //operand, reference class
#define ptgFunc       0x21 //operator
#define ptgFuncVar    0x22 //operator
#define ptgName       0x23 //operand, reference class
#define ptgRef        0x24 //operand, reference class
#define ptgArea       0x25 //operand, reference class
#define ptgMemArea    0x26 //operand, reference class
#define ptgMemErr     0x27 //operand, reference class
#define ptgMemNoMem   0x28 //control
#define ptgMemFunc    0x29 //control
#define ptgRefErr     0x2A //operand, reference class
#define ptgAreaErr    0x2B //operand, reference class
#define ptgRefN       0x2C //operand, reference class
#define ptgAreaN      0x2D //operand, reference class
#define ptgMemAreaN   0x2E //control
#define ptgMemAreaNoMemN 0x2F //control
#define ptgNameX      0x39 //operand, reference class
#define ptgRef3d      0x3A //operand, reference class
#define ptgArea3d     0x3B //operand, reference class
#define ptgRefErr3d   0x3C //operand, reference class
#define ptgAreaErr3d  0x3D //operand, reference class
#define ptgArrayV     0x40 //operand, value class
#define ptgFuncV      0x41 //operator
#define ptgFuncVarV   0x42 //operator
#define ptgNameV      0x43 //operand, value class
#define ptgRefV       0x44 //operand, value class
#define ptgAreaV      0x45 //operand, value class
#define ptgMemAreaV   0x46 //operand, value class
#define ptgMemErrV    0x47 //operand, value class
#define ptgMemNoMemV  0x48 //control
#define ptgMemFuncV   0x49 //control
#define ptgRefErrV    0x4A //operand, value class
#define ptgAreaErrV   0x4B //operand, value class
#define ptgRefNV      0x4C //operand, value class
#define ptgAreaNV     0x4D //operand, value class
#define ptgMemAreaNC  0x4E //control
#define ptgMemNoMemNV 0x4F //control
#define ptgFuncCEV    0x58 //operator
#define ptgNameXV     0x59 //operand, value class
#define ptgRef3dV     0x5A //operand, value class
#define ptgArea3dV    0x5B //operand, value class
#define ptgRefErr3dV  0x5C //operand, value class
#define ptgAreaErr3dV 0x5D //operand, value class
#define ptgArrayA     0x60 //operand, value class
#define ptgFuncA      0x61 //operator
#define ptgFuncVarA   0x62 //operator
#define ptgNameA      0x63 //operand, array class
#define ptgRefA       0x64 //operand, array class
#define ptgAreaA      0x65 //operand, array class
#define ptgMemAreaA   0x66 //operand, array class
#define ptgMemErrA    0x67 //operand, array class
#define ptgMemNoMemA  0x68 //control
#define ptgMemFuncA   0x69 //control
#define ptgRefErrA    0x6A //operand, array class
#define ptgAreaErrA   0x6B //operand, array class
#define ptgRefNA      0x6C //operand, array class
#define ptgAreaNA     0x6D //operand, array class
#define ptgMemAreaNA  0x6E //control
#define ptgMemNoMemNA 0x6F //control
#define ptgFuncCEA    0x78 //operator
#define ptgNameXA     0x79 //operand, array class(NEW ptg)
#define ptgRef3dA     0x7A //operand, array class(NEW ptg)
#define ptgArea3dA    0x7B //operand, array class(NEW ptg)
#define ptgRefErr3dA  0x7C //operand, array class(NEW ptg)
#define ptgAreaErr3dA 0x7D //operand, array class(NEW ptg)

//Chart BIFF Tokens
#define xlTHREED        0x103A  //Chart Group Is a 3-D Chart Group
#define xlAI            0x1051  //Linked Data
#define xlALRUNS        0x1050  //Text Formatting
#define xlAREA          0x101A  //Chart Group Is an Area Chart Group
#define xlAREAFORMAT    0x100A  //Colors and Patterns for an Area
#define xlATTACHEDLABEL 0x100C  //Series Data/Value Labels
#define xlAXESUSED      0x1046  //Number of Axes Sets
#define xlAXIS          0x101D  //Axis Type
#define xlAXISLINEFORMAT 0x1021 //Defines a Line That Spans an Axis
#define xlAXISPARENT    0x1041  //Axis Size and Location
#define xlBAR           0x1017  //Chart Group is a Bar or Column Chart Group
#define xlBEGIN         0x1033  //Defines the Beginning of an Object
#define xlCATSERRANGE   0x1020  //Defines a Category or Series Axis
#define xlCHART         0x1002  //Location and Overall Chart Dimensions
#define xlCHARTFORMAT   0x1014  //Parent Record for Chart Group
#define xlCHARTFORMATLINK 0x1022//Not Used
#define xlCHARTLINE     0x101C  //Drop/Hi-Lo/Series Lines on a Line Chart
#define xlDATAFORMAT    0x1006  //Series and Data Point Numbers
#define xlDEFAULTTEXT   0x1024  //Default Data Label Text Properties
#define xlDROPBAR       0x103D  //Defines Drop Bars
#define xlEND           0x1034  //Defines the End of an Object
#define xlFONTX         0x1026  //Font Index
#define xlFRAME         0x1032  //Defines Border Shape Around Displayed Text
#define xlIFMT          0x104E  //Number-Format Index
#define xlLEGEND        0x1015  //Legend Type and Position
#define xlLEGENDXN      0x1043  //Legend Exception
#define xlLINE          0x1018  //Chart Group Is a Line Chart Group
#define xlLINEFORMAT    0x1007  //Style of a Line or Border
#define xlMARKERFORMAT  0x1009  //Style of a Line Marker
#define xlOBJECTLINK    0x1027  //Attaches Text to Chart or to Chart Item
#define xlPICF          0x103C  //Picture Format
#define xlPIE           0x1019  //Chart Group Is a Pie Chart Group
#define xlPIEFORMAT     0x100B  //Position of the Pie Slice
#define xlPLOTAREA      0x1035  //Frame Belongs to Plot Area 
#define xlPOS           0x104F  //Position Information
#define xlRADAR         0x103E  //Chart Group Is a Radar Chart Group
#define xlRADARAREA     0x1040  //Chart Group Is a Radar Area Chart Group
#define xlSBASEREF      0x1048  //PivotTable Reference
#define xlSCATTER       0x101B  //Chart Group Is a Scatter Chart Group
#define xlSERAUXERRBAR  0x105B  //Series ErrorBar
#define xlSERAUXTREND   0x104B  //Series Trendline
#define xlSERFMT        0x105D  //Series Format
#define xlSERIES        0x1003  //Series Definition
#define xlSERIESLIST    0x1016  //Specifies the Series in an Overlay Chart
#define xlSERIESTEXT    0x100D  //Legend/Category/Value Text
#define xlSERPARENT     0x104A  //Trendline or ErrorBar Series Index
#define xlSERTOCRT      0x1045  //Series Chart-Group Index
#define xlSHTPROPS      0x1044  //Sheet Properties
#define xlSURFACE       0x103F  //Chart Group Is a Surface Chart Group
#define xlTEXTDISPLAY   0x1025  //Defines Display of Text Fields
#define xlTICK          0x101E  //Tick Marks and Labels Format
#define xlUNITS         0x1001  //Chart Units
#define xlVALUERANGE    0x101F  //Defines Value Axis Scale


#define XL_FILETYPE  _T("XLS")    //Excel File Type

#define BOF -2

typedef struct tag_biff
{
  WORD recnum;
  WORD datalen;
}BIFF, FAR *LPBIFF;

#if defined(_WIN64) || defined(_IA64)
#define LPOFFSET(lp, n) (LPBYTE)((LONG_PTR)lp + n)
#else
#define LPOFFSET(lp, n) (LPBYTE)((DWORD)lp + n)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// BIFF functions
short bif_Init(LPSSXL lpss);
short bif_Parse(LPBYTE lpBiffBuffer, LPSSXL lpss, unsigned long far *lplLen, LPSS_BOOK lpBook, int nSheetNum, LPSHORT lpsCurrentSheet, LPSHORT lpsSheetsLoadedCount, short sPass, LPBOOL lpIsValidSubstream, long lTotalLen);
short bif_Tokenize1(LPBIFF lpBiff, LPSSXL lpss, LPSS_BOOK lpBook, int nSheetNum);
short bif_Tokenize2(LPBIFF lpBiff, LPSSXL lpss, LPSS_BOOK lpBook, int nSheetNum, LPBOOL lpIsValidSubstream);
short bif_LoadData(TBGLOBALHANDLE FAR *lpgh, WORD wLen, LPBYTE lp, BOOL fNullTerm, BOOL fWChar, long lSize);
//Add by BOC 99.4.7(hyt)------------------------------------------------------------------------------------
//for support DBCS 
WORD bif_LoadDataBoc(TBGLOBALHANDLE FAR *lpgh, WORD wLen, LPBYTE lp, BOOL fNullTerm, BOOL fWChar, long lSize,BOOL fUnicode);
//------------------------------------------------------------------------------------------------------------
short bif_Build(LPBIFF lpBiff, LPVOID lp, LPVOID lpGH, LPLONG lplSize, LPVOID lpExtra);
short bif_SaveData(LPBIFF lpBiff, LPVOID lpData1, TBGLOBALHANDLE gh2, WORD wSize2, LPVOID lpGH, LPLONG lplSize);
BOOL bif_LoadCell(LPSSXL lpssXL, LPSPREADSHEET lpSS, LPyCELLVAL lpCell);
BOOL bif_SetRowProps(LPSSXL lpssXL, LPSPREADSHEET lpSS, LPyROW lpRow, BOOL fRowHeightOnly);
BOOL bif_SetColProps(LPSSXL lpssXL, LPSPREADSHEET lpSS, LPyCOLINFO lpCol);

//Utility functions
short Alloc(TBGLOBALHANDLE FAR *lpgh, long lSize);
COLORREF xl_ColorFromIndex(long lIndex);
long xl_IndexFromColor(COLORREF color);
long xl_CatWSToWS(LPTBGLOBALHANDLE lpghDest, LPLONG lplDestLen, LPTBGLOBALHANDLE lpghSrc, LPLONG lplSrcLen);


// Functions
#ifdef XL_DLL
short FP_API xl_LoadFile(LPCTSTR szXLFileName, LPSSXL lpSSXL, LPSPREADSHEET lpSS, int nSheetNum, short sPass, BOOL fCheckOnly, HANDLE hSpread);
#else
short FP_API xl_LoadFile(LPCTSTR szXLFileName, LPSSXL lpSSXL, LPSS_BOOK lpBook, int nSheetNum, short sPass, BOOL fCheckOnly);
#endif
short FP_API xl_SaveFile(LPCTSTR szFileName, LPCTSTR szSheetName, LPVOID lp, LPTBGLOBALHANDLE lpghBuffer, LPLONG lplBufferlen, BOOL fToFile, SHORT sFlags);
//short FP_API xl_LogFile(LPLONG lphf, short LogId, long lCol, long lRow, LPVOID lpData);
short FP_API xl_LogFile(LPSS_BOOK lpBook, short LogId, long lCol, long lRow, LPVOID lpData);
BOOL FP_API xl_ParseFormat(LPSPREADSHEET lpSS, LPTSTR lptstr, LPFMT lpf, LPWORD lpwType, LPVOID lpData);
BOOL FP_API xl_PreParseFormat(LPSPREADSHEET lpSS, LPTSTR lptstr, LPFMT lpf, LPWORD lpwType);
BOOL FP_API xl_ParseFormula(LPSPREADSHEET lpSS, LPSSXL lpss, LPyyFORMULA lpfx, LPTSTR lptstrFx, LPyCELLVAL lpcell, LPBYTE lpbType, LPFMT lpf);
BOOL FP_API bif_GetXF(TBGLOBALHANDLE gh, WORD wIndex, LPxXF8 lpxf);
BOOL FP_API bif_GetFont(TBGLOBALHANDLE gh, WORD wIndex, LPHFONT lphf, LPCOLORREF lpColor);
BOOL FP_API xl_GetCoord(LPSPREADSHEET lpSS, LPRWCO lprc, short sRefStyle, LPyCELLVAL lpcell, LPTSTR lptstr);
#ifdef SS_V70
BOOL FP_API xl_GetCoord3D(LPSPREADSHEET lpSS, LPSSXL lpss, BOOL fFirstSheet, LPRWCO lprc, short sRefStyle, short ixti, LPyCELLVAL lpcell, LPTSTR lptstr);
#endif
short FP_API xlGetDigitsLeft(double dbl);
short FP_API xlGetDigitsRight(double dbl);
short FP_API xl_FindShrFmla(LPSSXL lpss, LPyCELLVAL lpcell);
short FP_API xl_ParsePicFormat(LPSPREADSHEET lpSS, LPTSTR lptstrSrc, LPTSTR lptstrDest);

double FP_API xl_NumFromRk(long rk, BOOL FAR *lpbFloat);
BOOL FP_API xl_TimeToHMS(double dbl, LPSS_TIME lpssTime);

#ifdef SS_V80
#ifdef XL12
//BOOL FP_API xl_LoadExcel12File(LPFPCONTROL spread, LPCTSTR fileName);
//BOOL FP_API xl_SaveExcel12File(LPFPCONTROL spread, LPCTSTR fileName);
//BOOL FP_API xl_OpenExcel2007File(LPFPCONTROL spread, LPCTSTR fileName, LPCTSTR password, short sheet, short excelsheet, LPCTSTR logfilename);
//BOOL FP_API xl_SaveExcel2007File(LPFPCONTROL spread, LPCTSTR fileName, LPCTSTR password, short flags, LPCTSTR logfilename);
//BOOL FP_API xl_IsExcel2007File(LPFPCONTROL spread, LPCTSTR fileName);
#endif // XL12
#endif // SS_V80

#ifdef __cplusplus
}
#endif


#ifndef RC_INVOKED
#pragma pack()
#endif   // ifndef RC_INVOKED
