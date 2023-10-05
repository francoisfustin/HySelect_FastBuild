/***********************************************************************
* gdicopy.h -- Declarations for functions to create copies of 
*			   drag-and-drop data
*
* Copyright (C) 1991-1995 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/

#ifndef SS40
#include "stdafx.h"
#endif

// creates a new global handle that contains the same data as an existing one
HRESULT CopyHGlobal(HGLOBAL hGlobal, HGLOBAL* phRet);

// creates a new HBITMAP from an existing HBITMAP
HRESULT CopyHBitmap(HBITMAP hBitmap, HBITMAP* phRet);

// creates a new HPALETTE from an existing HPALETTE
HRESULT CopyHPalette(HPALETTE hPal, HPALETTE* phRet);

// creates a new HMETAFILEPICT from an existing HMETAFILEPICT
// (creates a new HMETAFILE for the new HMETAFILEPICT, too)
HRESULT CopyHMetaFilePict(HMETAFILEPICT hMetaFilePict, HMETAFILEPICT* phRet);

// creates a new HENHMETAFILE from an existing HMENHMETAFILE
HRESULT CopyHEnhMetaFile(HENHMETAFILE hEnhMetaFile, HENHMETAFILE* phRet);

