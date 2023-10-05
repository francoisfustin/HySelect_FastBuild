#include "stdafx.h"


#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include "MainFrm.h"
#include "TASelect.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "ImportExportHMNodeDefinitions.h"
#include "ImportHMBase.h"

CImportHMBase::CImportHMBase()
{ 
	m_rMainNode.nodeJson = Json::Value::null;
	// m_rMainNode.XML = null;
}

CImportHMBase::~CImportHMBase()
{
}

bool CImportHMBase::Import( CString strFileName, CString &strErrorMsg )
{
	strErrorMsg = _T("");

	// Read hydraulic network.
	NodeDefinition subNode;

	if( false == GetSubNode( m_rMainNode, IMPORTEXPORTHM_NODE_HYDRAULIC_NETWORK, subNode, strErrorMsg ) )
	{
		CString str;
		FormatString( str, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_ROOTNODEINVALID, IMPORTEXPORTHM_NODE_HYDRAULIC_NETWORK );
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE, str );
		return false;
	}

	if( false == _ReadHydraulicNetwork( subNode, m_clImportExportHMData.m_clHydraulicNetwork, strErrorMsg ) )
	{
		return false;
	}

	return true;
}

bool CImportHMBase::ReadFileInBuffer( CString strFileName, CString &strErrorMsg, ULONGLONG &ullBufferSize )
{
	strErrorMsg = _T("");

	// Try first to open in ASCII mode.
	std::ifstream inputFile;

	inputFile.open( strFileName, std::ifstream::binary );

	if( inputFile.fail() )
	{
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_CANTOPENFILE, strFileName );
		return false;
	}

	ullBufferSize = 0;
	inputFile.seekg( 0, inputFile.end );
    int iFileLength = (int)inputFile.tellg();
    inputFile.seekg( 0, inputFile.beg );
	
	std::stringstream streamBuffer;
	streamBuffer << inputFile.rdbuf();

	if( true == inputFile.fail() || true == inputFile.bad() )
	{
		// Can't read or file is too small!
		// FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_CANTOPENFILE, strFileName );
		inputFile.close();
		return false;
	}

	if( 0xFF == streamBuffer.get() && 0xFE == streamBuffer.get() )
	{
		// It's an UTF 16 file with BOM (Only little endian taking in charge).

		m_strFile.resize( ( iFileLength - 2 ) >> 1 );
		streamBuffer.read( (char *)m_strFile.c_str(), (long long)iFileLength - 2 );
	}
	else
	{
		streamBuffer.seekg( 0, streamBuffer.beg );

		if( true == _FindRootNodeUTF16( streamBuffer, 64 ) )
		{
			// It's an UFT 16 file without BOM.

			streamBuffer.seekg( 0, streamBuffer.beg );
			m_strFile.resize( iFileLength >> 1 );
			streamBuffer.read( (char *)m_strFile.c_str(), iFileLength );
		}
		else if( 0xEF == streamBuffer.get() && 0xBB == streamBuffer.get() && 0xBF == streamBuffer.get() )
		{
			// It's an UTF-8 file with BOM (Only little endian taking in charge).

			std::wstring_convert<std::codecvt_utf8_utf16<TCHAR>, TCHAR> converter;
			m_strFile = converter.from_bytes( streamBuffer.str() );
		}
		else
		{
			// Read all the file to check if we have and UTF-8 encoded character.
			streamBuffer.seekg( 0, streamBuffer.beg );

			bool bIsUTF8 = _CheckIfUTF8( streamBuffer, (int)streamBuffer.str().length() );

			if( true == bIsUTF8 )
			{
				// It's an UTF-8 file without BOM (Only little endian taking in charge).

				std::wstring_convert<std::codecvt_utf8_utf16<TCHAR>, TCHAR> converter;
				m_strFile = converter.from_bytes( streamBuffer.str() );
			}
			else
			{
				// It's either and UTF-8 file without special character or an ASCII file.

				m_strFile = std::wstring_convert<std::codecvt<TCHAR, char, mbstate_t>>().from_bytes( streamBuffer.str() );
			}
		}
	}

	return true;
}

bool CImportHMBase::_FindRootNodeUTF16( std::stringstream &cStream, int iSize )
{
	std::wstring wBuffer;
	wBuffer.resize( 64 );
	memcpy( (char *)wBuffer.c_str(), cStream.str().c_str(), 64 );
	return ( wstring::npos != wBuffer.find( IMPORTEXPORTHM_NODE_HYDRAULIC_NETWORK ) ) ? true : false;
}

bool CImportHMBase::_CheckIfUTF8( std::stringstream &cStream, int iSize )
{
	typedef std::vector<std::pair<UCHAR, UCHAR>> vecRange;
	vecRange octetDef;

	std::map<UCHAR, vecRange> mapRangeList;
	typedef std::map<UCHAR, vecRange>::reverse_iterator mapriter;

	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	mapRangeList.insert( std::pair<UCHAR, vecRange>( 0xC2, octetDef ) );

	octetDef.clear();
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0xA0, 0xBF ) );
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	mapRangeList.insert( std::pair<UCHAR, vecRange>( 0xE0, octetDef ) );

	octetDef.clear();
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	mapRangeList.insert( std::pair<UCHAR, vecRange>( 0xE1, octetDef ) );

	octetDef.clear();
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0x9F ) );
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	mapRangeList.insert( std::pair<UCHAR, vecRange>( 0xED, octetDef ) );

	octetDef.clear();
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	mapRangeList.insert( std::pair<UCHAR, vecRange>( 0xEE, octetDef ) );

	octetDef.clear();
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x90, 0xBF ) );
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	mapRangeList.insert( std::pair<UCHAR, vecRange>( 0xF0, octetDef ) );

	octetDef.clear();
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	mapRangeList.insert( std::pair<UCHAR, vecRange>( 0xF1, octetDef ) );

	octetDef.clear();
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0x8F ) );
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	octetDef.push_back( std::make_pair<UCHAR, UCHAR>( 0x80, 0xBF ) );
	mapRangeList.insert( std::pair<UCHAR, vecRange>( 0xF4, octetDef ) );

	int iOctetPos = -1;
	mapriter rCurrentMap = mapRangeList.rend();
	
	int iPos = 0;
	bool bAtLeastOneExtendedCharRead = false;
	bool bStop = false;

	// We can have either a file with ASCI non extended (1) or with ASCII extended (2) or full UTF-8 (3).
	//  1) we will never enter in the "if" condition. So 'bStop' and 'bAtLeastOneExtendedCharRead' are 'false'.
	//  2) Because some extended characters, we will enter sometimes in the 'if' condition. No valid UTF-8 character so we stop.
	//     Then 'bStop' is 'true' and 'bAtLeastOneExtendedCharRead' is 'true'.
	//  3) Full UTF-8 characters, 'bStop' is 'false' and 'bAtLeastOneExtendedCharRead' is 'true'.
	do
	{
		UCHAR ucChar = (UCHAR)cStream.get();

		if( ( -1 == iOctetPos && ( ucChar >= 0xC2 && ucChar <= 0xF4  ) )
				|| iOctetPos >= 0 )
		{
			bAtLeastOneExtendedCharRead = true;

			if( -1 == iOctetPos )
			{
				for( mapriter riter = mapRangeList.rbegin(); riter != mapRangeList.rend(); riter++ )
				{
					if( ucChar >= riter->first )
					{
						rCurrentMap = riter;
						iOctetPos = 0;
						break;
					}
				}
			}
			else
			{
				if( ucChar < rCurrentMap->second.at(iOctetPos).first || ucChar > rCurrentMap->second.at(iOctetPos).second )
				{
					bStop = true;
				}
				else if( ++iOctetPos == rCurrentMap->second.size() )
				{
					iOctetPos = -1;
				}
			}
		}

	}while( ++iPos < iSize && false == bStop );

	return ( false == bStop && true == bAtLeastOneExtendedCharRead );
}

