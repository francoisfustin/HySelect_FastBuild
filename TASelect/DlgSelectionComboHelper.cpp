#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "Global.h"
#include "Hydronic.h"
#include "ExtComboBox.h"
#include "ProductSelectionParameters.h"
#include "DlgSelectionComboHelper.h"


CDlgSelComboHelperBase::CDlgSelComboHelperBase( CProductSelelectionParameters *pclProductSelParams, SelectionProduct eSelectionProduct, SelectionType eSelectionType )
{
	m_pclProductSelParams = pclProductSelParams;
	m_eSelectionProduct = eSelectionProduct;
	m_eSelectionType = eSelectionType;
}

void CDlgSelComboHelperBase::PrepareComboContents()
{
	if( SP_PICV == m_eSelectionProduct || SP_DPCBCV == m_eSelectionProduct )
	{
		_PrepareComboContentsForControl();
	}
}

void CDlgSelComboHelperBase::FillComboType( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CString strTypeID )
{
	if( NULL == pCombo )
	{
		ASSERT_RETURN;
	}

	if( 0 == m_mapComboPtrToEComboType.count( pCombo ) )
	{
		m_mapComboPtrToEComboType[pCombo] = WC_Type;
	}

	if( 0 == m_mapComboPtrToEComboSize.count( pCombo ) )
	{
		m_mapComboPtrToEComboSize[pCombo] = eComboSize;
	}

	std::vector<int> vecIndex;
	PrepareVecList( pCombo, &vecIndex );

	int iSizeRange = 0;

	if( CS_All != eComboSize )
	{
		iSizeRange = (int)eComboSize - 1;
	}

	RESET_ONECOMBOCONTENT( m_armapCurrentStrID, iSizeRange, WC_Type );

	short nMap = 0;

	if( SelType_Individual == m_eSelectionType )
	{
		NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_Type, 0, IDT_All );
		nMap = 1;
	}

	short nToSelect = -1;

	CRankEx TypeList;

	for( std::vector<int>::iterator iterIndex = vecIndex.begin(); iterIndex != vecIndex.end(); iterIndex++ )
	{
		if( 0 == m_mapAllCombos[eComboSize].m_mapTypes.count( *iterIndex ) )
		{
			continue;
		}

		vecCStrID *pvecCStrID = &m_mapAllCombos[eComboSize].m_mapTypes[*iterIndex];

		for( vecCStrIDIter iterStrID = pvecCStrID->begin(); iterStrID != pvecCStrID->end(); iterStrID++ )
		{
			if( SelType_Individual == m_eSelectionType )
			{
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_Type, 0, *iterStrID );
			}

			LPARAM lParam = 0;
			IDPTR TypeIDPtr = ( *iterStrID ).m_IDPtr;
			std::wstring strItemText = ( ( CDB_StringID * )TypeIDPtr.MP )->GetString();

			if( false == TypeList.IfExist( strItemText, &lParam ) )
			{
				double dKey = ( double )_ttoi( ( ( CDB_StringID * )TypeIDPtr.MP )->GetIDstr() );

				NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_Type, nMap, IDT_Specific );
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_Type, nMap, *iterStrID );

				TypeList.Add( strItemText, dKey, ( LPARAM )nMap );

				if( false == strTypeID.IsEmpty() && 0 == CString( TypeIDPtr.ID ).Compare( strTypeID ) )
				{
					nToSelect = (int)nMap;
				}

				nMap++;
			}
			else
			{
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_Type, (short)lParam, *iterStrID );
			}
		}
	}

	FillCombo( pCombo, &TypeList, nToSelect );
	OnCbnSelChange( pCombo );
}

void CDlgSelComboHelperBase::FillComboCtrlType( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CDB_ControlProperties::CvCtrlType eCvCtrlType )
{
	if( NULL == pCombo )
	{
		ASSERT_RETURN;
	}

	if( 0 == m_mapComboPtrToEComboType.count( pCombo ) )
	{
		m_mapComboPtrToEComboType[pCombo] = WC_CtrlType;
	}

	if( 0 == m_mapComboPtrToEComboSize.count( pCombo ) )
	{
		m_mapComboPtrToEComboSize[pCombo] = eComboSize;
	}

	int iSizeRange = 0;

	if( CS_All != eComboSize )
	{
		iSizeRange = (int)eComboSize - 1;
	}

	std::vector<int> vecIndex;
	PrepareVecList( pCombo, &vecIndex );

	// Add each parameter eCvProportional, eCv3point, eCvOnOff because it is not place in the same way
	// in the enum. Do not change the enum because of TSP files compatibility.
	std::map< CDB_ControlProperties::CvCtrlType, UCHAR > mapSorted;
	mapSorted[ CDB_ControlProperties::eCvProportional ] = 0;
	mapSorted[ CDB_ControlProperties::eCv3point ] = 1;
	mapSorted[ CDB_ControlProperties::eCvOnOff ] = 2;

	RESET_ONECOMBOCONTENT( m_armapCurrentStrID, iSizeRange, WC_CtrlType );

	short nMap = 0;

	if( SelType_Individual == m_eSelectionType )
	{
		NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_CtrlType, 0, IDT_All );
		nMap = 1;
	}

	short nToSelect = -1;

	CRankEx CtrlTypeList;

	// Special case for batch selection. We have only one 'Control type' combo for both size limits (Below 65 & above 50).
	// We need this to run both sizes and mixed all the possibilities.
	std::vector<int> vecSizes;

	if( SelType_Individual == m_eSelectionType )
	{
		vecSizes.push_back( CS_All );
	}
	else
	{
		vecSizes.push_back( CS_Below65 );
		vecSizes.push_back( CS_Above50 );
	}

	for( std::vector<int>::iterator iterIndex = vecIndex.begin(); iterIndex != vecIndex.end(); iterIndex++ )
	{
		for( std::vector<int>::iterator iterSize = vecSizes.begin(); iterSize != vecSizes.end(); iterSize++ )
		{
			if( 0 == m_mapAllCombos[*iterSize].m_mapCtrlType.count( *iterIndex ) )
			{
				continue;
			}

			vecCStrID *pvecCStrID = &m_mapAllCombos[*iterSize].m_mapCtrlType[*iterIndex];

			for( vecCStrIDIter iterStrID = pvecCStrID->begin(); iterStrID != pvecCStrID->end(); iterStrID++ )
			{
				if( SelType_Individual == m_eSelectionType )
				{
					ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_CtrlType, 0, *iterStrID );
				}

				LPARAM lParam = 0;
				_string str = (*iterStrID).m_str;

				if( false == CtrlTypeList.IfExist( str, &lParam ) )
				{
					NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_CtrlType, nMap, IDT_Specific );
					ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_CtrlType, nMap, *iterStrID );

					if( ( int )CDB_ControlProperties::eCvProportional == ( *iterStrID ).m_int )
					{
						CtrlTypeList.Add( CDB_ControlProperties::GetCvCtrlTypeStr( CDB_ControlProperties::eCvProportional ).c_str(), mapSorted[CDB_ControlProperties::eCvProportional],
										  ( LPARAM )nMap );
					}
					else if( ( int )CDB_ControlProperties::eCv3point == ( *iterStrID ).m_int )
					{
						CtrlTypeList.Add( CDB_ControlProperties::GetCvCtrlTypeStr( CDB_ControlProperties::eCv3point ).c_str(), mapSorted[CDB_ControlProperties::eCv3point],
										  ( LPARAM )nMap );
					}
					else if( ( int )CDB_ControlProperties::eCvOnOff == ( *iterStrID ).m_int )
					{
						CtrlTypeList.Add( CDB_ControlProperties::GetCvCtrlTypeStr( CDB_ControlProperties::eCvOnOff ).c_str(), mapSorted[CDB_ControlProperties::eCvOnOff],
										  ( LPARAM )nMap );
					}

					if( (int)eCvCtrlType == ( *iterStrID ).m_int )
					{
						nToSelect = (int)nMap;
					}

					nMap++;
				}
				else
				{
					ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_CtrlType, (short)lParam, *iterStrID );
				}
			}
		}
	}

	CtrlTypeList.Transfer( pCombo );
	ASSERT( 0 != pCombo->GetCount() );

	int iSel = 0;

	for( int i = 0; i < pCombo->GetCount(); i++ )
	{
		if( nToSelect == (int)pCombo->GetItemData( i ) )
		{
			iSel = i;
			break;
		}
	}

	pCombo->SetCurSel( iSel );

	if( pCombo->GetCount() <= 1 )
	{
		pCombo->EnableWindow( false );
	}
	else
	{
		pCombo->EnableWindow( true );
	}

	OnCbnSelChange( pCombo );
}

