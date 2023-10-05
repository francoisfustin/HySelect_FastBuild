/***********
* DB_SDK.H
***********/

#ifndef DB_SDK_H
#define DB_SDK_H

#ifndef DBHANDLE
#define DBHANDLE long
#endif

#ifndef LPSTR
#define LPSTR char far *
#endif

#ifndef HPSTR
#define HPSTR char huge *
#endif

#ifndef LPDOUBLE
#define LPDOUBLE double far *
#endif

#ifndef LPLONG
#define LPLONG long far *
#endif

#ifdef DLLENTRY
#undef DLLENTRY
#endif


#if defined(WIN32) || defined(_WIN32)
   #if defined(SS_DLL) || defined(SS_OCX) || defined(SS_VB)
      #define DLLENTRY      __declspec(dllexport) __stdcall
      #define WINENTRY LONG __declspec(dllexport) __stdcall
      #define DLGENTRY BOOL __declspec(dllexport) __stdcall
   #else
      #define DLLENTRY      __stdcall
      #define WINENTRY LONG __stdcall
      #define DLGENTRY BOOL __stdcall
   #endif
#else
   #if defined(SS_DLL) || defined(SS_OCX) || defined(SS_VB)
      #define DLLENTRY      __export FAR PASCAL
      #define WINENTRY LONG __export FAR PASCAL
      #define DLGENTRY BOOL __export FAR PASCAL
   #else
      #define DLLENTRY      FAR PASCAL
      #define WINENTRY LONG FAR PASCAL
      #define DLGENTRY BOOL FAR PASCAL
   #endif
#endif

#define DATA_VT_BOOL	   1
#define DATA_VT_VAR_BYTE   2
#define DATA_VT_INTEGER    3
#define DATA_VT_LONG	   4
#define DATA_VT_CURRENCY   5
#define DATA_VT_SINGLE	   6
#define DATA_VT_DOUBLE	   7
#define DATA_VT_DATETIME   8
#define DATA_VT_TEXT	   10
#define DATA_VT_BINARY	   11
#define DATA_VT_MEMO	   12

/*********************
* Exported functions
*********************/
BOOL    DLLENTRY   xDB_LoadDLL();
void    DLLENTRY   xDB_FreeDLL();
HANDLE  DLLENTRY   xDB_GetDLLHandle();
int     DLLENTRY   Process_Errors(BOOL);
void    DLLENTRY   xDB_Error(void);
int     DLLENTRY   xDB_SDK_Begin_Trans(DBHANDLE);
int     DLLENTRY   xDB_SDK_Commit(DBHANDLE);
int     DLLENTRY   xDB_SDK_Connect(char far *);
int     DLLENTRY   xDB_SDK_Disconnect(DBHANDLE hdb);
int 	DLLENTRY 	xDB_SDK_Disconnect_Clone(DBHANDLE hdb);
int     DLLENTRY   xDB_SDK_Delete_Record(DBHANDLE);
int     DLLENTRY   xDB_SDK_End_Bind(DBHANDLE);
DBHANDLE DLLENTRY   xDB_SDK_Execute_SQL(DBHANDLE,DBHANDLE, char far *);
int     DLLENTRY   xDB_SDK_Get_Cell_Type(DBHANDLE,int,int far *, int far *);
LPSTR   DLLENTRY   xDB_SDK_Get_Char_Val_For_Field(DBHANDLE, char far *, int, char far *, int);
int     DLLENTRY   xDB_SDK_Get_Double_Val_For_Field(DBHANDLE hstmt, int Col, LPDOUBLE lpdfVal);
long    DLLENTRY   xDB_SDK_Get_Long_Val_For_Field(DBHANDLE hstmt, int Col);
int     DLLENTRY   xDB_SDK_Get_Date(DBHANDLE hstmt, int Col, LPSTR lpszDate, LPSTR lpszTime);
HGLOBAL DLLENTRY   xDB_SDK_Get_Binary(DBHANDLE hstmt, int Col, LPLONG lplLen);
int     DLLENTRY   xDB_SDK_Get_Current_Record_Number(DBHANDLE);
LPSTR   DLLENTRY   xDB_SDK_Get_Field_Name(DBHANDLE, char far *, int);
int     DLLENTRY   xDB_SDK_Get_Field_Size(DBHANDLE, int);
int     DLLENTRY   xDB_SDK_Get_Next_Record(DBHANDLE);
int     DLLENTRY   xDB_SDK_Get_Num_Of_Cols(DBHANDLE);
int     DLLENTRY   xDB_SDK_Get_Previous_Record(DBHANDLE);
int     DLLENTRY   xDB_SDK_Get_Random_Record(DBHANDLE, long);
int     DLLENTRY   xDB_SDK_New_Record(DBHANDLE, long);
void    DLLENTRY   xDB_SDK_Prompt_For_Query(DBHANDLE, char far *);
int     DLLENTRY   xDB_SDK_RollBack(DBHANDLE);
int     DLLENTRY   xDB_SDK_Set_Field(DBHANDLE, int, char far *, char far *);
int     DLLENTRY   xDB_SDK_Set_Binary(DBHANDLE hstmt, int Col, LPSTR lpVal, long lLen);
int     DLLENTRY   xDB_SDK_Set_Double(DBHANDLE hstmt, int Col, double dfVal);
int     DLLENTRY   xDB_SDK_Set_Int(DBHANDLE hstmt, int Col, int lVal);
int     DLLENTRY   xDB_SDK_Set_Long(DBHANDLE hstmt, int Col, long lVal);
int     DLLENTRY   xDB_SDK_Set_Null(DBHANDLE hstmt, int Col);
int     DLLENTRY   xDB_SDK_Set_Date(DBHANDLE hstmt, int Col, LPSTR lpszDate, LPSTR lpszTime);
int     DLLENTRY   xDB_SDK_Is_Rec_Mod_Or_Added(DBHANDLE hstmt);
int     DLLENTRY   xDB_SDK_Update_Record(DBHANDLE hstmt);
int     DLLENTRY   xDB_SDK_Lock_Rec(DBHANDLE);
long    DLLENTRY   xDB_SDK_Get_Num_Recs(DBHANDLE);

#endif
