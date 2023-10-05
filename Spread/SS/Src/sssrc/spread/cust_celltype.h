//--------------------------------------------------------------------
//
//  File: cust_celltype.h
//
//  Description: Routines to handle custom cell type registration
//

#ifndef CUST_CELLTYPE_H
#define CUST_CELLTYPE_H

#ifdef SS_V80

#include "ssdll.h"
#include "ct_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The following diagram shows an example of a custom cell type table
//  that contains "richedit" custom celltype.
//
//  +-----------+     +--------+--------+--------+---
//  | hElem     |---->| handle | handle | handle | ...
//  | nElemCnt  |     +--------+--------+--------+---
//  | nAllocCnt |       |
//  | nAllocInc |       |
//  +-----------+       |
//                      |     +-----------------+     +------------+
//                      +---->| hText           |---->| "richedit" |
//                            | Procs           |     +------------+
//                            | bEditable       |
//                            | bCanOverflow    |
//                            | bCanBeOverflown |
//                            | lRefCnt = 1     |
//                            +-----------------+
//

//--------------------------------------------------------------------
//
//  The SS_CUSTOMCELLTYPE structure represents a custom celltype.
//

typedef struct ss_customcelltype
   {
   CT_HANDLE hText;		// handle to name of custom function
   long lRefCnt;		// number of references to custom celltype
   BOOL	bEditable;		// whether the celltype is editable
   BOOL	bCanOverflow;	// whether the celltype can overflow
   BOOL	bCanBeOverflown;// whether the celltype can be overflown
   BOOL  bUseRendererControl; // whether the celltype uses a renderer control
   SS_CT_PROCS Procs;	// function pointers to callbacks
   } SS_CUSTOMCELLTYPE, FAR *LPSS_CUSTOMCELLTYPE;

//--------------------------------------------------------------------
//
//  The SS_CUSTCELLTYPETABLE structure represents a table of custom
//  celltypes.
//

typedef struct ss_cttable
{
  CT_HANDLE hElem;	// handle to an array of SS_CUSTOMCELLTYPE
  int nElemCnt;		// number of cell types in array
  int nAllocCnt;	// number array slots which have been allocated
  int nAllocInc;	// number of additional array slots on each ReAlloc
} SS_CT_TABLE, FAR* LPSS_CT_TABLE;

//--------------------------------------------------------------------
//
//  The following functions are used to handle a table of custom
//  celltypes.
//

BOOL CustCellTypeInit(LPSS_CT_TABLE lpTable);
BOOL CustCellTypeFree(LPSS_CT_TABLE lpTable);
BOOL CustCellTypeAdd(LPSS_CT_TABLE lpTable, CT_HANDLE hFunc);
BOOL CustCellTypeRemove(LPSS_CT_TABLE lpTable, CT_HANDLE hFunc);
CT_HANDLE CustCellTypeLookup(LPSS_CT_TABLE lpTable, LPCTSTR lpszName);
CT_HANDLE CustCellTypeFirst(LPSS_CT_TABLE lpTable);
CT_HANDLE CustCellTypeNext(LPSS_CT_TABLE lpTable, LPCTSTR lpszName);

//--------------------------------------------------------------------
//
//  The following functions are used to handle an individual custom
//  celltype.
//

CT_HANDLE CustCellTypeCreate(LPCTSTR lpszName, BOOL bEditable, BOOL bCanOverflow, BOOL bCanBeOverflown, BOOL bUseRendererControl, LPSS_CT_PROCS lpProcs);
CT_HANDLE CustCellTypeCreateRef(CT_HANDLE hCT);
BOOL CustCellTypeDestroy(CT_HANDLE hCT);
long CustCellTypeGetRefCnt(CT_HANDLE hCT);
int  CustCellTypeGetText(CT_HANDLE hCT, LPTSTR lpszText, int nLen);
int  CustCellTypeGetTextLen(CT_HANDLE hCT);
LPCTSTR CustCellTypeLockText(CT_HANDLE hCT);
void CustCellTypeUnlockText(CT_HANDLE hCT);
BOOL CustCellTypeGetEditable(CT_HANDLE hCT);
BOOL CustCellTypeSetEditable(CT_HANDLE hCT, BOOL bEditable);
BOOL CustCellTypeGetCanOverflow(CT_HANDLE hCT);
BOOL CustCellTypeSetCanOverflow(CT_HANDLE hCT, BOOL bCanOverflow);
BOOL CustCellTypeGetCanBeOverflown(CT_HANDLE hCT);
BOOL CustCellTypeSetCanBeOverflown(CT_HANDLE hCT, BOOL bCanBeOverflown);
BOOL CustCellTypeGetUseRendererControl(CT_HANDLE hCT);
BOOL CustCellTypeSetUseRendererControl(CT_HANDLE hCT, BOOL bUseRendererControl);
BOOL CustCellTypeGetProcs(CT_HANDLE hCT, LPSS_CT_PROCS lpProcs);
BOOL CustCellTypeSetProcs(CT_HANDLE hCT, LPSS_CT_PROCS lpProcs);
BOOL CustCellTypeHasProcs(CT_HANDLE hCT);
BOOL CustCellTypeGetStruct(CT_HANDLE hCT, LPSS_CUSTOMCELLTYPE lpCT);

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // SS_V80

#endif
