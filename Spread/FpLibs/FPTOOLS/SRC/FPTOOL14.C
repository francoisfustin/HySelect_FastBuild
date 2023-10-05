/*    PortTool v2.2     FPTOOL14.C          */

#include "fptools.h"

/*
int DLLENTRY SysRead (int fileHandle, LPVOID buffer, unsigned bytesToRead, LPWORD bytesRead)
{                      
   extern void FAR PASCAL DOS3Call();
   __asm
   {
      mov   ah, 3fh
      mov   bx, fileHandle
      mov   cx, bytesToRead
      lds   dx, buffer
      call  DOS3Call
      jc    SYSReadErr
      lds   bx, bytesRead
      mov   word ptr [bx], ax ; save the count
      xor   ax, ax
   SYSReadErr: 
   }
   return *bytesRead;
}
*/

#if defined(_WIN64) || defined(_IA64)
int DLLENTRY fpSysRead (LONG_PTR fileHandle, LPVOID buffer, unsigned bytesToRead, LPWORD bytesRead)
#else
int DLLENTRY SysRead (int fileHandle, LPVOID buffer, unsigned bytesToRead, LPWORD bytesRead)
#endif
{
  DWORD dwError;    
#if WIN32
  DWORD dwBytesRead;
  if( ReadFile((HANDLE)fileHandle, buffer, (DWORD)bytesToRead, &dwBytesRead, NULL) )
    *bytesRead = (WORD)dwBytesRead;
  else
    dwError = GetLastError();
#else
   UINT uRet;

   uRet = _lread( fileHandle, (void huge *)buffer, bytesToRead);
   
   if (uRet != HFILE_ERROR)
     *bytesRead = (WORD)uRet;
  else
     dwError = GetLastError();
#endif
     
   return *bytesRead;
}
