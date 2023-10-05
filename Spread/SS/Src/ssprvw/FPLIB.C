#include "windows.h"
#include "ssprvw.h"
#include "vbssprvw.h"

HANDLE fpInstance = NULL;
HANDLE taskDevEnvironment = NULL;

BOOL fpRegisterLibClass(HANDLE hInst);
LONG _export FAR PASCAL AboutBoxProc(HWND hWnd, UINT msg, WPARAM wParam,
                                     LPARAM lParam);

#define MACRO_STRING(v, m, s) LPSTR v = m s

//#ifdef FPVBX

BOOL DLLENTRY VBINITCC(USHORT usVersion, BOOL fRuntime)
{
   MACRO_STRING(vbxfile, FPVBX, ".VBX");
   if (!fRuntime)
      {
//      if (!fpCheckRuntimeLicense(vbxfile))
//         return (FALSE);
      }

   if (!fRuntime)
      {
      WNDCLASS class;

      class.style = 0;
      class.lpfnWndProc = AboutBoxProc;
      class.cbClsExtra = 0;
      class.cbWndExtra = 0;
      class.hInstance = fpInstance;
      class.hIcon = NULL;
      class.hCursor = NULL;
      class.hbrBackground = NULL;
      class.lpszMenuName = NULL;
      class.lpszClassName = CLASS_ABOUTBOX;

      if (! RegisterClass(&class))
         return FALSE;

      // Remember the task associated with the development environment
      taskDevEnvironment = GetCurrentTask();

      }

   return (fpRegisterLibClass(fpInstance));
}

//---------------------------------------------------------------------------
// Unregister the property popup , if this unload
// is from the development environment.
//---------------------------------------------------------------------------

VOID FAR PASCAL _export VBTERMCC()
{
// Unregister popup class if this is from the development environment
if (taskDevEnvironment == GetCurrentTask())
   {
   UnregisterClass(CLASS_ABOUTBOX, fpInstance);
   taskDevEnvironment = NULL;
   }
}

//#endif


int CALLBACK LibMain(HANDLE hInst, WORD wDataSeg, WORD wHeapSize, 
   LPSTR lpCmdLine)
{
   if (fpInstance)
      return TRUE;

   fpInstance = hInst;
   
   if (wHeapSize != 0)     // Let data segment move
      UnlockData(0);

#ifdef SS_VBX
   return (TRUE);
#else
   return (fpRegisterLibClass(hInst));
#endif
}

                   
int FAR PASCAL WEP(int bSystemExit)
{
   fpInstance = NULL;
   return (1);
}

//
// Register Library Classes
// (required by FPLIB)
//
BOOL fpRegisterLibClass(HANDLE hInst)
{
#ifdef SS_VBX
   VBSSPreviewRegister(hInst);
#else
   SSPreviewRegister(hInst);
#endif

   return (TRUE);
}
