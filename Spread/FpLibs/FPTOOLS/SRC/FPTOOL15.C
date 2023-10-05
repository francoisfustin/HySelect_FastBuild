/*    PortTool v2.2     FPTOOL15.C          */

//
// FarPoint Global Data Manager
//

#include "fptools.h"

#define GSHARE (GMEM_SHARE | GMEM_ZEROINIT | GMEM_MOVEABLE)

typedef struct fpGlobalData
{

   HINSTANCE hInstance;        // instance
   GLOBALHANDLE hData;         // data array (fuSize*fuN)
   UINT fuSize;                // size of each array element
   UINT fuUse;                 // number of objects in the instance
   UINT fuN;                   // number of elements
      
} FPGLOBALDATA, FAR *LPFPGLOBALDATA;


BOOL FPLIB fpCreateGlobalData (LPFPGLOBALDATAMANAGER lpDM, HINSTANCE hInstance, UINT fuSize)
{
    LPFPGLOBALDATA lpData;
    
    // See if global data already exists for this instance
    //
    if (lpDM->hData && (lpData = (LPFPGLOBALDATA)GlobalLock (lpDM->hData)))
       {
       BOOL fExists = FALSE;
       UINT i;
       for (i=0; i < lpDM->fuN; i++)
          if (lpData[i].hInstance == hInstance)
             {
             lpData[i].fuUse++;
			 // fix for ref-count problems in global data manager:
			 // re-init fuSize and increment lpDM->fuUse -scl
			 lpData[i].fuSize = fuSize;
             lpDM->fuUse++;
             fExists = TRUE;
             break;
             }
       GlobalUnlock(lpDM->hData);
       if (fExists) return TRUE;
       }
          
    // Expand Data Manager (add instance)
    //
    lpDM->fuUse++;
    if (lpDM->hData)
       lpDM->hData = GlobalReAlloc (lpDM->hData, sizeof(FPGLOBALDATA)*(lpDM->fuN+1), GMEM_ZEROINIT | GMEM_MOVEABLE); // GSHARE);
    else
	{
       lpDM->hData = GlobalAlloc (GSHARE, sizeof(FPGLOBALDATA));
#ifdef FP_APARTMENT // apartment model threading support -scl
	   InitializeCriticalSection(&lpDM->cs);
#endif // FP_APARTMENT
	}
    if (!lpDM->hData) return FALSE;
    
    // Initialize new instance
    lpData = (LPFPGLOBALDATA)GlobalLock (lpDM->hData);
    if (!lpData) return FALSE;

    lpData[lpDM->fuN].hInstance = hInstance;
    lpData[lpDM->fuN].fuSize = fuSize;
    lpData[lpDM->fuN].hData = NULL;
    lpData[lpDM->fuN].fuUse = 1;
    lpData[lpDM->fuN].fuN = 0;
    
    GlobalUnlock (lpDM->hData);
    lpDM->fuN++;
    
    return TRUE;
}

BOOL FPLIB fpDestroyGlobalData (LPFPGLOBALDATAMANAGER lpDM, HINSTANCE hInstance, void (*fpDestroyData)(GLOBALHANDLE, UINT))
{
    LPFPGLOBALDATA lpData;
    UINT i;
    
    if (!lpDM->hData) return FALSE;
    
    // Find instance and remove it
    //
    lpData = (LPFPGLOBALDATA)GlobalLock (lpDM->hData);
    if (!lpData) return FALSE;

    for (i=0; i<lpDM->fuN; i++)
       if (lpData[i].hInstance == hInstance)
          {
          if (--lpData[i].fuUse == 0)
             {
             lpData[i].hInstance = NULL;
             if (lpData[i].hData)
                {
                if (fpDestroyData) fpDestroyData(lpData[i].hData, lpData[i].fuN);
                GlobalFree(lpData[i].hData);
                }
             lpData[i].hData = NULL;
             lpData[i].fuSize = 0;
             lpData[i].fuN = 0;
//             lpDM->fuUse--;	// not right -scl
             }
          lpDM->fuUse--;	// need to release ref every time, not just when table refs are 0
          break;			// (see code above in fpCreateGlobalData) -scl
          }
          
    GlobalUnlock (lpDM->hData);
    
    // if last instance, cleanup
    // note: currently no reuse of unused instances
    if (lpDM->fuUse == 0)
       {
       if (lpDM->hData)
          GlobalFree (lpDM->hData);
       lpDM->hData = NULL;
       lpDM->fuN = 0;
#ifdef FP_APARTMENT // apartment model support -scl
		DeleteCriticalSection(&lpDM->cs);
#endif
       }
       
    return TRUE;
}

LPVOID FPLIB fpLockGlobalData (LPFPGLOBALDATAMANAGER lpDM, HINSTANCE hInstance, UINT FAR *lpfuN)
{
    GLOBALHANDLE hGlobal = NULL;
    LPFPGLOBALDATA lpData;
    UINT i;
    
    if (!lpDM->hData) return NULL;
    
#ifdef FP_APARTMENT
	EnterCriticalSection(&lpDM->cs);
#endif
    // Find instance
    //
    lpData = (LPFPGLOBALDATA)GlobalLock (lpDM->hData);
    if (!lpData)
       {
#ifdef FP_APARTMENT
       LeaveCriticalSection(&lpDM->cs);
#endif
       return NULL;
       }

    for (i=0; i<lpDM->fuN; i++)
       if (lpData[i].hInstance == hInstance)
          {
          if (lpData[i].hData)
             {
             // resize array if necessary
             // note: array size will grow but not shrink until removed
             if (lpData[i].fuN < *lpfuN)
                {
                lpData[i].hData = GlobalReAlloc (lpData[i].hData, ((DWORD)lpData[i].fuSize)*((DWORD)(*lpfuN)), GMEM_ZEROINIT | GMEM_MOVEABLE); // GSHARE);
                lpData[i].fuN = *lpfuN;
                }
             else
                *lpfuN = lpData[i].fuN;
             }
          else if (*lpfuN)
             {
             lpData[i].hData = GlobalAlloc (GSHARE, ((DWORD)lpData[i].fuSize)*((DWORD)(*lpfuN)));
             lpData[i].fuN = (*lpfuN);
             }
          
          hGlobal = lpData[i].hData;
          break;
          }
          
    GlobalUnlock (lpDM->hData);

#ifdef FP_APARTMENT
    if (!hGlobal)
       LeaveCriticalSection(&lpDM->cs);
#endif
    
    return (hGlobal ? GlobalLock(hGlobal) : NULL);
}

void FPLIB fpUnlockGlobalData (LPFPGLOBALDATAMANAGER lpDM, HINSTANCE hInstance)
{
    GLOBALHANDLE hGlobal = NULL;
    LPFPGLOBALDATA lpData;
    UINT i;
    
    if (!lpDM->hData) return;

    // Find instance
    //
    lpData = (LPFPGLOBALDATA)GlobalLock (lpDM->hData);
    if (!lpData) return;

    for (i=0; i<lpDM->fuN; i++)
       if (lpData[i].hInstance == hInstance)
          {
          hGlobal = lpData[i].hData;
          break;
          }
          
    GlobalUnlock (lpDM->hData);
    if (hGlobal)
       GlobalUnlock (hGlobal);

#ifdef FP_APARTMENT
	LeaveCriticalSection(&lpDM->cs);
#endif
}
