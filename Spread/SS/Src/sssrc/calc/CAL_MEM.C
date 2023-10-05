//--------------------------------------------------------------------
//
//  File: cal_mem.c
//
//  Description: Memory allocation routines
//

#include "calc.h"
#include "cal_mem.h"

//--------------------------------------------------------------------
//
//  When SmartHeap memory management is used, the calcMemPool is the
//  memory pool used by the calc engine routines.
//

#ifdef CALC_USE_SMARTHEAP
  MEM_POOL calcMemPool = 0;
#endif

//--------------------------------------------------------------------
//
//  The CalcMemPoolInit() function initializes a SmartHeap memory
//  pool which is used by the calc engine routines.
//
//  Note: If SmartHeap memory management is being used, then
//        CalcMemPoolInit() must be called before any other calc
//        engine routine is used.
//

void CalcMemPoolInit()
{
  MatMemPoolInit();
  VectMemPoolInit();
#ifdef CALC_USE_SMARTHEAP
  if( !calcMemPool )
    calcMemPool = MemPoolInit(MEM_POOL_DEFAULT);
#endif
}

//--------------------------------------------------------------------
//
//  The CalcMemPoolFree() function frees the SmartHeap memory pool
//  which is being used by the calc engine routines.
//
//  Note: If SmartHeap memory management is being used, then no
//        calc engine routine can be used after a call to
//        CalcMemPoolFree().
//

void CalcMemPoolFree()
{
#ifdef CALC_USE_SMARTHEAP
  if( calcMemPool )
    MemPoolFree(calcMemPool);
  calcMemPool = 0;
#endif
  MatMemPoolFree();
  VectMemPoolFree();
}
