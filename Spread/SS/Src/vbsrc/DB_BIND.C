//#include <windows.h>      
//#include <string.h>
//#include <toolbox.h>
//#include <stdlib.h>
//#include <vbapi.h>
#include <windows.h>
#include <vbapi.h>
#include <math.h>
#include <string.h>
#include "..\sssrc\spread\spread.h"
#include "..\sssrc\spread\ss_alloc.h"
#include "..\sssrc\spread\ss_type.h"
#include "..\sssrc\spread\ss_user.h"
#include "vbspread.h"
#include "qelib.h"
//#undef SS_BDM_ALWAYS
#include "db_sdk.h"
#include "db_bind.h"
#include "..\stringrc.h"

extern HANDLE hDynamicInst;

#ifdef SS_QE

#define malloc(Size)   (LPSTR)tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (long)Size)
#define free(lpString) tbGlobalFree(lpString)

int First_Read;

WORD                    _DBSS_GetBound(LPVBSPREAD lpSpread, WORD x);
void                    _DBSS_SetCellTypes(HWND hWndSS, LPVBSPREAD lpSpread,
                                           LPDBINFO lpDBInfo);
void                    _DBSS_SetFieldNames(HWND hWndSS, LPDBINFO lpDBInfo);
void                    _DBSS_SetColWidth(HWND hWndSS, LPSPREADSHEET LPSS,
                                          LPVBSPREAD lpSpread,
                                          LPDBINFO lpDBInfo);
void                    _DBSS_SetCurrentRow(LPDBINFO lpDBInfo, long Row);
BOOL FAR PASCAL _export _DBSS_ConnectDlgProc(HWND hDlg, USHORT Msg,
                                             WPARAM wParam, LPARAM lParam);
TBGLOBALHANDLE          _DBSS_BuildFieldList(LPSPREADSHEET lpSS,
                                             short Num_Cols);
BOOL                    _DBSS_SaveRow(HCTL hCtl, HWND hWnd,
                                      LPVBSPREAD lpSpread, LPSPREADSHEET lpSS,
                                      SS_COORD Row);
BOOL                    _DBSS_SaveRow2(HCTL hCtl, HWND hWnd,
                                       LPVBSPREAD lpSpread, LPSPREADSHEET lpSS,
                                       SS_COORD Row, SS_COORD Col,
                                       LPSS_COL lpCol, BYTE bRowDirty);
void                    _DBSS_SSToDB(HWND hWnd, LPVBSPREAD lpSpread,
                                     LPSPREADSHEET lpSS,
                                     LPSS_CELLTYPE lpCellType, short dFieldNum,
                                     SS_COORD Col, SS_COORD Row,
                                     USHORT usDataType);
void                    _DBSS_DBToSS(HWND hWnd, LPVBSPREAD lpSpread,
                                     LPSPREADSHEET lpSS, short dFieldNum,
                                     SS_COORD Col, SS_COORD Row,
                                     USHORT usDataType);

extern TBGLOBALHANDLE DLLENTRY SS_GetValue(LPSPREADSHEET lpSS,
                                           SS_COORD Col, SS_COORD Row);


/********************************************************************/
long DBSS_GetRecs (HWND hWndSS, LPVBSPREAD lpSpread, LPDBINFO lpDBInfo,
                   long lStart, long lNumRecsNeeded, LPBOOL lpfAtTop,
                   LPBOOL lpfAtBottom)
{
LPSPREADSHEET  lpSS;
TBGLOBALHANDLE hFieldList;
LPSHORT        lpdFieldList;
HCTL           hCtl = VBGetHwndControl(hWndSS);
SS_COORD       Row;
int            x;
int            Total_Rec_Count;
int            Temp_Rec_Count;
int            Num_Cols;
short          Precision;
short          ColSize;
long           lRowsLoaded;
BOOL           fRedraw;
TBGLOBALHANDLE hDatabaseFieldType;
LPSHORT        lpdDatabaseFieldType;
TBGLOBALHANDLE hDataFillEvent;
LPSHORT        lpdDataFillEvent;
LPSS_COL       lpCol;

    //if no connection then return
    if (!lpDBInfo->fConnected)
       return (0);

    //get the number of columns in the query
    Num_Cols = xDB_SDK_Get_Num_Of_Cols(lpDBInfo->hStmt);
    
    //set flag if at top of the file
    if (lStart == 1)
        *lpfAtTop = TRUE;
    else
        *lpfAtTop = FALSE;

    //position on top of the first rec being requested
    x = xDB_SDK_Get_Random_Record(lpDBInfo->hStmt, lStart);
    
    //if no records to read or error occurred then exit
    if (x)
        return (0);

    //set number of recs read so far
    Total_Rec_Count = 0;
    Temp_Rec_Count = 0;

    //get a pointer to the spreadsheet structure
    lpSS = SS_Lock(hWndSS);

    //get an array the maps the database fields into the spreadsheet columns
    hFieldList = _DBSS_BuildFieldList(lpSS, Num_Cols);
    lpdFieldList = (LPSHORT)tbGlobalLock(hFieldList);

    //create and initialize an array of database field types
    if (hDatabaseFieldType = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, Num_Cols * sizeof(short)))
        {
        lpdDatabaseFieldType = (LPSHORT)tbGlobalLock(hDatabaseFieldType);

        for (x = 1; x <= Num_Cols; x++)
            lpdDatabaseFieldType[x-1] = xDB_SDK_Get_Cell_Type(lpSpread->DBInfo.hStmt, x, &Precision, &ColSize);
        }
                                                            
    //create and initialize an array of datafill event flags                                                            
    if (hDataFillEvent = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, Num_Cols * sizeof(short)))
        {
        lpdDataFillEvent = (LPSHORT)tbGlobalLock(hDataFillEvent);

        for (x = 1; x <= Num_Cols; x++)
            if (lpCol = SS_LockColItem(lpSS, lpdFieldList[x - 1]))
                {
                lpdDataFillEvent[x-1] = lpCol->fDataFillEvent;
                SS_UnlockColItem(lpSS, lpdFieldList[x - 1]);
                }
        }

    //save the spreadsheet redraw flag
    fRedraw = lpSS->Redraw;           
    
    //turn the spreadsheet redraw flag off
    lpSS->Redraw = FALSE;

    //set number of record read so far to zero
    lRowsLoaded = 0;

    //get the specified number of records records
    for (Row = lStart; Row < lStart + lNumRecsNeeded; Row++)
        {
    
        //increment the number of rows actually read
        lRowsLoaded++;

        //get a record: field by field
        for(x = 1; x <= Num_Cols; x++)
            {
            //if there is a field
            if (lpdFieldList[x - 1])
               {
               //if the datafill event is true for that column
               if (lpdDataFillEvent[x-1])
                  {                                           
                  //then fire the data fill event, continue if CANCEL was returned TRUE
                  if (VBSSBoundFireDataFill((LONG)hCtl, lpSpread,
                                            lpdFieldList[x - 1], Row, TRUE,
                                            lpdDatabaseFieldType[x-1], Row, x))
                     continue;
                  }

               //get the field
               _DBSS_DBToSS(hWndSS, lpSpread, lpSS, x, lpdFieldList[x - 1], Row, lpdDatabaseFieldType[x-1]);
               }
            }
        
        //get the next record, if at end of file then exit loop
        if(xDB_SDK_Get_Next_Record(lpDBInfo->hStmt) == qeEOF)
            {
            *lpfAtBottom = TRUE;
            break;
            }
        }//end for loop

    //if this is the first read for the spreadsheet
