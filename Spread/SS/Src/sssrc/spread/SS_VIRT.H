/************
* SS_VIRT.H
************/

#ifndef SS_VIRT_H
#define SS_VIRT_H

#ifdef __cplusplus
extern "C" {
#endif

extern BOOL SS_VQueryData(LPSPREADSHEET lpSS, SS_COORD BottomVisCell);
extern void SS_VRefreshBuffer(LPSPREADSHEET lpSS, BOOL fTop);
extern BOOL SS_VClear(LPSPREADSHEET lpSS, SS_COORD Row1, SS_COORD Row2);

void SS_VGetBufferSize(LPSPREADSHEET lpSS, LPSS_COORD lpBufferSize,
                       LPSS_COORD lpOverlap);
void SS_VSetBufferSize(LPSPREADSHEET lpSS, SS_COORD BufferSize,
                       SS_COORD Overlap);

SS_COORD SS_VGetMax(LPSPREADSHEET lpSS);
SS_COORD SS_VSetMax(LPSPREADSHEET lpSS, SS_COORD Max);

SS_COORD SS_VGetPhysBufferSize(LPSPREADSHEET lpSS);
SS_COORD SS_VGetPhysBufferTop(LPSPREADSHEET lpSS);

LONG SS_VGetStyle(LPSPREADSHEET lpSS);
LONG SS_VSetStyle(LPSPREADSHEET lpSS, LONG lStyle);

BOOL SS_VScrollGetSpecial(LPSS_BOOK lpBook, LPWORD lpwOptions);
BOOL SS_VScrollSetSpecial(LPSS_BOOK lpBook, BOOL fUseSpecialVScroll, WORD wOptions);


#ifdef __cplusplus
}
#endif

#endif
