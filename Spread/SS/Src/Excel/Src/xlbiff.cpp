/*********************************************************
* XLbiff.cpp
*
* Copyright (C) 1999 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
* UPDATE LOG:
* -----------
*	RAP	- 11.19.98
* RAP01 - The Spread backcolor was not being displayed on import.     12.3.98
* RAP02 - Use the fls element of XF to determine whether to use the   12.3.98
*         color value in xf.backcolor.
* RAP03 - TIB5594-Importing XL file in Win95 results in no text color 07.21.99
* RAP04 - GIC9513 - Add support for LABEL record                      09.13.99
* RAP05 - GIC10743 - fonts converted with 0 length.                   10.11.99
* RAP06
* RAP07 - SEL5033
* RAP08 - 8683 - problem w/ CONTINUE & Ext bit.                       5.8.2001
* RAP09 - 9007 - col width not correct when applied from SD           06.06.01
* RAP10 - 9830 - problem w/ SST/CONTINUE.                             06.18.01 
* RAP11 - 9242 - problem w/ CONTINUE                                  08.17.01
* RAP12 - 9287 - problem w/ CONTINUE                                  09.11.01
* RAP13 - default row height was being set incorrectly because it was being set
*         prior to the default font being set.                        10.11.01
* RAP14 - 9633                                                        01.25.02  
*********************************************************/

#if defined(_WIN64) || defined(_IA64)
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "tchar.h"
#include "ctype.h"
#include "io.h"
#include "math.h"
#include <stdio.h>
#include "windows.h"

#include "spread.h"
#include "ssdll.h"
#include "xl.h"
#include "xlbiff.h"
#include "xlutl.h"
#include "biff.h"
#include "ss_alloc.h"
#include "ss_main.h"
#include <mbctype.h>    //for support DBCS
#ifdef SS_V40
#include "ss_span.h"
#endif

#ifdef SS_V70
  #define V7_TEST
#endif

#define PIXELS_TO_INTPT(Pixels) MulDiv(Pixels, 72, dyPixelsPerInch)

extern long ssm_BuildBIFFStr(LPTSTR lptstr, LPTBGLOBALHANDLE lpghBiffStr, BOOL bUnicode);

short ToFile(LPTSTR szString1, LPTSTR szString2);

BOOL bif_GetFONT(TBGLOBALHANDLE gh, WORD wIndex, LPxFONT lpfont);
BOOL bif_GetFORMAT(LPSSXL lpssXL, LPSPREADSHEET lpSS, WORD wIndex, LPTSTR lptstr, LPWORD lpwType, LPFMT FAR *plpf);
void bif_TokenGoToSheet(LPSSXL lpss, LPBIFF lpBiff, short sSheetNum, LPSHORT lpsSheetCount, LPBOOL lpIsValidSubstream);
BOOL bif_ProcessSST(LPSSXL lpss);
BOOL bif_GetLastBiffStrInSST(LPSSXL lpss, DWORD *pdwLastBfstrOffset, BOOL *fSeamString);
BOOL bif_UnCompressCharStr(LPSSXL lpss, DWORD dwLastBfstrOffset, LPBIFF lpCont, WORD wByteLen, BOOL fSST);
#ifdef SS_V70
TBGLOBALHANDLE bif_GetSupBookName(LPSSXL lpss, int nSupBookIndex, bool fSheetName, int index);
#endif

extern "C" SS_FONTID SS_InitFont(LPSS_BOOK lpBook, HFONT hFont, BOOL fDeleteFont,
                                 BOOL fCreateFont, LPBOOL lpfDeleteFontObject);


#ifdef _DEBUG
//#define _PARSE2FILE
#endif

#ifdef  _PARSE2FILE
#define PARSEFILE _T("C:\\BIFF.OUT")
#define WriteOut(s1, s2) ToFile(s1, s2)
#define WriteOutW2(s, n) {TCHAR tc[20];ToFile(s, itoa(n,tc,10));}
#define WriteOutDW2(s, n) {TCHAR tc[20];ToFile(s, ltoa(n,tc,10));}
#define WriteOutHex2(s, n) {TCHAR tc[20];ToFile(s, itoa(n,tc,16));}
#define WriteOutGH2(s, gh) {LPTSTR lp=(LPTSTR)tbGlobalLock(gh);  \
                            ToFile(s,lp);              \
                            tbGlobalUnlock(gh);}
#ifndef _UNICODE
#define WriteOutGHWC2(s, gh) {TCHAR tc[256];                      \
                              LPCWSTR lp=(LPCWSTR)tbGlobalLock(gh); \
                              WideCharToMultiByte(CP_ACP,         \
                              WC_COMPOSITECHECK, lp,              \
                               -1, tc, 256, NULL, NULL);          \
                              ToFile(s,tc);                       \
                              tbGlobalUnlock(gh);}
#else
#define WriteOutGHWC2(s, gh) {TCHAR tc[256];                      \
                              LPTSTR lp=(LPTSTR)tbGlobalLock(gh); \
                              memcpy(tc, lp, 256*sizeof(TCHAR));  \
                              ToFile(s,tc);                       \
                              tbGlobalUnlock(gh);}
#endif //UNICODE
#else
#define WriteOut(s1, s2)
#define WriteOutW2(s, n)
#define WriteOutDW2(s, n)
#define WriteOutHex2(s, n)
#define WriteOutGH2(s, gh)
#define WriteOutGHWC2(s, gh)
#endif

/***********************************************************************
* Name:   bif_Init - Initialize the Excel conversion structure.
*
* Usage:  short bif_Init(LPSSXL lpss)
*            lpss - pointer to the structure to be initialized
*
* Return: short - Success or Failure
***********************************************************************/
short bif_Init(LPSSXL lpss)
{
  if (!lpss)
    return 0;

  memset(lpss, 0, sizeof(SSXL));
  
  lpss->bProtect = (BYTE)-1;

#ifdef _PARSE2FILE  
  FILE *fp = _tfopen(PARSEFILE, _T("w"));
  if (fp)
    fclose(fp);
#endif //_PARSE2FILE

return 0;
}

/***********************************************************************
* Name:   bif_Parse - Parse the provided buffer. Set some of the Spread
*                     properties and build the Excel conversion structure.
*                     If the buffer contains anincomplete" BIFF structure
*                     and/or data, return the length of the "incomplete"
*                     portion to the calling function. If sPass is 0, go 
*                     through the buffer, and retrieve the "global" workbook
*                     data. If sPass is anything other than 0, scan through
*                     the buffer until the correct sheet is found, then read
*                     the sheet information from the buffer.
*
* Usage:  short bif_Parse(LPBYTE lpBiffBuffer, LPSSXL lpss,
*                         unsigned long far *lplLen, LPSPREADSHEET lpSS,
*                         LPSHORT lpsSheetCount, int nSheetNum, short sPass)
*            lpBiffBuffer - the buffer to be parsed.
*            lpss - pointer to the Excel conversion structure
*            lplLen - pointer to the length of the buffer to be parsed. This
*                     value is used for both input & output.
*            lpSS - pointer to the Spread control
*            lpsSheetCount - pointer to the number of 
*            nSheetNum - the index of the sheet to open.
*            sPass - which time through the function is it.
*
* Return: short - Success or Failure
***********************************************************************/
short bif_Parse(LPBYTE lpBiffBuffer, LPSSXL lpss, unsigned long far *lplLen, LPSS_BOOK lpBook, int nSheetNum, LPSHORT lpsCurrentSheet, LPSHORT lpsSheetsLoadedCount, short sPass, LPBOOL lpIsValidSubstream, long lTotalLen)
{
  short  sRet = 0;
  long   lInc = 0;
  long   lPos = 0;
  BOOL   fProcessedAToken = FALSE;
//  BOOL   isEOF = FALSE;

#ifdef SS_V70
  if (nSheetNum != *lpsCurrentSheet) //13356
  {
    if (nSheetNum == -1)
     lpBook->nSheet = (short)min(lpBook->nSheetCnt-1, max(0, *lpsCurrentSheet-1));
  }
  else
  {
    if (lpBook->nSheet == -1) //14071
      lpBook->nSheet = 0;
  }
#else
  lpBook->nSheet = 0;
  lpsCurrentSheet;
  lpsSheetsLoadedCount;
#endif

  do
  {
    if ((unsigned long)(lPos+sizeof(BIFF)) > *lplLen)
    {
      if (!fProcessedAToken)
        sRet = 1;
      else
        sRet = 2;
      break;
    }
    lpBiffBuffer = (LPBYTE)lpBiffBuffer + lInc;
    lInc = ((LPBIFF)lpBiffBuffer)->datalen + sizeof(BIFF);
    if ((unsigned long)(lPos+lInc) > *lplLen)
    {
      if (!fProcessedAToken)
        sRet = 1;
      else
        sRet = 2;
      break;
    }
//    lPos += lInc;
    
    if (sPass == 0)
    {
      //retrieve the Workbook Global values & the Worksheet names
      sRet = bif_Tokenize1((LPBIFF)lpBiffBuffer, lpss, lpBook, *lpsCurrentSheet);
      if (sRet == 999) // FILEPASS -- The file is password protected.
      {                // get out now before she blows!!!
        return 999;
      }                  
      else if (sRet == EOF)
      {
      }
      else
        sRet = 0;
    }
    else
    {
#ifdef SS_V70
      if ((nSheetNum == -1 && *lpsCurrentSheet <= 0) || (nSheetNum != -1 && *lpsCurrentSheet-1 != nSheetNum))
#else
      if (*lpsCurrentSheet == 0 || *lpsCurrentSheet-1 != nSheetNum)
#endif
      {
        short         sSheetIndex = (short)(nSheetNum == -1?0:(short)nSheetNum);
        LPxBUNDLESHT8 lpBS = (LPxBUNDLESHT8)tbGlobalLock(lpss->ghBundleShts);

        if ((long)lpBS[*lpsCurrentSheet].dwBOFPos == lTotalLen)
          bif_TokenGoToSheet(lpss, (LPBIFF)lpBiffBuffer, sSheetIndex, lpsCurrentSheet, lpIsValidSubstream);
      }
      else
      {
        //retrieve the Worksheet specific values & load
        LPxBUNDLESHT8 lpBS = (LPxBUNDLESHT8)tbGlobalLock(lpss->ghBundleShts);
        sRet = bif_Tokenize2((LPBIFF)lpBiffBuffer, lpss, lpBook, *lpsCurrentSheet, lpIsValidSubstream);

//        if (*lpIsValidSubstream && sRet == BOF)
        if (sRet == BOF)
        {
          if ((long)(lpBS[lpBook->nSheet+1].dwBOFPos) == lTotalLen)
          {
            (*lpsCurrentSheet)++;
            if (nSheetNum == -1)
            {
              lpBook->nSheet = (short)(*lpsCurrentSheet-1);
              (*lpsSheetsLoadedCount)++;
            }
            else
              sRet = EOF;
          }
          else if (nSheetNum != -1 && (long)(lpBS[*lpsCurrentSheet].dwBOFPos) == lTotalLen)
          {
            sRet = EOF;
          }
        }
        else if (sRet == EOF)
        {
          if ((DWORD)(lpBook->nSheet+1) < lpss->dwBundleShtCount)
            sRet = 0;
        }

//        if (!*lpIsValidSubstream)
//        {
//          sRet = EOF;
//        }
      }
    }

    lPos += lInc;
    lTotalLen += lInc;

    if (sRet == 0)
      fProcessedAToken = TRUE;
   
  }
#ifdef SS_V70
  while (sRet != EOF);
#else
  while (sRet != EOF && sRet != BOF);
#endif
  //the unParsed portion of the buffer needs to be recycled...
  *lplLen = abs((long)(lPos - (*lplLen)));
   
  return sRet;
}

/***********************************************************************
* Name:   bif_TokenGoToSheet - Check to see if this BIFF structure is a 
*                              beginning of file(BOF) token.
*
* Usage:  void bif_TokenGoToSheet(LPBIFF lpBiff, short sSheetNum,
*                                 LPSHORT lpsSheetCount)
*            lpBiff - pointer to the current BIFF structure.
*            sSheetNum - the the sheet number to read.
*            lpsSheetCount - pointer to the current sheet
*
* Return: short - Success or Failure
***********************************************************************/
void bif_TokenGoToSheet(LPSSXL lpss, LPBIFF lpBiff, short sSheetNum, LPSHORT lpsSheetCount, LPBOOL lpIsValidSubstream)
{
  sSheetNum;
  switch(lpBiff->recnum)
  {
    case xlBOF_BIFF2:   // beginning of book stream - BIFF2
    case xlBOF_BIFF3:   // beginning of book stream - BIFF3
    case xlBOF_BIFF4:   // beginning of book stream - BIFF4
      (*lpsSheetCount)++;
      break;
    case xlBOF_BIFF5_7_8: // beginning of book stream - BIFF5/BIFF7
      memset((LPzBOF)&(lpss->bof), 0, sizeof(zBOF));
      lpss->bof.wVerNum = *(LPWORD)(LPOFFSET(lpBiff, 4));
      lpss->bof.wBuild = *(LPWORD)(LPOFFSET(lpBiff, 8));
      if (lpss->bof.wVerNum == 0x0600)//BIFF8
      {
        lpss->wVer = 8;    
        memcpy((LPzBOF)&(lpss->bof), LPOFFSET(lpBiff, 4), sizeof(zBOF));
        (*lpsSheetCount)++;
        if (lpss->bof.wSubStrType != 0x05 && lpss->bof.wSubStrType != 0x10)
          *lpIsValidSubstream = FALSE;
        else
          *lpIsValidSubstream = TRUE;
      }
      else //BIFF5 or BIFF7
      {
        lpss->wVer = 5;    
        memcpy((LPzBOF)&(lpss->bof), LPOFFSET(lpBiff, 4), sizeof(zBOF)-2*sizeof(DWORD));
      }
      
      break;
  }
}

/***********************************************************************
* Name:   bif_Tokenize1 - Load the BIFF data into the Excel conversion
*                         structure based on the type of BIFF structure
*                         that is passed in. This function is called to
*                         load the workbook "global" data.
*
* Usage:  short bif_Tokenize1(LPBIFF lpBiff, LPSSXL lpss, LPSPREADSHEET lpSS)
*            lpBiff - pointer to the current BIFF structure.
*            lpss - the Excel conversion structure.
*            lpSS - pointer to the Spread structure
*
* Return: short - Success or Failure
***********************************************************************/
short bif_Tokenize1(LPBIFF lpBiff, LPSSXL lpss, LPSS_BOOK lpBook, int nSheetNum)
{
  LPSPREADSHEET lpSS = NULL;

  if (lpBook != NULL)  
    lpSS = SS_BookLockSheetIndex(lpBook, 0);

  nSheetNum; // to make the compiler happy

  if ((lpBiff->recnum != xlCONTINUE) &&
      (lpss != NULL && lpss->bProcessDataType == (BYTE)PDT_PROCESSINGSST))
  {
    bif_ProcessSST(lpss);
  }
  switch(lpBiff->recnum)
  {
    case xl1904:
      lpss->w1904 = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("1904"), lpss->w1904);
    break;
    case xlBACKUP:
      lpss->wBackup = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("BACKUP"), lpss->wBackup);
    break;

    case xlBOF_BIFF2:   // beginning of book stream - BIFF2
    {
      lpss->wVer = 2;     
      memset((LPzBOF)&(lpss->bof), 0, sizeof(zBOF));
      WriteOut(_T("BOF_BIFF2"), NULL);
    }
    break;
    case xlBOF_BIFF3:   // beginning of book stream - BIFF3
    {
      lpss->wVer = 3;     
      memset((LPzBOF)&(lpss->bof), 0, sizeof(zBOF));
      WriteOut(_T("BOF_BIFF3"), NULL);
    }
    break;
    case xlBOF_BIFF4:   // beginning of book stream - BIFF4
    {
      lpss->wVer = 4;    
      memset((LPzBOF)&(lpss->bof), 0, sizeof(zBOF));
      WriteOut(_T("BOF_BIFF4"), NULL);
    }
    break;
    case xlBOF_BIFF5_7_8: // beginning of book stream - BIFF5/BIFF7
    {
      memset((LPzBOF)&(lpss->bof), 0, sizeof(zBOF));
      lpss->bof.wVerNum = *(LPWORD)(LPOFFSET(lpBiff, 4));
      lpss->bof.wBuild = *(LPWORD)(LPOFFSET(lpBiff, 8));
      if (lpss->bof.wVerNum == 0x0600)//BIFF8
      {
        memcpy((LPzBOF)&(lpss->bof), LPOFFSET(lpBiff, 4), sizeof(zBOF));
        lpss->wVer = 8;    
        
        WriteOut(_T("BOF_BIFF8"), NULL);
        return BOF;
      }
      else //BIFF5 or BIFF7
      {
        lpss->wVer = 5;    
        memcpy((LPzBOF)&(lpss->bof), LPOFFSET(lpBiff, 4), sizeof(zBOF)-2*sizeof(DWORD));
        WriteOut(_T("BOF_BIFF5_7"), NULL);
        return BOF;
      }
    }
    break;

    case xlBOOKBOOL:
      lpss->wBookBool = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("BOOKBOOL"), lpss->wBookBool);
    break;
    case xlBUNDLESHEET:
    {
      xBUNDLESHT8 sheet;
      
      memset((LPxBUNDLESHT8)&sheet, 0, sizeof(xBUNDLESHT8));

      memcpy((LPxBUNDLESHT8)&sheet, LPOFFSET(lpBiff, 4), sizeof(xBUNDLESHT8)-sizeof(TBGLOBALHANDLE));

      //Modify by BOC 99.4.7(hyt)-----------------------------------
      BOOL bUnicode = sheet.wSheetNameLen & 0xff00;
//      sheet.wSheetNameLen = bif_LoadDataBoc((TBGLOBALHANDLE FAR *)&sheet.ghSheetName, 
      short nlen = bif_LoadDataBoc((TBGLOBALHANDLE FAR *)&sheet.ghSheetName, 
                                   (WORD)(sheet.wSheetNameLen & 0xff), 
                                   (LPBYTE)LPOFFSET(lpBiff, 12), TRUE, FALSE, 0, bUnicode);
#ifdef UNICODE
      TBGLOBALHANDLE ghBSName = (TBGLOBALHANDLE)0;
      LPTSTR         lpszBSName = (LPTSTR)tbGlobalLock(sheet.ghSheetName);
      LPTSTR         lpszNewBSName = NULL;
      
      sheet.wSheetNameLen &= 0xFF;

      Alloc(&ghBSName, (sheet.wSheetNameLen+1)*sizeof(TCHAR));
      lpszNewBSName = (LPTSTR)tbGlobalLock(ghBSName);

      // Convert the single byte string to a Unicode string.
      int nStringLen = (short)MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpszBSName,
                                                  -1, lpszNewBSName, sheet.wSheetNameLen+1);
      tbGlobalUnlock(sheet.ghSheetName);
      tbGlobalUnlock(ghBSName);
      tbGlobalFree(sheet.ghSheetName);
      
      sheet.ghSheetName = ghBSName;
#else
      sheet.wSheetNameLen = nlen;
#endif
      
      bif_LoadData((TBGLOBALHANDLE FAR *)&lpss->ghBundleShts, 
                   (WORD)sizeof(xBUNDLESHT8), 
                   (LPBYTE)&sheet, TRUE, FALSE, lpss->dwBundleShtCount*sizeof(xBUNDLESHT8));
      
      lpss->dwBundleShtCount++;
      lpss->dwSubStreamCount++;

      WriteOutGH2(_T("BUNDLESHEET"), sheet.ghSheetName);
    }
    break;
    case xlCODENAME:
      lpss->bCodeNameLen = *(LPBYTE)LPOFFSET(lpBiff, 4);
      bif_LoadData((TBGLOBALHANDLE FAR *)&lpss->ghCodeName, 
                   (WORD)lpss->bCodeNameLen,
                   (LPBYTE)LPOFFSET(lpBiff, 7),
                   TRUE, FALSE, 0);  //!!!RAP - the docs say the offset is 5
      WriteOutGH2(_T("CODENAME"), lpss->ghCodeName);
    case xlCODEPAGE:
      lpss->wCodePage = *(LPOFFSET(lpBiff, 4));
      WriteOut(_T("CODEPAGE"), NULL);
    break;

    case xlCONTINUE:
    {
      WriteOut(_T("CONTINUE"), NULL);
      switch (lpss->bProcessDataType)
      {
        case PDT_PROCESSINGSST:
        {
/*RAP06d
          BOOL   fSkipNull = FALSE;
          LPBYTE lpData = NULL;
          Alloc(&lpss->ghSSTData, lpss->dwSSTDataLen + lpBiff->datalen);
          lpData = (LPBYTE)tbGlobalLock(lpss->ghSSTData);
          lpData = (LPBYTE)lpData + lpss->dwSSTDataLen;
          if (*((LPBYTE)lpBiff+sizeof(BIFF)) == 0)
            fSkipNull = TRUE;

#ifndef SPREAD_JPN
          // The BOC fix causes the resulting xls file to be corrupt.
          // 99.8.12 rap
          memcpy(lpData, (LPBYTE)lpBiff + sizeof(BIFF) + (fSkipNull?1:0), lpBiff->datalen-(fSkipNull?1:0));
//      		memcpy(lpData, (LPBYTE)lpBiff + sizeof(BIFF) + 1, lpBiff->datalen-(fSkipNull?1:0));
#else
		  //Modify by BOC 99.7.21 (hyt)----------------------------------
      //for offset error spread corrupt
      		memcpy(lpData, (LPBYTE)lpBiff + sizeof(BIFF) + 1, lpBiff->datalen-(fSkipNull?1:0));
      //  memcpy(lpData, (LPBYTE)lpBiff + sizeof(BIFF) + (fSkipNull?1:0), lpBiff->datalen-(fSkipNull?1:0));
  		//------------------------------------------------------------------
#endif
          lpss->dwSSTDataLen += lpBiff->datalen-(fSkipNull?1:0);

          tbGlobalUnlock(lpss->ghSSTData);                        
RAP06d */
       
//RAP06a >>

          //A CONTINUE record has been encountered and it belongs to the SST.
          LPBYTE    lpData = NULL;
          DWORD     dwLastBfstrOffset = 0;
          BOOL      fSeamString = FALSE;
          //Find the beginning of the last BIFFSTR in the existing SST to determine where the seam for the SST/CONTINUE
          //exists.  If it is within the "record data" (not the char string) then there is no grbit to mess with and the
          //concatenation of the 2 buffers is effortless.  However, is the seam occurs in the char string, then we need
          //to properly adjust either the SST's portion of the char string, or the CONTINUE's portion of the char string.

          //Get the Last BiffStr in the SST and determine if the seam occurs in the record data or the char string.
          bif_GetLastBiffStrInSST(lpss, &dwLastBfstrOffset, &fSeamString);

          if (!fSeamString) // easy merge
          {
            Alloc(&lpss->ghSSTData, lpss->dwSSTDataLen + lpBiff->datalen);
            lpData = (LPBYTE)tbGlobalLock(lpss->ghSSTData);
            lpData = (LPBYTE)lpData + lpss->dwSSTDataLen;
            memcpy(lpData, (LPBYTE)lpBiff + sizeof(BIFF), lpBiff->datalen);
            tbGlobalUnlock(lpss->ghSSTData);                        
            lpss->dwSSTDataLen += lpBiff->datalen;
          }
          else // pain in the butt
          {
            BOOL fUnCompress = TRUE;
            LPBIFFSTR lpbfstrLast = NULL;

            lpData = (LPBYTE)tbGlobalLock(lpss->ghSSTData);
            lpbfstrLast = (LPBIFFSTR)LPOFFSET(lpData, dwLastBfstrOffset);

            //Grab the grbit from the CONTINUE
            fUnCompress = (BOOL)*(BYTE *)LPOFFSET(lpBiff, sizeof(BIFF));

            //Determine if a fixup is needed
            if (lpbfstrLast->fHighByte && fUnCompress ||
                !lpbfstrLast->fHighByte && !fUnCompress)
            {
              //no fixup is needed, just strip off the grbit from the CONTINUE and concatenate
              tbGlobalUnlock(lpss->ghSSTData);                        
              Alloc(&lpss->ghSSTData, lpss->dwSSTDataLen + lpBiff->datalen-1);
              lpData = (LPBYTE)tbGlobalLock(lpss->ghSSTData);
              lpData = (LPBYTE)lpData + lpss->dwSSTDataLen;
              memcpy(lpData, (LPBYTE)lpBiff + sizeof(BIFF)+1/*grbit*/, lpBiff->datalen-1);
              tbGlobalUnlock(lpss->ghSSTData);                        
              lpss->dwSSTDataLen += lpBiff->datalen-1/*grbit*/;
            }
            else
            {
              WORD wSSTPartByteLen = (WORD)(lpss->dwSSTDataLen-dwLastBfstrOffset);
  
              if (lpbfstrLast->fHighByte && !fUnCompress) // we need to uncompress the CONTINUE portion of the char string
              {
                bif_UnCompressCharStr(lpss, dwLastBfstrOffset, lpBiff, wSSTPartByteLen, FALSE);
              }
              else if (!lpbfstrLast->fHighByte && fUnCompress) // we need to uncompress the SST portion of the char string
              {  			  
                bif_UnCompressCharStr(lpss, dwLastBfstrOffset, lpBiff, wSSTPartByteLen, TRUE);
              }
              else
              {
                //we're lost...
              }
            }	
          }
//RAP06a <<          
        }
        break;
      }          
    }
    break;
  
    case xlCOUNTRY:
    {
      memset((LPzCOUNTRY)&(lpss->country), 0, sizeof(zCOUNTRY));
      memcpy((LPzCOUNTRY)&(lpss->country), LPOFFSET(lpBiff, 4), sizeof(zCOUNTRY));
      WriteOutW2(_T("COUNTRY"), lpss->country.wiCountryDef);
    }
    break;
    case xlDSF:
      lpss->wDSF = *(LPOFFSET(lpBiff, 4));
      WriteOut(_T("DSF"), NULL);
    break;
    case xlEOF_BIFF:
      if (lpss->ghShrFmla != NULL)
      {
        tbGlobalFree(lpss->ghShrFmla);
        lpss->ghShrFmla = (TBGLOBALHANDLE)0;
        lpss->dwShrFmlaCount = 0;
      }
      return EOF;
    break;
    case xlEXTSST:
    {
//      DWORD dwSize = 0;
//      DWORD dwTotal = 0;
//      LPBYTE lpb = (LPBYTE)LPOFFSET(lpBiff, 6);

      memset(&(lpss->extsst), 0, sizeof(zEXTSST));
      memcpy(&(lpss->extsst), LPOFFSET(lpBiff, 4), sizeof(zEXTSST)-sizeof(TBGLOBALHANDLE));
    
      WriteOut(_T("EXTSST"), NULL);

/*
      for (w=0; w<lpss->extsst.wSSTinfCount; w++)
      {
        bif_LoadData((TBGLOBALHANDLE FAR *)&lpss->extsst.ghSSTinf, 
                     sizeof(ISSTINF), 
                     lpb, FALSE, FALSE, 
                     dwTotal);
        dwSize = sizeof(ISSTINF);
        dwTotal += dwSize;
        lpb = (LPBYTE)(lpb + dwSize);
      }
*/
    }
    break;
    case xlFILEPASS:
      lpss->wProtPass = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("FILEPASS"), lpss->wProtPass);
      return 999;
    break;
    case xlFONT:
    {
      xFONT font;
  
      memset((LPxFONT)&font, 0, sizeof(xFONT));
      memcpy((LPxFONT)&font, LPOFFSET(lpBiff, 4), sizeof(xFONT)-sizeof(TBGLOBALHANDLE));

#ifndef SPREAD_JPN
      font.bCharSet &= ~0x80; //15301
#endif
//RAP05a >>
      if (font.bFontNameLen == 0)
      {
        // Another undocumented treat from Excel... It appears that
        // if there are several fonts in the spread sheet with the
        // same fontname, then these fonts are saved sequentially
        // by Excel with the fontname not appearing until after the
        // last font of that fontname...
        font.fGetName = TRUE; // tell the next font with a name to let
                              // me use it...

        TCHAR acFontName[] = _T("Arial");

// Just so there is a font in case my assumption is wrong... Set the font
// to Arial.
#ifndef _UNICODE
        WCHAR acFontNameW[100];
        font.bFontNameLen = (BYTE)MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)acFontName,
                                                      lstrlen(acFontName),
                                                      (LPWSTR)acFontNameW, 100);

        bif_LoadData((TBGLOBALHANDLE FAR *)&font.ghFontName, 
                     (WORD)font.bFontNameLen, 
                     (LPBYTE)acFontNameW, TRUE, 
                     font.bStrType==0?FALSE:TRUE, 0);

