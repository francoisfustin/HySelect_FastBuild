/***********
* SS_SPAN.H
***********/

#ifndef SS_SPAN_H
#define SS_SPAN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagSS_CELLSPAN
{
  SS_COORD lCol;
  SS_COORD lRow;
  SS_COORD lNumCols;
  SS_COORD lNumRows;
  TBGLOBALHANDLE hNext;
} SS_CELLSPAN, FAR *LPSS_CELLSPAN;

#define SS_SPANOVERLAP_NONE  0
#define SS_SPANOVERLAP_YES   1
#define SS_SPANOVERLAP_EQUAL 2

/******************************
* Declare function prototypes
******************************/

BOOL SS_AddCellSpan(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, SS_COORD lNumCols, SS_COORD lNumRows);
WORD SS_GetCellSpan(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPSS_COORD lplColAnchor,
                    LPSS_COORD lplRowAnchor, LPSS_COORD lplNumCols, LPSS_COORD lplNumRows);
WORD SS_IsSpanOverlap(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, SS_COORD lNumCols, SS_COORD lNumRows);
void SS_RemoveCellSpan(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow);
BOOL SS_DelCellSpanRowRange(LPSPREADSHEET lpSS, SS_COORD lRow, SS_COORD lNumRows);
void SS_InsCellSpanRowRange(LPSPREADSHEET lpSS, SS_COORD lRow, SS_COORD lNumRows);
BOOL SS_DelCellSpanColRange(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lNumCols, BOOL fInvalidate);
void SS_InsCellSpanColRange(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lNumCols);
BOOL SS_GetSpanLR(LPSPREADSHEET lpSS, LPSS_COORD lplCol, LPSS_COORD lplRow);
BOOL SS_GetSpanLREx(LPSPREADSHEET lpSS, LPSS_COORD lplCol, LPSS_COORD lplRow, SS_COORD lColLR, SS_COORD lRowLR);
BOOL SS_SpanCalcBlock(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPSS_SELBLOCK lpBlock);
BOOL SS_SpanMergeCalcBlock(LPSPREADSHEET lpSS, SS_COORD lRow, LPSS_ROW lpRow,
                           SS_COORD lCol, LPSS_COL lpCol, LPSS_SELBLOCK lpBlock);
void SS_CopySpan(LPSPREADSHEET lpSS, SS_COORD ColDest, SS_COORD RowDest, SS_COORD ColSrc,
                 SS_COORD RowSrc);
void SS_MoveSpan(LPSPREADSHEET lpSS, SS_COORD ColDest, SS_COORD RowDest, SS_COORD ColSrc,
                 SS_COORD RowSrc);
void SS_FreeSpanTable(LPSPREADSHEET lpSS);
BOOL SS_SpanIsCellAlreadyPainted(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow);
void SS_SpanFreePaintList(LPSPREADSHEET lpSS);
void SS_SpanInvalidateRange(LPSPREADSHEET lpSS, SS_COORD lCol1, SS_COORD lRow1, SS_COORD lCol2,
                            SS_COORD lRow2, BOOL fAnchorOnly);
BOOL SS_MergeCalcBlock(LPSPREADSHEET lpSS, SS_COORD lRow, LPSS_ROW lpRow,
                              SS_COORD lCol, LPSS_COL lpCol, LPSS_SELBLOCK lpBlock);
BOOL SS_AddBlockToPaintSpanList(LPSPREADSHEET lpSS, LPSS_SELBLOCK lpBlock);
void SS_MergeInvalidateRange(LPSPREADSHEET lpSS, SS_COORD lCol1, SS_COORD lRow1, SS_COORD lCol2,
                             SS_COORD lRow2);

#ifdef __cplusplus
}
#endif

#endif