bool CImportHMBase::_ReadHydraulicNetwork( NodeDefinition &nodeHydraulicNetwork, CImportExportHMData::CHydraulicNetworkData &clHydraulicNetwork, CString &strErrorMsg )
{
	// Read version.
	clHydraulicNetwork.m_dVersion = 0.0;

	if( false == ReadDoubleValue( nodeHydraulicNetwork, IMPORTEXPORTHM_NODE_HN_VERSION, strErrorMsg, clHydraulicNetwork.m_dVersion ) )
	{
		return false;
	}

	if( clHydraulicNetwork.m_dVersion < 1.000 || clHydraulicNetwork.m_dVersion > IMPORTEXPORTHM_VALUE_HN_VERSION )
	{
		// Error msg: The version (%1) is not valid. Must be between '1.000' and '%2'.
		CString strVersionInInport;
		strVersionInInport.Format( _T("%.3f"), clHydraulicNetwork.m_dVersion );
		CString strLastVersion;
		strLastVersion.Format( _T("%.3f"), IMPORTEXPORTHM_VALUE_HN_VERSION );
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDVERSION, strVersionInInport, strLastVersion );
		return false;
	}

	// Read direction.
	clHydraulicNetwork.m_strDirection = _T("");

	if( false == ReadStringValue( nodeHydraulicNetwork, IMPORTEXPORTHM_NODE_HN_DIRECTION, strErrorMsg, clHydraulicNetwork.m_strDirection ) )
	{
		return false;
	}

	if( 0 != clHydraulicNetwork.m_strDirection.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_DIRECTION ) )
	{
		// Error msg: The direction must be '%1'.
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDDIRECTION, IMPORTEXPORTHM_VALUE_HN_DIRECTION );
		return false;
	}

	// Read parameters.
	NodeDefinition subNodeParameters;

	if( false == GetSubNode( nodeHydraulicNetwork, IMPORTEXPORTHM_NODE_HN_PARAMETERS, subNodeParameters, strErrorMsg ) )
	{
		return false;
	}

	clHydraulicNetwork.m_clParameters.m_pclHydraulicNetworkData = &clHydraulicNetwork;

	if( false == _ReadParameters( subNodeParameters, clHydraulicNetwork.m_clParameters, strErrorMsg ) )
	{
		return false;
	}

	// Read hydraulic circuit list.
	NodeDefinition subNodeHydraulicCircuitList;

	if( false == GetSubNode( nodeHydraulicNetwork, IMPORTEXPORTHM_NODE_HN_HYDRAULICCIRCUITLIST, subNodeHydraulicCircuitList, strErrorMsg ) )
	{
		return false;
	}

	clHydraulicNetwork.m_clHydraulicCircuitList.m_pclHydraulicNetworkData = &clHydraulicNetwork;
	clHydraulicNetwork.m_clHydraulicCircuitList.m_pclHydraulicCircuitParent = NULL;

	if( false == _ReadHydraulicCircuitList( subNodeHydraulicCircuitList, &clHydraulicNetwork.m_clHydraulicCircuitList, strErrorMsg ) )
	{
		return false;
	}

	return true;
}

bool CImportHMBase::_ReadParameters( NodeDefinition &nodeParameters, CImportExportHMData::CParametersData &clParameters, CString &strErrorMsg )
{
	// Read heating/cooling mode.
	CString strMode = _T("");

	if( false == ReadStringValue( nodeParameters, IMPORTEXPORTHM_NODE_HN_P_MODE, strErrorMsg, strMode ) )
	{
		return false;
	}

	if( 0 != strMode.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_P_MODE_HEATING ) && 0 != strMode.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_P_MODE_COOLING ) )
	{
		// Error msg: The value for '%1' must be either '%2' or '%3'.
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEEITHEROR, IMPORTEXPORTHM_VALUE_HN_P_MODE_HEATING, IMPORTEXPORTHM_VALUE_HN_P_MODE_COOLING );
		return false;
	}

	if( 0 == strMode.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_P_MODE_HEATING ) )
	{
		clParameters.m_eMode = ProjectType::Heating;
	}
	else
	{
		clParameters.m_eMode = ProjectType::Cooling;
	}

	// Read water characteristic.
	NodeDefinition subNodeWaterChar;

	if( false == GetSubNode( nodeParameters, IMPORTEXPORTHM_NODE_HN_P_FLUIDCHARACTERISTIC, subNodeWaterChar, strErrorMsg ) )
	{
		return false;
	}

	if( false == _ReadFluidCharacteristic( subNodeWaterChar, clParameters.m_WC, clParameters.m_dReturnTemperature, strErrorMsg ) )
	{
		return false;
	}

	return true;
}

bool CImportHMBase::_ReadFluidCharacteristic( NodeDefinition &nodeFluidCharacteristic, CWaterChar &clWaterChar, double &dReturnTemperature, CString &strErrorMsg )
{
	// Read supply temperature (Mandatory).
	double dSupplyTemperature = 0.0;

	if( false == ReadDoubleValue( nodeFluidCharacteristic, IMPORTEXPORTHM_NODE_HN_P_FC_INSUPPLYTEMPERATURE, strErrorMsg, dSupplyTemperature ) )
	{
		return false;
	}

	clWaterChar.SetTemp( dSupplyTemperature );

	// Read return temperature (Mandatory).
	dReturnTemperature = 0.0;

	if( false == ReadDoubleValue( nodeFluidCharacteristic, IMPORTEXPORTHM_NODE_HN_P_FC_INRETURNTEMPERATURE, strErrorMsg, dReturnTemperature ) )
	{
		return false;
	}

	// Read additive family ID (Optional).
	CString strAdditiveFamilyID = _T("");

	if( false == ReadStringValue( nodeFluidCharacteristic, IMPORTEXPORTHM_NODE_HN_P_FC_INADDITIVEFAMILY, strErrorMsg, strAdditiveFamilyID ) )
	{
		// Not defined, we set the default value.
		clWaterChar.SetAdditFamID( _T("WATER_ADDIT" ) );
	}
	else
	{
		// Check if exist.
		if( NULL == TASApp.GetpTADB()->Get( strAdditiveFamilyID.MakeUpper() ).MP )
		{
			// Error msg: The additive family ID (%1) is not defined in the database.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_BADADDITTIVEFAMILYID, strAdditiveFamilyID.MakeUpper() );
			return false;
		}

		clWaterChar.SetAdditFamID( strAdditiveFamilyID.MakeUpper() );
	}

	// Read additive name ID (Optional).
	CString strAdditiveNameID = _T("");

	if( false == ReadStringValue( nodeFluidCharacteristic, IMPORTEXPORTHM_NODE_HN_P_FC_INADDITIVENAME, strErrorMsg, strAdditiveNameID ) )
	{
		// Not defined, we set the default value.
		clWaterChar.SetAdditID( _T("WATER" ) );
	}
	else
	{
		// Check if exist.
		if( NULL == TASApp.GetpTADB()->Get( strAdditiveNameID.MakeUpper() ).MP )
		{
			// Error msg: The additive name ID (%1) is not defined in the database.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_BADADDITTIVENAMEID, strAdditiveNameID.MakeUpper() );
			return false;
		}

		clWaterChar.SetAdditID( strAdditiveNameID.MakeUpper() );
	}

	// Read additive weight (Optional).
	double dAdditiveWeight = 0.0;
	ReadDoubleValue( nodeFluidCharacteristic, IMPORTEXPORTHM_NODE_HN_P_FC_INADDITIVEWEIGHT, strErrorMsg, dAdditiveWeight );
	clWaterChar.SetPcWeight( dAdditiveWeight );

	if( false == _VerifyTemperature( clWaterChar, dSupplyTemperature, IDS_IMPORTHMFROMJSONHELPER_SUPPLYTEMPERATURE, strErrorMsg ) )
	{
		return false;
	}
	else if( false == _VerifyTemperature( clWaterChar, dReturnTemperature, IDS_IMPORTHMFROMJSONHELPER_RETURNTEMPERATURE, strErrorMsg ) )
	{
		return false;
	}

	return true;
}

bool CImportHMBase::_VerifyTemperature( CWaterChar &clWaterChar, double dTemperature, int iPrefix, CString &strErrorMsg )
{
	double dSavedTemp = dTemperature;
	CWaterChar::eFluidRetCode eRetCode = clWaterChar.CheckFluidData( dTemperature );

	if( CWaterChar::eFluidRetCode::efrcFluidOk != eRetCode )
	{
		if( CWaterChar::efrcTemperatureTooLow == eRetCode )
		{
			CString strFreezePtTemp = WriteCUDouble( _U_TEMPERATURE, dTemperature, true, -1, 2 );
			CString strFreezingPointMsg;

			// "freezing point at %1"
			FormatString( strFreezingPointMsg, IDS_IMPORTHMFROMJSONHELPER_ERRORFREEZINGPOINT, strFreezePtTemp );
			
			// "%1 is too low (%2)."
			CString strPrefix = TASApp.LoadLocalizedString( iPrefix );
			CString strSecondStr = CString( WriteCUDouble( _U_TEMPERATURE, dSavedTemp, true, -1, 2 ) ) + _T(" <= ") + strFreezingPointMsg;
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_ERRORFLUIDDATATEMPTOOLOW, strPrefix, strSecondStr );
		}
		else if( CWaterChar::efrcTempTooHigh == eRetCode )
		{
			CString strTemp = CString( WriteCUDouble( _U_TEMPERATURE, dSavedTemp, true, -1, 2 ) )
							  + _T(" > ") + CString( WriteCUDouble( _U_TEMPERATURE, dTemperature, true, -1, 2 ) );

			// "%1 is too high (%2)."
			CString strPrefix = TASApp.LoadLocalizedString( iPrefix );
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_ERRORFLUIDDATATEMPTOOHIGH, strPrefix, strTemp );
		}
		else if( CWaterChar::efrcAdditiveTooHigh == eRetCode )
		{
			CString strTemp;
			strTemp.Format( _T("%f"), clWaterChar.GetPcWeight() );

			// "Additive in fluid (%1) is too high."
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_ERRORFLUIDDATATEMPADDTOOHIGH, strTemp );
		}
		
		return false;
	}

	return true;
}