#else
        font.bFontNameLen = lstrlen(acFontName);
        bif_LoadData((TBGLOBALHANDLE FAR *)&font.ghFontName, 
                     (WORD)font.bFontNameLen, 
                     (LPBYTE)acFontName, TRUE, 
                     font.bStrType==0?FALSE:TRUE, 0);
#endif

      }
//RAP05a <<
      else
      {
        int i;
        LPxFONT lpxfont = (LPxFONT)tbGlobalLock(lpss->ghFonts);  
        bif_LoadData((TBGLOBALHANDLE FAR *)&font.ghFontName, 
                     (WORD)font.bFontNameLen, 
                     (LPBYTE)LPOFFSET(lpBiff, 20), TRUE, 
                     font.bStrType==0?FALSE:TRUE, 0);

                
        for (i=0; i<(int)lpss->dwFontCount; i++)
        {
          if (lpxfont[i].fGetName)
          {
            lpxfont[i].bFontNameLen = font.bFontNameLen;
            bif_LoadData((TBGLOBALHANDLE FAR *)&lpxfont[i].ghFontName, 
                         (WORD)lpxfont[i].bFontNameLen, 
                         (LPBYTE)LPOFFSET(lpBiff, 20), TRUE, 
                         font.bStrType==0?FALSE:TRUE, 0);
          
            lpxfont[i].fGetName = FALSE;
          }
        }
        tbGlobalUnlock(lpss->ghFonts);        
      }
      bif_LoadData((TBGLOBALHANDLE FAR *)&lpss->ghFonts, 
                   (WORD)sizeof(xFONT), 
                   (LPBYTE)&font, FALSE, FALSE, lpss->dwFontCount*sizeof(xFONT));

      lpss->dwFontCount++;

      WriteOutGHWC2(_T("FONT"), font.ghFontName);
    }
    break;
    case xlFORMAT:
    {
      xFORMAT   format;
      LPBIFFSTR lpbfstr = (LPBIFFSTR)LPOFFSET(lpBiff, 6);
      short     nStringLen = 0;

      memset((LPxFORMAT)&format, 0, sizeof(xFORMAT));
      memcpy((LPxFORMAT)&format, LPOFFSET(lpBiff, 4), sizeof(xFORMAT)-sizeof(TBGLOBALHANDLE));

//------------
      if (lpbfstr->fRichSt)
      {
        //this is a rich string. strip out the format runs since
        //Rich Strings are not supported in this release
        LPBYTE    lpb = NULL;
        LPTSTR    lptstr = NULL;
        WORD      wRunCount = 0;

        lpbfstr->fRichSt = 0;
        lpbfstr->res2 = 0;
        lpb = (LPBYTE)((LPBYTE)lpbfstr + sizeof(BIFFSTR)-1);
        wRunCount = *(WORD *)lpb;
        lpb = (LPBYTE)(lpb + wRunCount*sizeof(WORD));
        lptstr = (LPTSTR)lpb;

        //Modify by BOC 99.4.7(hyt)--------------------------------------
        //UNICODE must convert before write to logfile
        //xl_LogFile(lpBook, LOG_RICHTEXTNOTSUPPORTED, 0, 0, (LPVOID)lptstr);

        // Check the HighByte to determine how the string was saved.
        if (lpbfstr->fHighByte)
        {
          LPBYTE lpb = NULL;
#ifndef _UNICODE
          // Convert the Unicode string to a single byte string.
          TBGLOBALHANDLE gh = 0;
          LPSTR  lpstr = NULL;

          //Modify by BOC 99.4.8(hyt)----------------------------
			    //for enough space for multibyte string
          //Alloc(&gh, (lpbfstr->wCharCount+1)*sizeof(CHAR));
          Alloc(&gh, (lpbfstr->wCharCount+1)*sizeof(WCHAR));
          lpstr = (LPSTR)tbGlobalLock(gh);
          //nStringLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, 
          //                                 (LPWSTR)lpbfstr->bString, lpbfstr->wCharCount,
          //                                 lpstr, (lpbfstr->wCharCount+1)*sizeof(CHAR),
          //                                 NULL, NULL);
          nStringLen = (short)WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, 
                                           (LPWSTR)lptstr, lpbfstr->wCharCount,
                                           lpstr, (lpbfstr->wCharCount+1)*sizeof(WCHAR),
                                           NULL, NULL);

          xl_LogFile(lpBook, LOG_RICHTEXTNOTSUPPORTED, 0, 0, (LPVOID)lpstr);

          Alloc(&format.ghFormat, sizeof(BIFFSTR)-1+nStringLen*sizeof(CHAR));

          lpb = (LPBYTE)tbGlobalLock(format.ghFormat);
          memcpy(lpb, lpbfstr, sizeof(BIFFSTR)-1);
          //add for change the string size on ANSI----
			    WORD* pwCharCount = (WORD*)lpb;
			    *pwCharCount = nStringLen;
			    //End BOC------------------------------
          memcpy(lpb+sizeof(BIFFSTR)-1, lpstr, nStringLen);

          tbGlobalUnlock(format.ghFormat);
          tbGlobalUnlock(gh);
          tbGlobalFree(gh);
#else //_UNICODE
          xl_LogFile(lpBook, LOG_RICHTEXTNOTSUPPORTED, 0, 0, (LPVOID)lptstr);
          // retrieve the biffstr from the stream
          Alloc(&format.ghFormat, sizeof(BIFFSTR)-1+lpbfstr->wCharCount*sizeof(WCHAR));
          lpb = (LPBYTE)tbGlobalLock(format.ghFormat);
          memcpy(lpb, lpbfstr, sizeof(BIFFSTR)-1+lpbfstr->wCharCount*sizeof(WCHAR));
          tbGlobalUnlock(format.ghFormat);
#endif
        }
        else // !HighByte
        {
          xl_LogFile(lpBook, LOG_RICHTEXTNOTSUPPORTED, 0, 0, (LPVOID)lptstr);

#ifdef _UNICODE
          // Convert the single byte string to a Unicode string.
          TBGLOBALHANDLE gh = 0;
          LPBYTE lpb = NULL;
          LPSTR  lpstr = NULL;
          LPWSTR lpwstr = NULL;
          LPWSTR lpwstrDest = NULL;

          Alloc(&gh, (lpbfstr->wCharCount+1)*sizeof(WCHAR));
          lpwstr = (LPWSTR)tbGlobalLock(gh);

//xxrap          nStringLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpbfstr->bString,
//                                           lpbfstr->wCharCount,
//                                           lpwstr, (lpbfstr->wCharCount+1)*sizeof(WCHAR));
          nStringLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpbfstr->bString,
                                           lpbfstr->wCharCount,
                                           lpwstr, (lpbfstr->wCharCount+1));
          
          Alloc(&format.ghFormat, sizeof(BIFFSTR)-1+nStringLen*sizeof(WCHAR));

          lpb = (LPBYTE)tbGlobalLock(format.ghFormat);
          memcpy(lpb, lpbfstr, sizeof(BIFFSTR)-1);
          memcpy(lpb+sizeof(BIFFSTR)-1, lpwstr, nStringLen*sizeof(WCHAR));

 
          tbGlobalUnlock(format.ghFormat);
          tbGlobalUnlock(gh);
          tbGlobalFree(gh);
#else //!_UNICODE
          // retrieve the biffstr from the stream
          Alloc(&format.ghFormat, sizeof(BIFFSTR)-1+lpbfstr->wCharCount*sizeof(CHAR));
          lpb = (LPBYTE)tbGlobalLock(format.ghFormat);
          memcpy(lpb, lpbfstr, sizeof(BIFFSTR)-1+lpbfstr->wCharCount*sizeof(CHAR));
          tbGlobalUnlock(format.ghFormat);
#endif
        }
      } //RichSt
      else
      { //!RichSt
        // Check the HighByte to determine how the string was saved.
        if (lpbfstr->fHighByte)
        {
          LPBYTE lpb = NULL;
#ifndef _UNICODE
          // Convert the Unicode string to a single byte string.
          TBGLOBALHANDLE gh = 0;
          LPSTR  lpstr = NULL;
          
          //Modify by BOC 99.4.8(hyt)----------------------------
		      //for enough space for multibyte string
          //Alloc(&gh, (lpbfstr->wCharCount+1)*sizeof(CHAR));
          Alloc(&gh, (lpbfstr->wCharCount+1)*sizeof(WCHAR));

          //Modify by BOC 99.4.8(hyt)----------------------------
		      //for enough space for multibyte string
          //Alloc(&gh, (lpbfstr->wCharCount+1)*sizeof(CHAR));
          Alloc(&gh, (lpbfstr->wCharCount+1)*sizeof(WCHAR));
          lpstr = (LPSTR)tbGlobalLock(gh);
          //nStringLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, 
          //                                 (LPWSTR)lpbfstr->bString, lpbfstr->wCharCount,
          //                                 lpstr, (lpbfstr->wCharCount+1)*sizeof(CHAR),
          //                                 NULL, NULL);
          nStringLen = (short)WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, 
                                           (LPWSTR)lpbfstr->bString, lpbfstr->wCharCount,
                                           lpstr, (lpbfstr->wCharCount+1)*sizeof(WCHAR),
                                           NULL, NULL);

          Alloc(&format.ghFormat, sizeof(BIFFSTR)-1+nStringLen*sizeof(CHAR));
          lpb = (LPBYTE)tbGlobalLock(format.ghFormat);
          memcpy(lpb, lpbfstr, sizeof(BIFFSTR)-1);
          //add for change the string size on ANSI----
			    WORD* pwCharCount = (WORD*)lpb;
			    *pwCharCount = nStringLen;
			    //End BOC---------------------------------------
          memcpy(lpb+sizeof(BIFFSTR)-1, lpstr, nStringLen*sizeof(CHAR));

          tbGlobalUnlock(format.ghFormat);
          tbGlobalUnlock(gh);
          tbGlobalFree(gh);

#else
          // retrieve the biffstr from the stream
          Alloc(&format.ghFormat, sizeof(BIFFSTR)-1+lpbfstr->wCharCount*sizeof(WCHAR));
          lpb = (LPBYTE)tbGlobalLock(format.ghFormat);
          memcpy(lpb, lpbfstr, sizeof(BIFFSTR)-1+lpbfstr->wCharCount*sizeof(WCHAR));
          tbGlobalUnlock(format.ghFormat);
#endif
        }
        else // !HighByte
        {
          LPBYTE lpb = NULL;
#ifdef _UNICODE
          // Convert the single byte string to a Unicode string.
          TBGLOBALHANDLE gh = 0;
          LPSTR  lpstr = NULL;
          LPWSTR lpwstr = NULL;
          LPWSTR lpwstrDest = NULL;

          Alloc(&gh, (lpbfstr->wCharCount+1)*sizeof(WCHAR));
          lpwstr = (LPWSTR)tbGlobalLock(gh);

//xxrap          nStringLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpbfstr->bString,
//                                           lpbfstr->wCharCount,
//                                           lpwstr, (lpbfstr->wCharCount+1)*sizeof(WCHAR));
          nStringLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpbfstr->bString,
                                           lpbfstr->wCharCount,
                                           lpwstr, (lpbfstr->wCharCount+1));
        
          Alloc(&format.ghFormat, sizeof(BIFFSTR)-1+nStringLen*sizeof(WCHAR));
          lpb = (LPBYTE)tbGlobalLock(format.ghFormat);
          memcpy(lpb, lpbfstr, sizeof(BIFFSTR)-1);
          memcpy(lpb+sizeof(BIFFSTR)-1, lpwstr, nStringLen*sizeof(WCHAR));

          tbGlobalUnlock(format.ghFormat);
          tbGlobalUnlock(gh);
          tbGlobalFree(gh);
#else //!_UNICODE
          // retrieve the biffstr from the stream
          Alloc(&(format.ghFormat), sizeof(BIFFSTR)-1+lpbfstr->wCharCount*sizeof(CHAR));
          lpb = (LPBYTE)tbGlobalLock(format.ghFormat);
          memcpy(lpb, lpbfstr, sizeof(BIFFSTR)-1+lpbfstr->wCharCount*sizeof(CHAR));
          tbGlobalUnlock(format.ghFormat);
#endif
        }

      } //!RichSt

      bif_LoadData((TBGLOBALHANDLE FAR *)&lpss->ghFormats, 
                   (WORD)sizeof(xFORMAT), 
                   (LPBYTE)&format, FALSE, FALSE, lpss->dwFormatCount*sizeof(xFORMAT));

      lpss->dwFormatCount++;

      WriteOutGH2(_T("FORMAT"), format.ghFormat);
    }
    break;
    case xlHIDEOBJ:
      lpss->wHideObj = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("HIDEOBJ"), lpss->wHideObj);
    break;
    case xlINDEX:
    {
      WriteOut(_T("INDEX"), NULL);
    }
    break;
    case xlINTERFACEEND:
      lpss->fInUISection = FALSE;
      WriteOut(_T("INTERFACEEND"), NULL);
    break;
    case xlINTERFACEHDR:
      lpss->fInUISection = TRUE;
      WriteOut(_T("INTERFACEHDR"), NULL);
    break;
    case xlMMS:
      WriteOut(_T("MMS"), NULL);
    break;
#ifdef SS_V80ROBBY
    case xlEXTERNNAME:
    {
      xEXTERNNAME name;
      BOOL       fWideCharName = FALSE;
      memset(&name, 0, sizeof(xEXTERNNAME));
      memcpy((LPxEXTERNNAME)&name, LPOFFSET(lpBiff, 4), sizeof(xEXTERNNAME)-(2*sizeof(TBGLOBALHANDLE))-sizeof(WORD));

      //the first byte in the string is 0x00-CHAR or 0x01-WCHAR indicating the name string format.
      if (*(LPBYTE)(LPOFFSET(lpBiff, 11)))
        fWideCharName = TRUE;
      bif_LoadData((TBGLOBALHANDLE FAR *)&name.ghName, 
                   (WORD)name.cch, 
                   (LPBYTE)LPOFFSET(lpBiff, 12), TRUE, fWideCharName, 0);

#ifdef UNICODE
      TBGLOBALHANDLE ghName = (TBGLOBALHANDLE)0;
      LPTSTR         lpszName = (LPTSTR)tbGlobalLock(name.ghName);
      LPTSTR         lpszNewName = NULL;
      
      Alloc(&ghName, (name.cch+1)*sizeof(TCHAR));
      lpszNewName = (LPTSTR)tbGlobalLock(ghName);

      // Convert the single byte string to a Unicode string.
      int nStringLen = (short)MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpszName,
                                                  -1, lpszNewName, name.cch+1);
      tbGlobalUnlock(name.ghName);
      tbGlobalUnlock(ghName);
      tbGlobalFree(name.ghName);
      
      name.ghName = ghName;
#endif
      memcpy(&name.cce, LPOFFSET(lpBiff, 12+name.cch*(fWideCharName?2:1)), sizeof(WORD));
      if (name.cce)
        bif_LoadData((TBGLOBALHANDLE FAR *)&name.ghDefinition, 
                     (WORD)name.cch, 
                     (LPBYTE)LPOFFSET(lpBiff, 12+name.cch*(fWideCharName?2:1)+sizeof(WORD)), TRUE, FALSE, 0);

      bif_LoadData((TBGLOBALHANDLE FAR *)&lpss->ghExternNames, 
                   (WORD)sizeof(xEXTERNNAME), 
                   (LPBYTE)&name, FALSE, FALSE, lpss->dwExternNameCount*sizeof(xEXTERNNAME));
      lpss->dwExternNameCount++;
      
      WriteOut(_T("EXTERNNAME"), NULL);
    }
      break;
#endif
    case xlNAME:
    case xlNAME2:
    {
      xNAME name;
      BOOL  fWideCharName = FALSE; 
      memset(&name, 0, sizeof(xNAME));
      memcpy((LPxNAME)&name, LPOFFSET(lpBiff, 4), sizeof(xNAME)-(6*sizeof(TBGLOBALHANDLE)));
//the first byte in the string is 0x00-CHAR or 0x01-WCHAR indicating the name string format.
      if (*(LPBYTE)(LPOFFSET(lpBiff, 18)))
        fWideCharName = TRUE;
      bif_LoadData((TBGLOBALHANDLE FAR *)&name.ghName, 
                   (WORD)name.cch, 
                   (LPBYTE)LPOFFSET(lpBiff, 19), TRUE, fWideCharName, 0);

#ifdef UNICODE
      TBGLOBALHANDLE ghName = (TBGLOBALHANDLE)0;
      LPTSTR         lpszName = (LPTSTR)tbGlobalLock(name.ghName);
      LPTSTR         lpszNewName = NULL;
      
      Alloc(&ghName, (name.cch+1)*sizeof(TCHAR));
      lpszNewName = (LPTSTR)tbGlobalLock(ghName);

      // Convert the single byte string to a Unicode string.
      int nStringLen = (short)MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpszName,
                                                  -1, lpszNewName, name.cch+1);
      tbGlobalUnlock(name.ghName);
      tbGlobalUnlock(ghName);
      tbGlobalFree(name.ghName);
      
      name.ghName = ghName;
#endif
      if (name.cce)
      {
        bif_LoadData((TBGLOBALHANDLE FAR *)&name.ghNameDef, 
                     (WORD)name.cce, 
                     (LPBYTE)LPOFFSET(lpBiff, 19+name.cch*(fWideCharName?2:1)), FALSE, FALSE, 0);
      }
      if (name.cchCustMenu)
        bif_LoadData((TBGLOBALHANDLE FAR *)&name.ghMenuText, 
                     (WORD)name.cchCustMenu, 
                     (LPBYTE)LPOFFSET(lpBiff, 19+name.cch*(fWideCharName?2:1)+name.cce), TRUE, FALSE, 0);
      if (name.cchDescription)
        bif_LoadData((TBGLOBALHANDLE FAR *)&name.ghDescription, 
                     (WORD)name.cchDescription, 
                     (LPBYTE)LPOFFSET(lpBiff, 19+name.cch*(fWideCharName?2:1)+name.cce+name.cchCustMenu), TRUE, FALSE, 0);
      if (name.cchHelptopic)
        bif_LoadData((TBGLOBALHANDLE FAR *)&name.ghHelptopic, 
                     (WORD)name.cchHelptopic, 
                     (LPBYTE)LPOFFSET(lpBiff, 19+name.cch*(fWideCharName?2:1)+name.cce+name.cchCustMenu+name.cchDescription), TRUE, FALSE, 0);
      if (name.cchStatustext)
        bif_LoadData((TBGLOBALHANDLE FAR *)&name.ghStatusBarText, 
                     (WORD)name.cchStatustext, 
                     (LPBYTE)LPOFFSET(lpBiff, 19+name.cch*(fWideCharName?2:1)+name.cce+name.cchCustMenu+name.cchDescription+name.cchHelptopic), TRUE, FALSE, 0);

      bif_LoadData((TBGLOBALHANDLE FAR *)&lpss->ghNames, 
                   (WORD)sizeof(xNAME), 
                   (LPBYTE)&name, FALSE, FALSE, lpss->dwNameCount*sizeof(xNAME));
      lpss->dwNameCount++;
      
      lpss->fNamesSet = FALSE;
//      if (name.ghName)
//      {
//        LPTSTR  lptstrName = NULL;
//        TCHAR  szFormula[5000];
//
//        memset(szFormula, 0, 5000*sizeof(TCHAR));
//        lptstrName = (LPTSTR)tbGlobalLock(name.ghName);
//        if (name.ghNameDef)
//        {
//          BYTE      bType = 0;
//          yyFORMULA f;
//
//          memset(&f, 0, sizeof(yyFORMULA));
//          f.wFormulaLen = name.cce;
//          f.ghFormula = name.ghNameDef;     
//          xl_ParseFormula(lpSS, lpss, (LPyyFORMULA)&f, szFormula, NULL, &bType, NULL);
//        }
//
//        BOOL ret = SSSetCustomName(lpBook->hWnd, lptstrName, szFormula);
//        tbGlobalUnlock(name.ghName);
//        if (name.ghNameDef)
//          tbGlobalUnlock(name.ghNameDef);
//      }

      WriteOut(_T("NAME"), NULL);
    }
    break;
    case xlOBJPROTECT:
      WriteOut(_T("OBJPROTECT"), NULL);
    break;
    case xlOBPROJ:
      WriteOut(_T("OBPROJ"), NULL);
    break;
    case xlPASSWORD:
      lpss->wPassword = *(LPOFFSET(lpBiff, 4));
      WriteOut(_T("PASSWORD"), NULL);
    break;
    case xlPRECISION:
      lpss->wPrecision = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("PRECISION"), lpss->wPrecision);
    break;
    case xlPROT4REV:
      lpss->wRevLock = *(LPOFFSET(lpBiff, 4));
      WriteOut(_T("PROT4REV"), NULL);
    break;
    case xlPROT4REVPASS:
      lpss->wRevPass = *(LPOFFSET(lpBiff, 4));
      WriteOut(_T("PROT4REVPASS"), NULL);
    break;
    case xlPROTECT:
      lpss->bProtect = *(LPOFFSET(lpBiff, 4)); //Protect Workbook
      WriteOutW2(_T("PROTECT-wkbklevel"), lpss->bProtect);
    break;
    case xlREFRESHALL:
      lpss->wRefreshAll = *(LPOFFSET(lpBiff, 4));
      WriteOut(_T("REFRESHALL"), NULL);
    break;
    case xlRSTRING:
      WriteOut(_T("RSTRING"), NULL);
    break;
    case xlSCENPROTECT:
      WriteOut(_T("SCENPROTECT"), NULL);
    break;

    case xlSST:
    {
      LPBYTE lpData = NULL;

      lpss->bProcessDataType = PDT_PROCESSINGSST;
      
      lpss->ghSSTData = (TBGLOBALHANDLE)0;
      lpss->dwSSTDataLen = 0;

      Alloc(&lpss->ghSSTData, lpBiff->datalen);
      lpData = (LPBYTE)tbGlobalLock(lpss->ghSSTData);
      lpss->dwSSTDataLen = lpBiff->datalen;
      memcpy(lpData, (LPBYTE)lpBiff + sizeof(BIFF), lpss->dwSSTDataLen);
//RAP06a >>
      memset(&(lpss->sst), 0, sizeof(zSST));
      memcpy(&(lpss->sst), lpData, sizeof(SST));
//RAP06a <<
      tbGlobalUnlock(lpss->ghSSTData);
    }
    break;
