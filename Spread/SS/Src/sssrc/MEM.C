/*
$Revision:   1.4  $
*/


/*
 *		Memory handling routines for gen dynamic link library.
 *		These were writen to add a layer between Windows and Drover
 *		products to enable:
 *			Error trapping for messages.
 *			Drover specific programming methodologies.
 *
 *		Always use these in place of windows functions.
 */

#include <windows.h>
#include	<toolbox.h>
#include    "private.h"
#include	"libgen.h"

static char	_File_[] = __FILE__;		/* for errors */

/*
 *		allocate or reallocate memory
 *
 *		returns handle to memory or NULL for failure
 *		see Error() for handling failure
 */
HANDLE  DLLENTRY tbMemAlloc (hMem, dwBytes, wFlags)
HANDLE	hMem;       /* Handle to memory to reallocate or NULL for alloc */
#if defined(_WIN64) || defined(_IA64)
DWORD_PTR    dwBytes;    /* Total number of bytes to be allocated */
#else
DWORD    dwBytes;    /* Total number of bytes to be allocated */
#endif
WORD     wFlags;     /* Allocation Option Flags - as per GlobalAlloc */
{
#if defined(_WIN64) || defined(_IA64)
	DWORD_PTR	Start, End;
#else
	DWORD	Start, End;
#endif
	char	huge * p;

	if (hMem)
	{
		Start = GlobalSize (hMem);
		hMem = GlobalReAlloc (hMem, dwBytes, wFlags);
	}
	else
	{
		Start = 0l;
		hMem = GlobalAlloc (wFlags, dwBytes);
	}

	if (!hMem)
      return NULL;

	/* zero memory - to fix GMEM_ZERO bug */
	End = GlobalSize (hMem);
    if (End > Start && (p = tbMemLock (hMem)))
	{
		p += Start;
		while (Start++ < End)
			*p++ = '\0';
        tbMemUnlock (hMem);
	}

	return hMem;
}

/*
 *		unlock and free memory
 *
 *		returns FALSE for failure
 *		see Error() for handling failure
 */
BOOL        DLLENTRY tbMemFreeLock (hMem)
HANDLE	hMem;       /* Handle to memory to unlock and free or NULL */
{
	if (!hMem)
		return TRUE;

/* 	while (GlobalFlags (hMem) & GMEM_LOCKCOUNT) */ 
   tbMemUnlock (hMem);

    return tbMemFree (hMem);
}

/*
 *		free memory
 *
 *		returns FALSE for failure
 *		see Error() for handling failure
 */
BOOL        DLLENTRY tbMemFree (hMem)
HANDLE	hMem;       /* Handle to memory to free or NULL */
{
	if (!hMem)
		return TRUE;
#ifndef WIN32
   if (GetWinFlags() & WF_PMODE)
   {
      if (GlobalFree (hMem))
   		return FALSE;
   }
   else
#endif
   {
      if ((GlobalFlags(hMem) & GMEM_LOCKCOUNT) == 0)
      {
         if (GlobalFree (hMem))
      		return FALSE;
      }
      else
      {
         return(FALSE);
      }
   }

    return TRUE;
}

/*
 *		lock memory
 */
void        FAR * FAR PASCAL tbMemLock (hMem)
HANDLE	hMem;       /* Handle to memory to lock or NULL */
{
   void  FAR * lpPtr;

	if (!hMem)
		return NULL;

   lpPtr = GlobalLock (hMem);

   return lpPtr;
}

/*
 *		unlock memory
 */
BOOL        DLLENTRY tbMemUnlock (hMem)
HANDLE	hMem;       /* Handle to memory to unlock or NULL */
{
	if (!hMem)
		return TRUE;

   return GlobalUnlock (hMem);
}
