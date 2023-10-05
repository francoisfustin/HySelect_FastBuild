#include <stdafx.h>


#include "MacroDefs.h"

bool CMacroBaseClassData::Write( CFile* pclFile )
{
	if( NULL == pclFile )
	{
		return false;
	}

	return true;
}

bool CMacroBaseClassData::Read( CFile* pclFile )
{
	if( NULL == pclFile )
	{
		return false;
	}

	return true;
}

bool CMacroMFCTabCtrlData::Write( CFile* pclFile )
{
	if( false == CMacroBaseClassData::Write( pclFile ) )
	{
		return false;
	}
	
	pclFile->Write( &m_iTabID, sizeof( m_iTabID ) );

	return true;
}

bool CMacroMFCTabCtrlData::Read( CFile* pclFile )
{
	if( false == CMacroBaseClassData::Write( pclFile ) )
	{
		return false;
	}

	pclFile->Read( &m_iTabID, sizeof( m_iTabID ) );

	return true;
}

bool CMacroTreeCtrlData::Write( CFile* pclFile )
{
	if( false == CMacroBaseClassData::Write( pclFile ) )
	{
		return false;
	}
	
	pclFile->Write( &m_dwParam, sizeof( m_dwParam ) );

	return true;
}

bool CMacroTreeCtrlData::Read( CFile* pclFile )
{
	if( false == CMacroBaseClassData::Write( pclFile ) )
	{
		return false;
	}

	pclFile->Read( &m_dwParam, sizeof( m_dwParam ) );

	return true;
}

bool CMacroMFCRibbonBarData::Write( CFile* pclFile )
{
	if( false == CMacroBaseClassData::Write( pclFile ) )
	{
		return false;
	}
	
	// pclFile->Write( &m_dwParam, sizeof( m_dwParam ) );

	return true;
}

bool CMacroMFCRibbonBarData::Read( CFile* pclFile )
{
	if( false == CMacroBaseClassData::Write( pclFile ) )
	{
		return false;
	}

	// pclFile->Read( &m_dwParam, sizeof( m_dwParam ) );

	return true;
}

CMacroWndInfo::CMacroWndInfo()
{
	m_iID = -1;
	m_strClassName = _T("");
	m_iAdDataID = -1;
	m_pclAdData = NULL;
}

CMacroWndInfo::~CMacroWndInfo()
{
	if( NULL != m_pclAdData )
	{
		delete m_pclAdData;
	}
}

bool CMacroWndInfo::Write( CFile* pclFile )
{
	if( NULL == pclFile )
	{
		return false;
	}

	pclFile->Write( &m_iID, sizeof( m_iID ) );

	int iClassNameLen = ( m_strClassName.GetLength() + 1 ) * sizeof( TCHAR );
	pclFile->Write( &iClassNameLen, sizeof( iClassNameLen ) );
	pclFile->Write( (PCTSTR)m_strClassName, iClassNameLen );

	pclFile->Write( &m_iAdDataID, sizeof( m_iAdDataID ) );
	
	if( -1 != m_iAdDataID && NULL != m_pclAdData )
	{
		m_pclAdData->Write( pclFile );
	}
	
	return true;
}

bool CMacroWndInfo::Read( CFile* pclFile )
{
	if( NULL == pclFile )
	{
		return false;
	}

	pclFile->Read( &m_iID, sizeof( m_iID ) );

	int iClassNameLen = 0;
	pclFile->Read( &iClassNameLen, sizeof( iClassNameLen ) );
	m_strClassName.Preallocate( iClassNameLen );
	pclFile->Read( (void*)(PCTSTR)m_strClassName, iClassNameLen );

	pclFile->Read( &m_iAdDataID, sizeof( m_iAdDataID ) );

	if( -1 != m_iAdDataID )
	{
		switch( m_iAdDataID )
		{
			case AdData_CMFCTabCtrl:
				m_pclAdData = (CMacroBaseClassData*)new CMFCTabCtrl();
				break;
		}

		if( NULL == m_pclAdData )
		{
			return false;
		}

		m_pclAdData->Read( pclFile );
	}

	return true;
}

bool CMacro::Write( CFile* pclFile )
{
	if( NULL == pclFile )
	{
		return false;
	}

	pclFile->Write( &m_msg, sizeof( m_msg ) );
	pclFile->Write( &m_pt, sizeof( m_pt ) );
	pclFile->Write( &m_dwTickCount, sizeof( m_dwTickCount ) );

	int iWndInfoSize = (int)m_vecWndHierarchy.size();
	pclFile->Write( &iWndInfoSize, sizeof( iWndInfoSize ) );

	if( iWndInfoSize > 0 )
	{
		for( vecWndInfoIter iter = m_vecWndHierarchy.begin(); iter != m_vecWndHierarchy.end(); iter++ )
		{
			if( NULL == *iter )
			{
				return false;
			}

			(*iter)->Write( pclFile );
		}
	}

	return true;
}

bool CMacro::Read( CFile* pclFile )
{
	if( NULL == pclFile )
	{
		return false;
	}

	pclFile->Read( &m_msg, sizeof( m_msg ) );
	pclFile->Read( &m_pt, sizeof( m_pt ) );
	pclFile->Read( &m_dwTickCount, sizeof( m_dwTickCount ) );

	int iWndInfoSize = 0;
	pclFile->Read( &iWndInfoSize, sizeof( iWndInfoSize ) );

	if( iWndInfoSize > 0 )
	{
		for( int iLoop = 0; iLoop < iWndInfoSize; iLoop++ )
		{
			CMacroWndInfo *pclWndInfo = new CMacroWndInfo();

			if( NULL == pclWndInfo )
			{
				return false;
			}

			pclWndInfo->Read( pclFile );
			m_vecWndHierarchy.push_back( pclWndInfo );
		}
	}

	return true;
}
