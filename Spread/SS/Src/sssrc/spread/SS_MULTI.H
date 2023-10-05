/************
* SS_MULTI.H
************/

#ifndef SS_MULTI_H
#define SS_MULTI_H

#ifdef __cplusplus
extern "C" {
#endif

//#ifdef SS_V40
#define SS_IsF8ExtSelMode(lpSS) ((lpSS->wOpMode == SS_OPMODE_EXTSEL && lpSS->nExtSelF8Mode) ? TRUE : FALSE)
//#else
//#define SS_IsF8ExtSelMode(lpSS) FALSE
//#endif // SS_V40

#define SS_F8EXTSEL_TIMER_ID				147		// id for timer used to flash focus rect for special Shift-F8 extended select keyboard interface
#define SS_F8EXTSEL_TIMER_DELAY			500		

#if defined(_WIN64) || defined(_IA64)
long SS_SelModeSendMessage(LPSPREADSHEET lpSS, UINT uMsg, long lParam1,
                           LONG_PTR lParam2, long lParam3);
#else
long SS_SelModeSendMessage(LPSPREADSHEET lpSS, UINT uMsg, long lParam1,
                           long lParam2, long lParam3);
#endif
GLOBALHANDLE SS_GetMultiSelBlocks(LPSPREADSHEET lpSS, LPSHORT lpdSelBlockCnt);

WORD SS_GetSelBlockOptions(LPSS_BOOK lpBook);
WORD SS_SetSelBlockOptions(LPSS_BOOK lpBook, WORD wOption);

BOOL SS_IsCellInSelection(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);

void SS_KillExtSelF8Mode(LPSPREADSHEET lpSS);

#ifdef __cplusplus
}
#endif

#endif
