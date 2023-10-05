//=--------------------------------------------------------------------------=
// ADBCTL.H:	Main header file for ODBCTL library
//=--------------------------------------------------------------------------=
// Copyright  1997  Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
#ifndef _ADBCTL_H_
#define _ADBCTL_H_

#define INC_OLE2
#include <windows.h>
#include <olectl.h>

// Define these to prevent additional header files from being included
//
// Define					Description
//---------------------------------------------------------------------------
// NO_ADBCTL_MACROS			Macros
// NO_ADBCTL_ROWSET			CRowset
// NO_ADBCTL_ODBCHELP		Miscellaneous OLE DB helpers		
// NO_ADBCTL_BOOKMARK		CBookmark
// NO_ADBCTL_STRUTILS		String helpers
// NO_ADBCTL_ADVCON			Advise Connection/Property Notify Sinks
// NO_ADBCTL_PERSIST		Persistence helpers
// NO_ADBCTL_PROPHELP		Property page and property browse helpers
// NO_ADBCTL_MEMSTREAM		Memory IStream implementation for persistence
//
#ifndef NO_ADBCTL_MACROS
#include <adbmacrs.h>
#endif // !NO_ADBCTL_MACROS

#ifndef NO_ADBCTL_ROWSET
#include <rowset.h>
#endif // !NO_ADBCTL_ROWSET

#ifndef NO_ADBCTL_ODBCHELP
#include <adbhelp.h>
#endif // !NO_ADBCTL_ODBCHELP

#ifndef NO_ADBCTL_BOOKMARK
#include <bookmark.h>
#endif // !NO_ADBCTL_BOOKMARK

#ifndef NO_ADBCTL_STRUTILS
#include <strutils.h>
#endif // !NO_ADBCTL_STRUTILS

#ifndef NO_ADBCTL_ADVCON
#include <advcon.h>
#endif // !NO_ADBCTL_ADVCON

#ifndef NO_ADBCTL_PERSIST
#include <persist.h>
#endif // !NO_ADBCTL_PERSIST

#ifndef NO_ADBCTL_PROPHELP
#include <prophelp.h>
#endif // !NO_ADBCTL_PROPHELP

#ifndef NO_ADBCTL_MEMSTREAM
#include <MemStm.h>
#endif // !NO_ADBCTL_MEMSTREAM

#endif // _ADBCTL_H_