void CDlgSelComboHelperBase::FillComboFamily( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CString strFamilyID )
{
	if( NULL == pCombo )
	{
		ASSERT_RETURN;
	}

	if( 0 == m_mapComboPtrToEComboType.count( pCombo ) )
	{
		m_mapComboPtrToEComboType[pCombo] = WC_Family;
	}

	if( 0 == m_mapComboPtrToEComboSize.count( pCombo ) )
	{
		m_mapComboPtrToEComboSize[pCombo] = eComboSize;
	}

	std::vector<int> vecIndex;
	PrepareVecList( pCombo, &vecIndex );

	int iSizeRange = 0;

	if( CS_All != eComboSize )
	{
		iSizeRange = (int)eComboSize - 1;
	}

	RESET_ONECOMBOCONTENT( m_armapCurrentStrID, iSizeRange, WC_Family );

	short nMap = 0;

	if( SelType_Individual == m_eSelectionType )
	{
		NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_Family, 0, IDT_All );
		nMap = 1;
	}

	short nToSelect = -1;

	CRankEx FamilyList;

	for( std::vector<int>::iterator iterIndex = vecIndex.begin(); iterIndex != vecIndex.end(); iterIndex++ )
	{
		if( 0 == m_mapAllCombos[eComboSize].m_mapFamily.count( *iterIndex ) )
		{
			continue;
		}

		vecCStrID *pvecCStrID = &m_mapAllCombos[eComboSize].m_mapFamily[*iterIndex];

		for( vecCStrIDIter iterStrID = pvecCStrID->begin(); iterStrID != pvecCStrID->end(); iterStrID++ )
		{
			if( SelType_Individual == m_eSelectionType )
			{
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_Family, 0, *iterStrID );
			}

			LPARAM lParam = 0;
			IDPTR FamilyIDPtr = ( *iterStrID ).m_IDPtr;
			std::wstring strItemText = ( ( CDB_StringID * )FamilyIDPtr.MP )->GetString();

			if( false == FamilyList.IfExist( strItemText, &lParam ) )
			{
				double dKey = ( double )_ttoi( ( ( CDB_StringID * )FamilyIDPtr.MP )->GetIDstr() );

				NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_Family, nMap, IDT_Specific );
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_Family, nMap, *iterStrID );

				FamilyList.Add( strItemText, dKey, ( LPARAM )nMap );

				if( false == strFamilyID.IsEmpty() && 0 == CString( FamilyIDPtr.ID ).Compare( strFamilyID ) )
				{
					nToSelect = (int)nMap;
				}

				nMap++;
			}
			else
			{
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_Family, (short)lParam, *iterStrID );
			}
		}
	}

	int iAllID = ( SelType_Individual == m_eSelectionType ) ? IDS_COMBOTEXT_ALL_FAMILIES : 0;
	FillCombo( pCombo, &FamilyList, nToSelect, iAllID );
	OnCbnSelChange( pCombo );
}

void CDlgSelComboHelperBase::FillComboBodyMaterial( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CString strBodyMatID )
{
	if( NULL == pCombo )
	{
		ASSERT_RETURN;
	}

	if( 0 == m_mapComboPtrToEComboType.count( pCombo ) )
	{
		m_mapComboPtrToEComboType[pCombo] = WC_BodyMaterial;
	}

	if( 0 == m_mapComboPtrToEComboSize.count( pCombo ) )
	{
		m_mapComboPtrToEComboSize[pCombo] = eComboSize;
	}

	int iSizeRange = 0;

	if( CS_All != eComboSize )
	{
		iSizeRange = (int)eComboSize - 1;
	}

	std::vector<int> vecIndex;
	PrepareVecList( pCombo, &vecIndex );

	RESET_ONECOMBOCONTENT( m_armapCurrentStrID, iSizeRange, WC_BodyMaterial );
	
	short nMap = 0;

	if( SelType_Individual == m_eSelectionType )
	{
		NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_BodyMaterial, 0, IDT_All );
		nMap = 1;
	}
	
	short nToSelect = -1;

	CRankEx BodyMatList;

	for( std::vector<int>::iterator iterIndex = vecIndex.begin(); iterIndex != vecIndex.end(); iterIndex++ )
	{
		if( 0 == m_mapAllCombos[eComboSize].m_mapBodyMat.count( *iterIndex ) )
		{
			continue;
		}

		vecCStrID *pvecCStrID = &m_mapAllCombos[eComboSize].m_mapBodyMat[*iterIndex];

		for( vecCStrIDIter iterStrID = pvecCStrID->begin(); iterStrID != pvecCStrID->end(); iterStrID++ )
		{
			if( SelType_Individual == m_eSelectionType )
			{
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_BodyMaterial, 0, *iterStrID );
			}

			LPARAM lParam = 0;
			IDPTR BodyMatIDPtr = ( *iterStrID ).m_IDPtr;
			std::wstring strItemText = ( ( CDB_StringID * )BodyMatIDPtr.MP )->GetString();

			if( false == BodyMatList.IfExist( strItemText, &lParam ) )
			{
				double dKey = ( double )_ttoi( ( ( CDB_StringID * )BodyMatIDPtr.MP )->GetIDstr() );

				NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_BodyMaterial, nMap, IDT_Specific );
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_BodyMaterial, nMap, *iterStrID );

				BodyMatList.Add( strItemText, dKey, ( LPARAM )nMap );

				if( false == strBodyMatID.IsEmpty() && 0 == CString( BodyMatIDPtr.ID ).Compare( strBodyMatID ) )
				{
					nToSelect = (int)nMap;
				}

				nMap++;

			}
			else
			{
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_BodyMaterial, (short)lParam, *iterStrID );
			}
		}
	}

	int iAllID = ( SelType_Individual == m_eSelectionType ) ? IDS_COMBOTEXT_ALL_MATERIALS : 0;
	FillCombo( pCombo, &BodyMatList, nToSelect, iAllID );
	OnCbnSelChange( pCombo );
}