#ifdef SS_V70
    case xlEXTERNSHEET:
    {
      LPEXTERNSHEET lpExtSheet;
      LPEXTERNREF   lpExternRefs;
      int           pos = 0;
      int           i;
      
      Alloc(&lpss->ghEXTERNSHEETs, sizeof(EXTERNSHEET));
      lpExtSheet = (LPEXTERNSHEET)tbGlobalLock(lpss->ghEXTERNSHEETs);

      memcpy(&lpExtSheet->dwEXTERNREFCount, LPOFFSET(lpBiff, 4), sizeof(WORD));
      
      if (lpExtSheet->dwEXTERNREFCount > 0)
      {
        Alloc(&lpExtSheet->ghEXTERNREFs, (lpExtSheet->dwEXTERNREFCount)*sizeof(EXTERNREF));
        lpExternRefs = (LPEXTERNREF)tbGlobalLock(lpExtSheet->ghEXTERNREFs);

        pos = 4 + sizeof(WORD);
        for (i=0; i<lpExtSheet->dwEXTERNREFCount; i++)
        {
          WORD wSupBookIndex;
          WORD wFirstSheet;
          WORD wLastSheet;
        
          memcpy(&wSupBookIndex, LPOFFSET(lpBiff, pos), sizeof(WORD));
          pos += 2;
          memcpy(&wFirstSheet, LPOFFSET(lpBiff, pos), sizeof(WORD));
          pos += 2;
          memcpy(&wLastSheet, LPOFFSET(lpBiff, pos), sizeof(WORD));
          pos += 2;

          // retrieve the sheetnames from the SUPBOOK list
          lpExternRefs[i].fileName   = bif_GetSupBookName(lpss, wSupBookIndex, FALSE, 0);
          lpExternRefs[i].startSheet = bif_GetSupBookName(lpss, wSupBookIndex, TRUE, wFirstSheet);
          lpExternRefs[i].endSheet   = bif_GetSupBookName(lpss, wSupBookIndex, TRUE, wLastSheet);
        }
  
        lpss->dwEXTERNSHEETCount++;
        tbGlobalUnlock(lpss->ghEXTERNSHEETs);
      }
    }
    break;
    case xlSUPBOOK:
    {
      LPGLOBALHANDLE lpghSUPBOOK = NULL;

      Alloc(&lpss->ghSUPBOOKData, (lpss->dwSUPBOOKCount+1) * sizeof(TBGLOBALHANDLE));
      
      lpghSUPBOOK = (LPGLOBALHANDLE)tbGlobalLock(lpss->ghSUPBOOKData);

      bif_LoadData((TBGLOBALHANDLE FAR *)&lpghSUPBOOK[lpss->dwSUPBOOKCount], lpBiff->datalen, LPOFFSET(lpBiff, 4), 
                   FALSE, FALSE, 0);
      lpss->dwSUPBOOKCount++;
      tbGlobalUnlock(lpss->ghSUPBOOKData);
    }
    break;
#endif
    case xlSTYLE:
    {
      xSTYLE style;
      
      memset((LPxSTYLE)&style, 0, sizeof(xSTYLE));

      memcpy((LPxSTYLE)&style, LPOFFSET(lpBiff, 4), sizeof(xSTYLE)-sizeof(TBGLOBALHANDLE));

      if (!(style.wixfe & 0x8000))
      {
        //this is a user-defined style -- get the name...
        bif_LoadData((TBGLOBALHANDLE FAR *)&style.ghStyleName, 
                     (WORD)style.xstyle.userdef.bStyleNameLen, 
                     (LPBYTE)LPOFFSET(lpBiff, 7), TRUE, FALSE, 0);
        WriteOutGH2(_T("STYLE"), style.ghStyleName);
      }
      else
        WriteOut(_T("STYLE"), NULL);

      bif_LoadData((TBGLOBALHANDLE FAR *)&lpss->ghStyles, 
                   (WORD)sizeof(xSTYLE), 
                   (LPBYTE)&style, TRUE, FALSE, lpss->dwStyleCount*sizeof(xSTYLE));

      lpss->dwStyleCount++;

    }
    break;
    case xlTABID:
      WriteOut(_T("TABID"), NULL);
    break;
    case xlUSESELFS:
      lpss->wUsesElfs = *(LPOFFSET(lpBiff, 4));
      WriteOut(_T("USESELFS"), NULL);
    break;
    case xlWINDOW1:
      memcpy(&lpss->wb, (LPWORD)LPOFFSET(lpBiff, 4), lpBiff->datalen); 
      WriteOut(_T("WINDOW1-workbook"), NULL);
    break;
    case xlWINDOWPROTECT:
      lpss->bWdwProtect = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("WINDOWPROTECT"), lpss->bWdwProtect);
    break;
    case xlWRITEACCESS:
      lpss->bWriteAccessLen = *(LPBYTE)LPOFFSET(lpBiff, 4);
      bif_LoadData((TBGLOBALHANDLE FAR *)&lpss->ghWriteAccess, 
                   (WORD)lpss->bWriteAccessLen,
                   (LPBYTE)LPOFFSET(lpBiff, 7),
                   TRUE, FALSE, 0);  //!!!RAP - the docs say the offset is 5
      WriteOutGH2(_T("WRITEACCESS"), lpss->ghWriteAccess);
    break;
    case xlXF:
    case xlXF_OLD:
    {
      xXF xf;
      xXF8 xf8;
  
      memset((LPxXF8)&xf8, 0, sizeof(xXF8));

      if (lpss->wVer != 8)
      {
        memset((LPxXF)&xf, 0, sizeof(xXF));
        memcpy((LPxXF)&xf, LPOFFSET(lpBiff, 4), sizeof(xXF));
        //copy the XF fields to XF8
        xf8.wifnt = xf.wifnt;
        xf8.wifmt = xf.wifmt;
        xf8.fLocked = xf.fLocked;
        xf8.fHidden = xf.fHidden;
        xf8.fStyle = xf.fStyle;
        xf8.f123Pfx = xf.f123Pfx;
        xf8.ixfParent = xf.ixfParent;
        xf8.fAlign = xf.fAlign;
        xf8.fWrap = xf.fWrap;
        xf8.fVAlign = xf.fVAlign;
        xf8.fJustLast = xf.fJustLast;
        xf8.trot = xf.fTextOrient==1?255:xf.fTextOrient==2?180:xf.fTextOrient==3?90:0;
        xf8.fAtrNum = xf.fAtrNum;
        xf8.fAtrFnt = xf.fAtrFnt;
        xf8.fAtrAlc = xf.fAtrAlc;
        xf8.fAtrBdr = xf.fAtrBdr;
        xf8.fAtrPat = xf.fAtrPat;
        xf8.fAtrProt = xf.fAtrProt;
        xf8.dgLeft = xf.dgLeft;
        xf8.dgRight = xf.dgRight;
        xf8.dgTop = xf.dgTop;
        xf8.dgBottom = xf.dgBottom;
        xf8.icvLeft = xf.icvLeft;
        xf8.icvRight = xf.icvRight;
        xf8.icvTop = xf.icvTop;
        xf8.icvBottom = xf.icvBottom;
        xf8.fls = xf.fls;
        xf8.fiForeColor = xf.fiForeColor;
        xf8.fiBackColor = xf.fiBackColor;
        xf8.fSxButton = xf.fSxButton;
      }
      else
      {
        memcpy((LPxXF8)&xf8, LPOFFSET(lpBiff, 4), sizeof(xXF8));
      }

      bif_LoadData((TBGLOBALHANDLE FAR *)&lpss->ghXFs, 
                   (WORD)sizeof(xXF8), 
                   (LPBYTE)&xf8, FALSE, FALSE, lpss->dwXFCount*sizeof(xXF8));

      lpss->dwXFCount++;
    }
    break;  
  
    default:
      WriteOutHex2(_T("!Unknown"), lpBiff->recnum);
    break;

  } //switch
   
  SS_BookUnlockSheetIndex(lpBook, 0); //nSheetNum);

  return 0;
}

/***********************************************************************
* Name:   bif_Tokenize2 - Load the BIFF data into the Excel conversion
*                         structure based on the type of BIFF structure
*                         that is passed in. This function is called to
*                         load the sheet specific data.
*
* Usage:  short bif_Tokenize2(LPBIFF lpBiff, LPSSXL lpss, LPSPREADSHEET lpSS)
*            lpBiff - pointer to the current BIFF structure.
*            lpss - the Excel conversion structure.
*            lpSS - pointer to the Spread structure
*
* Return: short - Success or Failure
***********************************************************************/
short bif_Tokenize2(LPBIFF lpBiff, LPSSXL lpss, LPSS_BOOK lpBook, int nSheetNum, LPBOOL lpIsValidSubstream)
{
  static BOOL   fFrozen = false;  
  LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, lpBook->nSheet);

  nSheetNum; // to make the compiler happy

  if (!*lpIsValidSubstream && (lpBiff->recnum != xlBOF_BIFF2 
                            && lpBiff->recnum != xlBOF_BIFF3
                            && lpBiff->recnum != xlBOF_BIFF4
                            && lpBiff->recnum != xlBOF_BIFF5_7_8
                            && lpBiff->recnum != xlEOF_BIFF)) //20158
  {
    SS_BookUnlockSheetIndex(lpBook, lpBook->nSheet); //nSheetNum);
    return 0;
  }
   
  switch(lpBiff->recnum)
  {
    case xlBOF_BIFF2:   // beginning of book stream - BIFF2
    case xlBOF_BIFF3:   // beginning of book stream - BIFF3
    case xlBOF_BIFF4:   // beginning of book stream - BIFF4
    case xlBOF_BIFF5_7_8: // beginning of book stream - BIFF5/BIFF7
    {
      zBOF bof;
      memcpy((LPzBOF)&(bof), LPOFFSET(lpBiff, 4), sizeof(zBOF));

      if (bof.wSubStrType == 0x05 || bof.wSubStrType == 0x10)
        *lpIsValidSubstream = TRUE;
      else
      {
        // this is a chart or other substream not supported by Spread
        *lpIsValidSubstream = FALSE;
      }

      fFrozen = false;
    }
      return BOF;

    case xlEOF_BIFF:
      if (lpss->ghShrFmla != NULL)
      {
        tbGlobalFree(lpss->ghShrFmla);
        lpss->ghShrFmla = (TBGLOBALHANDLE)0;
        lpss->dwShrFmlaCount = 0;
      }
      
      *lpIsValidSubstream = TRUE;

      fFrozen = false;
      return EOF;

    case xlCONTINUE:
    {
      WriteOut(_T("CONTINUE"), NULL);
      switch (lpss->bProcessDataType)
      {
        case PDT_PROCESSINGDBCELL:
        {
          //We don't need to do anything because Spread does not use the DBCELL for ROW/CELL offsets.
          lpss->bProcessDataType = 0;
        }
        break;
#ifdef SS_V40      
        case PDT_PROCESSINGTXO1:         
        {
          // Process the text for the TXO record.
          TBGLOBALHANDLE ghText = NULL;
          LPTSTR pszText = NULL;
          LPTSTR pszTXOText = NULL;          
          int nTextLen = lpBiff->datalen-1; // -1 for grbit
          int nCharCount = nTextLen;

          //Grab the grbit from the CONTINUE
          BOOL fUnCompress = (BOOL)*(BYTE *)LPOFFSET(lpBiff, sizeof(BIFF));
          if (fUnCompress)
          { 
            nCharCount /= sizeof(WCHAR); 
            Alloc(&ghText, nTextLen + sizeof(WCHAR));
          }
          else
          { 
            Alloc(&ghText, nTextLen + sizeof(CHAR));
          }
          pszText = (LPTSTR)tbGlobalLock(ghText);
          memcpy(pszText, (LPWORD)LPOFFSET(lpBiff, 5), nTextLen); 

//17769          Alloc(&lpss->SubStream.ghTXOs, (lpss->SubStream.dwTXOLen + nCharCount +1)*sizeof(TCHAR));
//          pszTXOText = (LPTSTR)tbGlobalLock(lpss->SubStream.ghTXOs);
//          
//          pszTXOText = (LPTSTR)((LPBYTE)pszTXOText + lpss->SubStream.dwTXOLen);
//          lpss->SubStream.dwTXOLen += (nCharCount+1)*sizeof(TCHAR);

#ifdef _UNICODE
          if (!fUnCompress)
          {
            // Convert the single byte string to a Unicode string.
//17769 >>
//            int nStringLen = (short)MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)pszText,
//                                                        0, NULL, nCharCount);
            int nStringLen = (short)MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)pszText,
                                                        -1, NULL, 0);

            Alloc(&lpss->SubStream.ghTXOs, (lpss->SubStream.dwTXOLen + nStringLen)*sizeof(TCHAR));
            pszTXOText = (LPTSTR)tbGlobalLock(lpss->SubStream.ghTXOs);
          
            pszTXOText = (LPTSTR)((LPBYTE)pszTXOText + lpss->SubStream.dwTXOLen);
            lpss->SubStream.dwTXOLen += nStringLen*sizeof(TCHAR); 
//<< 17769
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)pszText, nTextLen, pszTXOText, nCharCount);
          }
          else
          {
//17769 >>
            Alloc(&lpss->SubStream.ghTXOs, (lpss->SubStream.dwTXOLen + nCharCount +1)*sizeof(TCHAR));
            pszTXOText = (LPTSTR)tbGlobalLock(lpss->SubStream.ghTXOs);
          
            pszTXOText = (LPTSTR)((LPBYTE)pszTXOText + lpss->SubStream.dwTXOLen);
            lpss->SubStream.dwTXOLen += (nCharCount+1)*sizeof(TCHAR); 
//<< 17769

            _tcscpy(pszTXOText, pszText);
          }
#else //!UNICODE
          if (!fUnCompress)
          {
//17769 >>
            Alloc(&lpss->SubStream.ghTXOs, (lpss->SubStream.dwTXOLen + nCharCount +1)*sizeof(TCHAR));
            pszTXOText = (LPTSTR)tbGlobalLock(lpss->SubStream.ghTXOs);
          
            pszTXOText = (LPTSTR)((LPBYTE)pszTXOText + lpss->SubStream.dwTXOLen);
            lpss->SubStream.dwTXOLen += (nCharCount+1)*sizeof(TCHAR); 
//<<17769
            _tcscpy(pszTXOText, pszText);
          }
          else
          {
            // Convert the Unicode string to an ansi string.
//17769 >>
            int nStringLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, 
                                                 (LPWSTR)pszText, -1,
                                                 NULL, 0, //17769
                                                 NULL, NULL);

            Alloc(&lpss->SubStream.ghTXOs, (lpss->SubStream.dwTXOLen + nStringLen)*sizeof(TCHAR));
            pszTXOText = (LPTSTR)tbGlobalLock(lpss->SubStream.ghTXOs);
          
            pszTXOText = (LPTSTR)((LPBYTE)pszTXOText + lpss->SubStream.dwTXOLen);
            lpss->SubStream.dwTXOLen += nStringLen*sizeof(TCHAR); 
//<< 17769
    	      WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, 
                                (LPWSTR)pszText, nCharCount,
                                pszTXOText, (WORD)nCharCount*sizeof(WCHAR), //17769
                                NULL, NULL);
          }
#endif
          tbGlobalUnlock(ghText);
          tbGlobalFree(ghText);
          tbGlobalUnlock(lpss->SubStream.ghTXOs);
  
          // set the processing flag for the formatting run CONTINUE record.
          lpss->bProcessDataType = PDT_PROCESSINGTXO2;
        }
        break;
        case PDT_PROCESSINGTXO2:         
        {
          // Process the formatting runs of the TXO record.
          lpss->bProcessDataType = 0;
        }
        break;
#endif      
      }          
    }
    break;

#ifdef SS_V40      
    case xlNOTE:
    {
      yNOTE  note;
      short  i;
      LPTSTR pszText = (LPTSTR)tbGlobalLock(lpss->SubStream.ghTXOs);
      LPWORD lpwObjIDs = (LPWORD)tbGlobalLock(lpss->SubStream.ghOBJs);
      BOOL   isFound = FALSE;

      memcpy((LPyNOTE)&note, LPOFFSET(lpBiff, 4), sizeof(yNOTE));
      
      // look up the object id in the objID list
      for (i=0; i<(int)lpss->SubStream.dwOBJCount; i++)
      {
        if (lpwObjIDs[i] == note.idObj)
        {
          isFound = TRUE;
          break;
        }
      }
      
      tbGlobalUnlock(lpss->SubStream.ghOBJs);

      if (isFound)
      {
        for (int j=0; j<i; j++)
        {
          pszText = (LPTSTR)(pszText + lstrlen(pszText) + 1);
        }
        WORD wStatus;
        long lDelay;
        LOGFONT lf;
        COLORREF clrBack, clrFore;
        SSGetTextTip(lpBook->hWnd, &wStatus, &lDelay, &lf, &clrBack, &clrFore);  
        wStatus = FP_TT_STATUS_FIXED;
        SSSetTextTip(lpBook->hWnd, wStatus, lDelay, &lf, clrBack, clrFore);

        SSSetCellNote(lpBook->hWnd, note.col+1, note.rw+1, pszText);

        tbGlobalUnlock(lpss->SubStream.ghTXOs);
    
      }
    }
    break;
    case xlTXO:
    {
      yTXO txo;
      memcpy((LPyTXO)&txo, LPOFFSET(lpBiff, 4), sizeof(yTXO));
      if (txo.cchText)
        lpss->bProcessDataType = PDT_PROCESSINGTXO1;
    }
    break;
    case xlMERGECELLS:
    {
      WORD   wMergeCount;
      yMERGE merge;
      short  offset = 4;
      int    i;

      memcpy(&wMergeCount, LPOFFSET(lpBiff, offset), sizeof(WORD));
      offset += sizeof(WORD);

      for (i=0; i<wMergeCount; i++)
      {      
        memcpy((LPyMERGE)&merge, LPOFFSET(lpBiff, offset), sizeof(yMERGE));
        if (merge.col1 == 0 && merge.col2 == 0xffff)
          SSAddCellSpan(lpBook->hWnd, SS_ALLCOLS, merge.rw1+1, 0, (merge.rw2-merge.rw1+1));
        else if (merge.rw1 == 0 && merge.rw2 == 0xffff)
          SSAddCellSpan(lpBook->hWnd, merge.col1+1, SS_ALLROWS, (merge.col2-merge.col1+1), 0);
        else
          SSAddCellSpan(lpBook->hWnd, merge.col1+1, merge.rw1+1, (merge.col2-merge.col1+1), (merge.rw2-merge.rw1+1)); 
        offset += sizeof(yMERGE);
      }
    }
    break;
#endif

#ifdef SS_V70    
    case xlPANE:
    {
      yPANE pane;

      if (fFrozen)
      {
        memcpy(&pane, LPOFFSET(lpBiff, 4), sizeof(yPANE));
        if (pane.x > 0 || pane.y > 0)
          SSSetFreeze(lpBook->hWnd, pane.x, pane.y);
      }
    }
    break;
#endif

    //cell values...
    case xlBLANK:
    {
      yCELLVAL cell;
     
      memset((LPyCELLVAL)&cell, 0, sizeof(yCELLVAL));
      cell.wCellType = xlBLANK;
      memcpy((LPyCELLVAL)&(cell.wRow), LPOFFSET(lpBiff, 4), sizeof(yCELLVAL)-sizeof(WORD)-sizeof(TBGLOBALHANDLE));

      bif_LoadCell(lpss, lpSS, (LPyCELLVAL)&cell);

      if (cell.ghCell != 0)
        tbGlobalFree(cell.ghCell);

      WriteOutDW2(_T("CELL-BLANK"), cell.wCol);
    }
    break;
    case xlFORMULA:
    case xlFORMULA2:
    {
      yCELLVAL cell;
      yyFORMULA fx;
          
      memset((LPyCELLVAL)&cell, 0, sizeof(yCELLVAL));
      cell.wCellType = xlFORMULA;
      memcpy((LPyCELLVAL)&(cell.wRow), LPOFFSET(lpBiff, 4), sizeof(yCELLVAL)-sizeof(WORD)-sizeof(TBGLOBALHANDLE));

      memcpy((LPyyFORMULA)&fx, LPOFFSET(lpBiff, 10), sizeof(yyFORMULA)-sizeof(TBGLOBALHANDLE));
    
      bif_LoadData((TBGLOBALHANDLE FAR *)&(fx.ghFormula), 
                   (WORD)fx.wFormulaLen, 
                   (LPBYTE)LPOFFSET(lpBiff, 26), FALSE, FALSE, 0);
      
      bif_LoadData((TBGLOBALHANDLE FAR *)&(cell.ghCell), 
                   (WORD)sizeof(yyFORMULA), 
                   (LPBYTE)&fx, FALSE, FALSE, 0);

      int ret = bif_LoadCell(lpss, lpSS, (LPyCELLVAL)&cell);

      if (ret == 0 && cell.ghCell != 0)
      {
        if (fx.ghFormula != 0)
          tbGlobalFree(fx.ghFormula);
        tbGlobalFree(cell.ghCell);
      }

      WriteOutDW2(_T("CELL-FORMULA"), NULL);
    }
    break;
//RAP04a >>
    case xlRSTRING: //25092
    case xlLABEL:
    {
/* Merged BOC's changes with existing code.
      yCELLVAL cell;
      yyLABEL label;
          
      memset((LPyCELLVAL)&cell, 0, sizeof(yCELLVAL));

      memset((LPyyLABEL)&label, 0, sizeof(yyLABEL));
      memcpy((LPyyLABEL)&label, LPOFFSET(lpBiff, 4), sizeof(yyLABEL)-sizeof(TBGLOBALHANDLE));

  	  // BUG 01811 (3-2)
  	  // Support Of Label Of Excel Is New Added To Spread,
	    // So Some Code Need To Be Add To Support Japanese Version.
  	  // Modified By HaHa 1999.11.18
  	  BOOL fWChar=FALSE;
	    #ifdef SPREAD_JPN
	    #ifndef _UNICODE
  	  LPBIFFSTR lpBiffstr = (LPBIFFSTR)LPOFFSET(lpBiff, 10);
	    fWChar=lpBiffstr->fHighByte;
	    #endif
  	  #endif
	    bif_LoadData((TBGLOBALHANDLE FAR *)&(label.ghString), 
                   (WORD)sizeof(BIFFSTR)-1 + label.cch*sizeof(TCHAR) + sizeof(WORD), //BIFFSTR - 1byte for bString + sizeof string + sizeof cch(string length)
                   (LPBYTE)LPOFFSET(lpBiff, 10), TRUE, fWChar, 0);
      //bif_LoadData((TBGLOBALHANDLE FAR *)&(label.ghString), 
      //             (WORD)sizeof(BIFFSTR)-1 + label.cch*sizeof(TCHAR) + sizeof(WORD), //BIFFSTR - 1byte for bString + sizeof string + sizeof cch(string length)
      //             (LPBYTE)LPOFFSET(lpBiff, 10), TRUE, FALSE, 0);
      
      cell.wCellType = xlLABEL;
      cell.wCol = label.wCol;
      cell.wRow = label.wRow;
      cell.wixfe = label.wixfe;
      cell.ghCell = label.ghString;

      bif_LoadCell(lpss, lpSS, (LPyCELLVAL)&cell);

      WriteOutDW2(_T("CELL-LABEL"), NULL);
  
      tbGlobalFree(cell.ghCell); 
*/
      yCELLVAL cell;
      yyLABEL  label;
  	  BOOL     fWChar = FALSE;

	    #ifndef _UNICODE
    	  LPBIFFSTR lpBiffstr = (LPBIFFSTR)LPOFFSET(lpBiff, 10);
	      fWChar = lpBiffstr->fHighByte;
      #endif
                    
      memset((LPyCELLVAL)&cell, 0, sizeof(yCELLVAL));
      memset((LPyyLABEL)&label, 0, sizeof(yyLABEL));

      memcpy((LPyyLABEL)&label, LPOFFSET(lpBiff, 4), sizeof(yyLABEL)-sizeof(TBGLOBALHANDLE));
      if (lpss->bof.wSubStrType == 0x10 || lpss->bof.wSubStrType == 0x05)
      {
        bif_LoadData((TBGLOBALHANDLE FAR *)&(label.ghString), 
                     (WORD)(sizeof(BIFFSTR)-1 + label.cch*sizeof(TCHAR) + sizeof(WORD)), //BIFFSTR - 1byte for bString + sizeof string + sizeof cch(string length)
                     (LPBYTE)LPOFFSET(lpBiff, 10), TRUE, fWChar, 0);
      
        cell.wCellType = xlLABEL;
        cell.wCol = label.wCol;
        cell.wRow = label.wRow;
        cell.wixfe = label.wixfe;
        cell.ghCell = label.ghString;

        bif_LoadCell(lpss, lpSS, (LPyCELLVAL)&cell);

        if (cell.ghCell)
          tbGlobalFree(cell.ghCell); 
      }
      WriteOutDW2(_T("CELL-LABEL"), NULL);
    }
    break;
