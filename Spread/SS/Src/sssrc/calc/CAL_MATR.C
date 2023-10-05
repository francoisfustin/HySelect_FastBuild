//--------------------------------------------------------------------
//
//  File: cal_matr.c
//
//  Description: Routines to handle sparse matrixs.
//

#include <memory.h>
#include <windows.h>
#include "cal_matr.h"

#ifdef WIN32
  #define _fmemset memset
#endif

//--------------------------------------------------------------------
//
//  A sparse matrix is implemented as a tree.  Each node in the tree
//  has a fixed number of branches, which is determined by the
//  BRANCHES constant.  Each non-zero element in the matrix is stored
//  at a leaf on the tree.  All leaves of a given tree are at the same
//  depth.  The lCol and lRow fields of the MATRIX structure
//  determines the smallest column and row which the tree currently
//  spans.  The lColLeaves and lRowLeaves fields of the MATRIX
//  structure determine the total number of columns and rows that
//  the matrix currently spans.  In other words, all non-zero
//  elements have a column index c and a row index r such that
//      lCol <= c < lCol + lColLeaves
//      lRow <= r < lRow + lRowLeaves.
//  Since each node has a fixed number of branches, the lColLeaves
//  and lRowLeaves fields can also be used to determine the height
//  of the tree.  The index of an element can be determined by
//  following the branches from the root to the leaf.  The height of
//  the tree can grow or shrink as elements are added to or removed
//  from the sparse matrix.  It is assumed that all non NULL handles
//  in the tree will lead to a leaf.  Therefore it is important that
//  nodes are trimmed from the tree when no longer needed.  As the
//  height of the tree increases (or decreases), the lColLeaves and
//  lRowLeaves fields alternately increase (or decrease) by a factor
//  of BRANCHES.
//
//  The following is a diagram of a sparse matrix which has 4
//  branches per node and contains 3 items stored at (col=0,row=2),
//  (col=3,row=3) and (col=15,row=15).  The "X"'s represent valid
//  handles.  The "0"'s represent NULL handles.
//
//     +-----------------+
//  +--| hNode           |
//  |  | lCol = 0        |
//  |  | lRow = 0        |
//  |  | lColLeaves = 4  |
//  |  | lRowLeaves = 16 |
//  |  +-----------------+
//  |
//  |              +---------------+
//  |          +-->| X | 0 | 0 | X |
//  |          |   +---------------+
//  |          |     |           |
//  |          |     |  +---+    |             +---+
//  |   +---+  |     +->| 0 |    +------------>| 0 |
//  +-->| X |--+        +---+                  +---+
//      +---+           | 0 |   +---------+    | 0 |
//      | 0 |           +---+   | Item    |    +---+
//      +---+           | X |-->| Col = 0 |    | 0 |   +---------+
//      | 0 |           +---+   | Row = 2 |    +---+   | Item    |
//      +---+           | 0 |   +---------+    | X |-->| Col = 3 |
//      | X |--+        +---+                  +---+   | Row = 3 |
//      +---+  |                                       +---------+
//             |   +---------------+
//             +-->| 0 | 0 | 0 | X |
//                 +---------------+
//                               |
//                               |             +---+
//                               +------------>| 0 |
//                                             +---+
//                                             | 0 |
//                                             +---+
//                                             | 0 |   +----------+
//                                             +---+   | Item     |
//                                             | X |-->| Col = 3  |
//                                             +---+   | Row = 15 |
//                                                     +----------+
//
//
//  The following is a diagram of a sparse matrix which has 4 branches
//  per node and contains a single item stored at location (col=4,row=5).
// 
//    +----------------+     +---------+
//    | hNode          |---->| Item    |
//    | lCol = 4       |     | Col = 4 |
//    | lRow = 5       |     | Row = 5 |
//    | lColLeaves = 1 |     +---------+
//    | lRowLeaves = 1 |
//    +----------------+
//

//--------------------------------------------------------------------
//
//  The BRANCHES constant determines the number of column (or row)
//  branches at each node in a sparse matrix.  The MAXLEAVES
//  constant determines the largest column (or row) index that
//  a matrix can hold.  The MAXFULLLEAVES is the largest power
//  of BRANCHES which is less than of equal to MAXLEAVES.
//

//#define BRANCHES              16
//#define MAXFULLLEAVES  268435456
#define BRANCHES              32
#define MAXFULLLEAVES 1073741824
//#define BRANCHES              64
//#define MAXFULLLEAVES 1073741824
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
//  The BITS_PER_MAT_HANDLE constant determines the number of bits
//  in a matrix handle.
//

#define BITS_PER_MAT_HANDLE (8 * sizeof(MAT_HANDLE))

//--------------------------------------------------------------------
//
//  The following macros define the memory allocation scheme used by
//  the sparse matrix routines.
//

#ifdef MAT_USE_SMARTHEAP
  static MEM_POOL matMemPool = 0;
  #define MatMemAlloc() MemAllocFS(matMemPool)
  #define MatMemLock(hMem) ((LPMAT_HANDLE)hMem)
  #define MatMemUnlock(hMem)
  #define MatMemFree(hMem) MemFreeFS(hMem)
#else
  #define MatMemAlloc() \
      GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,BRANCHES*sizeof(MAT_HANDLE))
  #define MatMemLock(hMem) ((LPMAT_HANDLE)GlobalLock(hMem))
  #define MatMemUnlock(hMem) GlobalUnlock(hMem)
  #define MatMemFree(hMem) GlobalFree(hMem)
#endif

//--------------------------------------------------------------------
//
//  The MatMemPoolInit() function initializes a SmartHeap memory
//  pool which is used by the matrix routines.
//
//  Note: If SmartHeap memory management is being used, then
//        MatMemPoolInit() must be called before any other matrix
//        routine is used.
//

void MATAPI MatMemPoolInit()
{
#ifdef MAT_USE_SMARTHEAP
  if( !matMemPool )
    matMemPool = MemPoolInitFS(BRANCHES * sizeof(MAT_HANDLE),
                               16, MEM_POOL_DEFAULT);
#endif
}

//--------------------------------------------------------------------
//
//  The MatMemPoolFree() function frees the SmartHeap memory pool
//  which is being used by the matrix routines.
//
//  Note: If SmartHeap memory management is being used, then no
//        matrix routine can be used after a call to MatMemPoolFree().
//

void MATAPI MatMemPoolFree()
{
#ifdef MAT_USE_SMARTHEAP
  if( matMemPool )
    MemPoolFree(matMemPool);
  matMemPool = 0;
#endif
}

