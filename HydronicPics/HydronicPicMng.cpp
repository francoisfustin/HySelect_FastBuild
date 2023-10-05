// ProdPics.cpp : Defines the exported functions for the DLL application.
//

#include "StdAfx.h"
#include "HydronicPicMng.h"
#include <assert.h>
#include <string>
#include <tchar.h>

CProdPic* GetProdPic( LPCTSTR DescriptorID, CProdPic::eProdPicType eType )
{
	multimap<_string, CProdPic*>::iterator it;
	
	it = m_mmapPicsList.find( DescriptorID );
	if( it == m_mmapPicsList.end() )
		return NULL;

	// Verify that function's arguments are correctly implemented. 'assert()' is a C++ function that works as a VERIFY in the MFC.
	if( it->first != DescriptorID )
		assert( 0 );
	
	if( eType < CProdPic::PicTASymb || eType >= CProdPic::LastPicType )
		assert( 0 );
	
	// Goes to the right type.
	while( it != m_mmapPicsList.end() && it->first == DescriptorID && it->second->GetProdPicType() != eType )
		it++;

	// After having incremented the iterator it verify the object is still the same.
	if( it == m_mmapPicsList.end() || it->first != DescriptorID )
		return NULL;

	return it->second;
}

CDynCircSch* GetDynCircSch( LPCTSTR DescriptorID )
{
	multimap <_string, CDynCircSch>::iterator it;
	
	// Verify the dynamic circuit scheme were found.
	it = m_mmapCircSchList.find(DescriptorID);
	if( it == m_mmapCircSchList.end() )
		return NULL;		
	
	if( it->first != DescriptorID )
		assert( 0 );
	
	return &it->second;	
}

bool VerifyImgFound(LPCTSTR DescriptorID)
{
	multimap <_string, CProdPic*>::iterator it;
	it = m_mmapPicsList.find(DescriptorID);
	// Verify he found the ID
	if(it == m_mmapPicsList.end())
		return false;
	else
		return true;
}