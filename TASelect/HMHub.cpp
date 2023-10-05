#include "StdAfx.h"
#include "HMInclude.h"
#include "Select.h"


//////////////////////////////////////////////////////////////////////
//			CDS_HMHub
//////////////////////////////////////////////////////////////////////

CDS_HmHub::CDS_HmHub( CDataBase *pDataBase, LPCTSTR ID )
	: CDS_HydroMod( pDataBase, ID )
{
	m_bBypass = true;
	m_eSelMode = SelMode::Q;
	m_eCasing = Casing::Insulated;
	m_eMainFeedPos = MainFeedPos::Left;
	m_eApplication = Application::Heating;
	m_eEndOption = EndOption::Drain;
	m_BalTypeIDPtr = _NULL_IDPTR;
	m_SupplyValveIDPtr = _NULL_IDPTR;
	m_ReturnValveIDPtr = _NULL_IDPTR;
	m_PartnerConnectIDPtr = _NULL_IDPTR;
	m_StationsConnectIDPtr = _NULL_IDPTR;
	m_pSelectedInfos = new CSelectedInfos( this );
	m_pWC = NULL;
	m_WaterCharID = _T("");
	SetpWC();
}

CDS_HmHub::~CDS_HmHub()
{
	if( NULL != m_pSelectedInfos )
	{
		delete m_pSelectedInfos;
	}
}

void CDS_HmHub::Init( CString strBalTypeID, CTable *plcParent )
{
	IDPTR SchIDptr = _NULL_IDPTR;
	SetBalTypeID( strBalTypeID );
	bool bBvNeeded = true;

	if( strBalTypeID == _T("RVTYPE_BV" ) || strBalTypeID == _T( "RVTYPE_VV") )
	{
		SchIDptr = TASApp.GetpTADB()->Get( _T("DC_BV") );
		// Create Shut-Off valve on supply side.
		CreateSV( eHMObj::eShutoffValveSupply );
	}
	else if( strBalTypeID == _T("DPCTYPE_STD") )
	{
		// TODO CreateHub DpC Measuring valve primary or secondary
		SchIDptr = TASApp.GetpTADB()->Get( _T("DC_DP11") );				//Measuring valve in primary
		//SchIDptr = TASApp.GetpTADB()->Get(_T("DC_DP12"));				//Measuring valve in secondary
	}
	else if( strBalTypeID == _T("SHUTTYPE_BALL") )
	{
		SchIDptr = TASApp.GetpTADB()->Get( _T("SP") );
		// Create Shut-Off valve on supply and on return side
		CreateSV( eHMObj::eShutoffValveSupply );
		CreateSV( eHMObj::eShutoffValveReturn );
		bBvNeeded = false;
	}
	else
	{
		ASSERT( 0 );
	}

	CreateTermUnit();
	SetHMCalcMode( true );
	GetpTermUnit()->SetVirtual( true );
	SetFlagModule( true );

	// Type changed reset selected device.
	m_SupplyValveIDPtr = _NULL_IDPTR;
	m_ReturnValveIDPtr = _NULL_IDPTR;

	CDS_HydroMod::Init( SchIDptr, plcParent, bBvNeeded );
}

void CDS_HmHub::Copy( CDS_HydroMod *pHMHub, bool bValveDataOnly, bool bCreateLinkedDevice, bool bToClipboard )
{
	// No associated device with a Hub.
	CDS_HydroMod::Copy( pHMHub, false, false, bToClipboard );
	( ( CDS_HmHub * )pHMHub )->SetCasing( m_eCasing );
	( ( CDS_HmHub * )pHMHub )->SetMainFeedPos( m_eMainFeedPos );
	( ( CDS_HmHub * )pHMHub )->SetBypass( m_bBypass );
	( ( CDS_HmHub * )pHMHub )->SetApplication( m_eApplication );
	( ( CDS_HmHub * )pHMHub )->SetEndOption( m_eEndOption );
	( ( CDS_HmHub * )pHMHub )->SetSelMode( m_eSelMode );

	if( GetBalTypeID() == ( ( CDS_HmHub * )pHMHub )->GetBalTypeID() )
	{
		( ( CDS_HmHub * )pHMHub )->SetSupplyValveID( m_SupplyValveIDPtr.ID );
		( ( CDS_HmHub * )pHMHub )->SetReturnValveID( m_ReturnValveIDPtr.ID );
	}

	( ( CDS_HmHub * )pHMHub )->SetPartnerConnectIDPtr( m_PartnerConnectIDPtr );
	( ( CDS_HmHub * )pHMHub )->SetStationsConnectIDPtr( m_StationsConnectIDPtr );
	( ( CDS_HmHub * )pHMHub )->SetBalTypeID( m_BalTypeIDPtr.ID );
	m_pSelectedInfos->Copy( ( ( CDS_HmHub * )pHMHub )->GetpSelectedInfos() );
}

bool CDS_HmHub::IsCompletelyDefined()
{
	bool bRet = true;

	// Check all children.
	for( IDPTR IDPtr = GetFirst(); '\0' != *IDPtr.ID && true == bRet; IDPtr = GetNext() )
	{
		CDS_HmHubStation *pHubStation = ( CDS_HmHubStation * )IDPtr.MP;
		bRet = pHubStation->IsCompletelyDefined();
	}

	// Check Hub.
	bRet &= CDS_HydroMod::IsCompletelyDefined();
	return bRet;
}

CString CDS_HmHub::RenameMe( bool bForceAuto )
{
	// Rename Hub.
	TCHAR cFirstChar = bForceAuto ? '*' : '\0';
	CString str = TASApp.LoadLocalizedString( IDS_HUBPREFIX );
	str += _T(" ");
	SetHMName( TASApp.GetpTADS()->ComposeHUBHMName( this, cFirstChar, true, str, false ) );
	return GetHMName();
}

void CDS_HmHub::ResizeShutOffValves()
{
	if( 0 == GetBalTypeID().CompareNoCase( _T("SHUTTYPE_BALL") ) )
	{
		if( true == IsShutoffValveExist( eHMObj::eShutoffValveReturn ) )
		{
			IDPTR idptr = GetpShutoffValve( eHMObj::eShutoffValveReturn )->SelectBestShutoffValve();
			SetReturnValveID( FindHubPtnValve( CDB_HubValv::eSupplyReturnValv::Return, idptr ).ID );
		}
	}
}