//RAP04a <<
    case xlLABELSST:
    {
      yCELLVAL cell;
      yyLABELSST labelsst;
          
      memset((LPyCELLVAL)&cell, 0, sizeof(yCELLVAL));
      cell.wCellType = xlLABELSST;
      memcpy((LPyCELLVAL)&(cell.wRow), LPOFFSET(lpBiff, 4), sizeof(yCELLVAL)-sizeof(WORD)-sizeof(TBGLOBALHANDLE));

      memcpy((LPDWORD)&labelsst.dwisst, LPOFFSET(lpBiff, 10), sizeof(DWORD));

      if (lpss->bof.wSubStrType == 0x10 || lpss->bof.wSubStrType == 0x05)
      {
        bif_LoadData((TBGLOBALHANDLE FAR *)&(cell.ghCell), 
                     (WORD)sizeof(yyLABELSST), 
                     (LPBYTE)&labelsst, FALSE, FALSE, 0);

        bif_LoadCell(lpss, lpSS, (LPyCELLVAL)&cell);
        if (cell.ghCell != 0)
          tbGlobalFree(cell.ghCell);
      }

      WriteOutDW2(_T("CELL-LABELSST"), cell.wCol);
    }
    break;
    case xlNUMBER:
    {
      yCELLVAL cell;
      yyNUMBER number;
          
      memset((LPyCELLVAL)&cell, 0, sizeof(yCELLVAL));
      cell.wCellType = xlNUMBER;
      memcpy((LPyCELLVAL)&(cell.wRow), LPOFFSET(lpBiff, 4), sizeof(yCELLVAL)-sizeof(WORD)-sizeof(TBGLOBALHANDLE));

      memcpy((double far *)&number.dblNumber, LPOFFSET(lpBiff, 10), sizeof(double));
    
      if (lpss->bof.wSubStrType == 0x10 || lpss->bof.wSubStrType == 0x05)
      {
        bif_LoadData((TBGLOBALHANDLE FAR *)&(cell.ghCell), 
                    (WORD)sizeof(yyNUMBER), 
                     (LPBYTE)&number, FALSE, FALSE, 0);
        bif_LoadCell(lpss, lpSS, (LPyCELLVAL)&cell);
        if (cell.ghCell != 0)
          tbGlobalFree(cell.ghCell);
      }

      WriteOutDW2(_T("CELL-NUMBER"), NULL);
    }
    break;
    case xlRK:
    {
      yCELLVAL cell;
      yyRK rk;
          
      memset((LPyCELLVAL)&cell, 0, sizeof(yCELLVAL));
      cell.wCellType = xlRK;
      memcpy((LPyCELLVAL)&(cell.wRow), LPOFFSET(lpBiff, 4), sizeof(yCELLVAL)-sizeof(WORD)-sizeof(TBGLOBALHANDLE));

      memcpy((LPDWORD)&rk.dwRK, LPOFFSET(lpBiff, 10), sizeof(DWORD));
    
      if (lpss->bof.wSubStrType == 0x10 || lpss->bof.wSubStrType == 0x05)
      {
        bif_LoadData((TBGLOBALHANDLE FAR *)&(cell.ghCell), 
                    (WORD)sizeof(yyRK), 
                     (LPBYTE)&rk, FALSE, FALSE, 0);

        bif_LoadCell(lpss, lpSS, (LPyCELLVAL)&cell);
        if (cell.ghCell != 0)
          tbGlobalFree(cell.ghCell);
      }

      WriteOutDW2(_T("CELL-RK"), NULL);
    }
    break;
    case xlBOOLERR:
    {
      yCELLVAL cell;
      yyBOOLERR be;

      memset((LPyCELLVAL)&cell, 0, sizeof(yCELLVAL));
      cell.wCellType = xlBOOLERR;
      memcpy((LPyCELLVAL)&(cell.wRow), LPOFFSET(lpBiff, 4), sizeof(yCELLVAL)-sizeof(WORD)-sizeof(TBGLOBALHANDLE));

      memcpy((LPDWORD)&be, LPOFFSET(lpBiff, 10), sizeof(WORD));
    
      if (lpss->bof.wSubStrType == 0x10 || lpss->bof.wSubStrType == 0x05)
      {
        bif_LoadData((TBGLOBALHANDLE FAR *)&(cell.ghCell), 
                    (WORD)sizeof(yyBOOLERR), 
                     (LPBYTE)&be, FALSE, FALSE, 0);

        bif_LoadCell(lpss, lpSS, (LPyCELLVAL)&cell);
        if (cell.ghCell != 0)
          tbGlobalFree(cell.ghCell);
      }

      WriteOutDW2(_T("CELL-RK"), NULL);

    }
    break;
    case xlMULBLANK:
    {
      yCELLVAL    cell;
      WORD        wArraySize = (WORD)(lpBiff->datalen-0x06);//wRow+wFirstCol+wLastCol=6bytes
      WORD        wFirstCol;
      WORD        wLastCol;
      WORD        wRow;
      WORD        w;
      LPWORD      lpwixfe = (LPWORD)LPOFFSET(lpBiff, 8);
          
      wRow = *(LPWORD)LPOFFSET(lpBiff, 4);
      wFirstCol = *(LPWORD)LPOFFSET(lpBiff, 6);
      wLastCol = *(LPWORD)LPOFFSET(lpBiff, (wArraySize+0x08));
   
      cell.wCellType = xlBLANK;
      for (w=wFirstCol; w<=wLastCol; w++)
      {
        cell.wRow = wRow;
        cell.wCol = w;  
        cell.wixfe = lpwixfe[w-wFirstCol];
        cell.ghCell = 0;

        bif_LoadCell(lpss, lpSS, (LPyCELLVAL)&cell);
      } //for

      WriteOutDW2(_T("MULBLANK"), NULL);
    }
    break;

    case xlMULRK:
    {
      yCELLVAL    cell;
      WORD        wArraySize = (WORD)(lpBiff->datalen-0x06);//wRow+wFirstCol+wLastCol=6bytes
      WORD        wFirstCol;
      WORD        wLastCol;
      WORD        wRow;
      WORD        w;
      LPRKREC     lprkrec = (LPRKREC)LPOFFSET(lpBiff, 8);
          
      wRow = *(LPWORD)LPOFFSET(lpBiff, 4);
      wFirstCol = *(LPWORD)LPOFFSET(lpBiff, 6);
      wLastCol = *(LPWORD)LPOFFSET(lpBiff, (wArraySize+0x08));
   
      cell.wCellType = xlRK;
      for (w=wFirstCol; w<=wLastCol; w++)
      {
        cell.wRow = wRow;
        cell.wCol = w;  
        cell.wixfe = lprkrec[w-wFirstCol].wixfe;
        cell.ghCell = 0;

        bif_LoadData((TBGLOBALHANDLE FAR *)&cell.ghCell, 
                     (WORD)sizeof(yyRK), 
                     (LPBYTE)&(lprkrec[w-wFirstCol].rk), FALSE, FALSE, 0);

        bif_LoadCell(lpss, lpSS, (LPyCELLVAL)&cell);
        if (cell.ghCell != 0)
          tbGlobalFree(cell.ghCell);
      } //for

      WriteOutDW2(_T("MULRK"), NULL);
    }
    break;

    case xlARRAY:
    {
      yyARRAY     array;
      yySHRFMLA   shrfmla;
      LPyyFORMULA lpFmla = (LPyyFORMULA)tbGlobalLock(lpss->ShrFmlaCell.ghCell);
      LPBYTE      lpb = NULL;

      lpss->fShrFmlaFlag = TRUE; //Set the flag

      memcpy((LPyySHRFMLA)&array, LPOFFSET(lpBiff, 4), sizeof(yyARRAY)-sizeof(TBGLOBALHANDLE));

      shrfmla.cce = array.len;
      shrfmla.colFirst = array.colFirst;
      shrfmla.colLast = array.colLast;
      shrfmla.rwFirst = array.rwFirst;
      shrfmla.rwLast = array.rwLast;
      bif_LoadData((TBGLOBALHANDLE FAR *)&(shrfmla.ghFmla), 
                   (WORD)shrfmla.cce,
                   (LPBYTE)LPOFFSET(lpBiff, 18), FALSE, FALSE, 0);
      
      //Add the shared formula to the shared formula list.
      Alloc(&lpss->ghShrFmla, (lpss->dwShrFmlaCount+1)*sizeof(yySHRFMLA));
      lpb = (LPBYTE)tbGlobalLock(lpss->ghShrFmla);
      memcpy(lpb+(lpss->dwShrFmlaCount*sizeof(yySHRFMLA)), &shrfmla, sizeof(yySHRFMLA));
      tbGlobalUnlock(lpss->ghShrFmla);
      lpss->dwShrFmlaCount++;

      //Shared formulas always follow a FORMULA tag that references the shared formula.
      //Go ahead and process the first one.
      lpFmla->wFormulaLen = shrfmla.cce; 
      lpFmla->ghFormula = shrfmla.ghFmla;
      bif_LoadCell(lpss, lpSS, (LPyCELLVAL)&lpss->ShrFmlaCell);
            
      WriteOutDW2(_T("ARRAY"), NULL);
    }
    break;
    case xlSHRFMLA:
    case xlSHRFMLA2:
    {
      yySHRFMLA   shrfmla;
      LPyyFORMULA lpFmla = (LPyyFORMULA)tbGlobalLock(lpss->ShrFmlaCell.ghCell);
      LPBYTE      lpb = NULL;

//19748 >> 
//20154      if (lpFmla == NULL)  I don't think this was necessary...
//20154        break;
//<<19748

      lpss->fShrFmlaFlag = TRUE; //Set the flag

      memcpy((LPyySHRFMLA)&shrfmla, LPOFFSET(lpBiff, 4), sizeof(yySHRFMLA)-sizeof(TBGLOBALHANDLE));

      bif_LoadData((TBGLOBALHANDLE FAR *)&(shrfmla.ghFmla), 
                   (WORD)shrfmla.cce, 
                   (LPBYTE)LPOFFSET(lpBiff, 14), FALSE, FALSE, 0);
      
      //Add the shared formula to the shared formula list.
      Alloc(&lpss->ghShrFmla, (lpss->dwShrFmlaCount+1)*sizeof(yySHRFMLA));
      lpb = (LPBYTE)tbGlobalLock(lpss->ghShrFmla);
      memcpy(lpb+(lpss->dwShrFmlaCount*sizeof(yySHRFMLA)), &shrfmla, sizeof(yySHRFMLA));
      tbGlobalUnlock(lpss->ghShrFmla);
      lpss->dwShrFmlaCount++;

      //Shared formulas always follow a FORMULA tag that references the shared formula.
      //Go ahead and process the first one.
      if (lpFmla != NULL)
      {
        lpFmla->wFormulaLen = shrfmla.cce; 
        lpFmla->ghFormula = shrfmla.ghFmla;
      
        bif_LoadCell(lpss, lpSS, (LPyCELLVAL)&lpss->ShrFmlaCell);

        memset(&(lpss->ShrFmlaCell), 0, sizeof(yCELLVAL)); //19748
      }

      WriteOutDW2(_T("SHARED FORMULA"), NULL);
    }
    break;
    case xlCALCCOUNT:
    {
      lpss->SubStream.wMaxIterations = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("CALCCOUNT"), *(LPOFFSET(lpBiff, 4)));
    }
    break;
    case xlCALCMODE:
    {
//      WORD wCalcMode = *(LPOFFSET(lpBiff, 4));
      lpss->SubStream.wCalcMode = *(LPOFFSET(lpBiff, 4));
      //SetCalcMode
#ifdef SS_V70
//18783      SSSetBool(lpBook->hWnd, SSB_AUTOCALC, (BOOL)lpss->SubStream.wCalcMode);
#endif
      WriteOutW2(_T("CALCMODE"), *(LPOFFSET(lpBiff, 4)));
    }
    break;
    case xlCOLINFO:
    {
      yCOLINFO col;

      memset((LPyCOLINFO)&col, 0, sizeof(yCOLINFO));
      memcpy((LPyCOLINFO)&col, LPOFFSET(lpBiff, 4), sizeof(yCOLINFO));

      bif_SetColProps(lpss, lpSS, (LPyCOLINFO)&col);

      WriteOutW2(_T("COLINFO"), col.wColFirst);
    }
    break;
    case xlDEFAULTROWHEIGHT:
    {
      memcpy(&(lpss->DefRowHeight), LPOFFSET(lpBiff, 4), sizeof(yDEFROWHEIGHT));
      WriteOut(_T("DEFAULTROWHEIGHT"), NULL);
    }
    break;
    case xlDEFCOLWIDTH:
    {
      WORD     wDefColWidth = *(LPOFFSET(lpBiff, 4));
      COLORREF color = RGBCOLOR_DEFAULT;
      xXF8     xf;
      HFONT    hFont = (HFONT)0;
//      double   dblTwips = 0.0;
//      double   dblPixels = 0.0;
      HDC      hDC = GetDC(lpBook->hWnd);

      //RAP01d      bif_GetXF(lpss->ghXFs, 0, &xf);
      bif_GetXF(lpss->ghXFs, 15, &xf); //RAP01a
      //Get the SpreadSheet Font
      bif_GetFont(lpss->ghFonts, xf.wifnt, (LPHFONT)&hFont, (LPCOLORREF)&color);
      if (hFont)
      {
        LPLOGFONT lplf = NULL;

        if (lpss->ghDefaultLogFont)
        {
          tbGlobalFree(lpss->ghDefaultLogFont);
          lpss->ghDefaultLogFont = 0;
        }

        Alloc(&lpss->ghDefaultLogFont, sizeof(LOGFONT));
        lplf = (LPLOGFONT)tbGlobalLock(lpss->ghDefaultLogFont);
        //int   ret = GetObject(hFont, sizeof(LOGFONT), lplf);
        int   len = lstrlen(lplf->lfFaceName);
        memset(&(lplf->lfFaceName[len]), 0, (LF_FACESIZE-len)*sizeof(TCHAR));
        tbGlobalUnlock(lpss->ghDefaultLogFont);

        SSSetFont(lpBook->hWnd, SS_ALLCOLS, SS_ALLROWS, hFont, FALSE);
        SSSetFont(lpBook->hWnd, lpss->xlSS_HEADER, SS_ALLROWS, hFont, FALSE);
        SSSetFont(lpBook->hWnd, SS_ALLCOLS, lpss->xlSS_HEADER, hFont, TRUE);
      }

//15436D      //SetDefaultRowHeight  RAP13a
//15436D      dblTwips = MulDiv(lpss->DefRowHeight.wDefRowHeight, TWIP_PER_PT, 20);
//15436D      dblPixels = MulDiv((int)dblTwips, GetDeviceCaps(hDC, LOGPIXELSY), 1440);
//15436D      SSSetRowHeightInPixels(lpBook->hWnd, SS_ALLROWS, (WORD)dblPixels);
/*
      int      nColWidthPixels;
      HFONT    hFontOld;
  	  RECT     rc;


      hFont = SSGetFont(lpBook->hWnd, SS_ALLCOLS, SS_ALLROWS);     
      hFontOld = (HFONT)SelectObject(hDC, hFont);

      //Set the default column width
      int CharWidth = 0;
	    GetCharWidth32(hDC, '0', '0', &CharWidth);
  	  if(CharWidth<=0)
	  	  CharWidth=1;

      SelectObject(hDC, hFontOld);

      nColWidthPixels = (wDefColWidth * CharWidth) + 1; // +1 for gridline

      SSSetColWidthInPixels(lpBook->hWnd, SS_ALLCOLS, nColWidthPixels);
*/
      SSSetColWidthInPixels(lpBook->hWnd, SS_ALLCOLS, (wDefColWidth * 8));


//RAP02c - Excel uses the fls flag to determine whether to use the backcolor value in the XF structure.
//      COLORREF clrback = 0x00FFFFFF;
//RAP03a >>
      if (color == (0x80000000L | COLOR_INACTIVEBORDER) || color == (COLORREF)-1)
        color = (0x8000000 | COLOR_WINDOWTEXT);
//<< RAP03a
//      SSSetColor(lpBook->hWnd, SS_ALLCOLS, SS_ALLROWS, (xf.fls?xl_ColorFromIndex(xf.fiBackColor):clrback), color);

      //SetLock for SpreadSheet Workbook
      SSSetLock(lpBook->hWnd, SS_ALLCOLS, SS_ALLROWS, (BOOL)xf.fLocked);

      ReleaseDC(lpBook->hWnd, hDC);

      WriteOut(_T("DEFCOLWIDTH"), NULL);
    }
    break;

    case xlDBCELL:
    {
#ifndef V7TEST
      yDBCELL dbcell;
      WORD    wArraySize = (WORD)(lpBiff->datalen-sizeof(DWORD));
     
      lpss->bProcessDataType = PDT_PROCESSINGDBCELL;

      memset((LPyDBCELL)&dbcell, 0, sizeof(yDBCELL));
      dbcell.dwRowRecOffset = *(LPDWORD)LPOFFSET(lpBiff, 4);
  
      if (wArraySize)
      {
        bif_LoadData((TBGLOBALHANDLE FAR *)&(dbcell.ghStreamOffsets), 
                     wArraySize,
                     (LPBYTE)LPOFFSET(lpBiff, 8), FALSE, FALSE, 0);

        bif_LoadData((TBGLOBALHANDLE FAR *)&(lpss->SubStream.ghDBCells), 
                     (WORD)sizeof(yDBCELL), 
                     (LPBYTE)&dbcell, FALSE, FALSE, lpss->SubStream.dwDBCellCount*sizeof(yDBCELL));
      }
#endif
      WriteOutDW2(_T("DBCELL"), lpss->SubStream.dwDBCellCount);
    }
    break;
    case xlDELTA:
    {
      lpss->SubStream.dblMaxDelta = *(double FAR *)(LPOFFSET(lpBiff, 4));
      WriteOut(_T("DELTA"), NULL);
    }
    break;
    case xlDIMENSIONS:
    {
      memcpy(&(lpss->SubStream.dim), LPOFFSET(lpBiff, 4), sizeof(yDIM));
      WriteOut(_T("DIMENSIONS"), NULL);
    }
    break;
    break;
    case xlFOOTER:
    {
      lpss->SubStream.bFooterLen = *(LPOFFSET(lpBiff, 4));

      bif_LoadData((TBGLOBALHANDLE FAR *)&lpss->SubStream.ghFooter, 
                   (WORD)lpss->SubStream.bFooterLen, 
                   (LPBYTE)LPOFFSET(lpBiff, 5), TRUE, FALSE, 0);

      WriteOutGH2(_T("FOOTER"), lpss->SubStream.ghFooter);
    }
    break;
    case xlGRIDSET:
    {
      lpss->SubStream.wGridSet = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("GRIDSET"), *(LPOFFSET(lpBiff, 4)));
    }
    break;
    case xlGUTS:
    {
#ifndef V7_TEST
      memcpy(&(lpss->SubStream.guts), LPOFFSET(lpBiff, 4), sizeof(yGUTS));
      WriteOut(_T("GUTS"), NULL);
#endif
    }
    break;
    case xlHLINK:
    {
      yHLINK hlink;
      LPCWSTR lp = NULL;
      short  nRow, nCol;
      short  nLoadSize = (short)(lpBiff->datalen-(sizeof(yHLINK)-sizeof(DWORD)));
      BYTE   wURLMoniker[16] = {0xD0,0xC9,0xEA,0x79,0xF9,0xBA,0xCE,0x11,0x8C,0x82,0x00,0xAA,0x00,0x4B,0xA9,0x0B};
      BYTE   wURLGuid[16] = {0xE0,0xC9,0xEA,0x79,0xF9,0xBA,0xCE,0x11,0x8C,0x82,0x00,0xAA,0x00,0x4B,0xA9,0x0B};
      memset((LPyHLINK)&hlink, 0, sizeof(yHLINK));
      memcpy(&hlink, LPOFFSET(lpBiff, 4), sizeof(yHLINK)-sizeof(DWORD));

      if (hlink.colFirst > 255 || hlink.colLast > 255)
        break;

      bif_LoadData((TBGLOBALHANDLE FAR *)&(hlink.ghData), 
                   (WORD)nLoadSize,
                   (LPBYTE)LPOFFSET(lpBiff, 12), FALSE, FALSE, 0);
      lp = (LPCWSTR)tbGlobalLock(hlink.ghData);
      
      if (!memcmp(wURLMoniker, lp, 16))
      {
        // So far it is a valid HLINK
        LPBYTE lpPos = ((LPBYTE)lp + 16 + sizeof(DWORD));
        long lOptions = *(long *)lpPos;
        if (lOptions & 1)
        {
//          if (lOptions & 2)
          {
            lpPos += sizeof(DWORD);
            if (!memcmp(wURLGuid, lpPos, 16))
            {
              // URL
              TBGLOBALHANDLE ghURL = (TBGLOBALHANDLE)0;
              LPTSTR         lpszURL = NULL;
              long           byteLen = 0;

              lpPos += 16;
              byteLen = *(long*)lpPos;
              lpPos += sizeof(long);
              ghURL = tbGlobalAlloc(GHND, byteLen); 
              lpszURL = (LPTSTR)tbGlobalLock(ghURL);
// The hyperlink URL is saved as a WCHAR string!
#ifndef _UNICODE
              WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (LPCWSTR)lpPos, -1, lpszURL,
                                  byteLen, NULL, NULL);
#else
              memcpy(lpszURL, (LPTSTR)lpPos, byteLen);
#endif

              for (nCol=hlink.colFirst; nCol<=hlink.colLast; nCol++)
              {
                for (nRow=hlink.rwFirst; nRow<=hlink.rwLast; nRow++)
                {
                  TBGLOBALHANDLE ghBuffer = (TBGLOBALHANDLE)0;
                  LPTSTR         lpszBuffer = NULL;
                  short nAnchorLen = (short)SSGetDataLen(lpBook->hWnd, nCol+1, nRow+1);
                  Alloc(&ghBuffer, (nAnchorLen + byteLen + 20/*URL(, ) and some padding*/)*sizeof(TCHAR));
                  lpszBuffer = (LPTSTR)tbGlobalLock(ghBuffer);
                  if (nAnchorLen)
                  {
                    TBGLOBALHANDLE ghAnchor = tbGlobalAlloc(GHND, (nAnchorLen+1)*sizeof(TCHAR));
                    LPTSTR         lpszAnchor = (LPTSTR)tbGlobalLock(ghAnchor);
                
                    SSGetData(lpBook->hWnd, nCol+1, nRow+1, lpszAnchor);
                    _stprintf(lpszBuffer, _T("URL(\"%s\", \"%s\")"), lpszURL, lpszAnchor);
                    tbGlobalUnlock(ghAnchor);
                    tbGlobalFree(ghAnchor);
                  }
                  else
                    _stprintf(lpszBuffer, _T("URL(\"%s\")"), lpszURL);
                  SSSetFormula(lpBook->hWnd, nCol+1, nRow+1, lpszBuffer, TRUE);
                  tbGlobalUnlock(ghBuffer);
                  tbGlobalFree(ghBuffer);
                }
              }
              tbGlobalUnlock(ghURL);
              tbGlobalFree(ghURL);
            }
          }
        }
        else if (lOptions & 0x14)
        {
          if (lOptions & 8)
          {
          }
        }
      }
      tbGlobalUnlock(hlink.ghData);
      tbGlobalFree(hlink.ghData);

      WriteOut(_T("HLINK"), NULL);
    }
    break;
    case xlINDEX:
    {
#ifndef V7_TEST
      //An index rec is written following the BOF for each wksht.
      //Hopefully, this is the correct place to set up a new substream
      //structure.

      memset((LPSUBSTREAM)&lpss->SubStream, 0, sizeof(SUBSTREAM));
      if (lpss->wVer == 8)
        memcpy(&lpss->SubStream.index8, LPOFFSET(lpBiff, 4), sizeof(yINDEX8));
      else
        memcpy(&lpss->SubStream.index, LPOFFSET(lpBiff, 4), sizeof(yINDEX));
#endif
      WriteOut(_T("INDEX"), NULL);
    }
    break;
    case xlHCENTER:
    {
      lpss->SubStream.wHCenter = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("HCENTER"), *(LPOFFSET(lpBiff, 4)));
    }
    break;
    case xlHEADER:
    {
      if (lpBiff->datalen <= sizeof(BIFF))
        break;
      lpss->SubStream.bHeaderLen = *(LPOFFSET(lpBiff, 4));

      bif_LoadData((TBGLOBALHANDLE FAR *)&lpss->SubStream.ghHeader, 
                   (WORD)lpss->SubStream.bHeaderLen, 
                   (LPBYTE)LPOFFSET(lpBiff, 5), TRUE, FALSE, 0);

      WriteOutGH2(_T("HEADER"), lpss->SubStream.ghHeader);
    }
    break;
    case xlITERATION:
    {
      lpss->SubStream.wIteration = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("ITERATION"), *(LPOFFSET(lpBiff, 4)));
    }
    break;
    case xlOBJ:
    {
      WORD wSubRecordType = (WORD)*(LPOFFSET(lpBiff, 4));

      if (wSubRecordType == 0x15) // ftCmo - Common Object
      {
        WORD wObjType = (WORD)*(LPOFFSET(lpBiff, 8));
        if (wObjType == 0x19) //Comment
        {
          WORD   wObjID = *(LPWORD)(LPOFFSET(lpBiff, 10));
          LPWORD lpwObjID = NULL;
          Alloc(&lpss->SubStream.ghOBJs, (++lpss->SubStream.dwOBJCount)*sizeof(WORD));
          lpwObjID = (LPWORD)tbGlobalLock(lpss->SubStream.ghOBJs);
          lpwObjID[lpss->SubStream.dwOBJCount-1] = wObjID;
          tbGlobalUnlock(lpss->SubStream.ghOBJs);
        }
      }
      xl_LogFile(lpBook, LOG_CELLOBJECTSNOTSUPPORTED, 0, 0, NULL);
    }
    break;
    case xlPLS:
    {
#ifndef V7_TEST
      lpss->SubStream.pls.wEnv = *(LPOFFSET(lpBiff, 4));
      bif_LoadData((TBGLOBALHANDLE FAR *)&lpss->SubStream.pls.ghPrintStruct, 
                   (WORD)lpBiff->datalen, 
                   LPOFFSET(lpBiff, 6), TRUE, FALSE, 0);
#endif
      WriteOut(_T("PLS"), NULL);
    }
    break;
    case xlPRINTGRIDLINES:
    {
      lpSS->PrintOptions.pf.fShowGrid = *(LPOFFSET(lpBiff, 4));
      lpSS->PrintOptions.pf.fDrawBorder = lpSS->PrintOptions.pf.fShowGrid;
//      lpss->SubStream.wPrintGrid = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("PRINTGRIDLINES"), *(LPOFFSET(lpBiff, 4)));
    }
    break;
    case xlPRINTHEADERS:
    {
      lpSS->PrintOptions.pf.fShowColHeaders = lpSS->PrintOptions.pf.fShowRowHeaders = 
                                                                   *(LPOFFSET(lpBiff, 4));
//      lpss->SubStream.wPrintHeaders = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("PRINTHEADERS"), *(LPOFFSET(lpBiff, 4)));
    }
    break;
    case xlPROTECT:
    {
      lpss->SubStream.bProtect = *(LPOFFSET(lpBiff, 4)); //Protect Worksheet
      SSSetBool(lpBook->hWnd, SSB_PROTECT, lpss->SubStream.bProtect);

      WriteOutW2(_T("PROTECT-shtlevel"), lpss->SubStream.bProtect);
    }
    break;
    case xlREFMODE:
    {
      WORD wRefMode = *(LPOFFSET(lpBiff, 4));

      //SetRefStyle
      SSSetRefStyle(lpBook->hWnd, (short)((wRefMode==1)?SS_REFSTYLE_A1:SS_REFSTYLE_R1C1));
//      SS_SetRefStyle(lpSS, (short)((wRefMode==1)?SS_REFSTYLE_A1:SS_REFSTYLE_R1C1));
      
      if (wRefMode != 1)
        SSSetColHeaderDisplay(lpBook->hWnd, SS_HEADERDISPLAY_NUMBERS);

      if (!lpss->fNamesSet)
      {
        LPxNAME lpXName = NULL;

        if (lpss->dwNameCount > 0)
        {
          lpXName = (LPxNAME)tbGlobalLock(lpss->ghNames);
        }
        if (lpXName != NULL)
        {
          int i=0;
          for (i=0; i<(int)lpss->dwNameCount; i++)
          {
            LPTSTR  lptstrName = NULL;
            TCHAR   szFormula[5000];

            memset(szFormula, 0, 5000*sizeof(TCHAR));
            lptstrName = (LPTSTR)tbGlobalLock(lpXName[i].ghName);
            if (lpXName[i].ghNameDef)
            {
              BYTE      bType = 0;
              yyFORMULA f;
  
              memset(&f, 0, sizeof(yyFORMULA));
              f.wFormulaLen = lpXName[i].cce;
              f.ghFormula = lpXName[i].ghNameDef;     
              xl_ParseFormula(lpSS, lpss, (LPyyFORMULA)&f, szFormula, NULL, &bType, NULL);
            }

            SSSetCustomName(lpBook->hWnd, lptstrName, szFormula);
            tbGlobalUnlock(lpXName[i].ghName);
            if (lpXName[i].ghNameDef)
              tbGlobalUnlock(lpXName[i].ghNameDef);
          }
        }
        lpss->fNamesSet = TRUE;
      }

      WriteOutW2(_T("REFMODE"), *(LPOFFSET(lpBiff, 4)));
    }
    break;
    case xlROW:
    {
      yROW row;

      memset((LPyROW)&row, 0, sizeof(yROW));
      memcpy((LPyROW)&row, LPOFFSET(lpBiff, 4), sizeof(yROW));
      bif_SetRowProps(lpss, lpSS, (LPyROW)&row, FALSE);
// 19989, 15436 >>
      {
        LPyROW lp = NULL;
        Alloc(&lpss->ghRowInfos, (lpss->dwRowInfoCount+1) * sizeof(yROW));
        lp = (LPyROW)tbGlobalLock(lpss->ghRowInfos);
        memcpy(&(lp[lpss->dwRowInfoCount]), &row, sizeof(yROW));
        tbGlobalUnlock(lpss->ghRowInfos);
        lpss->dwRowInfoCount++;
      }
//<< 19989, 15436
      WriteOutW2(_T("ROW"), row.wRow);
    }
    break;
    case xlSAVERECALC:
    {
      lpss->SubStream.wSaveRecalc = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("SAVERECALC"), *(LPOFFSET(lpBiff, 4)));
    }
    break;
    case xlSELECTION:
    {
      memset((LPySEL)&(lpss->SubStream.sel), 0, sizeof(ySEL));
      memcpy((LPySEL)&(lpss->SubStream.sel), LPOFFSET(lpBiff, 4), sizeof(ySEL)-sizeof(TBGLOBALHANDLE));
  
      bif_LoadData((TBGLOBALHANDLE FAR *)&(lpss->SubStream.sel.ghRefs), 
                   (WORD)(lpss->SubStream.sel.wRefs*sizeof(yyREF)), 
                   (LPBYTE)LPOFFSET(lpBiff, 13), FALSE, FALSE, 0);
#ifdef SS_V70
      SSSetActiveCell(lpBook->hWnd, lpss->SubStream.sel.wCol+1, lpss->SubStream.sel.wRow+1);  
      //Set Multi-Block Selection to ALL TRUE.
      SSSetBool(lpBook->hWnd, SSB_ALLOWMULTIBLOCKS, TRUE);
      SSSetSelBlockOptions(lpBook->hWnd, SS_SELBLOCK_COLS | 
                                         SS_SELBLOCK_ROWS | 
                                         SS_SELBLOCK_BLOCKS | 
                                         SS_SELBLOCK_ALL);
      if (lpss->SubStream.sel.wRefs > 0)
      {
        LPyyREF         lpRef = (LPyyREF)tbGlobalLock(lpss->SubStream.sel.ghRefs);
        WORD            w;
        TBGLOBALHANDLE  gh = tbGlobalAlloc(GHND, lpss->SubStream.sel.wRefs * sizeof(SS_SELBLOCK));
        LPSS_SELBLOCK   lpSel = (LPSS_SELBLOCK)tbGlobalLock(gh);
        
        for (w=0; w<lpss->SubStream.sel.wRefs; w++)
        {
//          lpSel[w].UL.Col = lpRef[w].bCol1+1;
//          lpSel[w].UL.Row = lpRef[w].wRow1+1;
//          lpSel[w].LR.Col = lpRef[w].bCol2+1;
//          lpSel[w].LR.Row = lpRef[w].wRow2+1;

          if (lpRef[w].bCol1 == 0xFF)
            lpSel[w].UL.Col = -1;
          else
            lpSel[w].UL.Col = lpRef[w].bCol1+1;

          if (lpRef[w].bCol2 == 0xFF)
            lpSel[w].LR.Col = -1;
          else
            lpSel[w].LR.Col = lpRef[w].bCol2+1;

          if (lpRef[w].wRow1 == 0x3FFF)
            lpSel[w].UL.Row = -1;
          else
            lpSel[w].UL.Row = lpRef[w].wRow1+1;

          if (lpRef[w].wRow2 == 0x3FFF)
            lpSel[w].LR.Row = -1;
          else
            lpSel[w].LR.Row = lpRef[w].wRow2+1;
        }
        tbGlobalUnlock(lpss->SubStream.sel.ghRefs);

        SSSetMultiSelBlocks(lpBook->hWnd, lpSel, lpss->SubStream.sel.wRefs);
        tbGlobalUnlock(gh);
        tbGlobalFree(gh);
      }
      else  if (lpss->SubStream.sel.wRefs)
      {
        SS_CELLCOORD sscell;
        sscell.Col = lpss->SubStream.sel.wCol+1;
        sscell.Row = lpss->SubStream.sel.wRow+1;

        SSSetSelectBlock(lpBook->hWnd, (LPSS_CELLCOORD)&sscell, (LPSS_CELLCOORD)&sscell);  
      }

      SSSetBool(lpBook->hWnd, SSB_RETAINSELBLOCK, TRUE);  //RAP02a

      SSSetBool(lpBook->hWnd, SSB_RETAINSELBLOCK, TRUE);  //RAP02a

      //ShowScrollbars
      SSSetBool(lpBook->hWnd, SSB_HORZSCROLLBAR, (BOOL)lpss->wb.fDspHScroll);
      SSSetBool(lpBook->hWnd, SSB_VERTSCROLLBAR, (BOOL)lpss->wb.fDspVScroll);
#endif

      WriteOutDW2(_T("SELECTION"), lpss->SubStream.sel.wRefs);
    }
    break;
    case xlSETUP:
    {
//#ifdef SS_V70
      ySETUP setup;
      memcpy(&setup, LPOFFSET(lpBiff, 4), sizeof(ySETUP));
//      lpSS->PrintOptions.pf.fDrawBorder = setup.wPrintGrid;

      if (setup.fNoPls == 0)
        lpSS->PrintOptions.pf.wOrientation = (USHORT)((setup.fNoOrient? 0:(setup.fLandscape? SS_PRINTORIENT_PORTRAIT:SS_PRINTORIENT_LANDSCAPE)));
      lpSS->PrintOptions.pf.fDrawColors = setup.fNoColor? 0:1; 
#ifdef SS_V70
      if (setup.fNoPls == 0)
        lpSS->PrintOptions.pf.ZoomFactor = (double)((double)setup.wScale/100.0);
      lpSS->PrintOptions.pf.nBestFitPagesWide = setup.wFitWidth;
      lpSS->PrintOptions.pf.nBestFitPagesTall = setup.wFitHeight;
#endif
      lpSS->PrintOptions.pf.lFirstPageNumber = setup.wPageStart;
//#else
//      memcpy(&(lpss->SubStream.setup), LPOFFSET(lpBiff, 4), sizeof(ySETUP));
//#endif
      WriteOut(_T("SETUP"), NULL);
    }
    break;
    case xlSTRING:
    {
      if (lpss->fStringFmlaFlag)
      {
        yCELLVAL cell;
        yyLABEL  label;
  	    BOOL     fWChar = FALSE;

#ifndef _UNICODE
        LPBIFFSTR lpBiffstr = (LPBIFFSTR)LPOFFSET(lpBiff, 4);
	      fWChar = lpBiffstr->fHighByte;
#else
        fWChar = TRUE;
#endif
                    
        memset((LPyCELLVAL)&cell, 0, sizeof(yCELLVAL));
        memset((LPyyLABEL)&label, 0, sizeof(yyLABEL));
        label.wCol = lpss->StringFmlaCell.wCol;
        label.wRow = lpss->StringFmlaCell.wRow;
        label.cch = *(LPWORD)LPOFFSET(lpBiff, 4);
        if (lpss->bof.wSubStrType == 0x10 || lpss->bof.wSubStrType == 0x05)
        {
          int nLen = label.cch;
          if (fWChar)
            nLen *= 2;
          nLen += sizeof(BIFFSTR)-1;
          bif_LoadData((TBGLOBALHANDLE FAR *)&(label.ghString), 
                       (WORD)nLen,
                       (LPBYTE)LPOFFSET(lpBiff,4), FALSE, FALSE, 0);
      
          cell.wCellType = xlLABEL;
          cell.wCol = label.wCol;
          cell.wRow = label.wRow;
          cell.ghCell = label.ghString;

          bif_LoadCell(lpss, lpSS, (LPyCELLVAL)&cell);

  
          tbGlobalFree(cell.ghCell); 
        }
      }
      WriteOutDW2(_T("STRING"), NULL);
    }
    break;
    case xlLEFTMARGIN:
    {
      double margin = 0;
      memcpy(&margin, LPOFFSET(lpBiff, 4), sizeof(double));
//      lpSS->PrintOptions.pf.fMarginLeft = (float)margin*dxPixelsPerInch;
      lpSS->PrintOptions.pf.fMarginLeft = (float)margin;
    }
    break;
    case xlTOPMARGIN:
    {
      double margin = 0;
      memcpy(&margin, LPOFFSET(lpBiff, 4), sizeof(double));
//      lpSS->PrintOptions.pf.fMarginTop = (float)margin*dyPixelsPerInch;
      lpSS->PrintOptions.pf.fMarginTop = (float)margin;
    }
    break;
    case xlRIGHTMARGIN:
    {
      double margin = 0;
      memcpy(&margin, LPOFFSET(lpBiff, 4), sizeof(double));
//      lpSS->PrintOptions.pf.fMarginRight = (float)margin*dxPixelsPerInch;
      lpSS->PrintOptions.pf.fMarginRight = (float)margin;
    }
    break;
    case xlHORIZONTALPAGEBREAKS:
    {
      int breakCount = 0;
      TBGLOBALHANDLE ghPageBreaks = 0;
      LPPAGEBREAK lppb = NULL;
      int i;

      memcpy(&breakCount, LPOFFSET(lpBiff, 4), sizeof(short));
      Alloc(&ghPageBreaks, breakCount * sizeof(PAGEBREAK));
      lppb = (LPPAGEBREAK)tbGlobalLock(ghPageBreaks);

      memcpy(lppb, LPOFFSET(lpBiff, 4 + sizeof(short)), breakCount*sizeof(PAGEBREAK));

      for (i=0; i<breakCount; i++)
      {
        SSSetRowPageBreak(lpBook->hWnd, lppb[i].index, TRUE);
      }

      tbGlobalUnlock(ghPageBreaks);
      tbGlobalFree(ghPageBreaks);
    }
      break;
    case xlVERTICALPAGEBREAKS:
    {
      int breakCount = 0;
      TBGLOBALHANDLE ghPageBreaks = 0;
      LPPAGEBREAK lppb = NULL;
      int i;

      memcpy(&breakCount, LPOFFSET(lpBiff, 4), sizeof(short));
      Alloc(&ghPageBreaks, breakCount * sizeof(PAGEBREAK));
      lppb = (LPPAGEBREAK)tbGlobalLock(ghPageBreaks);

      memcpy(lppb, LPOFFSET(lpBiff, 4 + sizeof(short)), breakCount*sizeof(PAGEBREAK));

      for (i=0; i<breakCount; i++)
      {
        SSSetColPageBreak(lpBook->hWnd, lppb[i].index, TRUE);
      }

      tbGlobalUnlock(ghPageBreaks);
      tbGlobalFree(ghPageBreaks);
    }
      break;
    case xlBOTTOMMARGIN:
    {
      double margin = 0;
      memcpy(&margin, LPOFFSET(lpBiff, 4), sizeof(double));
//      lpSS->PrintOptions.pf.fMarginBottom = (float)margin*dyPixelsPerInch;
      lpSS->PrintOptions.pf.fMarginBottom = (float)margin;
    }
    break;
    case xlSORT:
    {
#ifndef V7_TEST
      ySORT sort;

      memset((LPySORT)&sort, 0, sizeof(ySORT));
      memcpy((LPySORT)&sort, LPOFFSET(lpBiff, 4), sizeof(ySORT)-3*sizeof(TBGLOBALHANDLE));

      if (sort.bKey1Len)
        bif_LoadData((TBGLOBALHANDLE FAR *)&sort.ghKey1, 
                     (WORD)sort.bKey1Len, 
                     (LPBYTE)LPOFFSET(lpBiff, 10), TRUE, FALSE, 0);//!!!RAP - the docs say the offset is 9
      if (sort.bKey2Len)
        bif_LoadData((TBGLOBALHANDLE FAR *)&sort.ghKey2, 
                     (WORD)sort.bKey2Len, 
                     (LPBYTE)LPOFFSET(lpBiff, (10+sort.bKey1Len)), TRUE, FALSE, 0);
      if (sort.bKey3Len)
        bif_LoadData((TBGLOBALHANDLE FAR *)&sort.ghKey3, 
                     (WORD)sort.bKey3Len, 
                     (LPBYTE)LPOFFSET(lpBiff, (10+sort.bKey1Len+sort.bKey2Len)), TRUE, FALSE, 0);

      bif_LoadData((TBGLOBALHANDLE FAR *)&(lpss->SubStream.ghSorts), 
                   (WORD)sizeof(ySORT), 
                   (LPBYTE)&sort, FALSE, FALSE, lpss->SubStream.dwSortCount*sizeof(ySORT));

      lpss->SubStream.dwSortCount++;
     
#ifdef _PARSE2FILE
      WriteOut(_T("SORT"), NULL);
      if (sort.ghKey1)
        WriteOutGH2(_T("    "), sort.ghKey1);
      if (sort.ghKey2)
        WriteOutGH2(_T("    "), sort.ghKey2);
      if (sort.ghKey3)
        WriteOutGH2(_T("    "), sort.ghKey3);
#endif
#endif
    }
    break;
    case xlVCENTER:
    {
      lpss->SubStream.wVCenter = *(LPOFFSET(lpBiff, 4));
      WriteOutW2(_T("VCENTER"), *(LPOFFSET(lpBiff, 4)));
    }
    break;
    case xlWINDOW2:
    {
      yWORKSHEET8 ws;
      COLORREF    color = 0;

      memcpy(&ws, LPOFFSET(lpBiff, 4), sizeof(yWORKSHEET));

      //ShowGrid
      SSSetGridType(lpBook->hWnd, (WORD)((ws.fDspGrid==1)?
                                (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL | SS_GRID_SOLID):0));
      if (RGBCOLOR_DEFAULT != (color = xl_ColorFromIndex(ws.dwiHdr)))
      {
#ifdef SS_V70
        lpSS->GridColor = color;
#else
        SSSetGridColor(lpBook->hWnd, color);
#endif
      }
      //ShowHeaders 
      SSSetBool(lpBook->hWnd, SSB_SHOWCOLHEADERS, (BOOL)ws.fDspRwCol);
      SSSetBool(lpBook->hWnd, SSB_SHOWROWHEADERS, (BOOL)ws.fDspRwCol);
      SSSetColor(lpBook->hWnd, lpss->xlSS_HEADER, SS_ALLROWS, xl_ColorFromIndex(ws.dwiHdr), color);

      if (ws.fFrozen)
        fFrozen = true;

//15436 >>
      {
        HDC    hDC = GetDC(lpBook->hWnd);
        double dblTwips = MulDiv(lpss->DefRowHeight.wDefRowHeight, TWIP_PER_PT, 20);
        double dblPixels = MulDiv((int)dblTwips, GetDeviceCaps(hDC, LOGPIXELSY), 1440);
        SSSetRowHeightInPixels(lpBook->hWnd, SS_ALLROWS, (WORD)dblPixels);
        ReleaseDC(lpBook->hWnd, hDC);
      }
//<< 15436
//19989 >>
      {
        DWORD i;
        LPyROW lp = (LPyROW)tbGlobalLock(lpss->ghRowInfos);
        for (i=0; i<lpss->dwRowInfoCount; i++)
        {
          bif_SetRowProps(lpss, lpSS, (LPyROW)&lp[i], true);
        }
        if (lp != NULL)
        {
          tbGlobalUnlock(lpss->ghRowInfos);
          tbGlobalFree(lpss->ghRowInfos);
          lpss->ghRowInfos = 0;
          lpss->dwRowInfoCount = 0;
        }
      }
//<< 19989

      WriteOut(_T("WINDOW2-worksheet"), NULL);
    }
    break;
    case xlWSBOOL:
    {
      memcpy(&(lpss->SubStream.wsbool), LPOFFSET(lpBiff, 4), sizeof(yWSBOOL));
      WriteOut(_T("WSBOOL"), NULL);
    }
    break;

  } //switch
   
  SS_BookUnlockSheetIndex(lpBook, 0); //nSheetNum);

  return 0;
}

