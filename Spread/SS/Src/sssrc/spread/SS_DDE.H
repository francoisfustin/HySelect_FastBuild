//--------------------------------------------------------------------
//
//  File: SS_DDE.H
//

#ifndef SS_DDE_H
#define SS_DDE_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------

#define SS_LINKMODE_NONE 0
#define SS_LINKMODE_HOT  1
#define SS_LINKMODE_COLD 2
#define SS_LINKMODE_WARM 3

#define SSM_LINKDISPATCHNOTCREATED      0x0630
#define SSM_LINKERROR                   0x0631
#define SSM_LINKITEMNOTSUPPORTED        0x0632
#define SSM_LINKITEMSUPPORTED           0x0633
#define SSM_LINKNEWDATA                 0x0634
#define SSM_LINKNOTIFY                  0x0635
#define SSM_LINKOUTOFMEMORY             0x0636
#define SSM_LINKPOKECOMPLETED           0x0637
#define SSM_LINKREQUESTCOMPLETED        0x0638
#define SSM_LINKSERVERDISCONNECTED      0x0639
#define SSM_LINKUNABLETOCONNECTTOSERVER 0x0640

typedef struct tagSS_LINKERROR
{
  LPCTSTR lpszServer;
  LPCTSTR lpszTopic;
  LPCTSTR lpszItem;
  LPCTSTR lpszError;
  short nError;
} SS_LINKERROR, FAR* LPSS_LINKERROR;

typedef struct tagSS_LINKPOKECOMPLETED
{
  LPCTSTR lpszServer;
  LPCTSTR lpszTopic;
  LPCTSTR lpszItem;
  long lCode;
} SS_LINKPOKECOMPLETED, FAR* LPSS_LINKPOKECOMPLETED;

typedef struct tagSS_LINKREQUESTCOMPLETED
{
  LPCTSTR lpszServer;
  LPCTSTR lpszTopic;
  LPCTSTR lpszItem;
  long lCode;
} SS_LINKREQUESTCOMPLETED, FAR* LPSS_LINKREQUESTCOMPLETED;

typedef struct tagSS_LINKMSG
{
  LPCTSTR lpszServer;
  LPCTSTR lpszTopic;
  LPCTSTR lpszItem;
} SS_LINKMSG, FAR* LPSS_LINKMSG;

