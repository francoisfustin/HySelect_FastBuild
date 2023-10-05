//---------*---------*---------*---------*---------*---------*---------*-----
//
// MODULE:		cal_vect.h
//
// PURPOSE:		Routines to handle sparse vectors.
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


#ifndef CAL_VECT_H
#define CAL_VECT_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The VECT_HANDLE data type represents a global memory handle used
//  by the sparse vector routines.
//

#if !defined(_WIN64) && !defined(_IA64) && !defined(SS_NO_USE_SH)
  #define VECT_USE_SMARTHEAP
#endif

#ifdef VECT_USE_SMARTHEAP
  #include "..\..\..\..\smrtheap\include\smrtheap.h"
  typedef MEM_HANDLE VECT_HANDLE, FAR* LPVECT_HANDLE;
#else
  typedef HGLOBAL VECT_HANDLE, FAR* LPVECT_HANDLE;
#endif

//--------------------------------------------------------------------
//
//  The VECTAPI type is used for all vector API routines.
//

#define VECTAPI

//--------------------------------------------------------------------
//
//  The VECT stucture represents a sparse vector.  The VECT struture
//  should be treated as a black box and only manipulated by means of
//  the functions listed below.
//

typedef struct tagVECT
{
  VECT_HANDLE hNode;  // handle to root
  long lIndex;        // smallest index tree can currently hold
  long lLeaves;       // total number leaves in tree
} /*VECT, FAR* LPVECT, BITVECT, FAR* LPBITVECT,*/
  VECTOR, FAR* LPVECTOR, BITVECTOR, FAR* LPBITVECTOR;

//--------------------------------------------------------------------
//
//  The VECT_ENUMPROC and BITVECT_ENUMPROC types represent call back
//  functions used for vector enumeration.
//
#if defined(_WIN64) || defined(_IA64)
typedef BOOL (CALLBACK* VECT_ENUMPROC)(long lIndex, VECT_HANDLE hValue,
                                       LONG_PTR lData);
typedef BOOL (CALLBACK* BITVECT_ENUMPROC)(long lIndex, LONG_PTR lData);
#else
typedef BOOL (CALLBACK* VECT_ENUMPROC)(long lIndex, VECT_HANDLE hValue,
                                       long lData);
typedef BOOL (CALLBACK* BITVECT_ENUMPROC)(long lIndex, long lData);
#endif

//--------------------------------------------------------------------
//
//  The following routines are used to initialize and free any memory
//  pools used by the vector routines.  These routines are only
//  needed when SmartHeap memory management is used.
//

void VECTAPI VectMemPoolInit();
void VECTAPI VectMemPoolFree();

//--------------------------------------------------------------------
//
//  The following routines are used to handle a sparse vector.
//

BOOL        VECTAPI VectInit(LPVECTOR lpVect);
BOOL        VECTAPI VectFree(LPVECTOR lpVect);
long        VECTAPI VectMinIndex(LPVECTOR lpVect);
long        VECTAPI VectMaxIndex(LPVECTOR lpVect);
VECT_HANDLE VECTAPI VectGet(LPVECTOR lpVect, long lIndex);
VECT_HANDLE VECTAPI VectGetFirst(LPVECTOR lpVect, LPLONG lplIndex);
VECT_HANDLE VECTAPI VectGetFirstIn(LPVECTOR lpVect, long lMin, long lMax,
                                   LPLONG lplIndex);
VECT_HANDLE VECTAPI VectGetNext(LPVECTOR lpVect, LPLONG lplIndex);
VECT_HANDLE VECTAPI VectGetNextIn(LPVECTOR lpVect, long lMin, long lMax,
                                  LPLONG lplIndex);
VECT_HANDLE VECTAPI VectSet(LPVECTOR lpVect, long lIndex, VECT_HANDLE hValue);
VECT_HANDLE VECTAPI VectRemove(LPVECTOR lpVect, long lIndex);
VECT_HANDLE VECTAPI VectRemoveFirst(LPVECTOR lpVect, LPLONG lplIndex);
BOOL        VECTAPI VectRemoveAll(LPVECTOR lpVect);
#if defined(_WIN64) || defined(_IA64)
BOOL        VECTAPI VectEnum(LPVECTOR lpVect, VECT_ENUMPROC lpProc, LONG_PTR lData);
#else
BOOL        VECTAPI VectEnum(LPVECTOR lpVect, VECT_ENUMPROC lpProc, long lData);
#endif

//--------------------------------------------------------------------
//
//  The following routines are used to handle a sparse bit vector.
//

BOOL VECTAPI BitVectInit(LPBITVECTOR lpVect);
BOOL VECTAPI BitVectFree(LPBITVECTOR lpVect);
BOOL VECTAPI BitVectGet(LPBITVECTOR lpVect, long lIndex);
BOOL VECTAPI BitVectGetFirst(LPBITVECTOR lpVect, LPLONG lplIndex);
BOOL VECTAPI BitVectGetNext(LPBITVECTOR lpVect, LPLONG lplIndex);
BOOL VECTAPI BitVectSet(LPBITVECTOR lpVect, long lIndex, BOOL bValue);
BOOL VECTAPI BitVectRemove(LPBITVECTOR lpVect, long lIndex);
BOOL VECTAPI BitVectRemoveFirst(LPBITVECTOR lpVect, LPLONG lplIndex);
BOOL VECTAPI BitVectRemoveAll(LPBITVECTOR lpVect);
BOOL VECTAPI BitVectEnum(LPBITVECTOR lpVect, BITVECT_ENUMPROC lpProc,
                         long lData);
BOOL VECTAPI BitVectCopy(LPBITVECTOR lpSrc, LPBITVECTOR lpDest);

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
