/************
* SS_SORT.C
************/

#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include <toolbox.h>

#define MIN_PART_CNT  27
#define MAX_STACKSIZE 50

static void NEAR PASCAL QSortSwapHuge(HPBYTE one, HPBYTE two, register WORD w);
static LONG             QSortPartitionHuge(HPBYTE lpBase, LPLONG lplLow,
                                           LPLONG lplHigh, short wWidth,
                                           long lUserData,
                                           int (FAR PASCAL *lpCompare)(HPVOID,
                                           HPVOID, long));
static BOOL             QSortStackPushHuge(LONG lNewIndex, LPLONG lplLow,
                                           LPLONG lplHigh, LPLONG lpStack,
                                           LPSHORT lpdStackIndex);
static void             InsertionSortHuge(HPBYTE lpBase, LONG lNum,
                                          short wWidth, LPBYTE lpBuffer,
                                          long lUserData,
                                          int (FAR PASCAL *lpCompare)(HPVOID,
                                          HPVOID, long));


static void NEAR PASCAL QSortSwapHuge(HPBYTE one, HPBYTE two, register WORD w)
{
register int Temp;

while (w--)
   {
   Temp = (BYTE)*one;      // get the first byte
   *one++ = *two;          // put byte from element two in element one
   *two++ = (BYTE)Temp;    // store the save byte
   }
}


void DLLENTRY QuickSortHuge(HPBYTE lpBase, LONG lNum, WORD wWidth, long lUserData,
                           int (FAR PASCAL *lpCompare)(HPVOID, HPVOID, long))
{
GLOBALHANDLE hBuffer;
LPBYTE       lpBuffer;
LONG         lHigh;
LONG         lLow;
LONG         lHighNew;
short        dStackIndex;
LONG         Stack[MAX_STACKSIZE];

if (!(hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, wWidth)))
   return;

lpBuffer = (LPBYTE)GlobalLock(hBuffer);

if (lNum <= MIN_PART_CNT)
   {
   if (lNum > 1)
      InsertionSortHuge(lpBase, lNum, wWidth, lpBuffer, lUserData, lpCompare);

   GlobalUnlock(hBuffer);
   GlobalFree(hBuffer);
   return;
   }

lHigh = lNum - 1;
lLow = 0;
dStackIndex = 0;

while (dStackIndex >= 0)
   {
   lHighNew = QSortPartitionHuge(lpBase, &lLow, &lHigh, wWidth, lUserData,
                           lpCompare);

   if (lHighNew > lLow)
      {
      if (!QSortStackPushHuge(lHighNew, &lLow, &lHigh, Stack, &dStackIndex))
         {
         GlobalUnlock(hBuffer);
         GlobalFree(hBuffer);
         return;
         }
      }
   else
      lHigh = lLow;

   if ((lHigh - lLow) < MIN_PART_CNT)
      {
      dStackIndex -= 2;
      lLow = Stack[dStackIndex];
      lHigh = Stack[dStackIndex + 1];
      }
   }

InsertionSortHuge(lpBase, lNum, wWidth, lpBuffer, lUserData, lpCompare);
GlobalUnlock(hBuffer);
GlobalFree(hBuffer);
}


