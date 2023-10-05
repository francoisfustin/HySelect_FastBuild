#pragma once


#include "reader.h"
#include "value.h"

// The version in Json file is input as "1.002".
#define IMPORTEXPORTHM_VALUE_HN_VERSION										IMPORTEXPORTHM_VERSION
#define IMPORTEXPORTHM_VALUE_HN_DIRECTION										_T("import_to_hyselect")

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class to help importation of an hydraulic network.

class CImportHMBase
{
public:
	typedef struct _NodeDefinition
	{
		Json::Value nodeJson;
		// XML nodeXML;
		Json::Value::iterator jsonIter;
	}NodeDefinition;

	CImportHMBase();
	virtual ~CImportHMBase();

	bool Import( CString strFileName, CString &strErrorMsg );

// Protected methods.
protected:
	bool ReadFileInBuffer( CString strFileName, CString &strErrorMsg, ULONGLONG &ullBuferSize );

	virtual bool GetSubNode( NodeDefinition &node, CString strNodeName, NodeDefinition &nodeSub, CString &strErrorMsg ) = 0;
	virtual bool GetFirstSubNode( NodeDefinition &node, NodeDefinition &nodeSub, CString &strErrorMsg ) = 0;
	virtual bool GetNextSubNode( NodeDefinition &node, NodeDefinition &nodeSub, CString &strErrorMsg ) = 0;
	
	// Allow to know if a node is defined in the 'node'.
	virtual bool IsNodeEmpty( NodeDefinition &node ) = 0;

	virtual bool ReadIntValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, int &iValue ) = 0;
	virtual bool ReadDoubleValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, double &dValue ) = 0;
	virtual bool ReadStringValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, CString &strValue ) = 0;
	
// Protected variables.
protected:
	CImportExportHMData m_clImportExportHMData;
	NodeDefinition m_rMainNode;

	int m_iVersion;
	std::wstring m_strFile;

	std::vector<CString> m_vecNames;

// Private methods.
private:
	bool _FindRootNodeUTF16( std::stringstream &cStream, int iSize );
	bool _CheckIfUTF8( std::stringstream &cStream, int iSize );

	bool _ReadHydraulicNetwork( NodeDefinition &nodeHydraulicNetwork, CImportExportHMData::CHydraulicNetworkData &clHydraulicNetwork, CString &strErrorMsg );

	bool _ReadParameters( NodeDefinition &nodeParameters, CImportExportHMData::CParametersData &clParameters, CString &strErrorMsg );
	bool _ReadFluidCharacteristic( NodeDefinition &nodeFluidCharacteristic, CWaterChar &clWaterChar, double &dReturnTemperature, CString &strErrorMsg );
	bool _VerifyTemperature( CWaterChar &clWaterChar, double dTemperature, int iPrefix, CString &strErrorMsg );
	bool _ReadHydraulicCircuitList( NodeDefinition &nodeHydraulicCircuitList, CImportExportHMData::CHydraulicCircuitListData *pclHydraulicCircuitList, CString &strErrorMsg );

	bool _ReadTerminalUnitList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	
	bool _ReadPipeList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _ReadPipeAccessoryList( NodeDefinition &nodePipeAccessoryList, CImportExportHMData::CPipeData *pclPipeData, CString &strErrorMsg );
	     	
	bool _ReadBalancingValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _ReadDpControllerValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _ReadControlValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _ReadShutoffValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _ReadPump( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
};
