#pragma once


#include "writer.h"
#include "value.h"


// The version in Json file is input as "1.002".
#define EXPORT_HN_VERSION										IMPORTEXPORTHM_VERSION
#define EXPORT_HN_DIRECTION										_T("export_from_hyselect")

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base class to help exportation of an hydraulic network.

class CImportExportHM;
class CExportHMBase
{
public:
	typedef struct _NodeDefinition
	{
		Json::Value *pnodeJson;
		// XML nodeXML;
		Json::Value::iterator jsonIter;
	}NodeDefinition;

	CExportHMBase();
	virtual ~CExportHMBase();

	bool Export( CTable *pclTable, ProjectType eProjecType, CWaterChar clWaterChar, double dReturnTemperature, CString strFileName, CString &strErrorMsg );

// Protected methods.
protected:
	virtual bool WriteFile( CString strFileName, CString &strErrorMsg ) = 0;

	virtual bool AddSubNodeArray( NodeDefinition &node, CString strNodeName, NodeDefinition &subNodeArray, CString &strErrorMsg ) = 0;
	virtual bool AddSubNodeObject( NodeDefinition &node, CString strNodeName, NodeDefinition &subNodeObject, CString &strErrorMsg ) = 0;

	virtual bool WriteIntValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, int iValue ) = 0;
	virtual bool WriteDoubleValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, double dValue ) = 0;
	virtual bool WriteStringValue( NodeDefinition &node, CString strNodeName, CString &strErrorMsg, CString strValue ) = 0;

// Protected variables.
protected:
	NodeDefinition m_rMainNode;
	CImportExportHM m_clImportExportHM;

// Private methods.
private:
	bool _WriteHydraulicNetwork( NodeDefinition &nodeHydraulicNetwork, CImportExportHMData::CHydraulicNetworkData &clHydraulicNetwork, CString &strErrorMsg );

	bool _WriteParameters( NodeDefinition &nodeParameters, CImportExportHMData::CParametersData &clParameters, CString &strErrorMsg );
	bool _WriteFluidCharacteristic( NodeDefinition &nodeFluidCharacteristic, CWaterChar &clWaterChar, double &dReturnTemperature, CString &strErrorMsg );
	bool _WriteHydraulicCircuitList( NodeDefinition &nodeHydraulicNetwork, CImportExportHMData::CHydraulicCircuitListData *pclHydraulicCircuitList, CString &strErrorMsg );

	bool _WriteTerminalUnitList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	
	bool _WritePipeList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _WritePipeAccessoryList( NodeDefinition &nodePipeList, CImportExportHMData::CPipeData *pclPipeData, CString &strErrorMsg );
	     	
	bool _WriteBalancingValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _WriteDpControllerValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _WriteControlValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _WriteShutoffValveList( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _WritePump( NodeDefinition &nodeHydraulicCircuit, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
};
