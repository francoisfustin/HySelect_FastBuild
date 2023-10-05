/*********************************************************
* VBBOUND.C
*
* Copyright (C) 1991-1993 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/

#if ((defined(SS_OCX) || defined(FP_DLL)) && defined(WIN32))
#define WIN32_LEAN_AND_MEAN   //exclude conflicts with VBAPI.H in WIN32
#endif

#include <windows.h>
#include <tchar.h>
#include "..\..\..\fplibs\fptools\src\fptools.h"
#include "..\..\..\fplibs\fptools\src\fpconvrt.h"    //DBOCX
#include "..\sssrc\spread\spread.h"

#ifdef SS_OCX
#include "..\..\..\fplibs\fpdb\src\fpdbutil.h"       //DBOCX
#endif
#include "vbsscnvt.h"                                //DBOCX

#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdio.h>
#include "fphdc.h"
#include "..\sssrc\spread\ss_alloc.h"
#include "..\sssrc\spread\ss_bool.h"
#include "..\sssrc\spread\ss_col.h"
#include "..\sssrc\spread\ss_cell.h"
#include "..\sssrc\spread\ss_doc.h"
#include "..\sssrc\spread\ss_draw.h"
#include "..\sssrc\spread\ss_emode.h"
#include "..\sssrc\spread\ss_main.h"
#include "..\sssrc\spread\ss_row.h"
#include "..\sssrc\spread\ss_scrol.h"
#include "..\sssrc\spread\ss_type.h"
#include "..\sssrc\spread\ss_virt.h"
#include "..\sssrc\spread\ss_data.h"
#include "..\sssrc\spread\ss_save4.h"
#include "..\sssrc\spread\ss_user.h"
#include "..\sssrc\classes\wintools.h"
#include "vbspread.h"
#include "vbdattim.h"
#include "vbmsg.h"              //DBOCX

#ifdef SS_OCX
#include "..\ssocx\ssocxdb.h"   //DBOCX (this file can be included in C files)
#endif

#ifdef SS_QE
#include "db_bind.h"
#endif

#ifdef SS_BOUNDCONTROL

// Use with FindHighlight()
#define FIND_CURRENT_BOOKMARK    0x0000  
#define FIND_SPECIFIED_BOOKMARK  0x0001
#define FIND_DONT_SETACTIVEROW   0x0002   // "OR" this with one of the above

//#define SS_BOUNDDEBUG 1

/*--------------------------------------------------------------------------*/

TCHAR szFalseTrue[] = _T("False\tTrue");

/*--------------------------------------------------------------------------*/
void DLLENTRY vbSS_SetBookmark(LPVOID lpDestBookMark, LPVOID srcBookMark);   

void SetDataCellTypes(LONG lObject, HWND hWnd, LPDATAACCESS lpda, short cFields,
                      LPVBSPREAD lpSpread, LPSPREADSHEET lpSS);
void SetDataCellTypes2(LONG lObject, HWND hWnd, LPDATAACCESS lpda, LPVBSPREAD lpSpread,
                       LPSPREADSHEET lpSS, SS_COORD Col);
void SetDataHeadings(LPSPREADSHEET lpSS, LONG lObject, LPDATAACCESS lpda);
void FillAllDataRows(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, LPDATAACCESS lpda, short cFields);
void FillDataRow(LONG lObject, HWND hWnd, LPVBSPREAD lpSpread,
                 LPSPREADSHEET lpSS, SS_COORD Row, LPDATAACCESS lpda,
                 short cFields, BOOL fUReadNext);
void FillAutoIncrDataRow(LONG lObject, HWND hWnd, LPVBSPREAD lpSpread,    //DBOCX
                 LPSPREADSHEET lpSS, SS_COORD Row, LPDATAACCESS lpda,
                 short cFields, BOOL fUReadNext);
void FillDataRow2(LONG lObject, HWND hWnd, LPVBSPREAD lpSpread,
                  LPSPREADSHEET lpSS, SS_COORD Row, LPDATAACCESS lpda,
                  SS_COORD Col, BOOL fUReadNext);
void MoveHighlight(LPSPREADSHEET lpSS, LONG lObject, LPDATAACCESS lpda, short Action);
BOOL MoveHighlightTestRow(LPDATAACCESS lpda, LPSPREADSHEET lpSS, SS_COORD Row);
BOOL MapColData(LONG lObject, LPLONG lplData, LPDATAACCESS lpda,
                USHORT usDataType, char Type, BOOL fUReadNext, LPSPREADSHEET lpSS);
BOOL DataSetRow(LPSPREADSHEET lpSS, LONG lObject, LPVBSPREAD lpSpread, LPDATAACCESS lpda,
                SS_COORD Row, LPBOOL lpfReReadData);
BOOL DataSetRow2(LPSPREADSHEET lpSS, LONG lObject, LPVBSPREAD lpSpread, LPDATAACCESS lpda,
                 SS_COORD Col, BYTE bRowDirty, LPBOOL lpfReReadData);
void vbSSBoundGotoRow(LPSPREADSHEET lpSS, LONG lObject, LPDATAACCESS lpda, SS_COORD RowDest);
BOOL  VBSSBoundIsEmpty(LONG lObject, LPDATAACCESS lpda, LPSPREADSHEET lpSS);
SS_COORD FindHighlight(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, LPVBSPREAD lpSpread, LPDATAACCESS lpda,
                       short Action, int fActionFlags);
SS_COORD SearchNearHighlight(HWND hWnd, LPDATAACCESS lpda, LPSPREADSHEET  lpSS,
  SS_COORD RowFirst, SS_COORD RowLast, BOOL fSetActive);
//DBOCX...
static BOOL ssCompareRowWithBookmark(HLSTR hlstrBookmark, LPSPREADSHEET lpSS,
  SS_COORD Row);
//DBOCX
static BOOL BookmarksEqual( HLSTR b1, HLSTR b2);

BOOL SS_DoubleToTime(double dfVal, LPTIME lpTime);
void VBSSBoundProcessPostAdd2(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, long lParam, 
                              HLSTR hlstrNewBookmark);


#ifdef SS_BOUNDDEBUG
void LogMsg(LPTSTR lpStr);
#endif


#ifdef SS_OCX
// Macros
#define SS_HCTL                        long
#define VBGetHwndControl(hWnd)         hCtl
#define DataCallPrimative(lpda, msg, action, row, col, lpSS) \
  SSOcxDataCall( lObject, lpda, msg, action, (SS_COORD)row, (SS_COORD)col, lpSS)
