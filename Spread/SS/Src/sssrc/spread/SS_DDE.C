//--------------------------------------------------------------------
//
//  File: SS_DDE.C
//

#include <windows.h>
#include "spread.h"
#include "ss_formu.h"
#include "ss_dde.h"
#include "..\calc\calc.h"
#include "..\calc\cal_cell.h"
#include "..\calc\cal_col.h"
#include "..\calc\cal_dde.h"
#include "..\calc\cal_expr.h"
#include "..\calc\cal_func.h"
#include "..\calc\cal_name.h"
#include "..\calc\cal_row.h"

//--------------------------------------------------------------------

BOOL SS_DdeQueryLink(LPSPREADSHEET lpSS, LPCTSTR lpszServerPrev, LPCTSTR lpszTopicPrev, LPCTSTR lpszItemPrev, LPTSTR lpszServer, LPTSTR lpszTopic, LPTSTR lpszItem, int nServerLen, int nTopicLen, int nItemLen)
{
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( lpszServerPrev && lpszServerPrev[0] )
    hDde = DdeNext(&lpSS->CalcInfo.DdeLinks, lpszServerPrev, lpszTopicPrev, lpszItemPrev);
  else
    hDde = DdeFirst(&lpSS->CalcInfo.DdeLinks);
  if( hDde )
  {
    DdeGetServer(hDde, lpszServer, nServerLen);
    DdeGetTopic(hDde, lpszTopic, nTopicLen);
    DdeGetItem(hDde, lpszItem, nItemLen);
    bRet = TRUE;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSDdeQueryLink(HWND hwndSS, LPCTSTR lpszServerPrev, LPCTSTR lpszTopicPrev, LPCTSTR lpszItemPrev, LPTSTR lpszServer, LPTSTR lpszTopic, LPTSTR lpszItem, int nServerLen, int nTopicLen, int nItemLen)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeQueryLink(lpSS, lpszServerPrev, lpszTopicPrev, lpszItemPrev, lpszServer, lpszTopic, lpszItem, nServerLen, nTopicLen, nItemLen);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_DdeQueryLinkLen(LPSPREADSHEET lpSS, LPCTSTR lpszServerPrev, LPCTSTR lpszTopicPrev, LPCTSTR lpszItemPrev, LPINT lpnServerLen, LPINT lpnTopicLen, LPINT lpnItemLen)
{
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( lpszServerPrev && lpszServerPrev[0] )
    hDde = DdeNext(&lpSS->CalcInfo.DdeLinks, lpszServerPrev, lpszTopicPrev, lpszItemPrev);
  else
    hDde = DdeFirst(&lpSS->CalcInfo.DdeLinks);
  if( hDde )
  {
    *lpnServerLen = DdeGetServerLen(hDde);
    *lpnTopicLen = DdeGetTopicLen(hDde);
    *lpnItemLen = DdeGetItemLen(hDde);
    bRet = TRUE;
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSDdeQueryLinkLen(HWND hwndSS, LPCTSTR lpszServerPrev, LPCTSTR lpszTopicPrev, LPCTSTR lpszItemPrev, LPINT lpnServerLen, LPINT lpnTopicLen, LPINT lpnItemLen)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeQueryLinkLen(lpSS, lpszServerPrev, lpszTopicPrev, lpszItemPrev, lpnServerLen, lpnTopicLen, lpnItemLen);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_DdeAddLink(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( !DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
  {
    hDde = DdeCreate(lpszServer, lpszTopic, lpszItem, lpSS->CalcInfo.lpfnDdeUpdate, lpSS->CalcInfo.hSS);
    bRet = DdeAdd(&lpSS->CalcInfo.DdeLinks, hDde);
    DdeDestroy(hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSDdeAddLink(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeAddLink(lpSS, lpszServer, lpszTopic, lpszItem);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_DdeRemoveLink(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
  {
    if( 1 == DdeGetRefCnt(hDde) )
      bRet = DdeRemove(&lpSS->CalcInfo.DdeLinks, hDde);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSDdeRemoveLink(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeRemoveLink(lpSS, lpszServer, lpszTopic, lpszItem);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_DdeRequestLink(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
    bRet = DdeRequest(hDde);
  return bRet;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSDdeRequestLink(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeRequestLink(lpSS, lpszServer, lpszTopic, lpszItem);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_DdePokeLink(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
    bRet = DdePoke(hDde);
  return bRet;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSDdePokeLink(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdePokeLink(lpSS, lpszServer, lpszTopic, lpszItem);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_DdePokeDataToLink(LPSPREADSHEET lpSS, long lCol, long lRow)
{
  CALC_HANDLE hExpr;
  CALC_HANDLE hDde;
  CALC_VALUE val;
  long lDdeCol, lDdeRow;
  BOOL bRet = FALSE;

  if( hExpr = CellGetExpr(&lpSS->CalcInfo.Cells, lCol, lRow) )
  {
    if( hDde = ExprGetDde(hExpr, &lDdeCol, &lDdeRow) )
    {
      lpSS->CalcInfo.lpfnGetData(lpSS->CalcInfo.hSS, lCol, lRow, &val);
      DdeSetValue(hDde, lDdeCol, lDdeRow, &val);
      ValFree(&val);
    }
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSDdePokeDataToLink(HWND hwndSS, long lCol, long lRow)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdePokeDataToLink(lpSS, lCol, lRow);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

short SS_DdeGetMode(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  CALC_HANDLE hDde;
  short nRet = 0;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
    nRet = DdeGetMode(hDde);
  return nRet;
}

//--------------------------------------------------------------------

short DLLENTRY SSDdeGetMode(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  LPSPREADSHEET lpSS;
  short nRet = 0;

  if( lpSS = SS_Lock(hwndSS) )
  {
    nRet = SS_DdeGetMode(lpSS, lpszServer, lpszTopic, lpszItem);
    SS_Unlock(hwndSS);
  }
  return nRet;
}

//--------------------------------------------------------------------

short SS_DdeSetMode(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, short nMode)
{
  CALC_HANDLE hDde;
  short nRet = 0;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
    nRet = DdeSetMode(hDde, nMode);
  return nRet;
}

//--------------------------------------------------------------------

short DLLENTRY SSDdeSetMode(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, short nMode)
{
  LPSPREADSHEET lpSS;
  short nRet = 0;

  if( lpSS = SS_Lock(hwndSS) )
  {
    nRet = SS_DdeSetMode(lpSS, lpszServer, lpszTopic, lpszItem, nMode);
    SS_Unlock(hwndSS);
  }
  return nRet;
}

//--------------------------------------------------------------------

int SS_DdeGetReadExpr(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPTSTR lpszExpr, int nLen)
{
  CALC_HANDLE hDde;
  int nRet = 0;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
    nRet = DdeGetReadExpr(hDde, lpszExpr, nLen);
  return nRet;
}

//--------------------------------------------------------------------

int DLLENTRY SSDdeGetReadExpr(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPTSTR lpszExpr, int nLen)
{
  LPSPREADSHEET lpSS;
  int nRet = 0;

  if( lpSS = SS_Lock(hwndSS) )
  {
    nRet = SS_DdeGetReadExpr(lpSS, lpszServer, lpszTopic, lpszItem, lpszExpr, nLen);
    SS_Unlock(hwndSS);
  }
  return nRet;
}

//--------------------------------------------------------------------

int SS_DdeGetReadExprLen(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  CALC_HANDLE hDde;
  int nRet = 0;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
    nRet = DdeGetReadExprLen(hDde);
  return nRet;
}

//--------------------------------------------------------------------

int DLLENTRY SSDdeGetReadExprLen(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  LPSPREADSHEET lpSS;
  int nRet = 0;

  if( lpSS = SS_Lock(hwndSS) )
  {
    nRet = SS_DdeGetReadExprLen(lpSS, lpszServer, lpszTopic, lpszItem);
    SS_Unlock(hwndSS);
  }
  return nRet;
}

//--------------------------------------------------------------------

BOOL SS_DdeSetReadExpr(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszExpr)
{
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
    bRet = DdeSetReadExpr(hDde, lpszExpr);
  return bRet;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSDdeSetReadExpr(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszExpr)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeSetReadExpr(lpSS, lpszServer, lpszTopic, lpszItem, lpszExpr);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

int SS_DdeGetWriteExpr(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPTSTR lpszExpr, int nLen)
{
  CALC_HANDLE hDde;
  int nRet = 0;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
    nRet = DdeGetWriteExpr(hDde, lpszExpr, nLen);
  return nRet;
}

//--------------------------------------------------------------------

int DLLENTRY SSDdeGetWriteExpr(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPTSTR lpszExpr, int nLen)
{
  LPSPREADSHEET lpSS;
  int nRet = 0;

  if( lpSS = SS_Lock(hwndSS) )
  {
    nRet = SS_DdeGetWriteExpr(lpSS, lpszServer, lpszTopic, lpszItem, lpszExpr, nLen);
    SS_Unlock(hwndSS);
  }
  return nRet;
}

//--------------------------------------------------------------------

int SS_DdeGetWriteExprLen(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  CALC_HANDLE hDde;
  int nRet = 0;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
    nRet = DdeGetWriteExprLen(hDde);
  return nRet;
}

//--------------------------------------------------------------------

int DLLENTRY SSDdeGetWriteExprLen(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  LPSPREADSHEET lpSS;
  int nRet = 0;

  if( lpSS = SS_Lock(hwndSS) )
  {
    nRet = SS_DdeGetWriteExprLen(lpSS, lpszServer, lpszTopic, lpszItem);
    SS_Unlock(hwndSS);
  }
  return nRet;
}

//--------------------------------------------------------------------

BOOL SS_DdeSetWriteExpr(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszExpr)
{
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
    bRet = DdeSetWriteExpr(hDde, lpszExpr);
  return bRet;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSDdeSetWriteExpr(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszExpr)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeSetWriteExpr(lpSS, lpszServer, lpszTopic, lpszItem, lpszExpr);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_DdeSetLink(LPSPREADSHEET lpSS, long lCol, long lRow, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lItemCol, long lItemRow)
{
  /*
  CALC_HANDLE hDde;
  TCHAR szExpr[128];
  BOOL bRet = FALSE;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
    hDde = DdeCreateRef(hDde);
  else if( hDde = DdeCreate(lpszServer, lpszTopic, lpszItem,
                            lpSS->CalcInfo.lpfnDdeUpdate, lpSS->CalcInfo.hSS) )
    DdeAdd(&lpSS->CalcInfo.DdeLinks, hDde);
  if( hDde )
  {
    DdeGetText(hDde, lCol, lRow, szExpr, sizeof(szExpr)/sizeof(szExpr[0]));
    bRet = SS_SetFormulaRange(lpSS, lCol, lRow, lCol, lRow, szExpr, TRUE);
  }
  return bRet;
  */

  CALC_HANDLE hDde;
  CALC_HANDLE hExpr;
  CALC_HANDLE hExprPrev;
  BOOL bRet = FALSE;

  if( lpszServer && lpszTopic && lpszItem &&
      lpszServer[0] && lpszTopic[0] && lpszItem[0] )
  {
    if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
      hDde = DdeCreateRef(hDde);
    else if( hDde = DdeCreate(lpszServer, lpszTopic, lpszItem,
                              lpSS->CalcInfo.lpfnDdeUpdate, lpSS->CalcInfo.hSS) )
    {
      DdeAdd(&lpSS->CalcInfo.DdeLinks, hDde);
      DdeSetMode(hDde, CALC_LINKMODE_HOT);
    }
    if( hDde )
    {
      if( hExpr = ExprCreateFromDde(hDde, lItemCol, lItemRow) )
      {
        if( CALC_ALLCOLS != lCol && CALC_ALLROWS != lRow )
        {
          hExprPrev = CellSetExpr(&lpSS->CalcInfo, lCol, lRow, hExpr);
          BitMatSet(&lpSS->CalcInfo.CellsToEval, lCol, lRow, TRUE);
          ExprDestroy(hExprPrev);
          bRet = TRUE;
        }
        else if(  CALC_ALLROWS != lRow )
        {
          hExprPrev = RowSetExpr(&lpSS->CalcInfo, lRow, hExpr);
          BitVectSet(&lpSS->CalcInfo.RowsToEval, lRow, TRUE);
          ExprDestroy(hExprPrev);
          bRet = TRUE;
        }
        else if( CALC_ALLCOLS != lCol )
        {
          hExprPrev = ColSetExpr(&lpSS->CalcInfo, lCol, hExpr);
          BitVectSet(&lpSS->CalcInfo.ColsToEval, lCol, TRUE);
          ExprDestroy(hExprPrev);
          bRet = TRUE;
        }
      }
      DdeDestroy(hDde);
    }
    if (lpSS->lpBook->CalcAuto)
       CalcEvalNeededCells(&lpSS->lpBook->CalcInfo);
  }
  else
  {
    bRet = SS_SetFormulaRange(lpSS, lCol, lRow, lCol, lRow, NULL, TRUE);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSDdeSetLink(HWND hwndSS, long lCol, long lRow, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lItemCol, long lItemRow)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeSetLink(lpSS, lCol, lRow, lpszServer, lpszTopic, lpszItem, lItemCol, lItemRow);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_DdeGetLink(LPSPREADSHEET lpSS, long lCol, long lRow, LPTSTR lpszServer, LPTSTR lpszTopic, LPTSTR lpszItem, LPLONG lplItemCol, LPLONG lplItemRow, int nServerLen, int nTopicLen, int nItemLen)
{
  CALC_HANDLE hExpr = 0;
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( CALC_ALLCOLS != lCol && CALC_ALLROWS != lRow )
    hExpr = CellGetExpr(&lpSS->CalcInfo, lCol, lRow);
  if( !hExpr && CALC_ALLROWS != lRow )
    hExpr = RowGetExpr(&lpSS->CalcInfo, lRow);
  if( !hExpr && CALC_ALLCOLS != lCol )
    hExpr = ColGetExpr(&lpSS->CalcInfo, lCol);
  if( hExpr  )
  {
    if( hDde = ExprGetDde(hExpr, lplItemCol, lplItemRow) )
    {
      DdeGetServer(hDde, lpszServer, nServerLen);
      DdeGetTopic(hDde, lpszTopic, nTopicLen);
      DdeGetItem(hDde, lpszItem, nItemLen);
      bRet = TRUE;
    }
  }
  if( !bRet )
  {
    if( lpszServer && nServerLen > 0 )
      lstrcpy(lpszServer, _T(""));
    if( lpszTopic && nTopicLen > 0 )
      lstrcpy(lpszTopic, _T(""));
    if( lpszItem && nItemLen > 0 )
      lstrcpy(lpszItem, _T(""));
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSDdeGetLink(HWND hwndSS, long lCol, long lRow, LPTSTR lpszServer, LPTSTR lpszTopic, LPTSTR lpszItem, LPLONG lplItemCol, LPLONG lplItemRow, int nServerLen, int nTopicLen, int nItemLen)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeGetLink(lpSS, lCol, lRow, lpszServer, lpszTopic, lpszItem, lplItemCol, lplItemRow, nServerLen, nTopicLen, nItemLen);
    SS_Unlock(hwndSS);
  }
  return bRet;
}


//--------------------------------------------------------------------

BOOL SS_DdeGetLinkLen(LPSPREADSHEET lpSS, long lCol, long lRow, LPINT lpnServerLen, LPINT lpnTopicLen, LPINT lpnItemLen)
{
  CALC_HANDLE hExpr;
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( hExpr = CellGetExpr(&lpSS->CalcInfo.Cells, lCol, lRow) )
  {
    if( hDde = ExprGetDde(hExpr, NULL, NULL) )
    {
      if( lpnServerLen )
        *lpnServerLen = DdeGetServerLen(hDde);
      if( lpnTopicLen )
        *lpnTopicLen = DdeGetTopicLen(hDde);
      if( lpnItemLen )
        *lpnItemLen = DdeGetItemLen(hDde);
      bRet = TRUE;
    }
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL DLLENTRY SSDdeGetLinkLen(HWND hwndSS, long lCol, long lRow, LPINT lpnServerLen, LPINT lpnTopicLen, LPINT lpnItemLen)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeGetLinkLen(lpSS, lCol, lRow, lpnServerLen, lpnTopicLen, lpnItemLen);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_DdeQueryDependCell(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lColPrev, long lRowPrev, LPLONG lplCol, LPLONG lplRow)
{
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
  {
    if( -1 == lColPrev && -1 == lRowPrev )
      bRet = DdeFirstDependCell(hDde, lplCol, lplRow);
    else
      bRet = DdeNextDependCell(hDde, lColPrev, lRowPrev, lplCol, lplRow);
  }
  return bRet;
}

BOOL DLLENTRY SSDdeQueryDependCell(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lColPrev, long lRowPrev, LPLONG lplCol, LPLONG lplRow)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeQueryDependCell(lpSS, lpszServer, lpszTopic, lpszItem, lColPrev, lRowPrev, lplCol, lplRow);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_DdeQueryDependCol(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lColPrev, LPLONG lplCol)
{
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
  {
    if( -1 == lColPrev )
      bRet = DdeFirstDependCol(hDde, lplCol);
    else
      bRet = DdeNextDependCol(hDde, lColPrev, lplCol);
  }
  return bRet;
}

BOOL DLLENTRY SSDdeQueryDependCol(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lColPrev, LPLONG lplCol)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeQueryDependCol(lpSS, lpszServer, lpszTopic, lpszItem, lColPrev, lplCol);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_DdeQueryDependRow(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lRowPrev, LPLONG lplRow)
{
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
  {
    if( -1 == lRowPrev )
      bRet = DdeFirstDependRow(hDde, lplRow);
    else
      bRet = DdeNextDependRow(hDde, lRowPrev, lplRow);
  }
  return bRet;
}

BOOL DLLENTRY SSDdeQueryDependRow(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lRowPrev, LPLONG lplRow)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeQueryDependRow(lpSS, lpszServer, lpszTopic, lpszItem, lRowPrev, lplRow);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

int SS_DdeQueryDependName(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszNamePrev, LPTSTR lpszName, int nNameLen)
{
  CALC_HANDLE hDde;
  CALC_HANDLE hName = 0;
  int nRet = 0;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
  {
    if( lpszNamePrev && lpszNamePrev[0] )
      hName = DdeNextDependName(hDde, lpszNamePrev);
    else
      hName = DdeFirstDependName(hDde);
    if( hName )
      nRet = NameGetText(hName, lpszName, nNameLen);
  }
  return nRet;
}

int DLLENTRY SSDdeQueryDependName(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszNamePrev, LPTSTR lpszName, int nNameLen)
{
  LPSPREADSHEET lpSS;
  int nRet = 0;

  if( lpSS = SS_Lock(hwndSS) )
  {
    nRet = SS_DdeQueryDependName(lpSS, lpszServer, lpszTopic, lpszItem, lpszNamePrev, lpszName, nNameLen);
    SS_Unlock(hwndSS);
  }
  return nRet;
}

//--------------------------------------------------------------------

int SS_DdeQueryDependNameLen(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszNamePrev)
{
  CALC_HANDLE hDde;
  CALC_HANDLE hName = 0;
  int nRet = 0;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
  {
    if( lpszNamePrev && lpszNamePrev[0] )
      hName = DdeNextDependName(hDde, lpszNamePrev);
    else
      hName = DdeFirstDependName(hDde);
    if( hName )
      nRet = NameGetTextLen(hName);
  }
  return nRet;
}

int DLLENTRY SSDdeQueryDependNameLen(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszNamePrev)
{
  LPSPREADSHEET lpSS;
  int nRet = 0;

  if( lpSS = SS_Lock(hwndSS) )
  {
    nRet = SS_DdeQueryDependNameLen(lpSS, lpszServer, lpszTopic, lpszItem, lpszNamePrev);
    SS_Unlock(hwndSS);
  }
  return nRet;
}

//--------------------------------------------------------------------

BOOL SS_DdeGetDim(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPLONG lplCols, LPLONG lplRows)
{
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
  {
    if( lplCols )
      *lplCols = DdeMaxCol(hDde);
    if( lplRows )
      *lplRows = DdeMaxRow(hDde);
    bRet = TRUE;
  }
  return bRet;
}

BOOL DLLENTRY SSDdeGetDim(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPLONG lplCols, LPLONG lplRows)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeGetDim(lpSS, lpszServer, lpszTopic, lpszItem, lplCols, lplRows);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

BOOL SS_DdeIsLink(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  CALC_HANDLE hDde;
  BOOL bRet = FALSE;

  if( hDde = DdeLookup(&lpSS->CalcInfo.DdeLinks, lpszServer, lpszTopic, lpszItem) )
  {
    bRet = TRUE;
  }
  return bRet;
}

BOOL DLLENTRY SSDdeIsLink(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeIsLink(lpSS, lpszServer, lpszTopic, lpszItem);
    SS_Unlock(hwndSS);
  }
  return bRet;
}

//--------------------------------------------------------------------

#if defined(SS_RSDLL)
BOOL SS_DdeGetDesignTime(LPSPREADSHEET lpSS)
{
  return lpSS->bDesignTime;
}
#endif

#if defined(SS_RSDLL)
BOOL DLLENTRY SSDdeGetDesignTime(HWND hwndSS)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeGetDesignTime(lpSS);
    SS_Unlock(hwndSS);
  }
  return bRet;
}
#endif

//--------------------------------------------------------------------

#if defined(SS_RSDLL)
BOOL SS_DdeSetDesignTime(LPSPREADSHEET lpSS, BOOL bDesignTime)
{
  LPCALC_HANDLE lpElem;
  int i;
  BOOL bRet;

  bRet = lpSS->bDesignTime;
  lpSS->bDesignTime = bDesignTime;
  if( lpSS->CalcInfo.DdeLinks.hElem &&
      (lpElem = (LPCALC_HANDLE)CalcMemLock(lpSS->CalcInfo.DdeLinks.hElem)) )
  {
    for( i = 0; i < lpSS->CalcInfo.DdeLinks.nElemCnt; i++ )
      DdeSetDesignTime(lpElem[i], bDesignTime);
    CalcMemUnlock(lpSS->CalcInfo.DdeLinks.hElem);
  }
  return bRet;
}
#endif

#if defined(SS_RSDLL)
BOOL DLLENTRY SSDdeSetDesignTime(HWND hwndSS, BOOL bDesignTime)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = SS_Lock(hwndSS) )
  {
    bRet = SS_DdeSetDesignTime(lpSS, bDesignTime);
    SS_Unlock(hwndSS);
  }
  return bRet;
}
#endif

//--------------------------------------------------------------------

#if defined(SS_RSDLL)
BOOL SS_CalcIsDesignTime(long hSS)
{
  LPSPREADSHEET lpSS;
  BOOL bRet = FALSE;

  if( lpSS = (LPSPREADSHEET)tbGlobalLock(hSS) )
  {
    bRet = SS_DdeGetDesignTime(lpSS);
    tbGlobalUnlock(hSS);
  }
  return bRet;
}
#endif
