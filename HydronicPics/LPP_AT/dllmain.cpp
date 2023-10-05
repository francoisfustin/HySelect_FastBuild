// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "tchar.h"
#include "resource.h"
#include "LPP_AT.h"

// Create a multimap to take into account that each Loc Product Image
multimap <_string, CProdPic> m_mapLocPicsDeaList;

// Create a pair because the insert function in the multimap accept only one argument
typedef std::pair<_string, CProdPic> PicPair;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	// Dll main function is used many times.
	// But we want to load the frames only one time.
	int iNbr = m_mapLocPicsDeaList.size();
	if(iNbr)
		return TRUE;
	/*==============================================================================
	Author: AL														Date:	14/08/08

	Function: ADDIMAGE(IDDescriptor,CProdPic(IDRes,CProdPic::PicLocSymb,ptIn,ptOut,...))

	Description: Add images and there properties into the multimap

	IN:	 The first argument is a DescriptorID (!!!STRING!!!) coming from 
		 the TAProduct's Database. Do not use '_T("")' because it is already written
		 in the macro function.
		 The CProdPic constructor object contain in order the Resource ID, the 
		 image type, the coordinate point in pixel for the Input, the Output,  
		 (the Bypass), the CapilPrim point, the CapilPrimAngle double, the CapilSec 
		 point, the CapilSecAngle double.

	OUT: -
	==============================================================================*/

	//ADDIMAGE(PX_DA50_32A,CProdPic(IDR_GIF1,CProdPic::PicLocSymb,pt(0,0),pt(0,0),pt(0,0),0));

	return TRUE;
}

