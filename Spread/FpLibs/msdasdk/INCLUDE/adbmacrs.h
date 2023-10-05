//=--------------------------------------------------------------------------=
// ADBMACRS.H:	Definition of Data Access helper macros
//=--------------------------------------------------------------------------=
// Copyright  1997  Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
#ifndef _ADBMACRS_H_
#define _ADBMACRS_H_

#define IS_SAME_GUIDS(riid1, riid2) (((riid1).Data1 == (riid2).Data1) && ((riid1) == (riid2)))

#ifndef RETURN_ON_FAILURE
#define RETURN_ON_FAILURE(hr) if (FAILED(hr)) return hr
#endif

#ifndef RETURN_ON_NULLALLOC
#define RETURN_ON_NULLALLOC(ptr) if (!(ptr)) return E_OUTOFMEMORY
#endif

#ifndef RELEASE_OBJECT
#define RELEASE_OBJECT(ptr)    if (ptr) { IUnknown *pUnk = (ptr); (ptr) = NULL; pUnk->Release(); }
#endif

#ifndef QUICK_RELEASE
#define QUICK_RELEASE(ptr)     if (ptr) ((IUnknown *)ptr)->Release();
#endif

#ifndef ADDREF_OBJECT
#define ADDREF_OBJECT(ptr)     if (ptr) (ptr)->AddRef()
#endif

#ifndef CHECK_POINTER
#define CHECK_POINTER(val) if (!(val) || IsBadReadPtr((void *)(val), sizeof(void *))) return E_POINTER
#endif

#ifndef CHECK_POINTER_SIZE
#define CHECK_POINTER_SIZE(val,sz) if (!(val) || IsBadReadPtr((void *)(val), sz)) return E_POINTER
#endif

#endif // !_ADBMACRS_H_