//--------------------------------------------------------------------
//
//  The MatInit() function initializes the sparse matrix structure.
//

BOOL MATAPI MatInit(LPMATRIX lpMatrix)
{
  lpMatrix->hNode = NULL;
  lpMatrix->lCol = 0;
  lpMatrix->lRow = 0;
  lpMatrix->lColLeaves = 1;
  lpMatrix->lRowLeaves = 1;
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The MatFree() function frees all memory associated with the
//  the sparse matrix.
//

BOOL MATAPI MatFree(LPMATRIX lpMatrix)
{
  MatRemoveAll(lpMatrix);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The MatMaxColEx() function returns the last column in the 
//  sub-matrix below the given node.
//

static long MatMaxColEx(MAT_HANDLE hNode, long lColLeaves, long lRowLeaves)
{
  LPMAT_HANDLE lpNode;
  int i;
  long lBranchMax;
  long lRet = 0;

  if( lColLeaves > 1 && hNode && (lpNode = MatMemLock(hNode)) )
  {
    if( lRowLeaves > lColLeaves )
    {
      lRowLeaves = SHRINKLEAVES(lRowLeaves);
      for( i = 0; i < BRANCHES; i++ )
      {
        if( lpNode[i] )
        {
          lBranchMax = MatMaxColEx(lpNode[i], lColLeaves, lRowLeaves);
          if( lBranchMax > lRet )
            lRet = lBranchMax;
        }
      }
    }
    else
    {
      lColLeaves = SHRINKLEAVES(lColLeaves);
      for( i = BRANCHES - 1; i >= 0 && 0 == lRet; i-- )
      {
        if( lpNode[i] )
        {
          lRet = i*lColLeaves + MatMaxColEx(lpNode[i],lColLeaves,lRowLeaves);
        }
      }
    }
    MatMemUnlock(hNode);
  }
  return lRet;
}

//--------------------------------------------------------------------
//
//  The MatMaxRowEx() function returns the last row in the 
//  sub-matrix below the given node.
//

static long MatMaxRowEx(MAT_HANDLE hNode, long lColLeaves, long lRowLeaves)
{
  LPMAT_HANDLE lpNode;
  int i;
  long lBranchMax;
  long lRet = 0;

  if( lRowLeaves > 1 && hNode && (lpNode = MatMemLock(hNode)) )
  {
    if( lRowLeaves > lColLeaves )
    {
      lRowLeaves = SHRINKLEAVES(lRowLeaves);
      for( i = BRANCHES - 1; i >= 0 && 0 == lRet; i-- )
      {
        if( lpNode[i] )
        {
          lRet = i*lRowLeaves + MatMaxRowEx(lpNode[i],lColLeaves,lRowLeaves);
        }
      }
    }
    else
    {
      lColLeaves = SHRINKLEAVES(lColLeaves);
      for( i = 0; i < BRANCHES; i++ )
      {
        if( lpNode[i] )
        {
          lBranchMax = MatMaxRowEx(lpNode[i], lColLeaves, lRowLeaves);
          if( lBranchMax > lRet )
            lRet = lBranchMax;
        }
      }
    }
    MatMemUnlock(hNode);
  }
  return lRet;
}

//--------------------------------------------------------------------
//
//  The MatMinCol() function returns the first column in the matrix.
//

long MATAPI MatMinCol(LPMATRIX lpMatrix)
{
  return lpMatrix->lCol;
}

//--------------------------------------------------------------------
//
//  The MatMaxCol() function returns the last column in the matrix.
//

long MATAPI MatMaxCol(LPMATRIX lpMatrix)
{
  //return lpMatrix->lCol + lpMatrix->lColLeaves - 1;
  return MatMaxColEx(lpMatrix->hNode,lpMatrix->lColLeaves,lpMatrix->lRowLeaves)
         + lpMatrix->lCol;
}
//--------------------------------------------------------------------
//
//  The MatMinRow() function returns the first row in the matrix.
//

long MATAPI MatMinRow(LPMATRIX lpMatrix)
{
  return lpMatrix->lRow;
}

//--------------------------------------------------------------------
//
//  The MatMaxRow() function returns the last row in the matrix.
//

long MATAPI MatMaxRow(LPMATRIX lpMatrix)
{
  //return lpMatrix->lRow + lpMatrix->lRowLeaves - 1;
  return MatMaxRowEx(lpMatrix->hNode,lpMatrix->lColLeaves,lpMatrix->lRowLeaves)
         + lpMatrix->lRow;
}

//--------------------------------------------------------------------
//
//  The MatGet() function retrieves an element from the sparse matrix
//  given its index.  If the element is found, the function returns
//  the handle of the element.  Otherwise, the function returns NULL.
//

MAT_HANDLE MATAPI MatGet(LPMATRIX lpMatrix, long lCol, long lRow)
{
  MAT_HANDLE hReturn = NULL;
  MAT_HANDLE hTemp;
  MAT_HANDLE hNode = lpMatrix->hNode;
  LPMAT_HANDLE lpNode;
  long lColLeaves = lpMatrix->lColLeaves;
  long lRowLeaves = lpMatrix->lRowLeaves;

  lCol -= lpMatrix->lCol;
  lRow -= lpMatrix->lRow;
  if( 0<= lCol && lCol < lColLeaves && 0 <= lRow && lRow < lRowLeaves )
  {
    while( lRowLeaves > 1 && hNode && (lpNode = MatMemLock(hNode)) )
    {
      if( lRowLeaves > lColLeaves )
      {
        lRowLeaves = SHRINKLEAVES(lRowLeaves);
        hTemp = lpNode[lRow/lRowLeaves];
        lRow %= lRowLeaves;
      }
      else
      {
        lColLeaves = SHRINKLEAVES(lColLeaves);
        hTemp = lpNode[lCol/lColLeaves];
        lCol %= lColLeaves;
      }
      MatMemUnlock(hNode);
      hNode = hTemp;
    }
    if( lRowLeaves == 1 )
      hReturn = hNode;
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The MatExpand() funciton expands the size of a tree until it
//  contains the given column and row indices.
//

static void MatExpand(LPMATRIX lpMatrix, long lCol, long lRow)
{
  LPMAT_HANDLE lpNode;
  MAT_HANDLE hTemp;
  BOOL b = 0;  // Used to work around a MSVC 1.5 compiler bug (fatal
               // error C1001: internal compiler error).  Bug occured
               // when building release versions of the OCX and VBX.

  while( lCol < lpMatrix->lCol
         || lpMatrix->lCol + lpMatrix->lColLeaves <= lCol
         || lRow < lpMatrix->lRow
         || lpMatrix->lRow + lpMatrix->lRowLeaves <= lRow )
  {
    if( lpMatrix->hNode
        && (hTemp = MatMemAlloc())
        && (lpNode = MatMemLock(hTemp)) )
    {
      _fmemset(lpNode, 0, BRANCHES * sizeof(MAT_HANDLE));
      if( lpMatrix->lRowLeaves > lpMatrix->lColLeaves )
        lpNode[(lpMatrix->lCol/lpMatrix->lColLeaves)%BRANCHES]
          = lpMatrix->hNode;
      else
        lpNode[(lpMatrix->lRow/lpMatrix->lRowLeaves)%BRANCHES]
          = lpMatrix->hNode;
      MatMemUnlock(hTemp);
      lpMatrix->hNode = hTemp;
    }
    if( lpMatrix->lRowLeaves > lpMatrix->lColLeaves )
    {
      lpMatrix->lColLeaves = EXPANDLEAVES(lpMatrix->lColLeaves);
      lpMatrix->lCol -= lpMatrix->lCol % lpMatrix->lColLeaves;
    }
    else
    {
      lpMatrix->lRowLeaves = EXPANDLEAVES(lpMatrix->lRowLeaves);
      lpMatrix->lRow -= lpMatrix->lRow % lpMatrix->lRowLeaves;
    }
  }
}

//--------------------------------------------------------------------
//
//  The MatSetEx() function places an element handle into the the
//  sparse matrix.
//
//  Note: It is assumed that hValue != 0 and 0 <= lCol < lColLeaves
//        and 0 <= lRow < lRowLeaves.
//
#if defined(_WIN64) || defined(_IA64)
static MAT_HANDLE MatSetEx(LPMAT_HANDLE lphNode,
                           long lColLeaves, long lRowLeaves,
                           long lCol, long lRow,
                           MAT_HANDLE hValue, LONG_PTR lSetBit)
#else
static MAT_HANDLE MatSetEx(LPMAT_HANDLE lphNode,
                           long lColLeaves, long lRowLeaves,
                           long lCol, long lRow,
                           MAT_HANDLE hValue, long lSetBit)
#endif
{
  MAT_HANDLE hReturn = NULL;
  MAT_HANDLE hTemp = NULL;
  MAT_HANDLE hNode = NULL;
  LPMAT_HANDLE lpNode;

  // scan down the tree, createing branches if needed
  hTemp = NULL;
  while( lRowLeaves > 1 && lphNode )
  {
    if( *lphNode )
      lpNode = MatMemLock(*lphNode);
    else
    {
      *lphNode = MatMemAlloc();
      if( lpNode = MatMemLock(*lphNode) )
        _fmemset(lpNode, 0, BRANCHES*sizeof(MAT_HANDLE));
    }
    if( *lphNode )
    {
      hNode = *lphNode;
      if( hTemp )
        MatMemUnlock( hTemp );
      hTemp = hNode;
      if( lpNode )
      {
        if( lRowLeaves > lColLeaves )
        {
          lRowLeaves = SHRINKLEAVES(lRowLeaves);
          lphNode = &lpNode[lRow/lRowLeaves];
          lRow %= lRowLeaves;
        }
        else
        {
          lColLeaves = SHRINKLEAVES(lColLeaves);
          lphNode = &lpNode[lCol/lColLeaves];
          lCol %= lColLeaves;
        }
      }
      else
        lphNode = NULL; // MatMemLock() failed, break loop
    }
    else
      lphNode = NULL; // MatMemAlloc failed, break loop
  }
  // finally at leaf, can store old value and set new value
  if( lphNode )
  {
    if( lSetBit )
    {
#if defined(_WIN64) || defined(_IA64)
      hReturn = (MAT_HANDLE)((LONG_PTR)*lphNode & lSetBit);
      *lphNode = (MAT_HANDLE)((LONG_PTR)*lphNode | lSetBit);
#else
      hReturn = (MAT_HANDLE)((long)*lphNode & lSetBit);
      *lphNode = (MAT_HANDLE)((long)*lphNode | lSetBit);
#endif
    }
    else
    {
      hReturn = *lphNode;
      *lphNode = hValue;
    }
  }
  if( hTemp )
    MatMemUnlock(hTemp);
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The MatSet() function places an element handle into the sparse
//  matrix.  If the new value is NULL then the element is actually
//  remove from the sparse matrix.  If needed, the size of the matrix
//  is expanded.  The function returns the previous value which was
//  stored in the matrix at the given index.
//

MAT_HANDLE MATAPI MatSet(LPMATRIX lpMatrix, long lCol, long lRow,
                         MAT_HANDLE hValue)
{
  MAT_HANDLE hReturn = NULL;

  if( 0 <= lCol && lCol <= MAXLEAVES && 0 <= lRow && lRow <= MAXLEAVES )
  {
    if( !hValue )
      hReturn = MatRemove(lpMatrix, lCol, lRow);
    else if( !lpMatrix->hNode )
    {
      lpMatrix->hNode = hValue;
      lpMatrix->lCol = lCol;
      lpMatrix->lRow = lRow;
      lpMatrix->lColLeaves = 1;
      lpMatrix->lRowLeaves = 1;
    }
    else
    {
      MatExpand(lpMatrix, lCol, lRow);
      lCol -= lpMatrix->lCol;
      lRow -= lpMatrix->lRow;
      hReturn = MatSetEx(&lpMatrix->hNode,
                         lpMatrix->lColLeaves, lpMatrix->lRowLeaves,
                         lCol, lRow,
                         hValue, 0);
    }
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The MatRemoveEx() function removes an element from the sparse
//  matrix which is stored at or below the given node.  Nodes with
//  no more branches are also removed.  The function returns the
//  previous value of the element an the given location.
//

static MAT_HANDLE MatRemoveEx(LPMAT_HANDLE lphNode,
                              long lColLeaves, long lRowLeaves,
                              long lCol, long lRow,
                              long lRemoveBit)
{
  MAT_HANDLE hReturn = NULL;
  LPMAT_HANDLE lpNode;
  BOOL bFree;
  int i;
  
  if( lRowLeaves > 1 )
  {
    if( *lphNode && (lpNode = MatMemLock(*lphNode)) )
    {
      if( lRowLeaves > lColLeaves )
      {
        lRowLeaves = SHRINKLEAVES(lRowLeaves);
        i = (int)(lRow / lRowLeaves);
        hReturn = MatRemoveEx(lpNode + i,
                              lColLeaves, lRowLeaves,
                              lCol, lRow%lRowLeaves,
                              lRemoveBit/*, &bFree*/);
      }
      else
      {
        lColLeaves = SHRINKLEAVES(lColLeaves);
        i = (int)(lCol / lColLeaves);
        hReturn = MatRemoveEx(lpNode + i,
                              lColLeaves, lRowLeaves,
                              lCol%lColLeaves, lRow,
                              lRemoveBit/*, &bFree*/);
      }
      if( bFree = !lpNode[i] )
      {
        for( i = 0; i < BRANCHES; i++ )
          if( lpNode[i] )
          {
            bFree = FALSE;
            break;
          }
      }
      MatMemUnlock(*lphNode);
      if( bFree )
      {
        MatMemFree(*lphNode);
        *lphNode = NULL;
      }
    }
  }
  else
  {
    if( lRemoveBit )
    {
#if defined(_WIN64) || defined(_IA64)
      hReturn = (MAT_HANDLE)((LONG_PTR)*lphNode & lRemoveBit);
      *lphNode = (MAT_HANDLE)((LONG_PTR)*lphNode & ~lRemoveBit);
#else
      hReturn = (MAT_HANDLE)((long)*lphNode & lRemoveBit);
      *lphNode = (MAT_HANDLE)((long)*lphNode & ~lRemoveBit);
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
//  The MatShrink() function shrinks the tree to the minimum height
//  which still spans all of its elements.
//

void MatShrink(LPMATRIX lpMatrix)
{
  MAT_HANDLE hNonZero;
  LPMAT_HANDLE lpNode;
  BOOL bShrink = TRUE;
  long lNonZero;
  int i;

  while( lpMatrix->lRowLeaves > 1 && lpMatrix->hNode && bShrink )
  {
    hNonZero = 0;
    if( lpNode = MatMemLock(lpMatrix->hNode) )
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
      MatMemUnlock(lpMatrix->hNode);
    }
    else
      bShrink = FALSE;
    if( bShrink && hNonZero )
    {
      MatMemFree(lpMatrix->hNode);
      lpMatrix->hNode = hNonZero;
      if( lpMatrix->lRowLeaves > lpMatrix->lColLeaves )
      {
        lpMatrix->lRowLeaves = SHRINKLEAVES(lpMatrix->lRowLeaves);
        lpMatrix->lRow += lNonZero * lpMatrix->lRowLeaves;
      }
      else
      {
        lpMatrix->lColLeaves = SHRINKLEAVES(lpMatrix->lColLeaves);
        lpMatrix->lCol += lNonZero * lpMatrix->lColLeaves;
      }
    }
  }
  if( !lpMatrix->hNode )
    MatInit(lpMatrix);
}

//--------------------------------------------------------------------
//
//  The MatRemove() function removes an element from the sparse
//  matrix.  If needed, the height of the tree is shrunk.  The
//  function returns the previous value of the element.
//

MAT_HANDLE MATAPI MatRemove(LPMATRIX lpMatrix, long lCol, long lRow)
{
  MAT_HANDLE hReturn = NULL;
  
  lCol -= lpMatrix->lCol;
  lRow -= lpMatrix->lRow;
  if( 0 <= lCol && lCol < lpMatrix->lColLeaves
      && 0 <= lRow && lRow < lpMatrix->lRowLeaves )
  {
    hReturn = MatRemoveEx(&lpMatrix->hNode,
                          lpMatrix->lColLeaves, lpMatrix->lRowLeaves,
                          lCol, lRow, 0);
    MatShrink(lpMatrix);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The MatRemoveFirstEx() function removes the first element from
//  the sparse matrix which is stored at or below the given node.
//  Nodes with no more branches are also removed.  The function
//  returns the previous value of the element an the given location.
//

static MAT_HANDLE MatRemoveFirstEx(LPMAT_HANDLE lphNode,
                                   long lColLeaves, long lRowLeaves,
                                   LPLONG lplCol, LPLONG lplRow,
                                   LPINT lpiBit, BOOL bRemoveBit)
{
  MAT_HANDLE hReturn = NULL;
  LPMAT_HANDLE lpNode;
  BOOL bFree = TRUE;
  int i;
  
  if( lRowLeaves > 1 )
  {
    if( *lphNode && (lpNode = MatMemLock(*lphNode)) )
    { 
      for( i = 0; i < BRANCHES; i++ )
      {
        if( lpNode[i] )
        {
          if( lRowLeaves > lColLeaves )
          {
            lRowLeaves = SHRINKLEAVES(lRowLeaves);
            hReturn = MatRemoveFirstEx(lpNode + i,
                                       lColLeaves, lRowLeaves,
                                       lplCol, lplRow,
                                       lpiBit, bRemoveBit);
            if( hReturn )
            {
              *lplRow += i * lRowLeaves;
              break;
            }
          }
          else
          {
            lColLeaves = SHRINKLEAVES(lColLeaves);
            hReturn = MatRemoveFirstEx(lpNode + i,
                                       lColLeaves, lRowLeaves,
                                       lplCol, lplRow,
                                       lpiBit, bRemoveBit);
            if( hReturn )
            {
              *lplCol += i * lColLeaves;
              break;
            }
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
      MatMemUnlock(*lphNode);
      if( bFree )
      {
        MatMemFree(*lphNode);
        *lphNode = NULL;
      }
    }
  }
  else
  {
    if( bRemoveBit )
    {
      for( i = 0; !hReturn && i < BITS_PER_MAT_HANDLE; i++ )
      {
#if defined(_WIN64) || defined(_IA64)
        if( (LONG_PTR)*lphNode >> i & 1 )
#else
        if( (long)*lphNode >> i & 1 )
#endif
        {
          hReturn = (MAT_HANDLE)1;
#if defined(_WIN64) || defined(_IA64)
          *lphNode = (MAT_HANDLE)((LONG_PTR)*lphNode & ~(1L << i));
#else
          *lphNode = (MAT_HANDLE)((long)*lphNode & ~(1L << i));
#endif
          *lplCol = 0;
          *lplRow = 0;
          *lpiBit = i;
        }
      }
    }
    else
    {
      hReturn = *lphNode;
      *lphNode = NULL;
      *lplCol = 0;
      *lplRow = 0;
    }
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The MatRemoveFirst() function removes the first element from the
//  sparse matrix.  If needed, the height of the tree is shrunk.  The
//  function returns the previous value of the element.
//

MAT_HANDLE MATAPI MatRemoveFirst(LPMATRIX lpMatrix,
                                 LPLONG lplCol, LPLONG lplRow)
{
  MAT_HANDLE hReturn;
  long lBaseCol = lpMatrix->lCol;
  long lBaseRow = lpMatrix->lRow;
  
  hReturn = MatRemoveFirstEx(&lpMatrix->hNode,
                             lpMatrix->lColLeaves, lpMatrix->lRowLeaves,
                             lplCol, lplRow, NULL, FALSE);
  if( hReturn )
  {
    *lplCol += lBaseCol;
    *lplRow += lBaseRow;
    MatShrink(lpMatrix);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The MatRemoveAllEx() function removes all elements from the
//  sparse matrix which are at or below the given node.
//
//  Note: The calling routine is responsible for setting hNode = NULL.
//

static BOOL MatRemoveAllEx(MAT_HANDLE hNode,
                           long lColLeaves, long lRowLeaves)
{
  LPMAT_HANDLE lpNode;
  int i;
  
  if( lRowLeaves > 1 && hNode )
  {
    if( lpNode = MatMemLock(hNode) )
    {
      for( i = 0; i < BRANCHES; i++ )
        if( lRowLeaves > lColLeaves )
          MatRemoveAllEx(lpNode[i], lColLeaves, SHRINKLEAVES(lRowLeaves));
        else
          MatRemoveAllEx(lpNode[i], SHRINKLEAVES(lColLeaves), lRowLeaves);
      MatMemUnlock(hNode);
    }
    MatMemFree(hNode);
  }
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The MatRemoveAll() function removes all items from the given
//  sparse matrix.  The sparse matrix is restored to its initial state.
//
//  Note: The calling routine is responsible for deleting all
//        memory associated with the values of the elements.
//

BOOL MATAPI MatRemoveAll(LPMATRIX lpMatrix)
{
  MatRemoveAllEx(lpMatrix->hNode,
                 lpMatrix->lColLeaves, lpMatrix->lRowLeaves);
  MatInit(lpMatrix);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The MatGetFirstEx() function retrieves the first element at or
//  below the given node.  If found, the function returns the handle
//  of the element and location of the element is stored in *lplCol
//  and *lplRow.  Otherwise, the function returns NULL and both
//  *lplCol and *lplRow are undefined.
//

static MAT_HANDLE MatGetFirstEx(MAT_HANDLE hNode,
                               long lColLeaves, long lRowLeaves,
                               LPLONG lplCol, LPLONG lplRow)
{
  MAT_HANDLE hTemp;
  LPMAT_HANDLE lpNode;
  MAT_HANDLE hReturn = NULL;
  int i;

  *lplCol = 0;
  *lplRow = 0;
  while( lRowLeaves > 1 && hNode && (lpNode = MatMemLock(hNode)) )
  {
    hTemp = NULL;
    for(i = 0; i < BRANCHES; i++ )
      if( lpNode[i] )
      {
        hTemp = lpNode[i];
        if( lRowLeaves > lColLeaves )
        {
          lRowLeaves = SHRINKLEAVES(lRowLeaves);
          *lplRow = *lplRow * BRANCHES + i;
        }
        else
        {
          lColLeaves = SHRINKLEAVES(lColLeaves);
          *lplCol = *lplCol * BRANCHES + i;
        }
        break;
      }
    MatMemUnlock(hNode);
    hNode = hTemp;
  }
  if( lRowLeaves == 1 )
    hReturn = hNode;
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The MatGetFirst() function retrieves the first element in the
//  sparse matrix.  If found, the function returns the handle of the
//  element and location of the element is stored in *lplCol and
//  *lplRow.  Otherwise, the function returns NULL and both *lplCol
//  and *lplRow are undefined.
//

MAT_HANDLE MATAPI MatGetFirst(LPMATRIX lpMatrix,
                              LPLONG lplCol, LPLONG lplRow)
{
  MAT_HANDLE hReturn = 0;
  
  if( lpMatrix->hNode )
    hReturn = MatGetFirstEx(lpMatrix->hNode,
                            lpMatrix->lColLeaves, lpMatrix->lRowLeaves,
                            lplCol, lplRow);
  if( hReturn )
  {
    *lplCol += lpMatrix->lCol;
    *lplRow += lpMatrix->lRow;
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The MatGetNextEx() function finds the next element which is at or
//  below the given node.  If an element is found, the function
//  returns the handle of the element and the location of the element
//  is stored in *lplCol and *lplRow.  Otherwise, the function
//  returns NULL and both *lplCol and *lplRow are undefined.
//

static MAT_HANDLE MatGetNextEx(MAT_HANDLE hNode,
                               long lColLeaves, long lRowLeaves,
                               LPLONG lplCol, LPLONG lplRow)
{
  LPMAT_HANDLE lpNode;
  MAT_HANDLE hReturn = NULL;
  long lCol = *lplCol;
  long lRow = *lplRow;
  long i;
  
  if( lRowLeaves > 1 && hNode && (lpNode = MatMemLock(hNode)) )
  {
    if( lRowLeaves > lColLeaves )
    {
      lRowLeaves = SHRINKLEAVES(lRowLeaves);
      lRow %= lRowLeaves;
      i = *lplRow / lRowLeaves;
    }
    else
    {
      lColLeaves = SHRINKLEAVES(lColLeaves);
      lCol %= lColLeaves;
      i = *lplCol / lColLeaves;
    }
    hReturn = MatGetNextEx(lpNode[i], lColLeaves, lRowLeaves, &lCol, &lRow);
    if( hReturn )
    {
      *lplCol = *lplCol/lColLeaves * lColLeaves + lCol;
      *lplRow = *lplRow/lRowLeaves * lRowLeaves + lRow;
    }
    else
    {
      for( i++; !hReturn && i < BRANCHES; i++ )
      {
        if( lpNode[i] )
        {
          if( hReturn = MatGetFirstEx(lpNode[i], lColLeaves, lRowLeaves,
                                      &lCol, &lRow) )
          {
            if( lRowLeaves > lColLeaves )
            {
              *lplCol = i * lColLeaves + lCol;
              *lplRow = lRow;
            }
            else
            {
              *lplCol = lCol;
              *lplRow = i * lRowLeaves + lRow;
            }
          }
        }
      }
    }
    MatMemUnlock(hNode);
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The MatGetNext() function finds the next element in the sparse
//  matrix.  If an element is found, the function returns the handle of
//  the element and the location of the element is stored in *lplCol
//  and *lplRow.  Otherwise, the function returns NULL and both
//  *lplCol and *lplRow are undefined.
//
//  Note: MatGetNext() may give inconsistent results if the matrix
//        is modified between calls to MatGetNext().
//

MAT_HANDLE MATAPI MatGetNext(LPMATRIX lpMatrix, LPLONG lplCol, LPLONG lplRow)
{
  long lCol = *lplCol - lpMatrix->lCol;
  long lRow = *lplRow - lpMatrix->lRow;
  MAT_HANDLE hReturn = NULL;

  if( lCol < 0 || lRow < 0 )
  {
    hReturn = MatGetFirstEx(lpMatrix->hNode,
                            lpMatrix->lColLeaves, lpMatrix->lRowLeaves,
                            &lCol, &lRow);
  }
  else if( lCol < lpMatrix->lColLeaves && lRow < lpMatrix->lRowLeaves )
  { 
    hReturn = MatGetNextEx(lpMatrix->hNode,
                           lpMatrix->lColLeaves, lpMatrix->lRowLeaves,
                           &lCol, &lRow);
  }
  if( hReturn )
  {
    *lplCol = lCol + lpMatrix->lCol;
    *lplRow = lRow + lpMatrix->lRow;
  }
  return hReturn;
}

//--------------------------------------------------------------------
//
//  The MatEnumEx() function enumerates the items which are at or
//  below the given node.
//
#if defined(_WIN64) || defined(_IA64)
static BOOL MatEnumEx(MAT_HANDLE hNode,
                      long lColLeaves, long lRowLeaves,
                      long lCol, long lRow,
                      MAT_ENUMPROC lpProc, LONG_PTR lData)
#else
static BOOL MatEnumEx(MAT_HANDLE hNode,
                      long lColLeaves, long lRowLeaves,
                      long lCol, long lRow,
                      MAT_ENUMPROC lpProc, long lData)
#endif
{
  LPMAT_HANDLE lpNode;
  BOOL bReturn = TRUE;
  int i;
  
  if( lRowLeaves == 1 )
  {
    if( hNode )
      bReturn = lpProc(lCol, lRow, hNode, lData);
  }
  else if( lRowLeaves > 1 && (lpNode = MatMemLock(hNode)) )
  {
    if( lRowLeaves > lColLeaves )
    {
      lRowLeaves = SHRINKLEAVES(lRowLeaves);
      for( i = 0; bReturn && i < BRANCHES; i++, lRow += lRowLeaves )
        if( lpNode[i] )
        {
          bReturn = MatEnumEx(lpNode[i], lColLeaves, lRowLeaves,
                              lCol, lRow, lpProc, lData);
        }
    }
    else
    {
      lColLeaves = SHRINKLEAVES(lColLeaves);
      for( i = 0; bReturn && i < BRANCHES; i++, lCol += lColLeaves )
        if( lpNode[i] )
        {
          bReturn = MatEnumEx(lpNode[i], lColLeaves, lRowLeaves,
                              lCol, lRow, lpProc, lData);
        }
    }
    MatMemUnlock(hNode);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The MatEnum() function enumerates the items in the given matrix.
//
#if defined(_WIN64) || defined(_IA64)
BOOL MATAPI MatEnum(LPMATRIX lpMatrix, MAT_ENUMPROC lpProc, LONG_PTR lData)
#else
BOOL MATAPI MatEnum(LPMATRIX lpMatrix, MAT_ENUMPROC lpProc, long lData)
#endif
{
  return MatEnumEx(lpMatrix->hNode,
                   lpMatrix->lColLeaves, lpMatrix->lRowLeaves,
                   lpMatrix->lCol, lpMatrix->lRow,
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
//  The following macros are used to adjust between sparse matrix
//  coordinates and sparse bit matrix coordinates.
//

#define ADJUSTCOL(lCol,lRow) ((lCol) / BITS_PER_MAT_HANDLE)
#define ADJUSTROW(lCol,lRow) (lRow)
#define TOCOL(lCol,lRow,iBit) ((lCol) * BITS_PER_MAT_HANDLE + iBit)
#define TOROW(lCol,lRow,iBit) (lRow)
#define TOBIT(lCol,lRow) ((lCol) % BITS_PER_MAT_HANDLE)

//#define ADJUSTCOL(lCol,lRow) ((lRow) / BITS_PER_MAT_HANDLE)
//#define ADJUSTROW(lCol,lRow) (lCol)
//#define TOCOL(lCol,lRow,iBit) (lRow)
//#define TOROW(lCol,lRow,iBit) ((lCol) * BITS_PER_MAT_HANDLE + iBit)
//#define TOBIT(lCol,lRow) ((lRow) % BITS_PER_MAT_HANDLE)

//--------------------------------------------------------------------
//
//  The BitMatInit() function initializes a sparse bit matrix
//  structure.
//

BOOL MATAPI BitMatInit(LPBITMATRIX lpMatrix)
{
  return MatInit(lpMatrix);
}

//--------------------------------------------------------------------
//
//  The BitMatFree() function frees all memory associated with
//  the sparse bit matrix.
//

BOOL MATAPI BitMatFree(LPBITMATRIX lpMatrix)
{
  return MatFree(lpMatrix);
}

//--------------------------------------------------------------------
//
//  The BitMatGet() function retrieves an element from a sparse
//  bit matrix.
//

BOOL MATAPI BitMatGet(LPBITMATRIX lpMatrix, long lCol, long lRow)
{
  MAT_HANDLE lValue;
  
  lValue = MatGet(lpMatrix, ADJUSTCOL(lCol,lRow), ADJUSTROW(lCol,lRow));
#if defined(_WIN64) || defined(_IA64)
  return (BOOL)((LONG_PTR)lValue >> TOBIT(lCol,lRow) & 1);
#else
  return (BOOL)((long)lValue >> TOBIT(lCol,lRow) & 1);
#endif
}

//--------------------------------------------------------------------
//
//  The BitMatGetFirst() function retrieves the first element from a
//  sparse bit matrix.
//

BOOL MATAPI BitMatGetFirst(LPBITMATRIX lpMatrix, LPLONG lplCol, LPLONG lplRow)
{
  MAT_HANDLE lValue;
  long lCol;
  long lRow;
  long i;
  
  if( lpMatrix->hNode
      && (lValue = MatGetFirst(lpMatrix, &lCol, &lRow)) )
  {
    for( i = 0; i < BITS_PER_MAT_HANDLE; i++ )
#if defined(_WIN64) || defined(_IA64)
      if( (LONG_PTR)lValue >> i & 1 )
#else
      if( (long)lValue >> i & 1 )
#endif
      {
        *lplCol = TOCOL(lCol,lRow,i);
        *lplRow = TOROW(lCol,lRow,i);
        return TRUE;
      }
  }
  return FALSE;
}

//--------------------------------------------------------------------
//
//  The BitMatGetNext() function retrives the next element from a
//  sparse bit matrix.
//

BOOL MATAPI BitMatGetNext(LPBITMATRIX lpMatrix, LPLONG lplCol, LPLONG lplRow)
{
  MAT_HANDLE lValue;
  long lCol = ADJUSTCOL(*lplCol,*lplRow);
  long lRow = ADJUSTROW(*lplCol,*lplRow);
  long i;
  
  lValue = MatGet(lpMatrix, lCol, lRow);
  i = TOBIT(*lplCol,*lplRow) + 1;
#if defined(_WIN64) || defined(_IA64)
  if( (LONG_PTR)lValue & lMoreBits[i] )
#else
  if( (long)lValue & lMoreBits[i] )
#endif
  {
    for( ; i < BITS_PER_MAT_HANDLE; i++ )
#if defined(_WIN64) || defined(_IA64)
      if( (LONG_PTR)lValue >> i & 1 )
#else
      if( (long)lValue >> i & 1 )
#endif
      {
        *lplCol = TOCOL(lCol,lRow,i);
        *lplRow = TOROW(lCol,lRow,i);
        return TRUE;
      }
  }
  if( lValue = MatGetNext(lpMatrix, &lCol, &lRow) )
  {
    for( i = 0; i < BITS_PER_MAT_HANDLE; i++ )
#if defined(_WIN64) || defined(_IA64)
      if( (LONG_PTR)lValue >> i & 1 )
#else
      if( (long)lValue >> i & 1 )
#endif
      {
        *lplCol = TOCOL(lCol,lRow,i);
        *lplRow = TOROW(lCol,lRow,i);
        return TRUE;
      }
  }
  return FALSE;
}

//--------------------------------------------------------------------
//
//  The BitMatSet() function sets (or clears) an element in the
//  sparse bit matrix.
//

BOOL MATAPI BitMatSet(LPBITMATRIX lpMatrix, long lCol, long lRow, BOOL bValue)
{
  BOOL bReturn = FALSE;
  long lAdjustCol = ADJUSTCOL(lCol,lRow);
  long lAdjustRow = ADJUSTROW(lCol,lRow);
#if defined(_WIN64) || defined(_IA64)
  LONG_PTR lSetBit = 1L << TOBIT(lCol,lRow);
#else
  long lSetBit = 1L << TOBIT(lCol,lRow);
#endif

  if( !bValue )
    bReturn = BitMatRemove(lpMatrix, lCol, lRow);
  else if( !lpMatrix->hNode )
  {
    lpMatrix->hNode = (MAT_HANDLE)lSetBit;
    lpMatrix->lCol = lAdjustCol;
    lpMatrix->lRow = lAdjustRow;
    lpMatrix->lColLeaves = 1;
    lpMatrix->lRowLeaves = 1;
  }
  else
  {
    MatExpand(lpMatrix, lAdjustCol, lAdjustRow);
    lAdjustCol -= lpMatrix->lCol;
    lAdjustRow -= lpMatrix->lRow;
    bReturn = 0 != MatSetEx(&lpMatrix->hNode,
                            lpMatrix->lColLeaves, lpMatrix->lRowLeaves,
                            lAdjustCol, lAdjustRow,
                            0, lSetBit);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The BitMatRemove() function clears an element in the sparse bit
//  matrix.
//

BOOL MATAPI BitMatRemove(LPBITMATRIX lpMatrix, long lCol, long lRow)
{
  BOOL bReturn = FALSE;
  long lAdjustCol = ADJUSTCOL(lCol,lRow) - lpMatrix->lCol;
  long lAdjustRow = ADJUSTROW(lCol,lRow) - lpMatrix->lRow;
  long lRemoveBit = 1L << TOBIT(lCol,lRow);
  
  if( 0 <= lAdjustCol && lAdjustCol < lpMatrix->lColLeaves
      && 0 <= lAdjustRow && lAdjustRow < lpMatrix->lRowLeaves )
  {
    bReturn = 0 != MatRemoveEx(&lpMatrix->hNode,
                               lpMatrix->lColLeaves, lpMatrix->lRowLeaves,
                               lAdjustCol, lAdjustRow, lRemoveBit);
    MatShrink(lpMatrix);
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The BitMatRemoveFirst() function clears the first element in the
//  sparse bit matrix.
//

BOOL MATAPI BitMatRemoveFirst(LPBITMATRIX lpMatrix,
                              LPLONG lplCol, LPLONG lplRow)
{
  //long lValue;
  //long lCol;
  //long lRow;
  //long i;
  //
  //if( lValue = (long)MatGetFirst(lpMatrix, &lCol, &lRow) )
  //{
  //  for( i = 0; i < BITS_PER_MAT_HANDLE; i++ )
  //    if( lValue >> i & 1 )
  //    {
  //      *lplCol = TOCOL(lCol,lRow,i);
  //      *lplRow = TOROW(lCol,lRow,i);
  //      lValue &= ~(1L << i);
  //      MatSet(lpMatrix, lCol, lRow, (MAT_HANDLE)lValue);
  //      return TRUE;
  //    }
  //}
  //return FALSE;
  
  BOOL bReturn = FALSE;
  long lCol;
  long lRow;
  long lBaseCol;
  long lBaseRow;
  int iBit;

  if( lpMatrix->hNode )
  {  
    lBaseCol = lpMatrix->lCol;
    lBaseRow = lpMatrix->lRow;
    bReturn = 0 != MatRemoveFirstEx(&lpMatrix->hNode,
                                    lpMatrix->lColLeaves, lpMatrix->lRowLeaves,
                                    &lCol, &lRow, &iBit, TRUE);
    if( bReturn )
    {
      *lplCol = TOCOL(lCol + lBaseCol, lRow + lBaseRow, iBit);
      *lplRow = TOROW(lCol + lBaseCol, lRow + lBaseRow, iBit);
      MatShrink(lpMatrix);
    }
  }
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The BitMatRemoveAll() function clears all elements from the
//  sparse bit matrix.
//

BOOL MATAPI BitMatRemoveAll(LPBITMATRIX lpMatrix)
{
  return MatRemoveAll(lpMatrix);
}

//--------------------------------------------------------------------
//
//  The BITMAT_ENUMINFO structure is used during the enumeration of
//  a sparse bit matrix.
//

typedef struct tagBITMAT_ENUMINFO
{
  BITMAT_ENUMPROC lpProc;
  long lData;
} BITMAT_ENUMINFO, FAR* LPBITMAT_ENUMINFO;

//--------------------------------------------------------------------
//
//  The BitMatEnumEx() function enumerates all non-zero bits in the
//  hValue variable.
//
#if defined(_WIN64) || defined(_IA64)
static BOOL CALLBACK BitMatEnumEx(long lCol, long lRow,
                                  MAT_HANDLE hValue, LONG_PTR lData)
#else
static BOOL CALLBACK BitMatEnumEx(long lCol, long lRow,
                                  MAT_HANDLE hValue, long lData)
#endif
{
  LPBITMAT_ENUMINFO lpData = (LPBITMAT_ENUMINFO)lData;
  BOOL bReturn = TRUE;
  int i;
  
  for( i = 0; bReturn && i < BITS_PER_MAT_HANDLE; i++ )
#if defined(_WIN64) || defined(_IA64)
    if( (LONG_PTR)hValue >> i & 1L )
#else
    if( (long)hValue >> i & 1L )
#endif
      bReturn = lpData->lpProc(TOCOL(lCol,lRow,i), TOROW(lCol,lRow,i),
                               lpData->lData);
  return bReturn;
}

//--------------------------------------------------------------------
//
//  The BitMatEnum() function enumerates all non-zero elements in the
//  sparse bit matrix.
//

BOOL MATAPI BitMatEnum(LPBITMATRIX lpMatrix, BITMAT_ENUMPROC lpProc, long lData)
{
  BITMAT_ENUMINFO info;
  
  info.lpProc = lpProc;
  info.lData = lData;
#if defined(_WIN64) || defined(_IA64)
  return MatEnum(lpMatrix, BitMatEnumEx, (LONG_PTR)&info);
#else
  return MatEnum(lpMatrix, BitMatEnumEx, (long)&info);
#endif
}

//--------------------------------------------------------------------
//
//  The BitMatCopy() function copies non-zero bits from the source
//  bit matrix to the destination matrix.
//

BOOL MATAPI BitMatCopy(LPBITMATRIX lpSrc, LPBITMATRIX lpDest)
{
  long lCol;
  long lRow;
  MAT_HANDLE hValue;
  
  hValue = MatGetFirst(lpSrc, &lCol, &lRow);
  while( hValue )
  {
    if( !lpDest->hNode )
    {
      lpDest->hNode = hValue;
      lpDest->lCol = lCol;
      lpDest->lRow = lRow;
      lpDest->lColLeaves = 1;
      lpDest->lRowLeaves = 1;
    }
    else
    {
      MatExpand(lpDest, lCol, lRow);
#if defined(_WIN64) || defined(_IA64)
      MatSetEx(&lpDest->hNode, lpDest->lColLeaves, lpDest->lRowLeaves,
               lCol - lpDest->lCol, lRow - lpDest->lRow, 0, (LONG_PTR)hValue);
#else
      MatSetEx(&lpDest->hNode, lpDest->lColLeaves, lpDest->lRowLeaves,
               lCol - lpDest->lCol, lRow - lpDest->lRow, 0, (long)hValue);
#endif
    }
    hValue = MatGetNext(lpSrc, &lCol, &lRow);
  }

  /*  
  BOOL bContinue;

  bContinue = BitMatGetFirst(lpSrc, &lCol, &lRow);
  while( bContinue )
  {
    BitMatSet(lpDest, lCol, lRow, TRUE);
    bContinue = BitMatGetNext(lpSrc, &lCol, &lRow);
  }
  */
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The BitMatCopyOffset() function copies non-zero bits from the
//  source bit matrix to an offset position in the the destination
//  matrix, provide the offset position is greater than the min
//  position.
//

BOOL MATAPI BitMatCopyOffset(LPBITMATRIX lpSrc, LPBITMATRIX lpDest,
                             long lColOffset, long lRowOffset,
                             long lColMin, long lRowMin)
{
  long lCol;
  long lRow;
  BOOL bContinue;

  bContinue = BitMatGetFirst(lpSrc, &lCol, &lRow);
  while( bContinue )
  {
    if( lCol + lColOffset >= lColMin && lRow + lRowOffset >= lRowMin )
      BitMatSet(lpDest, lCol + lColOffset, lRow + lRowOffset, TRUE);
    bContinue = BitMatGetNext(lpSrc, &lCol, &lRow);
  }
  return TRUE;
}

//--------------------------------------------------------------------
//
//  The BitMatHasInter() function determines if the two bit matrices
//  intersect (i.e. if they have any bits in common).
//

BOOL MATAPI BitMatHasInter(LPBITMATRIX lpMat1, LPBITMATRIX lpMat2)
{
  long lCol;
  long lRow;
#if defined(_WIN64) || defined(_IA64)
  LONG_PTR lValue;

  lValue = (LONG_PTR)MatGetFirst(lpMat1, &lCol, &lRow);
  while( lValue )
  {
    if( lValue & (LONG_PTR)MatGet(lpMat2, lCol, lRow) )
      return TRUE;
    else
      lValue = (LONG_PTR)MatGetNext(lpMat1, &lCol, &lRow);
  }
  return FALSE;
#else
  long lValue;

  lValue = (long)MatGetFirst(lpMat1, &lCol, &lRow);
  while( lValue )
  {
    if( lValue & (long)MatGet(lpMat2, lCol, lRow) )
      return TRUE;
    else
      lValue = (long)MatGetNext(lpMat1, &lCol, &lRow);
  }
  return FALSE;
#endif
}

//--------------------------------------------------------------------
//
//  The BitMatHasInterOffset() function determines if the first bit
//  matrix shifted by the given offset intersects the second bit
//  matrix.
//

BOOL MATAPI BitMatHasInterOffset(LPBITMATRIX lpMat1, LPBITMATRIX lpMat2,
                                  long lColOffset, long lRowOffset)
{
  long lCol;
  long lRow;
  BOOL bValue;

  bValue = BitMatGetFirst(lpMat1, &lCol, &lRow);
  while( bValue )
  {
    if( BitMatGet(lpMat2, lCol+lColOffset, lRow+lRowOffset) )
      return TRUE;
    else
      bValue = BitMatGetNext(lpMat1, &lCol, &lRow);
  }
  return FALSE;
}