#define DataCall(lpda, msg, action, lpSS)                \
  DataCallPrimative(lpda, VBM_DATA_##msg, DATA_##action, -1, -1, lpSS)
#define DataCallwErrCoord(lpda, msg, action, row, col, lpSS) \
  DataCallPrimative(lpda, VBM_DATA_##msg, DATA_##action, row, col, lpSS)
#define VBDerefControl(hctl)           SSOcxVBDerefControl(hctl)
#define VBDefControlProc(hCtl, hWnd, Msg, wParam, lParam)    0
#define IGNOREMOVELAST_FLD(struct_ptr)  struct_ptr->fInternalSetBookmark
#define IGNOREMOVENEXT_FLD(struct_ptr)  struct_ptr->fInternalSetBookmark
#else  //else not SS_OCX
#define  SS_HCTL  HCTL

SHORT DataCallPrimative(LPDATAACCESS lpda, UINT msg, UINT action, LPSPREADSHEET lpSS);
#define DataCall(lpda, msg, action, lpSS)                \
  DataCallPrimative(lpda, VBM_DATA_##msg, DATA_##action, lpSS)
#define DataCallwErrCoord(lpda, msg, action, row, col, lpSS) \
  DataCallPrimative(lpda, VBM_DATA_##msg, DATA_##action, lpSS)

#define IGNOREMOVELAST_FLD(struct_ptr)  struct_ptr->fIgnoreMoveNext
#define IGNOREMOVENEXT_FLD(struct_ptr)  struct_ptr->fIgnoreMoveNext

#endif //if-else SS_OCX


/* --- proc to handle all bound control messages --- */
LONG vbSpreadBoundProc(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, WORD Msg, WORD wParam, LONG lParam)
{
SS_HCTL        hCtl = (SS_HCTL)lObject;            //SSOCX
LPVBSPREAD     lpSpread;
LPSS_ROW       lpRow;
LPSS_COL       lpCol;
LPDATAACCESS   lpda = (LPDATAACCESS)lParam;
DATAACCESS     da;
SS_COORD       ColCnt;
SS_COORD       Col;
SS_COORD       Row;
SS_COORD       i;
LPTSTR         lpszFieldName;
BOOL           fRedraw;
BOOL           fIsEmpty;
BOOL           fReReadData;
short          dStartOffset;
#ifdef FP_ADB
BOOL   fFreeBookmark;
#endif

switch (Msg)
   {
#ifdef FP_ADB
   case VBM_DATA_QUERYUSERMODE:
      // Return TRUE if UserMode (Run mode) else FALSE (Design mode)
      *(LPBOOL)lParam = FALSE;   // Assume DesignMode.

      // If no hWnd, try to create it.  SCP 7/24/00
      if (!hWnd)
        hWnd = SSOcxDBCreateHWnd((void *)lObject);

      if (hWnd)
      {
        if (lpSS)
          *(LPBOOL)lParam = !lpSS->lpBook->bDesignTime;
      }
      return 0;

   case VBM_DATA_POSTMSGTOSELF:
      // Post the message to this control's message queue
      if (hWnd)
			{
			long lRet;

			// RFW - 10/31 - 17155
			if (lpSS->fProcessingVQueryData)
				lRet = SendMessage(hWnd, SSM_FPDB_POSTMSG, wParam, lParam);
			else
				lRet = PostMessage(hWnd, SSM_FPDB_POSTMSG, wParam, lParam);

			return lRet ? (LONG)TRUE : (LONG)FALSE;  // Posted OK, return TRUE
			}

      return (LONG)FALSE;
#endif

#ifdef SS_OCX
   // An error occurred while writing to the database.
   case VBM_DATA_UERROR:
    {
      struct FP_ERRINFO
    {
      long lErrCode;
      long lExtErrCode;
          LPSS_CELLCOORD lpCell; //see ssocxdb.cpp for use of "lpvUserData"
    } FAR *lpErrInfo = (struct FP_ERRINFO FAR *)lParam;
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
      lpSpread->fIgnoreSaveBoundRow = TRUE;
      SSOcxFireDbEditError(lObject, lpErrInfo->lErrCode, lpErrInfo->lpCell);
      lpSpread->fIgnoreSaveBoundRow = FALSE;
      }
    break;
#endif //SS_OCX

   case VBM_DATA_AVAILABLE:
#ifdef SS_BOUNDDEBUG
LogMsg(_T("******************************************************"));
LogMsg(_T("VBM_DATA_AVAILABLE"));
#endif
      da = *lpda;
   
      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
   
      switch (lpda->sAction)
         {
         case DATA_REFRESH:         /* 74 */
#ifdef SS_BOUNDDEBUG
LogMsg(_T("   DATA_REFRESH"));
#endif
            /* refresh the entire spreadsheet; anything could have happened,
               including a change in the table they're looking at */
#ifdef SS_V70
            lpSS->fBoundActive = TRUE;
#else
            lpSpread->fBoundActive = TRUE;
#endif

            DataCall(&da, GET, FIELDSCOUNT, lpSS);
            lpSpread->dFieldsCount = (SHORT)da.lData;

#ifdef SS_BOUNDDEBUG
{
TCHAR Buffer[111];

wsprintf(Buffer, _T("FieldCnt = %d"), lpSpread->dFieldsCount);
LogMsg(Buffer);
}
#endif

            DataCall(&da, GET, UPDATABLE, lpSS);
            lpSpread->fUpdatable = (BOOL)da.lData;

#ifdef SS_BOUNDDEBUG
if (lpSpread->fUpdatable)
   LogMsg(_T("Updatable"));
else
   LogMsg(_T("Not Updatable"));
#endif

            DataCall(&da, GET, BOOKMARKABLE, lpSS);
            lpSpread->fBookmarkable = (BOOL)da.lData;

#ifdef SS_BOUNDDEBUG
if (lpSpread->fBookmarkable)
   LogMsg(_T("Bookmarkable"));
else
   LogMsg(_T("Not Bookmarkable"));
#endif

				// RFW - 2/20/04 - 13759
				if (lpSS->lpBook->EditModeOn)
					{
					SS_CellEditModeOff(lpSS, 0);
					SS_PostSetEditMode(lpSS, TRUE, 1L);
					}

            fRedraw = lpSS->lpBook->Redraw;
            lpSS->lpBook->Redraw = FALSE;
// 9181...
            for (i = (lpSS->fVirtualMode ? lpSS->Virtual.VTop : lpSS->Row.HeaderCnt);
                 i < (lpSS->fVirtualMode ? lpSS->Row.DataCnt : lpSS->Row.AllocCnt); i++)
               if (lpRow = SS_LockRowItem(lpSS, i))
                  {
                  vbSS_SetBookmark( (LPVOID)&lpRow->hlstrBookMark, NULL);
                  SS_UnlockRowItem(lpSS, i);
                  }
// ...9181

            /****************************************
            * Clear the current data if any exists.
            * Do not clear headers.
            ****************************************/

            for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
               if (lpCol = SS_LockColItem(lpSS, i))
                  {
                  if (lpSS->Row.DataCnt > lpSS->Row.HeaderCnt && lpCol->dFieldNum)
                     SS_ClearDataRange(lpSS, i, -1, i, -1, FALSE, FALSE);

                  lpCol->dFieldNum = 0;

                  SS_UnlockColItem(lpSS, i);
                  }

/*  moved this code prior to SSClearDataRange above
            for (i = (lpSS->fVirtualMode ? lpSS->Virtual.VTop : 1);
                 i < (lpSS->fVirtualMode ? lpSS->Row.DataCnt : lpSS->Row.AllocCnt); i++)
               if (lpRow = SS_LockRowItem(lpSS, i))
                  {
                  vbSS_SetBookmark( (LPVOID)&lpRow->hlstrBookMark, NULL);
                  SS_UnlockRowItem(lpSS, i);
                  }
*/

            if (lpSS->DataFieldNameCnt)
               ColCnt = lpSS->Col.AllocCnt - lpSS->Col.HeaderCnt;
            else
               {
               ColCnt = lpSpread->dFieldsCount;

							 //>>99918158 Leon 20101108
               if (SS_GetMaxCols(lpSS) != ColCnt)
                  {
                  SS_SetMaxCols(lpSS, ColCnt);
#ifdef SS_VB
                  lpSpread->lMaxCols = SS_GetMaxCols(lpSS);
#endif
                  }
							 //<<99918158 Leon 20101108
               }

            dStartOffset = 0;
            lpda->sDataFieldIndex = 0;
            lpda->hszDataField = 0;
            if (DataCall(lpda, GET, FIELDPOSITION, lpSS) == 0)
               dStartOffset = (short)lpda->lData;

            for (i = lpSS->Col.HeaderCnt, Col = 1; Col < ColCnt + lpSS->Col.HeaderCnt; i++)
               if (lpCol = SS_AllocLockCol(lpSS, i))
                  {
                  lpCol->bDataType = 0;

                  if (lpSS->DataFieldNameCnt)
                     {
                     lpCol->dFieldNum = 0;

                     if (lpCol->hDBFieldName)
                        {
                        lpszFieldName = (LPTSTR)tbGlobalLock(lpCol->hDBFieldName);
                        if (*lpszFieldName == (TCHAR)'#')
                           lpCol->dFieldNum = StringToInt(&lpszFieldName[1]);
                        else
                           {
                           lpda->hszDataField = ssVBCreateHsz(lObject,
                                                            lpszFieldName);

                           if (DataCall(lpda, GET, FIELDPOSITION, lpSS) == 0)
                              lpCol->dFieldNum = (short)lpda->lData -
                                                 dStartOffset + 1;

                           ssVBDestroyHsz(lpda->hszDataField);
                           tbGlobalUnlock(lpCol->hDBFieldName);
                           }
                        }
                     }

                  else
                     {
                     lpCol->dFieldNum = (short)Col++;
                     }

                  SS_UnlockColItem(lpSS, i);
                  }
               else
                  break;

            da.hlstrBookMark = NULL;
            da.fs = 0;

            if (lpSpread->DataOpts.fCellTypes)
               SetDataCellTypes(lObject, hWnd, &da, lpSpread->dFieldsCount,
                                lpSpread, lpSS);
            else if (lpSpread->DataOpts.fHeadings)
               SetDataHeadings(lpSS, lObject, &da);

            lpSS->fProcessingBoundMove = TRUE;
            
            if (lpSpread->DataOpts.fAutoFill)
               {                       /* autofill the data into the spread */
               da.hlstrBookMark = NULL;
               da.fs = 0;

               if (lpSS->fVirtualMode)
                  SS_VRefreshBuffer(lpSS, TRUE);
               else
                  FillAllDataRows(lpSS, lObject, hWnd, &da, lpSpread->dFieldsCount);
               }

				SS_BookSetRedraw(lpSS->lpBook, fRedraw);
            SS_SetActiveCell(lpSS, lpSS->Col.HeaderCnt, lpSS->Row.HeaderCnt);
            lpSS->fProcessingBoundMove = FALSE;
            break;

         case DATA_ROLLBACK:        /* 76 */
#ifdef SS_BOUNDDEBUG
LogMsg(_T("   DATA_ROLLBACK"));
#endif
            /* no way to know what happened to the dynaset -- refresh data */
            //(?but can't we assume the table didn't change?)
            if (lpSpread->DataOpts.fAutoFill)
               {                       /* autofill the data into the spread */
               da.hlstrBookMark = NULL;
               da.fs = 0;

               if (lpSS->fVirtualMode)
                  SS_VRefreshBuffer(lpSS, TRUE);
               else
                  FillAllDataRows(lpSS, lObject, hWnd, &da, lpSpread->dFieldsCount);
               }

            break;
            
         case DATA_UNLOAD:          /* 82 */
         case DATA_CLOSE:           /* 77 */
#ifdef SS_BOUNDDEBUG
LogMsg(_T("   DATA_UNLOAD/CLOSE"));
#endif
#ifdef SS_V70
            lpSS->fBoundActive = FALSE;
#else
            lpSpread->fBoundActive = FALSE;
#endif
            break;
            
         case DATA_MOVEFIRST:       /* 64 */
         case DATA_MOVENEXT:        /* 65 */
         case DATA_MOVEPREV:        /* 66 */
         case DATA_MOVELAST:        /* 67 */
#ifdef SS_BOUNDDEBUG
LogMsg(_T("   DATA_MOVE*"));
#endif
            if (!lpSpread->fIgnoreMoveNext)
               MoveHighlight(lpSS, lObject, lpda, lpda->sAction);

            break;
   
         case DATA_FINDFIRST:       /* 68 */
         case DATA_FINDNEXT:        /* 69 */
         case DATA_FINDPREV:        /* 70 */
         case DATA_FINDLAST:        /* 71 */
#ifdef SS_BOUNDDEBUG
LogMsg(_T("   DATA_FIND*"));
#endif
            FindHighlight(lpSS, lObject, hWnd, lpSpread, lpda, lpda->sAction, 
              FIND_CURRENT_BOOKMARK);              //DBOCX
            break;

         case DATA_BOOKMARK:
            if (!lpSpread->fInternalSetBookmark)
               {
               i = FindHighlight(lpSS, lObject, hWnd, lpSpread, lpda, DATA_FINDFIRST,
                             FIND_CURRENT_BOOKMARK);             //DBOCX

#ifdef SS_OCX
               // If current record was not found, then check last //SCP 2/10/99
               if (i == -1)
                 {
                 DATAACCESS daTemp = *lpda;

                 daTemp.hlstrBookMark = NULL;
                 daTemp.fs = 0;
                 daTemp.lData = DATA_BOOKMARKCURRENT;
                 DataCall(&daTemp, GET, BOOKMARK, lpSS);
                 if (daTemp.lData)
                    {
                    daTemp.hlstrBookMark = (HLSTR)daTemp.lData;
                    daTemp.lData = DATA_BOOKMARKNEXT;
                    DataCall(&daTemp, GET, BOOKMARK, lpSS);
                    if (daTemp.fs & DA_fEOF)
                       {
                       MoveHighlight(lpSS, lObject, lpda, DATA_MOVELAST);
                       break;
                       }
                    }
                 } // if i == -1
#endif
               }
            else
               return (0);
            break;
            
         case DATA_UPDATE:          /* 81 */
#ifdef SS_BOUNDDEBUG
LogMsg(_T("   DATA_UPDATE"));
#endif
            // Data1.RecordSet.Update sends this;
            /* refresh the given row */
//DBOCX...
#ifdef SS_OCX  // use row specified in hlstrBookMark
#ifdef FP_ADB
            fFreeBookmark = FALSE;
            da.sDataFieldIndex = 0;
            if (DataCall(&da, GET, UEVENTBMK, lpSS) == 0 && da.lData)
              lpda->hlstrBookMark = (HLSTR)da.lData;
#endif // FP_ADB
            Row = (SS_COORD)-1;
            if (lpda->hlstrBookMark)
              {
              Row = FindHighlight(lpSS, lObject, hWnd, lpSpread, lpda, DATA_FINDFIRST,
                      FIND_SPECIFIED_BOOKMARK | FIND_DONT_SETACTIVEROW);
#ifdef FP_ADB
              if (Row == (SS_COORD)-1)
                ssVBDestroyHlstr(lpda->hlstrBookMark); // destroy bookmark
              else
                fFreeBookmark = TRUE;
#endif // FP_ADB
              }
            
            if (Row == (SS_COORD)-1)
               break;
//DBOCX
#else
            SS_GetActiveCell(lpSS, &Col, &Row);
#endif
            if (lpSpread->fAddNewFromDataControl)
               {
               IGNOREMOVELAST_FLD(lpSpread) = TRUE;
               DataCall(&da, METHOD, MOVELAST, lpSS);
               IGNOREMOVELAST_FLD(lpSpread) = FALSE;

               lpSpread->fUpdateReceived = TRUE;

               if (lpSpread->fBookmarkable)
                  {
                  da.lData = DATA_BOOKMARKCURRENT;
                  if (DataCall(&da, GET, BOOKMARK, lpSS) == 0)
                     if (lpRow = SS_AllocLockRow(lpSS, Row))
                        {
                        vbSS_SetBookmark( (LPVOID)&lpRow->hlstrBookMark, 
                          (LPVOID)da.lData);
                        SS_UnlockRowItem(lpSS, Row);
                        }
                  }
               }

            FillDataRow(lObject, hWnd, lpSpread, lpSS, Row, lpda,
                        lpSpread->dFieldsCount, FALSE);

            VBSSBoundFinishClipData(hWnd, lpSS, Row, Row,
                                    lpSpread->dFieldsCount);

#ifdef FP_ADB
            if (fFreeBookmark && lpda->hlstrBookMark)
              ssVBDestroyHlstr(lpda->hlstrBookMark); // destroy bookmark
#endif
            break;

#ifdef FP_ADB
         case DATA_UCOLUPDATE:
#ifdef SS_BOUNDDEBUG
LogMsg(_T("   DATA_UCOLUPDATE"));
#endif
            /* refresh the given col */
            da.sDataFieldIndex = 0;
            if (DataCall(&da, GET, UEVENTBMK, lpSS) == 0 && da.lData)
              lpda->hlstrBookMark = (HLSTR)da.lData;

            if (lpda->hlstrBookMark)
              Row = FindHighlight(lpSS, lObject, hWnd, lpSpread, lpda, DATA_FINDFIRST,
                      FIND_SPECIFIED_BOOKMARK | FIND_DONT_SETACTIVEROW);
            else
              Row = (SS_COORD)-1;
            
            if (Row != (SS_COORD)-1)
               {
               if (DataCall(&da, GET, UEVENTCOLCOUNT, lpSS) == 0 && da.lData)
                  {
                  LPSS_COL  lpCol;
                  SS_COORD  Col;
                  long      lColCount = da.lData;
						long      lColIndex;

                  for (lColIndex = 0; lColIndex < lColCount; lColIndex++)
                     {
                     da.sDataFieldIndex = (SHORT)lColIndex;
                     if (DataCall(&da, GET, UEVENTCOL, lpSS) == 0)
                        {
                        for (Col = lpSS->Col.HeaderCnt; Col < lpSS->Col.AllocCnt; Col++)
                           {
                           if (lpCol = SS_LockColItem(lpSS, Col))
                              {
                              if (lpCol->dFieldNum && lpCol->dFieldNum - 1 == da.lData)
                                 {
                                 lpda->sDataFieldIndex = lpCol->dFieldNum - 1;
                                 FillDataRow2(lObject, hWnd, lpSpread, lpSS, Row, lpda, Col, FALSE);
                                 break;
                                 }

                              SS_UnlockColItem(lpSS, Col);
                              }
                           }
                        }
                     }

                  VBSSBoundFinishClipData(hWnd, lpSS, Row, Row,
                                          lpSpread->dFieldsCount);
						}
               }

            if (lpda->hlstrBookMark)
              ssVBDestroyHlstr(lpda->hlstrBookMark); // destroy bookmark

            break;
#endif
            
         case DATA_DELETE:          /* 72 */
#ifdef SS_BOUNDDEBUG
LogMsg(_T("   DATA_DELETE"));
#endif
            if (!lpSpread->fDataDeleteInternal)
               {
#ifdef FP_ADB
               int iRowCt;
               // Loop for each bookmark in list of deleted rows
               DataCall(&da, GET, UEVENTBMKCOUNT, lpSS);
               iRowCt = (int)da.lData;
               for (i = 0; i < iRowCt; i++)
                 {
                 da.sDataFieldIndex = (SHORT)i;
                 if (DataCall(&da, GET, UEVENTBMK, lpSS) == 0 && da.lData)
                   {
                   da.hlstrBookMark = (HLSTR)da.lData;
                   Row = FindHighlight(lpSS, lObject, hWnd, lpSpread, &da, DATA_FINDFIRST,
                      FIND_SPECIFIED_BOOKMARK | FIND_DONT_SETACTIVEROW);
                   if (Row != (SS_COORD)-1)
                     SS_DelRowRange(lpSS, Row, Row);
                   ssVBDestroyHlstr(da.hlstrBookMark); // free the bookmark
                   }
                 }
#else // if not FP_ADB
               // Get active row and delete it.
               SS_GetActiveCell(lpSS, &Col, &Row);
               SS_DelRowRange(lpSS, Row, Row);
#endif
               }

            break;
            
         case DATA_ADDNEW:          /* 73 */
				{
#ifdef SS_BOUNDDEBUG
LogMsg(_T("   DATA_ADDNEW"));
#endif
#ifndef SS_OCX
            lpSpread->fDataIgnoreDataRequest = TRUE;
#endif
            if (lpSpread->RowBeingInserted == 0)
               {
               fIsEmpty = VBSSBoundIsEmpty(lObject, lpda, lpSS);

               if (lpSS->Row.CurAt < lpSS->Row.DataCnt && !fIsEmpty)
						{
						// RFW - 3/2/00 - GIC11570

						if (SS_GetRowCnt(lpSS) - 1 < lpSS->Row.DataCnt)            /* ensure there are enough rows */
							{
							SS_SetMaxRows(lpSS, lpSS->Row.Max + 1);
#ifdef SS_VB
							lpSpread->lMaxRows = SSGetMaxRows(hWnd);
#endif
							lpSS->Row.CurAt = max(lpSS->Row.CurAt, lpSS->Row.HeaderCnt);
							}

                  SS_InsRowRange(lpSS, lpSS->Row.CurAt, lpSS->Row.CurAt);
						}

               else if (fIsEmpty)
                  ssVBSetControlFlags(lObject, CTLFLG_DATACHANGED,
                                    CTLFLG_DATACHANGED, lpSS);

               lpSpread->fAddNewFromDataControl = TRUE;
               lpSpread->fUpdateReceived = FALSE;
               }
#ifdef SS_OCX    //DBOCX...
            // We have a bookmark of new row, save it for the row
            {
            HLSTR hlstr = (HLSTR)0;
#ifdef FP_ADB
            // If FP_ADB, get bookmark of new row via DATA_UEVENTBMK.
            da.sDataFieldIndex = (SHORT)0;
            if (DataCall(&da, GET, UEVENTBMK, lpSS) == 0 && da.lData)
              hlstr = (HLSTR)da.lData; // save bmk
#else // not FP_ADB
            // Use the bookmark passed in "lpda->hlstrBookMark" (make a copy)
            if (lpda->hlstrBookMark)
              hlstr = ssVBCreateHlstr( lpda->hlstrBookMark, 
                         ssVBGetHlstrLen(lpda->hlstrBookMark));
#endif // not FP_ADB

            VBSSBoundProcessPostAdd2(lpSS, lObject, hWnd, lpSS->Row.CurAt, hlstr);
            }
#else
            PostMessage(hWnd, SSM_POSTADDNEW, 0, lpSS->Row.CurAt);
#endif         //DBOCX
				}
            break;

#ifdef FP_ADB
         case DATA_UREPLACEBMK: // Change bookmarks from one to the other
            // Get first bookmark (this is the one we are currently using).
            da.sDataFieldIndex = (SHORT)0;
            if (DataCall(&da, GET, UEVENTBMK, lpSS) == 0 && da.lData)
              {
              HLSTR hlstrOldBmk = (HLSTR)da.lData; // save bmk
              da.hlstrBookMark = (HLSTR)da.lData;
              // Find row which is using this bookmark
              Row = FindHighlight(lpSS, lObject, hWnd, lpSpread, &da, DATA_FINDFIRST,
                 FIND_SPECIFIED_BOOKMARK | FIND_DONT_SETACTIVEROW);
              // If valid row and able to lock row item...
              if (Row != (SS_COORD)-1 && 
                  (lpRow = SS_LockRowItem(lpSS, Row)))
                {
                // Get other bookmark to replace the one we are using.
                da.sDataFieldIndex = (SHORT)1;
                if (DataCall(&da, GET, UEVENTBMK, lpSS) == 0 && da.lData)
                  vbSS_SetBookmark( (LPVOID)&lpRow->hlstrBookMark, (LPVOID)da.lData);  // use new bookmark
                SS_UnlockRowItem(lpSS, Row);
                }
              ssVBDestroyHlstr(hlstrOldBmk); // free the old bookmark.
              }

            break;
#endif

//       case DATA_FIELD:           /* 75 */
#ifdef SS_BOUNDDEBUG
LogMsg(_T("   DATA_FIELD"));
#endif
         case DATA_DATAFIELDCHANGED:/* 78 */
#ifdef SS_BOUNDDEBUG
LogMsg(_T("   DATA_FIELDCHANGED"));
#endif
            // we'll never get this, since we don't have the DataField 
            // property; in code, someone changed the field from one
            // field to another.
            return (0);
            // was on record 10
            // Available: AddNew (has all Nulls)
            // Request: Update
            // back on record 10
            // will get a data available in response to the update
   // -for our first cut:
   //  everytime we get an available, try to move there
            // HOW DO I KNOW WHAT DATA IS AVAILABLE??
            // -- need to keep track of state so you know the AddNew came
            // -- through;
   // -when you ask for bookmarks and ask for get next bookmark you'll get
   //  an DA_fEOF;
   // -Support the Refresh method to indicate bound data should be updated
        
         case DATA_SAVEDATA:        /* 79 */
         case DATA_READDATA:        /* 80 */
         default:
#ifdef SS_BOUNDDEBUG
LogMsg(_T("   DATA_SAVEDATA/READDATA"));
#endif
            // two methods on the data control: UpdateRecord (READDATA);
            // only refreshes the current data (reread it);
            // After an UpdateRecord (SAVEDATA) means the user is going to
            // save out this data
            return (VBDefControlProc(hCtl, hWnd, Msg, wParam, lParam));
         }

      return (0);
      
   case VBM_DATA_REQUEST:
#ifdef SS_BOUNDDEBUG
LogMsg(_T("VBM_DATA_REQUEST"));
#endif
      // VB will save the data if it knows the data has changed; it checks
      // the standard DataChanged property to determine that.
      
      // the code here will convert the data in the dirty cells to some
      // appropriate data type and transfer it to the database with 
      //   DataCall(&da, SET, FIELDVALUE, lpSS);

      lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

      if (lpSpread->fDataIgnoreDataRequest || !lpSpread->fUpdatable)
         return (0);

      if (lpSpread->RowBeingInserted)
         Row = lpSpread->RowBeingInserted;
      else if (lpSpread->RowBeingSaved)
         Row = lpSpread->RowBeingSaved;
      else
         SS_GetActiveCell(lpSS, &Col, &Row);

      fReReadData = FALSE;
      DataSetRow(lpSS, lObject, lpSpread, lpda, Row, &fReReadData);

      // This code will re-read the data that was just written
#ifdef SS_OCX      //DBOCX - This was commented-out, I added it back
      // If destroyed, exit
      if (!IsWindow(hWnd))
         return 0;

      if (fReReadData)
         {
/* GRB792  Removed again because, at this point,  row has not been added to table.
         FillAutoIncrDataRow(hCtl, hWnd, lpSpread, lpSS, Row, lpda,    //DBOCX
                     lpSpread->dFieldsCount, fUReadNext);
*/
         lpSpread->fReReadRow = TRUE; //re-read row because of autoincr field //GRB792
         }
#endif

      return (0);
   }
   
return (VBDefControlProc(hCtl, hWnd, Msg, wParam, lParam));
}


LONG vbSSBoundVQueryData(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, LPSS_VQUERYDATA lpVQueryData)
{
SS_HCTL       hCtl = (SS_HCTL)lObject;             //SSOCX
SS_COORD      BottomVisCell;
SS_COORD      RowsPerScreen;
SS_COORD      VSize;
SS_COORD      Row;
SS_COORD      RowQuery = lpVQueryData->Row;
SS_COORD      RowStart;
SS_COORD      i;
LPSS_ROW      lpRow;
DATAACCESS    da;
LPVBSPREAD    lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(hWnd));
short         dInc = 1;
short         cFields;
BOOL          fBookMarkSet;
BOOL          fIsRegistered = TRUE;
BOOL          fDefaultRead = TRUE;
BOOL          fUReadNext = FALSE;

SS_AdjustCellCoords(lpSS, NULL, &RowQuery);

// BJO 06Oct97 JOK3732 - Begin fix
#ifdef  BUGS
// Bug-017
if (!GetSpreadAutoFill(hWnd))
    return  FALSE;
#endif
// BJO 06Oct97 JOK3732 - End fix

_fmemset(&da, 0, sizeof(da));
da.usVersion = VB_VERSION;
da.hctlData = (HCTL)ssVBGetDataSourceControl(lObject, &fIsRegistered);
da.hctlBound = (HCTL)hCtl;

cFields = lpSpread->dFieldsCount;

RowStart = RowQuery;

fBookMarkSet = FALSE;

#ifdef FP_OCX
   DataCall(&da, GET, UREADENABLED, lpSS);
   if (da.lData)
      fDefaultRead = FALSE;    // use alternate method
#endif

if (lpSpread->fVRefreshCurrent)
   {
   da.lData = DATA_BOOKMARKCURRENT;
   DataCall(&da, GET, BOOKMARK, lpSS);

   if (RowQuery == lpSS->Row.HeaderCnt)
      {
      RowQuery = 1000;
      RowStart = RowQuery;
      }

   lpSS->Row.CurAt = RowQuery + lpSS->Row.HeaderCnt - 1;

   if (lpSS->Row.Max < SS_VMAXCOORD)
      {
      lpSS->Virtual.VMax = -1;
      lpSS->Row.Max = SS_VMAXCOORD;
      InvalidateRect(hWnd, NULL, TRUE);

      BottomVisCell = SS_GetBottomCell(lpSS, lpSS->Row.UL);
      RowsPerScreen = BottomVisCell - lpSS->Row.UL + 1;
      VSize = max(lpSS->Virtual.VSize, RowsPerScreen);

      if (lpVQueryData->RowsNeeded <= VSize)
         {
         lpVQueryData->RowsNeeded = VSize;
         InvalidateRect(hWnd, NULL, TRUE);
         }
      }
   }

else if (RowQuery == lpSS->Row.HeaderCnt || lpVQueryData->wDirection == SS_VTOP)
   {
   if (fDefaultRead)
      {
      da.lData = DATA_BOOKMARKFIRST;
      DataCall(&da, GET, BOOKMARK, lpSS);
      }
#ifdef FP_OCX
   else
      {
      if ((DataCall(&da, GET, UREADFIRST, lpSS) == 0) || (da.fs & DA_fEOF))
         {
         // if not EOF, get bookmark
         if (!(da.fs & DA_fEOF))
            DataCall(&da, GET, UREADBOOKMARK, lpSS);
         }
      }
#endif
   }

else if (lpVQueryData->wDirection == SS_VBOTTOM)
   {
   da.lData = DATA_BOOKMARKLAST;
   DataCall(&da, GET, BOOKMARK, lpSS);
   dInc = -1;
   RowStart = RowQuery + lpVQueryData->RowsNeeded - 1;
   }

else if (lpVQueryData->wDirection == SS_VREFRESH)
   {
	if (lpSS->hlstrVRefreshBookmark)
      {
      da.lData = (long)lpSS->hlstrVRefreshBookmark;
      fBookMarkSet = TRUE;
      }

   else if (lpRow = SS_LockRowItem(lpSS, RowQuery))
      {
      if (lpRow->hlstrBookMark)
         {
         da.lData = (long)lpRow->hlstrBookMark;
         fBookMarkSet = TRUE;
         }

      SS_UnlockRowItem(lpSS, RowQuery);
      }

   if (!fBookMarkSet)
      vbSSBoundGotoRow(lpSS, lObject, &da, RowQuery);
   }

else if (lpVQueryData->wDirection == SS_VDOWN)
   {
   if (lpRow = SS_LockRowItem(lpSS, RowQuery - 1))
      {
      if (lpRow->hlstrBookMark)
         {
         da.hlstrBookMark = lpRow->hlstrBookMark;

         da.lData = DATA_BOOKMARKNEXT;
         DataCall(&da, GET, BOOKMARK, lpSS);

         fBookMarkSet = TRUE;
         }

      SS_UnlockRowItem(lpSS, RowQuery - 1);
      }

   if (!fBookMarkSet)
      vbSSBoundGotoRow(lpSS, lObject, &da, RowQuery);
   /*
   else
      {
      da.lData = DATA_BOOKMARKCURRENT;
      DataCall(&da, GET, BOOKMARK, lpSS);
      }
   */
   }

else if (lpVQueryData->wDirection == SS_VUP)
   {
   if (lpRow = SS_LockRowItem(lpSS, RowQuery +
                              lpVQueryData->RowsNeeded))
      {
      if (lpRow->hlstrBookMark)
         {
         da.hlstrBookMark = lpRow->hlstrBookMark;

         da.lData = DATA_BOOKMARKPREV;
         DataCall(&da, GET, BOOKMARK, lpSS);

         fBookMarkSet = TRUE;
         }

      SS_UnlockRowItem(lpSS, RowQuery + lpVQueryData->RowsNeeded);
      }

   if (!fBookMarkSet)
      {
      vbSSBoundGotoRow(lpSS, lObject, &da, RowQuery + lpVQueryData->RowsNeeded - 1);
      //da.lData = (long)da.hlstrBookMark;  // BJO 18Aug97 GIL1218 - Remove
      }
   /*
   else
      {
      da.lData = DATA_BOOKMARKCURRENT;
      DataCall(&da, GET, BOOKMARK, lpSS);
      }
   */

   dInc = -1;
   RowStart = RowQuery + lpVQueryData->RowsNeeded - 1;
   }

lpVQueryData->RowsLoaded = 0;

for (Row = RowStart, i = 0; i < lpVQueryData->RowsNeeded &&
     !(da.fs & (DA_fEOF | DA_fBOF)); Row += dInc, i++)
   {
   da.hlstrBookMark = (HLSTR)da.lData;

   FillDataRow(lObject, hWnd, lpSpread, lpSS, Row, &da, cFields, fUReadNext);
   lpVQueryData->RowsLoaded++;

   if (da.hlstrBookMark)
      {                               /* store the bookmark w/ the row data */
      if (lpRow = SS_AllocLockRow(lpSS, Row))
         {
         vbSS_SetBookmark( (LPVOID)&lpRow->hlstrBookMark, 
           (LPVOID)da.hlstrBookMark);
         SS_UnlockRowItem(lpSS, Row);
         }
      }

#ifdef FP_OCX
   if (!fDefaultRead && dInc > 0)
      {
      if (DataCall(&da, GET, UREADNEXT, lpSS) == 0)
         DataCall(&da, GET, UREADBOOKMARK, lpSS);
      else
         da.fs |= DA_fEOF;

      fUReadNext = TRUE;
      }
   else
#endif
      {
      if (dInc > 0)
         da.lData = DATA_BOOKMARKNEXT;
      else
         da.lData = DATA_BOOKMARKPREV;

      DataCall(&da, GET, BOOKMARK, lpSS);
      }
   }

if (lpVQueryData->RowsLoaded)
   {
   if (lpVQueryData->RowsLoaded < lpVQueryData->RowsNeeded)
      {
      if (dInc == 1)
         lpVQueryData->fAtBottom = TRUE;
      else
         {
         lpVQueryData->fAtTop = TRUE;

         SS_MoveRange(lpSS, SS_ALLCOLS, RowQuery +
                      (lpVQueryData->RowsNeeded - lpVQueryData->RowsLoaded),
                      SS_ALLCOLS, RowQuery + lpVQueryData->RowsNeeded - 1,
                      SS_ALLCOLS, RowQuery);
         }
      }
	else if (da.fs & DA_fEOF)
      lpVQueryData->fAtBottom = TRUE;
	else if (da.fs & DA_fBOF)
      lpVQueryData->fAtTop = TRUE;

   // RFW - 5/6/99 - SPR_VIR_001_001
   // if (RowQuery == 1 || lpVQueryData->wDirection == SS_VTOP)
	if ((lpVQueryData->wDirection == SS_VREFRESH && RowQuery == 1) ||
		 lpVQueryData->wDirection == SS_VTOP)
      VBSSBoundSetColWidth(lpSS, hWnd, lpSpread);
   }
else
   {
   if (dInc == 1)
      lpVQueryData->fAtBottom = TRUE;
   else
      lpVQueryData->fAtTop = TRUE;
   }

VBSSBoundFinishClipData(hWnd, lpSS, RowQuery, RowQuery +
                        lpVQueryData->RowsLoaded - 1, cFields);

// rfw - KEM4 - 3/26/99 I added !lpSpread->fVRefreshCurrent
#ifdef SS_VB
if (!lpSS->fProcessingDataAvailable && !lpSS->fProcessingDataRequest)
#endif
/* RFW - 11/12/03 - 12653
	if (!lpSpread->fVRefreshCurrent)
*/
	if (!lpSpread->fVRefreshCurrent && !lpSS->fPrintingInProgress)
		vbSpreadBoundClick(lpSS, lObject, lpSS->Row.CurAt);

SS_AdjustCellCoordsOut(lpSS, NULL, &RowQuery);
lpVQueryData->Row = RowQuery;

return (TRUE);
}


BOOL vbSSBoundVClearData(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, LPSS_VCLEARDATA lpVClearData)
{
SS_HCTL    hCtl = (SS_HCTL)lObject;             //SSOCX
LPVBSPREAD lpSpread;
SS_COORD   Row = lpVClearData->Row;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

SS_AdjustCellCoords(lpSS, NULL, &Row);

if (lpSpread->DataOpts.fAutoSave)
   VBSSBoundManualSave(lpSS, lObject, hWnd, lpSpread, Row,
                       Row + lpVClearData->RowsBeingCleared, FALSE);

return (TRUE);
}


void vbSSBoundGotoRow(LPSPREADSHEET lpSS, LONG lObject, LPDATAACCESS lpda, SS_COORD RowDest)
{
SS_COORD Row;

lpda->lData = DATA_BOOKMARKFIRST;
DataCall(lpda, GET, BOOKMARK, lpSS);
lpda->hlstrBookMark = (HLSTR)NULL;

for (Row = lpSS->Row.HeaderCnt; Row < RowDest && !(lpda->fs & DA_fEOF); Row++)
   {
// lpda->hlstrBookMark = (HLSTR)lpda->lData;
   // frees any PREVOIUS bookmark (if OCX)
   vbSS_SetBookmark( (LPVOID)&lpda->hlstrBookMark, (LPVOID)lpda->lData);

   lpda->lData = DATA_BOOKMARKNEXT;
   DataCall(lpda, GET, BOOKMARK, lpSS);
   }
}


/* --- respond to a click on the bound data control --- */

BOOL vbSpreadBoundClick(LPSPREADSHEET lpSS, LONG lObject, SS_COORD Row)
{
SS_HCTL        hCtl = (SS_HCTL)lObject;            //SSOCX
LPSS_ROW       lpRow;
LPVBSPREAD     lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(hWnd));
DATAACCESS     da;
BOOL           fIsRegistered;
BOOL           fEmpty = FALSE;
BOOL           fRet = TRUE;

if (!lpSS->fProcessingBoundMove && !lpSpread->PostAddRowNew)
   {
   fRet = FALSE;

   _fmemset(&da, 0, sizeof(da));
   da.usVersion = VB_VERSION;
   da.hctlData = (HCTL)ssVBGetDataSourceControl(lObject, &fIsRegistered);
   da.hctlBound = (HCTL)hCtl;

   lpSpread->fInternalSetBookmark = TRUE;

   if (lpRow = SS_LockRowItem(lpSS, Row))
      {
      if (lpRow->hlstrBookMark)
         {
         da.hlstrBookMark = lpRow->hlstrBookMark;
         DataCall(&da, METHOD, BOOKMARK, lpSS);
         fRet = TRUE;
         }

      SS_UnlockRowItem(lpSS, Row);
      }

   if (!fRet)
      {
      da.lData = DATA_BOOKMARKCURRENT;
      if (DataCall(&da, GET, BOOKMARK, lpSS) != 0)
         fEmpty = TRUE;
      else 
      {
         if (da.fs & (DA_fBOF | DA_fEOF | 0x8000)) // BJO 02Jun97 SEL8176 - Added 0x8000
           fEmpty = TRUE;
         // free the bookmark
         vbSS_SetBookmark( (LPVOID)&da.lData, NULL);
      }  

      if (!fEmpty)
         {
         da.hlstrBookMark = 0;
         DataCall(&da, METHOD, BOOKMARK, lpSS); // NOTE: does not phyiscally move - DBOCX
         }
      }

   lpSpread->fInternalSetBookmark = FALSE;
   }

return (fRet);
}


/* --- move the active row to the current record --- */
void MoveHighlight(LPSPREADSHEET lpSS, LONG lObject, LPDATAACCESS lpda, short Action)
{
SS_COORD Row;
SS_COORD RowFirst;
SS_COORD RowLast;
SS_COORD RowInc;
SS_COORD MaxRows;
BOOL     fFound;

if (lpSS->fVirtualMode)
   {
   lpSS->fProcessingBoundMove = TRUE;

   switch (Action)
      {
      case DATA_MOVEFIRST:
         SS_ScrollHome(lpSS, SS_F_SCROLL_COL | SS_F_MOVEACTIVECELL);
         break;

      case DATA_MOVENEXT:
         SS_ScrollLineLR(lpSS, SS_F_SCROLL_DOWN | SS_F_MOVEACTIVECELL);
         break;

      case DATA_MOVEPREV:
         SS_ScrollLineUL(lpSS, SS_F_SCROLL_UP | SS_F_MOVEACTIVECELL);
         break;

      case DATA_MOVELAST:
         SS_ScrollEnd(lpSS, SS_F_SCROLL_COL | SS_F_MOVEACTIVECELL | SS_F_SCROLLMAX);
         break;
      }

   lpSS->fProcessingBoundMove = FALSE;
   }

else
   {
   lpda->hlstrBookMark = NULL;
   lpda->lData = DATA_BOOKMARKCURRENT;
   DataCall(lpda, GET, BOOKMARK, lpSS);
   if (!lpda->lData)
      return;

   lpda->hlstrBookMark = (HLSTR)lpda->lData;

   MaxRows = lpSS->Row.AllocCnt;

   switch (Action)
      {
      case DATA_MOVEFIRST:
         RowFirst = lpSS->Row.HeaderCnt;
         RowLast = MaxRows - 1;
         RowInc = 1;
         break;

      case DATA_MOVENEXT:
         RowFirst = lpSS->Row.CurAt + 1;
         RowLast = MaxRows - 1;
         RowInc = 1;
         break;

      case DATA_MOVEPREV:
         RowFirst = lpSS->Row.CurAt - 1;
         RowLast = lpSS->Row.HeaderCnt;
         RowInc = -1;
         break;

      case DATA_MOVELAST:
         RowFirst = MaxRows - 1;
         RowLast = lpSS->Row.HeaderCnt;
         RowInc = -1;
         break;
      }

   for (Row = RowFirst, fFound = FALSE; !fFound && Row >= lpSS->Row.HeaderCnt && Row < MaxRows;
        Row += RowInc)
      fFound = MoveHighlightTestRow(lpda, lpSS, Row);

   if (!fFound)
     {
     RowInc *= -1;

     for (Row = RowFirst + RowInc; !fFound && Row >= lpSS->Row.HeaderCnt && Row < MaxRows;
          Row += RowInc)
        fFound = MoveHighlightTestRow(lpda, lpSS, Row);
     }

   // free the bookmark
   if (lpda->hlstrBookMark)
     vbSS_SetBookmark((LPVOID)&lpda->hlstrBookMark, NULL);
   }
}


/* --- move the active row to the current record --- */
BOOL MoveHighlightTestRow(LPDATAACCESS lpda, LPSPREADSHEET lpSS, SS_COORD Row)
{
BOOL fFound = FALSE;

if (ssCompareRowWithBookmark(lpda->hlstrBookMark, lpSS, Row)) //DBOCX
   {
	// RFW - 7/13/99 - I added this conditional because when column
	// headers were being clicked on, the selection be being deselected
	// for the ADO build.
	if (Row != lpSS->Row.CurAt)
	{
	//Add by BOC 99.8.2 (hyt)--------------------
	//for set active cell to other row should remove mousemove message
	//if not will generate some incorrect selection.
		MSG Msg;
		SS_SetActiveCell(lpSS, lpSS->Col.CurAt, Row);
		PeekMessage(&Msg,lpSS->lpBook->hWnd,WM_MOUSEMOVE,WM_MOUSEMOVE,PM_REMOVE);
	//--------------------------------------------
	}
   fFound = TRUE;
   }

return (fFound);
}

//DBOCX...
/* --- Compare the bookmark for this row with the bookmark passed ---
       Returns TRUE if they match.                   */
static BOOL ssCompareRowWithBookmark(HLSTR hlstrBookmark, LPSPREADSHEET lpSS,
                                     SS_COORD Row)
{
LPSS_ROW lpRow;
BOOL     fFound = FALSE;

if (lpRow = SS_LockRowItem(lpSS, Row))
   {
   fFound = BookmarksEqual( hlstrBookmark, lpRow->hlstrBookMark);
	SS_UnlockRowItem(lpSS, Row);
   }

return (fFound);
}
//DBOCX



/* --- place default headings in the spreadsheet --- */
void SetDataHeadings(LPSPREADSHEET lpSS, LONG lObject, LPDATAACCESS lpda)
{
SS_HCTL  hCtl = (SS_HCTL)lObject;            //SSOCX
LPSS_COL lpCol;
SS_COORD i;

lpda->hszDataField = NULL;

for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
   {
   if (lpCol = SS_LockColItem(lpSS, i))
      {
      if (lpCol->dFieldNum)
         {
         lpda->sDataFieldIndex = lpCol->dFieldNum - 1;
         DataCall(lpda, GET, FIELDNAME, lpSS);
         if (lpda->lData)
            {
            SS_SetDataRange(lpSS, i, SS_GetColHeadersUserSortRow(lpSS), i, SS_GetColHeadersUserSortRow(lpSS),
                            ssVBDerefHsz((HSZ)lpda->lData), FALSE, FALSE, TRUE);
            ssVBDestroyHsz((HSZ)lpda->lData);
            }
         }

      SS_UnlockColItem(lpSS, i);
      }
   }
}

/* --- define cell types for each column of data --- */
void SetDataCellTypes(LONG lObject, HWND hWnd, LPDATAACCESS lpda, short cFields,
                      LPVBSPREAD lpSpread, LPSPREADSHEET lpSS)
{
/* lpda is initialized before entry */
SS_HCTL       hCtl = (SS_HCTL)lObject;             //SSOCX
LPSS_COL      lpCol;
SS_COORD      cFieldsTemp;
SS_COORD      i;

if (!lpSS->DataFieldNameCnt)
   {
   cFieldsTemp = cFields;
#ifdef SS_UTP
   if (lpSS->fUseScrollArrows)
      cFieldsTemp = cFields + 1;
#endif
   if (SS_GetMaxCols(lpSS) != cFieldsTemp)
      {
      SS_SetMaxCols(lpSS, cFieldsTemp);
#ifdef SS_VB
      lpSpread->lMaxCols = SS_GetMaxCols(lpSS);
#endif
      }
   }

lpda->hszDataField = NULL;

for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
   {
   if (lpCol = SS_LockColItem(lpSS, i))
      {
      if (lpCol->dFieldNum)
         {
         lpda->sDataFieldIndex = lpCol->dFieldNum - 1;
         SetDataCellTypes2(lObject, hWnd, lpda, lpSpread, lpSS, i);
         }

      SS_UnlockColItem(lpSS, i);
      }
   }
}


/* --- define cell types for each column of data --- */
void SetDataCellTypes2(LONG lObject, HWND hWnd, LPDATAACCESS lpda, LPVBSPREAD lpSpread,
                       LPSPREADSHEET lpSS, SS_COORD Col)
{
/* lpda is initialized before entry */
SS_HCTL        hCtl = (SS_HCTL)lObject;            //SSOCX
LPTSTR         lpszFieldName;
HSZ            hszFieldName = NULL;
USHORT         usDataType;

DataCall(lpda, GET, FIELDTYPE, lpSS);
usDataType = (USHORT)LOWORD(lpda->lData);

SetDataCellTypes3(hWnd, lpSS, Col, usDataType);

/* set the default heading if requested */

DataCall(lpda, GET, FIELDNAME, lpSS);
hszFieldName = (HSZ)lpda->lData;

if (hszFieldName)
   {
   lpszFieldName = ssVBDerefHsz(hszFieldName);

   if (lpSpread->DataOpts.fHeadings)
      SS_SetDataRange(lpSS, Col, SS_GetColHeadersUserSortRow(lpSS), Col, SS_GetColHeadersUserSortRow(lpSS),
                      lpszFieldName, FALSE, FALSE, TRUE);
   }
else
   lpszFieldName = NULL;

SS_AdjustCellCoordsOut(lpSS, &Col, NULL);
VBSSBoundFireDataColConfig(lObject, lpSpread, lpszFieldName, Col, usDataType, lpSS);

if (hszFieldName)
   ssVBDestroyHsz(hszFieldName);
}


/* --- define cell types for each column of data --- */
void SetDataCellTypes3(HWND hWnd, LPSPREADSHEET lpSS, SS_COORD Col, USHORT usDataType)
{
SS_CELLTYPE CellType;

switch (usDataType)
   {
   case DATA_VT_BOOL:                                 /* boolean (short) */
      SSSetTypeCheckBox(hWnd, &CellType, BS_CENTER, NULL, NULL, 0, NULL, 0,
                        NULL, 0, NULL, 0, NULL, 0, NULL, 0);
      break;

#ifdef SS_V40
   case DATA_VT_VAR_BYTE:                               /* unsigned char */
		if (lpSS->lpBook->fDAutoCellTypesOldStyle)
	      SSSetTypeInteger(hWnd, &CellType, 0, UCHAR_MAX);
		else
			SSSetTypeNumber(hWnd, &CellType, 0, 0, 0, UCHAR_MAX, FALSE);
      break;

   case DATA_VT_INTEGER:                                        /* short */
		if (lpSS->lpBook->fDAutoCellTypesOldStyle)
			SSSetTypeInteger(hWnd, &CellType, SHRT_MIN, SHRT_MAX);
		else
			SSSetTypeNumber(hWnd, &CellType, 0, 0, SHRT_MIN, SHRT_MAX, FALSE);
      break;

   case DATA_VT_LONG:
		if (lpSS->lpBook->fDAutoCellTypesOldStyle)
	      SSSetTypeInteger(hWnd, &CellType, LONG_MIN, LONG_MAX);
		else
			SSSetTypeNumber(hWnd, &CellType, 0, 0, LONG_MIN, LONG_MAX, FALSE);
      break;

   case DATA_VT_SINGLE:
   case DATA_VT_DOUBLE:
		if (lpSS->lpBook->fDAutoCellTypesOldStyle)
	      SSSetTypeFloat(hWnd, &CellType, FS_SEPARATOR, 12,
                     2, -999999999999.99, 999999999999.99); //@@min/max
		else
			SSSetTypeNumber(hWnd, &CellType, 0, 2, -999999999999.99, 999999999999.99, TRUE);
      break;

   case DATA_VT_CURRENCY:
		if (lpSS->lpBook->fDAutoCellTypesOldStyle)
	      SSSetTypeFloat(hWnd, &CellType, FS_SEPARATOR | FS_MONEY, 12,
                     2, -999999999999.99, 999999999999.99); //@@min/max
		else
			SSSetTypeCurrency(hWnd, &CellType, 0, 2, -999999999999.99, 999999999999.99, TRUE);
      break;

#else // SS_V40

   case DATA_VT_VAR_BYTE:                               /* unsigned char */
#if (!defined(SS_NOCT_INT) && !defined(SS_UTP))
      SSSetTypeInteger(hWnd, &CellType, 0, UCHAR_MAX);
#else
      SSSetTypeFloat(hWnd, &CellType, FS_SEPARATOR, 3, 0, 0.0,
                     (double)UCHAR_MAX);
#endif
      break;

   case DATA_VT_INTEGER:                                        /* short */
#if (!defined(SS_NOCT_INT) && !defined(SS_UTP))
      SSSetTypeInteger(hWnd, &CellType, SHRT_MIN, SHRT_MAX);
#else
      SSSetTypeFloat(hWnd, &CellType, FS_SEPARATOR, 5, 0, (double)SHRT_MIN,
                     (double)SHRT_MAX);
#endif
      break;

   case DATA_VT_LONG:
#if (!defined(SS_NOCT_INT) && !defined(SS_UTP))
      SSSetTypeInteger(hWnd, &CellType, LONG_MIN, LONG_MAX);
#else
      SSSetTypeFloat(hWnd, &CellType, FS_SEPARATOR, 10, 0, (double)LONG_MIN,
                     (double)LONG_MAX);
#endif
      break;

   case DATA_VT_SINGLE:
      SSSetTypeFloat(hWnd, &CellType, FS_SEPARATOR, 12,
                     2, -999999999999.99, 999999999999.99); //@@min/max
      break;

   case DATA_VT_DOUBLE:
      SSSetTypeFloat(hWnd, &CellType, FS_SEPARATOR, 12,
                     2, -999999999999.99, 999999999999.99); //@@min/max
      break;

   case DATA_VT_CURRENCY:
      SSSetTypeFloat(hWnd, &CellType, FS_SEPARATOR | FS_MONEY, 12,
                     2, -999999999999.99, 999999999999.99); //@@min/max
      break;
#endif // SS_V40

#if 0
   case DATA_VT_MEMO:                           /* unlimited length text */
   case DATA_VT_TEXT:                               /* fixed length text */
      lpda->lData = 0;
      DataCall(lpda, GET, FIELDSIZE, lpSS);  // test: does this specify the max?
      SSSetTypeEdit(hWnd, &CellType, lpda->lData > 100 ? ES_MULTILINE : 0,
                    (short)lpda->lData, SS_CHRSET_CHR, SS_CASE_NOCASE);
      break;
#endif

   case DATA_VT_TEXT:                               /* fixed length text */
#ifdef SPREAD_JPN
// 96' 5/24 BOC Gao. for make fixed length to 255.
      SSSetTypeEdit(hWnd, &CellType, ES_LEFT | ES_AUTOHSCROLL, 255,
                    SS_CHRSET_CHR, SS_CASE_NOCASE);
// ----------------------------<<
#if 0      
      //- JPNFIX0020 - (Masanori Iwasa)
      {
      LPSPREADSHEET  lpSS;
      LPSS_COL       lpCol;
      DATAACCESS     da;
      HCTL           hCtl;
      BOOL           fIsRegistered;

      _fmemset(&da, 0, sizeof(DATAACCESS));
      
      hCtl         = VBGetHwndControl(hWnd);
      da.usVersion = VB_VERSION;
      da.hctlData  = VBGetDataSourceControl(hCtl, &fIsRegistered);
      da.hctlBound = hCtl;
      da.lData = 0;
          
      if (lpCol = SS_LockColItem(lpSS, Col))
      {
         if (lpCol->dFieldNum)
            da.sDataFieldIndex = lpCol->dFieldNum - 1;
         DataCall(&da, GET, FIELDSIZE, lpSS);  
         SS_UnlockColItem(lpSS, Col);
      }
      else
         da.lData = 32000;

      SSSetTypeEdit(hWnd, &CellType, ES_LEFT | ES_AUTOHSCROLL,
                            (short)da.lData, SS_CHRSET_CHR, SS_CASE_NOCASE);
      }
#endif
#else
#ifdef SS_V70
      SSSetTypeEdit(hWnd, &CellType, ES_LEFT | ES_AUTOHSCROLL, 32000,
                    SS_CHRSET_CHR, SS_CASE_NOCASE);
#else
      SSSetTypeEdit(hWnd, &CellType, ES_LEFT | ES_AUTOHSCROLL, 60,
                    SS_CHRSET_CHR, SS_CASE_NOCASE);
#endif // SS_V70
#endif
      break;

   case DATA_VT_MEMO:                           /* unlimited length text */
      SSSetTypeEdit(hWnd, &CellType, ES_LEFT | ES_MULTILINE, 32000,
                    SS_CHRSET_CHR, SS_CASE_NOCASE);
      break;

   case DATA_VT_DATETIME:   /* double: int portion is date, frac is time */
      /* initialize to the date portion only;  use win.ini settings */
      SSSetTypeDate(hWnd, &CellType, DS_DEFCENTURY, NULL, NULL, NULL);
      break;

   case DATA_VT_BINARY:
      SSSetTypePicture(hWnd, &CellType, 0, 0L);
      break;

   }

SS_SetCellTypeRange(lpSS, Col, SS_ALLROWS, Col, SS_ALLROWS, &CellType);
}


/* --- read all rows data into the spread --- */
void FillAllDataRows(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, LPDATAACCESS lpda, short cFields)
{
/* lpda is initialized before entry */
SS_HCTL    hCtl = (SS_HCTL)lObject;             //SSOCX
SS_COORD   Row;
LPSS_ROW   lpRow;
LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(hWnd));
BOOL       fRedraw;
BOOL       fDefaultRead = TRUE;

fRedraw = lpSS->lpBook->Redraw;
lpSS->lpBook->Redraw = FALSE;

#ifdef FP_OCX
   DataCall(lpda, GET, UREADENABLED, lpSS);
   if (lpda->lData)
      {
      if ((DataCall(lpda, GET, UREADFIRST, lpSS) == 0) || (lpda->fs & DA_fEOF))
         {
         fDefaultRead = FALSE;    // use alternate method
         // if not EOF, get bookmark
         if (!(lpda->fs & DA_fEOF))
            DataCall(lpda, GET, UREADBOOKMARK, lpSS);
         }
      }      
#endif

if (fDefaultRead)
   {
   lpda->lData = DATA_BOOKMARKFIRST;
   //DBOCX...
   // if an error was returned, act as if it was EOF
   if (DataCall(lpda, GET, BOOKMARK, lpSS) != 0)
      lpda->fs |= DA_fEOF;
   //DBOCX
   }

for (Row = lpSS->Row.HeaderCnt; !(lpda->fs & DA_fEOF); Row++)
   {
   lpda->hlstrBookMark = (HLSTR)lpda->lData;

   if (SS_GetRowCnt(lpSS) <= Row)            /* ensure there are enough rows */
      {
      SS_SetMaxRows(lpSS, Row - lpSS->Row.HeaderCnt + 1);
#ifdef SS_VB
      lpSpread->lMaxRows = SS_GetMaxRows(lpSS);
#endif
      }

   FillDataRow(lObject, hWnd, lpSpread, lpSS, Row, lpda, cFields, !fDefaultRead);

   if (lpda->hlstrBookMark)
      {                               /* store the bookmark w/ the row data */
      if (lpRow = SS_AllocLockRow(lpSS, Row))
         {
         vbSS_SetBookmark( (LPVOID)&lpRow->hlstrBookMark, 
           (LPVOID)lpda->hlstrBookMark);
         SS_UnlockRowItem(lpSS, Row);
         }
      }

#ifdef FP_OCX
   if (!fDefaultRead)
      {
      if (DataCall(lpda, GET, UREADNEXT, lpSS) == 0)
         DataCall(lpda, GET, UREADBOOKMARK, lpSS);
      else
         lpda->fs |= DA_fEOF;
      }
#endif
   if (fDefaultRead)
      {
      lpda->lData = DATA_BOOKMARKNEXT;
      DataCall(lpda, GET, BOOKMARK, lpSS);
      }
   }

#ifdef SS_V70
OnDataLoaded(lObject);
#endif // SS_V70

VBSSBoundFinishClipData(hWnd, lpSS, lpSS->Row.HeaderCnt, Row, cFields);

VBSSBoundSetColWidth(lpSS, hWnd, lpSpread);
SS_BookSetRedraw(lpSS->lpBook, fRedraw);
}


/* --- fill a row of the spread with fields from the data control --- */
void FillDataRow(LONG lObject, HWND hWnd, LPVBSPREAD lpSpread,
                 LPSPREADSHEET lpSS, SS_COORD Row, LPDATAACCESS lpda,
                 short cFields, BOOL fUReadNext)
{
/* lpda is initialized before entry; don't change lpda->hlstrBookMark */
SS_HCTL  hCtl = (SS_HCTL)lObject;            //SSOCX
LPSS_COL lpCol;
SS_COORD i;

#ifdef SS_BOUNDDEBUG
{
TCHAR Buffer[80];

wsprintf(Buffer, _T("Bookmark retrieved - Reading Row #%ld"), Row);
LogMsg(Buffer);
}
#endif

lpda->hszDataField = NULL;
lpSpread->fUReadNext = fUReadNext;

for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
   {
   if (lpCol = SS_LockColItem(lpSS, i))
      {
      if (lpCol->dFieldNum)
         {
         lpda->sDataFieldIndex = lpCol->dFieldNum - 1;
         FillDataRow2(lObject, hWnd, lpSpread, lpSS, Row, lpda, i, fUReadNext);
         }

      SS_UnlockColItem(lpSS, i);
      }
   }

lpSpread->fUReadNext = FALSE;
}


/* --- Fill a row of the spread with fields from the data control which
       are strictly AUTO-INCREMENT fields.                              --- */
void FillAutoIncrDataRow(LONG lObject, HWND hWnd, LPVBSPREAD lpSpread,
                 LPSPREADSHEET lpSS, SS_COORD Row, LPDATAACCESS lpda,
                 short cFields, BOOL fUReadNext)
{
/* lpda is initialized before entry; don't change lpda->hlstrBookMark */
SS_HCTL  hCtl = (SS_HCTL)lObject;            //SSOCX
LPSS_COL lpCol;
SS_COORD i;

lpda->hszDataField = NULL;

for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
   {
   if (lpCol = SS_LockColItem(lpSS, i))
      {
      if (lpCol->dFieldNum)
         {
         lpda->sDataFieldIndex = lpCol->dFieldNum - 1;
         DataCall(lpda, GET, FIELDATTRIBUTES, lpSS);
         if ((short)lpda->lData & 16)          // only if AutoIncrement
            FillDataRow2(lObject, hWnd, lpSpread, lpSS, Row, lpda, i, fUReadNext);
         }

      SS_UnlockColItem(lpSS, i);
      }
   }
}

/* --- fill a row of the spread with fields from the data control --- */
void FillDataRow2(LONG lObject, HWND hWnd, LPVBSPREAD lpSpread,
                  LPSPREADSHEET lpSS, SS_COORD Row, LPDATAACCESS lpda,
                  SS_COORD Col, BOOL fUReadNext)
{
/* lpda is initialized before entry; don't change lpda->hlstrBookMark */
SS_HCTL        hCtl = (SS_HCTL)lObject;            //SSOCX
VALUE          Value;
#ifndef SS_OCX
VARIANT        Variant;
#endif
TCHAR          sz[85];
LPTSTR         lpsz;
BOOL           fDataFillEvent;
SS_CELLTYPE    CellType;
USHORT         usDataType = 0;
LPSS_COL       lpCol;
LONG           lData;
LONG           lDataCol;
double         dfVal;

/* [notes]
   There are four data representation spaces used here:
   (1) database types (e.g. DATA_VT_SINGLE) from the data control
   (2) VB property types (e.g. DT_REAL) for requesting data
   (3) cell types (e.g. SS_TYPE_FLOAT) set by us or the developer
   (4) variant types (e.g. VT_R4) used when the user validates data
   Since data may only be retrieved using the DT_* types, the DATA_VT_*
   types are useful only to determine the DATA_VT_* to DT_* mapping

   [algorithm] 
   for each column:
   (1) allow the user to examine the data (if they requested to) via 
        the DataFill event.  They can retrieve the data via an API in 
        a variant variable; the API maps the variable to the VT_* type
        most closely matching the DATA_VT_* type
   (2) retrieve the data according to the current cell type
   (3) if they cancel our processing of the column, go to (1)
   (4) if they changed the cell type, reretrieve the data based on the
        new type and put it into the cell.
*/

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

fDataFillEvent = lpSS->fDataFillEvent; // RFW - 2/20/00 - GIC11069

SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

if (lpCol = SS_LockColItem(lpSS, Col))
   {
   // RFW - 2/20/00 - GIC11069
   if (lpCol->bDataFillEvent == SS_DATAFILLEVENT_YES)
      fDataFillEvent = TRUE;
   else if (lpCol->bDataFillEvent == SS_DATAFILLEVENT_NO)
      fDataFillEvent = FALSE;

   usDataType = lpCol->bDataType;

   SS_UnlockColItem(lpSS, Col);
   }

if (!usDataType)
   {
   if (lpCol = SS_AllocLockCol(lpSS, Col))
      {
      DataCall(lpda, GET, FIELDTYPE, lpSS);
      usDataType = (USHORT)LOWORD(lpda->lData);

      lpCol->bDataType = (BYTE)usDataType;
      SS_UnlockColItem(lpSS, Col);
      }
   }

if (fDataFillEvent)
   {
   if (lpda->sDataFieldIndex)
      lDataCol = lpda->sDataFieldIndex;
   else
      {
      DataCall(lpda, GET, FIELDPOSITION, lpSS);
      lDataCol = lpda->lData;
      }

   if (VBSSBoundFireDataFill(lpSS, lObject, lpSpread, Col, Row, -1, usDataType,
                             (long)lpda->hlstrBookMark, lDataCol))
      return;                    /* they don't want us to set the data */

   SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);
   }

if (!MapColData(lObject, &lData, lpda, usDataType, CellType.Type, fUReadNext, lpSS))
   return;

switch (CellType.Type)
   {
   case SS_TYPE_DATE:
   case SS_TYPE_TIME:
      if (usDataType != DATA_VT_DATETIME)
         return;

      /*
      dfVal = *(float _far *)&lData;

      if (CellType.Type == SS_TYPE_DATE && (long)dfVal == 0 &&
          dfVal - (double)(long)dfVal != 0)
         {
         SSSetTypeTime(hWnd, &CellType, 0, NULL, NULL, NULL);
         SSSetCellType(hWnd, Col, SS_ALLROWS, &CellType);
         }

      VBFormat(VT_R8, &dfVal,
               CellType.Type == SS_TYPE_DATE ? "mmddyyyy" : "hhnnss",
               sz, sizeof(sz) - 1);
      SSSetValue(hWnd, Col, Row, sz);
      */

      Value.hlstr = (HLSTR)lpda->lData;

      dfVal = 0.0;
//DBOCX
#ifdef SS_OCX
      if (Value.hlstr)
    {
         fpConvertTSTRtoType( (LPTSTR)Value.hlstr, &dfVal, VT_DATE);
         ssVBDestroyHlstr(Value.hlstr);
    }
#else //not SS_OCX
      _fmemset(&Variant, '\0', sizeof(VARIANT));
      VBSetVariantValue(&Variant, VT_STRING, &Value);
      VBCoerceVariant(&Variant, VT_DATE, &dfVal);
      if (Value.hlstr)
         ssVBDestroyHlstr(Value.hlstr);
#endif
      /*
      dfVal = ((double)(long)(dfVal * 100000)) / 100000;
      */

#ifndef SS_NOCT_TIME
      if (CellType.Type == SS_TYPE_DATE && Row == 1 && (long)dfVal == 0 &&
          dfVal - (double)(long)dfVal != 0)
         {
         SSSetTypeTime(hWnd, &CellType, 0, NULL, NULL, NULL);
         SS_SetCellTypeRange(lpSS, Col, SS_ALLROWS, Col, SS_ALLROWS, &CellType);
         }
#endif

      *sz = '\0';

//      if ((CellType.Type == SS_TYPE_DATE && (long)dfVal != 0) ||
//          (CellType.Type == SS_TYPE_TIME && dfVal - (double)(long)dfVal != 0))
      if ((CellType.Type == SS_TYPE_DATE && (long)dfVal != 0) ||
          (CellType.Type == SS_TYPE_TIME && !(lpda->fs & DA_fNull)))
         {
         // The following code was changed in response to KEM2

         if (CellType.Type == SS_TYPE_DATE)
            {
            TB_DATE Date;

            vbDateJulianToDMY((long)dfVal - 2, &Date);
            DateToString(sz, &Date);
            }
         else
            {
            TIME Time;

            SS_DoubleToTime(dfVal, &Time);
            TimeToString(sz, &Time);
            }

//         ssVBFormat(lObject, VT_R8, &dfVal,
//                  CellType.Type == SS_TYPE_DATE ? _T("mmddyyyy") : _T("hhnnss"),
//                  sz, STRING_SIZE(sz) - 1);
         }

      SS_SetValue(lpSS, Col, Row, sz);
      break;

   case SS_TYPE_STATICTEXT:                       /* requested as DT_HSZ */
   case SS_TYPE_EDIT:
   case SS_TYPE_PIC:
      lpsz = ssVBDerefHsz((HSZ)lpda->lData);
      SS_ClipData(lpSS, Col, Row, lpsz, FALSE, FALSE);
      ssVBDestroyHsz((HSZ)lpda->lData);
      break;

   case SS_TYPE_FLOAT:
#ifdef SS_V40
   case SS_TYPE_CURRENCY:
   case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
	case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
      if (lpda->fs & DA_fNull)
         SS_ClipData(lpSS, Col, Row, _T(""), FALSE, FALSE);
      else
         {
         Value.hlstr = (HLSTR)lpda->lData;
         dfVal = 0.0;

//DBOCX...
#ifdef SS_OCX
         if (Value.hlstr)
				{
				SS_StringToFloat(lpSS, (LPTSTR)Value.hlstr, &dfVal);
				// RFW - 12/18/02 - 11339          StringToFloat((LPTSTR)Value.hlstr, &dfVal);
// RFW - 10/23/02 - 10334            fpConvertTSTRtoType((LPTSTR)Value.hlstr, &dfVal, VT_R8);
            ssVBDestroyHlstr(Value.hlstr);
				}
#else         
         _fmemset(&Variant, '\0', sizeof(VARIANT));
         VBSetVariantValue(&Variant, VT_STRING, &Value);
         VBCoerceVariant(&Variant, VT_R8, &dfVal);
         if (Value.hlstr)
            ssVBDestroyHlstr(Value.hlstr);
#endif
//DBOCX.         
         SS_SetFloatRange(lpSS, Col, Row, Col, Row, dfVal);
         }

      break;

#ifdef SS_V40
   case SS_TYPE_NUMBER:
      if (usDataType == DATA_VT_INTEGER)
			{
			lData = (short)lData;
	      SS_SetIntegerRange(lpSS, Col, Row, Col, Row, lData);
			}

      else if (usDataType == DATA_VT_LONG)
	      SS_SetIntegerRange(lpSS, Col, Row, Col, Row, lData);

		else if (lpda->fs & DA_fNull)
         SS_ClipData(lpSS, Col, Row, _T(""), FALSE, FALSE);
      else
         {
         Value.hlstr = (HLSTR)lpda->lData;
         dfVal = 0.0;

         if (Value.hlstr)
				{
				SS_StringToFloat(lpSS, (LPTSTR)Value.hlstr, &dfVal);
				// RFW - 12/18/02 - 11339          StringToFloat((LPTSTR)Value.hlstr, &dfVal);
// RFW - 10/23/02 - 10334            fpConvertTSTRtoType((LPTSTR)Value.hlstr, &dfVal, VT_R8);
            ssVBDestroyHlstr(Value.hlstr);
				}
         SS_SetFloatRange(lpSS, Col, Row, Col, Row, dfVal);
         }

      break;
#endif // SS_V40

   case SS_TYPE_INTEGER:                         /* requested as DT_LONG */
// fix for bug #8604 -scl
      if (usDataType == DATA_VT_INTEGER)
        lData = (short)lData;
      SS_SetIntegerRange(lpSS, Col, Row, Col, Row, lData);
      break;

   case SS_TYPE_COMBOBOX:
      if (usDataType == DATA_VT_TEXT || usDataType == DATA_VT_MEMO)
         {
         lpsz = ssVBDerefHsz((HSZ)lpda->lData);
         SS_ClipData(lpSS, Col, Row, lpsz, FALSE, FALSE);
         ssVBDestroyHsz((HSZ)lpda->lData);
         }
      else
         {
         wsprintf(sz,_T("%d"), (int)(usDataType == DATA_VT_BOOL ? (lData!=0) :
                  lData));
         SS_SetValue(lpSS, Col, Row, sz);
         }

      break;

   case SS_TYPE_CHECKBOX:
      wsprintf(sz,_T("%d"), (int)(usDataType==DATA_VT_BOOL ? (lData!=0) : lData));
      SS_SetValue(lpSS, Col, Row, sz);
      break;

   case SS_TYPE_PICTURE:
      if (lpda->lData)
         {
         LONG lLen = 0;
#ifndef FP_OCX  //not FP_OCX
         {
         WORD wLen;

         lpsz = ssVBDerefHlstrLen((HLSTR)lpda->lData, &wLen);
         lLen = wLen;
         }
#else
         lpsz = (LPTSTR)lpda->lData;
         lLen = LBlockLen((LBLOCK)lpda->lData);
#endif
         if (lpsz && lLen)
            {
            lpSpread->Col = Col;
            lpSpread->Row = Row;
            VBSSBmpLoad(lObject, hWnd, lpSpread, (HPSTR)lpsz, lLen);
            }
         ssVBDestroyHlstr((HLSTR)lpda->lData);
         }
      break;

   }
}


