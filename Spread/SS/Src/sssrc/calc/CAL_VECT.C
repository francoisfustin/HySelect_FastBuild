//--------------------------------------------------------------------
//
//  File: cal_vect.c
//
//  Description: Routines to handle sparse vectors.
//

#include <memory.h>
#include <windows.h>
#include "cal_vect.h"

//--------------------------------------------------------------------
//
//  The function _fmemset() is only defined in 16-bit windows.
//

#ifdef WIN32
  #define _fmemset memset
#endif

//--------------------------------------------------------------------
//
//  A sparse vector is implemented as a tree.  Each node in the tree
//  has a fixed number of branches, which is determined by the
//  BRANCHES constant.  Each non-zero element in the vector is stored
//  at a leaf in the tree.  All leaves of a given tree are at the same
//  depth.  The lIndex field of the VECT structure determines the
//  smallest index which the tree currently spans.  The lLeaves
//  field of the VECT structure determines the total number of leaves
//  the tree currently spans.  In other words, all non-zero elements
//  have an index i such that lIndex <= i < lIndex + lLeaves.  Since
//  each node has a fixed number of branches, the lLeaves field can
//  also be used to determine the height of the tree.  The index
//  of an element can be determined by following the branches from
//  the root to the leaf.  The height of the tree can grow or shrink
//  as elements are added to or removed from the sparse vector.  It
//  is assumed that all non NULL handles in the tree will lead to a
//  leaf.  Therefore it is important that nodes are trimmed from the
//  tree when no longer needed.
//
//  The following is a diagram of a sparse vector which has 4 branches
//  per node and contains items stored at locations 0, 2, and 13.  The
//  "X"'s represent valid handles.  The "0"'s represent NULL handles.
// 
//    +--------------+
//    | hNode        |-----+
//    | lIndex = 0   |     |
//    | lLeaves = 16 |     |
//    +--------------+     |
//                         \/
//                 +---------------+
//                 | X | 0 | 0 | X |
//                 +---------------+
//                  /             \ 
//                 /               \ 
//     +---------------+      +---------------+
//     | X | 0 | X | 0 |      | 0 | X | 0 | 0 |
//     +---------------+      +---------------+
//      /         \                   \ 
//     /           \                   \ 
//  +--------+  +--------+           +---------+
//  | Item 0 |  | Item 2 |           | Item 13 |
//  +--------+  +--------+           +---------+
//
//  The following is a diagram of a sparse vector which has 4 branches
//  per node and contains a single item stored at location 10.
// 
//    +-------------+
//    | hNode       |-----+
//    | lIndex = 10 |     |
//    | lLeaves = 1 |     |
//    +-------------+     |
//                        \/
//                    +---------+
//                    | Item 10 |
//                    +---------+
//

//--------------------------------------------------------------------
//
//  The BRANCHES constant determines the number of branches at each
//  node in a sparse vector.  The MAXLEAVES constant determines the
//  largest index that a vector can hold.  The MAXFULLLEAVES is the
//  largest power of BRANCHES which is less than or equal to
//  MAXFULLLEAVES.
//

//#define BRANCHES               4
//#define MAXFULLLEAVES 1073741824
//#define BRANCHES               8
//#define MAXFULLLEAVES 1073741824
//#define BRANCHES              16
//#define MAXFULLLEAVES  268435456
#define BRANCHES              32
#define MAXFULLLEAVES 1073741824
#define MAXLEAVES     2000000000

//--------------------------------------------------------------------
//
//  The EXPANDLEAVES()and SHRINKLEAVES() macros determine the number
//  of leaves at a higher or lower level in the tree, respectively.
//

#define EXPANDLEAVES(lLeaves) \
    (lLeaves < MAXFULLLEAVES ? lLeaves * BRANCHES : MAXLEAVES)
#define SHRINKLEAVES(lLeaves) \
    (lLeaves <= MAXFULLLEAVES ? lLeaves / BRANCHES : MAXFULLLEAVES)

//--------------------------------------------------------------------
//
//  The BITS_PER_VECT_HANDLE constant determines the number of bits
//  in a vector handle.
//

#define BITS_PER_VECT_HANDLE (8 * sizeof(VECT_HANDLE))

//--------------------------------------------------------------------
//
//  The following macros define the memory allocation scheme used by
//  the sparse vector routines.
//

#ifdef VECT_USE_SMARTHEAP
  static MEM_POOL vectMemPool = 0;
  #define VectMemAlloc() MemAllocFS(vectMemPool)
  #define VectMemLock(hMem) ((LPVECT_HANDLE)hMem)
  #define VectMemUnlock(hMem)
  #define VectMemFree(hMem) MemFreeFS(hMem)
#else
  #define VectMemAlloc() \
     GlobalAlloc(GMEM_MOVEABLE,BRANCHES*sizeof(VECT_HANDLE))
  #define VectMemLock(hMem) ((LPVECT_HANDLE)GlobalLock(hMem))
  #define VectMemUnlock(hMem) GlobalUnlock(hMem)
  #define VectMemFree(hMem) GlobalFree(hMem)
#endif

//--------------------------------------------------------------------
//
//  The VectMemPoolInit() function initializes a SmartHeap memory
//  pool which is used by the vector routines.
//
//  Note: If SmartHeap memory management is being used, then
//        VectMemPoolInit() must be called before any other vector
//        routine is used.
//

