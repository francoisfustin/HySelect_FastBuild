/*    PortTool v2.2     FPTOOL62.C          */

// 
// FarPoint Tabstops Table Manager
//

#include "fptools.h"
#include "fpmemmgr.h"
#include "string.h"


//A419
BOOL FPLIB fpCreateTabHnd (LPFPTABSTOPS lpTabStops, LPFPTABS lpTabHnd)
{
   UINT totalLen; 
   LPFPTABSTOPS lpTmpTabStops;
         
   *lpTabHnd = 0;
   
   // Allocate size of Tab structure plus the data it will contain.
   totalLen = sizeof( FPTABSTOPS);
   if ((lpTabStops != NULL) && (lpTabStops->ct > 0))
     totalLen += lpTabStops->ct * sizeof(lpTabStops->lpStops[0]);

   // Allocate buffer memory 
   *lpTabHnd = (FPTABS)fpGlobalAlloc (GHND, totalLen);

   // If allocation failed, return FALSE
   if (*lpTabHnd == 0)
     return( FALSE);

   // if only struct was allocated, return TRUE
   if (totalLen == sizeof(FPTABSTOPS))  
     return( TRUE);

   lpTmpTabStops = fpLockTabStops(*lpTabHnd);
   if (lpTmpTabStops == NULL)
      {
	  fpReleaseTabHnd( *lpTabHnd);	    // couldn't copy data (not likely)
	  *lpTabHnd = 0;

	  return(FALSE);					 
	  }

   // Copy data to new structure
   lpTmpTabStops->ct = lpTabStops->ct;
   lpTmpTabStops->lpStops = 
      (void FAR *)((char FAR *)lpTmpTabStops + sizeof(FPTABSTOPS));

   _fmemcpy( lpTmpTabStops->lpStops,
             lpTabStops->lpStops, 
       	     totalLen - sizeof(FPTABSTOPS));

   fpUnlockTabStops(*lpTabHnd);

   return( TRUE);

}


void FPLIB fpReleaseTabHnd (FPTABS fpTabHnd)
{
   if (fpTabHnd)
	  fpGlobalFree( (FPGLOBALHANDLE)fpTabHnd);			 // free the memory for this handle
    
   return;  
}


LPFPTABSTOPS FPLIB fpLockTabStops (FPTABS fpTabHnd)
{
   // Simply lock the memory for this handle and return the ptr.
   if (fpTabHnd)
      return( (LPFPTABSTOPS)fpGlobalLock( (FPGLOBALHANDLE)fpTabHnd));
   else
      return( NULL);
}


void FPLIB fpUnlockTabStops (FPTABS fpTabHnd)
{
   if (fpTabHnd)
      fpGlobalUnlock( (FPGLOBALHANDLE)fpTabHnd);

   return;
}



// Convert the units of the Tabstops from Dialog to Pixels.
void FPLIB fpConvertTabsToPixels (FPTABS fpTabHnd, FPFONT fpFont)
{  
   UINT i;

   WORD dlgWidthUnits = LOWORD(GetDialogBaseUnits());
   LPFPTABSTOPS lpTabstops = fpLockTabStops(fpTabHnd);


   if ((lpTabstops == NULL) || (lpTabstops->ct == 0))
      return;

   // Convert each tabstop in the array from dialog units to pixels
   for (i = 0; i < lpTabstops->ct; i++)
      lpTabstops->lpStops[i] = 
         (lpTabstops->lpStops[i] * dlgWidthUnits) / 4;

   fpUnlockTabStops(fpTabHnd);

   return;
}