static LONG QSortPartitionHuge(HPBYTE lpBase, LPLONG lplLow, LPLONG lplHigh,
                         short wWidth, long lUserData,
                         int (FAR PASCAL *lpCompare)(HPVOID, HPVOID, long))
{
register LONG lHighNew;
register LONG lLowNew;
LONG          lPartitionIndex;

lPartitionIndex = (*lplHigh - *lplLow) / 4;

for (lLowNew = *lplLow; lLowNew < *lplHigh; lLowNew += lPartitionIndex)
   for (lHighNew = lLowNew + lPartitionIndex; lHighNew <= *lplHigh;
        lHighNew += lPartitionIndex)
      if ((*lpCompare)(&lpBase[lLowNew * wWidth],
          &lpBase[lHighNew * wWidth], lUserData) > 0)
         QSortSwapHuge(&lpBase[lLowNew * wWidth], &lpBase[lHighNew * wWidth],
                     wWidth);

//lPartitionIndex = *lplLow + (2 * lPartitionIndex);
lPartitionIndex = (*lplLow + (2 * lPartitionIndex)) * wWidth;
//lPartitionIndex = *lplLow * wWidth;
//lPartitionIndex = (*lplLow + ((*lplHigh - *lplLow) / 2)) * wWidth;

for (lHighNew = *lplHigh; lHighNew > 0; lHighNew--)
   if ((*lpCompare)(&lpBase[lHighNew * wWidth],
//       &lpBase[lPartitionIndex * wWidth], lUserData) <= 0)
       &lpBase[lPartitionIndex], lUserData) <= 0)
      break;

 for (lLowNew = *lplLow; lLowNew < lHighNew; lLowNew++)
   if ((*lpCompare)(&lpBase[lLowNew * wWidth],
//       &lpBase[lPartitionIndex * wWidth], lUserData) >= 0)
       &lpBase[lPartitionIndex], lUserData) >= 0)
      break;

while (lHighNew > lLowNew)
   {
//Modify by BOC 99.8.19 (hyt)------------------------------------
//for ensure not swap when  have same value
	if ((*lpCompare)(&lpBase[lHighNew * wWidth],
          &lpBase[lLowNew * wWidth], lUserData) != 0)
//---------------------------------------------------------------
		QSortSwapHuge(&lpBase[lHighNew * wWidth], &lpBase[lLowNew * wWidth], wWidth);

   for (lHighNew--; lHighNew > 0; lHighNew--)
      if ((*lpCompare)(&lpBase[lHighNew * wWidth],
//          &lpBase[lPartitionIndex * wWidth], lUserData) <= 0)
          &lpBase[lPartitionIndex], lUserData) <= 0)
         break;

   for (lLowNew++; lLowNew < lHighNew; lLowNew++)
      if ((*lpCompare)(&lpBase[lLowNew * wWidth],
//          &lpBase[lPartitionIndex * wWidth], lUserData) >= 0)
          &lpBase[lPartitionIndex], lUserData) >= 0)
         break;
   }

//Modify by BOC 99.8.19 (hyt)------------------------------------
//for ensure not swap when  have same value
	if ((*lpCompare)(&lpBase[lHighNew * wWidth],
          &lpBase[lLowNew * wWidth], lUserData) != 0)
//---------------------------------------------------------------
	QSortSwapHuge(&lpBase[lHighNew * wWidth], &lpBase[*lplLow * wWidth], wWidth);

return (lHighNew);
}

static BOOL QSortStackPushHuge(LONG lNewIndex, LPLONG lplLow, LPLONG lplHigh,
                               LPLONG lpStack, LPSHORT lpdStackIndex)
{
if ((lNewIndex - *lplLow) > (*lplHigh - lNewIndex))
   {
   if ((lNewIndex - *lplLow) >= MIN_PART_CNT)
      {
      if (*lpdStackIndex >= MAX_STACKSIZE - 1)
         return (FALSE);

      lpStack[*lpdStackIndex] = *lplLow;
      lpStack[*lpdStackIndex + 1] = lNewIndex - 1;
      *lpdStackIndex += 2;
      }

   *lplLow = (lNewIndex < *lplHigh) ? lNewIndex + 1 : *lplHigh;
   }

else
   {
   if ((*lplHigh - lNewIndex) > MIN_PART_CNT)
      {
      if (*lpdStackIndex >= MAX_STACKSIZE - 1)
         return (FALSE);

      lpStack[*lpdStackIndex] = lNewIndex + 1;
      lpStack[*lpdStackIndex + 1] = *lplHigh - 1;
      *lpdStackIndex += 2;
      }

   *lplHigh = (lNewIndex > *lplLow) ? lNewIndex - 1 : *lplLow;
   }

return (TRUE);
}


static void InsertionSortHuge(HPBYTE lpBase, long lNum, short wWidth,
                             LPBYTE lpBuffer, long lUserData,
                             int (FAR PASCAL *lpCompare)(HPVOID, HPVOID, long))
{
HPBYTE CurrentPtr;
HPBYTE InsPtr;

for (CurrentPtr = lpBase + wWidth; CurrentPtr < lpBase + (lNum * wWidth);
     CurrentPtr += wWidth)
   {
   InsPtr = CurrentPtr;
   MemHugeCpy(lpBuffer, CurrentPtr, wWidth);

   /********************************************************
   * Bubble the prior elements upwards so long as they are
   * out of place relative to the current array element
   ********************************************************/

   for (; (InsPtr > lpBase) && ((*lpCompare)(InsPtr - wWidth, lpBuffer,
        lUserData) > 0); InsPtr -= wWidth)
      QSortSwapHuge(InsPtr, InsPtr - wWidth, wWidth);

   if (InsPtr != CurrentPtr)
      QSortSwapHuge(InsPtr, lpBuffer, wWidth);
   }
}