//    if(First_Read)
        {
        //turn flag off
        First_Read = FALSE;
        
        //set spreadsheet column widths
        _DBSS_SetColWidth(hWndSS, lpSS, lpSpread, lpDBInfo);
        
        //reposition on the first record read
//        ret = xDB_SDK_Get_Random_Record(lpDBInfo->hStmt, 1);
        }

    VBSSBoundFinishClipData(hWndSS, lpSS, lStart, lStart + lRowsLoaded - 1, Num_Cols);
//    SSSetBool(hWndSS, SSB_REDRAW, fRedraw);
    lpSS->Redraw = fRedraw;

    //free the list of field descriptions
    tbGlobalUnlock(hFieldList);
    tbGlobalFree(hFieldList);
                                   
    //free the database field type array                                   
    tbGlobalUnlock(hDatabaseFieldType);
    tbGlobalFree(hDatabaseFieldType);

    //free the database data fill event array                                   
    tbGlobalUnlock(hDataFillEvent);
    tbGlobalFree(hDataFillEvent);

    //release the pointer to the spreadsheet structure
    SS_Unlock(hWndSS);

    //return the number of records read
    return (lRowsLoaded);
}

/********************************************************************/
int DBSS_BindDatabase(HWND hWndSS, LPVBSPREAD lpSpread, LPDBINFO lpDBInfo,
                      LPSTR connection, LPSTR Query)
{
LPSPREADSHEET  lpSS;
BOOL           fRedraw;
int            ret, x, y;
                    
    //if already connected then end previous transactions
    if (lpSpread->DBInfo.fConnected)
        ret = xDB_SDK_Commit(lpDBInfo->hDB);
    else
        //attach to the database
        if (!(lpDBInfo->hDB = xDB_SDK_Connect(connection)))
            {
            lpDBInfo->hDB = 0;
            xDB_FreeDLL();
            return FALSE;
            }

   //begin new transaction
   ret = xDB_SDK_Begin_Trans(lpDBInfo->hDB);

   //execute the query
    if (!Query || lstrcmp(Query," "))
        if (!(lpDBInfo->hStmt = xDB_SDK_Execute_SQL(lpDBInfo->hDB, lpDBInfo->hStmt, Query)))
            return FALSE;

    lpDBInfo->fConnected = TRUE;

    lpSS = SS_Lock(hWndSS);
    //save the spreadsheet redraw flag
    fRedraw = lpSS->Redraw;           
    
    //turn the spreadsheet redraw flag off
    lpSS->Redraw = FALSE;

    _DBSS_SetFieldNames(hWndSS, lpDBInfo);

    //set spreadsheet cell types
    _DBSS_SetCellTypes(hWndSS, lpSpread, lpDBInfo);

    //if virtual mode is off then read in all the records in the database
    if(!SSGetBool(hWndSS, SSB_VIRTUALMODE) && _DBSS_GetBound(lpSpread, SSD_AUTOFILL))
       ret = (int)DBSS_GetRecs(hWndSS, lpSpread, lpDBInfo, 1, 9999999,
                               (int far *)&x, (int far *)&y);

    lpSS->Redraw = fRedraw;
    SS_Unlock(hWndSS);
    return TRUE;
}


/********************************************************************/
void DBSS_EditQuery(HWND hWndSS, LPVBSPREAD lpSpread, LPDBINFO lpDBInfo)
{
char far *Selection;
int  x, Num, ret;

    //end previous transaction
    ret = xDB_SDK_Commit(lpDBInfo->hDB);
        
    //get the number of columns in the current query
    Num = xDB_SDK_Get_Num_Of_Cols(lpDBInfo->hStmt);

    if (DBSS_PromptQueryString(hWndSS, lpDBInfo))
       {
       //turn the spreadsheet's redrawing off
       ret = SSSetBool(hWndSS,SSB_REDRAW,FALSE);

       //clear out the previous query's data
       for(x=1; x<Num; x++)
           ret = SSClear(hWndSS,x,SS_ALLROWS);

       //execute the query
       lpDBInfo->hStmt = xDB_SDK_Execute_SQL(lpDBInfo->hDB, lpDBInfo->hStmt, Selection);

       //set spreadsheet cell types
       _DBSS_SetCellTypes(hWndSS, lpSpread, lpDBInfo);

       //set flag so first time reac are read in, the col widths can be set
       First_Read = TRUE;

       //if virtual mode is already on
       if(SSGetBool(hWndSS,SSB_VIRTUALMODE))
           {
           x = TRUE;
           ret = SSSetBool(hWndSS,SSB_VIRTUALMODE,FALSE);
           SSSetMaxRows(hWndSS, (long)500);
           }
       else
           x = FALSE;

       //set the active cell to the top left corner
       ret = SSSetActiveCell(hWndSS, 1, 1);

       //if virtual mode was on
       if(x)
           {
           Yield();
           //turn virtual mode back on
           ret = SSSetBool(hWndSS,SSB_VIRTUALMODE,TRUE);
           }

       //turn the spreadsheet's redrawing back on
       ret = SSSetBool(hWndSS,SSB_REDRAW, TRUE);
       }
}


