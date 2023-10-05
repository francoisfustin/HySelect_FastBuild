// to do
// 1) set connect handle
// 2) set select handle
// 3) change locking
// 4) change auto commit

#include <windows.h>
#include <string.h>  
#include "vbapi.h"
#include "qelib.h"
#include "db_sdk.h"
#include "..\stringrc.h"

extern HANDLE hDynamicInst;

//function pointers into the Q+E Database Library DLL
static qeSTATUS (WINAPI *lpfnqeBeginTran)(qeHANDLE hdbc);
static qeLPSTR  (WINAPI *lpfnqeColName)(qeHANDLE hdbc, short col_num);
static qeSTATUS (WINAPI *lpfnqeColNameBuf)(qeHANDLE hdbc, qeLPSTR col_name, short col_num);
static short    (WINAPI *lpfnqeColScale)(qeHANDLE hdbc, short col_num);
static short    (WINAPI *lpfnqeColType)(qeHANDLE hdbc, short col_num);
static long     (WINAPI *lpfnqeColWidth)(qeHANDLE hdbc, short col_num);
static qeSTATUS (WINAPI *lpfnqeCommit)(qeHANDLE hdbc);
static qeHANDLE (WINAPI *lpfnqeConnect)(qeLPSTR con_string);
static qeSTATUS (WINAPI *lpfnqeDisconnect)(qeHANDLE hdbc);
static qeLPSTR  (WINAPI *lpfnqeErrMsg)(void);
static qeSTATUS (WINAPI *lpfnqeEndSQL)(qeHANDLE hstmt);
static qeSTATUS (WINAPI *lpfnqeFetchNext)(qeHANDLE hstmt);
static qeSTATUS (WINAPI *lpfnqeFetchPrev)(qeHANDLE hstmt);
static qeSTATUS (WINAPI *lpfnqeFetchRandom)(qeHANDLE hstmt, long rec_num);
static qeSTATUS (WINAPI *lpfnqeLibInit)(void);
static qeSTATUS (WINAPI *lpfnqeLibTerm)(void);
static short    (WINAPI *lpfnqeNumCols)(qeHANDLE hstmt);
static qeSTATUS (WINAPI *lpfnqePutBinary)(qeHANDLE hstmt, short col_num, LPSTR new_val, long val_len);
static qeSTATUS (WINAPI *lpfnqePutChar)(qeHANDLE hstmt, short col_num, qeLPSTR fmt_string, qeLPSTR new_val);
static qeSTATUS (WINAPI *lpfnqePutDouble)(qeHANDLE hstmt, short col_num, double new_val);
static qeSTATUS (WINAPI *lpfnqePutInt)(qeHANDLE hstmt, short col_num, int new_val);
static qeSTATUS (WINAPI *lpfnqePutLong)(qeHANDLE hstmt, short col_num, long new_val);
static qeSTATUS (WINAPI *lpfnqePutNull)(qeHANDLE hstmt, short col_num);
static qeHANDLE (WINAPI *lpfnqeQryAllocate)(qeHANDLE hdbc, qeLPSTR statement);
static qeSTATUS (WINAPI *lpfnqeQryBuilder)(qeHANDLE hqry, short parent_window, short flags, short init_dialog);
static qeSTATUS (WINAPI *lpfnqeQryGetStmtBuf)(qeHANDLE hqry, qeLPSTR stmt);
static qeSTATUS (WINAPI *lpfnqeQryFree)(qeHANDLE hqry);
static qeSTATUS (WINAPI *lpfnqeRecDelete)(qeHANDLE hstmt);
static qeSTATUS (WINAPI *lpfnqeRecLock)(qeHANDLE hstmt);
static qeSTATUS (WINAPI *lpfnqeRecNew)(qeHANDLE hstmt, long rec_num);
static long     (WINAPI *lpfnqeRecNum)(qeHANDLE hstmt);
static short    (WINAPI *lpfnqeRecState)(qeHANDLE hstmt);
static qeSTATUS (WINAPI *lpfnqeRecUpdate)(qeHANDLE hstmt);
static qeSTATUS (WINAPI *lpfnqeRollback)(qeHANDLE hdbc);
static qeSTATUS (WINAPI *lpfnqeSetAutoUpdate)(qeHANDLE hdbc, short option);
static qeSTATUS (WINAPI *lpfnqeSetSelectOptions)(qeHANDLE hdbc, long flags);
static qeSTATUS (WINAPI *lpfnqeSetLockOptions)(qeHANDLE hdbc, short option);
static qeHANDLE (WINAPI *lpfnqeSQLPrepare)(qeHANDLE hdbc, qeLPSTR stmt);
static qeSTATUS (WINAPI *lpfnqeSQLExecute)(qeHANDLE hdbc);
static qeLPSTR  (WINAPI *lpfnqeValChar)(qeHANDLE hstmt, short col_num, qeLPSTR fmt_string, short max_len);
static qeSTATUS (WINAPI *lpfnqeValCharBuf)(qeHANDLE hstmt, qeLPSTR char_val, short col_num, qeLPSTR fmt_string, short max_len);
static double   (WINAPI *lpfnqeValDouble)(qeHANDLE hstmt, short col_num);
static long     (WINAPI *lpfnqeValLong)(qeHANDLE hstmt, short col_num);
static qeSTATUS (WINAPI *lpfnqeCharToDateBuf)(qeLPSTR date_value,qeLPSTR char_value,qeLPSTR fmt_string);
static qeLPSTR  (WINAPI *lpfnqeDateToChar)(qeLPSTR date_val,qeLPSTR fmt_string);
static qeSTATUS (WINAPI *lpfnqeErr)(void);
static long     (WINAPI *lpfnqeDataLen)(qeHANDLE hstmt);
static long     (WINAPI *lpfnqeFetchNumRecs)(qeHANDLE hstmt);
       