/* --- retrieve a database field based on the current cell type --- */
BOOL MapColData(LONG lObject, LPLONG lplData, LPDATAACCESS lpda,
                USHORT usDataType, char Type, BOOL fUReadNext, LPSPREADSHEET lpSS)
{
/* determine how to request the data from VB based on the type of this
   cell (with hints from the DATA_VT_* type from the database */
switch (Type)
   {
   case SS_TYPE_DATE:
   case SS_TYPE_TIME:
      if (usDataType != DATA_VT_DATETIME)
         return FALSE;
      lpda->usDataType = DT_HLSTR;
      break;

   case SS_TYPE_STATICTEXT:
   case SS_TYPE_EDIT:
   case SS_TYPE_PIC:
      lpda->usDataType = DT_HSZ;
      break;

   case SS_TYPE_FLOAT:
#ifdef SS_V40
   case SS_TYPE_CURRENCY:
   case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
	case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
      lpda->usDataType = DT_HLSTR;
      break;

#ifdef SS_V40
   case SS_TYPE_NUMBER:
      if (usDataType == DATA_VT_INTEGER || usDataType == DATA_VT_LONG)
	      lpda->usDataType = DT_LONG;
		else
			lpda->usDataType = DT_HLSTR;
      break;
#endif // SS_V40

   case SS_TYPE_INTEGER:
      lpda->usDataType = DT_LONG;
      break;

   case SS_TYPE_COMBOBOX:
      if (usDataType == DATA_VT_TEXT || usDataType == DATA_VT_MEMO)
         lpda->usDataType = DT_HSZ;
      else
         lpda->usDataType = DT_SHORT;
      break;

   case SS_TYPE_CHECKBOX:
      lpda->usDataType = DT_SHORT;
      break;

   case SS_TYPE_PICTURE:
      lpda->usDataType = DT_HLSTR;
      break;

   case SS_TYPE_BUTTON:                          /* nothing to do for these */
   case SS_TYPE_OWNERDRAW:
      break;
   }

lpda->fs = 0L;
lpda->lData = 0L;

#ifdef FP_OCX
if (fUReadNext)
{
  if (DataCall(lpda, GET, UREADFIELDVALUE, lpSS) != 0)
    return (FALSE);
}
else
#endif
   {
   if (DataCall(lpda, GET, FIELDVALUE, lpSS) != 0)
      return (FALSE);
   }

*lplData = lpda->lData;
return (TRUE);
}


