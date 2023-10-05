#pragma once


#include "reader.h"
#include "value.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to help importation of an hydraulic network from a file of .json format.
class CImportHMFromJson : public CImportHMBase
{
public:
	CImportHMFromJson() {}
	virtual ~CImportHMFromJson() {}

	virtual bool OpenFile( CString strFileName, CString &strErrorMsg );
	
// Protected methods.
protected:

	// Members inherited from 'CImportHMBase'.
	virtual bool GetSubNode( NodeDefinition &node, CString strNodeName, NodeDefinition &nodeSub, CString &strErrorMsg );
	virtual bool GetFirstSubNode( NodeDefinition &node, NodeDefinition &nodeSub, CString &strErrorMsg );
	virtual bool GetNextSubNode( NodeDefinition &node, NodeDefinition &nodeSub, CString &strErrorMsg );

	// Allow to know if a node is defined in the 'node'.
	virtual bool IsNodeEmpty( NodeDefinition &node );

	virtual bool ReadIntValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, int &iValue );
	virtual bool ReadDoubleValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, double &dValue );
	virtual bool ReadStringValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, CString &strValue );
};