/********************************************************************/
void DBSS_EndBind(HWND hWndSS, LPDBINFO lpDBInfo)
{
short ret, readonly;
LPSTR p;

#if 0
    //get the update state ofthe current record
    state = xDB_SDK_Is_Rec_Mod_Or_Added(lpDBInfo->hStmt);
    
    //if the record changed
    if(state)
        {
        //save the record
        ret = xDB_SDK_Update_Record(lpDBInfo->hStmt);
        
        //end previous transaction
        ret = xDB_SDK_Commit(lpDBInfo->hDB);
        }
    else
        {
        //end previous transaction
        ret = xDB_SDK_RollBack(lpDBInfo->hDB);
        }
#endif
        
    //disconnect from the database

    p = (LPSTR)tbGlobalLock(lpDBInfo->hDataConnect);
    if(lstrcmp(p, " "))
        readonly = FALSE;
    else
        readonly = TRUE;
    tbGlobalUnlock(lpDBInfo->hDataConnect);                            
    ret = xDB_SDK_Commit(lpDBInfo->hDB);
    if (lpDBInfo->fConnected && !readonly)
       {
       ret = xDB_SDK_End_Bind(lpDBInfo->hStmt);
       ret = xDB_SDK_Disconnect(lpDBInfo->hDB);
       }                   
    else
        ret = xDB_SDK_Disconnect_Clone(lpDBInfo->hDB);


    lpDBInfo->hStmt = 0;
    lpDBInfo->hDB = 0;
    lpDBInfo->fConnected = FALSE;
}


/********************************************************************/
void DBSS_InsertRecord(LPDBINFO lpDBInfo, long Row)
{
int ret;

//insert a blank record into the database
ret = xDB_SDK_New_Record(lpDBInfo->hStmt, Row);
}


BOOL DBSS_PromptQueryString(HWND hWndSS, LPDBINFO lpDBInfo)
{
LPVBSPREAD     lpSpread;
TBGLOBALHANDLE hQuery;
LPSTR          lpszQuery;
LPSTR          lpszDataSelect;
LPSTR          lpszDataConnect;
short          dLen;
DBHANDLE       hDB = 0;

lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(hWndSS));

if (!lpSpread->DBInfo.fConnected)
   {
   if (lpSpread->DBInfo.hDataConnect)
      {
      lpszDataConnect = (LPSTR)tbGlobalLock(lpSpread->DBInfo.hDataConnect);

      //attach to the database
      hDB = xDB_SDK_Connect(lpszDataConnect);

      tbGlobalUnlock(lpSpread->DBInfo.hDataConnect);
      }
   }
else
   hDB = lpDBInfo->hDB;

if (!hDB)
   return (FALSE);

// allocate the select buffer
if (hQuery = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 5000))
   {
   lpszQuery = (LPSTR)tbGlobalLock(hQuery);

   if (lpSpread->DBInfo.hDataSelect)
      {
      lpszDataSelect = (LPSTR)tbGlobalLock(lpSpread->DBInfo.hDataSelect);
      lstrcpy(lpszQuery, lpszDataSelect);
      tbGlobalUnlock(lpSpread->DBInfo.hDataSelect);
      }

   //prompt for query
   xDB_SDK_Prompt_For_Query(hDB, lpszQuery);

   dLen = lstrlen(lpszQuery);
   tbGlobalUnlock(hQuery);

   if (dLen)
      hQuery = tbGlobalReAlloc(hQuery, dLen + 1,
                               GMEM_MOVEABLE | GMEM_ZEROINIT);

   else                                // The user pressed cancel
      {
      tbGlobalFree(hQuery);
      hQuery = 0;
      }
   }

if (hQuery)
   {
   if (lpSpread->DBInfo.hDataSelect)
      tbGlobalFree(lpSpread->DBInfo.hDataSelect);

   lpSpread->DBInfo.hDataSelect = hQuery;
   }

if (!lpSpread->DBInfo.fConnected)
   xDB_SDK_Disconnect(hDB);

return (hQuery ? TRUE : FALSE);
}


BOOL DBSS_ManualSave(HCTL hCtl, HWND hWnd, LPVBSPREAD lpSpread,
                     LPSPREADSHEET lpSS, SS_COORD Top, SS_COORD End)
{
LPSS_ROW lpRow;
SS_COORD i;
long     j;
long     lRowCurrent;
BOOL     fInsertRest = FALSE;

for (i = Top; i <= End; i++)
   if (lpRow = SS_LockRowItem(lpSS, i))
      {
      if (lpRow->bDirty == SS_ROWDIRTY_DEL)
         {
         if (!xDB_SDK_Get_Random_Record(lpSpread->DBInfo.hStmt, i))
            {
            if (xDB_SDK_Delete_Record(lpSpread->DBInfo.hStmt))
               break;

            SSDelRowRange(hWnd, i, i);
            i--;
            End--;
            }
         }

      else
         {
         if (fInsertRest)
            DBSS_InsertRecord(&lpSpread->DBInfo, i);

         else if (xDB_SDK_Get_Random_Record(lpSpread->DBInfo.hStmt, i))
            {
//            xDB_SDK_Get_Previous_Record(lpSpread->DBInfo.hStmt);

            lRowCurrent = xDB_SDK_Get_Num_Recs(lpSpread->DBInfo.hStmt);

            for (j = lRowCurrent + 1; j <= i; j++)
               DBSS_InsertRecord(&lpSpread->DBInfo, j);

            fInsertRest = TRUE;
            }

         else if (lpRow->bDirty == SS_ROWDIRTY_INS)
            DBSS_InsertRecord(&lpSpread->DBInfo, i);

         // Save Record

         _DBSS_SaveRow(hCtl, hWnd, lpSpread, lpSS, i);
         }

      SS_UnlockRowItem(lpSS, i);
      }

return (TRUE);
}


BOOL DBSS_BoundDel(HWND hWndSS, LPVBSPREAD lpSpread, SS_COORD Row1,
                   SS_COORD Row2)
{
LPSPREADSHEET lpSS;
LPSS_ROW      lpRow;
LPDBINFO      lpDBInfo = &lpSpread->DBInfo;
SS_COORD      i;
BOOL          fRet = TRUE;
int           ret;

lpSS = SS_Lock(hWndSS);

Row1 = max(Row1, 1);

if (!lpSpread->DataOpts.fAutoSave)
   {
   for (i = Row1; i <= Row2; i++)
      {
      if (lpRow = SS_LockRowItem(lpSS, i))
         lpRow->bDirty = SS_ROWDIRTY_DEL;

      SSShowRow(hWndSS, i, FALSE);
      }
   }

else
   {
   if (!xDB_SDK_Get_Random_Record(lpDBInfo->hStmt, Row1))
      {
      for (i = Row1; i <= Row2; i++)
         {
         if (xDB_SDK_Delete_Record(lpDBInfo->hStmt))
            break;

         SSDelRowRange(hWndSS, Row1, Row1);

         if (ret = xDB_SDK_Get_Next_Record(lpDBInfo->hStmt))
            {
            if (ret == qeEOF)
               ret = xDB_SDK_Get_Previous_Record(lpDBInfo->hStmt);

            break;
            }
         }
      }
   }

SS_Unlock(hWndSS);
return (TRUE);
}