BOOL DataSetRow(LPSPREADSHEET lpSS, LONG lObject, LPVBSPREAD lpSpread, LPDATAACCESS lpda,
                SS_COORD Row, LPBOOL lpfReReadData)
{
SS_HCTL  hCtl = (SS_HCTL)lObject;            //SSOCX
LPSS_ROW lpRow;
LPSS_COL lpCol;
SS_COORD i;
BOOL     fRet = TRUE;
BOOL     fValueSet = FALSE;

if (!(ssVBSetControlFlags(lObject, 0L, 0L, lpSS) & CTLFLG_DATACHANGED, lpSS))
   return 0;

lpSpread->Row = Row;

if (lpRow = SS_LockRowItem(lpSS, Row))
   {
   if (lpRow->bDirty == SS_ROWDIRTY_SOME || lpRow->bDirty == SS_ROWDIRTY_ALL)
      {
      lpda->hszDataField = NULL;

      for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
         {
         if (lpCol = SS_LockColItem(lpSS, i))
            {
            if (lpCol->dFieldNum)
               {
               lpda->sDataFieldIndex = lpCol->dFieldNum - 1;

               fRet = DataSetRow2(lpSS, lObject, lpSpread, lpda, i,
                                  lpRow->bDirty, lpfReReadData);
#ifdef SS_OCX  //DBOCX
               // If failed, break from loop
               if (!IsWindow(lpSS->lpBook->hWnd))
                  return FALSE;
#endif  
               // if any set works, set this flag
               if (fRet)
                 fValueSet = TRUE;
               }

            SS_UnlockColItem(lpSS, i);
            }
         }

//    if (fRet)   // not needed since "if" only checks last DataSetRow2()
      lpRow->bDirty = FALSE;
      }

   SS_UnlockRowItem(lpSS, Row);
   }

ssVBSetControlFlags(lObject, CTLFLG_DATACHANGED, 0L, lpSS);

#ifdef SS_OCX
// Complete the update (in case there's an error we can properly report it)
if (fValueSet)
   if (DataCallwErrCoord( lpda, SET, UFINISHUPDATE, lpSpread->Row, -1, lpSS) != 0)
      fRet = FALSE;
#endif       

return (fRet);
}