void CDlgSelComboHelperBase::FillComboConnect( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CString strConnectID )
{
	if( NULL == pCombo )
	{
		ASSERT_RETURN;
	}

	if( 0 == m_mapComboPtrToEComboType.count( pCombo ) )
	{
		m_mapComboPtrToEComboType[pCombo] = WC_Connect;
	}

	if( 0 == m_mapComboPtrToEComboSize.count( pCombo ) )
	{
		m_mapComboPtrToEComboSize[pCombo] = eComboSize;
	}

 	std::vector<int> vecIndex;
 	PrepareVecList( pCombo, &vecIndex );

	int iSizeRange = 0;

	if( CS_All != eComboSize )
	{
		iSizeRange = (int)eComboSize - 1;
	}

	RESET_ONECOMBOCONTENT( m_armapCurrentStrID, iSizeRange, WC_Connect );
	
	short nMap = 0;

	if( SelType_Individual == m_eSelectionType )
	{
		NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_Connect, 0, IDT_All );
		nMap = 1;
	}
	
	short nToSelect = -1;

	CRankEx ConnectList;

	for( std::vector<int>::iterator iterIndex = vecIndex.begin(); iterIndex != vecIndex.end(); iterIndex++ )
	{
		if( 0 == m_mapAllCombos[eComboSize].m_mapConnect.count( *iterIndex ) )
		{
			continue;
		}

		vecCStrID *pvecCStrID = &m_mapAllCombos[eComboSize].m_mapConnect[*iterIndex];

		for( vecCStrIDIter iterStrID = pvecCStrID->begin(); iterStrID != pvecCStrID->end(); iterStrID++ )
		{
			if( SelType_Individual == m_eSelectionType )
			{
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_Connect, 0, *iterStrID );
			}

			LPARAM lParam = 0;
			IDPTR ConnectIDPtr = ( *iterStrID ).m_IDPtr;
			std::wstring strItemText = ( ( CDB_StringID * )ConnectIDPtr.MP )->GetString();

			if( false == ConnectList.IfExist( strItemText, &lParam ) )
			{
				double dKey = ( double )_ttoi( ( ( CDB_StringID * )ConnectIDPtr.MP )->GetIDstr() );

				NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_Connect, nMap, IDT_Specific );
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_Connect, nMap, *iterStrID );

				ConnectList.Add( strItemText, dKey, ( LPARAM )nMap );

				if( false == strConnectID.IsEmpty() && 0 == CString( ConnectIDPtr.ID ).Compare( strConnectID ) )
				{
					nToSelect = (int)nMap;
				}

				nMap++;
			}
			else
			{
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_Connect, (short)lParam, *iterStrID );
			}
		}
	}

	int iAllID = ( SelType_Individual == m_eSelectionType ) ? IDS_COMBOTEXT_ALL_CONNECTIONS : 0;
	FillCombo( pCombo, &ConnectList, nToSelect, iAllID );
	OnCbnSelChange( pCombo );
}

void CDlgSelComboHelperBase::FillComboVersion( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CString strVersionID )
{
	if( NULL == pCombo )
	{
		ASSERT_RETURN;
	}

	if( 0 == m_mapComboPtrToEComboType.count( pCombo ) )
	{
		m_mapComboPtrToEComboType[pCombo] = WC_Version;
	}

	if( 0 == m_mapComboPtrToEComboSize.count( pCombo ) )
	{
		m_mapComboPtrToEComboSize[pCombo] = eComboSize;
	}

	std::vector<int> vecIndex;
 	PrepareVecList( pCombo, &vecIndex );

	int iSizeRange = 0;

	if( CS_All != eComboSize )
	{
		iSizeRange = (int)eComboSize - 1;
	}

	RESET_ONECOMBOCONTENT( m_armapCurrentStrID, iSizeRange, WC_Version );
	
	short nMap = 0;

	if( SelType_Individual == m_eSelectionType )
	{
		NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_Version, 0, IDT_All );
		nMap = 1;
	}
	
	short nToSelect = -1;

	CRankEx VersionList;

	for( std::vector<int>::iterator iterIndex = vecIndex.begin(); iterIndex != vecIndex.end(); iterIndex++ )
	{
		if( 0 == m_mapAllCombos[eComboSize].m_mapVersion.count( *iterIndex ) )
		{
			continue;
		}

		vecCStrID *pvecCStrID = &m_mapAllCombos[eComboSize].m_mapVersion[*iterIndex];

		for( vecCStrIDIter iterStrID = pvecCStrID->begin(); iterStrID != pvecCStrID->end(); iterStrID++ )
		{
			if( SelType_Individual == m_eSelectionType )
			{
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_Version, 0, *iterStrID );
			}

			LPARAM lParam = 0;
			IDPTR VersionIDPtr = ( *iterStrID ).m_IDPtr;
			std::wstring strItemText = ( ( CDB_StringID * )VersionIDPtr.MP )->GetString();

			if( false == VersionList.IfExist( strItemText, &lParam ) )
			{
				double dKey = ( double )_ttoi( ( ( CDB_StringID * )VersionIDPtr.MP )->GetIDstr() );

				NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_Version, nMap, IDT_Specific );
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_Version, nMap, *iterStrID );

				VersionList.Add( strItemText, dKey, ( LPARAM )nMap );

				if( false == strVersionID.IsEmpty() && 0 == CString( VersionIDPtr.ID ).Compare( strVersionID ) )
				{
					nToSelect = (int)nMap;
				}

				nMap++;
			}
			else
			{
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_Version, (short)lParam, *iterStrID );
			}
		}
	}

	int iAllID = ( SelType_Individual == m_eSelectionType ) ? IDS_COMBOTEXT_ALL_VERSIONS : 0;
	FillCombo( pCombo, &VersionList, nToSelect, iAllID );
	OnCbnSelChange( pCombo );
}

