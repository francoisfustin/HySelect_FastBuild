#pragma once


#include "reader.h"
#include "value.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to help exportation of an hydraulic network to a file of .json format.
class CExportHMToJson : public CExportHMBase
{
public:
	CExportHMToJson();
	virtual ~CExportHMToJson();

// Protected methods.
protected:
	virtual bool WriteFile( CString strFileName, CString &strErrorMsg );

	// Members inherited from 'CImportHMBase'.
	virtual bool AddSubNodeArray( NodeDefinition &node, CString strNodeName, NodeDefinition &subNodeArray, CString &strErrorMsg );
	virtual bool AddSubNodeObject( NodeDefinition &node, CString strNodeName, NodeDefinition &subNodeObject, CString &strErrorMsg );

	virtual bool WriteIntValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, int iValue );
	virtual bool WriteDoubleValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, double dValue );
	virtual bool WriteStringValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, CString strValue );
};