BOOL DataSetRow2(LPSPREADSHEET lpSS, LONG lObject, LPVBSPREAD lpSpread, LPDATAACCESS lpda,
                 SS_COORD Col, BYTE bRowDirty, LPBOOL lpfReReadData)
{
SS_HCTL       hCtl = (SS_HCTL)lObject;             //SSOCX
SS_CELLTYPE   CellType;
LPSS_CELL     lpCell;
LPSS_COL      lpCol;
USHORT        usFieldType;
VALUE         Value;
LPTSTR        lpszText;
TIME          Time;
TB_DATE       Date;
HSZ           hszText = 0;
double        dfTime;
double        dfVal;
long          lJulian;
long          lData;
long          lDataCol;
BOOL          fRet = TRUE;
BOOL          fDirty = FALSE;
BOOL          fDataFillEvent;
BOOL          fSetDateTime;
BOOL          fTimeNotEmpty = FALSE;
TCHAR         cDecimalSign;
TCHAR         Buffer[50];
#ifndef SS_OCX
VARIANT       Variant;
#endif

fDataFillEvent = lpSS->fDataFillEvent; // RFW - 2/20/00 - GIC11069

SS_GetCellType(lpSS, Col, lpSpread->Row, &CellType);

if (lpCell = SS_LockCellItem(lpSS, Col, lpSpread->Row))
   {
   fDirty = lpCell->fDirty;
   lpCell->fDirty = FALSE;
   SS_UnlockCellItem(lpSS, Col, lpSpread->Row);
   }

if (fDirty || bRowDirty == SS_ROWDIRTY_ALL)
   {
   lpSpread->Col = (SS_COORD)Col;

   DataCall(lpda, GET, FIELDATTRIBUTES, lpSS);
   if (!((short)lpda->lData & 0x20))          // Not Updatable
      return (FALSE);

   DataCall(lpda, GET, FIELDTYPE, lpSS);
   usFieldType = (USHORT)lpda->lData;

   if ((CellType.Type == SS_TYPE_COMBOBOX &&
       (usFieldType == DATA_VT_TEXT || usFieldType == DATA_VT_MEMO)) ||
       CellType.Type == SS_TYPE_PIC)
      {
#ifdef SS_OCX
      TBGLOBALHANDLE hText = SS_GetData(lpSS, &CellType, Col, lpSpread->Row, FALSE);
      if (hText)
         {
         LPTSTR lpszText = (LPTSTR)tbGlobalLock(hText);
         hszText = ssVBCreateHsz(lObject, lpszText);
         tbGlobalUnlock(hText);
         }
      else
         {
         hszText = ssVBCreateHsz(lObject, _T(""));
         }

    if (!hszText)
       return (FALSE);
#else
      if (VBGetControlProperty(hCtl, IPROP_SPREAD_TEXT, &hszText))
         return (FALSE);
#endif
      }
   else
      {
#ifdef SS_OCX
      TBGLOBALHANDLE hText = SS_GetValue(lpSS, Col, lpSpread->Row);
      if (hText)
         {
         LPTSTR lpszText = (LPTSTR)tbGlobalLock(hText);
         hszText = ssVBCreateHsz(lObject, lpszText);
         tbGlobalUnlock(hText);
         }
      else
         {
         hszText = ssVBCreateHsz(lObject, _T(""));
         }
    if (!hszText)
       return (FALSE);
#else
      if (VBGetControlProperty(hCtl, IPROP_SPREAD_VALUE, &hszText))
         return (FALSE);
#endif

      if (CellType.Type == SS_TYPE_FLOAT)
         {  
         if( DATA_VT_CURRENCY == usFieldType ||
             DATA_VT_SINGLE == usFieldType ||
             DATA_VT_DOUBLE == usFieldType )
	         {
            SS_FLOATFORMAT ff;
            FloatGetFormat(0, &ff);
            cDecimalSign = ff.cDecimalSign;
				}
         else if (CellType.Spec.Float.fSetFormat &&
             CellType.Spec.Float.Format.cDecimalSign)
            cDecimalSign = CellType.Spec.Float.Format.cDecimalSign;
         else
				{
            SS_FLOATFORMAT ff;
            SS_GetDefFloatFormat(lpSS->lpBook, &ff);
            cDecimalSign = ff.cDecimalSign;
				}

         if (cDecimalSign != '.')
            {
            lpszText = ssVBDerefHsz(hszText);
            SS_StrReplaceCh(lpszText, '.', cDecimalSign);
            }
         }
#ifdef SS_V40
		else if (CellType.Type == SS_TYPE_CURRENCY
               || CellType.Type == SS_TYPE_NUMBER
#ifdef SS_V70
               || CellType.Type == SS_TYPE_SCIENTIFIC
#endif // SS_V70
               || CellType.Type == SS_TYPE_PERCENT)
         {
         SS_FLOATFORMAT ff;
         FloatGetFormat(0, &ff);
         cDecimalSign = ff.cDecimalSign;

         if (cDecimalSign != '.')
            {
            lpszText = ssVBDerefHsz(hszText);
            SS_StrReplaceCh(lpszText, '.', cDecimalSign);
            }
         }
#endif // SS_V40
      }

   if (lpCol = SS_LockColItem(lpSS, Col))
      {
      // RFW - 2/20/00 - GIC11069
      //fDataFillEvent = lpCol->fDataFillEvent;
      if (lpCol->bDataFillEvent == SS_DATAFILLEVENT_YES)
         fDataFillEvent = TRUE;
      else if (lpCol->bDataFillEvent == SS_DATAFILLEVENT_NO)
         fDataFillEvent = FALSE;

      SS_UnlockColItem(lpSS, Col);
      }

   if (fDataFillEvent)
      {
      if (lpda->sDataFieldIndex)
         lDataCol = lpda->sDataFieldIndex;
      else
         {
         DataCall(lpda, GET, FIELDPOSITION, lpSS);
         lDataCol = (short)lpda->lData;
         }

      if (VBSSBoundFireDataFill(lpSS, lObject, lpSpread, Col, lpSpread->Row, FALSE,
                                usFieldType, (long)lpda->hlstrBookMark,
                                lDataCol))
         {
         if (hszText)
            ssVBDestroyHsz(hszText);

         return (TRUE);
         }
      }

   lpda->fs = 0;

   if (usFieldType == DATA_VT_BOOL)
      {
      *Buffer = '\0';
      SS_GetDataEx(lpSS, Col, lpSpread->Row, Buffer, 2);

      if (*Buffer == '\0' || *Buffer == '0')
         lpda->lData = 0L;
      else
         lpda->lData = 1L;
#ifdef SS_OCX
      lpda->usDataType = DT_BOOL;
#else
      lpda->usDataType = DT_SHORT;
#endif
      }

   else if (usFieldType == DATA_VT_DATETIME &&
            (CellType.Type == SS_TYPE_DATE || CellType.Type == SS_TYPE_TIME))
      {
      lpszText = ssVBDerefHsz(hszText);

      dfVal = 0.0;

      if (MapColData(lObject, &lData, lpda, DATA_VT_DATETIME, CellType.Type,
                     FALSE, lpSS))
         {
         Value.hlstr = (HLSTR)lData;

//DBOCX...
#ifdef SS_OCX
         if (Value.hlstr)
            {
            fpConvertTSTRtoType( (LPTSTR)Value.hlstr, &dfVal, VT_DATE);
            ssVBDestroyHlstr(Value.hlstr);
            }
#else
         _fmemset(&Variant, '\0', sizeof(VARIANT));
         VBSetVariantValue(&Variant, VT_STRING, &Value);
         VBCoerceVariant(&Variant, VT_DATE, &dfVal);
         if (Value.hlstr)
            ssVBDestroyHlstr(Value.hlstr);
#endif
//DBOCX.
         /*
         dfVal = ((double)(long)(dfVal * 100000)) / 100000;
         */
         }

      lpda->fs = 0;
      *Buffer = '\0';
      fSetDateTime = FALSE;

      if (CellType.Type == SS_TYPE_DATE)
         {
         if (lpszText && *lpszText)
            {
            DateValueToDMY(lpszText, &Date);
            lJulian = vbDateDMYToJulian(&Date) + 2;
            }
         else
            lJulian = 0L;

         dfVal = dfVal - (double)(long)dfVal + (double)lJulian;
         fSetDateTime = TRUE;
         }

      else if (CellType.Type == SS_TYPE_TIME)
         {
         dfTime = 0.0;

         if (TimeValidate(lpszText, &Time))
            {
            dfTime = (double)(((long)Time.nHour * 3600L) +
                              ((long)Time.nMinute * 60L) +
                               (long)Time.nSecond);

            /**************************************************
            * Move the Time value to the right of the decimal
            * and scale it from 0 to .99999
            **************************************************/

//            dfTime = dfTime / 100000.0 * 99999.0 / 86399.0;
            dfTime = dfTime * 0.99999999 / 86399.0;
//            dfTime = dfTime * 0.0000115740740740;
            fTimeNotEmpty = TRUE;
            }


         dfVal = (double)(long)dfVal + dfTime;
         fSetDateTime = TRUE;
         }

      // The following code was changed in response to KEM2

#ifdef SS_OCX

//      if (fSetDateTime && dfVal != 0.0)
      if ((fSetDateTime && dfVal != 0.0) || fTimeNotEmpty)
         {
         lpda->lData = (long)&dfVal;
         lpda->usDataType = DT_FP_DATETIME;
         }

      else
         {
         lpda->fs = DA_fNull;
         ssVBDestroyHsz(hszText);
         hszText = ssVBCreateHsz(lObject, _T(""));
         lpda->lData = (long)hszText;
         lpda->usDataType = DT_HSZ;
         }

#else
      if (fSetDateTime)
         {
         if (dfVal < 1.0 && (dfVal != 0.0 || fTimeNotEmpty))
            ssVBFormat(lObject, VT_R8, &dfVal, _T("Short Time"), Buffer,
                       sizeof(Buffer) - 1);
         else if (dfVal == 0.0)
            ;
         else
            {
            TCHAR szFormat[65];
            BOOL  fYearFound;
            short i;

            _fmemset(szFormat, '\0', sizeof(szFormat));
            lstrcpy(Buffer, fpGetIntl_sShortDate());

            for (i = 0, fYearFound = FALSE; i < lstrlen(Buffer); i++)
               {
               if (Buffer[i] == (TCHAR)'y' || Buffer[i] == (TCHAR)'Y')
                  {
                  if (!fYearFound)
                     {
                     lstrcat(szFormat, _T("yyyy"));
                     fYearFound = TRUE;
                     }
                  }

               else
                  szFormat[lstrlen(szFormat)] = Buffer[i];
               }

            _fmemset(Buffer, '\0', sizeof(Buffer));
            ssVBFormat(lObject, VT_R8, &dfVal, szFormat, Buffer,
                       sizeof(Buffer) - 1);

            if (dfVal != (double)(long)dfVal)  // Date and Time
               {
               Buffer[lstrlen(Buffer)] = ' ';

               ssVBFormat(lObject, VT_R8, &dfVal, _T("Short Time"),
                          &Buffer[lstrlen(Buffer)],
                          (USHORT)(sizeof(Buffer) - 1 - lstrlen(Buffer)));
               }
            }

         // BJO 12May97 SEL95051 - Begin fix
         if( !*Buffer )
            lpda->fs = DA_fNull;
         // BJO 12May97 SEL95051 - End fix

         ssVBDestroyHsz(hszText);
         hszText = ssVBCreateHsz(lObject, Buffer);
         }

      else
         {
         ssVBDestroyHsz(hszText);
         hszText = ssVBCreateHsz(lObject, _T(""));
         }

      lpda->lData = (long)hszText;
      lpda->usDataType = DT_HSZ;
#endif
      }

   else if (usFieldType == DATA_VT_BINARY &&
            SS_GetCellType(lpSS, Col, lpSpread->Row, &CellType) &&
            CellType.Type == SS_TYPE_PICTURE)
      {
      lpda->lData = (long)VBSSBmpSave(lpSS, (HBITMAP)CellType.Spec.ViewPict.hPictName,
                                     CellType.Spec.ViewPict.hPal,
                                     CellType.Style, Col, lpSpread->Row);
      lpda->usDataType = DT_HLSTR;
      }

   else if (CellType.Type == SS_TYPE_INTEGER
#ifdef SS_V40
            || CellType.Type == SS_TYPE_CURRENCY
            || CellType.Type == SS_TYPE_NUMBER
            || CellType.Type == SS_TYPE_PERCENT
#endif // SS_V40
#ifdef SS_V70
            || CellType.Type == SS_TYPE_SCIENTIFIC
#endif // SS_V70
            || CellType.Type == SS_TYPE_FLOAT)
      {
      lpszText = ssVBDerefHsz(hszText);
      if (!*lpszText)
         {
         DataCall(lpda, GET, FIELDATTRIBUTES, lpSS);

         /*****************************************************
         * If an auto increment field, then give JET a blank,
         * so it can create a unique key.
         *****************************************************/

         if ((short)lpda->lData & 16)
            {
            if (hszText)
               ssVBDestroyHsz(hszText);
            hszText = ssVBCreateHsz(lObject, _T(" "));
            *lpfReReadData = TRUE;
            }
         else
            lpda->fs = DA_fNull;
         }

      lpda->lData = (long)hszText;
      lpda->usDataType = DT_HSZ;
      }

   else
      {
      lpszText = ssVBDerefHsz(hszText);
      if (!*lpszText)
         lpda->fs = DA_fNull;

      lpda->lData = (long)hszText;
      lpda->usDataType = DT_HSZ;
      }

//DBOCX...
#ifdef SS_OCX
   if (DataCallwErrCoord(lpda, SET, FIELDVALUE, lpSpread->Row, Col, lpSS) != 0)
      fRet = FALSE;
   if (!IsWindow(lpSS->lpBook->hWnd))
      return FALSE;
#else   
   DataCall(lpda, SET, FIELDVALUE, lpSS);
#endif
//DBOCX.

   if (hszText)
      ssVBDestroyHsz(hszText);
   }

return (fRet);
}


