/*
 * Private routines to all Libraries
 */

#include "errs.h"          // include the error ids file

typedef enum   tagERRORTYPE {ABORT, RETRY, FAIL} ERRORTYPE;

ERRORTYPE   DLLENTRY ErrorSetStatus    (int);
FARPROC     DLLENTRY ErrorReturnFunc   (void);
BOOL        DLLENTRY ErrorInstalled    (void);
WORD        DLLENTRY GetTaskDS(void);
void        DLLENTRY SetDS(WORD);
WORD        DLLENTRY GetDS(void);

#ifndef  DRV_MEM

#define  DRV_MEM

#if _WIN64
HGLOBAL  DLLENTRY tbMemAlloc    (HGLOBAL, DWORD_PTR, WORD);
#else
HGLOBAL  DLLENTRY tbMemAlloc    (HGLOBAL, DWORD, WORD);
#endif
BOOL     DLLENTRY tbMemFree     (HGLOBAL);
BOOL     DLLENTRY tbMemFreeLock (HGLOBAL);
BOOL     DLLENTRY tbMemFreeWire (HGLOBAL);
BOOL     DLLENTRY tbMemUnlock   (HGLOBAL);
BOOL     DLLENTRY tbMemUnWire   (HGLOBAL);
LPVOID   DLLENTRY tbMemLock     (HGLOBAL);
LPVOID   DLLENTRY tbMemWire     (HGLOBAL);

#endif

#ifndef  STR_OUTPUI
int      DLLENTRY _output     (LPSTR, LPSTR, LPSTR);
#endif