void CDlgSelComboHelperBase::FillComboPN( CExtNumEditComboBox *pCombo, ComboSize eComboSize, CString strPNID )
{
	if( NULL == pCombo )
	{
		ASSERT_RETURN;
	}

	if( 0 == m_mapComboPtrToEComboType.count( pCombo ) )
	{
		m_mapComboPtrToEComboType[pCombo] = WC_PN;
	}

	if( 0 == m_mapComboPtrToEComboSize.count( pCombo ) )
	{
		m_mapComboPtrToEComboSize[pCombo] = eComboSize;
	}

	std::vector<int> vecIndex;
	PrepareVecList( pCombo, &vecIndex );

	int iSizeRange = 0;

	if( CS_All != eComboSize )
	{
		iSizeRange = (int)eComboSize - 1;
	}

	RESET_ONECOMBOCONTENT( m_armapCurrentStrID, iSizeRange, WC_PN );
	
	short nMap = 0;

	if( SelType_Individual == m_eSelectionType )
	{
		NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_PN, 0, IDT_All );
		nMap = 1;
	}
	
	short nToSelect = -1;

	CRankEx PNList;

	for( std::vector<int>::iterator iterIndex = vecIndex.begin(); iterIndex != vecIndex.end(); iterIndex++ )
	{
		if( 0 == m_mapAllCombos[eComboSize].m_mapPN.count( *iterIndex ) )
		{
			continue;
		}

		vecCStrID *pvecCStrID = &m_mapAllCombos[eComboSize].m_mapPN[*iterIndex];

		for( vecCStrIDIter iterStrID = pvecCStrID->begin(); iterStrID != pvecCStrID->end(); iterStrID++ )
		{
			if( SelType_Individual == m_eSelectionType )
			{
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_PN, 0, *iterStrID );
			}

			LPARAM lParam = 0;
			IDPTR PNIDPtr = ( *iterStrID ).m_IDPtr;
			std::wstring strItemText = ( ( CDB_StringID * )PNIDPtr.MP )->GetString();

			if( false == PNList.IfExist( strItemText, &lParam ) )
			{
				double dKey = ( double )_ttoi( ( ( CDB_StringID * )PNIDPtr.MP )->GetIDstr() );
				
				NEW_COMBO_ITEMDATA( m_armapCurrentStrID, iSizeRange, WC_PN, nMap, IDT_Specific );
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_PN, nMap, *iterStrID );

				PNList.Add( strItemText, dKey, ( LPARAM )nMap );

				if( false == strPNID.IsEmpty() && 0 == CString( PNIDPtr.ID ).Compare( strPNID ) )
				{
					nToSelect = (int)nMap;
				}

				nMap++;
			}
			else
			{
				ADD_COMBO_ITEMDATA_VECSTRID( m_armapCurrentStrID, iSizeRange, WC_PN, (short)lParam, *iterStrID );
			}
		}
	}

	int iAllID = ( SelType_Individual == m_eSelectionType ) ? IDS_COMBOTEXT_ALL : 0;
	FillCombo( pCombo, &PNList, nToSelect, iAllID );
	OnCbnSelChange( pCombo );
}

CString CDlgSelComboHelperBase::GetIDFromCombo( CExtNumEditComboBox *pclCombo, int iSelection )
{
	ItemData *prItemData = _GetItemData( pclCombo, iSelection );

	if( NULL == prItemData )
	{
		return _T("");
	}

	if( IDT_All == prItemData->m_iItemType )
	{
		return _T("");
	}

	if( 0 == (int)prItemData->m_vecCStrID.size() )
	{
		return _T("");
	}

	return prItemData->m_vecCStrID[0].m_IDPtr.ID;
}

IDPTR CDlgSelComboHelperBase::GetIDPtrFromCombo( CExtNumEditComboBox *pclCombo, int iSelection )
{
	ItemData *prItemData = _GetItemData( pclCombo, iSelection );

	if( IDT_All == prItemData->m_iItemType )
	{
		return _NULL_IDPTR;
	}

	if( 0 == (int)prItemData->m_vecCStrID.size() )
	{
		return _NULL_IDPTR;
	}

	return prItemData->m_vecCStrID[0].m_IDPtr;
}

int CDlgSelComboHelperBase::GetIntFromCombo( CExtNumEditComboBox *pclCombo, int iSelection )
{
	ItemData *prItemData = _GetItemData( pclCombo, iSelection );

	if( NULL == prItemData )
	{
		return -1;
	}

	if( IDT_All == prItemData->m_iItemType )
	{
		return -1;
	}

	if( 0 == (int)prItemData->m_vecCStrID.size() )
	{
		return -1;
	}

	return prItemData->m_vecCStrID[0].m_int;
}

int CDlgSelComboHelperBase::FindItemDataID( CExtNumEditComboBox *pclCombo, CString strValueID )
{
	if( NULL == pclCombo || NULL == m_mapComboPtrToEComboType.count( pclCombo ) || NULL == m_mapComboPtrToEComboSize.count( pclCombo ) )
	{
		return -1;
	}
	
	WhichCombo eWhichCombo = m_mapComboPtrToEComboType[pclCombo];
	int iSizeRange = 0;

	if( CS_All != m_mapComboPtrToEComboSize[pclCombo] )
	{
		iSizeRange = (int)m_mapComboPtrToEComboSize[pclCombo] - 1;
	}

	int iReturn = -1;

	for( int iLoopItem = 0; iLoopItem < pclCombo->GetCount(); iLoopItem++ )
	{
	    short nIndex = (short)pclCombo->GetItemData( pclCombo->GetCurSel() );

		if( 0 == m_armapCurrentStrID[iSizeRange][eWhichCombo].count( nIndex ) )
		{
			continue;
		}

		vecCStrID *pvecStrID = &m_armapCurrentStrID[iSizeRange][eWhichCombo][nIndex].m_vecCStrID;

		if( 0 == (int)pvecStrID->size() )
		{
			continue;
		}

		if( 0 == CString( pvecStrID->at( 0 ).m_IDPtr.ID ).Compare( strValueID ) )
		{
			iReturn = iLoopItem;
			break;
		}
	}

	return iReturn;
}

int CDlgSelComboHelperBase::FindItemDataInt( CExtNumEditComboBox *pclCombo, int iValue )
{
	if( NULL == pclCombo || NULL == m_mapComboPtrToEComboType.count( pclCombo ) || NULL == m_mapComboPtrToEComboSize.count( pclCombo ) )
	{
		return -1;
	}
	
	WhichCombo eWhichCombo = m_mapComboPtrToEComboType[pclCombo];
	int iTemp = 0;

	if( CS_All != m_mapComboPtrToEComboSize[pclCombo] )
	{
		iTemp = (int)m_mapComboPtrToEComboSize[pclCombo] - 1;
	}

	int iReturn = -1;

	for( int iLoopItem = 0; iLoopItem < pclCombo->GetCount(); iLoopItem++ )
	{
	    short nIndex = (short)pclCombo->GetItemData( pclCombo->GetCurSel() );

		if( 0 == m_armapCurrentStrID[iTemp][eWhichCombo].count( nIndex ) )
		{
			continue;
		}

		vecCStrID *pvecStrID = &m_armapCurrentStrID[iTemp][eWhichCombo][nIndex].m_vecCStrID;

		if( 0 == (int)pvecStrID->size() )
		{
			continue;
		}

		if( pvecStrID->at( 0 ).m_int == iValue )
		{
			iReturn = iLoopItem;
			break;
		}
	}

	return iReturn;
}