BOOL VBSSBoundDel(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, SS_COORD Row1, SS_COORD Row2)
{
SS_HCTL    hCtl = (SS_HCTL)lObject;             //SSOCX
LPVBSPREAD lpSpread;
LPSS_ROW   lpRow;
DATAACCESS da;
SS_COORD   i;
BOOL       fIsRegistered;
BOOL       fRet = FALSE;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (VBSSBoundIsBound(lObject, lpSS) && lpSpread->fUpdatable)
   {
   _fmemset(&da, 0, sizeof(da));
   da.usVersion = VB_VERSION;
   da.hctlData = (HCTL)ssVBGetDataSourceControl(lObject, &fIsRegistered);
   da.hctlBound = (HCTL)hCtl;

   lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

   Row1 = max(Row1, lpSS->Row.HeaderCnt);

   for (i = Row1; i <= Row2; i++)
      {
      if (!lpSpread->DataOpts.fAutoSave)
         {
         if (lpRow = SS_AllocLockRow(lpSS, i))
            {
            lpRow->bDirty = SS_ROWDIRTY_DEL;
            SS_UnlockRowItem(lpSS, i);
            }

         SS_ShowRow(lpSS, i, FALSE);
         fRet = TRUE;
         }

      else
         {
         if (vbSpreadBoundClick(lpSS, lObject, Row1))
            {
            SHORT rc;
            lpSpread->fDataDeleteInternal = TRUE;
            rc = DataCallwErrCoord(&da, METHOD, DELETE, Row1, -1, lpSS);
            if (!IsWindow(hWnd))
              return TRUE;
            lpSpread->fDataDeleteInternal = FALSE;
            fRet = TRUE;
            // If Delete row failed, break out of loop
            if (rc != (SHORT)0)
               break;
            SS_DelRowRange(lpSS, Row1, Row1);
            }
         }
      }

   if (fRet)
      {
      SS_GetActiveCell(lpSS, NULL, &Row1);
      vbSpreadBoundClick(lpSS, lObject, Row1);
      }
   }

return (fRet);
}


BOOL VBSSBoundAdd(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, SS_COORD Row1, SS_COORD Row2,
                  BOOL fInsert)
{
SS_HCTL       hCtl = (SS_HCTL)lObject;             //SSOCX
LPVBSPREAD    lpSpread;
DATAACCESS    da;
SS_COORD      i;
BOOL          fRet = FALSE;
BOOL          fIsRegistered;

Row2 = Row1;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (VBSSBoundIsBound(lObject, lpSS) && lpSpread->fUpdatable)
   {
   _fmemset(&da, 0, sizeof(da));
   da.usVersion = VB_VERSION;
   da.hctlData = (HCTL)ssVBGetDataSourceControl(lObject, &fIsRegistered);
   da.hctlBound = (HCTL)hCtl;

   Row1 = max(Row1, lpSS->Row.HeaderCnt);

   for (i = Row1; i <= Row2; i++)
      {
      lpSpread->RowBeingInserted = i;

      if (!lpSpread->fDataIgnoreDataRequest)
         {
#ifdef  SS_OCX
         SHORT rc;
         // Get last bookmark, add row, get new last bookmark. If the
         // two bookmarks are the same, then the insert failed!!!
         da.lData = DATA_BOOKMARKLAST;
         if (DataCall(&da, GET, BOOKMARK, lpSS) == 0)
           vbSS_SetBookmark( (LPVOID)&lpSpread->hlstrLastBookMark, 
             (LPVOID)da.lData);

         rc = DataCallwErrCoord(&da, METHOD, ADDNEW, i, -1, lpSS);
         if (!IsWindow(hWnd))
            return FALSE;
         if (rc != (SHORT)0)
         {
            // Error while adding row, free this bookmark and break loop
            vbSS_SetBookmark( (LPVOID)&lpSpread->hlstrLastBookMark, NULL);
            break;
         }
         VBSSBoundProcessPostAdd(lpSS, lObject, hWnd, (long)i);
         // Free hlstr
         vbSS_SetBookmark( (LPVOID)&lpSpread->hlstrLastBookMark, NULL);
#else
         if (VBSSBoundIsEmpty(lObject, &da, lpSS))
            {
            /*******************************************************
            * Fire an event so the application can issue an AddNew
            * if the database is empty.  This is because of a bug
            * in VB3.
            *******************************************************/
            VBFireEvent(hCtl, SS_EVENT_DataAddNew, NULL);
            }
         else
            DataCall(&da, METHOD, ADDNEW, lpSS);
#endif
         }

      if (fInsert)
         SS_InsRowRange(lpSS, lpSpread->RowBeingInserted, lpSpread->RowBeingInserted);

      fRet = TRUE;
      }
   }

return (fRet);
}


BOOL VBSSBoundSetColWidth(LPSPREADSHEET lpSS, HWND hWnd, LPVBSPREAD lpSpread)
{
SS_CELLTYPE CellType;
LPSS_COL    lpCol;
TIME        Time;
TB_DATE     Date;
HDC         hDC;
TCHAR       Buffer[30];
double      dfMaxWidth;
int         iWidth;
SS_COORD    Col;

if (lpSpread->DataOpts.fSizeCols == 1)
   {
   for (Col = lpSS->Col.HeaderCnt; Col < lpSS->Col.AllocCnt; Col++)
      if (lpCol = SS_LockColItem(lpSS, Col))
         {
         if (lpCol->dFieldNum)
            if (SS_GetMaxTextColWidth(lpSS, Col, &dfMaxWidth))
               SS_SetColWidth(lpSS, Col, dfMaxWidth);

         SS_UnlockColItem(lpSS, Col);
         }
   }

else if (lpSpread->DataOpts.fSizeCols == 2)
   {
   hDC = fpGetDC(hWnd);

   for (Col = lpSS->Col.HeaderCnt; Col < lpSS->Col.AllocCnt; Col++)
      if (lpCol = SS_LockColItem(lpSS, Col))
         {
         if (lpCol->dFieldNum && lpCol->dColWidthInPixels)
            {
            SS_GetCellType(lpSS, Col, -1, &CellType);
            lpCol = SS_LockColItem(lpSS, Col);

            switch (CellType.Type)
               {
               case SS_TYPE_DATE:            /* requested as DT_HSZ */
                  Date.nDay = 22;
                  Date.nMonth = 2;
                  Date.nYear = 1993;
                  DateDMYToString(&Date, Buffer, &CellType.Spec.Date.Format);
                  StrCat(Buffer, _T(" "));
                  SS_CalcCellMetrics(hDC, lpSS,
                                     lpCol, NULL, NULL, &CellType, NULL, Col, -1,
                                     TRUE, FALSE, NULL, Buffer, &iWidth, NULL);
                  break;

               case SS_TYPE_TIME:
                  Time.nHour = 22;
                  Time.nMinute = 22;
                  Time.nSecond = 22;
                  TimeHMSToString(&Time, Buffer, &CellType.Spec.Time.Format);
                  StrCat(Buffer, _T(" "));
                  SS_CalcCellMetrics(hDC, lpSS,
                                     lpCol, NULL, NULL, &CellType, NULL, Col, -1,
                                     TRUE, FALSE, NULL, Buffer, &iWidth, NULL);
                  break;

               case SS_TYPE_STATICTEXT:      /* requested as DT_HSZ */
               case SS_TYPE_EDIT:
                  SS_CalcCellMetrics(hDC, lpSS,
                                     lpCol, NULL, NULL, &CellType, NULL, Col, -1,
                                     TRUE, FALSE, NULL, _T("ssssssssssssssssssss"), &iWidth, NULL);
						// RFW - 8/2/04 - 14558
						iWidth++;
                  break;

               case SS_TYPE_PIC:
                  SS_CalcCellMetrics(hDC, lpSS,
                                     lpCol, NULL, NULL, &CellType, NULL, Col, -1,
                                     TRUE, FALSE, NULL, _T("ssssssssssss"), &iWidth, NULL);
                  break;

               case SS_TYPE_FLOAT:
                  SS_CalcCellMetrics(hDC, lpSS,
                                     lpCol, NULL, NULL, &CellType, NULL, Col, -1,
                                     TRUE, FALSE, NULL, _T("123456789"), &iWidth, NULL);
//                                  TRUE, FALSE, NULL, _T("123,456,789.12"), &iWidth, NULL);
                  break;

#ifdef SS_V40
               case SS_TYPE_CURRENCY:
                  SS_CalcCellMetrics(hDC, lpSS,
                                     lpCol, NULL, NULL, &CellType, NULL, Col, -1,
                                     TRUE, FALSE, NULL, _T("123456789"), &iWidth, NULL);
//                                  TRUE, FALSE, NULL, _T("$123,456,789.12"), &iWidth, NULL);
                  break;

               case SS_TYPE_NUMBER:
						if (CellType.Spec.Number.Right == 0)
							SS_CalcCellMetrics(hDC, lpSS,
												    lpCol, NULL, NULL, &CellType, NULL, Col, -1,
												    TRUE, FALSE, NULL, _T("12345678901"), &iWidth, NULL);
						else
							SS_CalcCellMetrics(hDC, lpSS,
							   					 lpCol, NULL, NULL, &CellType, NULL, Col, -1,
								   				 TRUE, FALSE, NULL, _T("123456789"), &iWidth, NULL);
//									   			 TRUE, FALSE, NULL, _T("123,456,789.12"), &iWidth, NULL);
						break;

               case SS_TYPE_PERCENT:
                  SS_CalcCellMetrics(hDC, lpSS,
                                     lpCol, NULL, NULL, &CellType, NULL, Col, -1,
                                     TRUE, FALSE, NULL, _T("123"), &iWidth, NULL);
//                                  TRUE, FALSE, NULL, _T("123.12%"), &iWidth, NULL);
                  break;
#endif // SS_V40

#ifdef SS_V70
					case SS_TYPE_SCIENTIFIC:
                  SS_CalcCellMetrics(hDC, lpSS,
                                     lpCol, NULL, NULL, &CellType, NULL, Col, -1,
                                     TRUE, FALSE, NULL, _T("1.11E+01"), &iWidth, NULL);
						break;
#endif // SS_V70

               case SS_TYPE_INTEGER:         /* requested as DT_LONG */
                  SS_CalcCellMetrics(hDC, lpSS,
                                     lpCol, NULL, NULL, &CellType, NULL, Col, -1,
                                     TRUE, FALSE, NULL, _T("12345678901"), &iWidth, NULL);
                  break;

               case SS_TYPE_COMBOBOX:
                  SS_CalcCellMetrics(hDC, lpSS,
                                     lpCol, NULL, NULL, &CellType, NULL, Col, -1,
                                     TRUE, FALSE, NULL, _T("sssssssssssssss"), &iWidth, NULL);
                  break;

               case SS_TYPE_CHECKBOX:
               case SS_TYPE_BUTTON:          /* nothing to do for these */
               case SS_TYPE_PICTURE:
               case SS_TYPE_OWNERDRAW:
                  iWidth = 50;
                  break;
               }

            SS_SetColWidthInPixels(lpSS, Col, iWidth);
            }

         SS_UnlockColItem(lpSS, Col);
         }

   ReleaseDC(hWnd, hDC);
   }

return (TRUE);
}