static void     (WINAPI *lpfnSpreadConnect)(HCTL , DBHANDLE , DBHANDLE);
static void     (WINAPI *lpfnSpreadDisconnect)(HCTL);
static void     (WINAPI *lpfnSpreadEditQuery)(HCTL );

void DLLENTRY DBSS_Connect(HCTL hCtl, DBHANDLE ConnectHandle, DBHANDLE SelectHandle);
void DLLENTRY DBSS_Disconnect(HCTL hCtl);
void DLLENTRY DBSS_Edit_Query(HCTL hCtl);

//Global variables
HANDLE hInstQE = 0;
HANDLE hInstSpread = 0;
int    Connetion_Count;
int    ProcessErrors;

/********************************************************************/
void DLLENTRY DB_MemHugeCpy(HPSTR d,HPSTR s, LONG l)
{
long x;

    for(x=0; x<l; x++)
        d[x] = s[x]; 
}

/********************************************************************/
void DB_StrRTrim(LPSTR s)
{
int max;
    
    max = _fstrlen(s) - 1;
    while(s[max] == ' ')
        --max;
    s[++max] = 0;
}

#if DB_SDK_DLL
/********************************************************************/
int FAR PASCAL LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{      
    return(1);
}

/********************************************************************/
int FAR PASCAL WEP (int bSystemExit)
{
    xDB_FreeDLL();
    return(1);
}
#endif