bool CImportHMBase::_ReadHydraulicCircuitList( NodeDefinition &nodeHydraulicCircuitList, CImportExportHMData::CHydraulicCircuitListData *pclHydraulicCircuitList, CString &strErrorMsg )
{
	NodeDefinition nodeHydraulicCircuit;
	
	if( false == GetFirstSubNode( nodeHydraulicCircuitList, nodeHydraulicCircuit, strErrorMsg ) )
	{
		// Error msg: At least one hydraulic circuit must be defined.
		strErrorMsg = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_HYDRAULICCIRCUITMISSING );
		return false;
	}

	std::vector<int> vecPositions;
	CDS_HydroMod::ReturnType eReturnMode = CDS_HydroMod::ReturnType::LastReturnType;

	do
	{
		CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit = new CImportExportHMData::CHydraulicCircuitData();
		pclHydraulicCircuit->m_pclHydraulicCircuitParent = pclHydraulicCircuitList->m_pclHydraulicCircuitParent;
		pclHydraulicCircuit->m_pclHydraulicNetworkData = pclHydraulicCircuitList->m_pclHydraulicNetworkData;

		// Read the name (Optional).
		if( false == ReadStringValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INNAME, strErrorMsg, pclHydraulicCircuit->m_strInName ) )
		{
			// We put '*' and the name will be automatically updated after.
			pclHydraulicCircuit->m_strInName = _T("*");
		}
		else
		{
			// If first character is '*', the name will be automatically updated after.

			if( _T('*') != pclHydraulicCircuit->m_strInName.GetAt( 0 ) )
			{
				// If first character is not '*', we verify if the name doesn't exist yet.
				if( m_vecNames.end() != std::find( m_vecNames.begin(), m_vecNames.end(), pclHydraulicCircuit->m_strInName.MakeLower() ) )
				{
					// Error msg: It is impossible to have two hydraulic circuit with the same name.
					FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_HYDRAULICCIRCUITBADNAME, pclHydraulicCircuit->m_strInName );
					return false;
				}

				m_vecNames.push_back( pclHydraulicCircuit->m_strInName.MakeLower() );
			}
		}

		// Read the type (Mandatory).
		CString strInType = _T("");

		if( false == ReadStringValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INTYPE, strErrorMsg, strInType ) )
		{
			// Error msg: The node '%1' must be defined.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_INTYPE );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;
			return false;
		}
	
		if( 0 != strInType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_TYPE_MODULE ) && 0 != strInType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_TYPE_CIRCUIT ) )
		{
			// Error msg: The value for '%1' must be either '%2' or '%3'.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEEITHEROR, IMPORTEXPORTHM_NODE_HN_HCL_HC_INTYPE, IMPORTEXPORTHM_VALUE_HN_HCL_HC_TYPE_MODULE, IMPORTEXPORTHM_VALUE_HN_HCL_HC_TYPE_CIRCUIT );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;
			return false;
		}

		pclHydraulicCircuit->m_bInModule = ( 0 == strInType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_TYPE_MODULE ) ) ? true : false;

		// Read the circuit type ID (Mandatory).
		CString strCircuitType = _T("");

		if( false == ReadStringValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INCIRCUITTYPEID, strErrorMsg, strCircuitType ) )
		{
			// Error msg: The node '%1' must be defined.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_INCIRCUITTYPEID );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;
			return false;
		}

		IDPTR CircuitSchemeIDPtr = TASApp.GetpTADB()->Get( strCircuitType.MakeUpper() );

		// Verify that the circuit type ID exist in the database.
		if( NULL == CircuitSchemeIDPtr.MP )
		{
			// Error msg: The circuit type ID (%1) doesn't exist in the database.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_BADCIRCUITTYPEID, strCircuitType.MakeUpper() );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;
			return false;
		}

		pclHydraulicCircuit->m_pclCircuitScheme = (CDB_CircuitScheme *)( CircuitSchemeIDPtr.MP );

		// Read the position (Optional).
		if( false == ReadIntValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INPOSITION, strErrorMsg, pclHydraulicCircuit->m_iInPosition ) )
		{
			// Take the first one available.
			if( vecPositions.size() > 0 )
			{
				pclHydraulicCircuit->m_iInPosition = vecPositions.back() + 1; 
			}
			else
			{
				pclHydraulicCircuit->m_iInPosition = 0;
			}

			vecPositions.push_back( pclHydraulicCircuit->m_iInPosition );
		}
		else
		{
			// Verify that position doesn't exist yet.
			if( vecPositions.end() != std::find( vecPositions.begin(), vecPositions.end(), pclHydraulicCircuit->m_iInPosition ) )
			{
				// Error msg: Can't have two sibling hydraulic circuits with the same position.
				CString strPosition;
				strPosition.Format( _T("%i"), pclHydraulicCircuit->m_iInPosition );
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_BADPOSITION, strPosition );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;
				return false;
			}
			
			vecPositions.push_back( pclHydraulicCircuit->m_iInPosition );
		}

		// Read the return mode (Optional -> default value = direct return mode).
		CString strReturnMode = _T("");

		if( false == ReadStringValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INRETURNMODE, strErrorMsg, strReturnMode ) )
		{
			// If the value is not defined: if no one yet defined, we take the direct mode otherwise we take the one that has already defined.
			pclHydraulicCircuit->m_eInReturnMode = ( CDS_HydroMod::ReturnType::LastReturnType == eReturnMode ) ? CDS_HydroMod::ReturnType::Direct : eReturnMode;
		}
		else
		{
			// Verify.
			if( 0 != strReturnMode.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_RETURNMODE_DIRECT ) && 0 != strReturnMode.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_RETURNMODE_REVERSE ) )
			{
				// Error msg: The value for '%1' must be either '%2' or '%3'.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEEITHEROR, IMPORTEXPORTHM_NODE_HN_HCL_HC_INRETURNMODE, IMPORTEXPORTHM_VALUE_HN_HCL_HC_RETURNMODE_DIRECT, IMPORTEXPORTHM_VALUE_HN_HCL_HC_RETURNMODE_REVERSE );
				return false;
			}

			CDS_HydroMod::ReturnType eTemp = ( 0 == strReturnMode.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_RETURNMODE_DIRECT ) ) ? CDS_HydroMod::ReturnType::Direct : CDS_HydroMod::ReturnType::Reverse;

			// Check if we have already one return mode defined.
			if( CDS_HydroMod::ReturnType::LastReturnType == eReturnMode )
			{
				eReturnMode = eTemp;
			}
			else if( eReturnMode != eTemp )
			{
				// Error msg: Can't have two sibling hydraulic circuits with different return mode.
				strErrorMsg = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_BADRETURNMODE );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;
				return false;
			}

			pclHydraulicCircuit->m_eInReturnMode = eTemp;
		}

		// Read the design temperature at the inlet supply (Optional).
		if( false == ReadDoubleValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNINLETSUPPLY, strErrorMsg, pclHydraulicCircuit->m_dTemperatureDesignInletSupply ) )
		{
			pclHydraulicCircuit->m_dTemperatureDesignInletSupply = pclHydraulicCircuit->m_pclHydraulicNetworkData->m_clParameters.m_WC.GetTemp();
		}

		// Read the design temperature at the inlet return (Optional).
		if( false == ReadDoubleValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNINLETRETURN, strErrorMsg, pclHydraulicCircuit->m_dTemperatureDesignInletReturn ) )
		{
			pclHydraulicCircuit->m_dTemperatureDesignInletReturn = pclHydraulicCircuit->m_pclHydraulicNetworkData->m_clParameters.m_dReturnTemperature;
		}

		// Read the design temperature at the outlet supply (Optional).
		if( false == ReadDoubleValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNOUTLETSUPPLY, strErrorMsg, pclHydraulicCircuit->m_dTemperatureDesignOutletSupply ) )
		{
			pclHydraulicCircuit->m_dTemperatureDesignOutletSupply = pclHydraulicCircuit->m_pclHydraulicNetworkData->m_clParameters.m_WC.GetTemp();
		}

		// Read the design temperature at the outlet return (Optional).
		if( false == ReadDoubleValue( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNOUTLETRETURN, strErrorMsg, pclHydraulicCircuit->m_dTemperatureDesignOutletReturn ) )
		{
			pclHydraulicCircuit->m_dTemperatureDesignOutletReturn = pclHydraulicCircuit->m_pclHydraulicNetworkData->m_clParameters.m_dReturnTemperature;
		}

		// Read the terminal unit.
		if( false == _ReadTerminalUnitList( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Read pipe list (Optional for the root, mandatory for others).
		if( false == _ReadPipeList( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Read balancing valve list.
		if( false == _ReadBalancingValveList( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Read Dp controller valve list.
		if( false == _ReadDpControllerValveList( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Read control valve list.
		if( false == _ReadControlValveList( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Read shutoff valve list.
		if( false == _ReadShutoffValveList( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

		// Read pump.
		if( false == _ReadPump( nodeHydraulicCircuit, pclHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}
		
		pclHydraulicCircuitList->m_vecpHydraulicCircuitList.push_back( pclHydraulicCircuit );

		// Check if the current hydraulic circuit is a module.
		if( true == pclHydraulicCircuit->m_bInModule )
		{
			NodeDefinition nodeHydraulicCircuitChildrenList;

			if( true == GetSubNode( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HYDRAULICCIRCUITLIST, nodeHydraulicCircuitChildrenList, strErrorMsg ) )
			{
				pclHydraulicCircuit->m_pclHydraulicChildrenCircuitList = new CImportExportHMData::CHydraulicCircuitListData();
				pclHydraulicCircuit->m_pclHydraulicChildrenCircuitList->m_pclHydraulicCircuitParent = pclHydraulicCircuitList;
				pclHydraulicCircuit->m_pclHydraulicChildrenCircuitList->m_pclHydraulicNetworkData = pclHydraulicCircuitList->m_pclHydraulicNetworkData;

				if( false == _ReadHydraulicCircuitList( nodeHydraulicCircuitChildrenList, pclHydraulicCircuit->m_pclHydraulicChildrenCircuitList, strErrorMsg ) )
				{
					return false;
				}
			}
		}
		
		if( false == GetNextSubNode( nodeHydraulicCircuitList, nodeHydraulicCircuit, strErrorMsg ) )
		{
			return false;
		}

	}while( false == IsNodeEmpty( nodeHydraulicCircuit ) );

	return true;
}

bool CImportHMBase::_ReadTerminalUnitList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	NodeDefinition nodeTerminalUnitList;

	// If current hydraulic circuit is a circuit, we need the terminal unit definition.
	if( false == pclHydraulicCircuit->m_bInModule )
	{
		// Read terminal unit list.
		if( false == GetSubNode( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_TERMINALUNITLIST, nodeTerminalUnitList, strErrorMsg ) )
		{
			// Error msg: Terminal unit must be defined for a hydraulic circuit of type 'circuit'.
			strErrorMsg = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_TERMINALUNITMISSING );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;
			return false;
		}
	}
	else
	{
		// Module -> no terminal unit.
		return true;
	}

	NodeDefinition nodeTerminalUnit;
	
	if( false == GetFirstSubNode( nodeTerminalUnitList, nodeTerminalUnit, strErrorMsg ) )
	{
		// Error msg: Terminal unit must be defined for a hydraulic circuit of type 'circuit'.
		strErrorMsg = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_TERMINALUNITMISSING );

		// Current hydraulic circuit: %s.
		CString strTemp;
		FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
		strErrorMsg += _T("\r\n") + strTemp;

		return false;
	}

	do
	{
		CImportExportHMData::CTerminalUnitData *pclTerminalUnitData = new CImportExportHMData::CTerminalUnitData();

		// Read the external ID (Optional).
		ReadStringValue( nodeTerminalUnit, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_INEXTERNALID, strErrorMsg, pclTerminalUnitData->m_strInExternalID );

		// Read the flow (Mandatory).
		NodeDefinition nodeTerminalUnitFlow;

		if( false == GetSubNode( nodeTerminalUnit, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_INFLOW, nodeTerminalUnitFlow, strErrorMsg ) )
		{
			// Error msg: The node '%1' must be defined.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_INFLOW );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		CString strFlowType = _T("");

		if( false == ReadStringValue( nodeTerminalUnitFlow, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_TYPE, strErrorMsg, strFlowType ) )
		{
			// Error msg: The node '%1' must be defined.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_TYPE );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		if( 0 != strFlowType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IF_TYPE_FLOW ) && 0 != strFlowType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IF_TYPE_POWERDT ) )
		{
			// Error msg: The value for '%1' must be either '%2' or '%3'.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEEITHEROR, IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IF_TYPE_FLOW, IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IF_TYPE_POWERDT );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		if( 0 == strFlowType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IF_TYPE_FLOW ) )
		{
			// Read the flow (Mandatory).
			pclTerminalUnitData->m_eFlowType = CTermUnit::_QType::Q;

			if( false == ReadDoubleValue( nodeTerminalUnitFlow, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_FLOW, strErrorMsg, pclTerminalUnitData->m_dFlow ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_FLOW );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
		else
		{
			// Read the power/DT (Mandatory).
			pclTerminalUnitData->m_eFlowType = CTermUnit::_QType::PdT;

			if( false == ReadDoubleValue( nodeTerminalUnitFlow, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_POWER, strErrorMsg, pclTerminalUnitData->m_dPower ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_POWER );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}

			if( false == ReadDoubleValue( nodeTerminalUnitFlow, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_DT, strErrorMsg, pclTerminalUnitData->m_dDT ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_DT );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}

		// Read the pressure drop (Mandatory).
		NodeDefinition nodeTerminalUnitPressureDrop;

		if( false == GetSubNode( nodeTerminalUnit, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_INPRESSUREDROP, nodeTerminalUnitPressureDrop, strErrorMsg ) )
		{
			// Error msg: The node '%1' must be defined.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_INPRESSUREDROP );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		CString strPressureDropType = _T("");

		if( false == ReadStringValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_TYPE, strErrorMsg, strPressureDropType ) )
		{
			// Error msg: The node '%1' must be defined.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_TYPE );
			
			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		if( 0 != strPressureDropType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_PRESSUREDROP ) && 0 != strPressureDropType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_QREFDPREF )
				&& 0 != strPressureDropType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_KV ) && 0 != strPressureDropType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_CV ) )
		{
			// Error msg: The value for '%1' must be one of these values:
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEONEOFTHESE, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_TYPE );
			strErrorMsg += CString( _T(" '") ) + IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_PRESSUREDROP + CString( _T("', ") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_QREFDPREF + CString( _T("', '") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_KV + CString( _T("' or '") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_CV + CString( _T(".") );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		if( 0 == strPressureDropType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_PRESSUREDROP ) )
		{
			// Read the pressure drop (Mandatory).
			pclTerminalUnitData->m_ePressureDropType = CDS_HydroMod::eDpType::Dp;

			if( false == ReadDoubleValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_PRESSUREDROP, strErrorMsg, pclTerminalUnitData->m_dPressureDrop ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_PRESSUREDROP );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
		else if( 0 == strPressureDropType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_QREFDPREF ) )
		{
			// Read the Qref/Dpref (Mandatory).
			pclTerminalUnitData->m_ePressureDropType = CDS_HydroMod::eDpType::QDpref;

			if( false == ReadDoubleValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_FLOW_REF, strErrorMsg, pclTerminalUnitData->m_dFlowRef ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_FLOW_REF );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}

			if( false == ReadDoubleValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_PRESSURE_DROP_REF, strErrorMsg, pclTerminalUnitData->m_dPressureDropRef ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_PRESSURE_DROP_REF );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
		else if( 0 == strPressureDropType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_KV ) )
		{
			// Read the kv (Mandatory).
			pclTerminalUnitData->m_ePressureDropType = CDS_HydroMod::eDpType::Kv;

			if( false == ReadDoubleValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_KV, strErrorMsg, pclTerminalUnitData->m_dKv ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_KV );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
		else
		{
			// Read cv (Mandatory).
			pclTerminalUnitData->m_ePressureDropType = CDS_HydroMod::eDpType::Cv;

			if( false == ReadDoubleValue( nodeTerminalUnitPressureDrop, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_CV, strErrorMsg, pclTerminalUnitData->m_dCv ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_CV );
			
				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
	
		pclTerminalUnitData->m_pclHydraulicCircuitParent = pclHydraulicCircuit;
		pclHydraulicCircuit->m_vecpTerminalUnitList.push_back( pclTerminalUnitData );

		if( false == GetNextSubNode( nodeTerminalUnitList, nodeTerminalUnit, strErrorMsg ) )
		{
			return false;
		}

	}while( false == IsNodeEmpty( nodeTerminalUnit ) );

	return true;
}

bool CImportHMBase::_ReadPipeList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	NodeDefinition nodePipeList;

	if( false == GetSubNode( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_PIPELIST, nodePipeList, strErrorMsg ) )
	{
		if( NULL != pclHydraulicCircuit->m_pclHydraulicCircuitParent )
		{
			// Error msg: There is no pipe for the root module.
			strErrorMsg = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_PIPENOTFORROOT );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}
	}

	if( true == IsNodeEmpty( nodePipeList ) )
	{
		return true;
	}

	NodeDefinition nodePipe;
	
	if( false == GetFirstSubNode( nodePipeList, nodePipe, strErrorMsg ) )
	{
		// Error msg: At least one '%1' node must be defined under '%2'.
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_SUBMODULEMISSING, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_PIPE, IMPORTEXPORTHM_NODE_HN_HCL_HC_PIPELIST );

		// Current hydraulic circuit: %s.
		CString strTemp;
		FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
		strErrorMsg += _T("\r\n") + strTemp;

		return false;
	}

	do
	{
		CImportExportHMData::CPipeData *pclPipeData = new CImportExportHMData::CPipeData();

		// Read the external ID (Optional).
		ReadStringValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INEXTERNALID, strErrorMsg, pclPipeData->m_strInExternalID );

		// Read the pipe type (Mandatory).
		CString strPipeType = _T("");

		if( false == ReadStringValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INTYPE, strErrorMsg, strPipeType ) )
		{
			// Error msg: The node '%1' must be defined.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INTYPE );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		if( 0 != strPipeType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_DISTRIBUTIONSUPPLY ) && 0 != strPipeType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_DISTRIBUTIONRETURN )
				&& 0 != strPipeType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_CIRCUITPRIMARY ) && 0 != strPipeType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_CIRCUITSECONDARY ) )
		{
			// Error msg: The value for '%1' must be one of these values:
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEONEOFTHESE, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INTYPE );
			strErrorMsg += CString( _T(" '") ) + IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_DISTRIBUTIONSUPPLY + CString( _T("', ") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_DISTRIBUTIONRETURN + CString( _T("', '") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_CIRCUITPRIMARY + CString( _T("' or '") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_CIRCUITSECONDARY + CString( _T(".") );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		// Check if it's a valid pipe in regards to the current hydraulic circuit.
		if( NULL == pclHydraulicCircuit->m_pclHydraulicCircuitParent )
		{
			// Error msg: There is no pipe for the root module.
			strErrorMsg = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_PIPENOTFORROOT );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}
		else if( true == pclHydraulicCircuit->m_bInModule )
		{
			if( 0 == strPipeType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_CIRCUITPRIMARY ) || 0 == strPipeType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_CIRCUITSECONDARY ) )
			{
				// Error msg: There is no circuit pipe for a module.
				strErrorMsg = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_CIRCUITPIPENOTFORMODULE );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
		else if( 0 == strPipeType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_CIRCUITSECONDARY ) && false == pclHydraulicCircuit->m_pclCircuitScheme->GetpSchCateg()->IsInjection() )
		{
			// Error msg: There is no secondary circuit pipe for a this kind of circuit.
			strErrorMsg = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_SENCONDARYPIPENOTNEEDED );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		// Save the pipe type.
		if( 0 != strPipeType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_DISTRIBUTIONSUPPLY ) )
		{
			pclPipeData->m_eInType = CDS_HydroMod::eHMObj::eDistributionSupplyPipe;
		}
		else if( 0 != strPipeType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_DISTRIBUTIONRETURN ) )
		{
			pclPipeData->m_eInType = CDS_HydroMod::eHMObj::eDistributionReturnPipe;
		}
		else if( 0 != strPipeType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_CIRCUITPRIMARY ) )
		{
			pclPipeData->m_eInType = CDS_HydroMod::eHMObj::eCircuitPrimaryPipe;
		}
		else if( 0 != strPipeType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_CIRCUITSECONDARY ) )
		{
			pclPipeData->m_eInType = CDS_HydroMod::eHMObj::eCircuitSecondaryPipe;
		}


		// Read the pipe size ID (Optional).
		ReadStringValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INSIZEID, strErrorMsg, pclPipeData->m_strInSizeID );

		if( false == pclPipeData->m_strInSizeID.IsEmpty() && NULL == TASApp.GetpTADB()->Get( pclPipeData->m_strInSizeID ).MP )
		{
			// Error msg: The size ID (%1) is not defined in the database.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_BADSIZEID, pclPipeData->m_strInSizeID );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		// Read the internal diameter (Optional).
		ReadDoubleValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_ININTERNALDIAMETER, strErrorMsg, pclPipeData->m_dInInternalDiameter );

		// Read the roughness (Optional).
		ReadDoubleValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INROUGHNESS, strErrorMsg, pclPipeData->m_dInRoughness );

		// Read the length (Optional).
		ReadDoubleValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INLENGTH, strErrorMsg, pclPipeData->m_dInLength );

		// Read the lock (Optional).
		CString strIsLocked = _T("");
		
		if( true == ReadStringValue( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INLOCKED, strErrorMsg, strIsLocked ) )
		{
			if( 0 != strIsLocked.CompareNoCase( _T("no") ) && 0 != strIsLocked.CompareNoCase( _T("yes") ) )
			{
				// Error msg: The value for '%1' must be either '%2' or '%3'.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEEITHEROR, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INLOCKED, _T("no"), _T("yes") );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}

			if( 0 == strIsLocked.CompareNoCase( _T("yes") ) )
			{
				pclPipeData->m_bInIsLocked = true;
			}
		}

		// Read pipe in accessory list (Optional).
		NodeDefinition subNodeAccessoryList;

		if( true == GetSubNode( nodePipe, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_ACCESSORYLIST, subNodeAccessoryList, strErrorMsg ) )
		{
			NodeDefinition subNodeInAccessoryList;

			if( true == GetSubNode( subNodeAccessoryList, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_INACCESSORYLIST, subNodeInAccessoryList, strErrorMsg ) )
			{
				if( false == _ReadPipeAccessoryList( subNodeInAccessoryList, pclPipeData, strErrorMsg ) )
				{
					return false;
				}
			}
		}

		pclPipeData->m_pclHydraulicCircuitParent = pclHydraulicCircuit;
		pclHydraulicCircuit->m_vecpPipeList.push_back( pclPipeData );

		if( false == GetNextSubNode( nodePipeList, nodePipe, strErrorMsg ) )
		{
			return false;
		}

	}while( false == IsNodeEmpty( nodePipe ) );

	return true;
}

bool CImportHMBase::_ReadPipeAccessoryList( NodeDefinition &nodePipeAccessoryList, CImportExportHMData::CPipeData *pclPipeData, CString &strErrorMsg )
{
	NodeDefinition nodePipeAccessory;
	
	if( false == GetFirstSubNode( nodePipeAccessoryList, nodePipeAccessory, strErrorMsg ) )
	{
		// Error msg: At least one '%1' node must be defined under '%2'.
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_SUBMODULEMISSING, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_ACCESSORY, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_INACCESSORYLIST );

		// Current hydraulic circuit: %s.
		CString strTemp;
		FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclPipeData->m_pclHydraulicCircuitParent->m_strInName );
		strErrorMsg += _T("\r\n") + strTemp;

		return false;
	}

	do
	{
		CImportExportHMData::CPipeAccessoryData *pclPipeAccessoryData = new CImportExportHMData::CPipeAccessoryData();

		// Read the external ID (Optional).
		ReadStringValue( nodePipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_EXTERNALID, strErrorMsg, pclPipeAccessoryData->m_strInExternalID );

		// Read the pipe accessory type (Mandatory).
		CString strPipeAccessoryType = _T("");

		if( false == ReadStringValue( nodePipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE, strErrorMsg, strPipeAccessoryType ) )
		{
			// Error msg: The node '%1' must be defined.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclPipeData->m_pclHydraulicCircuitParent->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		if( 0 != strPipeAccessoryType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_ELBOW ) && 0 != strPipeAccessoryType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_TEE )
				&& 0 != strPipeAccessoryType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_ZETA ) && 0 != strPipeAccessoryType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_DP )
				&& 0 != strPipeAccessoryType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_QREFDPREF ) && 0 != strPipeAccessoryType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_KV )
				&& 0 != strPipeAccessoryType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_CV ) )
		{
			// Error msg: The value for '%1' must be one of these values:
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEONEOFTHESE, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE );
			strErrorMsg += CString( _T(" '") ) + IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_ELBOW + CString( _T("', ") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_TEE + CString( _T("', '") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_ZETA + CString( _T("', '") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_DP + CString( _T("', '") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_QREFDPREF + CString( _T("', '") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_KV + CString( _T("' or '") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_CV + CString( _T(".") );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclPipeData->m_pclHydraulicCircuitParent->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		pclPipeAccessoryData->m_bIsElbow = false;

		if( 0 == strPipeAccessoryType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_ELBOW ) )
		{
			pclPipeAccessoryData->m_eType = CDS_HydroMod::eDpType::None;
			pclPipeAccessoryData->m_bIsElbow = true;
		}
		else if( 0 == strPipeAccessoryType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_TEE ) )
		{
			pclPipeAccessoryData->m_eType = CDS_HydroMod::eDpType::None;
		}
		else if( 0 == strPipeAccessoryType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_ZETA ) )
		{
			pclPipeAccessoryData->m_eType = CDS_HydroMod::eDpType::dzeta;

			// Read the zeta value (Mandatory).
			if( false == ReadDoubleValue( nodePipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_ZETA, strErrorMsg, pclPipeAccessoryData->m_dZetaValue ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_ZETA );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclPipeData->m_pclHydraulicCircuitParent->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
		else if( 0 == strPipeAccessoryType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_DP ) )
		{
			pclPipeAccessoryData->m_eType = CDS_HydroMod::eDpType::Dp;

			// Read the pressure drop value (Mandatory).
			if( false == ReadDoubleValue( nodePipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_PRESSUREDROP, strErrorMsg, pclPipeAccessoryData->m_dPressureDrop ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_PRESSUREDROP );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclPipeData->m_pclHydraulicCircuitParent->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
		else if( 0 == strPipeAccessoryType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_QREFDPREF ) )
		{
			pclPipeAccessoryData->m_eType = CDS_HydroMod::eDpType::QDpref;

			// Read the flow ref value (Mandatory).
			if( false == ReadDoubleValue( nodePipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_FLOWREF, strErrorMsg, pclPipeAccessoryData->m_dFlowRef ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_FLOWREF );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclPipeData->m_pclHydraulicCircuitParent->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}

			// Read the pressure drop ref value (Mandatory).
			if( false == ReadDoubleValue( nodePipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_PRESSUREDROPREF, strErrorMsg, pclPipeAccessoryData->m_dPressureDropRef ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_PRESSUREDROPREF );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclPipeData->m_pclHydraulicCircuitParent->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
		else if( 0 == strPipeAccessoryType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_KV ) )
		{
			pclPipeAccessoryData->m_eType = CDS_HydroMod::eDpType::Kv;

			// Read the Kv value (Mandatory).
			if( false == ReadDoubleValue( nodePipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_KV, strErrorMsg, pclPipeAccessoryData->m_dKv ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_KV );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclPipeData->m_pclHydraulicCircuitParent->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
		else
		{
			pclPipeAccessoryData->m_eType = CDS_HydroMod::eDpType::Cv;

			// Read the Cv value (Mandatory).
			if( false == ReadDoubleValue( nodePipeAccessory, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_CV, strErrorMsg, pclPipeAccessoryData->m_dCv ) )
			{
				// Error msg: The node '%1' must be defined.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_CV );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclPipeData->m_pclHydraulicCircuitParent->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}

		pclPipeAccessoryData->m_pclPipeParent = pclPipeData;
		pclPipeData->m_vecpInPipeAccessoryList.push_back( pclPipeAccessoryData );

		if( false == GetNextSubNode( nodePipeAccessoryList, nodePipeAccessory, strErrorMsg ) )
		{
			return false;
		}

	}while( false == IsNodeEmpty( nodePipeAccessory ) );

	return true;
}

bool CImportHMBase::_ReadBalancingValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	eBool3 eBvMandatory = eBool3::eb3Undef;

	if( ( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::BV_P ) && false == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtOptional( CAnchorPt::eFunc::BV_P ) )
			|| ( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::BV_S ) && false == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtOptional( CAnchorPt::eFunc::BV_S ) )
			|| ( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::BV_Byp )	&& false == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtOptional( CAnchorPt::eFunc::BV_Byp ) ) )
	{
		eBvMandatory = eBool3::eb3True;
	}
	else if( ( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::BV_P ) && true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtOptional( CAnchorPt::eFunc::BV_P ) )
			|| ( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::BV_S ) && true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtOptional( CAnchorPt::eFunc::BV_S ) )
			|| ( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::BV_Byp )	&& true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtOptional( CAnchorPt::eFunc::BV_Byp ) ) )
	{
		eBvMandatory = eBool3::eb3False;
	}

	NodeDefinition nodeBalancingValveList;

	if( eBool3::eb3Undef != eBvMandatory )
	{
		if( false == GetSubNode( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_BALANCINGVALVELIST, nodeBalancingValveList, strErrorMsg ) )
		{
			if( eBool3::eb3True == eBvMandatory )
			{
				// Error msg: The hydraulic circuit type contains a '%1' thus the node '%2' must be defined.
				CString strBalancingValve = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_BALANCINGVALVE );
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_MISSINGPPRODUCTDEF, strBalancingValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_BALANCINGVALVELIST );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
	}
	else
	{
		// No balancing valve to read.
		return true;
	}

	NodeDefinition nodeBalancingValve;
	
	if( false == GetFirstSubNode( nodeBalancingValveList, nodeBalancingValve, strErrorMsg ) )
	{
		// Error msg: At least one '%1' node must be defined under '%2'.
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_SUBMODULEMISSING, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BALANCINGVALVE, IMPORTEXPORTHM_NODE_HN_HCL_HC_BALANCINGVALVELIST );

		// Current hydraulic circuit: %s.
		CString strTemp;
		FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
		strErrorMsg += _T("\r\n") + strTemp;

		return false;
	}

	CDB_CircuitScheme *pclCircuitScheme = pclHydraulicCircuit->m_pclCircuitScheme;

	do
	{
		CImportExportHMData::CBalancingValveData *pclBalancingValveData = new CImportExportHMData::CBalancingValveData();

		// Read the external ID (Optional).
		ReadStringValue( nodeBalancingValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INEXTERNALID, strErrorMsg, pclBalancingValveData->m_strInExternalID );

		// Read the location (Mandatory).
		CString strInLocation = _T("");

		if( false == ReadStringValue( nodeBalancingValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INLOCATION, strErrorMsg, strInLocation ) )
		{
			// Error msg: The node '%1' must be defined.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEMUSTBEDEFINED, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INLOCATION );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		if( 0 != strInLocation.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_PRIMARY ) && 0 != strInLocation.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_SECONDARY )
				&& 0 != strInLocation.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_BYPASS ) )
		{
			// Error msg: The value for '%1' must be one of these values:
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEONEOFTHESE, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INLOCATION );
			strErrorMsg += CString( _T(" '") ) + IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_PRIMARY + CString( _T("', ") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_SECONDARY + CString( _T("' or '") );
			strErrorMsg += IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_BYPASS + CString( _T(".") );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		// Verify now in regards to the current hydraulic circuit type ID.
		if( 0 == strInLocation.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_PRIMARY ) )
		{
			if( false == pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::BV_P ) )
			{
				// Error msg: There is no %1 located in the primary side for this hydraulic circuit type.
				CString strBalancingValve = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_BALANCINGVALVE );
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_PRODUCTFORBIDDENPRIMARYSIDE, strBalancingValve );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}

			pclBalancingValveData->m_eInLocation = CDS_HydroMod::eHMObj::eBVprim;
		}
		else if( 0 == strInLocation.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_SECONDARY ) )
		{
			if( false == pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::BV_S ) )
			{
				// Error msg: There is no %1 located in the seconcary side for this hydraulic circuit type.
				CString strBalancingValve = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_BALANCINGVALVE );
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_PRODUCTFORBIDDENSECONDARYSIDE, strBalancingValve );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}

			pclBalancingValveData->m_eInLocation = CDS_HydroMod::eHMObj::eBVsec;
		}
		else if( 0 == strInLocation.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_BYPASS ) )
		{
			if( false == pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::BV_Byp ) )
			{
				// Error msg: There is no %1 located in the by-pass for this hydraulic circuit type.
				CString strBalancingValve = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_BALANCINGVALVE );
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_PRODUCTFORBIDDENBYPASS, strBalancingValve );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}

			pclBalancingValveData->m_eInLocation = CDS_HydroMod::eHMObj::eBVbyp;
		}

		// Read the balancing valve size ID (Optional).
		ReadStringValue( nodeBalancingValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INSIZEID, strErrorMsg, pclBalancingValveData->m_strInSizeID );

		if( false == pclBalancingValveData->m_strInSizeID.IsEmpty() && NULL == TASApp.GetpTADB()->Get( pclBalancingValveData->m_strInSizeID ).MP )
		{
			// Error msg: The size ID (%1) is not defined in the database.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_BADSIZEID, pclBalancingValveData->m_strInSizeID );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		// Read the flow (Optional).
		ReadDoubleValue( nodeBalancingValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INFLOW, strErrorMsg, pclBalancingValveData->m_dFlow );

		// Read the lock (Optional).
		CString strIsLocked = _T("");

		if( true == ReadStringValue( nodeBalancingValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INLOCKED, strErrorMsg, strIsLocked ) )
		{
			if( 0 != strIsLocked.CompareNoCase( _T("no") ) && 0 != strIsLocked.CompareNoCase( _T("yes") ) )
			{
				// Error msg: The value for '%1' must be either '%2' or '%3'.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEEITHEROR, _T("no"), _T("yes") );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}

			if( 0 == strIsLocked.CompareNoCase( _T("yes") ) )
			{
				pclBalancingValveData->m_bInIsLocked = true;
			}
		}

		pclBalancingValveData->m_pclHydraulicCircuitParent = pclHydraulicCircuit;
		pclHydraulicCircuit->m_vecpBalancingValveList.push_back( pclBalancingValveData );

		if( false == GetNextSubNode( nodeBalancingValveList, nodeBalancingValve, strErrorMsg ) )
		{
			return false;
		}

	}while( false == IsNodeEmpty( nodeBalancingValve ) );

	return true;
}