BOOL DBSS_BoundIns(HWND hWndSS, LPVBSPREAD lpSpread, SS_COORD Row1,
                   SS_COORD Row2)
{
LPSPREADSHEET lpSS;
LPSS_ROW      lpRow;
LPDBINFO      lpDBInfo = &lpSpread->DBInfo;
SS_COORD      i;

lpSS = SS_Lock(hWndSS);

Row1 = max(Row1, 1);

if (!lpSpread->DataOpts.fAutoSave)
   {
   for (i = Row1; i <= Row2; i++)
      if (lpRow = SS_LockRowItem(lpSS, i))
         lpRow->bDirty = SS_ROWDIRTY_INS;
   }

else
   {
   for (i = Row1; i <= Row2; i++)
      DBSS_InsertRecord(&lpSpread->DBInfo, i);
   }

SS_Unlock(hWndSS);
return (TRUE);
}


/********************************************************************/
WORD _DBSS_GetBound(LPVBSPREAD lpSpread, WORD x)
{
switch (x)
   {
   case SSD_AUTOSAVE:
      return (lpSpread->DataOpts.fAutoSave);

   case SSD_AUTOHEADINGS:
      return (lpSpread->DataOpts.fHeadings);

   case SSD_AUTOCELLTYPES:
      return (lpSpread->DataOpts.fCellTypes);

   case SSD_AUTOFILL:
      return (lpSpread->DataOpts.fAutoFill);

   case SSD_AUTOSIZECOLS:
      return (lpSpread->DataOpts.fSizeCols);
   }

return (0);
}


/********************************************************************/
void _DBSS_SetCellTypes(HWND hWndSS, LPVBSPREAD lpSpread, LPDBINFO lpDBInfo)
{
LPSPREADSHEET  lpSS;
TBGLOBALHANDLE hFieldList;
LPSHORT        lpdFieldList;
HCTL           hCtl;
int Num_Cols, ret, x, Do_Headers, Precision, Col_Size, CellType;
char s[100];

    //if no auto cell type adjustments then exit the sub
    if(!_DBSS_GetBound(lpSpread, SSD_AUTOCELLTYPES))
        return;

    //get the number of columns
    Num_Cols = xDB_SDK_Get_Num_Of_Cols(lpDBInfo->hStmt);
    
    //get if auto columns headers are requested
    Do_Headers = _DBSS_GetBound(lpSpread, SSD_AUTOHEADINGS);

    hCtl = VBGetHwndControl(hWndSS);
    lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

    lpSS = SS_Lock(hWndSS);

    hFieldList = _DBSS_BuildFieldList(lpSS, Num_Cols);
    lpdFieldList = (LPSHORT)tbGlobalLock(hFieldList);
    
    //for each column
    for(x=1; x<=Num_Cols; x++)
        {
        if (lpdFieldList[x - 1])
           {
           //get the cell type
           CellType = xDB_SDK_Get_Cell_Type(lpDBInfo->hStmt, x, &Precision,
                                            &Col_Size);

           SetDataCellTypes3(hWndSS, lpdFieldList[x - 1], CellType);

           //get the database field name
           s[0] = '\0';
           xDB_SDK_Get_Field_Name(lpDBInfo->hStmt, s, x);

           //if setting column headers
           if(Do_Headers)
               ret = SSSetData(hWndSS, lpdFieldList[x - 1], SS_HEADER, s);

           /* fire DataColConfig to allow them to make any changes */

           VBSSBoundFireDataColConfig((LONG)hCtl, lpSpread, s, lpdFieldList[x - 1],
                                      CellType);
           }
        }

    tbGlobalUnlock(hFieldList);
    tbGlobalFree(hFieldList);
}


void _DBSS_SetFieldNames(HWND hWndSS, LPDBINFO lpDBInfo)
{
LPSPREADSHEET lpSS;
LPSS_COL      lpCol;
LPSTR         lpszFieldName;
LPSTR         lpszDBFieldName;
short         Num_Cols;
short         FieldNum;
short         i;
BOOL          fFound;

lpSS = SS_Lock(hWndSS);

Num_Cols = xDB_SDK_Get_Num_Of_Cols(lpDBInfo->hStmt);

if (!lpSS->DataFieldNameCnt)
   {
   for (i = 1; i <= Num_Cols; i++)
      if (lpCol = SS_AllocLockCol(lpSS, i))
         {
         lpCol->bDataType = 0;
         lpCol->dFieldNum = (short)i;
         SS_UnlockColItem(lpSS, i);
         }
   }

else
   {
   for (i = 1; i < lpSS->Col.AllocCnt; i++)
      if (lpCol = SS_LockColItem(lpSS, i))
         {
         lpCol->bDataType = 0;
         lpCol->dFieldNum = 0;

         if (lpCol->hFieldName)
            {
            lpszFieldName = (LPSTR)tbGlobalLock(lpCol->hFieldName);

            if (*lpszFieldName == '#')
               lpCol->dFieldNum = StringToInt(&lpszFieldName[1]);

            tbGlobalUnlock(lpCol->hFieldName);
            }

         SS_UnlockColItem(lpSS, i);
         }

   for (FieldNum = 1; FieldNum <= Num_Cols; FieldNum++)
      {
      lpszDBFieldName = xDB_SDK_Get_Field_Name(lpDBInfo->hStmt, NULL, FieldNum);

      for (i = 1, fFound = FALSE; i < lpSS->Col.AllocCnt && !fFound; i++)
         if (lpCol = SS_LockColItem(lpSS, i))
            {
            if (lpCol->hFieldName)
               {
               lpszFieldName = (LPSTR)tbGlobalLock(lpCol->hFieldName);

               if (*lpszFieldName != '#')
                  if (lstrcmpi(lpszFieldName, lpszDBFieldName) == 0)
                     {
                     lpCol->dFieldNum = FieldNum;
                     fFound = TRUE;
                     }

               tbGlobalUnlock(lpCol->hFieldName);
               }

            SS_UnlockColItem(lpSS, i);
            }
      }
   }

SS_Unlock(hWndSS);
}