void CDS_HmHub::SetpWC( CString strID_WC )
{
	m_pWC = ( ( CDS_WaterCharacteristic * )TASApp.GetpTADS()->Get( ( LPCTSTR )strID_WC ).MP )->GetpWCData();
	ASSERT( NULL != m_pWC );
	m_WaterCharID = strID_WC;
	
	// Copy WaterChar information into SelectedInfo.
	*m_pSelectedInfos->GetpWCData() = *m_pWC;
}

CWaterChar *CDS_HmHub::GetpWC()
{
	CString IDstr = GetIDPtr().PP->GetIDPtr().ID;

	// In case of single selection use Waterchar saved into CSelectedInfo.
	if( _T("SELHUB_TAB" ) == IDstr || _T( "TMPHUB_TAB") == IDstr )
	{
		m_pWC = m_pSelectedInfos->GetpWCData();
	}
	else if( NULL == m_pWC )
	{
		m_pWC = ( ( CDS_WaterCharacteristic * )TASApp.GetpTADS()->Get( ( LPCTSTR )m_WaterCharID ).MP )->GetpWCData();
	}

	ASSERT( NULL != m_pWC );
	return m_pWC;
}

IDPTR CDS_HmHub::GetTADBValveIDPtr()
{
	CString TypeID = GetBalTypeIDPtr().ID;
	CDB_HubValv *pValv = NULL;

	if( _T("DPCTYPE_STD") == TypeID )
	{
		pValv = ( CDB_HubValv * )GetSupplyValveIDPtr().MP;
	}
	else
	{
		pValv = ( CDB_HubValv * )GetReturnValveIDPtr().MP;
	}

	IDPTR idptr = _NULL_IDPTR;

	if( NULL != pValv )
	{
		idptr = pValv->GetValveIDPtr();
	}

	return idptr;
}

