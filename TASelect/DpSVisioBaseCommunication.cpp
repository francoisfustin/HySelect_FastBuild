#include "stdafx.h"
#include "DpSVisioIncludes.h"

CDpSVisioBaseCommunication::CDpSVisioBaseCommunication( CWnd *pParent, _CommunicationType eCommunicationType )
{
	m_pParent = pParent;
	m_eCommunicationType = eCommunicationType;
	InitializeCriticalSection( &m_CriticalSection );
}

bool CDpSVisioBaseCommunication::IsDpSVisioConnected( void )
{
	return m_pclDpSVisioInformation->m_bIsConnected;
}
