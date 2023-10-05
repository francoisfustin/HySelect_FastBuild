//---------*---------*---------*---------*---------*---------*---------*-----
//
// MODULE:		cal_matr.h
//
// PURPOSE:		Routines to handle sparse matrices.
//
// COPYRIGHT:   Copyright (c)1995 - FarPoint Technologies and Bunka Orient Co. 
//              All rights reserved.
//
//				No part of this source code may be copied, modified or 
//				reproduced in any form without retaining the above copyright 
//				notice.  This source code, or source code derived from it, 
//				may not be redistributed without express written permission 
//				of FarPoint Technologies.
//
// UPDATE LOG:
//
//		<flag>	<fixed date>	<bug #> - Description
//
//---------*---------*---------*---------*---------*---------*---------*-----


#ifndef CAL_MATR_H
#define CAL_MATR_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The MAT_HANDLE data type represents a global memory handle
//  used by the sparse matrix routines.
//

#if !defined(_WIN64) && !defined(_IA64) && !defined(SS_NO_USE_SH)
  #define MAT_USE_SMARTHEAP
#endif

#ifdef MAT_USE_SMARTHEAP
  #include "..\..\..\..\smrtheap\include\smrtheap.h"
  typedef MEM_HANDLE MAT_HANDLE, FAR* LPMAT_HANDLE;
#else
  typedef HGLOBAL MAT_HANDLE, FAR* LPMAT_HANDLE;
#endif

//--------------------------------------------------------------------
//
//  The MATAPI type is used for all matrix API routines.
//

#define MATAPI

//--------------------------------------------------------------------
//
//  The MATRIX stucture represents a sparse matrix.  The MATRIX
//  struture should be treated as a black box and only manipulated by
//  means of the functions listed below.
//

typedef struct tagMATRIX
{
  MAT_HANDLE hNode;  // handle to root
  long lCol;         // smallest column currently in matrix
  long lRow;         // smallest row currently in matrix
  long lColLeaves;   // total number of columns in matrix
  long lRowLeaves;   // total number of rows in matrix
} MATRIX, FAR *LPMATRIX, BITMATRIX, FAR* LPBITMATRIX;

//--------------------------------------------------------------------
//
//  The MAT_ENUMPROC and BITMAT_ENUMPROC types represent call back
//  functions used for matrix enumeration.
//
#if defined(_WIN64) || defined(_IA64)
typedef BOOL (CALLBACK* MAT_ENUMPROC)(long lCol, long lRow,
                                      MAT_HANDLE hValue, LONG_PTR lData);
typedef BOOL (CALLBACK* BITMAT_ENUMPROC)(long lCol, long lRow, LONG_PTR lData);
#else
typedef BOOL (CALLBACK* MAT_ENUMPROC)(long lCol, long lRow,
                                      MAT_HANDLE hValue, long lData);
typedef BOOL (CALLBACK* BITMAT_ENUMPROC)(long lCol, long lRow, long lData);
#endif
//--------------------------------------------------------------------
//
//  The following routines are used to initialize and free any memory
//  pools used by the matrix routines.  These routines are only
//  needed when SmartHeap memory management is used.
//

void MATAPI MatMemPoolInit();
void MATAPI MatMemPoolFree();

//--------------------------------------------------------------------
//
//  The following routines are used to handle a sparse matrix.
//

BOOL       MATAPI MatInit(LPMATRIX lpMatrix);
BOOL       MATAPI MatFree(LPMATRIX lpMatrix);
long       MATAPI MatMinCol(LPMATRIX lpMatrix);
long       MATAPI MatMaxCol(LPMATRIX lpMatrix);
long       MATAPI MatMinRow(LPMATRIX lpMatrix);
long       MATAPI MatMaxRow(LPMATRIX lpMatrix);
MAT_HANDLE MATAPI MatGet(LPMATRIX lpMatrix, long lCol, long lRow);
MAT_HANDLE MATAPI MatGetFirst(LPMATRIX lpMatrix, LPLONG lplCol, LPLONG lplRow);
MAT_HANDLE MATAPI MatGetNext(LPMATRIX lpMatrix, LPLONG lplCol, LPLONG lplRow);
MAT_HANDLE MATAPI MatSet(LPMATRIX lpMatrix,long lCol,long lRow,MAT_HANDLE hValue);
MAT_HANDLE MATAPI MatRemove(LPMATRIX lpMatrix, long lCol, long lRow);
MAT_HANDLE MATAPI MatRemoveFirst(LPMATRIX lpMatrix, LPLONG lplCol, LPLONG lplRow);
BOOL       MATAPI MatRemoveAll(LPMATRIX lpMatrix);
#if defined(_WIN64) || defined(_IA64)
BOOL       MATAPI MatEnum(LPMATRIX lpMatrix, MAT_ENUMPROC lpProc, LONG_PTR lData);
#else
BOOL       MATAPI MatEnum(LPMATRIX lpMatrix, MAT_ENUMPROC lpProc, long lData);
#endif

//--------------------------------------------------------------------
//
//  The following routines are used to handle a sparse bit vector.
//

BOOL MATAPI BitMatInit(LPBITMATRIX lpMatrix);
BOOL MATAPI BitMatFree(LPBITMATRIX lpMatrix);
BOOL MATAPI BitMatGet(LPBITMATRIX lpMatrix, long lCol, long lRow);
BOOL MATAPI BitMatGetFirst(LPBITMATRIX lpMatrix, LPLONG lplCol, LPLONG lplRow);
BOOL MATAPI BitMatGetNext(LPBITMATRIX lpMatrix, LPLONG lplCol, LPLONG lplRow);
BOOL MATAPI BitMatSet(LPBITMATRIX lpMatrix, long lCol, long lRow, BOOL bValue);
BOOL MATAPI BitMatRemove(LPBITMATRIX lpMatrix, long lCol, long lRow);
BOOL MATAPI BitMatRemoveFirst(LPBITMATRIX lpMatrix, LPLONG lplCol, LPLONG lplRow);
BOOL MATAPI BitMatRemoveAll(LPBITMATRIX lpMatrix);
BOOL MATAPI BitMatEnum(LPBITMATRIX lpMatrix, BITMAT_ENUMPROC lpProc, long lData);
BOOL MATAPI BitMatCopy(LPBITMATRIX lpSrc, LPBITMATRIX lpDest);
BOOL MATAPI BitMatCopyOffset(LPBITMATRIX lpSrc, LPBITMATRIX lpDest,
                             long lColOffset, long lRowOffset,
                             long lColMin, long lRowMin);
BOOL MATAPI BitMatHasInter(LPBITMATRIX lpMat1, LPBITMATRIX lpMat2);
BOOL MATAPI BitMatHasInterOffset(LPBITMATRIX lpMat1, LPBITMATRIX lpMat2,
                                  long lColOffset, long lRowOffset);

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
