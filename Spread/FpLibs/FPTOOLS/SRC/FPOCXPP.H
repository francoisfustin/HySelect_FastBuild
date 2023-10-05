/***********************************************************************
* FPOCXPP.H  -  Farpoint declarations for putting OCX Property Pages in 
*               a seperate DLL.
*
* Copyright (C) 1991-1996 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/
#ifndef FPOCXPP_H
#define FPOCXPP_H

// The difference between this and the standard "IMPLEMENT_OLECREATE_EX"
// is that the GUID is passed as one parameter (as a structure).
#define FPOCXPP_IMPLEMENT_OLECREATE_EX(class_name, external_name, \
			guid_struct) \
	static const TCHAR _szProgID_##class_name[] = _T(external_name); \
	AFX_DATADEF class_name::class_name##Factory class_name::factory( \
		class_name::guid, RUNTIME_CLASS(class_name), FALSE, \
		_szProgID_##class_name); \
	const AFX_DATADEF GUID class_name::guid = guid_struct; \
	HRESULT class_name::GetClassID(LPCLSID pclsid) \
		{ *pclsid = guid; return NOERROR; }

FPLIB_(BOOL) fpVerifyCOMClsid(GUID FAR *lpClsid, LPCTSTR tstrDllName);

#endif