/********************************************************************/
void _DBSS_SetColWidth(HWND hWndSS, LPSPREADSHEET LPSS,
                       LPVBSPREAD lpSpread, LPDBINFO lpDBInfo)
{
LPSPREADSHEET lpSS;
short         dFieldCnt;

    //if no auto column sizing then exit
    if(!_DBSS_GetBound(lpSpread, SSD_AUTOSIZECOLS))
        return;

    //get the number of columns
    dFieldCnt = xDB_SDK_Get_Num_Of_Cols(lpDBInfo->hStmt);
    
    //for each column

    lpSS = SS_Lock(hWndSS);
    VBSSBoundSetColWidth(hWndSS, lpSpread);

#if 0
    for (x = 1; x < lpSS->Col.AllocCnt; x++)
       {
       fAutoSize = FALSE;

       if (lpCol = SS_LockColItem(lpSS, x))
          {
          if (lpCol->dFieldNum && lpCol->dFieldNum <= dFieldCnt)
             fAutoSize = TRUE;

          SS_UnlockColItem(lpSS, x);
          }

       if (fAutoSize)
          {
          //if smart col size then size col to widest text in that col
          if(_DBSS_GetBound(lpSpread, SSD_AUTOSIZECOLS) == 1)
              {
              ret = SSGetMaxTextColWidth(hWndSS, x, &Text_Width);
              ret = SSSetColWidth(hWndSS, x, Text_Width);
              }
          else //field width
              {
              //get width of widest text in col, including col header text
              //ret = SSGetMaxTextColWidth(hWndSS,x,&Text_Width);

              //get width of database field in bytes
              Field_Width = xDB_SDK_Get_Field_Size(lpDBInfo->hStmt,
                                                   lpCol->dFieldNum);

              //set the col width to the wider of the two
              /*
              if(Text_Width > Field_Width)
                  ret = SSSetColWidth(hWndSS, x, Text_Width);
              else
                  ret = SSSetColWidth(hWndSS, x, Field_Width);
              */
              ret = SSSetColWidth(hWndSS, x, Field_Width);
              }
          }
       }
#endif

    SS_Unlock(hWndSS);
}


#if 0
/********************************************************************/
void _DBSS_SetCurrentRow(LPDBINFO lpDBInfo, long Row)
{
int ret, Cur_Row, state;

    //get the current record
    Cur_Row = xDB_SDK_Get_Current_Record_Number(lpDBInfo->hStmt);

    //get the state of hte current record
    state = xDB_SDK_Is_Rec_Mod_Or_Added(lpDBInfo->hStmt);
    
    //if the requested record is not the current record
    if(Cur_Row != Row)
        {
        //if the record changed
        if(state)
            {
            //save the record
            ret = xDB_SDK_Update_Record(lpDBInfo->hStmt);
            
            //end previous transaction
            ret = xDB_SDK_Commit(lpDBInfo->hDB);
            }
        else
            {
            //end previous transaction
            ret = xDB_SDK_RollBack(lpDBInfo->hDB);
            }
            
        //begin new transaction
        ret = xDB_SDK_Begin_Trans(lpDBInfo->hDB);
        
        //set the current record
        ret = xDB_SDK_Get_Random_Record(lpDBInfo->hStmt, Row);

        //if past end of file then insert the record into the database
        if(ret == qeEOF)
            DBSS_InsertRecord(lpDBInfo, Row);

        //lock the record
        ret = xDB_SDK_Lock_Rec(lpDBInfo->hStmt);
        }
}
#endif


BOOL DBSS_QueryConnect(HWND hWndSS)
{
return (VBDialogBoxParam(GetWindowWord(hWndSS, GWW_HINSTANCE), "VBSSOPENDB",
                         (FARPROC)_DBSS_ConnectDlgProc, (long)hWndSS));
}


BOOL FAR PASCAL _export _DBSS_ConnectDlgProc(HWND hDlg, USHORT Msg,
                                             WPARAM wParam, LPARAM lParam)
{
static HBRUSH hBrush;
static HWND   hWndSS;

switch (Msg)
   {
   case WM_INITDIALOG:
      DlgBoxCenter(hDlg);
      hBrush = CreateSolidBrush(RGBCOLOR_PALEGRAY);
      hWndSS = (HWND)lParam;

      {
      HINSTANCE hInstODBC;
      FARPROC   lpfnProc;
      long      hEnv = 0;
      char      szDataSource[32 + 1];
      char      szDescription[255 + 1];
      short     dDataSourceLen;
      short     dDescriptionLen;
      BOOL      fOK = FALSE;
      int       iParam2;
      int       iRet;

      if ((hInstODBC = LoadLibrary("odbc.dll")) > HINSTANCE_ERROR)
         {
         if ((FARPROC)lpfnProc = GetProcAddress(hInstODBC, "SQLALLOCENV"))
            if ((*lpfnProc)((LPLONG)&hEnv) != -1)
               {
               if ((FARPROC)lpfnProc = GetProcAddress(hInstODBC,
                                                      "SQLDATASOURCES"))
                  {
                  fOK = TRUE;
                  iParam2 = 2;
                  do
                     {
                     _fmemset(szDataSource, '\0', sizeof(szDataSource));
                     _fmemset(szDescription, '\0', sizeof(szDescription));

                     iRet = (*lpfnProc)(hEnv, iParam2, (LPSTR)szDataSource,
                                       (short)sizeof(szDataSource) - 1,
                                       (LPSHORT)&dDataSourceLen,
                                       (LPSTR)szDescription,
                                       (short)sizeof(szDescription) - 1,
                                       (LPSHORT)&dDescriptionLen);

                     if (iRet == 0 || iRet == 1)
                        SendDlgItemMessage(hDlg, IDD_OPENDB_SOURCE,
                                           CB_ADDSTRING, 0,
                                           (LONG)(LPSTR)szDataSource);

                     iParam2 = 1;
                     } while (iRet == 0 || iRet == 1);
                  }
               }
         FreeLibrary(hInstODBC);
         }
      if (!fOK)
         {
         char   szTitle[36];
         char   szMsg[256];
         LoadString(hDynamicInst, IDS_ERR_NODBENGINE + LANGUAGE_BASE, szMsg, sizeof(szMsg)-1);
         LoadString(hDynamicInst, IDS_ERR_DBENGINEHDR + LANGUAGE_BASE, szTitle, sizeof(szTitle)-1);
         MessageBox(hDlg, szMsg, szTitle, MB_OK);

         EndDialog(hDlg, 0);
         }
      }
      return (TRUE);

   case WM_CTLCOLOR:
      if (HIWORD(lParam) == CTLCOLOR_DLG)
         return (hBrush);

      else if (HIWORD(lParam) == CTLCOLOR_STATIC)
         {
         SetBkColor((HDC)wParam, RGBCOLOR_PALEGRAY);
         return (hBrush);
         }

      break;

   case WM_DESTROY:
      DeleteObject(hBrush);
      break;

   case WM_COMMAND:
      switch (wParam)
         {
         case IDOK:
            {
            LPVBSPREAD     lpSpread;
            TBGLOBALHANDLE hConnect = 0;
            LPSTR          lpszConnect;
            char           szBuffer[100];
            char           szConnect[1000];
            HCTL           hCtl = VBGetHwndControl(GetParent(hWndSS));

            lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

            szBuffer[0] = '\0';
            GetDlgItemText(hDlg, IDD_OPENDB_SOURCE, (LPSTR)szBuffer, 100);

            if (szBuffer[0])
               {
               lstrcpy(szConnect, "DSN=");
               _fstrcat(szConnect, szBuffer);

               // Build connect string

               szBuffer[0] = '\0';
               GetDlgItemText(hDlg, IDD_OPENDB_USERID, (LPSTR)szBuffer, 100);

               if (szBuffer[0])
                  {
                  _fstrcat(szConnect, ";UID=");
                  _fstrcat(szConnect, szBuffer);
                  }

               szBuffer[0] = '\0';
               GetDlgItemText(hDlg, IDD_OPENDB_PASSWORD, (LPSTR)szBuffer, 100);

               if (szBuffer[0])
                  {
                  _fstrcat(szConnect, ";PWD=");
                  _fstrcat(szConnect, szBuffer);
                  }

               szBuffer[0] = '\0';
               GetDlgItemText(hDlg, IDD_OPENDB_DB, (LPSTR)szBuffer, 100);

               if (szBuffer[0])
                  {
                  _fstrcat(szConnect, ";DATABASE=");
                  _fstrcat(szConnect, szBuffer);
                  }

               hConnect = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                        lstrlen(szConnect) + 1);

               if (hConnect)
                  {
                  lpszConnect = (LPSTR)tbGlobalLock(hConnect);
                  lstrcpy(lpszConnect, szConnect);
                  tbGlobalUnlock(hConnect);
                  }
               }

            if (lpSpread->DBInfo.hDataConnect)
               tbGlobalFree(lpSpread->DBInfo.hDataConnect);

            lpSpread->DBInfo.hDataConnect = hConnect;

            EndDialog(hDlg, TRUE);
            }
            return (TRUE);

         case IDCANCEL:
            EndDialog(hDlg, 0);
            return (TRUE);
         }

      break;
   }

