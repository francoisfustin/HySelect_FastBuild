#include "stdafx.h"


#include "Global.h"
#include "TASelect.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "ImportExportHMNodeDefinitions.h"
#include "ImportHMBase.h"
#include "ImportHMFromJson.h"

bool CImportHMFromJson::OpenFile( CString strFileName, CString &strErrorMsg )
{
	ULONGLONG ullFileLength = 0;

	if( false == ReadFileInBuffer( strFileName, strErrorMsg, ullFileLength ) )
	{
		return false;
	}

	strErrorMsg = _T("");

	// Force locale info to be in English - United States to have the same between all computers.
	LCID CurrentLocale = GetThreadLocale();

	// Prefer the 'SetThreadLocale' function instead of the '_tsetlocale' function. Because the first function will set the DEFAULT
	// regional settings corresponding to the LCID and not the one that the user can change in the Windows settings.
	SetThreadLocale( 0x0409 );

	Json::Value val;
	Json::Reader reader;

	reader.parse( m_strFile, val );

	// Restore regional settings.
	SetThreadLocale( CurrentLocale );

	// Must be only one node "hydraulic_network"!
	if( val.size() != 1 )
	{
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE, TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_ONEROOTNODE ) );
		return false;
	}

	m_rMainNode.nodeJson = val;
	return true;
}

bool CImportHMFromJson::GetSubNode( NodeDefinition &node, CString strNodeName, NodeDefinition &nodeSub, CString &strErrorMsg )
{
	nodeSub.nodeJson = Json::Value::null;

	if( false == node.nodeJson.isMember( strNodeName ) )
	{
		// Error: the 'strNodeName' is not a subnode of 'node'.
		// FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_NODENOTDEFINED, strNodeName );
		return false;
	}

	nodeSub.nodeJson = node.nodeJson[strNodeName];
	return true;
}

bool CImportHMFromJson::GetFirstSubNode( NodeDefinition &node, NodeDefinition &nodeSub, CString &strErrorMsg)
{
	if( false == node.nodeJson.isArray() )
	{
		// Error: the node "node" is not an array.?
		// FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_NODENOTDEFINED, strNodeName );
		return false;
	}

	node.jsonIter = node.nodeJson.begin();
	nodeSub.nodeJson = *node.jsonIter;

	return true;
}

bool CImportHMFromJson::GetNextSubNode( NodeDefinition &node, NodeDefinition &nodeSub, CString &strErrorMsg )
{
	if( true == (*node.jsonIter).isNull() )
	{
		// Error: 'GetFirstSubNode' must be called first.
		// FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_NODENOTDEFINED, strNodeName );
		return false;
	}

	if( false == node.nodeJson.isArray() )
	{
		// Error: the node "node" is not an array.?
		// FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_NODENOTDEFINED, strNodeName );
		return false;
	}

	Json::Value::iterator nulliter;
	
	if( ++node.jsonIter == node.nodeJson.end() )
	{
		// End of iteration.
		nodeSub.nodeJson = Json::Value::null;
		return true;
	}

	nodeSub.nodeJson = *node.jsonIter;
	return true;
}

bool CImportHMFromJson::IsNodeEmpty( NodeDefinition &node )
{
	return ( Json::Value::null == node.nodeJson ) ? true : false;
}

bool CImportHMFromJson::ReadIntValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, int &iValue )
{
	if( false == node.nodeJson.isMember( strNodeName ) )
	{
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_NODENOTDEFINED, strNodeName );
		return false;
	}

	if( false == node.nodeJson[strNodeName].isInt() )
	{
		CString strInt = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_NODETYPEINT );
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEVALUEINVALID, strNodeName, strInt );
		return false;
	}

	iValue = node.nodeJson[strNodeName].asInt();
	return true;
}

bool CImportHMFromJson::ReadDoubleValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, double &dValue )
{
	if( false == node.nodeJson.isMember( strNodeName ) )
	{
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_NODENOTDEFINED, strNodeName );
		return false;
	}

	if( false == node.nodeJson[strNodeName].isDouble() && false == node.nodeJson[strNodeName].isInt() )
	{
		CString strDouble = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_NODETYPEDOUBLE );
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEVALUEINVALID, strNodeName, strDouble );
		return false;
	}

	dValue = node.nodeJson[strNodeName].asDouble();
	return true;
}

