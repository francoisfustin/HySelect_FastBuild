#pragma once


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class that will manage the creation of an hydraulic network from the one imported from a file into the "CImportExportHMData"
// and will manage the importation of an hydraulic network int the "CImportExportHMData" class before exporting to file.
class CImportExportHM
{
public:
	CImportExportHM() {}
	~CImportExportHM() {}

	bool ConvertHMBeforeToExport( CTable *pclTable, ProjectType eProjecType, CWaterChar clWaterChar, double dReturnTemperature, CString strErrorMsg );
	bool CreateHMAfterImport( CTable *pclTable, CString strErrorMsg );
	CImportExportHMData *GetImportExportHMData() { return &m_clImportExportHMData; }

// Private methods.
private:
	bool _ReadHydraulicCircuitList( CTable *pclTable, CImportExportHMData::CHydraulicCircuitListData *pclHydraulicCircuitList, CString &strErrorMsg );

	bool _ReadTerminalUnitList( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	
	bool _ReadPipeList( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _ReadPipeAccessoryList( CPipes *pclHMPipes, CImportExportHMData::CPipeData *pclPipeData, CString &strErrorMsg );
	     	
	bool _ReadBalancingValveList( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _ReadDpControllerValveList( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _ReadControlValveList( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _ReadShutoffValveList( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );
	bool _ReadPump( CDS_HydroMod *pHM, CImportExportHMData::CHydraulicCircuitData *pclHydraulicCircuit, CString &strErrorMsg );

// Private variables.
private:
	CImportExportHMData m_clImportExportHMData;
};