void CDlgSelComboHelperBase::OnCbnSelChange( CExtNumEditComboBox *pclCombo )
{
 	if( 0 == m_mapComboPtrToEComboType.count( pclCombo ) || 0 == m_mapComboPtrToEComboSize.count( pclCombo ) )
 	{
 		ASSERT_RETURN( 0 );
 	}
 
	WhichCombo eWhichCombo = m_mapComboPtrToEComboType[pclCombo];
    short nIndex = (short)pclCombo->GetItemData( pclCombo->GetCurSel() );

	int iTemp = 0;

	if( CS_All != m_mapComboPtrToEComboSize[pclCombo] )
	{
		iTemp = (int)m_mapComboPtrToEComboSize[pclCombo] - 1;
	}

	m_arnCurrentChoice[iTemp][eWhichCombo] = nIndex;
}

CDlgSelComboHelperBase::CStringID *CDlgSelComboHelperBase::IfStrExist( vecCStrID* pvecIDPtrID, CString str )
{
	if( NULL == pvecIDPtrID )
	{
		return NULL;
	}

	CStringID *pCStrID = NULL;
	
	for( vecCStrIDIter iter = pvecIDPtrID->begin(); iter != pvecIDPtrID->end() && NULL == pCStrID; iter++ )
	{
		if( (*iter).m_str == str )
		{
			pCStrID = &( *iter );
		}
	}

	return pCStrID;
}

void CDlgSelComboHelperBase::PrepareVecList( CExtNumEditComboBox *pCurrentCombo, vecInt *pvecToFill )
{
	if( NULL == pCurrentCombo || 0 == m_mapComboPtrToEComboType.count( pCurrentCombo ) || 0 == m_mapComboPtrToEComboSize.count( pCurrentCombo )
		|| NULL == pvecToFill )
	{
		return;
	}
	
	int iPrevComboType = _GetPrevComboType( m_mapComboPtrToEComboType[pCurrentCombo] );
	vecCStrID *pvecStrID = NULL;

	if( -1 != iPrevComboType )
	{
		int iTemp = 0;

		if( CS_All != m_mapComboPtrToEComboSize[pCurrentCombo] )
		{
			iTemp = (int)m_mapComboPtrToEComboSize[pCurrentCombo] - 1;
		}

		short nCurrentChoice = m_arnCurrentChoice[iTemp][iPrevComboType];

		if( 0 != m_armapCurrentStrID[iTemp][iPrevComboType].count( nCurrentChoice ) )
		{
			pvecStrID = &m_armapCurrentStrID[iTemp][iPrevComboType][nCurrentChoice].m_vecCStrID;
		}
	}

	pvecToFill->clear();

	if( NULL != pvecStrID && pvecStrID->size() > 0 )
	{
		for( vecCStrIDIter iter = pvecStrID->begin(); iter != pvecStrID->end(); iter++ )
		{
			pvecToFill->push_back( (*iter).m_nID );
		}
	}
	else
	{
		pvecToFill->push_back( 0 );
	}
}

void CDlgSelComboHelperBase::FillCombo( CExtNumEditComboBox *pCurrentCombo, CRankEx *pList, short nToSelect, int iAllIds )
{
	if( NULL == pCurrentCombo || NULL == pList )
	{
		return;
	}

	pCurrentCombo->ResetContent();

	// Add "** All..." string
	if( 0 != iAllIds && pList->GetCount() > 1 )
	{
		CString str = TASApp.LoadLocalizedString( iAllIds );
		int iItem = pCurrentCombo->AddString( str );
        pCurrentCombo->SetItemData( iItem, 0 );
	}
	
	int iSelPos = 0;
	std::wstring str;
	LPARAM lParam;

	for( bool bContinue = pList->GetFirst( str, lParam ); true == bContinue; bContinue = pList->GetNext( str, lParam ) )
	{
		int iItem = pCurrentCombo->AddString( str.c_str() );
        pCurrentCombo->SetItemData( iItem, lParam );

		if( -1 != nToSelect && nToSelect == (short)lParam )
		{
			iSelPos = iItem;
		}
	}

	pCurrentCombo->SetCurSel( iSelPos );

	if( pCurrentCombo->GetCount() <= 1 )
	{
		pCurrentCombo->EnableWindow( FALSE );
	}
	else
	{
		pCurrentCombo->EnableWindow( TRUE );
	}
}

