#include "stdafx.h"


#include "Global.h"
#include "TASelect.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "ImportExportHMNodeDefinitions.h"
#include "ImportExportHM.h"
#include "ExportHMBase.h"
#include "ExportHMToJson.h"

CExportHMToJson::CExportHMToJson()
{
	m_rMainNode.pnodeJson = new Json::Value( Json::objectValue );
}

CExportHMToJson::~CExportHMToJson()
{
	if( NULL != m_rMainNode.pnodeJson )
	{
		delete m_rMainNode.pnodeJson;
		m_rMainNode.pnodeJson = NULL;
	}
}

bool CExportHMToJson::WriteFile( CString strFileName, CString &strErrorMsg )
{
	// Force locale info to be in English - United States to have the same between all computers.
	LCID CurrentLocale = GetThreadLocale();

	// Prefer the 'SetThreadLocale' function instead of the '_tsetlocale' function. Because the first function will set the DEFAULT
	// regional settings corresponding to the LCID and not the one that the user can change in the Windows settings.
	SetThreadLocale( 0x0409 );

	Json::StyledWriter writer;
	// Json::FastWriter writer;
	_string strJsonStructure = writer.write( *m_rMainNode.pnodeJson );

	// Restore regional settings.
	SetThreadLocale( CurrentLocale );

	std::wofstream outputFile;

	outputFile.open( strFileName, std::ofstream::binary );

	if( outputFile.fail() )
	{
		strErrorMsg.Format( _T("Can't open the '%s' file."), strFileName );
		return false;
	}

	outputFile << strJsonStructure;
	outputFile.close();

	return true;
}

bool CExportHMToJson::AddSubNodeArray( NodeDefinition &node, CString strNodeName, NodeDefinition &subNodeArray, CString &strErrorMsg )
{
	if( true == node.pnodeJson->isArray() )
	{
		subNodeArray.pnodeJson = &node.pnodeJson->append( Json::Value( Json::arrayValue ) );
	}
	else
	{
		Json::Value temp = Json::Value( Json::arrayValue );
		(*node.pnodeJson)[strNodeName] = temp;
		subNodeArray.pnodeJson = &(*node.pnodeJson)[strNodeName];
	}

	return true;
}

bool CExportHMToJson::AddSubNodeObject( NodeDefinition &node, CString strNodeName, NodeDefinition &subNodeObject, CString &strErrorMsg )
{
	if( true == node.pnodeJson->isArray() )
	{
		subNodeObject.pnodeJson = &node.pnodeJson->append( Json::Value( Json::objectValue ) );
	}
	else
	{
		Json::Value temp = Json::Value( Json::objectValue );
		(*node.pnodeJson)[strNodeName] = temp;
		subNodeObject.pnodeJson = &(*node.pnodeJson)[strNodeName];
	}

	return true;
}

bool CExportHMToJson::WriteIntValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, int iValue )
{
	Json::Value val( iValue );
	(*node.pnodeJson)[strNodeName] = val;
	return true;
}

bool CExportHMToJson::WriteDoubleValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, double dValue )
{
	Json::Value val( dValue );
	(*node.pnodeJson)[strNodeName] = val;
	return true;
}

bool CExportHMToJson::WriteStringValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, CString strValue )
{
	Json::Value val( (LPCTSTR)strValue );
	(*node.pnodeJson)[strNodeName] = val;
	return true;
}