#ifdef SS_V70
TBGLOBALHANDLE bif_GetSupBookName(LPSSXL lpss, int nSupBookIndex, bool fSheetName, int index)
{
  TBGLOBALHANDLE   gh = 0;
  LPTBGLOBALHANDLE lpghSUPBOOKs = (LPTBGLOBALHANDLE)tbGlobalLock(lpss->ghSUPBOOKData);
  LPBYTE           lpSUPBOOK = (LPBYTE)tbGlobalLock(lpghSUPBOOKs[nSupBookIndex]);
  WORD             sheetCount;
  WORD             w;
  long             pos = 0;

  memcpy(&sheetCount, LPOFFSET(lpSUPBOOK, pos), sizeof(WORD));
  pos += 2;
  memcpy(&w, LPOFFSET(lpSUPBOOK, 2), sizeof(WORD));
  pos += 2;
    
  if ((short)index == -1 || (short)index == -2) //-1 is end of 3D or External References, 
    return 0;                                   //-2 is end of Internal References

  if (w == 0x0401) // 3D Reference
  {
    if (fSheetName)
    {
      LPxBUNDLESHT8 lpBS = (LPxBUNDLESHT8)tbGlobalLock(lpss->ghBundleShts);
      LPTSTR        sheetName = (LPTSTR)tbGlobalLock(lpBS[index].ghSheetName);
      LPTSTR        name;
      
      Alloc(&gh, (lstrlen(sheetName)+1)*sizeof(TCHAR));
      name = (LPTSTR)tbGlobalLock(gh);
      _tcscpy(name, sheetName);
      tbGlobalUnlock(gh);
      tbGlobalUnlock(lpBS[index].ghSheetName);
      tbGlobalUnlock(lpss->ghBundleShts);
      return gh;
    }
    else
      return 0;
  }
  else if (w == 0x3A01 || w == 0x0001) // AddIn function
  {
  }
  else
  {
    WORD   encodedFileNameLen = w;
    BYTE   biffstrFlag;
    LPTSTR lptstr;
    WORD   bytesRead = 0;
    
    memcpy(&biffstrFlag, LPOFFSET(lpSUPBOOK, pos), sizeof(BYTE));
    pos += 1;
    
    if (fSheetName)
    {
      WORD  sheetNameLen=0;
      TCHAR sheetName[256];
      int  i=0;
      
      memset(sheetName, 0, 256*sizeof(TCHAR));
      pos += (encodedFileNameLen * (biffstrFlag?2:1));

      while (i <= index)
      {
        memcpy(&sheetNameLen, LPOFFSET(lpSUPBOOK, pos), sizeof(WORD));
        pos += sizeof(WORD);
        memcpy(&biffstrFlag, LPOFFSET(lpSUPBOOK, pos), sizeof(BYTE));
        pos ++;
        if (i == index)
        {
          memcpy(sheetName, LPOFFSET(lpSUPBOOK, pos), (sheetNameLen * (biffstrFlag?2:1)));
          break;
        }
        else
          i++;
        pos += (sheetNameLen * (biffstrFlag?2:1));
      }
      
#ifdef UNICODE
      if (biffstrFlag)
      {
        Alloc(&gh, (sheetNameLen+1) * sizeof(TCHAR));
        lptstr = (LPTSTR)tbGlobalLock(gh);
        _tcscpy(lptstr, sheetName);
      }
      else
      {
        int    nLen = 0;
        LPWSTR lpwstr = NULL;
        Alloc(&gh, (sheetNameLen+1) * sizeof(WCHAR));
        lpwstr = (LPWSTR)tbGlobalLock(gh);
        nLen = (WORD)(MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)sheetName,
                                          sheetNameLen, lpwstr, sheetNameLen));
      }
#else
      if (biffstrFlag)
      {
        int    nLen = 0;
        LPTSTR lptstr = NULL;
        Alloc(&gh, (sheetNameLen+1) * sizeof(TCHAR));
        lptstr = (LPTSTR)tbGlobalLock(gh);
        nLen = (short)WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, 
                                          (LPWSTR)sheetName, sheetNameLen,
                                          lptstr, sheetNameLen,
                                          NULL, NULL);
      }
      else
      {
        Alloc(&gh, (sheetNameLen+1) * sizeof(TCHAR));
        lptstr = (LPTSTR)tbGlobalLock(gh);
        _tcscpy(lptstr, sheetName);
      }
#endif      
      
      tbGlobalUnlock(gh);
    }
    else
    {
      BYTE  encodeChar;
    
      Alloc(&gh, 256*sizeof(TCHAR));
      lptstr = (LPTSTR)tbGlobalLock(gh);
  
      memcpy(&encodeChar, LPOFFSET(lpSUPBOOK, pos), sizeof(BYTE));
      pos += 1;
    
      if (encodeChar == 0x01)
      {
        short backSlashIndex = 0;
        short len;
        while (bytesRead < encodedFileNameLen-1)
        {
          BYTE b = *LPOFFSET(lpSUPBOOK, pos);
          if (b==0x01 || b==0x02 || b==0x03 || b==0x04)
            pos++;
          else
            b = 0;
        
          switch (b)
          {
            case 0x01:
            {
              TCHAR t = *LPOFFSET(lpSUPBOOK, pos);
              pos += sizeof(TCHAR);
              bytesRead += sizeof(TCHAR);

              if (t == '@')
                _tcscat(lptstr, _T("\\\\"));
              else
              {
                lptstr[lstrlen(lptstr)] = t;
                _tcscat(lptstr, _T(":\\"));
              }
              backSlashIndex = (short)lstrlen(lptstr);
            }
            break;
            case 0x02:
              _tcscat(lptstr, _T("\\"));
              bytesRead++;
              backSlashIndex = (short)lstrlen(lptstr);
            break;
            case 0x03:
              _tcscat(lptstr, _T("\\"));
              bytesRead++;
              backSlashIndex = (short)lstrlen(lptstr);
              break;
            case 0x04:
              _tcscat(lptstr, _T("..\\"));
              bytesRead++;
              backSlashIndex = (short)lstrlen(lptstr);
            break;
            default:
              TCHAR t = *LPOFFSET(lpSUPBOOK, pos);
              pos += sizeof(TCHAR);
              bytesRead += sizeof(TCHAR);
              lptstr[lstrlen(lptstr)] = t;
            break;
          }
        }

        len = (short)lstrlen(lptstr);
        for (int i= len; i>backSlashIndex; i--)
          lptstr[i] = lptstr[i-1];
        lptstr[backSlashIndex] = (TCHAR)'[';
        _tcscat(lptstr, _T("]"));

        tbGlobalUnlock(gh);
      }
      else if (encodeChar == 0x02)
      {
        // external reference to the own document or sheet
        return 0;
      }
      else
      {
      }
    }
  }
  return gh;
}
#endif