return (FALSE);
}


TBGLOBALHANDLE _DBSS_BuildFieldList(LPSPREADSHEET lpSS, short Num_Cols)
{
TBGLOBALHANDLE hFieldList;
LPSS_COL       lpCol;
LPSHORT        lpdFieldList;
short          i;

    //allocate an array of short that will hold the database field number
    if (hFieldList = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, Num_Cols * sizeof(short)))
        {
        //get a pointer to that array
        lpdFieldList = (LPSHORT)tbGlobalLock(hFieldList);

        //for each allocated column in the spreadsheet
        for (i = 1; i < lpSS->Col.AllocCnt; i++)
            //get the coresponding spreadsheet coumn number for the database field
            if (lpCol = SS_LockColItem(lpSS, i))
                {
                if (lpCol->dFieldNum && lpCol->dFieldNum <= Num_Cols)
                    lpdFieldList[lpCol->dFieldNum - 1] = i;

                SS_UnlockColItem(lpSS, i);
                }

        //unlock the array
        tbGlobalUnlock(hFieldList);
        }
                          
    //return the handle of the array that maps the database fields to the spreadsheet columns
    return (hFieldList);
}


BOOL _DBSS_SaveRow(HCTL hCtl, HWND hWnd, LPVBSPREAD lpSpread,
                   LPSPREADSHEET lpSS, SS_COORD Row)
{
LPSS_ROW lpRow;
LPSS_COL lpCol;
short    i;
BOOL     fRet = TRUE;

if (lpRow = SS_LockRowItem(lpSS, Row))
   {
   if (lpRow->bDirty == SS_ROWDIRTY_SOME || lpRow->bDirty == SS_ROWDIRTY_ALL ||
       lpRow->bDirty == SS_ROWDIRTY_INS)
      {
      xDB_SDK_Begin_Trans(lpSpread->DBInfo.hDB);
      // Set the current record
      xDB_SDK_Get_Random_Record(lpSpread->DBInfo.hStmt, Row);
      xDB_SDK_Lock_Rec(lpSpread->DBInfo.hStmt);

      for (i = 1; i < lpSS->Col.AllocCnt; i++)
         {
         if (lpCol = SS_LockColItem(lpSS, i))
            {
            if (lpCol->dFieldNum)
               _DBSS_SaveRow2(hCtl, hWnd, lpSpread, lpSS, Row, i, lpCol,
                              lpRow->bDirty);

            SS_UnlockColItem(lpSS, i);
            }
         }

      xDB_SDK_Update_Record(lpSpread->DBInfo.hStmt);
      xDB_SDK_Commit(lpSpread->DBInfo.hDB);
      lpRow->bDirty = FALSE;
      }

   SS_UnlockRowItem(lpSS, Row);
   }

return (fRet);
}


BOOL _DBSS_SaveRow2(HCTL hCtl, HWND hWnd, LPVBSPREAD lpSpread,
                    LPSPREADSHEET lpSS, SS_COORD Row, SS_COORD Col,
                    LPSS_COL lpCol, BYTE bRowDirty)
{
SS_CELLTYPE CellType;
LPSS_CELL   lpCell;
USHORT      usFieldType;
BOOL        fRet = TRUE;
BOOL        fDirty = FALSE;
BOOL        fDataFillEvent = FALSE;
short       Precision;
short       ColSize;

SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

if (lpCell = SS_LockCellItem(lpSS, Col, Row))
   {
   fDirty = lpCell->fDirty;
   lpCell->fDirty = FALSE;
   SS_UnlockCellItem(lpSS, Col, Row);
   }

/*****************************
* If cell is dirty then save
*****************************/

if (fDirty || bRowDirty == SS_ROWDIRTY_ALL || bRowDirty == SS_ROWDIRTY_INS)
   {
   usFieldType = xDB_SDK_Get_Cell_Type(lpSpread->DBInfo.hStmt, lpCol->dFieldNum,
                                       &Precision, &ColSize);

   if (lpCol = SS_LockColItem(lpSS, Col))
      {
      fDataFillEvent = lpCol->fDataFillEvent;
      SS_UnlockColItem(lpSS, Col);
      }

   if (fDataFillEvent)
      if (VBSSBoundFireDataFill((LONG)hCtl, lpSpread, Col, Row, FALSE,
                                usFieldType, Row, lpCol->dFieldNum))
         return (TRUE);

   _DBSS_SSToDB(hWnd, lpSpread, lpSS, &CellType, lpCol->dFieldNum, Col, Row,
                usFieldType);
   }

return (fRet);
}


