/************************************************************************/
/* SSOCXDB.H  -  DB-Specific functions for CSsocxCtrl class             */
/************************************************************************/
#ifndef SSOCXDB_H
#define SSOCXDB_H

#ifdef __cplusplus                    // added by SCP for TABPRO.OCX - DB
extern "C" {
#endif

// Interfaces from CSsocxCtrl to VBBOUND.C
//
SHORT  SSOcxDataCall( LONG lObject, LPVOID lpda, USHORT msg, USHORT action,
  SS_COORD row, SS_COORD col, LPSPREADSHEET lpSS);
LPVOID SSOcxVBDerefControl( LONG lObject);

void SSOcxFireDbEditError(LONG lObject, LONG lDbErrCode, 
  LPSS_CELLCOORD lpCell);
void SSOcxFireDataFill(LONG lObject, long Col, long Row, short DataType, 
  short fGetData, short FAR* lpCancel, LPSPREADSHEET lpSS);
void SSOcxFireDataColConfig(LONG lObject, long Col, LPCTSTR DataField, 
  short DataType, LPSPREADSHEET lpSS);

LPVOID  SSOcxLockVBSpread( LONG lObject);
VOID    SSOcxUnlockVBSpread( LONG lObject);

BOOL SSOcxSetTypePictPicture(LONG lObject, SS_COORD lCol, SS_COORD lRow,
  HBITMAP hBitmap, HPALETTE hPal, HICON hIcon);

HWND    SSOcxDBCreateHWnd(void *pObject);

LONG OnDataLoaded(LONG lObject);

// User-defined DATA messages (used with DataCall)
#define UDATA_DATAFIELD    200            // use with VBM_DATA_SET

#ifdef SS_V70
void SSOcxFreeSheetOleBinding(LPSPREADSHEET lpSS);
#endif

#ifdef __cplusplus
}
#endif

//-------------------------------------------------------------------------
// Functions specific to C++
//-------------------------------------------------------------------------
#ifdef __cplusplus
#if defined(SS_V70) && defined(FP_ADB)
void SSOcxInitSheetOleBinding(CSsocxCtrl FAR *lpSSOcx, LPSPREADSHEET lpSS);
#else
void    SSOcxInitOleBinding( CSsocxCtrl FAR *lpSSOcx);
#endif
LRESULT SSOcxBoundProc( LPVOID lpObject, UINT uMsg, WPARAM wParam, 
                        LPARAM lParam);

BOOL    SSOcxCreateVBSpread( LPTBGLOBALHANDLE lphVBSpread);
void    SSOcxDestroyVBSpread( TBGLOBALHANDLE hVBSpread);
void    SSOcxInitDB(TBGLOBALHANDLE hVBSpread);

#endif

#endif //not SSOCXDB_H