void CDlgSelComboHelperBase::_PrepareComboContentsForControl()
{
	if( NULL == m_pclProductSelParams || NULL == m_pclProductSelParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CTableDN *pclTableDN = (CTableDN *)( m_pclProductSelParams->m_pTADB->Get( _T("DN_TAB") ).MP );
	
	if( NULL == pclTableDN )
	{
		ASSERT_RETURN;
	}
	
	int iDNAbove50 = pclTableDN->GetSize( _T("DN_65") );
	int iDNBelow65 = pclTableDN->GetSize( _T("DN_50") );

	CTADatabase::CvTargetTab eCvTarget;

	if( SP_PICV == m_eSelectionProduct )
	{
		eCvTarget = CTADatabase::eForPiCv;
	}
	else
	{
		eCvTarget = CTADatabase::eForDpCBCV;
	}

	CTADatabase::FilterSelection eFilterSelection = CTADatabase::FilterSelection::ForIndAndBatchSel;
	CDB_ControlProperties::CV2W3W Cv2W3W = CDB_ControlProperties::LastCV2W3W;
	CDB_ControlProperties::eCVFUNC CVFunc = CDB_ControlProperties::LastCVFUNC;

	// Create a vector with all tables needed.
	std::vector<_string> vTab;
	m_pclProductSelParams->m_pTADB->FillTargetTable( &vTab, eCvTarget );

	// Fill an array with all eCvCtrlType possibilities.
	bool bAllChecked = true;

	// Do the loop on the different tables.
	for( int i = CS_First; i < CS_Last; i++ )
	{
		m_mapAllCombos[i].m_mapPN.clear();
		m_mapAllCombos[i].m_mapVersion.clear();
		m_mapAllCombos[i].m_mapConnect.clear();
		m_mapAllCombos[i].m_mapBodyMat.clear();
		m_mapAllCombos[i].m_mapFamily.clear();
		m_mapAllCombos[i].m_mapCtrlType.clear();
		m_mapAllCombos[i].m_mapTypes.clear();
	}

	for( UINT i = 0; i < vTab.size(); i++ )
	{
		CTable *pTab = (CTable *)( m_pclProductSelParams->m_pTADB->Get( vTab[i].c_str() ).MP );
		
		if( NULL == pTab )
		{
			continue;
		}
	
		// Do the loop on the valves of the current table.
		for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			// Keep only CDB_RegulatingValve and children.
			CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve*>( IDPtr.MP );

			if( NULL == pclControlValve )
			{
				continue;
			}

			// Test if product is selectable.
			if( false == pclControlValve->IsSelectable( true ) )
			{
				continue;
			}

			if( true == m_pclProductSelParams->m_pTADB->MustExclude( pclControlValve, eFilterSelection ) )
			{
				continue;
			}

			// Test Cv2W3W only if we are below lastCV2W3W (CV2W, CV3W, CV4W or CV6W).
			if( Cv2W3W < CDB_ControlProperties::LastCV2W3W && pclControlValve->GetCtrlProp()->GetCv2w3w() != Cv2W3W )
			{
				continue;
			}
			
			// Test control valve function (NoControl, ControlOnly, Presettable or PresetPT).
			if( CVFunc < CDB_ControlProperties::LastCVFUNC && pclControlValve->GetCtrlProp()->GetCvFunc() != CVFunc ) 
			{
				continue;
			}
			
			// Test DN.
			int iDN = pclControlValve->GetSizeKey();
			int iInsertIn;
			
			if( SelType_Individual == m_eSelectionType )
			{
				iInsertIn = CS_All;
			}
			else if( iDN >= iDNAbove50 ) // must be >= DN65
			{
				iInsertIn = CS_Above50;
			}
			else
			{
				iInsertIn = CS_Below65;
			}

			CTable *pclActrTab = dynamic_cast<CTable*>( pclControlValve->GetActuatorGroupIDPtr().MP );
			
			if( NULL == pclActrTab )
			{
				continue;
			}

			// Run all actuators in this group.
			bool ar[CDB_ControlProperties::eLastCvCtrlType];
			bool fAllChecked = true;
			memset( ar, 0, sizeof( ar ) );

			/*
			for( IDPTR ActrIDPtr = pclActrTab->GetFirst(); _T('\0') != *ActrIDPtr.ID; ActrIDPtr = pclActrTab->GetNext() )
			{
				CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator*>( ActrIDPtr.MP );

				if( NULL == pclElectroActuator )
				{
					continue;
				}
				
				if( false == pclElectroActuator->IsSelectable() )
				{
					continue;
				}
				
				if( true == m_pTADB->MustExclude( pclElectroActuator, CTADatabase::FilterSelection::ForIndAndBatchSel ) )
				{
					continue;
				}

				if( true == pclElectroActuator->IsActuatorControlFit( CDB_ControlProperties::eCvProportional, false ) )
				{
					ar[CDB_ControlProperties::eCvProportional] = true;
				}

				if( true == pclElectroActuator->IsActuatorControlFit( CDB_ControlProperties::eCv3point, false ) )
				{
					ar[CDB_ControlProperties::eCv3point] = true;
				}

				if( true == pclElectroActuator->IsActuatorControlFit( CDB_ControlProperties::eCvOnOff, false ) )
				{
					ar[CDB_ControlProperties::eCvOnOff] = true;
				}

				fAllChecked = true;	
				for( int i = 0; i < CDB_ControlProperties::eLastCvCtrlType && true == fAllChecked; i++ )
				{
					if( false == ar[i] )
					{
						fAllChecked = false;
					}
				}

				if( true == fAllChecked )
				{
					break;
				}
			}
			*/

			// Valve with no characteristic, linear or equal % can all be controlled by a On/Off control type.
			_InsertContent( CDB_ControlProperties::eCvOnOff, pclControlValve, iInsertIn );

			if( CDB_ControlProperties::eCTRLCHAR::NotCharacterized != pclControlValve->GetCtrlProp()->GetCvCtrlChar() )
			{
				// Valve with linear and equal % can be also controlled by a 3-points or proportional control type but
				// not the valve that are not characterized.
				_InsertContent( CDB_ControlProperties::eCvProportional, pclControlValve, iInsertIn );
				_InsertContent( CDB_ControlProperties::eCv3point, pclControlValve, iInsertIn );
			}
		}
	}
}