/***********************************************************************
* Name:   bif_LoadData - Append the data to a buffer, and perform any
*                        necessary conversions on the data.
*
* Usage:  short bif_LoadData(TBGLOBALHANDLE FAR *lpgh, WORD wLen, LPBYTE lp,
*                            BOOL fNullTerm, BOOL fWChar, LONG lSize)
*            lpgh - pointer to the buffer to append to
*            wLen - length of the data to be added.
*            lp - pointer to the data to be added.
*            fNullTerm - is the data to have a NULL terminator added?
*            fWChar - is the data to be converted to wide chars?
*            lSize - current size of the buffer.
*
* Return: short - Success or Failure
***********************************************************************/
short bif_LoadData(TBGLOBALHANDLE FAR *lpgh, WORD wLen, LPBYTE lp, BOOL fNullTerm, BOOL fWChar, LONG lSize)
{
  LPBYTE lpb = NULL;
  WORD   wAllocLen = wLen;

  fNullTerm? wAllocLen++:0;
  fWChar? wAllocLen*=2, wLen*=2:0;

  if (*lpgh == 0 || lSize == 0)
  {
    *lpgh = tbGlobalAlloc(GHND, wAllocLen + lSize);
    if (*lpgh == (TBGLOBALHANDLE)0)
      return 1;
    lpb = (LPBYTE)tbGlobalLock(*lpgh);
  }
  else
  {
    *lpgh = tbGlobalReAlloc(*lpgh, wAllocLen + lSize, GMEM_ZEROINIT);
    if (*lpgh == (TBGLOBALHANDLE)0)
      return 1;
    lpb = (LPBYTE)tbGlobalLock(*lpgh);
    lpb = LPOFFSET(lpb, lSize);
  }

  memcpy(lpb, lp, wLen);

  fNullTerm? lpb[wAllocLen-1] = 0:0;
  fWChar? lpb[wAllocLen-2] = 0:0;

  tbGlobalUnlock(*lpgh);
  
  return 0;
}

/***********************************************************************
* Name:   bif_LoadDataBoc - Append the data to a buffer, and perform any
*                        necessary conversions on the data.
*
* Usage:  short bif_LoadData(TBGLOBALHANDLE FAR *lpgh, WORD wLen, LPBYTE lp,
*                            BOOL fNullTerm, BOOL fWChar, LONG lSize)
*            lpgh - pointer to the buffer to append to
*            wLen - length of the data to be added.
*            lp - pointer to the data to be added.
*            fNullTerm - is the data to have a NULL terminator added?
*            fWChar - is the data to be converted to wide chars?
*            lSize - current size of the buffer.
*            fUnicode - the source buffer is base on unicode
*
* Return: WORD - Length of append /-1 Failure
*
* Add by BOC 99.4.7 (hyt)
***********************************************************************/
WORD bif_LoadDataBoc(TBGLOBALHANDLE FAR *lpgh, WORD wLen, LPBYTE lp, BOOL fNullTerm, BOOL fWChar, LONG lSize,BOOL fUnicode)
{
  LPBYTE lpb = NULL;
  WORD   wAllocLen = wLen;

  fNullTerm? wAllocLen++:0;
  fUnicode?wAllocLen*=2:0;
  fWChar? wAllocLen*=2, wLen*=2:0;

  if (*lpgh == 0 || lSize == 0)
  {
    *lpgh = tbGlobalAlloc(GHND, wAllocLen + lSize);
    if (*lpgh == (TBGLOBALHANDLE)0)
      return (WORD)-1;
    lpb = (LPBYTE)tbGlobalLock(*lpgh);
  }
  else
  {
    *lpgh = tbGlobalReAlloc(*lpgh, wAllocLen + lSize, GMEM_ZEROINIT);
    if (*lpgh == (TBGLOBALHANDLE)0)
      return (WORD)-1;
    lpb = (LPBYTE)tbGlobalLock(*lpgh);
    lpb = LPOFFSET(lpb, lSize);
  }

  int nStringLen =wLen;
  if(fUnicode)
  {
      nStringLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, 
                                           (LPWSTR)lp, wLen,
                                           (LPSTR)lpb, wAllocLen,
                                           NULL, NULL);

      wAllocLen = (WORD)(nStringLen + (fNullTerm?1:0) + (fWChar?1:0));
      tbGlobalUnlock(*lpgh);

      *lpgh = tbGlobalReAlloc(*lpgh, wAllocLen + lSize, GMEM_ZEROINIT);
      if (*lpgh == (TBGLOBALHANDLE)0)
        return (WORD)-1;
      lpb = (LPBYTE)tbGlobalLock(*lpgh);
      lpb = LPOFFSET(lpb, lSize);
  }
  else
    memcpy(lpb, lp, wLen);

  fNullTerm? lpb[wAllocLen-1] = 0:0;
  fWChar? lpb[wAllocLen-2] = 0:0;

  tbGlobalUnlock(*lpgh);
  
  return (WORD)nStringLen;
}