bool CImportHMBase::_ReadDpControllerValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	eBool3 eDpCMandatory = eBool3::eb3Undef;

	if( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::DpC ) )
	{
		eDpCMandatory = eBool3::eb3True;
	}
	else if( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtOptional( CAnchorPt::eFunc::DpC ) )
	{
		eDpCMandatory = eBool3::eb3False;
	}
	
	NodeDefinition nodeDpControllerValveList;

	if( eBool3::eb3Undef != eDpCMandatory )
	{
		if( false == GetSubNode( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCONTROLLERVALVELIST, nodeDpControllerValveList, strErrorMsg ) )
		{
			if( eBool3::eb3True == eDpCMandatory )
			{
				// Error msg: The hydraulic circuit type contains a '%1' thus the node '%2' must be defined.
				CString strDpControllerValve = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_DPCONTROLLERVALVE );
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_MISSINGPPRODUCTDEF, strDpControllerValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCONTROLLERVALVELIST );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
	}
	else
	{
		// Nothing to read.
		return true;
	}
	
	NodeDefinition nodeDpControllerValve;
	
	if( false == GetFirstSubNode( nodeDpControllerValveList, nodeDpControllerValve, strErrorMsg ) )
	{
		// Error msg: At least one '%1' node must be defined under '%2'.
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_SUBMODULEMISSING, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCONTROLLERVALVE, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCONTROLLERVALVELIST );

		// Current hydraulic circuit: %s.
		CString strTemp;
		FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
		strErrorMsg += _T("\r\n") + strTemp;

		return false;
	}

	do
	{
		CImportExportHMData::CDpControllerValveData *pclDpControllerValveData = new CImportExportHMData::CDpControllerValveData();

		// Read the external ID (Optional).
		ReadStringValue( nodeDpControllerValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INEXTERNALID, strErrorMsg, pclDpControllerValveData->m_strInExternalID );

		// Read the Dp controller valve size ID (Optional).
		ReadStringValue( nodeDpControllerValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INSIZEID, strErrorMsg, pclDpControllerValveData->m_strInSizeID );

		if( false == pclDpControllerValveData->m_strInSizeID.IsEmpty() && NULL == TASApp.GetpTADB()->Get( pclDpControllerValveData->m_strInSizeID ).MP )
		{
			// Error msg: The size ID (%1) is not defined in the database.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_BADSIZEID, pclDpControllerValveData->m_strInSizeID );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		// Read the flow (Optional).
		ReadDoubleValue( nodeDpControllerValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INFLOW, strErrorMsg, pclDpControllerValveData->m_dFlow );

		// Read the lock (Optional).
		CString strIsLocked = _T("");

		if( true == ReadStringValue( nodeDpControllerValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INLOCKED, strErrorMsg, strIsLocked ) )
		{
			if( 0 != strIsLocked.CompareNoCase( _T("no") ) && 0 != strIsLocked.CompareNoCase( _T("yes") ) )
			{
				// Error msg: The value for '%1' must be either '%2' or '%3'.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEEITHEROR, _T("no"), _T("yes") );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}

			if( 0 == strIsLocked.CompareNoCase( _T("yes") ) )
			{
				pclDpControllerValveData->m_bInIsLocked = true;
			}
		}

		pclDpControllerValveData->m_pclHydraulicCircuitParent = pclHydraulicCircuit;
		pclHydraulicCircuit->m_vecpDpControllerValveList.push_back( pclDpControllerValveData );

		if( false == GetNextSubNode( nodeDpControllerValveList, nodeDpControllerValve, strErrorMsg ) )
		{
			return false;
		}

	}while( false == IsNodeEmpty( nodeDpControllerValve ) );

	return true;
}

bool CImportHMBase::_ReadControlValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	eBool3 eControlValveMandatory = eBool3::eb3Undef;

	if( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::ControlValve ) || true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::PICV ) 
			|| true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::DPCBCV ) )
	{
		eControlValveMandatory = eBool3::eb3True;
	}
	else if( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtOptional( CAnchorPt::eFunc::ControlValve ) || true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtOptional( CAnchorPt::eFunc::PICV ) 
			|| true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtOptional( CAnchorPt::eFunc::DPCBCV ) )
	{
		eControlValveMandatory = eBool3::eb3False;
	}

	NodeDefinition nodeControlValveList;

	if( eBool3::eb3Undef != eControlValveMandatory )
	{
		if( false == GetSubNode( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_CONTROLVALVELIST, nodeControlValveList, strErrorMsg ) )
		{
			if( eBool3::eb3True == eControlValveMandatory )
			{
				// Error msg: The hydraulic circuit type contains a '%1' thus the node '%2' must be defined.
				CString strControlValve = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_CONTROLVALVE );
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_MISSINGPPRODUCTDEF, strControlValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_CONTROLVALVELIST );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
	}
	else
	{
		// Nothing to read.
		return true;
	}

	NodeDefinition nodeControlValve;
	
	if( false == GetFirstSubNode( nodeControlValveList, nodeControlValve, strErrorMsg ) )
	{
		// Error msg: At least one '%1' node must be defined under '%2'.
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_SUBMODULEMISSING, IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CONTROLVALVE, IMPORTEXPORTHM_NODE_HN_HCL_HC_CONTROLVALVELIST );

		// Current hydraulic circuit: %s.
		CString strTemp;
		FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
		strErrorMsg += _T("\r\n") + strTemp;

		return false;
	}

	do
	{
		CImportExportHMData::CControlValveData *pclControlValveData = new CImportExportHMData::CControlValveData();

		// Read the external ID (Optional).
		ReadStringValue( nodeControlValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INEXTERNALID, strErrorMsg, pclControlValveData->m_strInExternalID );

		// Read the control type (Mandatory).
		CString strControlType = _T("");

		if( false == ReadStringValue( nodeControlValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INCONTROLTYPE, strErrorMsg, strControlType ) )
		{
			// Error msg: The node '%1' must be defined.
			FormatString( strErrorMsg, IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INCONTROLTYPE, IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INLOCATION );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		if( 0 != strControlType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_CVL_CV_INCONTROLTYPE_PROPORTIONAL ) && 0 != strControlType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_CVL_CV_INCONTROLTYPE_ONOFF ) )
		{
			// Error msg: The value for '%1' must be either '%2' or '%3'.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEEITHEROR, IMPORTEXPORTHM_VALUE_HN_HCL_HC_CVL_CV_INCONTROLTYPE_PROPORTIONAL, IMPORTEXPORTHM_VALUE_HN_HCL_HC_CVL_CV_INCONTROLTYPE_ONOFF );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		if( 0 == strControlType.CompareNoCase( IMPORTEXPORTHM_VALUE_HN_HCL_HC_CVL_CV_INCONTROLTYPE_PROPORTIONAL ) )
		{
			pclControlValveData->m_eControlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
		}
		else
		{
			pclControlValveData->m_eControlType = CDB_ControlProperties::CvCtrlType::eCvOnOff;
		}

		// Read the control valve size ID (Optional).
		ReadStringValue( nodeControlValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INSIZEID, strErrorMsg, pclControlValveData->m_strInSizeID );

		if( false == pclControlValveData->m_strInSizeID.IsEmpty() && NULL == TASApp.GetpTADB()->Get( pclControlValveData->m_strInSizeID ).MP )
		{
			// Error msg: The size ID (%1) is not defined in the database.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_BADSIZEID, pclControlValveData->m_strInSizeID );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		// Read the flow (Optional).
		ReadDoubleValue( nodeControlValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INFLOW, strErrorMsg, pclControlValveData->m_dFlow );

		// Read the lock (Optional).
		CString strIsLocked = _T("");

		if( true == ReadStringValue( nodeControlValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INLOCKED, strErrorMsg, strIsLocked ) )
		{
			if( 0 != strIsLocked.CompareNoCase( _T("no") ) && 0 != strIsLocked.CompareNoCase( _T("yes") ) )
			{
				// Error msg: The value for '%1' must be either '%2' or '%3'.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEEITHEROR, _T("no"), _T("yes") );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}

			if( 0 == strIsLocked.CompareNoCase( _T("yes") ) )
			{
				pclControlValveData->m_bInIsLocked = true;
			}
		}

		pclControlValveData->m_pclHydraulicCircuitParent = pclHydraulicCircuit;
		pclHydraulicCircuit->m_vecpControlValveList.push_back( pclControlValveData );

		if( false == GetNextSubNode( nodeControlValveList, nodeControlValve, strErrorMsg ) )
		{
			return false;
		}

	}while( false == IsNodeEmpty( nodeControlValve ) );

	return true;
}

bool CImportHMBase::_ReadShutoffValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	eBool3 eShutoffValveMandatory = eBool3::eb3Undef;

	if( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::ShutoffValve ) )
	{
		eShutoffValveMandatory = eBool3::eb3True;
	}
	else if( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtOptional( CAnchorPt::eFunc::ShutoffValve ) )
	{
		eShutoffValveMandatory = eBool3::eb3False;
	}

	NodeDefinition nodeShutoffValveList;

	if( eBool3::eb3Undef != eShutoffValveMandatory )
	{
		if( false == GetSubNode( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_SHUTOFFVALVELIST, nodeShutoffValveList, strErrorMsg ) )
		{
			if( eBool3::eb3True == eShutoffValveMandatory )
			{
				// Error msg: The hydraulic circuit type contains a '%1' thus the node '%2' must be defined.
				CString strShutoffValve = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_SHUTOFFVALVE );
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_MISSINGPPRODUCTDEF, strShutoffValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_SHUTOFFVALVELIST );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
	}
	else
	{
		// Nothing to read.
		return true;
	}

	NodeDefinition nodeShutoffValve;
	
	if( false == GetFirstSubNode( nodeShutoffValveList, nodeShutoffValve, strErrorMsg ) )
	{
		// Error msg: At least one '%1' node must be defined under '%2'.
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_SUBMODULEMISSING, IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SHUTOFFVALVE, IMPORTEXPORTHM_NODE_HN_HCL_HC_SHUTOFFVALVELIST );

		// Current hydraulic circuit: %s.
		CString strTemp;
		FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
		strErrorMsg += _T("\r\n") + strTemp;

		return false;
	}

	do
	{
		CImportExportHMData::CShutoffValveData *pclShutoffValveData = new CImportExportHMData::CShutoffValveData();

		// Read the external ID (Optional).
		ReadStringValue( nodeShutoffValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INEXTERNALID, strErrorMsg, pclShutoffValveData->m_strInExternalID );

		// Read the shutoff valve size ID (Optional).
		ReadStringValue( nodeShutoffValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INSIZEID, strErrorMsg, pclShutoffValveData->m_strInSizeID );

		if( false == pclShutoffValveData->m_strInSizeID.IsEmpty() && NULL == TASApp.GetpTADB()->Get( pclShutoffValveData->m_strInSizeID ).MP )
		{
			// Error msg: The size ID (%1) is not defined in the database.
			FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_BADSIZEID, pclShutoffValveData->m_strInSizeID );

			// Current hydraulic circuit: %s.
			CString strTemp;
			FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
			strErrorMsg += _T("\r\n") + strTemp;

			return false;
		}

		// Read the flow (Optional).
		ReadDoubleValue( nodeShutoffValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INFLOW, strErrorMsg, pclShutoffValveData->m_dFlow );

		// Read the lock (Optional).
		CString strIsLocked = _T("");

		if( true == ReadStringValue( nodeShutoffValve, IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INLOCKED, strErrorMsg, strIsLocked ) )
		{
			if( 0 != strIsLocked.CompareNoCase( _T("no") ) && 0 != strIsLocked.CompareNoCase( _T("yes") ) )
			{
				// Error msg: The value for '%1' must be either '%2' or '%3'.
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_VALUEMUSTBEEITHEROR, _T("no"), _T("yes") );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}

			if( 0 == strIsLocked.CompareNoCase( _T("yes") ) )
			{
				pclShutoffValveData->m_bInIsLocked = true;
			}
		}

		pclShutoffValveData->m_pclHydraulicCircuitParent = pclHydraulicCircuit;
		pclHydraulicCircuit->m_vecpShutoffValveList.push_back( pclShutoffValveData );

		if( false == GetNextSubNode( nodeShutoffValveList, nodeShutoffValve, strErrorMsg ) )
		{
			return false;
		}

	}while( false == IsNodeEmpty( nodeShutoffValve ) );

	return true;
}