BOOL VBSSBoundManualSave(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, LPVBSPREAD lpSpread,
                         SS_COORD Top, SS_COORD End, BOOL fSetCurrentRow)
{
SS_HCTL    hCtl = (SS_HCTL)lObject;             //SSOCX
DATAACCESS da;
LPSS_ROW   lpRow;
SS_COORD   i;
BOOL       fIsRegistered;
SHORT      rcDataCall;

Top = max(Top, lpSS->fVirtualMode ? lpSS->Virtual.VTop : lpSS->Row.HeaderCnt);

if (End == -1)
   End = lpSS->Row.DataCnt;

#ifdef SS_QE
if (lpSpread->DBInfo.fConnected)
   {
   BOOL fRet;

   fRet = DBSS_ManualSave(hCtl, hWnd, lpSpread, lpSS, Top, End);
   return (fRet);
   }
#endif

if (!lpSpread->fUpdatable)
   return (TRUE);

_fmemset(&da, 0, sizeof(da));
da.usVersion = VB_VERSION;
da.hctlData = (HCTL)ssVBGetDataSourceControl(lObject, &fIsRegistered);
da.hctlBound = (HCTL)hCtl;

for (i = Top; i < End; i++)
   if (lpRow = SS_LockRowItem(lpSS, i))
      {
      rcDataCall = (SHORT)0;
      if (lpRow->bDirty == SS_ROWDIRTY_DEL)   // Delete?
         {
         if (lpRow->hlstrBookMark)
            {
            vbSpreadBoundClick(lpSS, lObject, i);

            lpSpread->fDataDeleteInternal = TRUE;
            rcDataCall = DataCallwErrCoord(&da, METHOD, DELETE, i, -1, lpSS);
            if (!IsWindow(hWnd))
               return TRUE;
            lpSpread->fDataDeleteInternal = FALSE;
            }

         // If Delete row failed, break out-of-loop
         if (rcDataCall != (SHORT)0)
            break;
         SS_DelRowRange(lpSS, i, i);
         End--;
         i--;

         SS_UnlockRowItem(lpSS, i);
         }

      else if (lpRow->bDirty && lpRow->hlstrBookMark)
         {
         vbSpreadBoundClick(lpSS, lObject, i);  // RFW - 9/3/94
         ssVBSetControlFlags(lObject, CTLFLG_DATACHANGED, CTLFLG_DATACHANGED, lpSS);
         lpSpread->RowBeingSaved = i;
         SS_UnlockRowItem(lpSS, i);
         }

      else if (lpRow->bDirty && !lpRow->hlstrBookMark)
         {
         // RFW - 7/27/94

         BOOL fIsEmpty = VBSSBoundIsEmpty(lObject, &da, lpSS);

         if (!fIsEmpty)
            {
            IGNOREMOVELAST_FLD(lpSpread) = TRUE;
            DataCall(&da, METHOD, MOVELAST, lpSS);
            IGNOREMOVELAST_FLD(lpSpread) = FALSE;
            }

         lpSpread->RowBeingSaved = i;
         lpSpread->RowBeingInserted = i;
         if (!lpSpread->fDataIgnoreDataRequest)
            {
#ifdef SS_OCX 
            // GRB792  

            // Get last bookmark, add row, get new last bookmark. If the
            // two bookmarks are the same, then the insert failed!!!
            da.lData = DATA_BOOKMARKLAST;
            if (DataCall(&da, GET, BOOKMARK, lpSS) == 0)
               vbSS_SetBookmark( (LPVOID)&lpSpread->hlstrLastBookMark, 
                 (LPVOID)da.lData);

            // NOTE: The OCX will not support the DataAddNew Event (since 
            // it was a fix for a VB30 bug), therefore it does not matter
      // if the database is empty
            rcDataCall = DataCallwErrCoord(&da, METHOD, ADDNEW, i, -1, lpSS);
            if (!IsWindow(hWnd))
               return TRUE;
            // If Insert row failed, break out-of-loop
            if (rcDataCall != (SHORT)0)
            {
               // free hlstr
               vbSS_SetBookmark( (LPVOID)&lpSpread->hlstrLastBookMark, NULL);
               break;
            }
//          lpSpread->fAddNewFromDataControl = TRUE;
// - The above line was preventing Inserts from updating properly. SCP 6/23/99

            // Call VBSSBoundProcessPostAdd() directly for OCX
            VBSSBoundProcessPostAdd(lpSS, lObject, hWnd, (long)i);
            // free hlstr
            vbSS_SetBookmark( (LPVOID)&lpSpread->hlstrLastBookMark, NULL);
#else  //VBX
            if (fIsEmpty)
               {
               /*******************************************************
               * Fire an event so the application can issue an AddNew
               * if the database is empty.  This is because of a bug
               * in VB3.
               *******************************************************/
               VBFireEvent(hCtl, SS_EVENT_DataAddNew, NULL);
               }
            else
               DataCall(&da, METHOD, ADDNEW, lpSS);
#endif
            }

         SS_UnlockRowItem(lpSS, i);
         return (TRUE);
         }

      else
         SS_UnlockRowItem(lpSS, i);
      }

if (fSetCurrentRow)
   vbSpreadBoundClick(lpSS, lObject, lpSS->Row.CurAt);

lpSpread->RowBeingSaved = 0;
return (TRUE);
}


BOOL VBSSBoundIsBound(LONG lObject, LPSPREADSHEET lpSS)
{
  SS_HCTL    hCtl = (SS_HCTL)lObject;            //SSOCX
// SS_OCX: Since this can be called from almost anywhere, LOCK the
// VBSPREAD structure, check flag, then unlock.
#ifdef SS_OCX
  LPVBSPREAD lpSpread = (LPVBSPREAD)SSOcxLockVBSpread(lObject);
#ifdef SS_V70
  BOOL fRet = lpSS->fBoundActive;
#else
  BOOL fRet = lpSpread->fBoundActive;
#endif
  SSOcxUnlockVBSpread(lObject);
  return (fRet);
#else //not SS_OCX
  LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
  return (lpSpread->fBoundActive);
#endif
}


BOOL VBSSBmpLoad(LONG lObject, HWND hWnd, LPVBSPREAD lpSpread, HPSTR lpBmp,
                 long lSize)
{
SS_HCTL      hCtl = (SS_HCTL)lObject;            //SSOCX
GLOBALHANDLE hDib;
HPALETTE     hPal;
HBITMAP      hBitMap;
BOOL         fRet = FALSE;

if (hDib = tbOpenDIB(NULL, lpBmp, lSize))
   {
   hPal = tbCreateDibPalette(hDib);

   if (hBitMap = tbBitmapFromDib(hDib, hPal, TRUE))
      {
//DBOCX...
#ifdef SS_OCX
      fRet = SSOcxSetTypePictPicture(lObject, lpSpread->Col, lpSpread->Row, 
                                     hBitMap, hPal, 0);
#else
      PIC  Pic;
      HPIC hPic;

      Pic.picType = PICTYPE_BITMAP;
      Pic.picData.bmp.hbitmap = hBitMap;

      if (fpVBGetVersion() >= VB200_VERSION)
         Pic.picData.bmp.hpal = hPal;

      if (fpVBGetVersion() >= VB200_VERSION)
         hPic = fpVBRefPic(fpVBAllocPicEx(&Pic, VB_VERSION));
      else
         hPic = fpVBRefPic(fpVBAllocPic(&Pic));

      FPx_TypePictPicture(TRUE, hCtl, hWnd, 0, IPROP_SPREAD_TYPEPICTPICTURE,
                            (long)hPic, lpSpread);
      fRet = TRUE;
#endif
//DBOCX..
      }

   GlobalFree(hDib);
   }
/*
else
	{
#ifdef SS_OCX
	HICON hIcon = CreateIconFromResource(lpBmp, lSize, TRUE, 0x00030000);
	if (hIcon)
		fRet = SSOcxSetTypePictPicture(lObject, lpSpread->Col, lpSpread->Row, 
			                            0, 0, hIcon);
#endif
	}
*/

return (fRet);
}


HLSTR VBSSBmpSave(LPSPREADSHEET lpSS, HBITMAP hBitmap, HPALETTE hPal, long lStyle,
                  SS_COORD Col, SS_COORD Row)
{
GLOBALHANDLE hDib;
GLOBALHANDLE hBuffer;
COLORREF     Color;
HBITMAP      hBitmapTemp = 0;
HLSTR        hlstr;
LPTSTR       lpBuffer;
long         lBufferSize;

if (!hBitmap)
   return (0);

if (lStyle & VPS_ICON)
   {
   SS_GetColor(lpSS, Col, Row, &Color, NULL);
   hBitmap = tbIconToBitmap(lpSS->lpBook->hWnd, (HICON)hBitmap, Color);
   hBitmapTemp = hBitmap;
   }

else if (!(lStyle & VPS_BMP))
   return (0);

if (hDib = tbBitmapToDIB(hBitmap, hPal))
   {
   if (hBuffer = tbSaveDIBToBuffer(hDib, &lBufferSize))
      {
      lpBuffer = (LPTSTR)GlobalLock(hBuffer);

#ifdef SS_OCX
      hlstr = (HLSTR)LBlockAllocLen((LPBYTE)lpBuffer, lBufferSize);
#else
      hlstr = ssVBCreateHlstr((LPVOID)lpBuffer, (WORD)lBufferSize);
#endif

      GlobalUnlock(hBuffer);
      GlobalFree(hBuffer);
      }

   GlobalFree(hDib);
   }

if (hBitmapTemp)
   DeleteObject(hBitmapTemp);

return (hlstr);
}

void VBSSBoundProcessPostAdd(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, long lParam)
{
  // Call other function to process but pass NULL as the bookmark of 
  // the row being added.
  VBSSBoundProcessPostAdd2(lpSS, lObject, hWnd, lParam, NULL);
}

void VBSSBoundProcessPostAdd2(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, long lParam, 
  HLSTR hlstrNewBookmark)
{
SS_HCTL       hCtl = (SS_HCTL)lObject;             //SSOCX
LPVBSPREAD    lpSpread;
LPSS_ROW      lpRow;
DATAACCESS    da;
BOOL          fIsRegistered;
SS_COORD      row = (SS_COORD)lParam;
#ifndef SS_OCX
SHORT         rc;
#endif

if (!IsWindow(hWnd))
   return;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

lpSpread->fDataIgnoreDataRequest = FALSE;

_fmemset(&da, 0, sizeof(da));
da.usVersion = VB_VERSION;
da.hctlData = (HCTL)ssVBGetDataSourceControl(lObject, &fIsRegistered);
da.hctlBound = (HCTL)hCtl;

if (lpSpread->fAddNewFromDataControl)
   {
   lpSpread->fAddNewFromDataControl = FALSE;

   if (!lpSpread->fUpdateReceived)
      {
      if (lpRow = SS_AllocLockRow(lpSS, row))
         {
#ifndef SS_OCX
         ssVBSetControlFlags(lObject, CTLFLG_DATACHANGED, CTLFLG_DATACHANGED, lpSS);

         lpRow->bDirty = SS_ROWDIRTY_ALL;
         lpSpread->fReReadRow = FALSE;    // set if any autoincr fields  //GRB792
         IGNOREMOVENEXT_FLD(lpSpread) = TRUE;
         // Note: MoveNext will fail if there are no rows in the table. If
         // the move fails, try MoveLast.  SCP 10/22/98 
         rc = DataCall(&da, METHOD, MOVENEXT, lpSS);
         IGNOREMOVENEXT_FLD(lpSpread) = FALSE;
#ifdef SS_OCX
         if (rc != 0)
            {
            // If we are here, then it's because the MoveNext failed, 
            // so we will try a MoveLast. SCP
            IGNOREMOVELAST_FLD(lpSpread) = TRUE;
            DataCall(&da, METHOD, MOVELAST, lpSS);
            IGNOREMOVELAST_FLD(lpSpread) = FALSE;
            }
#endif
#endif // not SS_OCX
         da.lData = DATA_BOOKMARKLAST;
         // If we were given a bookmark OR we get a bookmark and it differs
         // from the "last" bookmark, save it.
         if (hlstrNewBookmark ||
             ((DataCall(&da, GET, BOOKMARK, lpSS) == 0) &&
              !BookmarksEqual((HLSTR)da.lData, lpSpread->hlstrLastBookMark)))
            {
            vbSS_SetBookmark( (LPVOID)&lpRow->hlstrBookMark, 
              (hlstrNewBookmark ? hlstrNewBookmark : (LPVOID)da.lData));

            if (lpSpread->fReReadRow)
               {
               da.hlstrBookMark = lpRow->hlstrBookMark;
               FillAutoIncrDataRow((long)hCtl, hWnd, lpSpread, lpSS, row,
                                   &da, lpSpread->dFieldsCount, FALSE);
               lpSpread->fReReadRow = FALSE;
               }
// fix for bug #5392 -scl
#ifdef FP_ADB
            da.hlstrBookMark = lpRow->hlstrBookMark;
            FillDataRow((long)hCtl, hWnd, lpSpread, lpSS, row, &da, lpSpread->dFieldsCount, FALSE);
            SS_InvalidateRow(lpSS, row);
#endif
/*
            da.hlstrBookMark = lpRow->hlstrBookMark;
            if (lpSpread->fReReadRow)
               {
               FillAutoIncrDataRow((long)hCtl, hWnd, lpSpread, lpSS, row,
                                   &da, lpSpread->dFieldsCount, FALSE);
               lpSpread->fReReadRow = FALSE;
               }
            else
               FillDataRow((long)hCtl, hWnd, lpSpread, lpSS, row, &da,
                           lpSpread->dFieldsCount, FALSE);
*/

#ifndef SS_OCX  // Only call ...Click for VBX.  -SCP
            vbSpreadBoundClick(lpSS, lObject, lpSS->Row.CurAt);
#endif
            }

         SS_UnlockRowItem(lpSS, row);
         }
      }

   else
      {
      IGNOREMOVENEXT_FLD(lpSpread) = TRUE;
      DataCall(&da, METHOD, MOVENEXT, lpSS);
      IGNOREMOVENEXT_FLD(lpSpread) = FALSE;
      }

   lpSpread->fUpdateReceived = FALSE;
   #ifdef SS_OCX
   lpSpread->RowBeingInserted = 0;  // BJO 17May96 SEL3984
   #endif
   }

else if (lpSpread->RowBeingInserted)
   {
   if (lpRow = SS_LockRowItem(lpSS, lpSpread->RowBeingInserted))
      {
      ssVBSetControlFlags(lObject, CTLFLG_DATACHANGED, CTLFLG_DATACHANGED, lpSS);

      lpRow->bDirty = SS_ROWDIRTY_ALL;
      lpSpread->fReReadRow = FALSE;    // set if any autoincr fields  //GRB792
      IGNOREMOVELAST_FLD(lpSpread) = TRUE;
      // if empty, move NEXT to cause update (LAST won't work until inserted)
      if (VBSSBoundIsEmpty(lObject, &da, lpSS))
         DataCall(&da, METHOD, MOVENEXT, lpSS);           //DBOCX
      DataCall(&da, METHOD, MOVELAST, lpSS);            //DBOCX
      IGNOREMOVELAST_FLD(lpSpread) = FALSE;

      // If GET was OK AND this bookmark differs from the "last" one, 
      // use it.
      da.lData = DATA_BOOKMARKLAST;
      if (DataCall(&da, GET, BOOKMARK, lpSS) == 0)
         if (!BookmarksEqual((HLSTR)da.lData, lpSpread->hlstrLastBookMark))
            {
            vbSS_SetBookmark( (LPVOID)&lpRow->hlstrBookMark, (LPVOID)da.lData);
            if (lpSpread->fReReadRow)
               {
               da.hlstrBookMark = lpRow->hlstrBookMark;
               FillAutoIncrDataRow((long)hCtl, hWnd, lpSpread, lpSS,
                                   lpSpread->RowBeingInserted,
                                   &da, lpSpread->dFieldsCount, FALSE);
               lpSpread->fReReadRow = FALSE;
               }
            }
         else
            vbSS_SetBookmark( (LPVOID)&da.lData, NULL);

      SS_UnlockRowItem(lpSS, lpSpread->RowBeingInserted);
      }

   lpSpread->RowBeingInserted = 0;

   ssVBSetControlFlags(lObject, CTLFLG_DATACHANGED, 0L, lpSS);
   }

if (lpSpread->RowBeingSaved)
   VBSSBoundManualSave(lpSS, lObject, hWnd, lpSpread, lpSpread->RowBeingSaved + 1,
                       -1, TRUE);

else if (lpSpread->PostAddRowNew)
   {
   SS_CELLCOORD CellCoord;

   CellCoord.Col = lpSpread->PostAddRowNewCol;
   CellCoord.Row = lpSpread->PostAddRowNew;
   lpSpread->PostAddRowNew = 0;
   lpSpread->PostAddRowNewCol = 0;
   SS_AdjustCellCoordsOut(lpSS, &CellCoord.Col, &CellCoord.Row);
   SendMessage(hWnd, SSM_BOUNDCLICK, 0, (long)(LPVOID)&CellCoord);
   }

lpSpread->PostAddRowNew = 0;
lpSpread->PostAddRowNewCol = 0;
}


void VBSSBoundChangeMade(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, SS_COORD Col, SS_COORD Row)
{
SS_HCTL    hCtl = (SS_HCTL)lObject;             //SSOCX
LPVBSPREAD lpSpread;
LPSS_ROW   lpRow;
LPSS_CELL  lpCell;
BOOL       fBound;
BOOL       fConnected = FALSE;

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

fBound = VBSSBoundIsBound(lObject, lpSS);

#ifdef SS_QE
fConnected = lpSpread->DBInfo.fConnected;
#endif

if (fBound || fConnected)
   {
   lpRow = SS_LockRowItem(lpSS, Row);

   if (lpSpread->DataOpts.fAutoSave)
      {
      if (Row == lpSS->Row.CurAt)
         {
         if (lpRow && lpRow->hlstrBookMark && fBound)
            ssVBSetControlFlags(lObject, CTLFLG_DATACHANGED, CTLFLG_DATACHANGED, lpSS);
         }
      else
         {
         if (!lpSpread->fChangeMsgPosted)
            {
            lpSpread->fChangeMsgPosted = TRUE;
            PostMessage(hWnd, SSM_POSTDATACHANGE, 0, (LPARAM)Row);
            }
         }
      }

   if (lpRow)
      {
      if (!lpRow->bDirty)
         lpRow->bDirty = SS_ROWDIRTY_SOME;

      if (Col != SS_ALLCOLS &&
          (lpCell = SS_AllocLockCell(lpSS, lpRow, Col, Row)))
         {
         lpCell->fDirty = TRUE;
         SSx_UnlockCellItem(lpSS, lpRow, Col, Row);
         }

      SS_UnlockRowItem(lpSS, Row);
      }
   }
}


BOOL VBSSBoundIsEmpty(LONG lObject, LPDATAACCESS lpda, LPSPREADSHEET lpSS)
{
BOOL fEmpty = FALSE;

lpda->fs = 0;

lpda->lData = DATA_BOOKMARKFIRST;
if (DataCall(lpda, GET, BOOKMARK, lpSS) != 0)
   fEmpty = TRUE;
else 
   {
   if (lpda->fs & (DA_fBOF | DA_fEOF))
     fEmpty = TRUE;
   // Free bookmark
   vbSS_SetBookmark((LPVOID)&lpda->lData, NULL);
   }

return (fEmpty);
}