void _DBSS_SSToDB(HWND hWnd, LPVBSPREAD lpSpread, LPSPREADSHEET lpSS,
                  LPSS_CELLTYPE lpCellType, short dFieldNum, SS_COORD Col,
                  SS_COORD Row, USHORT usDataType)
{
TBGLOBALHANDLE hData;
HLSTR          hlstrData;

#ifdef  BUGS
// Bug-002
char           szBuffer[330];
#else
char           szBuffer[50];
#endif

LPSTR          lpData = NULL;
short          uLen;
BOOL           fValue = TRUE;
char           cDecimalSign;

if ((lpCellType->Type == SS_TYPE_COMBOBOX &&
    (usDataType == DATA_VT_TEXT || usDataType == DATA_VT_MEMO)) ||
    lpCellType->Type == SS_TYPE_PIC || usDataType == DATA_VT_BOOL)
   fValue = FALSE;

if (fValue)
   hData = SS_GetValue(lpSS, Col, Row);
else if (lpSS->EditModeOn && lpSS->Col.CurAt == Col && lpSS->Row.CurAt == Row)
   hData = SS_GetEditModeOnData(hWnd, lpSS, fValue);
else
   hData = SS_GetData(lpSS, lpCellType, Col, Row, fValue);

if (hData)
   lpData = (LPSTR)tbGlobalLock(hData);

if (usDataType == DATA_VT_BOOL)
   {
   if (!lpData || !*lpData || *lpData == '0')
      xDB_SDK_Set_Long(lpSpread->DBInfo.hStmt, dFieldNum, 0L);
   else
      xDB_SDK_Set_Long(lpSpread->DBInfo.hStmt, dFieldNum, 1L);
   }

else if (usDataType == DATA_VT_INTEGER)
   xDB_SDK_Set_Int(lpSpread->DBInfo.hStmt, dFieldNum, StringToInt(lpData));

else if (usDataType == DATA_VT_LONG)
   xDB_SDK_Set_Long(lpSpread->DBInfo.hStmt, dFieldNum, StringToLong(lpData));

else if (!lpData && lpCellType->Type != SS_TYPE_PICTURE)
   xDB_SDK_Set_Null(lpSpread->DBInfo.hStmt, dFieldNum);

else if (usDataType == DATA_VT_DATETIME)
   {
   if (lpCellType->Type == SS_TYPE_DATE)
      xDB_SDK_Set_Date(lpSpread->DBInfo.hStmt, dFieldNum, lpData, NULL);
   else if (lpCellType->Type == SS_TYPE_TIME)
      xDB_SDK_Set_Date(lpSpread->DBInfo.hStmt, dFieldNum, NULL, lpData);
   }

else if (lpCellType->Type == SS_TYPE_FLOAT)
   {
   SS_FLOATFORMAT ff;

   SS_GetDefFloatFormat(lpSS->lpBook, & ff);

   if (lpCellType->Spec.Float.fSetFormat &&
       lpCellType->Spec.Float.Format.cDecimalSign)
      cDecimalSign = lpCellType->Spec.Float.Format.cDecimalSign;
   else
      cDecimalSign = ff.cDecimalSign;

   if (cDecimalSign != '.')
      {
      lstrcpy(szBuffer, lpData);
      lpData = szBuffer;
      SS_StrReplaceCh(szBuffer, '.', cDecimalSign);
      }

   xDB_SDK_Set_Field(lpSpread->DBInfo.hStmt, dFieldNum, "", lpData);
   }

else if (usDataType == DATA_VT_BINARY &&
         lpCellType->Type == SS_TYPE_PICTURE)
   {
   hlstrData = VBSSBmpSave(lpSS, (HBITMAP)lpCellType->Spec.ViewPict.hPictName,
                           lpCellType->Spec.ViewPict.hPal,
                           lpCellType->Style, Col, Row);
   lpData = VBDerefHlstr(hlstrData);
   VBDerefHlstrLen(hlstrData, &uLen);
   xDB_SDK_Set_Binary(lpSpread->DBInfo.hStmt, dFieldNum, lpData, uLen);
   VBDestroyHlstr(hlstrData);
   }

else
   xDB_SDK_Set_Field(lpSpread->DBInfo.hStmt, dFieldNum, "", lpData);

if (hData)
   {
   tbGlobalUnlock(hData);

   if (fValue || (lpSS->EditModeOn && lpSS->Col.CurAt == Col &&
       lpSS->Row.CurAt == Row))
      tbGlobalFree(hData);
   }
}


void _DBSS_DBToSS(HWND hWnd, LPVBSPREAD lpSpread, LPSPREADSHEET lpSS,
                  short dFieldNum, SS_COORD Col, SS_COORD Row,
                  USHORT usDataType)
{
TBGLOBALHANDLE hData;
char           sz[85];
char           szDate[8 + 1];
LPSTR          lpText;
HPSTR          hpData;
SS_CELLTYPE    CellType;
LONG           lData;
LONG           lLen;
double         dfData;

SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

switch (CellType.Type)
   {
   case SS_TYPE_DATE:
      if (usDataType == DATA_VT_DATETIME)
         if (!xDB_SDK_Get_Date(lpSpread->DBInfo.hStmt, dFieldNum, sz, NULL))
            SS_ClipData(lpSS, Col, Row, sz, TRUE, FALSE);

      break;

   case SS_TYPE_TIME:
      if (usDataType == DATA_VT_DATETIME)
         if (!xDB_SDK_Get_Date(lpSpread->DBInfo.hStmt, dFieldNum, szDate, sz))
            {
            if (CellType.Type == SS_TYPE_DATE && sz[0] && !szDate[0])
               {
               SSSetTypeTime(hWnd, &CellType, 0, NULL, NULL, NULL);
               SSSetCellType(hWnd, Col, SS_ALLROWS, &CellType);
               }

            SS_ClipData(lpSS, Col, Row, sz, TRUE, FALSE);
            }

      break;

   case SS_TYPE_STATICTEXT:
   case SS_TYPE_EDIT:
   case SS_TYPE_PIC:
      lpText = xDB_SDK_Get_Char_Val_For_Field(lpSpread->DBInfo.hStmt, NULL,
                                              dFieldNum, "", 0);

      SS_ClipData(lpSS, Col, Row, lpText, FALSE, FALSE);
      break;

   case SS_TYPE_FLOAT:
      xDB_SDK_Get_Double_Val_For_Field(lpSpread->DBInfo.hStmt, dFieldNum,
                                       &dfData);
      SSSetFloat(hWnd, Col, Row, dfData);
      break;

   case SS_TYPE_INTEGER:
      lData = xDB_SDK_Get_Long_Val_For_Field(lpSpread->DBInfo.hStmt,
                                             dFieldNum);
      SSSetInteger(hWnd, Col, Row, lData);
      break;

   case SS_TYPE_COMBOBOX:
      if (usDataType == DATA_VT_TEXT || usDataType == DATA_VT_MEMO)
         {
         lpText = xDB_SDK_Get_Char_Val_For_Field(lpSpread->DBInfo.hStmt, NULL,
                                                 dFieldNum, "", 0);
         SS_ClipData(lpSS, Col, Row, lpText, FALSE, FALSE);
         }
      else
         {
         lData = xDB_SDK_Get_Long_Val_For_Field(lpSpread->DBInfo.hStmt,
                                                dFieldNum);
         wsprintf(sz,"%d", (int)(usDataType == DATA_VT_BOOL ? (lData != 0) :
                  lData));
         SSSetValue(hWnd, Col, Row, sz);
         }

      break;

   case SS_TYPE_CHECKBOX:
      lData = xDB_SDK_Get_Long_Val_For_Field(lpSpread->DBInfo.hStmt,
                                             dFieldNum);
      wsprintf(sz,"%d", (int)(usDataType == DATA_VT_BOOL ? (lData != 0) :
               lData));
      SSSetValue(hWnd, Col, Row, sz);
      break;

   case SS_TYPE_PICTURE:
      if (usDataType == DATA_VT_BINARY &&
          (hData = xDB_SDK_Get_Binary(lpSpread->DBInfo.hStmt, dFieldNum,
                                      &lLen)))
         {
         if (lLen)
            {
            hpData = (LPSTR)tbGlobalLock(hData);

            lpSpread->Col = Col;
            lpSpread->Row = Row;

            VBSSBmpLoad((LONG)VBGetHwndControl(hWnd), hWnd, lpSpread, hpData, lLen);
            tbGlobalUnlock(hData);
            }

         tbGlobalFree(hData);
         }

      break;

   case SS_TYPE_BUTTON:                          /* nothing to do for these */
   case SS_TYPE_OWNERDRAW:
      break;
   }

SS_Unlock(hWnd);   
}


