//
// FarPoint VBX Entry Point
//
//

#include "fptools.h"
#include <tchar.h>

PROPINFO PropertyNULL =
   {
   _T("NULL"),
   DT_SHORT | PF_fNoShow,
   0, 0, 0, NULL, 0
   };


static DWORD fpCheckSum (LPBYTE lpsz, int n)
{
   DWORD sum = 0;
   int i;
   
   for (i=0; i<n && lpsz[i] != (char)26; i++)
      sum += ((BYTE)lpsz[i] >> 1) | (BYTE)((lpsz[i] & 0x01) ? 0x80 : 0x00);
   if ((i+5) == n)
      {
      LONG lValue = (LONG)MAKELONG(MAKEWORD(lpsz[i+4],lpsz[i+3]),MAKEWORD(lpsz[i+2],lpsz[i+1]));   
      sum += lValue;
      }
      
   return sum;
}

//
// Check Runtime License File
//
// CheckSum algorithm does a circular rotation of the bits
// to the right before adding the byte to the sum.  Assume
// the total file size is less than 2K.
//
// if bCreateLicense = TRUE, farpoint.lic will be created in the default directory
// when the existing license file is not valid.
//
void FPLIB fpDisplayTimeoutMsg(LPTSTR lpszVBX);
BOOL FPLIB fpCheckRuntimeLicense (LPTSTR lpszVBX,
   LPTSTR lpszLIC, DWORD dwProductID, FPDATE fpTimeOut, BOOL bCreateLicense)
{
   OFSTRUCT  OFStruct;
   HFILE     hFile;
   HFILE     hOutput;
   BOOL      fStatus = FALSE;

#ifdef UNICODE  // if unicode, then convert to MBCS and store in lpszLIC
   char      szTemp[1000];
   WideCharToMultiByte(CP_ACP, 0, lpszLIC, -1, szTemp, sizeof(szTemp), NULL, NULL);
   lpszLIC = (LPTSTR)szTemp;  // make lpszLIC ptr to converted string.
#endif   

   hFile = OpenFile((LPSTR)lpszLIC, &OFStruct, OF_READ);          
      
   if (hFile == HFILE_ERROR)   
      fpDisplayRuntimeLicenseMsg(lpszVBX);
   else
      {
      GLOBALHANDLE hBuffer = GlobalAlloc (GHND, 2048);
      LPBYTE lpBuffer = GlobalLock (hBuffer);
      DWORD  dwCheckSum = 0;
      int n;
      
      if (bCreateLicense)
         hOutput = OpenFile("\\farpoint.lic", &OFStruct, OF_CREATE);
      
      n = _lread (hFile, lpBuffer, 2048);
      
      if (n > 2048 - 5) n = 2048 - 5;

      if (bCreateLicense)
          _lwrite (hOutput, lpBuffer, n);

      dwCheckSum = fpCheckSum (lpBuffer, n);
      
      _lclose (hFile);
      
      if (dwCheckSum == dwProductID)
         fStatus = TRUE;
      else
         {
         fpDisplayRuntimeLicenseMsg(lpszVBX);

         if (bCreateLicense)
         {
            LONG lValue;

            lValue = dwProductID - dwCheckSum;
         
            lpBuffer[0] = (char)26;
            lpBuffer[1] = (char)HIBYTE(HIWORD(lValue));
            lpBuffer[2] = (char)LOBYTE(HIWORD(lValue));
            lpBuffer[3] = (char)HIBYTE(LOWORD(lValue));
            lpBuffer[4] = (char)LOBYTE(LOWORD(lValue));
         
            _lwrite (hOutput, (LPSTR)lpBuffer, 5);
         }
        
         }

      GlobalUnlock (hBuffer); 
      GlobalFree (hBuffer);

      if (bCreateLicense)
         _lclose (hOutput);

      }
   
   if (fStatus && fpTimeOut != fpNODATE)
      {
      if (fpTodaysDate() > fpTimeOut)
         {
         fpDisplayTimeoutMsg(lpszVBX);
         fStatus = FALSE;
         }
      }

   return fStatus;
}

//
// Display MessageBox
//
void FPLIB fpDisplayRuntimeLicenseMsg(LPTSTR lpszVBX)
{
   LPTSTR lpszText = 
#ifdef FP_BOC      // Changed by BOC  -Masanori
      _T("カスタムコントロールのライセンスファイルが見つかりませ")
      _T("んので、デザイン環境では使用できません。\n\n正規ライセ")
      _T("ンスの購入については、文化オリエント(株)  パワーツール")
      _T("係(022-373-1460)へ連絡ください。");
#else
      _T("License file for custom control not found.  ")
      _T("You do not have an appropriate license to ")
      _T("use this custom control in the design ")
      _T("environment.\n\nCall FarPoint at:\n")
      //"     1-800-645-5913\n";
	  _T("     1-919-460-1887\n"); // scl
#endif

   MessageBox (0, lpszText, lpszVBX, MB_TASKMODAL | MB_OK | MB_ICONSTOP);
}              

void FPLIB fpDisplayTimeoutMsg(LPTSTR lpszVBX)
{
   LPTSTR lpszText =
#ifdef TAB_BOC      // Changed by BOC  -Masanori
      _T("ベータテスト期間がすでに過ぎています。\n\n正規ライセンスの")
      _T("購入については、文化オリエント(株)  パワーツール係")
      _T("(022-373-1460)へ連絡ください。");
#else
      _T("Test version design-time timeout.  ")
      _T("You must obtain the final release version ")
      _T("of this custom control to continue to use ")
      _T("it in the design environment.\n\n")
      _T("Call FarPoint at:\n")
      //"     1-800-645-5913\n";
	  _T("     1-919-460-1887\n"); // scl
#endif

   MessageBox (0, lpszText, lpszVBX, MB_TASKMODAL | MB_OK | MB_ICONSTOP);
}              