//DBOCX...
// Returns the Row if found, otherwise -1
SS_COORD FindHighlight(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, LPVBSPREAD lpSpread, LPDATAACCESS lpda,
                       short Action, int fActionFlags)
{
SS_COORD Row;
SS_COORD RowFirst;
SS_COORD RowLast;
SS_COORD RowInc;
SS_COORD MinRow;
SS_COORD MaxRows;
BOOL     fFound;
BOOL     fSetActive = ((fActionFlags & FIND_DONT_SETACTIVEROW) == 0);
SS_COORD RowFound = (SS_COORD)-1;

// If bookmark is NOT specified, get Current bookmark (put in 
// "lpda->hlstrBookMark"  (Who frees the hlstr for VB3? - SCP)
if ((fActionFlags & FIND_SPECIFIED_BOOKMARK) == 0)
   {
   lpda->hlstrBookMark = NULL;
   lpda->lData = DATA_BOOKMARKCURRENT;
   DataCall(lpda, GET, BOOKMARK, lpSS);
   if (!lpda->lData)
      return (SS_COORD)-1;

   lpda->hlstrBookMark = (HLSTR)lpda->lData;
   }

if (lpSS->fVirtualMode)
   MinRow = lpSS->Virtual.VTop;
else
   MinRow = 1;

//MaxRows = lpSS->Row.AllocCnt - 1;
MaxRows = lpSS->Row.AllocCnt;
fFound = FALSE;                   // init here, may change before "for" loop.

switch (Action)
   {
   case DATA_FINDFIRST:
#ifdef FP_OCX                     // FPDB API does not know "next", "prev" etc.
      // First look in the obvious places.
      RowFound = SearchNearHighlight( hWnd, lpda, lpSS, MinRow, MaxRows - 1, fSetActive);
#endif
      // if found, set flag fFound, else set search parameters.
      if (RowFound != (SS_COORD)-1)
         fFound = TRUE;
      else
         {
         RowFirst = MinRow;
         RowLast = MaxRows - 1;
         RowInc = 1;
         }
      break;

   case DATA_FINDNEXT:
      RowFirst = lpSS->Row.CurAt + 1;
      RowLast = MaxRows - 1;
      RowInc = 1;
      break;

   case DATA_FINDPREV:
      RowFirst = lpSS->Row.CurAt - 1;
      RowLast = MinRow;
      RowInc = -1;
      break;

   case DATA_FINDLAST:
      RowFirst = MaxRows - 1;
      RowLast = MinRow;
      RowInc = -1;
      break;
   }

// SCP 10/4/96 - "fFound" was initialized earlier, and at this point, may
//               already be TRUE, thus skipping "for" loop (!fFound).
for (Row = RowFirst; !fFound && Row >= MinRow &&
     Row < MaxRows; Row += RowInc)
   {
   if (fSetActive)
      fFound = MoveHighlightTestRow(lpda, lpSS, Row);
   else
      fFound = ssCompareRowWithBookmark(lpda->hlstrBookMark, lpSS, Row);
   if (fFound)
      RowFound = Row;             //save row
   }
if (!fFound)
   {
   RowInc *= -1;

   for (Row = RowFirst + RowInc; !fFound && Row >= MinRow && Row < MaxRows;
        Row += RowInc)
    {
      if (fSetActive)
         fFound = MoveHighlightTestRow(lpda, lpSS, Row);
      else
         fFound = ssCompareRowWithBookmark(lpda->hlstrBookMark, lpSS, Row);
      if (fFound)
         RowFound = Row;          //save row
    }
   }

if (!fFound && lpSS->fVirtualMode)
   {
	// RFW - 3/12/03 - 11457
	//if ((fActionFlags & FIND_SPECIFIED_BOOKMARK) != 0)
      lpSpread->fVRefreshCurrent = TRUE;
   SS_VRefreshBuffer(lpSS, FALSE);
   lpSpread->fVRefreshCurrent = FALSE;
   }

if ((fActionFlags & FIND_SPECIFIED_BOOKMARK) == 0)
   {
   // free the bookmark
   if (lpda->hlstrBookMark)
     vbSS_SetBookmark((LPVOID)&lpda->hlstrBookMark, NULL);
   }

return RowFound;
}
//DBOCX


// Returns the Row if found, otherwise -1
SS_COORD SearchNearHighlight(HWND hWnd, LPDATAACCESS lpda, LPSPREADSHEET  lpSS,
                             SS_COORD RowFirst, SS_COORD RowLast, BOOL fSetActive)
{
int      iCurrAction;
SS_COORD Row;
SS_COORD RowCurr = lpSS->Row.CurAt;
BOOL     fFound = FALSE;

RowCurr = max(RowCurr, lpSS->Row.HeaderCnt);

// Here's the search sequence: FINDNEXT, FINDPREV, FINDFIRST, FINDLAST,
// and FINDCURRENT (represented by 0).
iCurrAction = DATA_FINDNEXT;
Row = min(RowLast, RowCurr + 1L);

while ((Row != (SS_COORD)-1) && !fFound)
   {
   if (fSetActive)
      fFound = MoveHighlightTestRow(lpda, lpSS, Row);
   else
      fFound = ssCompareRowWithBookmark(lpda->hlstrBookMark, lpSS, Row);

   // If not found, try the next one in the sequence (iCurrAction)
   if (!fFound)
      {
      switch (iCurrAction)
         {
         case DATA_FINDNEXT:  // 69
           iCurrAction = DATA_FINDPREV;  // next action
           Row = max(RowFirst, RowCurr - 1L);
           break;
         case DATA_FINDPREV:  // 70
           iCurrAction = DATA_FINDFIRST; // next action
           Row = RowFirst;
           break;
         case DATA_FINDFIRST: // 68
           iCurrAction = DATA_FINDLAST;  // next action
           Row = RowLast;
           break;
         case DATA_FINDLAST:  // 71
           iCurrAction = 0;              // next action (check current row)
           Row = RowCurr;
           break;
         default:             // Not found. This sets return code & ends loop!
           Row = (SS_COORD)-1; 
         } //switch
      } // if !fFound
   } // while ...

  return Row;                 // -1 if not found, else the row.
}


// Expects Col to be in External units
void VBSSBoundFireDataColConfig(LONG lObject, LPVBSPREAD lpSpread,
                                LPTSTR lpszFieldName, SS_COORD Col,
                                USHORT usDataType, LPSPREADSHEET lpSS)
{
SS_HCTL hCtl = (SS_HCTL)lObject;             //SSOCX
HLSTR   hlstrFieldName;

if (lpszFieldName && *lpszFieldName)
   hlstrFieldName = ssVBCreateHlstr(lpszFieldName, (USHORT)(lstrlen(lpszFieldName) * sizeof(TCHAR)));
else
   hlstrFieldName = ssVBCreateHlstr(NULL, 0);

//DBOCX...
#ifdef SS_OCX
SSOcxFireDataColConfig(lObject, Col, (LPCTSTR)hlstrFieldName, 
  (short)usDataType, lpSS);

#else
{
SPREADPARAMS3 Params3;
HLSTR         hlstrFieldNameTemp = hlstrFieldName;

/* fire DataColConfig to allow them to make any changes */
Params3.lpParam1 = &Col;
Params3.lpParam2 = hlstrFieldNameTemp;
Params3.lpParam3 = &usDataType;
VBFireEvent(hCtl, SS_EVENT_DataColConfig, &Params3);
}
#endif
//DBOCX.

if (hlstrFieldName)
   ssVBDestroyHlstr(hlstrFieldName);
}


// This function is expecting Col and Row to be in internal coords
BOOL VBSSBoundFireDataFill(LPSPREADSHEET lpSS, LONG lObject, LPVBSPREAD lpSpread, SS_COORD Col,
                           SS_COORD Row, BOOL fGetData, USHORT usDataType,
                           long lBookMark, long lDataCol)
{
#ifdef SS_EVENT_DataFill
SS_HCTL hCtl = (SS_HCTL)lObject;             //SSOCX
BOOL    fCancel = FALSE;
short   sCancel = (short)FALSE;

if (fGetData)
   lpSpread->dDataGetFillCol = (short)lDataCol;
else
   lpSpread->dDataSetFillCol = (short)lDataCol;

lpSpread->hlstrDataFillBookMark = (HLSTR)lBookMark;

SS_AdjustCellCoordsOut(lpSS, &Col, &Row);

//DBOCX...
#ifdef SS_OCX
SSOcxFireDataFill(lObject, Col, Row, (short)usDataType, (short)fGetData, 
  &sCancel, lpSS);
fCancel = (BOOL)sCancel;

#else
{
SPREADPARAMS5 Param5;

Param5.lpParam1 = &Col;   /* col */
Param5.lpParam2 = &Row;
Param5.lpParam3 = &usDataType;
Param5.lpParam4 = &fGetData;
Param5.lpParam5 = &fCancel;
VBFireEvent(hCtl, SS_EVENT_DataFill, &Param5);
}
#endif
//DBOCX.

lpSpread->dDataGetFillCol = -1;
lpSpread->dDataSetFillCol = -1;

return (fCancel);
#else
return (0);
#endif
}


void VBSSBoundFinishClipData(HWND hWnd, LPSPREADSHEET lpSS, SS_COORD Row1,
                             SS_COORD Row2, SS_COORD ColCnt)
{
SS_InvalidateRowRange(lpSS, Row1, Row2);

SS_AdjustDataColCnt(lpSS, lpSS->Col.HeaderCnt, lpSS->Col.HeaderCnt + ColCnt - 1);
SS_AdjustDataRowCnt(lpSS, Row1, Row2);

SS_SetHScrollBar(lpSS);
SS_SetVScrollBar(lpSS);
}


HSZ VBSSBoundGetFieldName(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, SS_COORD Col)
{
SS_HCTL    hCtl = (SS_HCTL)lObject;             //SSOCX
DATAACCESS da;
LPSS_COL   lpCol;
BOOL       fIsRegistered;
HSZ        hszText = 0;

if (VBSSBoundIsBound(lObject, lpSS))
   {
   _fmemset(&da, 0, sizeof(da));
   da.usVersion = VB_VERSION;
   da.hctlData = (HCTL)ssVBGetDataSourceControl(lObject, &fIsRegistered);
   da.hctlBound = (HCTL)hCtl;

   if (lpCol = SS_LockColItem(lpSS, Col))
      {
      if (lpCol->dFieldNum)
         {
         da.sDataFieldIndex = lpCol->dFieldNum - 1;
         DataCall(&da, GET, FIELDNAME, lpSS);
         if (da.lData)
            {
            hszText = ssVBCreateHsz(lObject, ssVBDerefHsz((HSZ)da.lData));
            ssVBDestroyHsz((HSZ)da.lData);
            }
         }

      SS_UnlockColItem(lpSS, Col);
      }
   }

if (!hszText)
   hszText = ssVBCreateHsz(lObject, _T(""));

return (hszText);
}


#ifdef SS_BOUNDDEBUG
void LogMsg(LPTSTR lpStr)
{
OFSTRUCT    OfStruct;
HFILE       hFile;
TCHAR       Buffer[111];
static TCHAR Dir[144];

if (!Dir[0])
   {
   GetSystemDirectory(Dir, STRING_SIZE(Dir));
   lstrcpy(&Dir[lstrlen(Dir)], _T("\\fp.log"));
   }

#ifdef _UNICODE  // if unicode, convert WCHAR string to CHAR string for 
                // functions which take LPSTR (not LPWSTR).
   {
     CHAR szBuff[STRING_SIZE(Dir)];
     WideCharToMultiByte(CP_ACP, 0, Dir, -1, szBuff, sizeof(szBuff);
     strcpy((LPSTR)Dir, szBuff);
   }
#endif

if (OpenFile((LPSTR)Dir, &OfStruct, OF_EXIST) == HFILE_ERROR)
   {
   hFile = OpenFile((LPSTR)Dir, &OfStruct, OF_CREATE | OF_WRITE);
   _lclose(hFile);
   }

hFile = _lopen((LPSTR)Dir, WRITE);

lstrcpy(Buffer, lpStr);
lstrcpy(&Buffer[lstrlen(Buffer)], _T("\n"));

_llseek(hFile, 0, 2);
_lwrite(hFile, Buffer, lstrlen(Buffer)*sizeof(TCHAR));
_lclose(hFile);
}
#endif

//DBOCX...
ULONG ssVBSetControlFlags(LONG lObject, ULONG mask, ULONG value, LPSPREADSHEET lpSS)
{
#ifdef SS_OCX
  DATAACCESS da;

  // if mask is 0, then GET flags
  if (mask == (ULONG)0)
  {
    DataCall( &da, GET, UCTLFLAGS, lpSS);
  }
  // else Setting and/or Resetting bits
  else 
  {
    // Get flags, RESET bits specified in mask, SET bits in (value & mask)
    DataCall( &da, GET, UCTLFLAGS, lpSS);
  da.lData &= ~mask;              // reset bits specified by mask
  da.lData |= (value & mask);   // set value bits allowed by mask
    DataCall( &da, SET, UCTLFLAGS, lpSS);
  }

  return (ULONG)da.lData;

#else // VBX
  return VBSetControlFlags( (HCTL)lObject, mask, value);
#endif
} //ssVBSetControlFlags()
//DBOCX.


void DLLENTRY vbSS_SetBookmark(LPVOID lpDestBookMark, LPVOID srcBookMark)
{
  // NOTE: lpDestBookMark is actually "HLSTR FAR *", use "*lpDestBookMark"
  // for the bookmark.

  // BJO 03Oct96 JAP4025 - Begin fix
  if( *(HLSTR FAR*)lpDestBookMark == (HLSTR)srcBookMark )
    return;
  // BJO 03Oct96 JAP4025 - End fix

//#ifdef SS_OCX
  // If a bookmark is already there, free it.
  if (*(HLSTR FAR *)lpDestBookMark)
    ssVBDestroyHlstr(*(HLSTR FAR *)lpDestBookMark);
//#endif

  // assign the new bookmark
  *(HLSTR FAR *)lpDestBookMark = (HLSTR)srcBookMark;

  return;
}

static BOOL BookmarksEqual( HLSTR b1, HLSTR b2)
{
  LONG l1 = 0;
  LONG l2 = 0;
  BOOL fReturn = FALSE;

  if (b1)
     l1 = (LONG)ssVBGetHlstrLen(b1);
  if (b2)
     l2 = (LONG)ssVBGetHlstrLen(b2);

  // if both lengths are not 0 and are equal, compare bytes 
  if (l1 && l2 && (l1 == l2))
#ifdef SS_OCX
     fReturn = (_fmemcmp( (LPSTR)b1, (LPSTR)b2, (size_t)l1) == 0);
#else
     {
     LPVOID lpvSrc;
     LPVOID lpvDest;

     lpvSrc  = ssVBDerefHlstr(b1);
     lpvDest = ssVBDerefHlstr(b2);
     fReturn = (_fmemcmp(lpvSrc, lpvDest, (size_t)l1) == 0);
     }
#endif

  return fReturn;
}


//*************************************************************************
//*
//*  NOTICE:  For SS_OCX only!  Copied from VBSPREAD.C
//*
//*************************************************************************
#ifdef SS_OCX

BOOL vbSpreadSaveBoundRow(LPSPREADSHEET lpSS, LONG lObject, HWND hWnd, SS_COORD Row, SS_COORD RowNew,
                          SS_COORD ColNew)
{
SS_HCTL    hCtl = (SS_HCTL)lObject;             //SSOCX
LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
LPSS_ROW   lpRow;
BOOL       fAdd;
BOOL       fCancel = FALSE;
BOOL       fDirty = FALSE;

if (VBSSBoundIsBound(hCtl, lpSS) && !lpSpread->fIgnoreSaveBoundRow)
   {
   fAdd = FALSE;
   if (lpRow = SS_LockRowItem(lpSS, Row))
      {
      fDirty = (lpRow->bDirty != 0);
      if ((lpRow->bDirty == SS_ROWDIRTY_SOME ||
           lpRow->bDirty == SS_ROWDIRTY_ALL) && !lpRow->hlstrBookMark &&
           lpSpread->DataOpts.fAutoSave)
         {
         VBSSBoundAdd(lpSS, hCtl, hWnd, Row, Row, FALSE);
//GRB792 fAdd = TRUE;
//         fCancel = TRUE;
         }

      SS_UnlockRowItem(lpSS, Row);
      }

   if (fAdd)
      {
      lpSpread->PostAddRowNew = RowNew;
      lpSpread->PostAddRowNewCol = (ColNew == -1 ? lpSS->Col.CurAt : ColNew);
      }
   else if (lpSpread->DataOpts.fAutoDataInform && (fDirty || Row != RowNew))
      vbSpreadBoundClick(lpSS, hCtl, RowNew);
#ifdef FP_ADB
// fix for #8720 -scl
   PostMessage(hWnd, SSM_SETACTIVECELLINTERNAL, (ColNew == -1 ? lpSS->Col.CurAt : ColNew), RowNew);
#endif
   }

return (fCancel);
}

#endif

#ifndef SS_OCX

SHORT DataCallPrimative(LPDATAACCESS lpda, UINT msg, UINT action, LPSPREADSHEET lpSS)
{
((LPDATAACCESS)lpda)->sAction = action;
return ((SHORT)VBSendControlMsg(((LPDATAACCESS)lpda)->hctlData, msg,
                                0, (LONG)(LPDATAACCESS)lpda));
}

#endif

//--------------------------------------------------------------------
//
//  The following function converts a double to a time.
//

BOOL SS_DoubleToTime(double dfVal, LPTIME lpTime)
{
  double dfInt;
  long lVal = (long)(modf(dfVal,&dfInt) * 24.0 * 60.0 * 60.0 + 0.5);

  lpTime->nHour = (int)(lVal / 60 / 60);
  lpTime->nMinute = (int)(lVal / 60 % 60);
  lpTime->nSecond = (int)(lVal % 60);
  return TRUE;
}

#if 0
BOOL SS_DoubleToDate(double dfVal, short *pnYear, short *pnMon, short *pnDay)
{
  long lVal = (long)dfVal;

  *pnYear = 1900 + lVal / 30 / 12;
  *pnMon = (lVal / 30) % 12 + 1;
  *pnDay = lVal % 30 + 1;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The following function converts a date to double.
//
//  Note: This function is not complete.
//

BOOL SS_DateToDouble(short nYear, short nMon, short nDay, double *pdfVal)
{
  *pdfVal = (long)(nYear - 1900) * 30 * 12 + (long)(nMon-1) * 30 + (long)(nDay-1);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The following function converts a double to a time.
//

BOOL SS_DoubleToTime(double dfVal, short *pnHour, short *pnMin, short *pnSec)
{
  double dfInt;
  long lVal = (long)(modf(dfVal,&dfInt) * 24.0 * 60.0 * 60.0 + 0.5);

  *pnHour = lVal / 60 / 60;
  *pnMin = lVal / 60 % 60;
  *pnSec = lVal % 60;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The following function converts a time to double.
//

BOOL SS_TimeToDouble(short nHour, short nMin, short nSec, double *pdfVal)
{
  long lVal;

  lVal = (long)nHour * 60 * 60 + (long)nMin * 60 + nSec;
  *pdfVal = (double)lVal / (24.0 * 60.0 * 60.0);
  return TRUE;
}
#endif // 0

#ifdef SS_V70

void SS_DataRefresh(LPSPREADSHEET lpSS, long lObject)
{
SS_HCTL    hCtl = (SS_HCTL)lObject;             //SSOCX
LPVBSPREAD lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

if (VBSSBoundIsBound(hCtl, lpSS))
	{
	if (lpSpread->DataOpts.fAutoFill) // autofill the data into the spread
		{                       
		DATAACCESS da;
		BOOL       fIsRegistered;

		_fmemset(&da, '\0', sizeof(DATAACCESS));
		da.usVersion = VB_VERSION;
		da.hctlData = (HCTL)ssVBGetDataSourceControl(lObject, &fIsRegistered);
		da.hctlBound = (HCTL)lObject;

		if (lpSS->fVirtualMode)
			SS_VRefreshBuffer(lpSS, TRUE);
		else
			FillAllDataRows(lpSS, lObject, lpSS->lpBook->hWnd, &da, lpSpread->dFieldsCount);
		}
	}
}

#endif // SS_V70

#endif // SS_BOUNDCONTROL
