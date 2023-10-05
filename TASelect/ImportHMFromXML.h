#pragma once


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class to help importation of an hydraulic network from a file of .json format.
class CImportHMFromXML : public CImportHMBase
{
public:
	CImportHMFromXML() {}
	virtual ~CImportHMFromXML() {}

	virtual bool OpenFile( CString strFileName, CString &strErrorMsg );

// Protected methods.
protected:

	// Members inherited from 'CImportHMBase'.
	virtual CString GetNodeText( NodeDefinition &nodeMain );
	virtual bool GetSubNode( NodeDefinition &nodeMain, CString strNode, NodeDefinition &nodeSub );
};