void CDlgSelComboHelperBase::_InsertContent( CDB_ControlProperties::CvCtrlType CvCtrlType, CDB_RegulatingValve *pclValve, int iInsertIn )
{
	// Run all maps to see from which map we need to insert.
	bool bToInsert = false;
	short nCurrentIndex = 0;

	int iLoopMap = 0;
	int ariIndexList[WC_Last] = { 0, -1, -1, -1, -1, -1, -1 };

	if( 0 == wcscmp( L"TA_MODULATOR_65A", pclValve->GetIDPtr().ID ) )
	{
		int i = 0;
	}

	for( ; iLoopMap < WC_Last; iLoopMap++ )
	{
		int iCurrentIndex = ariIndexList[iLoopMap];

		switch( m_vecComboOrder[iLoopMap] )
		{
			case WC_Type:
			{
				CStringID *pStrID = NULL;

				if( 0 != (int)m_mapAllCombos[iInsertIn].m_mapTypes.size() && 
					0 != m_mapAllCombos[iInsertIn].m_mapTypes.count( iCurrentIndex ) )
				{
					pStrID = IfStrExist( &m_mapAllCombos[iInsertIn].m_mapTypes[iCurrentIndex], pclValve->GetType() );
				}

				if( NULL == pStrID )
				{
					bToInsert = true;
				}
				else
				{
					if( iLoopMap < ( WC_Last - 1 ) )
					{
						ariIndexList[iLoopMap + 1 ] = pStrID->m_nID;
					}
				}
			}
			break;

			case WC_CtrlType:
			{
				CStringID *pStrID = NULL;

				if( 0 != (int)m_mapAllCombos[iInsertIn].m_mapCtrlType.size() && 
					0 != m_mapAllCombos[iInsertIn].m_mapCtrlType.count( iCurrentIndex ) )
				{
					pStrID = IfStrExist( &m_mapAllCombos[iInsertIn].m_mapCtrlType[iCurrentIndex], CDB_ControlProperties::GetCvCtrlTypeStr( CvCtrlType ).c_str() );
				}

				if( NULL == pStrID )
				{
					bToInsert = true;
				}
				else
				{
					if( iLoopMap < ( WC_Last - 1 ) )
					{
						ariIndexList[iLoopMap + 1 ] = pStrID->m_nID;
					}
				}
			}
			break;

			case WC_Family:
			{
				CStringID *pStrID = NULL;

				if( 0 != (int)m_mapAllCombos[iInsertIn].m_mapFamily.size() && 
					0 != m_mapAllCombos[iInsertIn].m_mapFamily.count( iCurrentIndex ) )
				{
					pStrID = IfStrExist( &m_mapAllCombos[iInsertIn].m_mapFamily[iCurrentIndex], pclValve->GetFamily() );
				}

				if( NULL == pStrID )
				{
					bToInsert = true;
				}
				else
				{
					if( iLoopMap < ( WC_Last - 1 ) )
					{
						ariIndexList[iLoopMap + 1 ] = pStrID->m_nID;
					}
				}
			}
			break;

			case WC_BodyMaterial:
			{
				CStringID *pStrID = NULL;

				if( 0 != (int)m_mapAllCombos[iInsertIn].m_mapBodyMat.size() && 
					0 != m_mapAllCombos[iInsertIn].m_mapBodyMat.count( iCurrentIndex ) )
				{
					pStrID = IfStrExist( &m_mapAllCombos[iInsertIn].m_mapBodyMat[iCurrentIndex], pclValve->GetBodyMaterial() );
				}

				if( NULL == pStrID )
				{
					bToInsert = true;
				}
				else
				{
					if( iLoopMap < ( WC_Last - 1 ) )
					{
						// Allow to know which index to use to continue the list with the 'Connect' map.
						ariIndexList[iLoopMap + 1 ] = pStrID->m_nID;
					}
				}
			}
			break;

			case WC_Connect:
			{
				CStringID *pStrID = NULL;

				if( 0 != (int)m_mapAllCombos[iInsertIn].m_mapConnect.size() && 
					0 != m_mapAllCombos[iInsertIn].m_mapConnect.count( iCurrentIndex ) )
				{
					pStrID = IfStrExist( &m_mapAllCombos[iInsertIn].m_mapConnect[iCurrentIndex], pclValve->GetConnect() );
				}

				if( NULL == pStrID )
				{
					bToInsert = true;
				}
				else
				{
					if( iLoopMap < ( WC_Last - 1 ) )
					{
						ariIndexList[iLoopMap + 1 ] = pStrID->m_nID;
					}
				}
			}
			break;

			case WC_Version:
			{
				CStringID *pStrID = NULL;

				if( 0 != (int)m_mapAllCombos[iInsertIn].m_mapVersion.size() && 
					0 != m_mapAllCombos[iInsertIn].m_mapVersion.count( iCurrentIndex ) )
				{
					pStrID = IfStrExist( &m_mapAllCombos[iInsertIn].m_mapVersion[iCurrentIndex], pclValve->GetVersion() );
				}

				if( NULL == pStrID )
				{
					bToInsert = true;
				}
				else
				{
					if( iLoopMap < ( WC_Last - 1 ) )
					{
						ariIndexList[iLoopMap + 1 ] = pStrID->m_nID;
					}
				}
			}
			break;

			case WC_PN:
			{
				CRankEx rPNList;
				pclValve->GetPNList( &rPNList );

				LPARAM lpParam;
				_string str;

				if( rPNList.GetCount() != (int)m_mapAllCombos[iInsertIn].m_mapPN[iCurrentIndex].size() )
				{
					// Not need to compare if there is not the same PN numbers.
					bToInsert = true;
				}
				else
				{
					for( bool fContinue = rPNList.GetFirst( str, lpParam ); true == fContinue; fContinue = rPNList.GetNext( str, lpParam ) )
					{
						CDB_StringID *pDBStringID = dynamic_cast<CDB_StringID*>( (CData*)lpParam );

						if( NULL == pDBStringID )
						{
							continue;
						}

						CStringID *pStrID = NULL;

						if( 0 != (int)m_mapAllCombos[iInsertIn].m_mapPN.size() && 
							0 != m_mapAllCombos[iInsertIn].m_mapPN.count( iCurrentIndex ) )
						{
							pStrID = IfStrExist( &m_mapAllCombos[iInsertIn].m_mapPN[iCurrentIndex], pDBStringID->GetString() );
						}

						if( NULL == pStrID )
						{
							bToInsert = true;
							break;
						}
					}
				}
			}
			break;
		}

		if( true == bToInsert )
		{
			break;
		}

	}

	if( false == bToInsert )
	{
		return;
	}

	// Do insertion.
	CString str;
	for( int iLoopInsert = 6; iLoopInsert >= iLoopMap; iLoopInsert-- )
	{
		switch( m_vecComboOrder[iLoopInsert] )
		{
			case WC_PN:
			{
				if( -1 == ariIndexList[iLoopInsert] )
				{
					ariIndexList[iLoopInsert] = m_mapAllCombos[iInsertIn].m_mapPN.size();
				}

				CRankEx rPNList;
				pclValve->GetPNList( &rPNList );

				LPARAM lpParam;
				_string str;

				for( bool fContinue = rPNList.GetFirst( str, lpParam ); true == fContinue; fContinue = rPNList.GetNext( str, lpParam ) )
				{
					CDB_StringID *pDBStringID = dynamic_cast<CDB_StringID*>( (CData*)lpParam );

					if( NULL == pDBStringID )
					{
						continue;
					}

					if( NULL != IfStrExist( &m_mapAllCombos[iInsertIn].m_mapPN[ariIndexList[iLoopInsert]], pDBStringID->GetString() ) )
					{
						continue;
					}
				
					CStringID rStringID;
					rStringID.m_str = pDBStringID->GetString();
					rStringID.m_IDPtr = pDBStringID->GetIDPtr();
					rStringID.m_nID = -1;
					m_mapAllCombos[iInsertIn].m_mapPN[ariIndexList[iLoopInsert]].push_back( rStringID );
				}
			}
			break;

			case WC_Version:
			{
				if( -1 == ariIndexList[iLoopInsert] )
				{
					ariIndexList[iLoopInsert] = m_mapAllCombos[iInsertIn].m_mapVersion.size();
				}

				CStringID rStringID;
				rStringID.m_str = pclValve->GetVersion();
				rStringID.m_IDPtr = pclValve->GetVersionIDPtr();
				rStringID.m_nID = ariIndexList[iLoopInsert + 1];
				m_mapAllCombos[iInsertIn].m_mapVersion[ariIndexList[iLoopInsert]].push_back( rStringID );
			}
			break;

			case WC_Connect:
			{
				if( -1 == ariIndexList[iLoopInsert] )
				{
					ariIndexList[iLoopInsert] = m_mapAllCombos[iInsertIn].m_mapConnect.size();
				}

				CStringID rStringID;
				rStringID.m_str = pclValve->GetConnect();
				rStringID.m_IDPtr = pclValve->GetConnectIDPtr();
				rStringID.m_nID = ariIndexList[iLoopInsert + 1];
				m_mapAllCombos[iInsertIn].m_mapConnect[ariIndexList[iLoopInsert]].push_back( rStringID );
			}
			break;

			case WC_BodyMaterial:
			{
				if( -1 == ariIndexList[iLoopInsert] )
				{
					ariIndexList[iLoopInsert] = m_mapAllCombos[iInsertIn].m_mapBodyMat.size();
				}

				CStringID rStringID;
				rStringID.m_str = pclValve->GetBodyMaterial();
				rStringID.m_IDPtr = pclValve->GetBodyMaterialIDPtr();
				rStringID.m_nID = ariIndexList[iLoopInsert + 1];
				m_mapAllCombos[iInsertIn].m_mapBodyMat[ariIndexList[iLoopInsert]].push_back( rStringID );
			}
			break;

			case WC_Family:
			{
				if( -1 == ariIndexList[iLoopInsert] )
				{
					ariIndexList[iLoopInsert] = m_mapAllCombos[iInsertIn].m_mapFamily.size();
				}

				CStringID rStringID;
				rStringID.m_str = pclValve->GetFamily();
				rStringID.m_IDPtr = pclValve->GetFamilyIDPtr();
				rStringID.m_nID = ariIndexList[iLoopInsert + 1];
				m_mapAllCombos[iInsertIn].m_mapFamily[ariIndexList[iLoopInsert]].push_back( rStringID );
			}
			break;

			case WC_CtrlType:
			{
				if( -1 == ariIndexList[iLoopInsert] )
				{
					ariIndexList[iLoopInsert] = m_mapAllCombos[iInsertIn].m_mapCtrlType.size();
				}

				CStringID rStringID;
				rStringID.m_str = CDB_ControlProperties::GetCvCtrlTypeStr( CvCtrlType ).c_str();
				rStringID.m_IDPtr = _NULL_IDPTR;
				rStringID.m_int = (int)CvCtrlType;
				rStringID.m_nID = ariIndexList[iLoopInsert + 1];
				m_mapAllCombos[iInsertIn].m_mapCtrlType[ariIndexList[iLoopInsert]].push_back( rStringID );
			}
			break;

			case WC_Type:
			{
				if( -1 == ariIndexList[iLoopInsert] )
				{
					ariIndexList[iLoopInsert] = m_mapAllCombos[iInsertIn].m_mapTypes.size();
				}

				CStringID rStringID;
				rStringID.m_str = pclValve->GetType();
				rStringID.m_IDPtr = pclValve->GetTypeIDPtr();
				rStringID.m_nID = ariIndexList[iLoopInsert + 1];
				m_mapAllCombos[iInsertIn].m_mapTypes[ariIndexList[iLoopInsert]].push_back( rStringID );
			}
			break;
		}
	}
}