/********************************************************************/
BOOL DLLENTRY xDB_LoadDLL()
{
OFSTRUCT OFStruct;
HANDLE   hInstQEOld;
short DLLENTRY SetHandles(HCTL, DBHANDLE, DBHANDLE, char far *, char far *);

    hInstQEOld = hInstQE;
    
    #if DB_SDK_DLL
    if ((hInstSpread = LoadLibrary("Spread20.VBX")) > HINSTANCE_ERROR)
        lpfnSpreadConnect = (void (WINAPI *)(HCTL hCtl, DBHANDLE, DBHANDLE, BOOL))GetProcAddress(hInstSpread, "DBSS_Connect");
        lpfnSpreadDisconnect = (void (WINAPI *)(HCTL hCtl, BOOL))GetProcAddress(hInstSpread, "DBSS_Disconnect");
        lpfnSpreadEditQuery = (void (WINAPI *)(HCTL))GetProcAddress(hInstSpread, "DBSS_Edit_Query");
    #else
        lpfnSpreadConnect = DBSS_Connect;
        lpfnSpreadDisconnect = DBSS_Disconnect;
        lpfnSpreadEditQuery = DBSS_Edit_Query;
    #endif
        
    //load the dll
    if (OpenFile("qelib.dll", &OFStruct, OF_EXIST) != HFILE_ERROR &&
        (hInstQE = LoadLibrary("qelib.dll")) > HINSTANCE_ERROR)
        {   
        //if dll was not previously loaded then resolve function addresses
        if (!hInstQEOld)
        {
        lpfnqeBeginTran = (qeSTATUS (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeBeginTran");
        lpfnqeColName = (qeLPSTR (WINAPI *)(qeHANDLE, short))GetProcAddress(hInstQE, "qeColName");
        lpfnqeColNameBuf = (qeSTATUS (WINAPI *)(qeHANDLE, qeLPSTR, short))GetProcAddress(hInstQE, "qeColNameBuf");
        lpfnqeColScale = (short (WINAPI *)(qeHANDLE, short))GetProcAddress(hInstQE, "qeColScale");
        lpfnqeColType = (short (WINAPI *)(qeHANDLE, short))GetProcAddress(hInstQE, "qeColType");
        lpfnqeColWidth = (long (WINAPI *)(qeHANDLE, short))GetProcAddress(hInstQE, "qeColWidth");
        lpfnqeCommit = (qeSTATUS (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeCommit");
        lpfnqeConnect = (qeHANDLE (WINAPI *)(qeLPSTR))GetProcAddress(hInstQE, "qeConnect");
        lpfnqeDisconnect = (qeSTATUS (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeDisconnect");
        lpfnqeErrMsg = (qeLPSTR (WINAPI *)(void))GetProcAddress(hInstQE, "qeErrMsg");
        lpfnqeEndSQL = (qeSTATUS (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeEndSQL");
        lpfnqeFetchNext = (qeSTATUS (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeFetchNext");
        lpfnqeFetchPrev = (qeSTATUS (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeFetchPrev");
        lpfnqeFetchRandom = (qeSTATUS (WINAPI *)(qeHANDLE hstmt, long))GetProcAddress(hInstQE, "qeFetchRandom");
        lpfnqeLibInit = (qeSTATUS (WINAPI *)(void))GetProcAddress(hInstQE, "qeLibInit");
        lpfnqeLibTerm = (qeSTATUS (WINAPI *)(void))GetProcAddress(hInstQE, "qeLibTerm");
        lpfnqeNumCols = (short (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeNumCols");
        lpfnqePutBinary = (qeSTATUS (WINAPI *)(qeHANDLE, short, LPSTR, long))GetProcAddress(hInstQE, "qePutBinary");
        lpfnqePutChar = (qeSTATUS (WINAPI *)(qeHANDLE, short, qeLPSTR, qeLPSTR))GetProcAddress(hInstQE, "qePutChar");
        lpfnqePutDouble = (qeSTATUS (WINAPI *)(qeHANDLE, short, double))GetProcAddress(hInstQE, "qePutDouble");
        lpfnqePutInt = (qeSTATUS (WINAPI *)(qeHANDLE, short, int))GetProcAddress(hInstQE, "qePutInt");
        lpfnqePutLong = (qeSTATUS (WINAPI *)(qeHANDLE, short, long))GetProcAddress(hInstQE, "qePutLong");
        lpfnqePutNull = (qeSTATUS (WINAPI *)(qeHANDLE, short))GetProcAddress(hInstQE, "qePutNull");
        lpfnqeQryAllocate = (qeHANDLE (WINAPI *)(qeHANDLE, qeLPSTR))GetProcAddress(hInstQE, "qeQryAllocate");
        lpfnqeQryBuilder = (qeSTATUS (WINAPI *)(qeHANDLE, short, short, short))GetProcAddress(hInstQE, "qeQryBuilder");
        lpfnqeQryGetStmtBuf = (qeSTATUS (WINAPI *)(qeHANDLE, qeLPSTR))GetProcAddress(hInstQE, "qeQryGetStmtBuf");
        lpfnqeQryFree = (qeSTATUS (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeQryFree");
        lpfnqeRecDelete = (qeSTATUS (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeRecDelete");
        lpfnqeRecLock = (qeSTATUS (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeRecLock");
        lpfnqeRecNew = (qeSTATUS (WINAPI *)(qeHANDLE hstmt, long))GetProcAddress(hInstQE, "qeRecNew");
        lpfnqeRecNum = (long (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeRecNum");
        lpfnqeRecState = (short (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeRecState");
        lpfnqeRecUpdate = (qeSTATUS (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeRecUpdate");
        lpfnqeRollback = (qeSTATUS (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeRollback");
        lpfnqeSetAutoUpdate = (qeSTATUS (WINAPI *)(qeHANDLE, short))GetProcAddress(hInstQE, "qeSetAutoUpdate");
        lpfnqeSetSelectOptions = (qeSTATUS (WINAPI *)(qeHANDLE, long))GetProcAddress(hInstQE, "qeSetSelectOptions");
        lpfnqeSetLockOptions = (qeSTATUS (WINAPI *)(qeHANDLE, short))GetProcAddress(hInstQE, "qeSetLockOptions");
        lpfnqeSQLPrepare = (qeHANDLE (WINAPI *)(qeHANDLE, qeLPSTR))GetProcAddress(hInstQE, "qeSQLPrepare");
        lpfnqeSQLExecute = (qeSTATUS (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeSQLExecute");
        lpfnqeValChar = (qeLPSTR (WINAPI *)(qeHANDLE, short, qeLPSTR, short))GetProcAddress(hInstQE, "qeValChar");
        lpfnqeValCharBuf = (qeSTATUS (WINAPI *)(qeHANDLE, qeLPSTR, short, qeLPSTR, short))GetProcAddress(hInstQE, "qeValCharBuf");
        lpfnqeValDouble = (double (WINAPI *)(qeHANDLE, short))GetProcAddress(hInstQE, "qeValDouble");
        lpfnqeValLong = (long (WINAPI *)(qeHANDLE, short))GetProcAddress(hInstQE, "qeValLong");
        lpfnqeCharToDateBuf = (qeSTATUS (WINAPI *)(qeLPSTR, qeLPSTR, qeLPSTR))GetProcAddress(hInstQE, "qeCharToDateBuf");
        lpfnqeDateToChar = (qeLPSTR (WINAPI *)(qeLPSTR, qeLPSTR))GetProcAddress(hInstQE, "qeDateToChar");
        lpfnqeErr = (qeSTATUS (WINAPI *)(void))GetProcAddress(hInstQE, "qeErr");
        lpfnqeDataLen = (long (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeDataLen");
        lpfnqeFetchNumRecs = (long (WINAPI *)(qeHANDLE))GetProcAddress(hInstQE, "qeFetchNumRecs");
        }
    //return successful dll load
        return (TRUE);
        }
    else
        {
        char    szTitle[36];
        char    szMsg[256];

        LoadString(hDynamicInst, IDS_ERR_NO_QE_DLL + LANGUAGE_BASE, szMsg, sizeof(szMsg)-1);
        LoadString(hDynamicInst, IDS_ERR_DLLERROR + LANGUAGE_BASE, szTitle, sizeof(szTitle)-1);
        MessageBox(NULL, szMsg, szTitle, MB_OK | MB_ICONEXCLAMATION);
        }
    //dll load failed
    hInstQE = 0;
    return (FALSE);
}

/********************************************************************/
void DLLENTRY xDB_FreeDLL()
{                    
    //if the dll was loaded then unload the dll
    if(hInstQE)
        FreeLibrary(hInstQE);    
        
    #if DB_SDK_DLL
    if(hInstSpread)
        FreeLibrary(hInstSpread);
    #endif
            
    hInstQE = 0;
    hInstSpread = 0;
}

/********************************************************************/
HANDLE DLLENTRY xDB_GetDLLHandle()
{   
    //get the handle to the loaded library
    return (hInstQE);
}

/********************************************************************/
int DLLENTRY Spread_DB_Process_Errors(BOOL Flag)
{   
int temp;

    //toggle automatic error processing on/off
    temp = ProcessErrors;
    ProcessErrors = Flag;
    return temp;
}

/********************************************************************/
void DLLENTRY Spread_DB_Disconnect(HCTL hCtl)
{                                
    lpfnSpreadDisconnect(hCtl);
}

/********************************************************************/
void DLLENTRY Spread_DB_Edit_Query(HCTL hCtl)
{                                
    lpfnSpreadEditQuery(hCtl);
}

/********************************************************************/
void DLLENTRY Spread_DB_Connect(HCTL hCtl, DBHANDLE ConnectHandle, DBHANDLE SelectHandle)
{
    if(Connetion_Count == 0)        //if the first connection to the engine
        xDB_LoadDLL();

    if (hInstQE)
       {
       (*lpfnSpreadConnect)(hCtl,ConnectHandle, SelectHandle);
       ++Connetion_Count;
       }
}

/********************************************************************/
void DLLENTRY xDB_Error()
{
int   ret;
LPSTR p;
char  szTitle[36];
                  
    //if process error flag is off then do nothing
    if(!ProcessErrors)
        return;

    //get the error string
    p = (LPSTR)(*lpfnqeErrMsg)(); 
    
    //display the error
    LoadString(hDynamicInst, IDS_ERR_ERROR + LANGUAGE_BASE, szTitle, sizeof(szTitle)-1);
    ret = MessageBox(NULL,p,szTitle,MB_OK);
}

/********************************************************************/
int DLLENTRY xDB_SDK_Begin_Trans(DBHANDLE hdb)
{
int ret;

    if(hdb)
        return FALSE;
    
    //begin a SQL transaction
    ret = (*lpfnqeBeginTran)((int)hdb);
    
    //if an error occurred then process it
    if(ret)
        xDB_Error();                       
        
    //return the result of the BeginTran function
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Commit(DBHANDLE hdb)
{
int ret;

    if(hdb)
        return FALSE;

    //if no connection exists
    if(!hdb)
        return FALSE;
    
    //commit the SQL transaction
    ret = (*lpfnqeCommit)((int)hdb); 
    
    //check if an error occurred
    if(ret)
        xDB_Error();            
        
    //return the commit result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Connect(char far *Connection)
{
int hdb = 0;
char NewConnect[256];
     
    if(Connetion_Count == 0)        //if the first connection to the engine
        if(xDB_LoadDLL())           //load the DLL
            if((*lpfnqeLibInit)())  //initialize the engine
                xDB_Error();        //process an errors

    if (hInstQE)
       {
       //increment the number of Q+E connections
       ++Connetion_Count;

       //connecto to the database
       hdb = (*lpfnqeConnect)(Connection);

       //if the connection failed
       if(hdb == 0)
           {
           //reconnect and prompt user for addition information
           lstrcpy(NewConnect,Connection);
           lstrcat(NewConnect,";DLG=1");
           hdb = (*lpfnqeConnect)(NewConnect);
           if(!hdb)
               {
               xDB_Error();
               return 0;
               }
           }

       //issue warnning if record being edited had been changed by a user
   //    if((*lpfnqeSetLockOptions)(hdb, qeLOCK_COMPARE))
   //      xDB_Error();

       //write all changes back to the database
       if((*lpfnqeSetAutoUpdate)(hdb, qeAUTOUPD_UPDATE))
           xDB_Error();

       //define random seek cursors
       if((*lpfnqeSetSelectOptions)(hdb, qeFETCH_ANY_DIR))
           xDB_Error();
       }

    //set the return database handle
    return hdb;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Disconnect_Clone(DBHANDLE hdb)
{
int ret;
        
    //if no connection
    if(Connetion_Count == 0)
        return 0;
        
    //decrement the number of Q+E database connections
    --Connetion_Count;
    
    //if no more connections then unload the DLL
    if(Connetion_Count == 0)
        {
        xDB_FreeDLL();      //unload the Q+E DLL
        hInstQE = 0;        //reset Q+E DLL libaray handle
        }

    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Disconnect(DBHANDLE hdb)
{
int ret;
        
    //if no connection
    if(Connetion_Count == 0)
        return 0;
        
    //disconnect from the database
    ret = (*lpfnqeDisconnect)((int)hdb);
    if(ret)
        xDB_Error();

    //decrement the number of Q+E database connections
    --Connetion_Count;
    
    //if no more connections then unload the DLL
    if(Connetion_Count == 0)
        {
        ret = (*lpfnqeLibTerm)(); //shut down the Q+E engine
        if(ret)
            xDB_Error();
        xDB_FreeDLL();      //unload the Q+E DLL
        hInstQE = 0;        //reset Q+E DLL libaray handle
        }

    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Delete_Record(DBHANDLE hstmt)
{
int ret;

    //delete the current record
    ret = (*lpfnqeRecDelete)((int)hstmt);
    
    //check if any errors occurred
    if(ret)
        xDB_Error();              
        
    //return the result of deleting the record
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_End_Bind(DBHANDLE hstmt)
{
    //if there are not connection to begin with
    if(Connetion_Count == 0)
        return 0;

    if(hstmt)                       //if there is an SQL statement
        if((*lpfnqeEndSQL)((int)hstmt))  //end the SQL statement
            xDB_Error();            //process any errors

    return 1;                       //return success
}

/********************************************************************/
DBHANDLE DLLENTRY xDB_SDK_Execute_SQL(DBHANDLE hdb,DBHANDLE hstmt, char far *s)
{
    //if a SQL statement already exists the kill it
    if(hstmt)
        if((*lpfnqeEndSQL)((int)hstmt))
            xDB_Error();

    //prepare the SQL statement
    hstmt = (*lpfnqeSQLPrepare)((int)hdb, s);
    
    //execute the SQL statement
    if((*lpfnqeSQLExecute)((int)hstmt))
        {
        xDB_Error();
        hstmt = 0;
        }

    //return the statement handle
    return hstmt;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Get_Cell_Type(DBHANDLE hstmt,int Col,int far *Precision, int far *Field_Length)
{
    *Precision = 0;
    *Field_Length = 0;

    //map Q+E field types into Access cell types
    switch((*lpfnqeColType)((int)hstmt, Col))
        {
        case qeCHAR:
            *Field_Length = (int)(*lpfnqeColWidth)((int)hstmt, Col);
            return (DATA_VT_TEXT);

        case qeVARCHAR:
            *Field_Length = (int)(*lpfnqeColWidth)((int)hstmt, Col);
            return (DATA_VT_TEXT);

        case qeLONGVARCHAR:
            *Field_Length = (int)(*lpfnqeColWidth)((int)hstmt, Col);
            return (DATA_VT_MEMO);

        case qeINTEGER:
        case qeNUMERIC:
        case qeBIGINT:
            return (DATA_VT_LONG);

        case qeSMALLINT:
        case qeTINYINT:
            return (DATA_VT_INTEGER);

        case qeDECIMAL:
        case qeFLOAT:
            *Precision = (*lpfnqeColScale)((int)hstmt, Col);
            return (DATA_VT_SINGLE);

        case qeDOUBLEPRECISION:
            *Precision = (*lpfnqeColScale)((int)hstmt, Col);
            return (DATA_VT_DOUBLE);

        case qeMONEY:
            *Precision = (*lpfnqeColScale)((int)hstmt, Col);
            return (DATA_VT_CURRENCY);

        case qeDATETIME:
        case qeDATE:
        case qeTIME:
            return (DATA_VT_DATETIME);

        case qeBIT:
            return (DATA_VT_BOOL);

        case qeBINARY:
        case qeVARBINARY:
        case qeLONGVARBINARY:
            return (DATA_VT_BINARY);

        default:
            return (DATA_VT_TEXT);
        }
    
    return 1;
}

/********************************************************************/
LPSTR DLLENTRY xDB_SDK_Get_Char_Val_For_Field(DBHANDLE hstmt, char far *Field, int Col, char far *Formatting, int Length)
{
    LPSTR lpszText;
      
    //if a data buffer is supplied to hold the 
    if(Field)
       {
       //get the data into the supplied buffer
       if((*lpfnqeValCharBuf)((int)hstmt, Field, Col, Formatting, Length))
            return (NULL);
       else
            return (Field);
       }
    else
       {
       //get a pointer to the data
       lpszText = (LPSTR)(*lpfnqeValChar)((int)hstmt, Col, Formatting, Length);
       
       //if there is data then trim off the trailing spaces
       if(lpszText)
            DB_StrRTrim(lpszText);
       return (lpszText);
       }
}

/********************************************************************/
int DLLENTRY xDB_SDK_Get_Double_Val_For_Field(DBHANDLE hstmt, int Col, LPDOUBLE lpdfVal)
{
    *lpdfVal = (*lpfnqeValDouble)((int)hstmt, Col);
    return (0);
}

/********************************************************************/
long DLLENTRY xDB_SDK_Get_Long_Val_For_Field(DBHANDLE hstmt, int Col)
{
    return ((long)(*lpfnqeValLong)((int)hstmt, Col));
}

/********************************************************************/
int DLLENTRY xDB_SDK_Get_Date(DBHANDLE hstmt, int Col, LPSTR lpszDate, LPSTR lpszTime)
{
LPSTR lpszTemp;
int   ret = 0;

    //get the date field
    if(lpszTemp = (LPSTR)(*lpfnqeValChar)((int)hstmt, Col, "YYYY-MM-DD HH:MM:SS.SSSSSS", 0))
        if ((*lpfnqeErr)())       
            lpszTemp = NULL;

    //if a date buffer was supplied
    if(lpszDate)
        {
        //if there was no date returned
        if (!lpszTemp || !_fstrncmp(lpszTemp, "0000", 4))
            *lpszDate = '\0';
        else
            {
            //extract the returned date
            _fmemcpy(&lpszDate[0], &lpszTemp[5], 2);
            _fmemcpy(&lpszDate[2], &lpszTemp[8], 2);
            _fmemcpy(&lpszDate[4], &lpszTemp[0], 4);
            lpszDate[8] = '\0';
            }
        }

    //if a time buffer was supplied
    if(lpszTime)
        {                          
        //if there was no time returned
        if (!lpszTemp || !_fstrncmp(&lpszTemp[11], "00:00:00.000000", 15))
            *lpszTime = '\0';
        else
            {                          
            //extract the returned time
            _fmemcpy(&lpszTime[0], &lpszTemp[11], 2);
            _fmemcpy(&lpszTime[2], &lpszTemp[14], 2);
            _fmemcpy(&lpszTime[4], &lpszTemp[17], 2);
            lpszTime[6] = '\0';
            }
        }

    return ret;
}

/********************************************************************/
HGLOBAL DLLENTRY xDB_SDK_Get_Binary(DBHANDLE hstmt, int Col, LPLONG lplLen)
{
HGLOBAL        hData = 0;
LPSTR          lpData;
HPSTR          hpDataTemp;
long           lLen = 0;
long           lLenTotal = 0;
BOOL           fMore = TRUE;

    //while getting BLOB in multiple 64K chunks
    while (fMore)
        {
        //set flag for not being done
        fMore = FALSE;

        //get the binary data
        if(lpData = (LPSTR)(*lpfnqeValChar)((int)hstmt, Col, NULL, 0))
            {                
            //get the length of the data
            lLen = (*lpfnqeDataLen)((int)hstmt);

            //if there is no data area allocated then allocate space
            if (!hData)
                {
                if (!(hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, lLen)))
                    return ((HGLOBAL)0);
                }
            //reallocate the data buffer to append addition data
            else
                {       
                if (!(hData = GlobalReAlloc(hData, lLenTotal + lLen, GMEM_MOVEABLE | GMEM_ZEROINIT)))
                    return ((HGLOBAL)0);
                }

            //append the data to the global data buffer         
            hpDataTemp = (HPSTR)GlobalLock(hData);
            DB_MemHugeCpy(&hpDataTemp[lLenTotal], lpData, lLen);
            GlobalUnlock(hData);

            //adjust the recorded length of the buffer
            lLenTotal += lLen;

            //check if we got all the data
            if((*lpfnqeErr)() == qeTRUNCATION)
                fMore = TRUE;
            }
        }

    //return the length of the data
    *lplLen = lLenTotal;

    //return the handle to the global data
    return (hData);
}

/********************************************************************/
int DLLENTRY xDB_SDK_Get_Current_Record_Number(DBHANDLE hstmt)
{
int ret;

    //get current record number
    ret = (int)(*lpfnqeRecNum)((int)hstmt);
    
    //if an errors occurred
    if(ret < 1)
        xDB_Error();       
        
    //return the result
    return ret;
}

/********************************************************************/
LPSTR DLLENTRY xDB_SDK_Get_Field_Name(DBHANDLE hstmt, char far *Field_Name, int Col)
{
    //if a buffer is supplied to hold the field name
    if(Field_Name)
        {
        //get the field name and place it in the supplied buffer
        if ((*lpfnqeColNameBuf)((int)hstmt, Field_Name, Col))
            return (NULL);
        else
            return (Field_Name);
        }
    else
        //return a pointer to the field name
        return ((LPSTR)(*lpfnqeColName)((int)hstmt, Col));
}

/********************************************************************/
int DLLENTRY xDB_SDK_Get_Field_Size(DBHANDLE hstmt, int Col)
{
int ret;
                       
    //get the length of a field in bytes
    ret = (int)(*lpfnqeColWidth)((int)hstmt, Col);
    
    //process any errors
    if(ret < 1)
        xDB_Error();    
        
    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Get_Next_Record(DBHANDLE hstmt)
{
int ret;

    //get the next record in the query
    ret = (*lpfnqeFetchNext)((int)hstmt);
    
    //process any errors
    if(ret && ret != qeEOF)
        xDB_Error();    
        
    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Get_Num_Of_Cols(DBHANDLE hstmt)
{                      
    //get the number of columns in the query
    return (*lpfnqeNumCols)((int)hstmt);
}

/********************************************************************/
int DLLENTRY xDB_SDK_Get_Previous_Record(DBHANDLE hstmt)
{
int ret;

    //get the previous record in the query
    ret = (*lpfnqeFetchPrev)((int)hstmt);
    
    //process any errors
    if(ret)
        xDB_Error();
    
    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Get_Random_Record(DBHANDLE hstmt, long num)
{
int ret;

    //position the current record on the specified record number
    ret = (*lpfnqeFetchRandom)((int)hstmt, num);                     
    
    //process any errors
    if(ret && ret != qeEOF)
        xDB_Error();       
        
    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_New_Record (DBHANDLE hstmt, long row)
{
int ret;

    //create a new record
    ret = (*lpfnqeRecNew)((int)hstmt, row);
    
    //process an errors
    if(ret)
        xDB_Error();   
        
    //return the result
    return ret;
}

/********************************************************************/
long DLLENTRY xDB_SDK_Get_Num_Recs (DBHANDLE hstmt)
{
    return (*lpfnqeFetchNumRecs)((int)hstmt);
}

/********************************************************************/
void DLLENTRY xDB_SDK_Prompt_For_Query(DBHANDLE hdb, char far *Query)
{
int hqry, style;

    //allocate a query
    hqry = (*lpfnqeQryAllocate)((int)hdb, Query);
    
    //process any errors
    if(hqry == 0)
        {
        xDB_Error();
        return;
        }

    //prompt for query
    style = qeQRY_BIG_ICONS | qeQRY_TABLES | qeQRY_VIEWS | qeQRY_ALLOW_SRC_CHANGE;
    if((*lpfnqeQryBuilder)(hqry, 0, style, qeQRY_FILE))
        {
        xDB_Error();
        return;
        }

    //get query string
    if((*lpfnqeQryGetStmtBuf)(hqry, Query)) 
        {
        xDB_Error();
        return;
        }
    
    if((*lpfnqeQryFree)(hqry)) 
        xDB_Error();
}

/********************************************************************/
int DLLENTRY xDB_SDK_RollBack(DBHANDLE hdb)
{
int ret;

    if(hdb)
        return FALSE;
    
    //rollback a SQL transaction
    ret = (*lpfnqeRollback)((int)hdb);
    
    //process an errors
    if(ret)
        xDB_Error();   
        
    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Set_Field(DBHANDLE hstmt, int Col, char far *Formatting, char far *Field_Data)
{
int ret;
                       
    //set the value of a field (generic version)  
    ret = (*lpfnqePutChar)((int)hstmt, Col, Formatting, Field_Data);
    
    //process an errors
    if(ret)
        xDB_Error();   
        
    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Set_Binary(DBHANDLE hstmt, int Col, LPSTR lpVal, long lLen)
{
int ret;

    //set a binary field value
    ret = (*lpfnqePutBinary)((int)hstmt, Col, lpVal, lLen); 
    
    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Set_Double(DBHANDLE hstmt, int Col, double dfVal)
{
int ret;
                       
    //set a double field value
    ret = (*lpfnqePutDouble)((int)hstmt, Col, dfVal); 
    
    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Set_Int(DBHANDLE hstmt, int Col, int iVal)
{
int ret;
                       
    //set a long field value
    ret = (*lpfnqePutInt)((int)hstmt, Col, iVal);
    
    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Set_Long(DBHANDLE hstmt, int Col, long lVal)
{
int ret;
                       
    //set a long field value
    ret = (*lpfnqePutLong)((int)hstmt, Col, lVal); 
    
    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Set_Null(DBHANDLE hstmt, int Col)
{
int ret;
    
    //set a field value to NULL
    ret = (*lpfnqePutNull)((int)hstmt, Col);
    
    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Set_Date(DBHANDLE hstmt, int Col, LPSTR lpszDate, LPSTR lpszTime)
{
LPSTR lpszTemp;
char  szDate[10 + 1] = {0};
char  szTime[15 + 1] = {0};
char  szDateTime[26 + 1];
int   ret = 0;

    if (lpszTemp = (LPSTR)(*lpfnqeValChar)((int)hstmt, Col,"YYYY-MM-DD HH:MM:SS.SSSSSS", 0))
        if ((*lpfnqeErr)())
            lpszTemp = NULL;

    /**************
    * Format Date
    **************/
    if (!lpszDate && lpszTemp)
       {
       _fmemcpy(szDate, lpszTemp, 10);
       szDate[10] = '\0';
       }
    else if (lpszDate && *lpszDate)
       {
       _fmemcpy(&szDate[0], &lpszDate[4], 4);
       _fmemcpy(&szDate[4], "-", 1);
       _fmemcpy(&szDate[5], &lpszDate[0], 2);
       _fmemcpy(&szDate[7], "-", 1);
       _fmemcpy(&szDate[8], &lpszDate[2], 2);
       szDate[10] = '\0';
       }
    else
       lstrcpy(szDate, "0000-00-00");

    /**************
    * Format Time
    **************/
    if (!lpszTime && lpszTemp)
       {
       _fmemcpy(szTime, &lpszTemp[11], 15);
       szTime[15] = '\0';
       }
    else if (lpszTime && *lpszTime)
       {
       _fmemcpy(&szTime[0], &lpszTime[0], 2);
       _fmemcpy(&szTime[2], ":", 1);
       _fmemcpy(&szTime[3], &lpszTime[2], 2);
       _fmemcpy(&szTime[5], ":", 1);
       _fmemcpy(&szTime[6], &lpszTime[4], 2);
       _fmemcpy(&szTime[8], ".000000", 8);
       }
    else
       lstrcpy(szTime, "00:00:00.000000");

    lstrcpy(szDateTime, szDate);
    lstrcat(szDateTime, " ");
    lstrcat(szDateTime, szTime);

    ret = (*lpfnqePutChar)((int)hstmt, Col, "YYYY-MM-DD HH:MM:SS.SSSSSS",szDateTime);

    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Is_Rec_Mod_Or_Added(DBHANDLE hstmt)
{
int ret;
                       
    //get the current update/add state of the current record
    ret = (*lpfnqeRecState)((int)hstmt);                         
    
    //if the record was updated/added then return TRUE
    if((ret == 1) || (ret == 3))
        return TRUE;                                  
        
    //if not then return FALSE
    return FALSE;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Update_Record(DBHANDLE hstmt)
{
int ret;

    //update the current record
    ret = (*lpfnqeRecUpdate)((int)hstmt);
    
    //process an errors
    if(ret)
        xDB_Error();   
        
    //return the result
    return ret;
}

/********************************************************************/
int DLLENTRY xDB_SDK_Lock_Rec(DBHANDLE hstmt)
{ 
    if(hstmt)
        return FALSE;

    //return the result of locking the current record
    return ((*lpfnqeRecLock)((int)hstmt));
}
