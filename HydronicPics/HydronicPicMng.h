// HydronicPics.h : Defines the exported functions for the DLL application.
//
#pragma once

#include "stdafx.h"
#include "..\TASelect\Global.h"
#include "..\TASelect\HydronicPic.h"

#include <map>

extern multimap <_string, CProdPic *> m_mmapPicsList;
extern multimap <_string, CDynCircSch > m_mmapCircSchList;

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
Author: AL													Date:	08/08/08

Function: DLLDEC CProdPic* GetProdPic(LPCTSTR,CProdPic::eProdPicType);

Description: Use that function to return a pointer to a ProdPic object

IN:	 The first argument is a DescriptorID'LPCTSTR that is coming from the TAProduct's 
 	 Database. The second is the type of image. It can be an Image, a TA Symbol
 	 Image or a Localized Symbol Image.

OUT: Returns a CProdPic object pointer or NULL pointer if Image doesn't exist

==============================================================================*/
DLLDEC CProdPic* GetProdPic(LPCTSTR,CProdPic::eProdPicType);
DLLDEC CDynCircSch* GetDynCircSch(LPCTSTR);
DLLDEC bool VerifyImgFound(LPCTSTR);

#ifdef __cplusplus
}
#endif