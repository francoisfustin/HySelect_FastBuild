// LPPDea.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "LPP_AT.h"
#include <assert.h>
#include <string>
#include <tchar.h>

CProdPic* GetLocProdPic(LPCTSTR DescriptorID)
{
	multimap <_string, CProdPic>::iterator it;
	
	it = m_mapLocPicsDeaList.find(DescriptorID);
	// Verify he found the ID
	if(it == m_mapLocPicsDeaList.end())
		return NULL;

	// Verify that function's arguments are correctly 
	// implemented. 'assert()' is a C++ function that 
	// works as a VERIFY in the MFC.
	if(it->first != DescriptorID)
		assert(0);

	if (it!=m_mapLocPicsDeaList.end())
		return &it->second;
	
	return NULL;
}
