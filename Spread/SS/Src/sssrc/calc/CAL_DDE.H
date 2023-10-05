//--------------------------------------------------------------------
//
//  File: cal_dde.h
//
//  Description: Routnies to support DDE links in formulas
//

#ifndef CAL_DDE_H
#define CAL_DDE_H

#if defined(SS_DDE)

#include <oleauto.h>

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The CALC_DDE structure represents a DDE link.
//

typedef struct tagCALC_DDE
{
  CALC_HANDLE hServer;    // server name used in DDE link
  CALC_HANDLE hTopic;     // topic name used in DDE link
  CALC_HANDLE hItem;      // item name used in DDE link
  CALC_HANDLE hReadExpr;  // read expression used in DDE link
  CALC_HANDLE hWriteExpr; // write expression used in DDE link
  CALC_HANDLE hValues;    // value matrix (CALC_VALUE) passed from DDE link
  long lValueCols;        // number of columns in value matrix
  long lValueRows;        // number of rows in value matrix
  long lRefCnt;           // reference count
  CALC_NAMETABLE DependNames;  // dependent names which use this DDE link
  BITMATRIX DependCells;       // dependent cells which use this DDE link
  BITVECTOR DependCols;        // dependent columns which use this DDE link
  BITVECTOR DependRows;        // dependent rows which use this DDE link
  LPCALC_DDEPROC lpfnDdeUpdate;// call back notification of new DDE data
  long hSS;                    // first parameter to call back
  short nMode;                 // type of link (i.e. hot, cold, or warm)
  LPDISPATCH lpdispLink;       // dispatch of RSJBoxLink object
  DWORD dwCookie;              // returned from IConnectPoint::Advise()
} CALC_DDE, FAR* LPCALC_DDE;

#define CALC_LINKMODE_NONE 0
#define CALC_LINKMODE_HOT  1
#define CALC_LINKMODE_COLD 2
#define CALC_LINKMODE_WARM 3

//--------------------------------------------------------------------
//
//  The following functions are used to handle a table of DDE links.
//

BOOL DdeInit(LPCALC_DDETABLE lpTable);
BOOL DdeFree(LPCALC_DDETABLE lpTable);
BOOL DdeAdd(LPCALC_DDETABLE lpTable, CALC_HANDLE hDde);
BOOL DdeRemove(LPCALC_DDETABLE lpTable, CALC_HANDLE hDde);
CALC_HANDLE DdeLookup(LPCALC_DDETABLE lpTable, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
CALC_HANDLE DdeFirst(LPCALC_DDETABLE lpTable);
CALC_HANDLE DdeNext(LPCALC_DDETABLE lpTable, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);

//--------------------------------------------------------------------
//
//  The following functions are used to handle an individual DDE link.
//

CALC_HANDLE DdeCreate(LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem,
                      LPCALC_DDEPROC lpfnDdeUpdate, long hSS);
CALC_HANDLE DdeCreateRef(CALC_HANDLE hDde);
BOOL DdeDestroy(CALC_HANDLE hDde);
BOOL DdeGetValue(CALC_HANDLE hDde, long lCol, long lRow, LPCALC_VALUE lpValue);
BOOL DdeSetValue(CALC_HANDLE hDde, long lCol, long lRow, LPCALC_VALUE lpValue);
BOOL DdeGetValues(CALC_HANDLE hDde, LPVARIANT lpVar);
BOOL DdeSetValues(CALC_HANDLE hDde, long lCols, long lRows, LPVARIANT lpVar);
long DdeMaxIndex(CALC_HANDLE hDde);
long DdeMaxCol(CALC_HANDLE hDde);
long DdeMaxRow(CALC_HANDLE hDde);
int DdeGetText(CALC_HANDLE hDde, long lCol, long lRow, LPTSTR lpszText, int nLen);
int DdeGetTextLen(CALC_HANDLE hDde, long lCol, long lRow);
int DdeGetServer(CALC_HANDLE hDde, LPTSTR lpszText, int nLen);
int DdeGetServerLen(CALC_HANDLE hDde);
int DdeGetTopic(CALC_HANDLE hDde, LPTSTR lpszText, int nLen);
int DdeGetTopicLen(CALC_HANDLE hDde);
int DdeGetItem(CALC_HANDLE hDde, LPTSTR lpszText, int nLen);
int DdeGetItemLen(CALC_HANDLE hDde);
long DdeGetRefCnt(CALC_HANDLE hDde);
int DdeGetReadExpr(CALC_HANDLE hDde, LPTSTR lpszText, int nLen);
int DdeGetReadExprLen(CALC_HANDLE hDde);
BOOL DdeSetReadExpr(CALC_HANDLE hDde, LPCTSTR lpszText);
int DdeGetWriteExpr(CALC_HANDLE hDde, LPTSTR lpszText, int nLen);
int DdeGetWriteExprLen(CALC_HANDLE hDde);
BOOL DdeSetWriteExpr(CALC_HANDLE hDde, LPCTSTR lpszText);
short DdeGetMode(CALC_HANDLE hDde);
short DdeSetMode(CALC_HANDLE hDde, short nMode);
BOOL DdeRequest(CALC_HANDLE hDde);
BOOL DdePoke(CALC_HANDLE hDde);

BOOL DdeSetLinkToCell(CALC_HANDLE hDde, long lCol, long lRow, BOOL bLink);
BOOL DdeSetLinkToCol(CALC_HANDLE hDde, long lCol, BOOL bLink);
BOOL DdeSetLinkToRow(CALC_HANDLE hDde, long lRow, BOOL bLink);
BOOL DdeSetLinkToName(CALC_HANDLE hDde, CALC_HANDLE hhDepend, BOOL bLink);

BOOL DdeMarkDepend(CALC_HANDLE hDde, LPBITMATRIX lpDependCells, LPBITVECTOR lpDependCols, LPBITVECTOR lpDependRows);

BOOL DdeUpdate(CALC_HANDLE hDde);

BOOL DdeFirstDependCell(CALC_HANDLE hDde, LPLONG lplCol, LPLONG lplRow);
BOOL DdeNextDependCell(CALC_HANDLE hDde, long lColPrev, long lRowPrev, LPLONG lplCol, LPLONG lplRow);
BOOL DdeFirstDependCol(CALC_HANDLE hDde, LPLONG lplCol);
BOOL DdeNextDependCol(CALC_HANDLE hDde, long lColPrev, LPLONG lplCol);
BOOL DdeFirstDependRow(CALC_HANDLE hDde, LPLONG lplRow);
BOOL DdeNextDependRow(CALC_HANDLE hDde, long lRowPrev, LPLONG lplRow);
CALC_HANDLE DdeFirstDependName(CALC_HANDLE hDde);
CALC_HANDLE DdeNextDependName(CALC_HANDLE hDde, LPCTSTR lpszNamePrev);

void DdeSetDesignTime(CALC_HANDLE hDde, BOOL bDesignTime);

//--------------------------------------------------------------------
//
//  Call back routines for firing events.
//

BOOL SS_CalcIsDesignTime(long hSS);
void SS_CalcLinkDispatchNotCreated(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkError(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszError, short iError);
void SS_CalcLinkItemNotSupported(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkItemSupported(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkNewData(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkNotify(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkOutOfMemory(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkPokeCompleted(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lCode);
void SS_CalcLinkRequestCompleted(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lCode);
void SS_CalcLinkServerDisconnected(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkUnableToConnectToServer(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif  // defined(SS_DDE)

#endif
