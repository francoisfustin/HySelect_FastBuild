//
// FarPoint VBX Entry Point
//
// Implement VBINITCC as follows:
//
// BOOL DLLENTRY VBINITCC(USHORT usVersion, BOOL fRuntime)
// { 
//    if (!fRuntime)
//       {
//       if (!fpCheckRuntimeLicense("FPTEXT.VBX", "FPTEXT.LIC"))
//          return (FALSE);
//       }
//    return (fpRegisterLibClass(fpInstance));
// }
//

#include "fptools.h"

#ifndef FP_OCX
      
//
// VBX WEP
//
int FAR PASCAL WEP (int bSystemExit)
{
//   fpInstance  = NULL;
   return (1);
}

//
// VBX LibMain
//
int CALLBACK LibMain(HANDLE hInst, WORD wDataSeg, WORD wHeapSize, 
   LPSTR lpCmdLine)
{                  
#ifdef _DEBUG   
   //DebugBreak();
#endif

   if (fpInstance)
      return TRUE;

   fpInstance = hInst;
   
#ifndef WIN32
   if (wHeapSize != 0)     // Let data segment move
      UnlockData(0);
#endif

   return (TRUE);
} 
#endif // !FP_OCX