BOOL DLLENTRY SSDdeAddLink(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
BOOL DLLENTRY SSDdeGetDim(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPLONG lplCols, LPLONG lplRows);
BOOL DLLENTRY SSDdeGetLink(HWND hwndSS, long lCol, long lRow, LPTSTR lpszServer, LPTSTR lpszTopic, LPTSTR lpszItem, LPLONG lplItemCol, LPLONG lplItemRow, int nServerLen, int nTopicLen, int nItemLen);
BOOL DLLENTRY SSDdeGetLinkLen(HWND hwndSS, long lCol, long lRow, LPINT lpnServerLen, LPINT lpnTopicLen, LPINT lpnItemLen);
short DLLENTRY SSDdeGetMode(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
int DLLENTRY SSDdeGetReadExpr(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPTSTR lpszExpr, int nLen);
int DLLENTRY SSDdeGetReadExprLen(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
int DLLENTRY SSDdeGetWriteExpr(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPTSTR lpszExpr, int nLen);
int DLLENTRY SSDdeGetWriteExprLen(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
BOOL DLLENTRY SSDdeIsLink(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
BOOL DLLENTRY SSDdePokeDataToLink(HWND hwndSS, long lCol, long lRow);
BOOL DLLENTRY SSDdePokeLink(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lspzTopic, LPCTSTR lpszItem);
BOOL DLLENTRY SSDdeQueryDependCell(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lColPrev, long lRowPrev, LPLONG lplCol, LPLONG lplRow);
BOOL DLLENTRY SSDdeQueryDependCol(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lColPrev, LPLONG lplCol);
int DLLENTRY SSDdeQueryDependName(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszNamePrev, LPTSTR lpszName, int nNameLen);
int DLLENTRY SSDdeQueryDependNameLen(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszNamePrev);
BOOL DLLENTRY SSDdeQueryDependRow(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lRowPrev, LPLONG lplRow);
BOOL DLLENTRY SSDdeQueryLink(HWND hwndSS, LPCTSTR lpszServerPrev, LPCTSTR lpszTopicPrev, LPCTSTR lpszItemPrev, LPTSTR lpszServer, LPTSTR lpszTopic, LPTSTR lpszItem, int nServerLen, int nTopicLen, int nItemLen);
BOOL DLLENTRY SSDdeQueryLinkLen(HWND hwndSS, LPCTSTR lpszServerPrev, LPCTSTR lpszTopicPrev, LPCTSTR lpszItemPrev, LPINT lpnServerLen, LPINT lpnTopicLen, LPINT lpnItemLen);
BOOL DLLENTRY SSDdeRemoveLink(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
BOOL DLLENTRY SSDdeRequestLink(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
BOOL DLLENTRY SSDdeSetLink(HWND hwndSS, long lCol, long lRow, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lItemCol, long lItemRow);
short DLLENTRY SSDdeSetMode(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, short nMode);
BOOL DLLENTRY SSDdeSetReadExpr(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszExpr);
BOOL DLLENTRY SSDdeSetWriteExpr(HWND hwndSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszExpr);

BOOL DLLENTRY SSDdeGetDesignTime(HWND hwndSS);
BOOL DLLENTRY SSDdeSetDesignTime(HWND hwndSS, BOOL bDesignTime);

//--------------------------------------------------------------------

#if defined(SS_DDE) && (defined(SS_DLL) || defined(SS_OCX))

BOOL SS_DdeAddLink(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
BOOL SS_DdeGetDim(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPLONG lplCols, LPLONG lplRows);
BOOL SS_DdeGetLink(LPSPREADSHEET lpSS, long lCol, long lRow, LPTSTR lpszServer, LPTSTR lpszTopic, LPTSTR lpszItem, LPLONG lplItemCol, LPLONG lplItemRow, int nServerLen, int nTopicLen, int nItemLen);
BOOL SS_DdeGetLinkLen(LPSPREADSHEET lpSS, long lCol, long lRow, LPINT lpnServerLen, LPINT lpnTopicLen, LPINT lpnItemLen);
short SS_DdeGetMode(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
int SS_DdeGetReadExpr(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPTSTR lpszExpr, int nLen);
int SS_DdeGetReadExprLen(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
int SS_DdeGetWriteExpr(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPTSTR lpszExpr, int nLen);
int SS_DdeGetWriteExprLen(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
BOOL SS_DdeIsLink(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
BOOL SS_DdePokeDataToLink(LPSPREADSHEET lpSS, long lCol, long lRow);
BOOL SS_DdePokeLink(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lspzTopic, LPCTSTR lpszItem);
BOOL SS_DdeQueryDependCell(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lColPrev, long lRowPrev, LPLONG lplCol, LPLONG lplRow);
BOOL SS_DdeQueryDependCol(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lColPrev, LPLONG lplCol);
int SS_DdeQueryDependName(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszNamePrev, LPTSTR lpszName, int nNameLen);
int SS_DdeQueryDependNameLen(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszNamePrev);
BOOL SS_DdeQueryDependRow(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lRowPrev, LPLONG lplRow);
BOOL SS_DdeQueryLink(LPSPREADSHEET lpSS, LPCTSTR lpszServerPrev, LPCTSTR lpszTopicPrev, LPCTSTR lpszItemPrev, LPTSTR lpszServer, LPTSTR lpszTopic, LPTSTR lpszItem, int nServerLen, int nTopicLen, int nItemLen);
BOOL SS_DdeQueryLinkLen(LPSPREADSHEET lpSS, LPCTSTR lpszServerPrev, LPCTSTR lpszTopicPrev, LPCTSTR lpszItemPrev, LPINT lpnServerLen, LPINT lpnTopicLen, LPINT lpnItemLen);
BOOL SS_DdeRemoveLink(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
BOOL SS_DdeRequestLink(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
BOOL SS_DdeSetLink(LPSPREADSHEET lpSS, long lCol, long lRow, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lItemCol, long lItemRow);
short SS_DdeSetMode(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, short nMode);
BOOL SS_DdeSetReadExpr(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszExpr);
BOOL SS_DdeSetWriteExpr(LPSPREADSHEET lpSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszExpr);

BOOL SS_DdeGetDesignTime(LPSPREADSHEET lpSS);
BOOL SS_DdeSetDesignTime(LPSPREADSHEET lpSS, BOOL bDesignTime);

BOOL SS_CalcIsDesignTime(long hSS);
void SS_CalcLinkError(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, LPCTSTR lpszError, short iError);
void SS_CalcLinkItemNotSupported(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkItemSupported(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkNewData(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkNotify(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkOutOfMemory(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkServerDisconnected(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkUnableToConnectToServer(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);
void SS_CalcLinkPokeCompleted(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lCode);
void SS_CalcLinkRequestCompleted(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem, long lCode);
void SS_CalcLinkDispatchNotCreated(long hSS, LPCTSTR lpszServer, LPCTSTR lpszTopic, LPCTSTR lpszItem);

#endif  // defined(SS_DDE) && (defined(SS_DLL) || defined(SS_OCX))

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