bool CImportHMFromJson::ReadStringValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, CString &strValue )
{
	if( false == node.nodeJson.isMember( strNodeName ) )
	{
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_NODENOTDEFINED, strNodeName );
		return false;
	}

	if( false == node.nodeJson[strNodeName].isString() )
	{
		CString strString = TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_NODETYPESTRING );
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_NODEVALUEINVALID, strNodeName, strString );
		return false;
	}

	strValue = node.nodeJson[strNodeName].asString().c_str();
	return true;
}

/*
bool CImportHMFromJson::ReadHydraulicNetwork( NodeDefinition &nodeHydraulicNetwork, CImportExportHMNodeDefinitions::CNodeDefinition &clNodeDefinition, CString &strErrorMsg )
{
	Json::Value valHydraulicNetwork = nodeHydraulicNetwork.nodeJson;

	if( false == valHydraulicNetwork.isMember( clNodeDefinition.GetNodeName() ) )
	{
		FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE, TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_ROOTNODEINVALID ) );
		return false;
	}

	Json::Value &valHydraulicNetworkDefinition = valHydraulicNetwork[clNodeDefinition.GetNodeName()];
	
	CImportExportHMNodeDefinitions::CNodeDefinition *pclSubNodeDefinition = clNodeDefinition.GetFirstSubNodeDefinition();

	while( NULL != pclSubNodeDefinition )
	{
		if( pclSubNodeDefinition->GetNodeName().CompareNoCase( IMPORTEXPORTHM_NODE_HN_VERSION ) )
		{
			double dVersion = valHydraulicNetworkDefinition[IMPORTEXPORTHM_NODE_HN_VERSION].asDouble();

			if( dVersion < 1.000 || dVersion > 1.0002 )
			{
				// Error msg: not the good version.
				// FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE, TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_ONEROOTNODE ) );
				return false;
			}
		}
		else if( pclSubNodeDefinition->GetNodeName().CompareNoCase( IMPORTEXPORTHM_NODE_HN_DIRECTION ) )
		{
			if( 0 != CStringA( valHydraulicNetworkDefinition[IMPORTEXPORTHM_NODE_HN_DIRECTION].asString().c_str() ).CompareNoCase("import_to_hyselect") )
			{
				// Error msg: the direction must be "import_to_hyselect".
				// FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE, TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_ONEROOTNODE ) );
				return false;
			}
		}
		else if( pclSubNodeDefinition->GetNodeName().CompareNoCase( IMPORTEXPORTHM_NODE_HN_PARAMETERS ) )
		{
			if( false == _ReadParameters( valHydraulicNetwork[IMPORTEXPORTHM_NODE_HN_PARAMETERS], *pclSubNodeDefinition, strErrorMsg ) )
			{
				return false;
			}
		}
		else if( pclSubNodeDefinition->GetNodeName().CompareNoCase( IMPORTEXPORTHM_NODE_HN_HYDRAULICCIRCUITLIST ) )
		{
			if( false == _ReadHydraulicCircuitList( valHydraulicNetwork[IMPORTEXPORTHM_NODE_HN_HYDRAULICCIRCUITLIST], *pclSubNodeDefinition, strErrorMsg ) )
			{
				return false;
			}
		}
		else
		{
			// Error msg: node name not recognized.
			// FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE, TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_ONEROOTNODE ) );
			return false;
		}

		pclSubNodeDefinition = clNodeDefinition.GetNextSubNodeDefinition();
	}

	return true;
}

bool CImportHMFromJson::_ReadParameters( Json::Value &valParameters, CImportExportHMNodeDefinitions::CNodeDefinition &clNodeDefinition, CString &strErrorMsg )
{
	CImportExportHMNodeDefinitions::CNodeDefinition *pclSubNodeDefinition = clNodeDefinition.GetFirstSubNodeDefinition();

	while( NULL != pclSubNodeDefinition )
	{
		if( pclSubNodeDefinition->GetNodeName().CompareNoCase( IMPORTEXPORTHM_NODE_HN_P_MODE ) )
		{
			CStringA strMode = valParameters[IMPORTEXPORTHM_NODE_HN_VERSION].asString().c_str();

			if( dVersion < 1.000 || dVersion > 1.0002 )
			{
				// Error msg: not the good version.
				// FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE, TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_ONEROOTNODE ) );
				return false;
			}
		}
		else if( pclSubNodeDefinition->GetNodeName().CompareNoCase( IMPORTEXPORTHM_NODE_HN_DIRECTION ) )
		{
			if( 0 != CStringA( valHydraulicNetworkDefinition[IMPORTEXPORTHM_NODE_HN_DIRECTION].asString().c_str() ).CompareNoCase("import_to_hyselect") )
			{
				// Error msg: the direction must be "import_to_hyselect".
				// FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE, TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_ONEROOTNODE ) );
				return false;
			}
		}
		else if( pclSubNodeDefinition->GetNodeName().CompareNoCase( IMPORTEXPORTHM_NODE_HN_PARAMETERS ) )
		{
			if( false == _ReadParameters( valHydraulicNetwork[IMPORTEXPORTHM_NODE_HN_PARAMETERS], *pclSubNodeDefinition, strErrorMsg ) )
			{
				return false;
			}
		}
		else if( pclSubNodeDefinition->GetNodeName().CompareNoCase( IMPORTEXPORTHM_NODE_HN_HYDRAULICCIRCUITLIST ) )
		{
			if( false == _ReadHydraulicCircuitList( valHydraulicNetwork[IMPORTEXPORTHM_NODE_HN_HYDRAULICCIRCUITLIST], *pclSubNodeDefinition, strErrorMsg ) )
			{
				return false;
			}
		}
		else
		{
			// Error msg: node name not recognized.
			// FormatString( strErrorMsg, IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE, TASApp.LoadLocalizedString( IDS_IMPORTHMFROMJSONHELPER_INVALIDFILE_ONEROOTNODE ) );
			return false;
		}

		pclSubNodeDefinition = clNodeDefinition.GetNextSubNodeDefinition();
	}

	return true;
}

bool CImportHMFromJson::_ReadHydraulicCircuitList( Json::Value &val, CImportExportHMNodeDefinitions::CNodeDefinition &clNodeDefinition, CString &strErrorMsg )
{
	return true;
}

bool CImportHMFromJson::_ReadHydraulicCircuit( Json::Value &val, CImportExportHMNodeDefinitions::CNodeDefinition &clNodeDefinition, CString &strErrorMsg )
{
	return true;
}

bool CImportHMFromJson::_ReadTerminalUnit( Json::Value &valTerminalUnit, CImportExportHMNodeDefinitions::CNodeDefinition &clNodeDefinition, CString &strErrorMsg )
{
	return true;
}
	
bool CImportHMFromJson::_ReadPipe( Json::Value &valPipe, CImportExportHMNodeDefinitions::CNodeDefinition &clNodeDefinition, CString &strErrorMsg )
{
	return true;
}

bool CImportHMFromJson::_ReadPipeAccessoryList( Json::Value &valPipeAccessoryList, CImportExportHMNodeDefinitions::CNodeDefinition &clNodeDefinition, CString &strErrorMsg )
{
	return true;
}

bool CImportHMFromJson::_ReadPipeAccessory( Json::Value &valPipeAccessory, CImportExportHMNodeDefinitions::CNodeDefinition &clNodeDefinition, CString &strErrorMsg )
{
	return true;
}
	
bool CImportHMFromJson::_ReadBalancingValve( Json::Value &valBalancingValve, CImportExportHMNodeDefinitions::CNodeDefinition &clNodeDefinition, CString &strErrorMsg )
{
	return true;
}

bool CImportHMFromJson::_ReadDifferentialPressureController( Json::Value &valDifferentialPressureController, CImportExportHMNodeDefinitions::CNodeDefinition &clNodeDefinition, CString &strErrorMsg )
{
	return true;
}

bool CImportHMFromJson::_ReadControlValve( Json::Value &valControlValve, CImportExportHMNodeDefinitions::CNodeDefinition &clNodeDefinition, CString &strErrorMsg )
{
	return true;
}

bool CImportHMFromJson::_ReadShutoffValve( Json::Value &valShutoffValve, CImportExportHMNodeDefinitions::CNodeDefinition &clNodeDefinition, CString &strErrorMsg )
{
	return true;
}

bool CImportHMFromJson::_ReadPump( Json::Value &valPump, CImportExportHMNodeDefinitions::CNodeDefinition &clNodeDefinition, CString &strErrorMsg )
{
	return true;
}
*/