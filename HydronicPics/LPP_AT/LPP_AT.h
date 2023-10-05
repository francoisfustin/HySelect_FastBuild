// LPPDea.h : Defines the exported functions for the DLL application.
//
#pragma once

#include "stdafx.h"
#include "..\..\TASelect\Global.h"
#include "..\..\TASelect\HydronicPic.h"

#include <map>

extern multimap <_string, CProdPic> m_mapLocPicsDeaList;

// Functions that can be exported
#ifdef _WINDLL
#define DLLDEC __declspec(dllexport)
#else
#define DLLDEC __declspec(dllimport)
#endif
#ifdef __cplusplus
extern "C"
{
#endif

/*==============================================================================
Author: AL														Date:	14/08/08

Function: DLLDEC CProdPic* GetLocProdPic(LPCTSTR);

Description: Use that function to return a pointer to a LocProdPic object

IN:	 The argument is a DescriptorID'LPCTSTR that is coming from the TAProduct's 
 	 Database. 

OUT: Return a CProdPic object pointer or NULL pointer if LocPic doesn't exist

==============================================================================*/
DLLDEC CProdPic* GetLocProdPic(LPCTSTR);

#ifdef __cplusplus
}
#endif