/***********************************************************************
* Name:   bif_Build - Build a BIFF structure and add it to the buffer
*
*
* Usage:  short bif_Build(LPBIFF lpBiff, LPVOID lp, LPVOID lpGH,
*                         LPLONG lplSize, LPVOID lpExtra)
*            lpBiff - pointer to the BIFF structure to build
*            lp - pointer to the Spread structure
*            lpGH - pointer to the buffer
*            lplSize - pointer to the size of the buffer
*            lpExtra - any extra data that needs to be added to the
*                      BIFF structure.
*
* Return: short - Success or Failure
***********************************************************************/
short bif_Build(LPBIFF lpBiff, LPVOID lp, LPVOID lpGH, LPLONG lplSize, LPVOID lpExtra)
{
  LPSS_BOOK     lpBook = NULL;
  LPSPREADSHEET lpSS = NULL;
  HDC           hDC = GetDC(GetDesktopWindow());
  //short         dyPixelsPerInch = (short)GetDeviceCaps(hDC, LOGPIXELSY);
#ifdef SS_V70
  SS_COORD      colsFrozen = 0;
  SS_COORD      rowsFrozen = 0;
#endif
  int i;

  if (lpBiff->recnum != xlBOF_BIFF5_7_8)
  {
    lpBook = (LPSS_BOOK)lp;
    lpSS = SS_BookLockSheet(lpBook);
    ReleaseDC(GetDesktopWindow(), hDC);
    hDC = GetDC(lpBook->hWnd);
  }

  switch(lpBiff->recnum)
  {
    case xl1904:
    {
//      short nStartYear = DateGetJulianStart();
//      WORD w1904 = (nStartYear == 1904?TRUE:FALSE);
      WORD w1904 = 0;
      bif_SaveData(lpBiff, (LPVOID)&w1904, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlBOF_BIFF5_7_8:
    {
      zBOF  bof;
      short sBOF = *(LPSHORT)lp;
      //write out the BOF header
      memset(&bof, 0, sizeof(zBOF));
      bof.wVerNum = 0x0600;
      if (sBOF == 0) // the BOF is a Workbook Global
        bof.wSubStrType = 0x0005;
      else if (sBOF == 1) // the BOF is a Worksheet
        bof.wSubStrType = 0x0010;
      bof.wBuild = 0x0DBB;
      bof.wYear = 0x07CC;
      bof.dwfh = 0x00C1;
      bof.dwsfo = 0x0006;
      bif_SaveData(lpBiff, (LPVOID)&bof, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlBUNDLESHEET:
    {
      xBUNDLESHT8 sheet;
      LPTSTR      lptstr;
      BOOL        fSheetVisible = TRUE;
      bool        isHighByte = false;

#ifdef SS_V70
      fSheetVisible = (*(LPBYTE)lpExtra==1?TRUE:FALSE);
      lpExtra = (LPBYTE)lpExtra + sizeof(BYTE);
#endif
      memset((LPxBUNDLESHT8)&sheet, 0, sizeof(xBUNDLESHT8));
      sheet.dt = 0;
      sheet.hsState = (fSheetVisible?0:1);
     
      sheet.wSheetNameLen = (WORD)lstrlen((LPTSTR)lpExtra);
      if (sheet.wSheetNameLen)
      {
#ifdef _UNICODE // if Unicode, convert WCHAR string to CHAR string
        UINT len = sheet.wSheetNameLen;
        // Allocate twice as many bytes.
        sheet.ghSheetName = tbGlobalAlloc(GHND,  len * sizeof(WCHAR));
        lptstr = (LPTSTR)tbGlobalLock(sheet.ghSheetName);
        //set the unicode format flags
        sheet.wSheetNameLen |= 0x100;

//14294  #ifdef SPREAD_JPN
        //Modify by BOC 99.4.7(hyt)--------------------------------------------
        //DBCS must save as unicode format
        memcpy(lptstr, (LPTSTR)lpExtra, (sheet.wSheetNameLen & 0xFF)*sizeof(WCHAR)); // exclude the null-term.

//14294  #else
//        // Convert to multi-byte string
//        sheet.wSheetNameLen = WideCharToMultiByte(CP_ACP, 0, 
//          (LPCWSTR)lpExtra, len, (LPSTR)lptstr, len * sizeof(WCHAR),
//          NULL, NULL);
//  #endif

#else
  #ifdef SPREAD_JPN
        //Modify by BOC 99.4.7(hyt)--------------------------------------------
        //DBCS must save as unicode format
        //sheet.ghSheetName = tbGlobalAlloc(GHND, sheet.wSheetNameLen);
        //lptstr = (LPTSTR)tbGlobalLock(sheet.ghSheetName);
        //memcpy(lptstr, (LPTSTR)lpExtra, sheet.wSheetNameLen); // exclude the null-term.
        sheet.ghSheetName = tbGlobalAlloc(GHND, sheet.wSheetNameLen*sizeof(WCHAR));
        lptstr = (LPTSTR)tbGlobalLock(sheet.ghSheetName);
        sheet.wSheetNameLen = (WORD)(MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpExtra,
                                           sheet.wSheetNameLen,
                                           (LPWSTR)lptstr, sheet.wSheetNameLen*sizeof(WCHAR)));

  #else
//20862 >>
      int j;
      for (j=0; j<sheet.wSheetNameLen; j++)
      {
        BYTE* pByte = (BYTE*)(&((BYTE*)lpExtra)[j]);
        if(IsDBCSLeadByte(*pByte) || _ismbbkana(*pByte))
  	    {
          isHighByte = true;
          break;
	      }
        else if(*pByte > 127)
        {
          isHighByte = true;
          break;
        }
  	    else if(*pByte ++ ==0x0d && *pByte==0x0a)
	      {
          j++;
          continue;
	      }
      }        
       
      if (isHighByte)
      {
        sheet.ghSheetName = tbGlobalAlloc(GHND, sheet.wSheetNameLen*sizeof(WCHAR));
        lptstr = (LPTSTR)tbGlobalLock(sheet.ghSheetName);
        sheet.wSheetNameLen = (WORD)(MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpExtra,
                                             sheet.wSheetNameLen,
                                             (LPWSTR)lptstr, sheet.wSheetNameLen*sizeof(WCHAR)));
      }
      else
//<< 20862
      {
        sheet.ghSheetName = tbGlobalAlloc(GHND, sheet.wSheetNameLen);
        lptstr = (LPTSTR)tbGlobalLock(sheet.ghSheetName);
        memcpy(lptstr, (LPTSTR)lpExtra, sheet.wSheetNameLen); // exclude the null-term.
      }
  #endif

#endif
        tbGlobalUnlock(sheet.ghSheetName);
      }
#ifdef SPREAD_JPN
      sheet.dwBOFPos = *lplSize + sizeof(BIFF) + lpBiff->datalen + (sheet.wSheetNameLen & 0xFF)*sizeof(WCHAR) + 
                       sizeof(BIFF) + sizeof(zCOUNTRY) +
                       sizeof(BIFF);

      //set the unicode format flags
      sheet.wSheetNameLen |= 0x100;
#else
//20862 >>
      if (isHighByte)
      {
        sheet.dwBOFPos = *lplSize + sizeof(BIFF) + lpBiff->datalen + (sheet.wSheetNameLen & 0xFF)*sizeof(WCHAR) + 
                         sizeof(BIFF) + sizeof(zCOUNTRY) +
                         sizeof(BIFF);
            //set the unicode format flags
        sheet.wSheetNameLen |= 0x100; 
      }
      else
//<< 20862
        sheet.dwBOFPos = *lplSize + sizeof(BIFF) + lpBiff->datalen + (sheet.wSheetNameLen & 0xFF)*sizeof(TCHAR) + 
                         sizeof(BIFF) + sizeof(zCOUNTRY) +
                         sizeof(BIFF);
#endif

// hatakeyama@boc.co.jp 12/12-2000 for Seetname Access violation.
// Prevents an Access Violation when the sheet name is set.
#ifdef SPREAD_JPN
      bif_SaveData(lpBiff, (LPVOID)&sheet, sheet.ghSheetName, (short)((sheet.wSheetNameLen & 0xFF) * sizeof(WCHAR)), lpGH, lplSize);
#else
//20862 >>
      if (isHighByte)
        bif_SaveData(lpBiff, (LPVOID)&sheet, sheet.ghSheetName, (short)((sheet.wSheetNameLen & 0xFF) * sizeof(WCHAR)), lpGH, lplSize);
      else
//<< 20862
        bif_SaveData(lpBiff, (LPVOID)&sheet, sheet.ghSheetName, (short)((sheet.wSheetNameLen & 0xFF) * sizeof(TCHAR)), lpGH, lplSize);
#endif
  
      if (sheet.wSheetNameLen)
        tbGlobalFree(sheet.ghSheetName);
    }
    break;
    case xlCALCCOUNT:
    {
      short sIterations;
      double dfMaxChange;
      SSGetIteration(lpBook->hWnd, (short FAR *)&sIterations, (double far *)&dfMaxChange);

      bif_SaveData(lpBiff, (LPVOID)&sIterations, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlCALCMODE:
    {
      BOOL fAutoCalc = SSGetBool(lpBook->hWnd, SSB_AUTOCALC);
      bif_SaveData(lpBiff, (LPVOID)&fAutoCalc, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlCODEPAGE:
    {
//      WORD wCodePage = 0xB0E4;  // ANSI (Microsoft Windows)
      WORD wCodePage = 0x04B0;  // ANSI (Microsoft Windows)
      bif_SaveData(lpBiff, (LPVOID)&wCodePage, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlCOLINFO:
    {
      LPyCOLINFO lpColInfo = (LPyCOLINFO)lpExtra;
      double     dblColWidth = 0.0;
      LPSS_COL   lpCol = SS_LockColItem(lpSS, lpColInfo->wColFirst+lpSS->Col.HeaderCnt);
      HDC        hDC = GetDC(lpBook->hWnd);
      double     dblColWidthPixels = -1;
      HFONT      hFont = SSGetFont(lpBook->hWnd, SS_ALLCOLS, SS_ALLROWS);     
      HFONT      hFontOld = (HFONT)SelectObject(hDC, hFont);
      BOOL       fDefaultWidth = FALSE;

      if (lpCol != NULL)
        dblColWidthPixels = lpCol->dColWidthInPixels;

      if (dblColWidthPixels == -1)
      {
        fDefaultWidth = TRUE;
        if (lpColInfo->fHidden)
          SSGetColWidth(lpBook->hWnd, SS_ALLCOLS, (LPDOUBLE)&dblColWidth);
        else
        {
          if (lpColInfo->wColFirst+1 > lpSS->Col.Max)
            SSGetColWidth(lpBook->hWnd, SS_ALLCOLS, (LPDOUBLE)&dblColWidth);
          SSGetColWidth(lpBook->hWnd, lpColInfo->wColFirst+1, (LPDOUBLE)&dblColWidth);
        }
//19561        dblColWidthPixels = (double)SS_ColWidthToPixels(lpSS, dblColWidth) -1;  
        dblColWidthPixels = (double)SS_ColWidthToPixels(lpSS, dblColWidth) -1;  //19561
      }
      else
        dblColWidthPixels = __max(0.0, dblColWidthPixels - 1.0);  // 21574
        // dblColWidthPixels--;  //19792

      int CharWidth = 0;
      SIZE size;
      GetTextExtentPoint32(hDC, _T("0"), 1, &size);
      CharWidth = size.cx;
	  
      if (CharWidth<=0)
  		  CharWidth=1;

      SelectObject(hDC, hFontOld);
      ReleaseDC(lpBook->hWnd, hDC);
	  	
      dblColWidth = dblColWidthPixels/(double)CharWidth;
//#endif
//-----------------------------------------------------------------------------------      

//      lpColInfo->wdxCol = (WORD)(dblColWidth*256);

//      lpColInfo->wdxCol = (WORD)max(0, (MulDiv((int)dblColWidthPixels, 256, (int)CharWidth) - 1));
//      lpColInfo->wdxCol = (WORD)max(0, MulDiv((int)(dblColWidthPixels-1), 256, (int)CharWidth));
      
      lpColInfo->wdxCol = (WORD)(dblColWidth * 256);
      lpColInfo->fHidden = (lpCol?lpCol->fColHidden:0);
      lpColInfo->res1 = 1; // undocumented MS flag -- RAP07a 
      lpColInfo->fCollapsed = 0;
//      lpColInfo->res4 = 0x2C; // undocumented MS flag -- RAP07a

      bif_SaveData(lpBiff, (LPVOID)lpColInfo, NULL, 0, lpGH, lplSize);
      SS_UnlockColItem(lpSS, lpColInfo->wColFirst+lpSS->Col.HeaderCnt);

    }
    break;
    case xlCOUNTRY:
    {
      zCOUNTRY Country;
      memset((LPzCOUNTRY)&Country, 0, sizeof(zCOUNTRY));
      Country.wiCountryDef = 1;
      Country.wiCountryWinIni = 1;
      bif_SaveData(lpBiff, (LPVOID)&Country, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlDBCELL:
    {
      yDBCELL DBCell;
      memset((LPyDBCELL)&DBCell, 0, sizeof(yDBCELL));

      bif_SaveData(lpBiff, (LPVOID)&DBCell, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlDEFCOLWIDTH:
    {
      WORD   wDefColWidth;
      int    iColWidthPixels;
/*
      double dblDefColWidth = 8.0;
      HDC    hDC = GetDC(lpBook->hWnd);
      HFONT  hFont = SSGetFont(lpBook->hWnd, SS_ALLCOLS, SS_ALLROWS);     
      HFONT  hFontOld = (HFONT)SelectObject(hDC, hFont);
      int    CharWidth = 0;

      SSGetColWidthInPixels(lpBook->hWnd, SS_ALLCOLS, &iColWidthPixels);
        
      GetCharWidth32(hDC, '0', '0', &CharWidth);
  	  if(CharWidth<=0)
	  	  CharWidth=1;

      SelectObject(hDC, hFontOld);
      ReleaseDC(lpBook->hWnd, hDC);
	  	
      dblDefColWidth = (double)(iColWidthPixels/(double)CharWidth); //-1 for gridline

      wDefColWidth = (WORD)dblDefColWidth;
//      if (dblDefColWidth-wDefColWidth >=0.5)
//        wDefColWidth++;

*/
      SSGetColWidthInPixels(lpBook->hWnd, SS_ALLCOLS, &iColWidthPixels);

      wDefColWidth = (WORD)((iColWidthPixels+1)/8);

      bif_SaveData(lpBiff, (LPVOID)&wDefColWidth, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlDEFAULTROWHEIGHT:
    {
      yDEFROWHEIGHT DefRowHeight;
      double dblTwips;
      int    iPixels;

      memset((LPyDEFROWHEIGHT)&DefRowHeight, 0, sizeof(yDEFROWHEIGHT));

      SSGetRowHeightInPixels(lpBook->hWnd, SS_ALLROWS, &iPixels);
      dblTwips = MulDiv(iPixels /*gridline*/, 1440, GetDeviceCaps(hDC, LOGPIXELSY));
      DefRowHeight.wDefRowHeight = (WORD)MulDiv((int)dblTwips, 20, (int)TWIP_PER_PT);
      
      DefRowHeight.fUnsynced = 1;
      DefRowHeight.fDyZero = 0;
      DefRowHeight.fExAsc = 0;
      DefRowHeight.fExDsc = 0;
      
      bif_SaveData(lpBiff, (LPVOID)&DefRowHeight, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlDELTA:
    {
      short sIterations;
      double dfMaxChange;
      SSGetIteration(lpBook->hWnd, (short FAR *)&sIterations, (double far *)&dfMaxChange);

      bif_SaveData(lpBiff, (LPVOID)&dfMaxChange, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlDIMENSIONS:
    {
      yDIM8 Dim;
      memset((LPyDIM8)&Dim, 0, sizeof(yDIM8));
      bif_SaveData(lpBiff, (LPVOID)&Dim, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlDSF:
    {
      WORD wDSF = 0;
      bif_SaveData(lpBiff, (LPVOID)&wDSF, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlEOF_BIFF:
    {
      bif_SaveData(lpBiff, (LPVOID)NULL, NULL, 0, lpGH, lplSize);
    }
    break;
#ifdef SS_V80ROBBY
    case xlEXTERNNAME:
    {
      xEXTERNNAME    name;
      LPBYTE         lpb = NULL;
      TBGLOBALHANDLE ghExtra = (TBGLOBALHANDLE)0;
      short          dataLen = lpBiff->datalen;
      memset(&name, 0, sizeof(xEXTERNNAME));
      
      Alloc(&ghExtra, lpBiff->datalen);
      lpb = (LPBYTE)tbGlobalLock(ghExtra);
      memcpy(lpb, lpExtra, lpBiff->datalen);

      lpBiff->datalen = sizeof(xEXTERNNAME) - sizeof(BYTE) - sizeof(WORD) - 2*sizeof(TBGLOBALHANDLE);
      bif_SaveData(lpBiff, (LPVOID)&name, ghExtra, dataLen, lpGH, lplSize);

      tbGlobalFree(ghExtra);
    }
    break;
#endif
    case xlFONT:
    {
      LPxFONT lpFont = (LPxFONT)lpExtra;
      bif_SaveData(lpBiff, (LPVOID)lpExtra, lpFont->ghFontName, (WORD)(sizeof(WCHAR)*lpFont->bFontNameLen), lpGH, lplSize);

      tbGlobalUnlock(lpFont->ghFontName);
    }
    break;
    case xlFOOTER:
    {
      BYTE bFooterLen = 0;
      bif_SaveData(lpBiff, (LPVOID)&bFooterLen, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlFORMAT:
    {
      bif_SaveData(lpBiff, (LPVOID)lpExtra, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlGRIDSET:
    {
      WORD wGridSet = 1;
      bif_SaveData(lpBiff, (LPVOID)&wGridSet, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlGUTS:
    {
      yGUTS Guts;
      
      memset((LPyGUTS)&Guts, 0, sizeof(yGUTS));
      Guts.dxRowGut = 0;
      Guts.dyColGut = 0;
      Guts.wLevelRowMax = 0;
      Guts.wLevelColMax = 0;
      bif_SaveData(lpBiff, (LPVOID)&Guts, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlHCENTER:
    {
      WORD wHCenter = 0;
      bif_SaveData(lpBiff, (LPVOID)&wHCenter, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlHEADER:
    {
      BYTE bHeaderLen = 0;
      bif_SaveData(lpBiff, (LPVOID)&bHeaderLen, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlHLINK:
    {
      LPyHLINK lphlink = (LPyHLINK)lpExtra;
      yHLINK   hlink;

      memcpy((LPyHLINK)&hlink, lphlink, sizeof(yHLINK)-sizeof(TBGLOBALHANDLE));
      
//      bif_SaveData(lpBiff, (LPVOID)&sheet, sheet.ghSheetName, sheet.wSheetNameLen, lpGH, lplSize);
    }
    break;
    case xlHORIZONTALPAGEBREAKS:
    {
      SS_COORD row;
      TBGLOBALHANDLE ghBreaks = 0;
      LPPAGEBREAK lppb = NULL;
      short breakCount = 0;
      for (row = 0; row<lpSS->Row.Max; row++)
      {
        if (SSGetRowPageBreak(lpBook->hWnd, row))
        {
          Alloc(&ghBreaks, (breakCount+1)*(sizeof(PAGEBREAK)));
          lppb = (LPPAGEBREAK)tbGlobalLock(ghBreaks);
          lppb[breakCount].index = (WORD)(row-1);
          lppb[breakCount].startingIndex = (WORD)0;
          lppb[breakCount].endingIndex = (WORD)255;
          tbGlobalUnlock(ghBreaks);
          breakCount++;
        }
      }
      if (breakCount > 0)
      {
        bif_SaveData(lpBiff, (LPVOID)&breakCount, ghBreaks, (WORD)(breakCount*sizeof(PAGEBREAK)), lpGH, lplSize);
        tbGlobalFree(ghBreaks);
      }
    }
      break;
    case xlVERTICALPAGEBREAKS:
    {
      SS_COORD col;
      TBGLOBALHANDLE ghBreaks = 0;
      LPPAGEBREAK lppb = NULL;
      short breakCount = 0;
      for (col = 0; col<lpSS->Col.Max; col++)
      {     
        if (SSGetColPageBreak(lpBook->hWnd, col))
        {
          Alloc(&ghBreaks, (breakCount+1)*(sizeof(PAGEBREAK)));
          lppb = (LPPAGEBREAK)tbGlobalLock(ghBreaks);
          lppb[breakCount].index = (WORD)(col-1);
          lppb[breakCount].startingIndex = (WORD)0;
          lppb[breakCount].endingIndex = (WORD)65535;
          tbGlobalUnlock(ghBreaks);
          breakCount++;
        }
      }
      if (breakCount > 0)
      {
        bif_SaveData(lpBiff, (LPVOID)&breakCount, ghBreaks, (WORD)(breakCount*sizeof(short)), lpGH, lplSize);
        tbGlobalFree(ghBreaks);
      }
    }
      break;
    case xlINDEX:
    {
      //An index rec is written following the BOF for each wksht.
      yINDEX8 Index;

      memset((LPyINDEX8)&Index, 0, sizeof(yINDEX8));
      Index.dwFirstRow = (DWORD)0;
      Index.dwLastRowP1 = (DWORD)0;
      bif_SaveData(lpBiff, (LPVOID)&Index, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlINTERFACEHDR:
    {
      WORD wCodePage = 0x04B0;  // only in BIFF8 - ANSI (Microsoft Windows)
      bif_SaveData(lpBiff, (LPVOID)&wCodePage, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlINTERFACEEND:
    {
      bif_SaveData(lpBiff, (LPVOID)NULL, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlITERATION:
    {
      short sIterations;
      double dfMaxChange;
      BOOL  fIteration = SSGetIteration(lpBook->hWnd, (short FAR *)&sIterations, (double far *)&dfMaxChange);
#ifdef SS_V70
      SSSetIteration(lpBook->hWnd, fIteration, sIterations, dfMaxChange);
#endif
      bif_SaveData(lpBiff, (LPVOID)&fIteration, NULL, 0, lpGH, lplSize);
    }
    break;

    case xlNAME2:
    case xlNAME:
    {
#ifdef SS_V70
      xNAME          name;
      TBGLOBALHANDLE gh = (TBGLOBALHANDLE)0;
      LPBYTE         lpb = NULL;
      LPBYTE         lpPos = NULL;
      BOOL           fUnicode = FALSE;
      memset(&name, 0, sizeof(xNAME));
      
      name.cch = (BYTE)((*(LPBYTE)lpExtra) - 1);
      lpExtra = (LPBYTE)lpExtra + sizeof(WORD);
      fUnicode = (BOOL)*(LPBYTE)lpExtra;

      lpPos = (LPBYTE)lpExtra + 1+name.cch * (fUnicode?sizeof(WCHAR):1);
      name.cce = *(LPWORD)lpPos;

      lpPos += sizeof(WORD);
      Alloc(&gh, 1 + name.cch * (fUnicode?sizeof(WCHAR):1) + name.cce);
      lpb = (LPBYTE)tbGlobalLock(gh);
      memcpy(lpb, lpExtra, 1+name.cch*(fUnicode?sizeof(WCHAR):1));
      memcpy(lpb+1+name.cch*(fUnicode?sizeof(WCHAR):1), lpPos, name.cce);
      tbGlobalUnlock(gh);
      bif_SaveData(lpBiff, (LPVOID)&name, gh, (WORD)(1+name.cch*(fUnicode?sizeof(WCHAR):1)+name.cce), lpGH, lplSize);

      tbGlobalFree(gh);
#endif
    }
    break;

#ifdef SS_V40
    case xlMERGECELLS:
    {
      // Get any cell merges and add them to the buffer
      TBGLOBALHANDLE ghSpan = 0;
      LPyMERGE       lpMerge = NULL;
      TBGLOBALHANDLE ghSpan2 = 0;
      LPyMERGE       lpMerge2 = NULL;
      short          mergeCount = 0;
  		LPSS_CELLSPAN  lpSpan = (LPSS_CELLSPAN)tbGlobalLock(lpSS->hCellSpan);
      TBGLOBALHANDLE ghUnlock = lpSS->hCellSpan;
      short          index = 0;  
      short          maxMergeRecs = 1025;
      short          mergeRecsInThisBiffRec = 0;
      long           lBufferSize = 0;
      
      while (lpSpan)
      {
        short col1 = (short)min(256, (short)(lpSpan->lCol-lpSS->Col.HeaderCnt));
        short col2 = (short)min(256, (short)(lpSpan->lNumCols+lpSpan->lCol-1-lpSS->Col.HeaderCnt));
        short row1 = (short)min(65536, (short)(lpSpan->lRow-lpSS->Row.HeaderCnt));
        short row2 = (short)min(65536, (lpSpan->lNumRows+lpSpan->lRow-1-lpSS->Row.HeaderCnt));
        if (col1>=0 && col1 <= 256 && col2>=0 && col2 <= 256 && // 256 max cols in Excel
            row1>=0 && row1 <=65536 && row2>=0 && row2 <=65536)
        {
          mergeCount++;
          Alloc(&ghSpan, mergeCount*sizeof(yMERGE));
          lpMerge = (LPyMERGE)tbGlobalLock(ghSpan);
          lpMerge = (LPyMERGE)((LPBYTE)lpMerge + (mergeCount-1)*sizeof(yMERGE));
          lpMerge->col1 = col1;
          lpMerge->rw1 = row1;
          lpMerge->col2 = col2;
          lpMerge->rw2 = row2;
          tbGlobalUnlock(ghSpan);
        }
        if (lpSpan->hNext)
        {
          TBGLOBALHANDLE ghTemp = lpSpan->hNext;
          tbGlobalUnlock(ghUnlock);
          ghUnlock = ghTemp;
          lpSpan = (LPSS_CELLSPAN)tbGlobalLock(ghUnlock);
        }
        else
        {
          tbGlobalUnlock(ghUnlock);
          break;
        }  
      }
      if (mergeCount > 0)
      {
        Alloc(&ghSpan2, min(maxMergeRecs, mergeCount)*sizeof(yMERGE));
        lpMerge2 = (LPyMERGE)tbGlobalLock(ghSpan2);
        lpMerge = (LPyMERGE)tbGlobalLock(ghSpan);
        mergeRecsInThisBiffRec = min(mergeCount, maxMergeRecs);
        for (i=0; i<mergeRecsInThisBiffRec; i++)
        {
          lpMerge2[index++] = lpMerge[mergeCount-1-i];
        }
        tbGlobalUnlock(ghSpan2);

        lpBiff->datalen = 2;
        bif_SaveData(lpBiff, &mergeRecsInThisBiffRec, ghSpan2, (short)(mergeRecsInThisBiffRec*sizeof(yMERGE)), lpGH, lplSize);
        lBufferSize = *lplSize;

        tbGlobalFree(ghSpan2);
        mergeCount = (short)(mergeCount-maxMergeRecs);

        while (mergeCount > 0)
        {
          index = 0;
          mergeRecsInThisBiffRec = min(mergeCount, maxMergeRecs);
          ghSpan2 = 0;
          Alloc(&ghSpan2, min(maxMergeRecs, mergeCount)*sizeof(yMERGE));
          lpMerge2 = (LPyMERGE)tbGlobalLock(ghSpan2);
          lpMerge = (LPyMERGE)tbGlobalLock(ghSpan);
          for (i=0; i<mergeRecsInThisBiffRec; i++)
          {
            lpMerge2[index++] = lpMerge[mergeCount-1-i];
          }
          lpBiff->datalen = 2;
          *lplSize = lBufferSize;
          bif_SaveData(lpBiff, &mergeRecsInThisBiffRec, ghSpan2, (short)(mergeRecsInThisBiffRec*sizeof(yMERGE)), lpGH, lplSize);
          lBufferSize += *lplSize;
        
          tbGlobalUnlock(ghSpan2);
          tbGlobalFree(ghSpan2);
          mergeCount = (short)(mergeCount-maxMergeRecs);
        }
  
        *lplSize = lBufferSize;
      }

      if (ghSpan != 0)
      {
        tbGlobalUnlock(ghSpan);
        tbGlobalFree(ghSpan);
      }

    }
    break;
    case xlMERGE_DONTKNOW:
    {
      BYTE dontknow[6] = {0,0,0x37,0,0,0};
      bif_SaveData(lpBiff, (LPVOID)dontknow, NULL, 0, lpGH, lplSize);
    }
    break;
//=========================
    case xlMSODRAWING:
    {
      bif_SaveData(lpBiff, lpExtra, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlNOTE:
    {
      lpBiff->datalen = sizeof(yNOTE);
      bif_SaveData(lpBiff, lpExtra, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlOBJ:
    {
      struct { ftCmo commonObj; ftGeneric note; BYTE b[22]; ftGeneric end; } objData;
      memset(&objData, 0, sizeof(objData));
      objData.commonObj.ft = 0x15;
      objData.commonObj.cb = 0x12;
      objData.commonObj.ot = 0x19;
      objData.commonObj.id = *(short *)lpExtra;
      objData.commonObj.fLocked = 1;
      objData.commonObj.fPrint = 1;
      objData.commonObj.fAutoLine = 1;
      objData.note.ft = 0x0D;
      objData.note.cb = 0x16;
      objData.end.ft = 0;
      objData.end.cb = 0;
      lpBiff->datalen = sizeof(objData);
      bif_SaveData(lpBiff, (LPVOID)&objData, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlTXO:
    {
      TBGLOBALHANDLE gh = 0;
      LPBYTE         lp = NULL; 
      struct {yyTXORuns r1; yyTXORuns r2; } txoRuns;
      yTXO           txo;
      int            nLen = lstrlen((LPTSTR)lpExtra);
      int            nTXOSize = 0;

      memset (&txo, 0, sizeof(yTXO));
      txo.alcH = 1;
      txo.alcV = 1;
      txo.fLockText = FALSE;
      txo.cchText = (short)nLen;
      lpBiff->datalen = sizeof(yTXO);
      bif_SaveData(lpBiff, (LPVOID)&txo, NULL, 0, lpGH, lplSize);
      nTXOSize = *lplSize;
      *lplSize = nTXOSize;

      lpBiff->recnum = xlCONTINUE;
      // this CONTINUE record contains the note string
      lpBiff->datalen = (short)((nLen * sizeof(TCHAR)) + 1); // +1 for the undocumented CONTINUE grbit
      Alloc(&gh, lpBiff->datalen + sizeof(TCHAR)); // +sizeof(TCHAR) for NULL
      lp = (LPBYTE)tbGlobalLock(gh);
#ifdef _UNICODE
      *lp = 1;
#else
      *lp = 0;
#endif
/*
      if (sizeof(TCHAR) == 1)
        *lp = 0;
      else
        *lp = 1;
*/
      _tcscpy((LPTSTR)(lp+1), (LPTSTR)lpExtra);

      bif_SaveData(lpBiff, (LPVOID)lp, NULL, 0, lpGH, lplSize);
      nTXOSize += *lplSize;
      *lplSize = nTXOSize;

      tbGlobalUnlock(gh);
      tbGlobalFree(gh);

      // this CONTINUE record contains the formatting runs
      lpBiff->datalen = sizeof(txoRuns);
      memset(&txoRuns, 0, sizeof(txoRuns));

      txoRuns.r1.ichFirst = 0;
      txoRuns.r1.ifnt = 1;
      txoRuns.r2.ichFirst = (short)nLen;
      txoRuns.r2.ifnt = 0;
      bif_SaveData(lpBiff, (LPVOID)&txoRuns, NULL, 0, lpGH, lplSize);
      *lplSize += nTXOSize;
    }
    break;
//=========================
#endif

    case xlMMS:
    {
      WORD bAddDelMenu = 0;
      bif_SaveData(lpBiff, (LPVOID)&bAddDelMenu, NULL, 0, lpGH, lplSize);
    }
    break;

#ifdef SS_V70
    case xlPANE:
    {
      yPANE    pane;
      SS_COORD colsFrozen = 0;
      SS_COORD rowsFrozen = 0;
      SSGetFreeze(lpBook->hWnd, &colsFrozen, &rowsFrozen);  
      pane.x = (short)colsFrozen;
      pane.y = (short)rowsFrozen;
      if (pane.x >0 || pane.y>0)
      {
        if (pane.x > 0)
          pane.colLeft = pane.x;
        else
          pane.colLeft = 0x40;
        if (pane.y > 0)
          pane.rwTop = pane.y;
        else
          pane.rwTop = 0x40;

       if (colsFrozen>0 && rowsFrozen==0)
          pane.pnnAct =  1;
        else if (colsFrozen==0 && rowsFrozen>0)
          pane.pnnAct = 2;
        else if (colsFrozen>0 && rowsFrozen>0)
          pane.pnnAct = 0;

        bif_SaveData(lpBiff, (LPVOID)&pane, NULL, 0, lpGH, lplSize);
      }
    }
    break;
#endif

    case xlPRINTGRIDLINES:
    {
      WORD wPrintGridLines = (WORD)lpSS->PrintOptions.pf.fShowGrid;
//      if (wPrintGridLines)
//      {
//        if (!lpSS->GridLines)
//          wPrintGridLines = 0;
//      }
      bif_SaveData(lpBiff, (LPVOID)&wPrintGridLines, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlPRINTHEADERS:
    {
      WORD wPrintHeaders = lpSS->PrintOptions.pf.fShowColHeaders || lpSS->PrintOptions.pf.fShowRowHeaders;
      if (wPrintHeaders)
      {
        if (!SSGetBool(lpBook->hWnd, SSB_SHOWCOLHEADERS) && !SSGetBool(lpBook->hWnd, SSB_SHOWROWHEADERS))
          wPrintHeaders = 0;
      }
      bif_SaveData(lpBiff, (LPVOID)&wPrintHeaders, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlPROTECT: //Structures option in Protect WorkBook dlg box
                    //Protect WorkSheet
    {
      WORD wProtect = (WORD)(SSGetBool(lpBook->hWnd, SSB_PROTECT));
// hatakeyama@boc.co.jp 11/21-2000	for excel document protects.
// Sets sheet/document protect.
		if( NULL != lpExtra)	{
			wProtect = *((WORD *)lpExtra);
		}
      bif_SaveData(lpBiff, (LPVOID)&wProtect, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlPROT4REV:
    {
      WORD wSharedProtect = 0;
      bif_SaveData(lpBiff, (LPVOID)&wSharedProtect, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlREFMODE:
    {
      WORD wRefMode = SSGetRefStyle(lpBook->hWnd);
      if (wRefMode == SS_REFSTYLE_A1 || (wRefMode == SS_REFSTYLE_DEFAULT))
        wRefMode = 1;
      else if (wRefMode == SS_REFSTYLE_R1C1)
        wRefMode = 0;
      bif_SaveData(lpBiff, (LPVOID)&wRefMode, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlROW:
    {
      LPyROW   lpRow = (LPyROW)lpExtra;
      double   dblTwips;
      int      iPixels = -1;
      LPSS_ROW lprow = SS_LockRowItem(lpSS, lpRow->wRow+lpSS->Row.HeaderCnt);

      lpRow->wCol1 = 0;
      lpRow->wCol2 = 0;

      if (lprow != NULL)
        iPixels = lprow->dRowHeightInPixels;
      if (lprow == NULL || iPixels == -1)
        SSGetRowHeightInPixels(lpBook->hWnd, lpRow->wRow+1, &iPixels);

      dblTwips = MulDiv(iPixels, 1440, GetDeviceCaps(hDC, LOGPIXELSY));

//14596 changes >>
      if (dblTwips == 0)
      {
        lpRow->wyRow = 0xFF;
        lpRow->fDyZero = 1;
      }
      else
      {
        lpRow->wyRow = (WORD)MulDiv((int)dblTwips, 20, (int)TWIP_PER_PT);
        if (!lprow)
          lpRow->fDyZero = 0;
        else
          lpRow->fDyZero = (lprow->fRowHidden?1:0);
      }
//14596 changes <<

      lpRow->wiRow = 0;
      lpRow->wiOutLevel = 0;
      lpRow->fCollapsed = 0;
      
      lpRow->fUnsynced = 1;
//ROBBY - changed 6/24/2002.  I'm not sure if this breaks anything?!?!?
// This fixes the situation where a column has backcolor or border, but no data in a 
// cell.  What was happening was the ROW settings were overriding the COLINFO settings
// because of this flag.
//     lpRow->fGhostDirty = 1;
     if (lprow && (lprow->Color.BackgroundId != 0 || lprow->FontId != 0))
        lpRow->res3 = 1; // this little undocumented treat forces the 
                         // row color specified in the XF to be used.
      bif_SaveData(lpBiff, (LPVOID)lpRow, NULL, 0, lpGH, lplSize);
      SS_UnlockRowItem(lpSS, lpRow->wRow+lpSS->Row.HeaderCnt);
    }
    break;
    case xlSAVERECALC:
    {
//      WORD wSaveRecalc = 1;
      WORD wSaveRecalc = 0x0303;
      bif_SaveData(lpBiff, (LPVOID)&wSaveRecalc, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlSELECTION:
    {
      ySEL         Sel;
      SS_COORD     ssRow;
      SS_COORD     ssCol;
      TBGLOBALHANDLE ghRefs = (TBGLOBALHANDLE)0;
      LPyyREF      lpRef;
      SS_CELLCOORD UL;
      SS_CELLCOORD LR;
      BOOL         fMulti = SSGetBool(lpBook->hWnd, SSB_ALLOWMULTIBLOCKS);
      short        sBlockCount;  
      GLOBALHANDLE gh = (GLOBALHANDLE)0;
      short        i;
      BYTE         bActivePanel = 3;
      long         lBufferSize = 0;
      
#ifdef SS_V70
      SS_COORD     rowsFrozen = 0;
      SS_COORD     colsFrozen = 0;
      SSGetFreeze(lpBook->hWnd, &colsFrozen, &rowsFrozen);

      if (colsFrozen >0 || rowsFrozen > 0)
      {
        Alloc(&ghRefs, sizeof(yyREF));
        lpRef = (LPyyREF)tbGlobalLock(ghRefs);
        memset(lpRef, 0, sizeof(yyREF));
      }
      if (colsFrozen > 0)
      {
        memset(&Sel, 0, sizeof(ySEL));
        if (rowsFrozen > 0)
        {
          Sel.bPaneNum = 3;
          Sel.wRefs = 1;
          lpBiff->datalen = 9;
          bif_SaveData(lpBiff, (LPVOID)&Sel, ghRefs, (WORD)(sizeof(yyREF)), lpGH, lplSize);
          lBufferSize = *lplSize;

          Sel.bPaneNum = 1;
          lpBiff->datalen = 9;
          *lplSize = lBufferSize;
          bif_SaveData(lpBiff, (LPVOID)&Sel, ghRefs, (WORD)(sizeof(yyREF)), lpGH, lplSize);
          lBufferSize += *lplSize;

          Sel.bPaneNum = 2;
          lpBiff->datalen = 9;
          *lplSize = lBufferSize;        
          bif_SaveData(lpBiff, (LPVOID)&Sel, ghRefs, (WORD)(sizeof(yyREF)), lpGH, lplSize);
          lBufferSize += *lplSize;
          *lplSize = lBufferSize;
          bActivePanel = 0;
        }
        else
        {
          Sel.bPaneNum = 3;
          Sel.wRefs = 1;
          lpBiff->datalen = 9;
          bif_SaveData(lpBiff, (LPVOID)&Sel, ghRefs, (WORD)(sizeof(yyREF)), lpGH, lplSize);
          lBufferSize = *lplSize;
          *lplSize = lBufferSize;
          bActivePanel = 1; 
        }
      }
      else if (rowsFrozen > 0)
      {
        memset(&Sel, 0, sizeof(ySEL)); //20993
        Sel.bPaneNum = 3;
        Sel.wRefs = 1;
        lpBiff->datalen = 9;
        bif_SaveData(lpBiff, (LPVOID)&Sel, ghRefs, (WORD)(Sel.wRefs*sizeof(yyREF)), lpGH, lplSize);
        lBufferSize = *lplSize;
        *lplSize = lBufferSize;
        bActivePanel = 2;
      }

      if (ghRefs != 0)
      {
        tbGlobalUnlock(ghRefs);      
        ghRefs = (TBGLOBALHANDLE)0;
        lpBiff->datalen = 9;
      }
#endif

      memset((LPySEL)&Sel, 0, sizeof(ySEL));
      
      SSGetActiveCell(lpBook->hWnd, (LPSS_COORD)&ssCol, (LPSS_COORD)&ssRow);
      if (ssCol > 256 || ssRow > 65536)
        ssRow = ssCol = 1;

      Sel.bPaneNum = bActivePanel;
      Sel.wRow = (WORD)max(0, ssRow-1);
      Sel.wCol = (WORD)max(0, ssCol-1);
      Sel.wiRefNum = 0;
      Sel.wRefs = 1;

      if (fMulti)
      {
        LPSS_SELBLOCK lpSelBlk;
        gh = SSGetMultiSelBlocks(lpBook->hWnd, &sBlockCount);
        if (gh != (GLOBALHANDLE)0)
        {
          lpSelBlk = (LPSS_SELBLOCK)GlobalLock(gh);
          Sel.wRefs = sBlockCount;
          Alloc(&ghRefs, sBlockCount*sizeof(yyREF));
          lpRef = (LPyyREF)tbGlobalLock(ghRefs);
          for (i=0; i<sBlockCount; i++)
          {
            if (lpSelBlk[i].UL.Row == -1 || lpSelBlk[i].LR.Row == -1)
            {
              lpRef[i].wRow1 = (WORD)0;
              lpRef[i].wRow2 = (WORD)0x3FFF;
            }
            else
            {
              lpRef[i].wRow1 = (WORD)max(0, lpSelBlk[i].UL.Row-1);
              lpRef[i].wRow2 = (WORD)max(0, lpSelBlk[i].LR.Row-1);
            }

            if (lpSelBlk[i].UL.Col == -1 || lpSelBlk[i].LR.Col == -1)
            {
              lpRef[i].bCol1 = (BYTE)0;
              lpRef[i].bCol2 = (BYTE)0xFF;
            }
            else
            {
              lpRef[i].bCol1 = (BYTE)max(0, lpSelBlk[i].UL.Col-1);
              lpRef[i].bCol2 = (BYTE)max(0, lpSelBlk[i].LR.Col-1);
            }
            
            if (Sel.wRow >= lpRef[i].wRow1 &&
                Sel.wRow <= lpRef[i].wRow2 &&
                Sel.wCol >= lpRef[i].bCol1 &&
                Sel.wCol <= lpRef[i].bCol2)
              Sel.wiRefNum = i;
          }
          GlobalUnlock(gh);
          GlobalFree(gh);
          tbGlobalUnlock(ghRefs);      
        }
        else
        {
          Sel.wRefs = 1;
          Alloc(&ghRefs, sizeof(yyREF));
          lpRef = (LPyyREF)tbGlobalLock(ghRefs);
          if (0 != SSGetSelectBlock(lpBook->hWnd, &UL, &LR))
          {
            lpRef->wRow1 = (WORD)max(0, UL.Row-1);
            lpRef->wRow2 = (WORD)max(0, LR.Row-1);
            lpRef->bCol1 = (BYTE)max(0, UL.Col-1);
            lpRef->bCol2 = (BYTE)max(0, LR.Col-1);
          }
          else
          {
            lpRef->wRow1 = (WORD)max(0, ssRow-1);
            lpRef->wRow2 = (WORD)max(0, ssRow-1);
            lpRef->bCol1 = (BYTE)max(0, ssCol-1);
            lpRef->bCol2 = (BYTE)max(0, ssCol-1);
          }
          tbGlobalUnlock(ghRefs);      
        }
      }
      else //if (!fMulti)
      {
        Sel.wRefs = 1;
        Alloc(&ghRefs, sizeof(yyREF));
        lpRef = (LPyyREF)tbGlobalLock(ghRefs);
        if (0 != SSGetSelectBlock(lpBook->hWnd, &UL, &LR))
        {
          if (UL.Row == -1)
          {
            lpRef->wRow1 = (WORD)0;
            lpRef->wRow2 = (WORD)-1;
          }
          else
          {
            lpRef->wRow1 = (WORD)max(0, UL.Row-1);
            lpRef->wRow2 = (WORD)max(0, LR.Row-1);
          }

          if (UL.Col == -1)
          {
            lpRef->bCol1 = (BYTE)0;
            lpRef->bCol2 = (BYTE)-1;
          }
          else
          {
            lpRef->bCol1 = (BYTE)max(0, UL.Col-1);
            lpRef->bCol2 = (BYTE)max(0, LR.Col-1);
          }
        }
        else
        {
          lpRef->wRow1 = (WORD)max(0, ssRow-1);
          lpRef->wRow2 = (WORD)max(0, ssRow-1);
          lpRef->bCol1 = (BYTE)max(0, ssCol-1);
          lpRef->bCol2 = (BYTE)max(0, ssCol-1);
        }
        tbGlobalUnlock(ghRefs);      
      }

      bif_SaveData(lpBiff, (LPVOID)&Sel, ghRefs, (WORD)(Sel.wRefs*sizeof(yyREF)), lpGH, lplSize);
      lBufferSize += *lplSize;
      *lplSize = lBufferSize;

      tbGlobalFree(ghRefs);
    }
    break;
    case xlSETUP:
    {
      WORD fLandscape = (WORD)((SS_PRINTORIENT_LANDSCAPE == lpSS->PrintOptions.pf.wOrientation)? 0:1);
      WORD fNoOrient = (WORD)((SS_PRINTORIENT_DEFAULT == lpSS->PrintOptions.pf.wOrientation)? 1:0);
	  	WORD fNoColor = (WORD)((!lpSS->PrintOptions.pf.fDrawColors)? 1:0);
      ySETUP Setup;
      memset((LPySETUP)&Setup, 0, sizeof(ySETUP));
#ifdef SPREAD_JPN
      Setup.wPaperSize = 9;
      Setup.fNoPls = 0;
#else
      Setup.wPaperSize = 1;
#endif
#ifdef SS_V70
      double dbl = lpSS->PrintOptions.pf.ZoomFactor*100;
      if ((dbl - dbl/1.0) >= 0.5)
        dbl++;
      Setup.wScale = (WORD)dbl;
      if (Setup.wScale == 100 && fNoOrient)
#ifdef SPREAD_JPN
        Setup.fNoPls = 0;
#else
        Setup.fNoPls = 1;
#endif
      else
        Setup.fNoPls = 0;
      Setup.wFitHeight = lpSS->PrintOptions.pf.nBestFitPagesTall;
      Setup.wFitWidth = lpSS->PrintOptions.pf.nBestFitPagesWide;
      Setup.wPageStart = (WORD)lpSS->PrintOptions.pf.lFirstPageNumber;
#else
      Setup.wScale = (WORD)0;
      if (fNoOrient)
        Setup.fNoPls = 1;
      else
        Setup.fNoPls = 0;
      Setup.wFitWidth = 1;
      Setup.wFitHeight = 1;
      Setup.wPageStart = 1;
      Setup.wRes = 0;
      Setup.wVRes = 0;
#endif
      Setup.fLeftToRight = 0;
      Setup.fLandscape = fLandscape;
      Setup.fNoColor = fNoColor;
      Setup.fDraft = 0;
      Setup.fNotes = 0;
      Setup.fNoOrient = 0;
      Setup.fUsePage = 0;
      Setup.numHdr = 0;
      Setup.numFtr = 0;
      Setup.wCopies = 1;

      bif_SaveData(lpBiff, (LPVOID)&Setup, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlLEFTMARGIN:
    {
      double margin = (double)lpSS->PrintOptions.pf.fMarginLeft;
      if (margin != 0)
        bif_SaveData(lpBiff, (LPVOID)&margin, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlTOPMARGIN:
    {
      double margin = (double)lpSS->PrintOptions.pf.fMarginTop;
      if (margin != 0)
        bif_SaveData(lpBiff, (LPVOID)&margin, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlRIGHTMARGIN:
    {
      double margin = (double)lpSS->PrintOptions.pf.fMarginRight;
      if (margin != 0)
        bif_SaveData(lpBiff, (LPVOID)&margin, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlBOTTOMMARGIN:
    {
      double margin = (double)lpSS->PrintOptions.pf.fMarginBottom;
      bif_SaveData(lpBiff, (LPVOID)&margin, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlSTYLE:
    {
      bif_SaveData(lpBiff, lpExtra, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlVCENTER:
    {
      WORD wVCenter = 0;
      bif_SaveData(lpBiff, (LPVOID)&wVCenter, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlWINDOW1:
    {
      WORKBOOK   wb;
      RECT       rect;

      memset(&wb, 0, sizeof(WORKBOOK));
      
      GetWindowRect(lpBook->hWnd, &rect);
//      wb.cx = (short)PIXELS_TO_INTPT(rect.right-rect.left+1);
//      wb.cy = (short)PIXELS_TO_INTPT(rect.bottom-rect.top+1);
      wb.cx = 0x32eb;
      wb.cy = 0x22f9;
      
      wb.fDspHScroll = SSGetBool(lpBook->hWnd, SSB_HORZSCROLLBAR);
      wb.fDspVScroll = SSGetBool(lpBook->hWnd, SSB_VERTSCROLLBAR);
      wb.fBotAdornment = 1;
      wb.nNumberOfSelTabs = 1;
#ifdef SS_V70      
      if (*(LPBOOL)lpExtra) //save sheet, not book
      {
        wb.nSelTabIndex = (short)0;
        wb.nFirstTabIndex = (short)0;
      }
      else
      {
        short i=0;

        wb.nSelTabIndex = (short)(SSGetActiveSheet(lpBook->hWnd)-1);
        wb.nFirstTabIndex = (short)max(0, lpBook->nTabStripLeftSheet); //(WORD)SSGetTabStripLeftSheet(lpBook->hWnd)-1;

        if (!SSGetSheetVisible(lpBook->hWnd, wb.nSelTabIndex))
        {
          for (i=0; i<lpBook->nSheetCnt; i++)
          {
            if (SSGetSheetVisible(lpBook->hWnd, (short)(i+1)))
            {
              wb.nSelTabIndex = i;
              break;
            }
          }
        }
        if (!SSGetSheetVisible(lpBook->hWnd, wb.nFirstTabIndex))
        {
          for (i=0; i<lpBook->nSheetCnt; i++)
          {
            if (SSGetSheetVisible(lpBook->hWnd, (short)(i+1)))
            {
              wb.nFirstTabIndex = i;
              break;
            }
          }
        }
      }
#endif
      wb.nWidthToHScrollRatio = 0x258;

      bif_SaveData(lpBiff, (LPVOID)&wb, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlWINDOW2:
    {
      yWORKSHEET8 WS;
      WORD        wGridType = SSGetGridType(lpBook->hWnd);
      long        color = 0x40;
      memset((LPyWORKSHEET8)&WS, 0, sizeof(yWORKSHEET8));

      WS.fDspFmla = 0;
      if (wGridType & 1 || wGridType & 2)
        WS.fDspGrid = 1;
      else
        WS.fDspGrid = 0;
      if (!SSGetBool(lpBook->hWnd, SSB_SHOWCOLHEADERS) &&
          !SSGetBool(lpBook->hWnd, SSB_SHOWROWHEADERS))
        WS.fDspRwCol = 0;
      else
        WS.fDspRwCol = 1;
      WS.fDspZeros = 1;
      WS.fDefaultHdr = 0;
      WS.fArabic = 0;
      WS.fDspGuts = 1;
      WS.fFrozenNoSplit = 0;
#ifdef SS_V70
      BOOL  fActiveSheet = *(LPBOOL)lpExtra;

      SSGetFreeze(lpBook->hWnd, &colsFrozen, &rowsFrozen);
      if (colsFrozen>0 || rowsFrozen>0)
        WS.fFrozen = 1;
      else
        WS.fFrozen = 0;

      if (fActiveSheet)
        WS.fSelected = 1;
      else
        WS.fSelected = 0;
#else
      WS.fFrozen = 0;
      WS.fSelected = 1;
#endif
      WS.fPaged = 1;
      WS.fSLV = 0;
      WS.wRowTop = 0;
      WS.wColLeft = 0;
      // if the color is the standard/automatic color (0x16) assign the default to Excel (0x40)
      // otherwise, if a color is set, any border set w/ default color will pick-up the gridline 
      // assigned color.  This will have to be okay if the gridline color is explicitly set; however,
      // we don't want to impose this functionality when the gridlines are set to their default color value.
      color = xl_IndexFromColor(SSGetGridColor(lpBook->hWnd));
      WS.dwiHdr = (color==0x16?0x40:color);
      WS.wScaleSLV = 0;
      WS.wScaleNormal = 0;

      bif_SaveData(lpBiff, (LPVOID)&WS, NULL, 0, lpGH, lplSize);
//---------------------
//*lplSize += lSize;
//---------------------
    }
    break;

//Spread3.0 does not support WorkBooks.
    case xlWINDOWPROTECT:  //Windows option in Protect WorkBook dlg box
    {
      WORD wProtect = 0;
      bif_SaveData(lpBiff, (LPVOID)&wProtect, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlWSBOOL:
    {
      yWSBOOL WSBool;
      memset((LPyWSBOOL)&WSBool, 0, sizeof(yWSBOOL));

      WSBool.fShowAutoBreaks = 1;
      WSBool.fDialog = 0;
      WSBool.fApplyStyles = 0;
      WSBool.fRowSumsBelow = 1;
      WSBool.fColSumsRight = 1;
#ifdef SS_V70
      //15544
//18756d      if (lpSS->PrintOptions.pf.nBestFitPagesWide != 1 || lpSS->PrintOptions.pf.nBestFitPagesTall != 1)
//18756 >>
      if (lpSS->PrintOptions.pf.wScalingMethod == SS_PRINTSCALINGMETHOD_BESTFIT &&
          (lpSS->PrintOptions.pf.nBestFitPagesWide != 0 && lpSS->PrintOptions.pf.nBestFitPagesTall != 0))
//<< 18756
        WSBool.fFitToPage = 1;
      else
        WSBool.fFitToPage = 0;
#endif
      WSBool.fDspGuts = 1;
      WSBool.fAee = 0;
      WSBool.fAfe = 0;
      bif_SaveData(lpBiff, (LPVOID)&WSBool, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlXF:
    {
      bif_SaveData(lpBiff, (LPVOID)lpExtra, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlSUPBOOK:
    {
      bif_SaveData(lpBiff, (LPVOID)lpExtra, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlEXTERNSHEET:
    {
      bif_SaveData(lpBiff, (LPVOID)lpExtra, NULL, 0, lpGH, lplSize);
    }
    break;
  
    //cell values...
    case xlBLANK:
    {
      bif_SaveData(lpBiff, (LPVOID)lpExtra, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlFORMULA:
    case xlFORMULA2:
    {
      TBGLOBALHANDLE FAR *lpgh = (TBGLOBALHANDLE FAR *)lpGH;
      LPBYTE lpFormula = NULL;

      if (!lpExtra)
        break;
      
      lpFormula = (LPBYTE)tbGlobalLock(*lpgh);

      bif_SaveData(lpBiff, lpExtra, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlSTRING:
    {
      bif_SaveData(lpBiff, lpExtra, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlLABEL:
    {
      bif_SaveData(lpBiff, (LPVOID)lpExtra, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlLABELSST:
    {
      bif_SaveData(lpBiff, (LPVOID)lpExtra, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlNUMBER:
    {
      bif_SaveData(lpBiff, (LPVOID)lpExtra, NULL, 0, lpGH, lplSize);
    }
    break;
    case xlRK:
    {
      yCELLVAL cell;
          
      memset((LPyCELLVAL)&cell, 0, sizeof(yCELLVAL));
      cell.wCellType = xlRK;
    }
    break;

    case xlMULBLANK:
    {
      yCELLVAL    cell;

      memset((LPyCELLVAL)&cell, 0, sizeof(yCELLVAL));
      cell.wCellType = xlBLANK;
    }
    break;

    case xlMULRK:
    {
      yCELLVAL    cell;
          
      memset((LPyCELLVAL)&cell, 0, sizeof(yCELLVAL));
      cell.wCellType = xlRK;
    }
    break;
 
    case xlBOOLERR:
    {
      yCELLVAL   cell;
      memset((LPyCELLVAL)&cell, 0, sizeof(yCELLVAL));
      cell.wCellType = xlBOOLERR;
    }
    break;
  }

  if (lpBiff->recnum != xlBOF_BIFF5_7_8)
  {
    SS_BookUnlockSheet(lpBook);
    ReleaseDC(lpBook->hWnd, hDC);
  }
  else
    ReleaseDC(GetDesktopWindow(), hDC);

  return 0;
}

/***********************************************************************
* Name:   bif_SaveData - Save the data to the save buffer.
*
* Usage:  short bif_SaveData(LPBIFF lpBiff, LPVOID lpData1,
*                            TBGLOBALHANDLE gh2, WORD wSize2, LPVOID lpGH,
*                            LPLONG lplSize)
*            lpBiff - pointer to the BIFF structure to add to the buffer
*            lpData1 - pointer to the BIFF data. (this buffer's size is in 
*                                                 the BIFF structure)
*            gh2 - pointer to any additional data to be added to the end
*                  of the biff data.
*            wSize2 - size of the additional data.
*            lpGH - pointer to the buffer.
*            lplSize - size of the buffer.
*
* Return: short - Success or Failure
***********************************************************************/
short bif_SaveData(LPBIFF lpBiff, LPVOID lpData1, TBGLOBALHANDLE gh2, WORD wSize2, LPVOID lpGH, LPLONG lplSize)
{
  LPVOID lp;
  TBGLOBALHANDLE FAR *lpgh = (TBGLOBALHANDLE FAR *)lpGH;
  WORD wLen = lpBiff->datalen;
  BOOL fReAlloc = FALSE;
  long lSize = *lplSize;

  *lplSize = sizeof(BIFF) + lpBiff->datalen + wSize2;
  if (*lpgh)
    fReAlloc = TRUE;
  Alloc(lpgh, lSize + *lplSize);
  lp = (LPVOID)tbGlobalLock(*lpgh);
  if (fReAlloc)
    lp = (LPBYTE)lp + lSize;

  lpBiff->datalen = (unsigned short)((unsigned short)lpBiff->datalen + (unsigned short)wSize2);
  memcpy(lp, lpBiff, sizeof(BIFF));
  if (lpData1)
  {
    memcpy((LPBYTE)lp + sizeof(BIFF), lpData1, wLen);
    if (wSize2)
    {
      LPVOID lpData2 = (LPVOID)tbGlobalLock(gh2);
      wLen += sizeof(BIFF);
      memcpy((LPBYTE)lp + wLen, lpData2, wSize2);
      tbGlobalUnlock(gh2);
    }
  }
  tbGlobalUnlock(*lpgh);
  return 0;
}

/***********************************************************************
* Name:   ToFile - Print the string information to the file specified by
*                  PARSEFILE.
*
* Usage:  short ToFile(LPTSTR szString1, LPTSTR szString2)
*            szString1 - input string1
*            szString3 - input string2
*
* Return: short - Success or Failure
***********************************************************************/
short ToFile(LPTSTR szString1, LPTSTR szString2)
{
  szString1;
  szString2;
#ifdef _PARSE2FILE
  FILE *fp = _tfopen(PARSEFILE, _T("a"));

  if (fp)
  {
    _ftprintf(fp, _T("%s - %s\n"), szString1, szString2);

    fflush(fp);
 
    fclose(fp);
  }
#endif
  return 0;
}


BOOL bif_GetLastBiffStrInSST(LPSSXL lpss, DWORD *pdwLastBfstrOffset, BOOL *fSeamString)
{
  LPBIFFSTR lpbfstr = NULL;
  DWORD     dw;
  DWORD     dwOffset = 8;
  LPBYTE    lpData = NULL;

  lpData = (LPBYTE)tbGlobalLock(lpss->ghSSTData);


  for (dw=0; dw<lpss->sst.dwStrCount; dw++)
  {
    long           lOffset = 0;
    long           lOffsetAfterText = 0;
    LPBYTE         lpb = NULL;

    lpbfstr = (LPBIFFSTR)LPOFFSET(lpData, dwOffset);
    
    // if the biffstr header is not complete, return
    lOffset += sizeof(BIFFSTR)-1;
    if (dwOffset + lOffset > lpss->dwSSTDataLen)
    {
      *fSeamString = FALSE;
      *pdwLastBfstrOffset = (DWORD)-1;
      break;
    }

    lpb = (LPBYTE)((LPBYTE)lpbfstr + sizeof(BIFFSTR)-1);

    if (lpbfstr->fRichSt)
    {
      DWORD weRunCount = *(WORD *)lpb;
      lOffset += sizeof(WORD);
      lOffsetAfterText += weRunCount * 4;
      lpb += sizeof(WORD);
    }

    if (lpbfstr->fExtSt)
    {
      DWORD dweExtRstLen = *(DWORD *)lpb;
        
      lOffset += sizeof(DWORD);
      lOffsetAfterText += dweExtRstLen;
    }
    
    if (dwOffset + lOffset > lpss->dwSSTDataLen)
    {
      *fSeamString = FALSE;
      *pdwLastBfstrOffset = (DWORD)-1;
      break;
    }

    lOffset += lpbfstr->wCharCount*(lpbfstr->fHighByte?2:1);
    if (dwOffset + lOffset > lpss->dwSSTDataLen)
    {  
      *fSeamString = TRUE;
      *pdwLastBfstrOffset = dwOffset;
      break;
    }

    lOffset += lOffsetAfterText;
    if (dwOffset + lOffset > lpss->dwSSTDataLen)
    {
      *fSeamString = FALSE;
      *pdwLastBfstrOffset = (DWORD)-1;
      break;
    }
  
    dwOffset += lOffset;
  }

  return FALSE;
}
//RAP06a <<


//RAP06a >>
BOOL bif_UnCompressCharStr(LPSSXL lpss, DWORD dwLastBfstrOffset, LPBIFF lpCont, WORD wPartByteLen, BOOL fSST)
{
  LPBYTE    lpData = (LPBYTE)tbGlobalLock(lpss->ghSSTData);
  LPBIFFSTR lpbfstr = (LPBIFFSTR)LPOFFSET(lpData, dwLastBfstrOffset);
  short     nStringLen = 0;
  TBGLOBALHANDLE gh = 0;
  LPSTR     lpstr = NULL;
  LPWSTR    lpwstr = NULL;
  WORD      wCharLen;
  long      lBiffStrHdrLen = 0;

  if (fSST) //if we need to uncompress a char string with a BIFFSTR header
  {
    lBiffStrHdrLen = sizeof(BIFFSTR)-1;
    
    if (lpbfstr->fExtSt)
      lBiffStrHdrLen += sizeof(DWORD);
    if (lpbfstr->fRichSt)
      lBiffStrHdrLen += sizeof(WORD);

    wCharLen = (WORD)(min(lpbfstr->wCharCount, wPartByteLen - lBiffStrHdrLen)); 
    lpbfstr->fHighByte = 1;
    lpstr = (LPSTR)((LPBYTE)lpbfstr + lBiffStrHdrLen);
  } // fSST
  else //uncompress the part of the char string that is in the CONTINUE record
  {
    lpstr = (LPSTR)((LPBYTE)lpCont + sizeof(BIFF) + 1/*grbit*/);
//RAP12d    wByteLen = (WORD)(lpbfstr->wCharCount - (wPartByteLen-sizeof(BIFF))/sizeof(WCHAR));
    wCharLen = (WORD)(lpbfstr->wCharCount - (wPartByteLen - (sizeof(BIFFSTR)-1) - (lpbfstr->fRichSt?2:0) - (lpbfstr->fExtSt?4:0))/sizeof(WCHAR)); //RAP12a
  }

  // Convert the single byte string to a Unicode string.
  Alloc(&gh, wCharLen*sizeof(WCHAR));
  lpwstr = (LPWSTR)tbGlobalLock(gh);
  nStringLen = (short)MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpstr,
                                          wCharLen,
                                          lpwstr, wCharLen);
         
  if (fSST)
  {
    DWORD dwOffset = (DWORD)(dwLastBfstrOffset + lBiffStrHdrLen);

    tbGlobalUnlock(lpss->ghSSTData);
        
    //increment the size of the buffer by the uncompressed portion of the char string part        
    lpss->dwSSTDataLen += wCharLen + lpCont->datalen-1/*grbit*/; 
       
    Alloc(&lpss->ghSSTData, lpss->dwSSTDataLen);
    lpData = (LPBYTE)tbGlobalLock(lpss->ghSSTData);
    memcpy(lpData+dwOffset, lpwstr, nStringLen*sizeof(WCHAR));
	  memcpy(lpData+dwOffset+(nStringLen*sizeof(WCHAR)), ((LPBYTE)lpCont+sizeof(BIFF)+1/*grbit*/), lpCont->datalen-1/*grbit*/);  
  }
  else
  {
//RAP12d    DWORD dwOffset = (DWORD)lpbfstr - (DWORD)lpData + sizeof(BIFFSTR);
    DWORD dwOffset = dwLastBfstrOffset + wPartByteLen; //RAP12a

    tbGlobalUnlock(lpss->ghSSTData);
        
    //increment the size of the buffer by the uncompressed portion of the char string part        
//RAP12d    lpss->dwSSTDataLen += wCharLen + lpCont->datalen; 
    lpss->dwSSTDataLen += wCharLen + lpCont->datalen-1 /*grbit*/;  //RAP12a
       
    Alloc(&lpss->ghSSTData, lpss->dwSSTDataLen);
    lpData = (LPBYTE)tbGlobalLock(lpss->ghSSTData);
    memcpy(lpData+dwOffset, lpwstr, nStringLen*sizeof(WCHAR));
//RAP12d    memcpy(lpData+dwOffset+nStringLen*sizeof(WCHAR), ((LPBYTE)lpCont+sizeof(BIFF)+1/*grbit*/ + wByteLen), lpCont->datalen-1/*grbit*/);
    memcpy(lpData+dwOffset+nStringLen*sizeof(WCHAR), ((LPBYTE)lpCont+sizeof(BIFF)+1/*grbit*/ + wCharLen), lpCont->datalen-1/*grbit*/-wCharLen); //RAP12a
  }

  tbGlobalUnlock(lpss->ghSSTData);

  tbGlobalUnlock(gh);
  tbGlobalFree(gh);
    
  return FALSE;
}
//RAP06a <<


BOOL bif_ProcessSST(LPSSXL lpss)
{
  LPBIFFSTR      lpbfstr = NULL;
  DWORD          dw;
  TBGLOBALHANDLE ghUncompress = 0;
  TBGLOBALHANDLE ghConvert = 0;
  int            nStringLen = 0;
  LPBYTE         lpData = NULL;
  TBGLOBALHANDLE ghNew = 0;
  int            nNewLen = 0;
  LPSTR          lpstr = NULL;

  WriteOut(_T("SST"), NULL);

  lpss->bProcessDataType = 0;

  lpData = (LPBYTE)tbGlobalLock(lpss->ghSSTData);

  lpbfstr = (LPBIFFSTR)LPOFFSET(lpData, 8);

  for (dw=0; dw<lpss->sst.dwStrCount; dw++)
  {
    int    byteOffset = (lpbfstr->fExtSt?4:0)+(lpbfstr->fRichSt?2:0); // determine the offset to the string from the end of the grbit.
    LPBYTE pbString = (LPBYTE)lpbfstr + sizeof(WORD) + sizeof(BYTE) + byteOffset; // find beginning of the string by jumping the grbit and the offsets.
  
    if (lpbfstr->wCharCount == 0)
    {
      // I don't know why this happens, but it does occasionally...
      Alloc(&ghNew, nNewLen + sizeof(int));
      lpData = (LPBYTE)tbGlobalLock(ghNew);
      nStringLen = 0;
      memcpy(lpData+nNewLen, &nStringLen, sizeof(int));
      nNewLen += sizeof(int);
      tbGlobalUnlock(ghNew);
      
      lpbfstr = (LPBIFFSTR)((LPBYTE)lpbfstr + sizeof(BIFFSTR)-sizeof(BYTE)+ byteOffset);
      continue;
    }

    if (!lpbfstr->fHighByte)
    {
      LPBYTE lpb = NULL;
      int i;

      Alloc(&ghUncompress, lpbfstr->wCharCount*2);
      lpb = (LPBYTE)tbGlobalLock(ghUncompress);
      for (i=0; i<lpbfstr->wCharCount; i++)
      {
        lpb[i*2] = pbString[i];
        lpb[i*2+1] = 0x00;
      }
      pbString = lpb;
    }

#ifdef _UNICODE
    nStringLen = lpbfstr->wCharCount*sizeof(TCHAR);
#else
    Alloc(&ghConvert, lpbfstr->wCharCount*sizeof(WCHAR)); //Use WCHAR to alloc enough space for Multibyte
    
    lpstr = (LPSTR)tbGlobalLock(ghConvert);
    nStringLen = (short)WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, 
                                            (LPWSTR)pbString, lpbfstr->wCharCount,
                                            lpstr, (WORD)(lpbfstr->wCharCount*sizeof(WCHAR)),
                                            NULL, NULL);
    pbString = (LPBYTE)lpstr;
#endif

    Alloc(&ghNew, nNewLen + sizeof(int) + nStringLen);
    lpData = (LPBYTE)tbGlobalLock(ghNew);
    memcpy(lpData+nNewLen, &nStringLen, sizeof(int));
    nNewLen += sizeof(int);
    memcpy(lpData+nNewLen, pbString, nStringLen);
    nNewLen += nStringLen;
    tbGlobalUnlock(ghNew);

    if (ghConvert)
    { 
      tbGlobalUnlock(ghConvert);
      tbGlobalFree(ghConvert);
      ghConvert = 0;
    }

    if (ghUncompress)
    {
      tbGlobalUnlock(ghUncompress);
      tbGlobalFree(ghUncompress);
      ghUncompress = 0;
    }

    // advance the buffer pointer
    if (lpbfstr->fExtSt)
    {          
      WORD   weRunCount=0;
      DWORD  dweExtRstLen;
      LPBYTE lpb = (LPBYTE)((LPBYTE)lpbfstr + sizeof(BIFFSTR)-1);

      if(lpbfstr->fRichSt)
      {
        weRunCount = *(WORD *)lpb;
        dweExtRstLen = *(DWORD *)(lpb+sizeof(WORD));
      }
      else
      {
        dweExtRstLen = *(DWORD *)lpb;
      }
      byteOffset += dweExtRstLen + weRunCount*4;                
    }
    else
    {
      WORD   weRunCount=0;
      LPBYTE lpb = (LPBYTE)((LPBYTE)lpbfstr + sizeof(BIFFSTR)-1);
      if(lpbfstr->fRichSt)
      {
        weRunCount = *(WORD *)lpb;
      }
      byteOffset += weRunCount*4;                
    }

    byteOffset += lpbfstr->wCharCount * (lpbfstr->fHighByte?2:1);
  
    lpbfstr = (LPBIFFSTR)((LPBYTE)lpbfstr + sizeof(BIFFSTR)-sizeof(BYTE)+ byteOffset);

  }      

  tbGlobalUnlock(lpss->ghSSTData);
  tbGlobalFree(lpss->ghSSTData);
  lpss->ghSSTData = 0;
  lpss->dwSSTDataLen =0;
  if (lpss->sst.ghStrings)
  {
    tbGlobalFree(lpss->sst.ghStrings);
  }
  lpss->sst.ghStrings = ghNew;

  
  return FALSE;
}