int CDlgSelComboHelperBase::_GetPrevComboType( WhichCombo eWhichCombo )
{
	int iReturn = -1;

	for( int i = 0; i < (int)m_vecComboOrder.size(); i++ )
	{
		if( m_vecComboOrder[i] == eWhichCombo && i > 0 )
		{
			iReturn = m_vecComboOrder[i - 1];
			break;
		}
	}

	return iReturn;
}

int CDlgSelComboHelperBase::_GetNextComboType( WhichCombo eWhichCombo )
{
	int iReturn = -1;

	for( int i = 0; i < (int)m_vecComboOrder.size(); i++ )
	{
		if( m_vecComboOrder[i] == eWhichCombo && ( i < ( (int)m_vecComboOrder.size() - 1 ) ) )
		{
			iReturn = m_vecComboOrder[i + 1];
			break;
		}
	}

	return iReturn;
}

CDlgSelComboHelperBase::ItemData *CDlgSelComboHelperBase::_GetItemData( CExtNumEditComboBox *pclCombo, int iSelection )
{
	if( NULL == pclCombo || 0 == m_mapComboPtrToEComboType.count( pclCombo) || 0 == m_mapComboPtrToEComboSize.count( pclCombo) )
	{
		return NULL;
	}

	if( -1 == iSelection )
	{
		iSelection = pclCombo->GetCurSel();
	}

	if( iSelection < 0 || iSelection >= pclCombo->GetCount() )
	{
		return NULL;
	}

	WhichCombo eWhichCombo = m_mapComboPtrToEComboType[pclCombo];
    short nIndex = (short)pclCombo->GetItemData( iSelection );

	int iTemp = 0;

	if( CS_All != m_mapComboPtrToEComboSize[pclCombo] )
	{
		iTemp = (int)m_mapComboPtrToEComboSize[pclCombo] - 1;
	}

	if( 0 == m_armapCurrentStrID[iTemp][eWhichCombo].count( nIndex ) )
	{
		return NULL;
	}

	return &m_armapCurrentStrID[iTemp][eWhichCombo][nIndex];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PIBCV
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDlgSelComboHelperPIBCV::CDlgSelComboHelperPIBCV( CProductSelelectionParameters *pclProductSelParams, SelectionType eSelectionType )
	: CDlgSelComboHelperBase( pclProductSelParams, SP_PICV, eSelectionType )
{
	if( SelType_Individual == m_eSelectionType )
	{
		m_vecComboOrder.push_back( WC_Type );
		m_vecComboOrder.push_back( WC_CtrlType );
		m_vecComboOrder.push_back( WC_Family );
		m_vecComboOrder.push_back( WC_BodyMaterial );
		m_vecComboOrder.push_back( WC_Connect );
		m_vecComboOrder.push_back( WC_Version );
		m_vecComboOrder.push_back( WC_PN );
	}
	else if( SelType_Batch == m_eSelectionType )
	{
		m_vecComboOrder.push_back( WC_CtrlType );
		m_vecComboOrder.push_back( WC_Type );
		m_vecComboOrder.push_back( WC_Family );
		m_vecComboOrder.push_back( WC_BodyMaterial );
		m_vecComboOrder.push_back( WC_Connect );
		m_vecComboOrder.push_back( WC_Version );
		m_vecComboOrder.push_back( WC_PN );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DPCBCV
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDlgSelComboHelperDpCBCV::CDlgSelComboHelperDpCBCV( CProductSelelectionParameters *pclProductSelParams, SelectionType eSelectionType )
	: CDlgSelComboHelperBase( pclProductSelParams, SP_DPCBCV, eSelectionType )
{
	if( SelType_Individual == m_eSelectionType )
	{
		m_vecComboOrder.push_back( WC_Type );
		m_vecComboOrder.push_back( WC_CtrlType );
		m_vecComboOrder.push_back( WC_Family );
		m_vecComboOrder.push_back( WC_BodyMaterial );
		m_vecComboOrder.push_back( WC_Connect );
		m_vecComboOrder.push_back( WC_Version );
		m_vecComboOrder.push_back( WC_PN );
	}
	else if( SelType_Batch == m_eSelectionType )
	{
		m_vecComboOrder.push_back( WC_CtrlType );
		m_vecComboOrder.push_back( WC_Type );
		m_vecComboOrder.push_back( WC_Family );
		m_vecComboOrder.push_back( WC_BodyMaterial );
		m_vecComboOrder.push_back( WC_Connect );
		m_vecComboOrder.push_back( WC_Version );
		m_vecComboOrder.push_back( WC_PN );
	}
}