void VECTAPI VectMemPoolInit()
{
#ifdef VECT_USE_SMARTHEAP
  if( !vectMemPool )
    vectMemPool = MemPoolInitFS(BRANCHES * sizeof(VECT_HANDLE), 16, MEM_POOL_DEFAULT);
#endif
}

//--------------------------------------------------------------------
//
//  The VectMemPoolFree() function frees the SmartHeap memory pool
//  which is being used by the vector routines.
//
//  Note: If SmartHeap memory management is being used, then no
//        vector routine can be used after a call to VectMemPoolFree().
//

void VECTAPI VectMemPoolFree()
{
#ifdef VECT_USE_SMARTHEAP
  if( vectMemPool )
    MemPoolFree(vectMemPool);
  vectMemPool = 0;
#endif
}
    
//--------------------------------------------------------------------
//
//  The VectInit() function initializes the sparse vector structure.
//

BOOL VECTAPI VectInit(LPVECTOR lpVect)
{
  lpVect->hNode = NULL;
  lpVect->lIndex = 0;
  lpVect->lLeaves = 1;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The VectFree() function frees all memory associated with the
//  sparse vector.
//

BOOL VECTAPI VectFree(LPVECTOR lpVect)
{
  VectRemoveAll(lpVect);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The VectMaxIndexEx() function returns the index of the last item
//  in the sub-vector below the given node.
//

static long VectMaxIndexEx(VECT_HANDLE hNode, long lLeaves)
{
  VECT_HANDLE hTemp;
  LPVECT_HANDLE lpNode;
  int i;
  long lRet = 0;

  while( lLeaves > 1 && hNode && (lpNode = VectMemLock(hNode)) )
  {
    hTemp = NULL;
    for( i = BRANCHES - 1; i >= 0 && NULL == hTemp; i-- )
    {
      if( lpNode[i] )
      {
        hTemp = lpNode[i];
        lLeaves = SHRINKLEAVES(lLeaves);
        lRet = lRet * BRANCHES + i;
      }
    }
    VectMemUnlock(hNode);
    hNode = hTemp;
  }
  return lRet;
}

//--------------------------------------------------------------------
//
//  The VectMinIndex() function returns the index of the first item
//  in the vector.
//

long VECTAPI VectMinIndex(LPVECTOR lpVect)
{
  return lpVect->lIndex;
}

//--------------------------------------------------------------------
//
//  The VectMaxIndex() function returns the index of the last item
//  in the vector.
//

long VECTAPI VectMaxIndex(LPVECTOR lpVect)
{
  //return lpVect->lIndex + lpVect->lLeaves - 1;
  return lpVect->lIndex + VectMaxIndexEx(lpVect->hNode,lpVect->lLeaves);
}

//--------------------------------------------------------------------
//
//  The VectGet() function retrieves an element from the sparse vector
//  given its index.  If the element is found, the function returns
//  the handle of the element.  Otherwise, the function returns NULL.
//

VECT_HANDLE VECTAPI VectGet(LPVECTOR lpVect, long lIndex)
{
  VECT_HANDLE hReturn = NULL;
  VECT_HANDLE hTemp;
  VECT_HANDLE hNode = lpVect->hNode;
  LPVECT_HANDLE lpNode;
  long lLeaves = lpVect->lLeaves;

  lIndex -= lpVect->lIndex;
  if( 0 <= lIndex && lIndex < lLeaves )
  {
    while( lLeaves > 1 && hNode && (lpNode = VectMemLock(hNode)) )
    {
      lLeaves = SHRINKLEAVES(lLeaves);
      hTemp = lpNode[lIndex / lLeaves];
      lIndex %= lLeaves;
      VectMemUnlock(hNode);
      hNode = hTemp;
    }
    if( lLeaves == 1 )
      hReturn = hNode;
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The VectExpand() function expands the size of the tree until it
//  contains the given index.
//

void VectExpand(LPVECTOR lpVect, long lIndex)
{
  LPVECT_HANDLE lpNode;
  VECT_HANDLE hTemp;
  
  while( lIndex < lpVect->lIndex
         || lpVect->lIndex + lpVect->lLeaves <= lIndex )
  {
    if( lpVect->hNode )
    {
      if( (hTemp = VectMemAlloc()) && (lpNode = VectMemLock(hTemp)) )
      {
        _fmemset(lpNode, 0, BRANCHES * sizeof(VECT_HANDLE));
        lpNode[(lpVect->lIndex/lpVect->lLeaves) % BRANCHES] = lpVect->hNode;
        VectMemUnlock(hTemp);
        lpVect->hNode = hTemp;
      }
      else
        lpVect->hNode = NULL;
    }
    lpVect->lLeaves = EXPANDLEAVES(lpVect->lLeaves);
    lpVect->lIndex -= lpVect->lIndex % lpVect->lLeaves;
  }
}
//--------------------------------------------------------------------
//
//  The VectSetEx() function places an element handle into the sparse
//  matrix.
//
//  Note: It is assumed that hValue != 0 and 0 <= lIndex < lLeaves.
//
#if defined(_WIN64) || defined(_IA64)
static VECT_HANDLE VectSetEx(LPVECT_HANDLE lphNode, long lLeaves,
                             long lIndex, VECT_HANDLE hValue, LONG_PTR lSetBit)
#else
static VECT_HANDLE VectSetEx(LPVECT_HANDLE lphNode, long lLeaves,
                             long lIndex, VECT_HANDLE hValue, long lSetBit)
#endif
{
  VECT_HANDLE hReturn = NULL;
  VECT_HANDLE hTemp = NULL;
  VECT_HANDLE hNode = NULL;
  LPVECT_HANDLE lpNode;

  // scan down tree, creating branches if needed
  hTemp = NULL;
  while( lLeaves > 1 && lphNode )
  {
    if( *lphNode )
      lpNode = VectMemLock(*lphNode);
    else
    {
      *lphNode = VectMemAlloc();
      if( lpNode = VectMemLock(*lphNode) )
        _fmemset(lpNode, 0, BRANCHES*sizeof(VECT_HANDLE));
    }
    if( *lphNode )
    {
      hNode = *lphNode;
      if( hTemp )
        VectMemUnlock( hTemp );
      hTemp = hNode;
      if( lpNode )
      {
        lLeaves = SHRINKLEAVES(lLeaves);
        lphNode = &lpNode[lIndex / lLeaves];
        lIndex %= lLeaves;
      }
      else
        lphNode = NULL; // VectMemLock() failed, break loop
    }
    else
      lphNode = NULL; // VectMemAlloc failed, break loop
  }
  // finally at leaf, can store old value and set new value
  if( lphNode )
  {
    if( lSetBit )
    {
#if defined(_WIN64) || defined(_IA64)
      hReturn = (VECT_HANDLE)((LONG_PTR)*lphNode & lSetBit);
      *lphNode = (VECT_HANDLE)((LONG_PTR)*lphNode | lSetBit);
#else
      hReturn = (VECT_HANDLE)((long)*lphNode & lSetBit);
      *lphNode = (VECT_HANDLE)((long)*lphNode | lSetBit);
#endif
    }
    else
    {
      hReturn = *lphNode;
      *lphNode = hValue;
    }
  }
  if( hTemp )
    VectMemUnlock(hTemp);
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The VectSet() function places an element handle into the sparse
//  vector.  If the new value is NULL then the element is actually
//  remove from the sparse vector.  If needed, the size of the vector
//  is expanded.  The function returns the previous value which was
//  stored in the vector at the given index.
//      

VECT_HANDLE VECTAPI VectSet(LPVECTOR lpVect, long lIndex, VECT_HANDLE hValue)
{
  VECT_HANDLE hReturn = NULL;

  if( 0 <= lIndex && lIndex <= MAXLEAVES )
  {  
    if( !hValue )
      hReturn = VectRemove(lpVect, lIndex);
    else if( !lpVect->hNode )
    {
      lpVect->hNode = hValue;
      lpVect->lIndex = lIndex;
      lpVect->lLeaves = 1;
    }
    else
    {
      // expand height of tree if needed
      VectExpand(lpVect, lIndex);
      lIndex -= lpVect->lIndex;
      hReturn = VectSetEx(&lpVect->hNode, lpVect->lLeaves, lIndex,
                          hValue, 0);
    }
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The VectRemoveEx() function removes an element from the sparse vector
//  which is stored at or below the given node.  Nodes with no more
//  branches are also removed.  The function returns the previous
//  value of the element an the given index.
//

static VECT_HANDLE VectRemoveEx(LPVECT_HANDLE lphNode, long lLeaves,
                                long lIndex, long lRemoveBit)
{
  VECT_HANDLE hReturn = NULL;
  LPVECT_HANDLE lpNode;
  BOOL bFree;
  int i;
  
  if( lLeaves > 1 )
  {
    if( *lphNode && (lpNode = VectMemLock(*lphNode)) )
    {
      lLeaves = SHRINKLEAVES(lLeaves);
      i = (int)(lIndex / lLeaves);
      hReturn = VectRemoveEx(lpNode+i, lLeaves, lIndex%lLeaves, lRemoveBit);
      if( bFree = !lpNode[i])
      {
        for( i = 0; i < BRANCHES; i++ )
          if( lpNode[i] )
          {
            bFree = FALSE;
            break;
          }
      }
      VectMemUnlock(*lphNode);
      if( bFree )
      {
        VectMemFree(*lphNode);
        *lphNode = NULL;
      }
    }
  }
  else
  {
    if( lRemoveBit )
    {
#if defined(_WIN64) || defined(_IA64)
      hReturn = (VECT_HANDLE)((LONG_PTR)*lphNode & lRemoveBit);
      *lphNode = (VECT_HANDLE)((LONG_PTR)*lphNode & ~lRemoveBit);
#else
      hReturn = (VECT_HANDLE)((long)*lphNode & lRemoveBit);
      *lphNode = (VECT_HANDLE)((long)*lphNode & ~lRemoveBit);
#endif
    }
    else
    {
      hReturn = *lphNode;
      *lphNode = NULL;
    }
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The VectShrink() function shrinks the tree to the minimum
//  height which still spans all of its elements.
//

static void VectShrink(LPVECTOR lpVect)
{
  VECT_HANDLE hNonZero;
  LPVECT_HANDLE lpNode;
  BOOL bShrink = TRUE;
  long lNonZero;
  int i;

  while( lpVect->lLeaves > 1 && lpVect->hNode && bShrink )
  {
    hNonZero = 0;
    if( lpNode = VectMemLock(lpVect->hNode) )
    {
      for( i = 0; !hNonZero && i < BRANCHES; i++ )
        if( lpNode[i] )
        {
            hNonZero = lpNode[i];
            lNonZero = i;
        }
      for( ; bShrink && i < BRANCHES; i++ )
        if( lpNode[i] )
          bShrink = FALSE;
      VectMemUnlock(lpVect->hNode);
    }
    else
      bShrink = FALSE;
    if( bShrink && hNonZero )
    {
      VectMemFree(lpVect->hNode);
      lpVect->hNode = hNonZero;
      lpVect->lLeaves = SHRINKLEAVES(lpVect->lLeaves);
      lpVect->lIndex += lNonZero * lpVect->lLeaves;
    }
  }
  if( !lpVect->hNode )
    VectInit(lpVect);
}

//--------------------------------------------------------------------
//
//  The VectRemove() function removes an element from the sparse
//  vector.  If needed, the height of the tree is shrunk.  The
//  function returns the previous value of the element.
//

VECT_HANDLE VECTAPI VectRemove(LPVECTOR lpVect, long lIndex)
{
  VECT_HANDLE hReturn = NULL;
  
  lIndex -= lpVect->lIndex;
  if( 0 <= lIndex && lIndex < lpVect->lLeaves )
  {
    hReturn = VectRemoveEx(&lpVect->hNode, lpVect->lLeaves, lIndex, 0);
    VectShrink(lpVect);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The VectRemoveFirstEx() function removes the first element from
//  the sparse vector which is stored at or below the given node.
//  Nodes with no more branhes are also removed.  The function
//  returns the previous value of the element an the given location.

static VECT_HANDLE VectRemoveFirstEx(LPVECT_HANDLE lphNode,
                                     long lLeaves, LPLONG lplIndex,
                                     LPINT lpiBit, BOOL bRemoveBit)
{
  VECT_HANDLE hReturn = NULL;
  LPVECT_HANDLE lpNode;
  BOOL bFree = TRUE;
  int i;
  
  if( lLeaves > 1 )
  {
    if( *lphNode && (lpNode = VectMemLock(*lphNode)) )
    {
      for( i = 0; i < BRANCHES; i++ )
      {
        if( lpNode[i] )
        {
          lLeaves = SHRINKLEAVES(lLeaves);
          hReturn = VectRemoveFirstEx(lpNode+i, lLeaves, lplIndex,
                                      lpiBit, bRemoveBit);
          if( hReturn )
          {
            *lplIndex += i * lLeaves;
            break;
          }
        }
      }
      for( ; i < BRANCHES; i++ )
      {
        if( lpNode[i] )
        {
          bFree = FALSE;
          break;
        }
      }
      VectMemUnlock(*lphNode);
      if( bFree )
      {
        VectMemFree(*lphNode);
        *lphNode = NULL;
      }
    }
  }
  else
  {
    if( bRemoveBit )
    {
      for( i = 0; !hReturn && i < BITS_PER_VECT_HANDLE; i++ )
      {
#if defined(_WIN64) || defined(_IA64)
        if( (LONG_PTR)*lphNode >> i & 1 )
#else
        if( (long)*lphNode >> i & 1 )
#endif
        {
          hReturn = (VECT_HANDLE)1;
#if defined(_WIN64) || defined(_IA64)
          *lphNode = (VECT_HANDLE)((LONG_PTR)*lphNode & ~(1L << i));
#else
          *lphNode = (VECT_HANDLE)((long)*lphNode & ~(1L << i));
#endif
          *lplIndex = 0;
          *lpiBit = i;
        }
      }
    }
    else
    {
      hReturn = *lphNode;
      *lphNode = NULL;
      *lplIndex = 0;
    }
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The VectRemoveFirst() function removes the first elelemnt from the
//  sparse vector.  If needed, the height of the tree is shrunk.  The
//  function returns the previous value of the element.
//

VECT_HANDLE VECTAPI VectRemoveFirst(LPVECTOR lpVect, LPLONG lplIndex)
{
  VECT_HANDLE hReturn;
  long lBaseIndex = lpVect->lIndex;
  
  hReturn = VectRemoveFirstEx(&lpVect->hNode, lpVect->lLeaves,
                              lplIndex, NULL, FALSE);
  if( hReturn )
  {
    *lplIndex += lBaseIndex;
    VectShrink(lpVect);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The VectRemoveAllEx() function removes all elements from the sparse
//  vector which are at or below the given node.
//
//  Note: The calling routine is responsible for setting hNode = NULL.
//

static BOOL VectRemoveAllEx(VECT_HANDLE hNode, long lLeaves)
{
  LPVECT_HANDLE lpNode;
  int i;
  
  if( lLeaves > 1 && hNode )
  {
    if( lpNode = VectMemLock(hNode) )
    {
      for( i = 0; i < BRANCHES; i++ )
        VectRemoveAllEx(lpNode[i], SHRINKLEAVES(lLeaves));
      VectMemUnlock(hNode);
    }
    VectMemFree(hNode);
  }
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The VectRemoveAll() function removes all items from the given
//  sparse vector.  The sparse vector is restored to its initial state.
//
//  Note: The calling routine is responsible for deleting all
//        memory associated with the values of the elements.
//

BOOL VECTAPI VectRemoveAll(LPVECTOR lpVect)
{
  VectRemoveAllEx(lpVect->hNode, lpVect->lLeaves);
  VectInit(lpVect);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The VectGetFirstEx() function retrieves the first element in the
//  sparse vector which is at or below the given node.  If found, the
//  function returns the handle of the element and *lplIndex contains
//  the index of the element.  Otherwise, the function returns NULL
//  and *lplIndex is undefined.
//

static VECT_HANDLE VectGetFirstEx(VECT_HANDLE hNode, long lLeaves,
                                  LPLONG lplIndex)
{
  VECT_HANDLE hTemp;
  LPVECT_HANDLE lpNode;
  VECT_HANDLE hReturn = NULL;
  int i;

  *lplIndex = 0;
  while( lLeaves > 1 && hNode && (lpNode = VectMemLock(hNode)) )
  {
    hTemp = NULL;
    for(i = 0; i < BRANCHES; i++)
      if( lpNode[i] )
      {
        hTemp = lpNode[i];
        lLeaves = SHRINKLEAVES(lLeaves);
        *lplIndex = *lplIndex * BRANCHES + i;
        break;
      }
    VectMemUnlock(hNode);
    hNode = hTemp;
  }
  if( lLeaves == 1 )
    hReturn = hNode;
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The GetFirstIn() function retrieves the first element in the sparse
//  vector which is at or below the given node.  If found, the
//  function returns the handle of the element and *lplIndex contains
//  the index of the element.  Otherwise, the function returns NULL
//  and *lplIndex is undefined.
//

static VECT_HANDLE GetFirstIn(VECT_HANDLE hNode, long lLeaves, long lMin, long lMax,
                        LPLONG lplIndex)
{
  LPVECT_HANDLE lpNode;
  VECT_HANDLE hReturn = NULL;
  int i, iMin, iMax;
  
  if( lLeaves > 1 && hNode && (lpNode = VectMemLock(hNode)) )
  {
    lLeaves = SHRINKLEAVES(lLeaves);
    iMin = (int)(lMin / lLeaves);
    iMax = (int)(lMax / lLeaves);
    for( i = iMin; !hReturn && i <= iMax; i++ )
    {
      if( lpNode[i] )
      {
        hReturn = GetFirstIn(
          lpNode[i],
          lLeaves,
          (i == iMin ? lMin%lLeaves : 0),
          (i == iMax ? lMax%lLeaves : lLeaves-1),
          lplIndex
        );
        if( hReturn )
          *lplIndex += i * lLeaves;
      }
    }
    VectMemUnlock(hNode);
  }
  else if( lLeaves == 1)
  {
    hReturn = hNode;
    *lplIndex = 0;
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The VectGetFirst() function retrieves the first element in the
//  sparse vector.  If found, the function returns the handle of the
//  element and *lplIndex contains the index of the element.
//  Otherwise, the function returns NULL and *lpIndex is undefined.
//

VECT_HANDLE VECTAPI VectGetFirst(LPVECTOR lpVect, LPLONG lplIndex)
{
  VECT_HANDLE hReturn;
  
  //*lplIndex = 0;
  hReturn = VectGetFirstEx(lpVect->hNode, lpVect->lLeaves, lplIndex);
  //hReturn = GetFirstIn(lpVect->hNode, lpVect->lLeaves, 0, lpVect->lLeaves-1, lplIndex);
  if( hReturn )
    *lplIndex += lpVect->lIndex;
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The VectGetFirstIn() function retrieves the first element in the
//  sparse vector.  If found, the function returns the handle of the
//  element and *lplIndex contains the index of the element.
//  Otherwise, the function returns NULL and *lpIndex is undefined.
//

VECT_HANDLE VECTAPI VectGetFirstIn(LPVECTOR lpVect, long lMin, long lMax,
                                   LPLONG lplIndex)
{
  VECT_HANDLE hReturn;
  
  *lplIndex = 0;
  lMin = max(0, lMin - lpVect->lIndex);
  lMax = min(lpVect->lLeaves-1, lMax - lpVect->lIndex);
  hReturn = GetFirstIn(lpVect->hNode, lpVect->lLeaves, lMin, lMax,
                       lplIndex);
  if( hReturn )
    *lplIndex += lpVect->lIndex;
  return hReturn;
}

//--------------------------------------------------------------------
//  The VectGetNextEx() function finds the next elelemnt which is at
//  of below the given node.  If an element is found,  the function
//  returns the handle of the element and the location of the element
//  is stored in *lplIndex.  Otherwise, the function returns NULL
//  and *lplIndex is undefined.
//

static VECT_HANDLE VectGetNextEx(VECT_HANDLE hNode, long lLeaves,
                                 LPLONG lplIndex)
{
  LPVECT_HANDLE lpNode;
  VECT_HANDLE hReturn = NULL;
  long lIndex = *lplIndex;
  long i;
  
  if( lLeaves > 1 && hNode && (lpNode = VectMemLock(hNode)) )
  {
    lLeaves = SHRINKLEAVES(lLeaves);
    lIndex %= lLeaves;
    i = *lplIndex / lLeaves;
    if( hReturn = VectGetNextEx(lpNode[i], lLeaves, &lIndex) )
      *lplIndex = *lplIndex/lLeaves * lLeaves + lIndex;
    else
    {
      for( i++; !hReturn && i < BRANCHES; i++ )
        if( lpNode[i] )
          if( hReturn = VectGetFirstEx(lpNode[i], lLeaves, &lIndex) )
            *lplIndex = i * lLeaves + lIndex;
    }
    VectMemUnlock(hNode);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The VectGetNext() function finds the next element in the sparse
//  vector.  If an element is found, the function returns the handle of
//  the element and *lplIndex contains the index of the element.
//  Otherwise, the function retruns NULL and *lplIndex is undefined.
//

VECT_HANDLE VECTAPI VectGetNext(LPVECTOR lpVect, LPLONG lplIndex)
{
  //VECT_HANDLE hNextNode = NULL;
  //long lNextIndex;
  //long lNextLeaves;
  //VECT_HANDLE hNode = lpVect->hNode;
  long lIndex = *lplIndex - lpVect->lIndex;
  //long lLeaves = lpVect->lLeaves;
  VECT_HANDLE hReturn = NULL;
  //VECT_HANDLE hTemp;
  //LPVECT_HANDLE lpNode;
  //long i;

  if( lIndex < 0 )
  {
    hReturn = VectGetFirstEx(lpVect->hNode,lpVect->lLeaves, &lIndex);
  } 
  else if( lIndex < lpVect->lLeaves )
  {
    hReturn = VectGetNextEx(lpVect->hNode, lpVect->lLeaves, &lIndex);
    // scan down tree searching for *lplIndex while keeping track of
    // next non empty branch
    /*
    while( lLeaves > 1 && hNode && (lpNode = VectMemLock(hNode)) )
    {
      lLeaves = SHRINKLEAVES(lLeaves);
      for( i = lIndex / lLeaves + 1; i < BRANCHES; i++ )
      {
        if( lpNode[i] )
        {
          hNextNode = lpNode[i];
          lNextIndex = (*lplIndex - lpVect->lIndex) / lLeaves;
          lNextIndex += i - lNextIndex % BRANCHES;
          lNextLeaves = lLeaves;
          break;
        }
      }
      hTemp = lpNode[lIndex / lLeaves];
      lIndex %= lLeaves;
      VectMemUnlock(hNode);
      hNode = hTemp;
    }
    //  scan down tree searching for next item
    if( hNextNode )
    {
      hReturn = GetFirst(hNextNode, lNextLeaves, lplIndex);
      *lplIndex += lNextIndex * lNextLeaves;
    }
    */ 
  }
  if( hReturn )
    *lplIndex = lIndex + lpVect->lIndex;
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The VectEnumEx() function enumrates the items which are at or
//  below the given node.
//
#if defined(_WIN64) || defined(_IA64)
static BOOL VectEnumEx(VECT_HANDLE hNode, long lLeaves, long lIndex,
                       VECT_ENUMPROC lpProc, LONG_PTR lData)
#else
static BOOL VectEnumEx(VECT_HANDLE hNode, long lLeaves, long lIndex,
                       VECT_ENUMPROC lpProc, long lData)
#endif
{
  LPVECT_HANDLE lpNode;
  BOOL bReturn = TRUE;
  int i;
  
  if( lLeaves == 1 )
  {
    if( hNode )
      bReturn = lpProc(lIndex, hNode, lData);
  }
  else if( lLeaves > 1 && (lpNode = VectMemLock(hNode)) )
  {
    lLeaves = SHRINKLEAVES(lLeaves);
    for( i = 0; bReturn && i < BRANCHES; i++, lIndex += lLeaves )
      bReturn = VectEnumEx(lpNode[i], lLeaves, lIndex, lpProc, lData);
    VectMemUnlock(hNode);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The VectEnum() function enumerates the items in the given vector.
//
#if defined(_WIN64) || defined(_IA64)
BOOL VECTAPI VectEnum(LPVECTOR lpVect, VECT_ENUMPROC lpProc, LONG_PTR lData)
#else
BOOL VECTAPI VectEnum(LPVECTOR lpVect, VECT_ENUMPROC lpProc, long lData)
#endif
{
  return VectEnumEx(lpVect->hNode, lpVect->lLeaves, lpVect->lIndex,
                    lpProc, lData);
}

//--------------------------------------------------------------------
//
//  The ulMoreBits[] array is used to determine if there are any
//  bits in the remaining portion of an unsigned long.
//

static unsigned long lMoreBits[] =
{
  0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFFFC, 0xFFFFFFF8,
  0xFFFFFFFF, 0xFFFFFFE0, 0xFFFFFFC0, 0xFFFFFF80,
  0xFFFFFFF0, 0xFFFFFE00, 0xFFFFFC00, 0xFFFFF800,
  0xFFFFFF00, 0xFFFFE000, 0xFFFFC000, 0xFFFF8000,
  0xFFFF0000, 0xFFFE0000, 0xFFFC0000, 0xFFF80000,
  0xFFF00000, 0xFFE00000, 0xFFC00000, 0xFF800000,
  0xFF000000, 0xFE000000, 0xFC000000, 0xF8000000,
  0xF0000000, 0xE0000000, 0xC0000000, 0x80000000,
  0x00000000
};

//--------------------------------------------------------------------
//
//  The following routines are used to handle a sparse bit vector.
//

#define ADJUSTINDEX(lIndex) (lIndex / BITS_PER_VECT_HANDLE)
#define TOBIT(lIndex) (lIndex % BITS_PER_VECT_HANDLE)
#define TOINDEX(lIndex,iBit) (lIndex * BITS_PER_VECT_HANDLE + iBit)

//--------------------------------------------------------------------
//
//  The BitVectInit() function initializes a sparse bit vector
//  structure.
//

BOOL VECTAPI BitVectInit(LPBITVECTOR lpVect)
{
  return VectInit(lpVect);
}

//--------------------------------------------------------------------
//
//  The BitVectFree() function frees all memory associated with
//  the sparse bit vector.
//

BOOL VECTAPI BitVectFree(LPBITVECTOR lpVect)
{
  return VectFree(lpVect);
}

//--------------------------------------------------------------------
//
//  The BitVectGet() function retrieves an element from a sparse
//  bit vector.
//

BOOL VECTAPI BitVectGet(LPBITVECTOR lpVect, long lIndex)
{
#if defined(_WIN64) || defined(_IA64)
  LONG_PTR lValue;
  
  lValue = (LONG_PTR)VectGet(lpVect, ADJUSTINDEX(lIndex));
#else
  long lValue;
  
  lValue = (long)VectGet(lpVect, ADJUSTINDEX(lIndex));
#endif
  return (BOOL)(lValue >> TOBIT(lIndex) & 1);
}

//--------------------------------------------------------------------
//
//  The BitMatGetFirst() function retrives the first elelemnt from a
//  sparse bit vector.
//

BOOL VECTAPI BitVectGetFirst(LPBITVECTOR lpVect, LPLONG lplIndex)
{
#if defined(_WIN64) || defined(_IA64)
  LONG_PTR lValue;
#else
  long lValue;
#endif
  long lIndex;
  long i;
  
#if defined(_WIN64) || defined(_IA64)
  if( lValue = (LONG_PTR)VectGetFirst(lpVect, &lIndex) )
#else
  if( lValue = (long)VectGetFirst(lpVect, &lIndex) )
#endif
  {
    for( i = 0; i < BITS_PER_VECT_HANDLE; i++ )
      if( lValue >> i & 1 )
      {
        *lplIndex = TOINDEX(lIndex,i);
        return TRUE;
      }
  }
  return FALSE;
}

//--------------------------------------------------------------------
//
//  The BitVectGetNext() function retrieves the next element from a
//  sparse bit vector.
//

BOOL VECTAPI BitVectGetNext(LPBITVECTOR lpVect, LPLONG lplIndex)
{ 
#if defined(_WIN64) || defined(_IA64)
  LONG_PTR lValue;
#else
  long lValue;
#endif
  long lIndex = ADJUSTINDEX(*lplIndex);
  long i;
  
#if defined(_WIN64) || defined(_IA64)
  lValue = (LONG_PTR)VectGet(lpVect, lIndex);
#else
  lValue = (long)VectGet(lpVect, lIndex);
#endif
  i = TOBIT(*lplIndex) + 1;
  if( lValue & lMoreBits[i] )
  {
    for( ; i < BITS_PER_VECT_HANDLE; i++ )
      if( lValue >> i & 1 )
      {
        *lplIndex = TOINDEX(lIndex,i);
        return TRUE;
      }
  }
#if defined(_WIN64) || defined(_IA64)
  if( lValue = (LONG_PTR)VectGetNext(lpVect, &lIndex) )
#else
  if( lValue = (long)VectGetNext(lpVect, &lIndex) )
#endif
  {
    for( i = 0; i < BITS_PER_VECT_HANDLE; i++ )
      if( lValue >> i & 1 )
      {
        *lplIndex = TOINDEX(lIndex,i);
        return TRUE;
      }
  }
  return FALSE;
}

//--------------------------------------------------------------------
//
//  The BitVectSet() function sets (or clears) an element in the
//  sparse bit matrix.
//

BOOL VECTAPI BitVectSet(LPBITVECTOR lpVect, long lIndex, BOOL bValue)
{
  BOOL bReturn = FALSE;
  long lAdjustIndex = ADJUSTINDEX(lIndex);
#if defined(_WIN64) || defined(_IA64)
  LONG_PTR lSetBit = 1L << TOBIT(lIndex);
#else
  long lSetBit = 1L << TOBIT(lIndex);
#endif

  if( !bValue )
    bReturn = BitVectRemove(lpVect, lIndex);
  else if( !lpVect->hNode )
  {
    lpVect->hNode = (VECT_HANDLE)lSetBit;
    lpVect->lIndex = lAdjustIndex;
    lpVect->lLeaves = 1;
  }
  else
  {
    VectExpand(lpVect, lAdjustIndex);
    lAdjustIndex -= lpVect->lIndex;
    bReturn = 0 != VectSetEx(&lpVect->hNode, lpVect->lLeaves,
                             lAdjustIndex, 0, lSetBit);
  }
  return bReturn;

  /*
  long lValue;
  BOOL bReturn;
  
  lValue = (long)VectGet(lpVect, ADJUSTINDEX(lIndex));
  bReturn = (BOOL)(lValue >> TOBIT(lIndex) & 1);
  if( bValue )
    lValue |= 1L << TOBIT(lIndex);
  else
    lValue &= ~(1L << TOBIT(lIndex));
  VectSet(lpVect, ADJUSTINDEX(lIndex), (VECT_HANDLE)lValue);
  return bReturn;
  */
}

//--------------------------------------------------------------------
//
//  The BitVectRemove() function clears an elment in the sparse bit
//  vector.
//

BOOL VECTAPI BitVectRemove(LPBITVECTOR lpVect, long lIndex)
{
  BOOL bReturn = FALSE;
  long lAdjustIndex = ADJUSTINDEX(lIndex) - lpVect->lIndex;
  long lRemoveBit = 1L << TOBIT(lIndex);
  
  if( 0 <= lAdjustIndex && lAdjustIndex < lpVect->lLeaves )
  {
    bReturn = 0 != VectRemoveEx(&lpVect->hNode, lpVect->lLeaves,
                                lAdjustIndex, lRemoveBit);
    VectShrink(lpVect);
  }
  return bReturn;

  /*
  long lValue;
  BOOL bReturn;
  
  lValue = (long)VectGet(lpVect, ADJUSTINDEX(lIndex));
  bReturn = (BOOL)(lValue >> TOBIT(lIndex) & 1);
  lValue &= ~(1L << TOBIT(lIndex));
  VectSet(lpVect, ADJUSTINDEX(lIndex), (VECT_HANDLE)lValue);
  return bReturn;
  */
}

//--------------------------------------------------------------------
//
//  The BitVectRemoveFirst() function clears the first element in the
//  sparse bit vector.
//

BOOL VECTAPI BitVectRemoveFirst(LPBITVECTOR lpVect, LPLONG lplIndex)
{
  //long lValue;
  //long lIndex;
  //long i;
  //
  //if( lValue = (long)VectGetFirst(lpVect, &lIndex) )
  //{
  //  for( i = 0; i < BITS_PER_VECT_HANDLE; i++ )
  //    if( lValue >> i & 1 )
  //    {
  //      *lplIndex = TOINDEX(lIndex,i);
  //      lValue &= ~(1L << i);
  //      VectSet(lpVect, lIndex, (VECT_HANDLE)lValue);
  //      return TRUE;
  //    }
  //}
  //return FALSE;
  
  BOOL bReturn = FALSE;
  long lIndex;
  long lBaseIndex;
  int iBit;
  
  if( lpVect->hNode )
  {
    lBaseIndex = lpVect->lIndex;
    bReturn = 0 != VectRemoveFirstEx(&lpVect->hNode, lpVect->lLeaves,
                                     &lIndex, &iBit, TRUE);
    if( bReturn )
    {
      *lplIndex = TOINDEX(lIndex + lBaseIndex, iBit);
      VectShrink(lpVect);
    }
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The BitVectRemoveAll() function clears all elements from the
//  sparse bit vector.
//

BOOL VECTAPI BitVectRemoveAll(LPBITVECTOR lpVect)
{
  return VectRemoveAll(lpVect);
}

//--------------------------------------------------------------------
//
//  The BITVECT_ENUMINFO structure is used during the enumeration of
//  a sparse bit vector.
//

typedef struct tagBITVECT_ENUMINFO
{
  BITVECT_ENUMPROC lpProc;
  long lData;
} BITVECT_ENUMINFO, FAR* LPBITVECT_ENUMINFO;

//--------------------------------------------------------------------
//
//  The BitVectEnumEx() function enumerates all non-zero bits in the
//  hValue variable.
//
#if defined(_WIN64) || defined(_IA64)
static BOOL CALLBACK BitVectEnumEx(long lIndex, VECT_HANDLE hValue,
                                   LONG_PTR lData)
#else
static BOOL CALLBACK BitVectEnumEx(long lIndex, VECT_HANDLE hValue,
                                   long lData)
#endif
{
  LPBITVECT_ENUMINFO lpData = (LPBITVECT_ENUMINFO)lData;
  BOOL bReturn = TRUE;
  int i;
  
  for( i = 0; bReturn && i < BITS_PER_VECT_HANDLE; i++ )
#if defined(_WIN64) || defined(_IA64)
    if( (LONG_PTR)hValue >> i & 1L )
#else
    if( (long)hValue >> i & 1L )
#endif
      bReturn = lpData->lpProc(TOINDEX(lIndex,i), lpData->lData);
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The BitVectEnum() function enumerates all non-zero elements in
//  the sparse bit vector.
//

BOOL VECTAPI BitVectEnum(LPVECTOR lpVect, BITVECT_ENUMPROC lpProc, long lData)
{
  BITVECT_ENUMINFO info;
  
  info.lpProc = lpProc;
  info.lData = lData;
#if defined(_WIN64) || defined(_IA64)
  return VectEnum(lpVect, BitVectEnumEx, (LONG_PTR)(LPBITVECT_ENUMINFO)&info);
#else
  return VectEnum(lpVect, BitVectEnumEx, (long)(LPBITVECT_ENUMINFO)&info);
#endif
}

//--------------------------------------------------------------------
//
//  The BitVectCopy() function copies non-zero bits in the source
//  bit vector to the destination bit vector.
//

BOOL VECTAPI BitVectCopy(LPBITVECTOR lpSrc, LPBITVECTOR lpDest)
{
  long lCol;
  BOOL bContinue;
  
  bContinue = BitVectGetFirst(lpSrc, &lCol);
  while( bContinue )
  {
    BitVectSet(lpDest, lCol, TRUE);
    bContinue = BitVectGetNext(lpSrc, &lCol);
  }
  return TRUE;
}