bool CImportHMBase::_ReadPump( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg )
{
	eBool3 ePumpMandatory = eBool3::eb3Undef;

	if( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtExist( CAnchorPt::eFunc::PUMP ) )
	{
		ePumpMandatory = eBool3::eb3True;
	}
	else if( true == pclHydraulicCircuit->m_pclCircuitScheme->IsAnchorPtOptional( CAnchorPt::eFunc::PUMP ) )
	{
		ePumpMandatory = eBool3::eb3False;
	}

	NodeDefinition nodePump;

	if( eBool3::eb3Undef != ePumpMandatory )
	{
		if( false == GetSubNode( nodeHydraulicCircuit, IMPORTEXPORTHM_NODE_HN_HCL_HC_PUMP, nodePump, strErrorMsg ) )
		{
			if( eBool3::eb3True == ePumpMandatory )
			{
				// Error msg: The hydraulic circuit type contains a '%1' thus the node '%2' must be defined.
				CString strPump = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_PUMP );
				FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_MISSINGPPRODUCTDEF, strPump, IMPORTEXPORTHM_NODE_HN_HCL_HC_PUMP );

				// Current hydraulic circuit: %s.
				CString strTemp;
				FormatString( strTemp, IDS_IMPORTHMFROMJSONHELPER_CURRENTHYDRAULICCIRCUIT, pclHydraulicCircuit->m_strInName );
				strErrorMsg += _T("\r\n") + strTemp;

				return false;
			}
		}
	}
	else
	{
		// Nothing to read.
		return true;
	}

	pclHydraulicCircuit->m_Pump.m_bExist = true;

	// Read the external ID (Optional).
	ReadStringValue( nodePump, IMPORTEXPORTHM_NODE_HN_HCL_HC_P_INEXTERNALID, strErrorMsg, pclHydraulicCircuit->m_Pump.m_strInExternalID );

	// Read Pump hmin (Optional).
	ReadDoubleValue( nodePump, IMPORTEXPORTHM_NODE_HN_HCL_HC_P_HMIN, strErrorMsg, pclHydraulicCircuit->m_Pump.m_dHMin );

	return true;
}