//--------------------------------------------------------------------------------------
// bForBVSizing is used to return associated Balancing valve type in case of the Hub is DpCType
// TODO : Hard coded ???
IDPTR CDS_HmHub::GetBalTypeIDPtr( bool bForBVSizing )
{
	ASSERT( *m_BalTypeIDPtr.ID );

	if( '\0' != *m_BalTypeIDPtr.ID )
	{
		if( NULL == m_BalTypeIDPtr.MP )
		{
			m_BalTypeIDPtr.DB = TASApp.GetpTADB();
			Extend( &m_BalTypeIDPtr );
		}

		if( true == bForBVSizing )
		{
			CString TypeID = m_BalTypeIDPtr.ID;

			if( 0 == TypeID.CompareNoCase( _T("DPCTYPE_STD") ) )
			{
				return ( TASApp.GetpTADB()->Get( _T("RVTYPE_BV") ) );
			}
		}

		return m_BalTypeIDPtr;
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HmHub::GetSupplyValveIDPtr()
{
	if( '\0' != *m_SupplyValveIDPtr.ID )
	{
		if( NULL == m_SupplyValveIDPtr.MP )
		{
			m_SupplyValveIDPtr.DB = TASApp.GetpTADB();
			Extend( &m_SupplyValveIDPtr );
		}

		return m_SupplyValveIDPtr;
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HmHub::GetReturnValveIDPtr()
{
	if( '\0' != *m_ReturnValveIDPtr.ID )
	{
		if( NULL == m_ReturnValveIDPtr.MP )
		{
			m_ReturnValveIDPtr.DB = TASApp.GetpTADB();
			Extend( &m_ReturnValveIDPtr );
		}

		return m_ReturnValveIDPtr;
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HmHub::GetPartnerConnectIDPtr()
{
	if( '\0' != *m_PartnerConnectIDPtr.ID )
	{
		if( NULL == m_PartnerConnectIDPtr.MP )
		{
			m_PartnerConnectIDPtr.DB = TASApp.GetpTADB();
			Extend( &m_PartnerConnectIDPtr );
		}

		return m_PartnerConnectIDPtr;
	}

	return _NULL_IDPTR;
}

IDPTR CDS_HmHub::GetStationsConnectIDPtr()
{
	if( '\0' != *m_StationsConnectIDPtr.ID )
	{
		if( NULL == m_StationsConnectIDPtr.MP )
		{
			m_StationsConnectIDPtr.DB = TASApp.GetpTADB();
			Extend( &m_StationsConnectIDPtr );
		}

		return m_StationsConnectIDPtr;
	}

	return _NULL_IDPTR;
}

bool CDS_HmHub::SetBalTypeID( CString strBalTypeID )
{
	ASSERT( false == strBalTypeID.IsEmpty() );

	if( false == strBalTypeID.IsEmpty() )
	{
		// ID already match.
		if( 0 == _tcscmp( m_BalTypeIDPtr.ID, ( LPCTSTR )strBalTypeID ) )
		{
			return true;
		}

		ASSERT( strBalTypeID.GetLength() <= _ID_LENGTH );

		if( strBalTypeID.GetLength() <= _ID_LENGTH )
		{
			m_BalTypeIDPtr = TASApp.GetpTADB()->Get( strBalTypeID );
			Modified();
			return true;
		}
	}

	return false;
}

void CDS_HmHub::SetSupplyValveID( CString SupplyValveID )
{
	if( ( ( CString )m_SupplyValveIDPtr.ID ) != SupplyValveID )
	{
		m_SupplyValveIDPtr = _NULL_IDPTR;
		_tcsncpy_s( m_SupplyValveIDPtr.ID, SIZEOFINTCHAR( m_SupplyValveIDPtr.ID ), ( LPCTSTR )SupplyValveID, SIZEOFINTCHAR( m_SupplyValveIDPtr.ID ) - 1 );

		if( true == SupplyValveID.IsEmpty() )
		{
			return;
		}

		if( true == IsShutoffValveExist( CDS_HydroMod::eHMObj::eShutoffValveSupply ) )
		{
			CDB_HubValv *pSupValv = dynamic_cast<CDB_HubValv *>( GetSupplyValveIDPtr().MP );
			ASSERT( NULL != pSupValv );

			if( NULL == pSupValv )
			{
				return;
			}

			GetpShutoffValve( eHMObj::eShutoffValveSupply )->SetBestIDPtr( pSupValv->GetValveIDPtr() );
			GetpShutoffValve( eHMObj::eShutoffValveSupply )->SetIDPtr( pSupValv->GetValveIDPtr() );
		}

		Modified();
	}
}

void CDS_HmHub::SetSupplyValveIDPtr( IDPTR SupplyValveIDPtr )
{
	m_SupplyValveIDPtr = SupplyValveIDPtr;

	// In case of DpC, size the Balancing valve after the DpC selection
	CString TypeID = GetBalTypeIDPtr().ID;

	if( 0 == TypeID.CompareNoCase( _T("DPCTYPE_STD") ) )
	{
		IDPTR IDPtrMeasValve = ( ( CDB_HubValv * )m_SupplyValveIDPtr.MP )->GetValveIDPtr();
		SetLock( CDS_HydroMod::eHMObj::eBVprim, false, false );
		GetpBv()->SetBestBvIDPtr( IDPtrMeasValve );
		GetpBv()->SetIDPtr( IDPtrMeasValve );
		SetLock( CDS_HydroMod::eHMObj::eBVprim, true, false );
		GetpBv()->ResizeBv( GetpBv()->GetDpMin() );
	}
	else if( true == IsShutoffValveExist( CDS_HydroMod::eHMObj::eShutoffValveSupply ) )
	{
		CDB_HubValv *pSupValv = ( CDB_HubValv * )m_SupplyValveIDPtr.MP;
		GetpShutoffValve( eHMObj::eShutoffValveSupply )->SetBestIDPtr( pSupValv->GetValveIDPtr() );
		GetpShutoffValve( eHMObj::eShutoffValveSupply )->SetIDPtr( pSupValv->GetValveIDPtr() );
	}

}

void CDS_HmHub::SetReturnValveID( CString strReturnValveID )
{
	m_ReturnValveIDPtr = _NULL_IDPTR;
	_tcsncpy_s( m_ReturnValveIDPtr.ID, SIZEOFINTCHAR( m_ReturnValveIDPtr.ID ), ( LPCTSTR )strReturnValveID, SIZEOFINTCHAR( m_ReturnValveIDPtr.ID ) - 1 );
	Modified();

	if( true == strReturnValveID.IsEmpty() )
	{
		return;
	}

	CDB_HubValv *pRetValv = ( CDB_HubValv * )GetReturnValveIDPtr().MP;
	ASSERT( NULL != pRetValv );

	if( NULL == pRetValv )
	{
		return;
	}

	if( 0 == GetBalTypeID().CompareNoCase( _T("SHUTTYPE_BALL") ) )
	{
		GetpShutoffValve( eHMObj::eShutoffValveReturn )->SetBestIDPtr( pRetValv->GetValveIDPtr() );
		GetpShutoffValve( eHMObj::eShutoffValveReturn )->SetIDPtr( pRetValv->GetValveIDPtr() );
	}

	// Verify and Set Bv/DpC according to the return valve ID
	CString TypeID = GetBalTypeIDPtr().ID;

	if( 0 == TypeID.CompareNoCase( _T("DPCTYPE_STD") ) )
	{
		if( GetpDpC()->GetIDPtr().MP != pRetValv->GetValveIDPtr().MP )
		{
			GetpDpC()->SetIDPtr( pRetValv->GetValveIDPtr() );
		}
		else
		{
			if( GetpBv()->GetIDPtr().MP != pRetValv->GetValveIDPtr().MP )
			{
				GetpBv()->SetIDPtr( pRetValv->GetValveIDPtr() );
			}
		}
	}

	// Find corresponding Supply Valve.
	CTable *pTab = ( CTable * )TASApp.GetpTADB()->Get( _T("HUBSUPRET_TAB") ).MP;
	ASSERT( NULL != pTab );

	if( NULL == pTab )
	{
		return;
	}

	CTable *pTabValve = ( CTable * )GetReturnValveIDPtr().PP;
	ASSERT( NULL != pTabValve );

	if( NULL == pTabValve )
	{
		return;
	}

	IDPTR IDPtrPairs = _NULL_IDPTR;

	// Find matching pairs.
	for( IDPtrPairs = pTab->GetFirst(); '\0' != *IDPtrPairs.ID; IDPtrPairs = pTab->GetNext() )
	{
		CDB_HubSupRetLnk *pHubSR = ( CDB_HubSupRetLnk * )IDPtrPairs.MP;

		if( ( pHubSR->GetTAISReturn() == pRetValv->GetTAISPartner() ) )
		{
			// Pairs found
			// Search supply valve with the same size.
			for( IDPTR IDPtr = pTabValve->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTabValve->GetNext() )
			{
				CDB_HubValv *pSupValv = ( CDB_HubValv * )IDPtr.MP;

				if( pSupValv->GetSupplyOrReturn() != CDB_HubValv::eSupplyReturnValv::Supply )
				{
					continue;
				}

				if( ( pSupValv->GetTAISPartner() == pHubSR->GetTAISSupply() ) &&
					( pSupValv->GetTAISSize() == pRetValv->GetTAISSize() ) )
				{
					// Found
					SetSupplyValveIDPtr( IDPtr );

					if( 0 == pRetValv->GetTAISSize().Compare( _T("DN25") ) )
					{
						SetPartnerConnectIDPtr( TASApp.GetpTADB()->Get( _T("CONN_ISO7_1F") ) );
					}
					else
					{
						SetPartnerConnectIDPtr( TASApp.GetpTADB()->Get( _T("CONN_ISO7_114F") ) );
					}

					break;
				}
			}
		}
	}
}

IDPTR CDS_HmHub::FindHubPtnValve( CDB_HubValv::eSupplyReturnValv SupRet, IDPTR SelValveIDPtr )
{
	if( NULL != SelValveIDPtr.MP )
	{
		CTable *pTab = ( CTable * )TASApp.GetpTADB()->Get( _T("HUBVALV_TAB") ).MP;

		for( IDPTR idptr = pTab->GetFirst( CLASS( CDB_HubValv ) ); '\0' != *idptr.ID; idptr = pTab->GetNext() )
		{
			CDB_HubValv *pHV = ( CDB_HubValv * )idptr.MP;

			if( pHV->GetSupplyOrReturn() != SupRet )
			{
				continue;
			}

			if( pHV->GetValveIDPtr().MP == SelValveIDPtr.MP )
			{
				return idptr;
			}
		}
	}

	return _NULL_IDPTR;
}

void CDS_HmHub::RenameStations()
{
	// Rename Stations.
	for( IDPTR idptr = GetFirst(); '\0' != *idptr.ID; idptr = GetNext() )
	{
		CDS_HmHubStation *pHubStation = ( CDS_HmHubStation * )idptr.MP;

		// Rename only if the first char is '*'.
		if( '*' == *pHubStation->GetHMName() )
		{
			pHubStation->SetHMName( TASApp.GetpTADS()->ComposeHUBHMName( pHubStation, '*', true ) );
		}
	}
}

CString CDS_HmHub::CheckMaxTotalFlow()
{
	const double dMaxTotQ = 4500.0 / 3600.0 / 1000.0;	// 3500l/h in m3/s
	CString str, strMaxQ;

	if( GetQ() > dMaxTotQ )
	{
		strMaxQ = WriteCUDouble( _U_FLOW, dMaxTotQ, true );
		FormatString( str, IDS_SHEETSSELP_MAXFLOWWARNINGMSG, strMaxQ );
	}

	return str;
}

int CDS_HmHub::CompareSelectionTo( CData *pclSelectionCompareWith, int iKey )
{
	if( NULL == pclSelectionCompareWith )
	{
		return 0;
	}

	CDS_HmHub *pclHmHubCompareWith = dynamic_cast<CDS_HmHub*>( pclSelectionCompareWith );

	if( NULL == pclHmHubCompareWith )
	{
		return CDS_HydroMod::CompareSelectionTo( pclSelectionCompareWith, iKey );
	}

	CDB_HubValv *pclHubValveToCompare = dynamic_cast<CDB_HubValv *>( m_ReturnValveIDPtr.MP );
	CDB_HubValv *pclHubValveCompareWidth = dynamic_cast<CDB_HubValv *>( pclHmHubCompareWith->GetReturnValveIDPtr().MP );

	int iReturn = 0;
	switch( iKey )
	{
		case PageField_enum::epfBVINFOPRODUCTNAME:
			if( NULL != pclHubValveToCompare && NULL != pclHubValveCompareWidth )
			{
				iReturn = StringCompare( pclHubValveToCompare->GetTAISPartner(), pclHubValveCompareWidth->GetTAISPartner() );
			}
			break;

		case PageField_enum::epfBVINFOPRODUCTSIZE:
			if( NULL != pclHubValveToCompare && NULL != pclHubValveCompareWidth )
			{
				iReturn = StringCompare( pclHubValveToCompare->GetTAISSize(), pclHubValveCompareWidth->GetTAISSize() );
			}
			break;

		case PageField_enum::epfARTICLE:
			if( NULL != pclHubValveToCompare && NULL != pclHubValveCompareWidth )
			{
				CDB_TAProduct *pclValveToCompare = dynamic_cast<CDB_TAProduct *>( pclHubValveToCompare->GetValveIDPtr().MP );
				CDB_TAProduct *pclValveCompareWith = dynamic_cast<CDB_TAProduct *>( pclHubValveCompareWidth->GetValveIDPtr().MP );

				if( NULL != pclValveToCompare && NULL != pclValveCompareWith )
				{
					if( false == TASApp.IsTAArtNumberHidden() )
					{
						iReturn = StringCompare( pclValveToCompare->GetArtNum(), pclValveCompareWith->GetArtNum() );
					}
				}
				else
				{
					CString strLocArtNum1 = LocArtNumTab.GetLocalArticleNumber( pclValveToCompare->GetArtNum() );
					CString strLocArtNum2 = LocArtNumTab.GetLocalArticleNumber( pclValveCompareWith->GetArtNum() );
					iReturn = StringCompare( strLocArtNum1, strLocArtNum2 );
				}
			}
			break;

		default:
			iReturn = CDS_HydroMod::CompareSelectionTo( pclSelectionCompareWith, iKey );
	}

	return iReturn;
}

#define CDS_HMHUB_VERSION	1
void CDS_HmHub::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CDS_HMHUB_VERSION;
	outf.write( ( char * )&Version, sizeof( Version ) );
	
	// Base class.
	CDS_HydroMod::Write( outf );
	
	// Info.
	outf.write( ( char * )&m_eCasing, sizeof( m_eCasing ) );
	outf.write( ( char * )&m_eMainFeedPos, sizeof( m_eMainFeedPos ) );
	outf.write( ( char * )&m_bBypass, sizeof( m_bBypass ) );
	outf.write( ( char * )&m_eApplication, sizeof( m_eApplication ) );
	outf.write( ( char * )&m_eEndOption, sizeof( m_eEndOption ) );
	WriteString( outf, ( LPCTSTR )m_BalTypeIDPtr.ID );
	outf.write( ( char * )&m_eSelMode, sizeof( m_eSelMode ) );
	WriteString( outf, ( LPCTSTR )m_SupplyValveIDPtr.ID );
	WriteString( outf, ( LPCTSTR )m_ReturnValveIDPtr.ID );
	WriteString( outf, ( LPCTSTR )m_PartnerConnectIDPtr.ID );
	WriteString( outf, ( LPCTSTR )m_StationsConnectIDPtr.ID );
	
	// Selected infos.
	m_pSelectedInfos->Write( outf );
}

bool CDS_HmHub::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CDS_HMHUB_VERSION;
	inpf.read( ( char * )&Version, sizeof( Version ) );

	if( Version < 1 || Version > CDS_HMHUB_VERSION )
	{
		return false;
	}

	// Base class.
	if( false == CDS_HydroMod::Read( inpf ) )
	{
		return false;
	}

	// Info.
	inpf.read( ( char * )&m_eCasing, sizeof( m_eCasing ) );
	inpf.read( ( char * )&m_eMainFeedPos, sizeof( m_eMainFeedPos ) );
	inpf.read( ( char * )&m_bBypass, sizeof( m_bBypass ) );
	inpf.read( ( char * )&m_eApplication, sizeof( m_eApplication ) );
	inpf.read( ( char * )&m_eEndOption, sizeof( m_eEndOption ) );
	ReadString( inpf, m_BalTypeIDPtr.ID, sizeof( m_BalTypeIDPtr.ID ) );
	inpf.read( ( char * )&m_eSelMode, sizeof( m_eSelMode ) );
	m_SupplyValveIDPtr = _NULL_IDPTR;
	m_ReturnValveIDPtr = _NULL_IDPTR;
	ReadString( inpf, m_SupplyValveIDPtr.ID, sizeof( m_SupplyValveIDPtr.ID ) );
	ReadString( inpf, m_ReturnValveIDPtr.ID, sizeof( m_ReturnValveIDPtr.ID ) );
	ReadString( inpf, m_PartnerConnectIDPtr.ID, sizeof( m_PartnerConnectIDPtr.ID ) );
	ReadString( inpf, m_StationsConnectIDPtr.ID, sizeof( m_StationsConnectIDPtr.ID ) );
	
	// Selected info.
	m_pSelectedInfos->Read( inpf );
	return true;
}

//////////////////////////////////////////////////////////////////////
//			CDS_HmHubStation
//////////////////////////////////////////////////////////////////////

CDS_HmHubStation::CDS_HmHubStation( CDataBase *pDataBase, LPCTSTR ID )
	: CDS_HydroMod( pDataBase, ID )
{
	m_CvTaKv = eCVTAKV::eNone;
	m_BalTypeIDPtr = _NULL_IDPTR;
	m_SupplyValveIDPtr = _NULL_IDPTR;
	m_ReturnValveIDPtr = _NULL_IDPTR;
	m_ActuatorGroupIDPtr = _NULL_IDPTR;
	m_ReturnActuatorIDPtr = _NULL_IDPTR;
	m_bPlugged = false;
}

void CDS_HmHubStation::Init( CString BalTypeID, CTable *plcParent, eCVTAKV CvTaKv )
{
	IDPTR SchIDptr = _NULL_IDPTR;

	// Type changing delete previous allocated structure
	if( IsBvExist() )
	{
		DeleteBv( &m_pBv );
	}

	if( IsDpCExist() )
	{
		DeleteDpC();
	}

	if( !SetBalTypeID( BalTypeID ) )
	{
		return;
	}

	// Remove locking if exist
	SetLock( eHMObj::eALL, false, false, true );
	m_CvTaKv = CvTaKv;

	if( BalTypeID == _T("RVTYPE_BV" ) || BalTypeID == _T("RVTYPE_VV") || BalTypeID == _T( "RVTYPE_TBV") )
	{
		SchIDptr = TASApp.GetpTADB()->Get( _T("DC_BV") );
	}
	else if( BalTypeID == _T("RVTYPE_BVC") )
	{
		SchIDptr = TASApp.GetpTADB()->Get( _T("2WD_BV3") );
		CvTaKv = eCVTAKV::eTAValve;
	}
	else
	{
		ASSERT( 0 );
		return;
	}

	if( CvTaKv != eCVTAKV::eNone )
	{
		CDB_CircuitScheme *pSch = ( CDB_CircuitScheme * )SchIDptr.MP;

		if( NULL == GetpCV() )
		{
			CreateCv( pSch->GetCvLoc() );
		}

		if( BalTypeID != _T("RVTYPE_BVC") )
		{
			SchIDptr = TASApp.GetpTADB()->Get( _T("2WD_BV1") );
			// For Hub station CV are always KvMode
			GetpCV()->SetTACv( false );
		}
		else
		{
			GetpCV()->SetTACv( true );
			GetpCV()->SetCtrlType( CDB_ControlProperties::CvCtrlType::eCvOnOff );
		}

		// No more balancing valve needed when control valve is PresetPt or Presettable
		if( pSch->GetCvFunc() == CDB_ControlProperties::eCVFUNC::ControlOnly )
		{
			CDS_HydroMod::Init( SchIDptr, plcParent, true );    // Balancing valve needed
		}
		else
		{
			CDS_HydroMod::Init( SchIDptr, plcParent, false );
		}
	}
	else
	{
		if( GetpCV() )
		{
			//GetpCV()->RemoveLinkedTAProd();
			DeleteCv();
		}

		CDS_HydroMod::Init( SchIDptr, plcParent, true );	// Balancing valve needed
	}

	CreateTermUnit();
	SetHMCalcMode( true );
	CreateSV( eHMObj::eShutoffValveSupply );
	GetpTermUnit()->SetVirtual( false );
	SetFlagModule( false );
	SetSchemeID( SchIDptr.ID );
	// Type Changed reset selected device
	m_SupplyValveIDPtr = _NULL_IDPTR;
	m_ReturnValveIDPtr = _NULL_IDPTR;
	m_ActuatorGroupIDPtr = _NULL_IDPTR;
	m_ReturnActuatorIDPtr = _NULL_IDPTR;
}
#define CDS_HMHUBSTATION_VERSION	2
void CDS_HmHubStation::Write( OUTSTREAM outf )
{
	// Version
	BYTE Version = CDS_HMHUBSTATION_VERSION;
	outf.write( ( char * )&Version, sizeof( Version ) );
	// Base class
	CDS_HydroMod::Write( outf );
	// Info
	outf.write( ( char * )&m_CvTaKv, sizeof( Version ) );
	WriteString( outf, ( LPCTSTR )m_BalTypeIDPtr.ID );
	WriteString( outf, ( LPCTSTR )m_SupplyValveIDPtr.ID );
	WriteString( outf, ( LPCTSTR )m_ReturnValveIDPtr.ID );
	WriteString( outf, ( LPCTSTR )m_ActuatorGroupIDPtr.ID );
	WriteString( outf, ( LPCTSTR )m_ReturnActuatorIDPtr.ID );
	outf.write( ( char * )&m_bPlugged, sizeof( m_bPlugged ) );
}
void CDS_HmHubStation::PatchActr()
{
	// Patch TSE 150 actuator that had been removed TSE100_NC, TSE100_NO
	if( *m_ReturnActuatorIDPtr.ID )
	{
		if( !IDcmp( _T("TSE100_NC"), m_ReturnActuatorIDPtr.ID ) )
		{
			_tcsncpy_s( m_ReturnActuatorIDPtr.ID, SIZEOFINTCHAR( m_ReturnActuatorIDPtr.ID ), _T("TSE-NC-24-2"), _ID_LENGTH );
		}
		else if( !IDcmp( _T("TSE100_NO"), m_ReturnActuatorIDPtr.ID ) )
		{
			_tcsncpy_s( m_ReturnActuatorIDPtr.ID, SIZEOFINTCHAR( m_ReturnActuatorIDPtr.ID ), _T("TSE-NO-24-2"), _ID_LENGTH );
		}
	}

	// Patch TSE for fixing database problem for version up to 4.0.1.4
	if( *m_ReturnActuatorIDPtr.ID )
	{
		CString str = m_ReturnActuatorIDPtr.ID;
		int pos = str.Find( _T("-24-5") );

		if( pos > -1 )
		{
			str.Delete( pos, str.GetLength() - pos );
			str += _T("-24-2");
		}
		else
		{
			pos = str.Find( _T("-230-") );

			if( pos > -1 )
			{
				str.Delete( pos, str.GetLength() - pos );
				str += _T("-24-2");
			}
			else
			{
				pos = str.Find( _T("-2-") );

				if( pos > -1 )
				{
					str.Delete( pos, str.GetLength() - pos );
					str += _T("-2");
				}
			}

			ASSERT( str == _T("TSE-NO-24-2" ) || str == _T("TSE-NC-24-2") || str == _T( "TSE-M-24-2") );
			_tcsncpy_s( m_ReturnActuatorIDPtr.ID, SIZEOFINTCHAR( m_ReturnActuatorIDPtr.ID ), ( LPCTSTR )str, _ID_LENGTH );
		}
	}

	// Fix ActGr
	if( *m_ActuatorGroupIDPtr.ID )
	{
		CString str = m_ActuatorGroupIDPtr.ID;
		int pos = str.Find( _T("ACTGR_TSE") );

		if( pos > -1 )
		{
			str.Insert( 6, _T("HUB_") );
		}

		ASSERT( str == _T("ACTGR_HUB_TSE" ) || str == _T( "ACTGR_HUB_TSE-M") );
		_tcsncpy_s( m_ActuatorGroupIDPtr.ID, SIZEOFINTCHAR( m_ActuatorGroupIDPtr.ID ), ( LPCTSTR )str, _ID_LENGTH );
	}
}
bool CDS_HmHubStation::Read( INPSTREAM inpf )
{
	// Version
	BYTE Version = CDS_HMHUBSTATION_VERSION;
	inpf.read( ( char * )&Version, sizeof( Version ) );

	if( Version < 1 || Version > CDS_HMHUBSTATION_VERSION )
	{
		return false;
	}

	// Base class
	if( !CDS_HydroMod::Read( inpf ) )
	{
		return false;
	}

	// Info
	inpf.read( ( char * )&m_CvTaKv, sizeof( Version ) );
	m_BalTypeIDPtr = _NULL_IDPTR;
	m_SupplyValveIDPtr = _NULL_IDPTR;
	m_ReturnValveIDPtr = _NULL_IDPTR;
	m_ActuatorGroupIDPtr = _NULL_IDPTR;
	m_ReturnActuatorIDPtr = _NULL_IDPTR;
	ReadString( inpf, m_BalTypeIDPtr.ID, sizeof( m_BalTypeIDPtr.ID ) );
	ReadString( inpf, m_SupplyValveIDPtr.ID, sizeof( m_SupplyValveIDPtr.ID ) );
	ReadString( inpf, m_ReturnValveIDPtr.ID, sizeof( m_ReturnValveIDPtr.ID ) );
	ReadString( inpf, m_ActuatorGroupIDPtr.ID, sizeof( m_ActuatorGroupIDPtr.ID ) );
	ReadString( inpf, m_ReturnActuatorIDPtr.ID, sizeof( m_ReturnActuatorIDPtr.ID ) );

	if( Version > 1 )
	{
		inpf.read( ( char * )&m_bPlugged, sizeof( m_bPlugged ) );
	}

	PatchActr();
	return true;
}
void CDS_HmHubStation::Copy( CDS_HydroMod *pHMHub, bool bValveDataOnly /*= false*/, bool bCreateLinkedDevice/*=true*/, bool bToClipboard/*=false*/ )
{
	if( GetCvTaKv() != ( ( CDS_HmHubStation * )pHMHub )->GetCvTaKv() )
	{
		( ( CDS_HmHubStation * )pHMHub )->SetControlValve( GetCvTaKv(), GetCvCtrlType() );
	}

	//CDS_HydroMod::Copy(pHMHub,bValveDataOnly,bCreateLinkedDevice, bToClipboard);
	//No associated device with a Hub
	CDS_HydroMod::Copy( pHMHub, false, false, bToClipboard );
	( ( CDS_HmHubStation * )pHMHub )->SetBalTypeID( m_BalTypeIDPtr.ID );
	( ( CDS_HmHubStation * )pHMHub )->SetSupplyValveID( m_SupplyValveIDPtr.ID );
	( ( CDS_HmHubStation * )pHMHub )->SetReturnValveID( m_ReturnValveIDPtr.ID );
	//((CDS_HmHubStation *)pHMHub)->SetActuatorGroupID(m_ActuatorGroupIDPtr.ID);
	( ( CDS_HmHubStation * )pHMHub )->SetReturnActuatorID( m_ReturnActuatorIDPtr.ID );
	( ( CDS_HmHubStation * )pHMHub )->SetPlugged( m_bPlugged );
}

void CDS_HmHubStation::ResizeBalDevForH( double Havail, bool bForPending, CAnchorPt::CircuitSide eCircuitSide )
{
	if( Havail && !GetpDpC() )
	{
		if( IsCvExist() )
		{
			if( !GetpCV()->IsPresettable() )
			{
				Havail -= GetpCV()->GetDp();
			}
		}
	}

	CDS_HydroMod::ResizeBalDevForH( Havail, bForPending, eCircuitSide );
}

double CDS_HmHubStation::GetHmin()
{
	if( IsPlugged() )
	{
		return 0.0;
	}
	else
	{
		return ( CDS_HydroMod::GetDp(true) );
	}
}
// Return current Dp for the circuit
/*virtual*/ double CDS_HmHubStation::GetDp( bool fDpMin/*= false*/ )
{
	if( IsPlugged() )
	{
		return 0.0;
	}
	else
	{
		return ( CDS_HydroMod::GetDp() );
	}
}

IDPTR CDS_HmHubStation::GetBalTypeIDPtr()
{
	ASSERT( *m_BalTypeIDPtr.ID );

	if( *m_BalTypeIDPtr.ID )
	{
		if( !m_BalTypeIDPtr.MP )
		{
			m_BalTypeIDPtr.DB = TASApp.GetpTADB();
			Extend( &m_BalTypeIDPtr );
		}

		return m_BalTypeIDPtr;
	}

	return _NULL_IDPTR;
}
IDPTR CDS_HmHubStation::GetSupplyValveIDPtr()
{
	if( *m_SupplyValveIDPtr.ID )
	{
		if( !m_SupplyValveIDPtr.MP )
		{
			m_SupplyValveIDPtr.DB = TASApp.GetpTADB();
			Extend( &m_SupplyValveIDPtr );
		}

		return m_SupplyValveIDPtr;
	}

	return _NULL_IDPTR;
}
IDPTR CDS_HmHubStation::GetReturnValveIDPtr()
{
	if( *m_ReturnValveIDPtr.ID )
	{
		if( !m_ReturnValveIDPtr.MP )
		{
			m_ReturnValveIDPtr.DB = TASApp.GetpTADB();
			Extend( &m_ReturnValveIDPtr );
		}

		return m_ReturnValveIDPtr;
	}

	return _NULL_IDPTR;
}
IDPTR CDS_HmHubStation::GetActuatorGroupIDPtr()
{
	if( *m_ActuatorGroupIDPtr.ID )
	{
		if( !m_ActuatorGroupIDPtr.MP )
		{
			m_ActuatorGroupIDPtr.DB = TASApp.GetpTADB();
			Extend( &m_ActuatorGroupIDPtr );
		}

		return m_ActuatorGroupIDPtr;
	}

	return _NULL_IDPTR;
}
IDPTR	CDS_HmHubStation::GetReturnActuatorIDPtr()
{
	if( *m_ReturnActuatorIDPtr.ID )
	{
		if( !m_ReturnActuatorIDPtr.MP )
		{
			m_ReturnActuatorIDPtr.DB = TASApp.GetpTADB();
			Extend( &m_ReturnActuatorIDPtr );
		}

		return m_ReturnActuatorIDPtr;
	}

	return _NULL_IDPTR;
}
void CDS_HmHubStation::SetSupplyValveID( CString SupplyValveID )
{
	if( ( ( CString )m_SupplyValveIDPtr.ID ) != SupplyValveID )
	{
		m_SupplyValveIDPtr = _NULL_IDPTR;
		_tcsncpy_s( m_SupplyValveIDPtr.ID, SIZEOFINTCHAR( m_SupplyValveIDPtr.ID ), ( LPCTSTR )SupplyValveID, SIZEOFINTCHAR( m_SupplyValveIDPtr.ID ) - 1 );

		if( true == IsShutoffValveExist( eHMObj::eShutoffValveSupply ) )
		{
			CDB_HubStaValv *pSupValv = dynamic_cast<CDB_HubStaValv *>( GetSupplyValveIDPtr().MP );
			ASSERT( pSupValv );

			if( !pSupValv )
			{
				return;
			}

			GetpShutoffValve( eHMObj::eShutoffValveSupply )->SetBestIDPtr( pSupValv->GetValveIDPtr() );
			GetpShutoffValve( eHMObj::eShutoffValveSupply )->SetIDPtr( pSupValv->GetValveIDPtr() );
		}

		Modified();
	}
}
void CDS_HmHubStation::SetReturnValveID( CString ReturnValveID )
{
	//if (((CString)m_ReturnValveIDPtr.ID)!=ReturnValveID)
	{
		m_ReturnValveIDPtr = _NULL_IDPTR;
		_tcsncpy_s( m_ReturnValveIDPtr.ID, SIZEOFINTCHAR( m_ReturnValveIDPtr.ID ), ( LPCTSTR )ReturnValveID, SIZEOFINTCHAR( m_ReturnValveIDPtr.ID ) - 1 );
		Modified();

		if( ReturnValveID.IsEmpty() )
		{
			return;
		}

		CDB_HubStaValv *pRetValv = dynamic_cast<CDB_HubStaValv *>( GetReturnValveIDPtr().MP );
		ASSERT( pRetValv );

		if( !pRetValv )
		{
			return;
		}

		if( GetpBv() && GetpBv()->GetIDPtr().MP != pRetValv->GetValveIDPtr().MP )
		{
			GetpBv()->SetIDPtr( pRetValv->GetValveIDPtr() );
		}

		// Find corresponding Supply Valve
		CTable *pTab = ( CTable * )TASApp.GetpTADB()->Get( _T("STASUPRET_TAB") ).MP;
		ASSERT( pTab );

		if( !pTab )
		{
			return;
		}

		CTable *pTabValve = ( CTable * )GetReturnValveIDPtr().PP;
		ASSERT( pTabValve );

		if( !pTabValve )
		{
			return;
		}

		IDPTR IDPtrPairs = _NULL_IDPTR;

		// Find matching pairs
		for( IDPtrPairs = pTab->GetFirst(); *IDPtrPairs.ID; IDPtrPairs = pTab->GetNext() )
		{
			CDB_HubSupRetLnk *pHubSR = ( CDB_HubSupRetLnk * )IDPtrPairs.MP;

			if( ( pHubSR->GetTAISReturn() == pRetValv->GetTAISValve() ) )
			{
				// Pairs found
				// Search supply valve with the same size
				for( IDPTR IDPtr = pTabValve->GetFirst(); *IDPtr.ID; IDPtr = pTabValve->GetNext() )
				{
					CDB_HubStaValv *pSupValv = dynamic_cast<CDB_HubStaValv *>( IDPtr.MP );

					if( !pSupValv )
					{
						continue;
					}

					if( pSupValv->GetSupplyOrReturn() != CDB_HubValv::eSupplyReturnValv::Supply )
					{
						continue;
					}

					if( /*(pSupValv->GetTAISPartner() == pHubSR->GetTAISSupply())&&*/
						( pSupValv->GetTAISSize() == pRetValv->GetTAISSize() ) )
					{
						// Found
						SetSupplyValveIDPtr( IDPtr );
						break;
					}
				}
			}
		}
	}
}
void CDS_HmHubStation::SetSupplyValveIDPtr( IDPTR SupplyValveIDPtr )
{
	m_SupplyValveIDPtr = SupplyValveIDPtr;

	if( true == IsShutoffValveExist( CDS_HydroMod::eHMObj::eShutoffValveSupply ) )
	{
		CDB_HubStaValv *pSupValv = dynamic_cast<CDB_HubStaValv *>( m_SupplyValveIDPtr.MP );

		if( !pSupValv )
		{
			return;
		}

		GetpShutoffValve( eHMObj::eShutoffValveSupply )->SetBestIDPtr( pSupValv->GetValveIDPtr() );
		GetpShutoffValve( eHMObj::eShutoffValveSupply )->SetIDPtr( pSupValv->GetValveIDPtr() );
	}
	else
	{
		ASSERT( 0 );
	}
}

void CDS_HmHubStation::SetReturnActuatorID( CString ReturnActuatorID )
{
	m_ReturnActuatorIDPtr = _NULL_IDPTR;

	if( ReturnActuatorID.IsEmpty() )
	{
		return;
	}

	_tcsncpy_s( m_ReturnActuatorIDPtr.ID, SIZEOFINTCHAR( m_ReturnActuatorIDPtr.ID ), ( LPCTSTR )ReturnActuatorID, SIZEOFINTCHAR( m_ReturnActuatorIDPtr.ID ) - 1 );
	Modified();
}

bool CDS_HmHubStation::SetBalTypeID( CString BalTypeID )
{
	ASSERT( !BalTypeID.IsEmpty() );

	if( !BalTypeID.IsEmpty() )
	{
		// ID already match
		if( !_tcscmp( m_BalTypeIDPtr.ID, ( LPCTSTR )BalTypeID ) )
		{
			return true;
		}

		ASSERT( BalTypeID.GetLength() <= _ID_LENGTH );

		if( BalTypeID.GetLength() <= _ID_LENGTH )
		{
			m_BalTypeIDPtr = TASApp.GetpTADB()->Get( BalTypeID );
			Modified();
			return true;
		}
	}

	return false;
}
/*virtual*/ void CDS_HmHubStation::ResizeShutOffValves()
{
	// No resizing needed, Shut off valve is only on the supply side and the resizing occurs
	// when the return valve is determined
}

IDPTR CDS_HmHubStation::FindHubStaValve( CDB_HubStaValv::eSupplyReturnValv SupRet, IDPTR SelValveIDPtr )
{
	if( SelValveIDPtr.MP )
	{
		CTable *pTab = ( CTable * )TASApp.GetpTADB()->Get( _T("HUBSTAVALV_TAB") ).MP;

		for( IDPTR idptr = pTab->GetFirst( CLASS( CDB_HubStaValv ) ); *idptr.ID; idptr = pTab->GetNext() )
		{
			CDB_HubStaValv *pHSV = dynamic_cast<CDB_HubStaValv *>( idptr.MP );

			if( !pHSV )
			{
				continue;
			}

			if( pHSV->GetSupplyOrReturn() != SupRet )
			{
				continue;
			}

			if( pHSV->GetValveIDPtr().MP == SelValveIDPtr.MP )
			{
				return idptr;
			}
		}
	}

	return _NULL_IDPTR;
}
CString	CDS_HmHubStation::GetCvTypeString( eCVTAKV CvTaKv, bool bStrShort /*= true*/ )
{
	CString str;

	switch( CvTaKv )
	{
		case eNone:
			str = TASApp.LoadLocalizedString( IDS_SSHEETTAHUB_CVTYPENONE );
			break;

		case eKv:
		{
			CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
			ASSERT( pUnitDB );

			if( !pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				if( bStrShort )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETTAHUB_CVTYPEKV );
				}
				else
				{
					str = TASApp.LoadLocalizedString( IDS_DIALOGHUBCVCB_CVTYPEKV );
				}
			}
			else
			{
				if( bStrShort )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETTAHUB_CVTYPECV );
				}
				else
				{
					str = TASApp.LoadLocalizedString( IDS_DIALOGHUBCVCB_CVTYPECV );
				}
			}

			break;
		}

		case eTAValve:
			if( bStrShort )
			{
				str = TASApp.LoadLocalizedString( IDS_SSHEETTAHUB_CVTYPETAVALVE );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_DIALOGHUBCVCB_CVTYPETAVALVE );
			}

			break;
	}

	return str;
}
bool CDS_HmHubStation::SetControlValve( eCVTAKV CvTaKv, CDB_ControlProperties::CvCtrlType CvCtrlType,
										CDB_StringID *pConnect/*=NULL*/,
										CDB_StringID *pVersion/*=NULL*/,
										CDB_StringID *pPN/*=NULL*/ )
{
	if( GetBalTypeID() == _T("RVTYPE_BVC") )
	{
		return false;
	}

	if( GetpCV() )
	{
		DeleteCv();
	}

	m_CvTaKv = CvTaKv;

	if( m_CvTaKv == eCVTAKV::eNone )	// Remove CV
	{
		SetSchemeIDPtr( TASApp.GetpTADB()->Get( _T("DC_BV") ) );
		return true;
	}

	SetSchemeIDPtr( TASApp.GetpTADB()->Get( _T("2WD_BV1") ) );
	
	if( NULL != GetpSch() )
	{
		CreateCv( GetpSch()->GetCvLoc() );
	}
	
	GetpCV()->SetTACv( CvTaKv == eCVTAKV::eTAValve );
	GetpCV()->SetCtrlType( CvCtrlType );

	if( CvTaKv == eCVTAKV::eTAValve )
	{
		GetpCV()->SetpUserChoiceCVConn( pConnect );
		GetpCV()->SetpUserChoiceCVVers( pVersion );
		GetpCV()->SetpUserChoiceCVPN( pPN );
	}

	return true;
}

// return WaterChar used by parent
CWaterChar *CDS_HmHubStation::GetpWC()
{
	CDS_HmHub *pHmHub = (CDS_HmHub *)GetIDPtr().PP;
	ASSERT( pHmHub );
	return ( pHmHub->GetpWC() );
}

/*virtual*/ IDPTR CDS_HmHubStation::GetTADBValveIDPtr()
{
	CDB_HubStaValv *pValv = dynamic_cast<CDB_HubStaValv *>( GetReturnValveIDPtr().MP );
	IDPTR idptr = _NULL_IDPTR;

	if( pValv )
	{
		idptr = pValv->GetValveIDPtr();
	}

	return idptr;
}
