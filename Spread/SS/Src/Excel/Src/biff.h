/*********************************************************
* biff.h
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
*********************************************************/

// Structured Storage Model functions
extern long ssm_Initialize();
extern long ssm_UnInitialize();
extern long ssm_CreateStorageObject(LPCWSTR szFileName, LPCWSTR szStreamName,
                                   IStorage **ppstg, IStream **ppstm);
extern long ssm_OpenStorageObject(LPCWSTR szFileName, LPCWSTR szStreamName,
                                 IStorage **ppstg, IStream **ppstm, BOOL fLoad);
extern long ssm_GetStorageObjectList(IStorage *pstg, TBGLOBALHANDLE *pghStreamList, short *lpsCount);
extern long ssm_StreamRead(IStream *pstm, LPSSXL lpss, LPSS_BOOK lpBook, int nSheetNum, short sPass);
extern long ssm_StreamWrite(IStream *pstm, LPVOID lp, LPCTSTR szSheetName, LPTBGLOBALHANDLE lpghBuffer, LPLONG lplBufferLen, BOOL fToStream, SHORT sFlags);

extern long ssm_Write(IStream *pstm, LPBIFF lpBiff, LPVOID lp, LPLONG lplPos, LPVOID lpExtra);
extern long ssm_DumpWorkSheet(IStream *pstm, TBGLOBALHANDLE gh, LPLONG lplLen);
extern long ssm_CleanUp(IStream *pstm);