BOOL DBSS_GetDataFillData(HCTL hCtl, LPVAR lpVar, LPSTR lpszText,
                          WORD wVarType)
{
LPVBSPREAD lpSpread;
VALUE      Value;
LPSTR      lpszStr;
double     dfData;
BOOL       fRet = FALSE;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (lpSpread->dDataGetFillCol != -1)
   {
   if (lpszText)
      {
      lpszStr = xDB_SDK_Get_Char_Val_For_Field(lpSpread->DBInfo.hStmt, NULL,
                                                lpSpread->dDataGetFillCol, "",
                                                0);
      lstrcpy(lpszText, lpszStr);
      return (-1);
      }

   switch (wVarType)
      {
      case VT_I2:
         Value.i2 = (short)xDB_SDK_Get_Long_Val_For_Field(
                           lpSpread->DBInfo.hStmt,
                           lpSpread->dDataGetFillCol);
         break;

      case VT_I4:
         Value.i4 = xDB_SDK_Get_Long_Val_For_Field(lpSpread->DBInfo.hStmt,
                                                   lpSpread->dDataGetFillCol);
         break;

      case VT_R4:
         xDB_SDK_Get_Double_Val_For_Field(lpSpread->DBInfo.hStmt,
                                          lpSpread->dDataGetFillCol, &dfData);
         Value.r4 = (float)dfData;
         break;

      case VT_R8:
         xDB_SDK_Get_Double_Val_For_Field(lpSpread->DBInfo.hStmt,
                                          lpSpread->dDataGetFillCol, &dfData);
         Value.r8 = dfData;
         break;

      case VT_STRING:
      default:
         wVarType = VT_STRING;
         lpszStr = xDB_SDK_Get_Char_Val_For_Field(lpSpread->DBInfo.hStmt, NULL,
                                                  lpSpread->dDataGetFillCol,
                                                  "", 0);

         Value.hlstr = VBCreateHlstr(lpszStr, lstrlen(lpszStr));
         break;
      }

   VBSetVariantValue(lpVar, wVarType, &Value);
   fRet = -1;
   }

return (fRet);
}

/********************************************************************/
void DLLENTRY DBSS_Connect(HCTL hCtl, DBHANDLE ConnectHandle, DBHANDLE SelectHandle)
{   
LPVBSPREAD      lpSpread;
TBGLOBALHANDLE  h;
LPSTR           sptr;
HWND            hWnd;

    //get window handle
    lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);    
    
    //if conncected, then disconnect                                      
    if(lpSpread->DBInfo.fConnected)
        {
        hWnd = VBGetControlHwnd(hCtl);
        DBSS_EndBind(hWnd, &lpSpread->DBInfo);
        }

    //set connection TRUE
    lpSpread->DBInfo.fConnected = TRUE;
    
    //set database conneciton handle
    lpSpread->DBInfo.hDB = ConnectHandle;
    
    //set select connection handle
    lpSpread->DBInfo.hStmt = SelectHandle;
                                  
    //free database connection string
    if(lpSpread->DBInfo.hDataConnect)           
        {
        tbGlobalFree(lpSpread->DBInfo.hDataConnect);
        lpSpread->DBInfo.hDataConnect = 0;
        }
                                     
    //free select connection string
    if (lpSpread->DBInfo.hDataSelect)
        {
        tbGlobalFree(lpSpread->DBInfo.hDataSelect);
        lpSpread->DBInfo.hDataSelect = 0;
        }
    
    //allocate space for connection string
    h = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 2);
    
    //copy connection string
    sptr = (LPSTR)tbGlobalLock(h);
    lstrcpy(sptr, " ");
    tbGlobalUnlock(h);                            
    lpSpread->DBInfo.hDataConnect = h;
 
    //allocate space for selection string
    h = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 2);
    
    //copy selection string
    sptr = (LPSTR)tbGlobalLock(h);
    lstrcpy(sptr, " ");
    tbGlobalUnlock(h);                            
    lpSpread->DBInfo.hDataSelect = h;
}

/********************************************************************/
void DLLENTRY DBSS_Disconnect(HCTL hCtl)
{   
LPVBSPREAD      lpSpread;
HWND            hWnd;

    //get window handle
    lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);    
    hWnd = VBGetControlHwnd(hCtl);
    DBSS_EndBind(hWnd, &lpSpread->DBInfo);

    if (lpSpread->DBInfo.hDataSelect)
        tbGlobalFree(lpSpread->DBInfo.hDataSelect);

    if (lpSpread->DBInfo.hDataConnect)
        tbGlobalFree(lpSpread->DBInfo.hDataConnect);

    lpSpread->DBInfo.hDataSelect = 0;
    lpSpread->DBInfo.hDataConnect = 0;
    
}
    
/********************************************************************/
void DLLENTRY DBSS_Edit_Query(HCTL hCtl)
{   
LPVBSPREAD      lpSpread;
HWND            hWnd;

    //get window handle
    lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);    
    hWnd = VBGetControlHwnd(hCtl);
    
    DBSS_EditQuery(hWnd, lpSpread, &lpSpread->DBInfo);
}   

#endif
