#pragma once


#include "RViewSSelSS.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES ALL VERSIONS

// Remark: 'CW' is for 'Column Width'.
// Version 3: 2015-11-02: Too much error, need to reset the column width.
#define CW_RVIEWSSELPM_COMPUTEDDATA_VERSION					10

// Version 2: 'CD_PriceIndex_ExpansionVesselMembrane' added.
// Version 3: 2015-11-02: Too much error, need to reset the column width.
#define CW_RVIEWSSELPM_PRICEINDEX_VERSION					10

// Version 5: 'CD_ExpansionVessel_MaxWeight' added.
// Version 6: 'CD_ExpansionVessel_PriceIndexIcon', 'CD_ExpansionVessel_PriceIndexValue' and 'CD_ExpansionVessel_PriceIndexBorder' added.
// Version 7: 2015-11-02: Too much error, need to reset the column width.
// Version 11: 2016-10-19: 'CD_ExpansionVessel_CheckBox' added.
// Version 12: 2018-08-24: Swap of the 'CD_ExpansionVessel_OptimizedInitialPressure' and 'CD_ExpansionVessel_OptimizedWaterReserve' columns.
// Version 13: 2019-02-18: HYS-1022: 'pa' (Initial pressure) becomes 'pa,min' (Minimum initial pressure). And this value is set in the 'Calculated data'.
//             We thus delete this column. 'Vwrop' becomes 'Vwr', 'paopt' becomes 'pa'. And we switch the two columns.
// Version 14: 2020-02-05: HYS-1161: Add the required nominal volume column when selection is done with the EN12828 norm.
// Version 15: 2020-03-18: HYS-1352: Add a information icon in each row for the 'Requires nominal volume' column when we are in heating and EN12828.
#define CW_RVIEWSSELPM_EXPANSIONVESSEL_VERSION				15

// Version 2: 2015-11-02: Too much error, need to reset the column width.
// Version 11: 2016-10-19: 'CD_ExpansionVessel_CheckBox' added.
// Version 12: 2018-08-24: Swap of the 'CD_ExpansionVessel_OptimizedInitialPressure' and 'CD_ExpansionVessel_OptimizedWaterReserve' columns.
// Version 13: 2019-02-18: HYS-1022: 'pa' (Initial pressure) becomes 'pa,min' (Minimum initial pressure). And this value is set in the 'Calculated data'.
//             We thus delete this column. 'Vwrop' becomes 'Vwr', 'paopt' becomes 'pa'. And we switch the two columns.
// Version 14: 2020-03-18: HYS-1352: Add a information icon in each row for the 'Requires nominal volume' column when we are in heating and EN12828. 
#define CW_RVIEWSSELPM_EXPANSIONVESSELMEMBRANE_VERSION		14

// Version 5: 'CD_TecBoxVssl_Prim_MaxWeight' and 'CD_TecBoxVssl_Sec_MaxWeight' added.
// Version 6: 'CD_TecBox_PriceIndexIcon', 'CD_TecBox_PriceIndexValue', 'CD_TecBox_PriceIndexBorder', 'CD_TecBoxVssl_PriceIndexRatioIcon',
//			  'CD_TecBoxVssl_PriceIndexRatioValue' and 'CD_TecBoxVssl_PriceIndexRatioBorder' added.
// Version 7: 2015-11-02: Too much error, need to reset the column width.
// Version 11: 2016-10-19: 'CD_TecBox_CheckBox' added.
// Version 12: 2017-09-29: 'CD_TecBox_IntegratedVessel' and 'SD_CompressoSecondaryVessel' added.
#define CW_RVIEWSSELPM_COMPRESSO_VERSION					12

// Version 6: 'CD_TecBoxVssl_Prim_MaxWeight' and 'CD_TecBoxVssl_Sec_MaxWeight' added.
// Version 7: 'CD_TecBox_PriceIndexIcon', 'CD_TecBox_PriceIndexValue', 'CD_TecBox_PriceIndexBorder', 'CD_TecBoxVssl_PriceIndexRatioIcon',
//			  'CD_TecBoxVssl_PriceIndexRatioValue' and 'CD_TecBoxVssl_PriceIndexRatioBorder' added.
// Version 8: 2015-11-02: Too much error, need to reset the column width.
// Version 11: 2016-10-19: 'CD_TecBox_CheckBox' added.
#define CW_RVIEWSSELPM_TRANSFERO_VERSION					11

// Version 3: 'CD_IntermVssl_MaxWeight' added.
// Version 4: 2015-11-02: Too much error, need to reset the column width.
#define CW_RVIEWSSELPM_INTERMEDIATEVESSEL_VERSION			11

// Version 3: 'CD_TecBoxPleno_PriceIndexIcon', 'CD_TecBoxPleno_PriceIndexValue' and 'CD_TecBoxPleno_PriceIndexBorder' added.
// Version 4: 2015-11-02: Too much error, need to reset the column width.
// Version 11: 2016-10-19: 'CD_TecBoxPleno_CheckBox' added.
// Version 12: 2017-10-03: Pleno refill: 'CD_PlenoRefill_CheckBox' added.
#define CW_RVIEWSSELPM_PLENO_VERSION						12

// version 3: 'CD_TecBoxVento_PriceIndexIcon', 'CD_TecBoxVento_PriceIndexValue' and 'CD_TecBoxVento_PriceIndexBorder' added.
// Version 4: 2015-11-02: Too much error, need to reset the column width.
// Version 11: 2016-10-19: 'CD_TecBoxVento_CheckBox' added.
#define CW_RVIEWSSELPM_VENTO_VERSION						11
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINE ALL SHEET ID
//
// Remark: don't take enum 'SheetDescription' defined in .h. Because if for any reason we insert a new sheet, the following ID will
//         be increment and will no more correspond to the good one.
//
//         And it is the case for PM. I add 'SD_ExpansionVesselMembrane' thus next sheet ID are no more correct.

#define CW_RVIEWSSELPM_SHEETID_COMPUTEDATA					( 1 )
#define CW_RVIEWSSELPM_SHEETID_PRICEINDEX					( 2 )
#define CW_RVIEWSSELPM_SHEETID_EXPANSIONVESSEL				( 3 )
#define CW_RVIEWSSELPM_SHEETID_TECBOXCOMPRESSO				( 5 )
#define CW_RVIEWSSELPM_SHEETID_TECBOXTRANSFERO				( 9 )
#define CW_RVIEWSSELPM_SHEETID_INTERMEDIATEVESSEL			( 13 )
#define CW_RVIEWSSELPM_SHEETID_COMPRESSOVESSEL				( 7 )
#define CW_RVIEWSSELPM_SHEETID_TRANSFEROVESSEL				( 11 )
#define CW_RVIEWSSELPM_SHEETID_TECBOXPLENO					( 17 )
#define CW_RVIEWSSELPM_SHEETID_TECBOXVENTO					( 15 )
#define CW_RVIEWSSELPM_SHEETID_EXPANSIONVESSELMEMBRANE		( 23 )	// 23 to be sure to not interfere with previous sheetID
#define CW_RVIEWSSELPM_SHEETID_COMPRESSOSECVESSEL			( 24 )
#define CW_RVIEWSSELPM_SHEETID_PLENOREFILL					( 25 )
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRViewSSelPM : public CRViewSSelSS
{
public:

	// For pressurisation we can have more that one sheet in the right view. To "ease" the coding, we have created group and parent.
	// A sheet is always defined with its sheet description ID. And a sheet can belong to a group and have a parent:
	//
	//		SheetDescriptionID							GroupID					ParentID
	//      -------------------------------------------------------------------------------------------------------------
	//		SD_TecBoxCompresso							SD_TecBoxCompresso		-1
	//		SD_TecBoxCompressoAccessory					SD_TecBoxCompresso		SD_TecBoxCompresso
	//		SD_CompressoVessel							SD_TecBoxCompresso		-1
	//		SD_CompressoVesselAccessory					SD_TecBoxCompresso		SD_CompressoVessel
	//		SD_CompressoSecondaryVessel					SD_TecBoxCompresso		-1
	//		SD_CompressoSecondaryVesselAccessory		SD_TecBoxCompresso		SD_CompressoSecondaryVesselAccessory
	// 
	//		SD_TecBoxTransfero							SD_TecBoxTransfero		-1
	//		SD_TecBoxTransferoAccessory					SD_TecBoxTransfero		SD_TecBoxTransfero
	//		SD_TecBoxTransferoBufferVesselAccessory		SD_TecBoxTransfero		SD_TecBoxTransfero
	//		SD_TransferoVessel							SD_TecBoxTransfero		-1
	//		SD_TransferoVesselAccessory					SD_TecBoxTransfero		SD_TransferoVessel

	// These enum help us to manage 'Pleno' and 'Vento'.
	enum SheetDescriptionGroup
	{
		SDG_ExpansionVessel	= 0x1000,
		SDG_ExpansionVesselMembrane	= 0x2000,
		SDG_Compresso = 0x4000,
		SDG_Transfero = 0x8000,
		SDG_Mask = ( SDG_ExpansionVessel + SDG_ExpansionVesselMembrane + SDG_Compresso + SDG_Transfero )
	};
#define GETWITHOUTGROUPID( i )		( i & (~SDG_Mask) )

	// Description of all available sheets.
	// Remark: Be sure that there is always one sheet and just after its accessories.
	enum SheetDescription
	{
		SD_First = 1,
		SD_ComputedData = SD_First,
		SD_PriceIndex,
		SD_ExpansionVessel,
		SD_ExpansionVesselAccessory,
		SD_ExpansionVesselMembrane,
		SD_ExpansionVesselMembraneAccessory,
		SD_TecBoxCompresso,
		SD_TecBoxCompressoAccessory,
		SD_CompressoVessel,
		SD_CompressoVesselAccessory,
		SD_CompressoSecondaryVessel,					// For Simply Compresso
		SD_CompressoSecondaryVesselAccessory,			// For Simply Compresso
		SD_TecBoxTransfero,
		SD_TecBoxTransferoAccessory,
		SD_TransferoVessel,
		SD_TransferoVesselAccessory,
		SD_IntermediateVessel,
		SD_IntermediateVesselAcc,
		SD_TecBoxVento,
		SD_TecBoxVentoAccessory,
		SD_TecBoxPleno,
		SD_TecBoxPlenoAccessory,
		SD_PlenoWaterMakeUpProtection,
		SD_PlenoRefill,
		SD_PlenoRefillAccessory,
		SD_TecBoxTransferoBufferVesselAccessory,
		SD_Last
	};

	// Row description for the 'Computed data' sheet.
	enum RowDescription_ComputedData
	{
		RD_ComputedData_FirstRow = 1,
		RD_ComputedData_GroupName,
		RD_ComputedData_ColName,
		RD_ComputedData_FirstAvailRow, // 4
		RD_ComputedData_SystemExpansionCoeff = RD_ComputedData_FirstAvailRow,
		RD_ComputedData_XFactor,							// For SWKI HE301-01 norm.
		RD_ComputedData_SystemExpansionVolume,
		RD_ComputedData_StorageTankExpCoeff,				// For SWKI HE301-01 norm.
		RD_ComputedData_StorageTankExpansionVolume,			// For SWKI HE301-01 norm.
		RD_ComputedData_TotalExpansionVolume,				// HYS-1565: Ve,tot = Ve [+ storage tank expansion]
		RD_ComputedData_SolarCollectorMultiplierFactor,
		RD_ComputedData_SolarCollectorSecurityVolume,
		RD_ComputedData_MinWaterReserveToAdd,				// Except for SWKI HE301-01 norm.
		RD_ComputedData_DegassingWaterReserve,
		RD_ComputedData_VesselNetVolume,					// HYS-1565: Vn = Ve,tot [+ Solar water reserve] [+ Min water reserve] [+ Degassing water reserve]
		RD_ComputedData_PressureFactor,
		RD_ComputedData_NominalVolume,
		RD_ComputedData_MinWaterReserveIncludedInExpansion,	// For SWKI HE301-01 norm -> Vwr,min is already included with the X factor in Ve.
		RD_ComputedData_ContractedVolume,					// For cooling system.
		RD_ComputedData_VaporPressure,
		RD_ComputedData_Pz,
		RD_ComputedData_MinimumPressure,
		RD_ComputedData_MinimumInitialPressure,
		RD_ComputedData_InitialPressure,
		RD_ComputedData_TargetPressure,
		RD_ComputedData_FinalPressure,
		RD_ComputedData_MinimumPSV,
		RD_ComputedData_NeededQRateEqVol,
		RD_ComputedData_ExpansionPipeBlw10,
		RD_ComputedData_ExpansionPipeAbv10,
		RD_ComputedData_LastRow
	};

	// Column description for the 'Computed data' sheet.
	enum ColumnDescription_ComputedData
	{
		CD_ComputedData_FirstColumn = 1,
		CD_ComputedData_Name,
		CD_ComputedData_Value,
		CD_ComputedData_ValueIndex,
		CD_ComputedData_ExpansionVessel,
		CD_ComputedData_ExpansionVesselIndex,
		CD_ComputedData_Compresso,
		CD_ComputedData_CompressoIndex,
		CD_ComputedData_Transfero,
		CD_ComputedData_TransferoIndex,
		CD_ComputedData_LastColumn = CD_ComputedData_TransferoIndex
	};

	// Row description for the 'Price index' sheet.
	enum RowDescription_PriceIndex
	{
		RD_PriceIndex_FirstRow = 1,
		RD_PriceIndex_GroupName,
		RD_PriceIndex_ColName,
		RD_PriceIndex_FirstAvailRow, // 4
		RD_PriceIndex_BestPriceIndex = RD_PriceIndex_FirstAvailRow,
		RD_PriceIndex_CurrentSelection,
		RD_PriceIndex_LastRow = RD_PriceIndex_CurrentSelection
	};

	// Column description for the 'Price index' sheet.
	enum ColumnDescription_PriceIndex
	{
		CD_PriceIndex_FirstColumn = 1,
		CD_PriceIndex_Name,
		CD_PriceIndex_ExpansionVessel,
		CD_PriceIndex_ExpansionVesselMembrane,
		CD_PriceIndex_Compresso,
		CD_PriceIndex_Transfero,
		CD_PriceIndex_LastColumn = CD_PriceIndex_Transfero
	};

	// Row description for the 'Expansion vessel' sheet.
	enum RowDescription_ExpansionVessel
	{
		RD_ExpansionVessel_FirstRow = 1,
		RD_ExpansionVessel_GroupName,
		RD_ExpansionVessel_ColName,
		RD_ExpansionVessel_ColUnit,
		RD_ExpansionVessel_FirstAvailRow
	};

	// Column description for the 'Expansion vessel' sheet.
	enum ColumnDescription_ExpansionVessel
	{
		CD_ExpansionVessel_FirstColumn = 1,
		CD_ExpansionVessel_CheckBox,
		CD_ExpansionVessel_PriceIndexIcon,
		CD_ExpansionVessel_PriceIndexValue,
		CD_ExpansionVessel_PriceIndexBorder,
		CD_ExpansionVessel_Name,
		CD_ExpansionVessel_NbreOfVssl,
		CD_ExpansionVessel_Volume,
		CD_ExpansionVessel_MaxPressure,
		CD_ExpansionVessel_TempRange,
		CD_ExpansionVessel_FactoryPresetPressure,
		CD_ExpansionVessel_Diameter,
		CD_ExpansionVessel_Height,
		CD_ExpansionVessel_Weight,
		CD_ExpansionVessel_MaxWeight,
		CD_ExpansionVessel_Standing,
		CD_ExpansionVessel_Connection,
		CD_ExpansionVessel_RequiredNominalVolumeEN12828,
		CD_ExpansionVessel_RequiredNominalVolumeEN12828InfoIcon,
		CD_ExpansionVessel_InitialPressure,
		CD_ExpansionVessel_WaterReserve,
		CD_ExpansionVessel_Pointer,
		CD_ExpansionVessel_LastColumn
	};

	// Row description for intermediate Vessel sheet.
	enum RowDescription_IntermVssl
	{
		RD_IntermVssl_FirstRow = 1,
		RD_IntermVssl_GroupName,
		RD_IntermVssl_ColName,
		RD_IntermVssl_ColUnit,
		RD_IntermVssl_FirstAvailRow
	};

	enum ColumnDescription_IntermVssl
	{
		CD_IntermVssl_FirstColumn = 1,
		CD_IntermVssl_CheckBox,
		CD_IntermVssl_Type,
		CD_IntermVssl_NbreOfVssl,
		CD_IntermVssl_Volume,
		CD_IntermVssl_MaxPressure,
		CD_IntermVssl_TempRange,
		CD_IntermVssl_Diameter,
		CD_IntermVssl_Height,
		CD_IntermVssl_Weight,
		CD_IntermVssl_MaxWeight,
		CD_IntermVssl_Connection,
		CD_IntermVssl_Pointer,
		CD_IntermVssl_LastColumn
	};

	// Row description for the 'Compresso' sheet.
	enum RowDescription_TecBox
	{
		RD_TecBox_FirstRow = 1,
		RD_TecBox_GroupName,
		RD_TecBox_ColName,
		RD_TecBox_ColUnit,
		RD_TecBox_FirstAvailRow
	};

	// Column description for the 'Compresso' sheet.
	enum ColumnDescription_TecBox
	{
		CD_TecBox_FirstColumn = 1,
		CD_TecBox_CheckBox,
		CD_TecBox_PriceIndexIcon,
		CD_TecBox_PriceIndexValue,
		CD_TecBox_PriceIndexBorder,
		CD_TecBox_Name,
		CD_TecBox_NbreOfDevice,
		CD_TecBox_PS,
		CD_TecBox_TempRange,
		CD_TecBox_MaxWaterMakeUpTemp,
		CD_TecBox_B,
		CD_TecBox_H,
		CD_TecBox_T,
		CD_TecBox_Weight,
		CD_TecBox_Standing,
		CD_TecBox_ElectricalPower,
		CD_TecBox_SupplyVoltage,
		CD_TecBox_Decibel,
		CD_TecBox_BufferVessel,					// Buffer vessel in some Transfero
		CD_TecBox_IntegratedVessel,				// Integrated vessel like CD 80 in the Simply Compresso.
		CD_TecBox_Pointer,
		CD_TecBox_LastColumn
	};

	// Row description for the 'Compresso Vessel' sheet.
	enum RowDescription_TecBoxVssl
	{
		RD_TecBoxVssl_FirstRow = 1,
		RD_TecBoxVssl_GroupName,
		RD_TecBoxVssl_ColName,
		RD_TecBoxVssl_ColUnit,
		RD_TecBoxVssl_FirstAvailRow
	};

	enum ColumnDescription_TecBox_Vssl
	{
		CD_TecBoxVssl_FirstColumn = 1,
		CD_TecBoxVssl_CheckBox,
		CD_TecBoxVssl_PriceIndexRatioIcon,
		CD_TecBoxVssl_PriceIndexRatioValue,
		CD_TecBoxVssl_PriceIndexRatioBorder,
		CD_TecBoxVssl_Prim_Name,
		CD_TecBoxVssl_Prim_Volume,
		CD_TecBoxVssl_Prim_MaxPressure,
		CD_TecBoxVssl_Prim_TempRange,
		CD_TecBoxVssl_Prim_Diameter,
		CD_TecBoxVssl_Prim_Height,
		CD_TecBoxVssl_Prim_Weight,
		CD_TecBoxVssl_Prim_MaxWeight,
		CD_TecBoxVssl_Sec_Name,
		CD_TecBoxVssl_Sec_NbreOfVssl,
		CD_TecBoxVssl_Sec_Diameter,
		CD_TecBoxVssl_Sec_Height,
		CD_TecBoxVssl_Sec_Weight,
		CD_TecBoxVssl_Sec_MaxWeight,
		CD_TecBoxVssl_Pointer,
		CD_TecBoxVssl_LastColumn
	};

	enum ColumnDescription_TecBox_SecondaryVessel
	{
		CD_TecBoxSecVssl_FirstColumn = 1,
		CD_TecBoxSecVssl_CheckBox,
		CD_TecBoxSecVssl_PriceIndexRatioIcon,
		CD_TecBoxSecVssl_PriceIndexRatioValue,
		CD_TecBoxSecVssl_PriceIndexRatioBorder,
		CD_TecBoxSecVssl_Name,
		CD_TecBoxSecVssl_Volume,
		CD_TecBoxSecVssl_MaxPressure,
		CD_TecBoxSecVssl_TempRange,
		CD_TecBoxSecVssl_Diameter,
		CD_TecBoxSecVssl_Height,
		CD_TecBoxSecVssl_Weight,
		CD_TecBoxSecVssl_MaxWeight,
		CD_TecBoxSecVssl_Pointer,
		CD_TecBoxSecVssl_LastColumn
	};

	enum ColumnDescription_TecBoxVento
	{
		CD_TecBoxVento_FirstColumn = 1,
		CD_TecBoxVento_CheckBox,
		CD_TecBoxVento_PriceIndexIcon,
		CD_TecBoxVento_PriceIndexValue,
		CD_TecBoxVento_PriceIndexBorder,
		CD_TecBoxVento_Name,
		CD_TecBoxVento_NbreOfDevice,
		CD_TecBoxVento_MaxPressure,
		CD_TecBoxVento_TempRange,
		CD_TecBoxVento_MaxWaterMakeUpTemp,			// In case of VP.
		CD_TecBoxVento_Width,
		CD_TecBoxVento_Height,
		CD_TecBoxVento_Depth,
		CD_TecBoxVento_Weight,
		CD_TecBoxVento_Standing,
		CD_TecBoxVento_ElectricalPower,
		CD_TecBoxVento_SupplyVoltage,
		CD_TecBoxVento_VNd,							// Vento: Water capacity for which a device is rated.
		CD_TecBoxVento_SPL,							// Sound pressure level.
		CD_TecBoxVento_dpu,							// Working pressure area of unit. Not for P/PI.
		CD_TecBoxVento_qNwm,						// Nominal volumetric flow related to the water make-up device.
		CD_TecBoxVento_IP,
		CD_TecBoxVento_Pointer,
		CD_TecBoxVento_LastColumn
	};

	enum ColumnDescription_TecBoxPleno
	{
		CD_TecBoxPleno_FirstColumn = 1,
		CD_TecBoxPleno_CheckBox,
		CD_TecBoxPleno_PriceIndexIcon,
		CD_TecBoxPleno_PriceIndexValue,
		CD_TecBoxPleno_PriceIndexBorder,
		CD_TecBoxPleno_Name,
		CD_TecBoxPleno_NbreOfDevice,
		CD_TecBoxPleno_MaxPressure,
		CD_TecBoxPleno_TempRange,					// Pleno alone the tmax is in fact the tmax water make-up.
		CD_TecBoxPleno_Width,
		CD_TecBoxPleno_Height,
		CD_TecBoxPleno_Depth,
		CD_TecBoxPleno_Weight,
		CD_TecBoxPleno_Standing,
		CD_TecBoxPleno_ElectricalPower,
		CD_TecBoxPleno_SupplyVoltage,
		CD_TecBoxPleno_Kvs,							// Pleno P/PI
		CD_TecBoxPleno_SPL,							// Sound pressure level.
		CD_TecBoxPleno_dpu,							// Working pressure area of unit. Not for P/PI.
		CD_TecBoxPleno_qNwm,						// Nominal volumetric flow related to the water make-up device.
		CD_TecBoxPleno_IP,
		CD_TecBoxPleno_Pointer,
		CD_TecBoxPleno_LastColumn
	};

	enum ColumnDescription_PlenoRefill
	{
		CD_PlenoRefill_FirstColumn = 1,
		CD_PlenoRefill_CheckBox,
		CD_PlenoRefill_PriceIndexIcon,
		CD_PlenoRefill_PriceIndexValue,
		CD_PlenoRefill_PriceIndexBorder,
		CD_PlenoRefill_Name,
		CD_PlenoRefill_NbreOfDevice,
		CD_PlenoRefill_MaxPressure,
		CD_PlenoRefill_TempRange,
		CD_PlenoRefill_Functions,
		CD_PlenoRefill_Capacity,
		CD_PlenoRefill_MaxVolume,					// Max volume that Pleno Refill can softening/desalinate.
		CD_PlenoRefill_MaxFlow,
		CD_PlenoRefill_SuplyWaterPressureRange,
		CD_PlenoRefill_InConnectAndSize,
		CD_PlenoRefill_OutConnectAndSize,
		CD_PlenoRefill_Height,
		CD_PlenoRefill_WidthWoConnect,
		CD_PlenoRefill_WidthWConnect,
		CD_PlenoRefill_Weight,
		CD_PlenoRefill_Pointer,
		CD_PlenoRefill_LastColumn
	};

	enum RowDescription_AccessoryHeader
	{
		RD_Accessory_FirstRow = 1,
		RD_Accessory_GroupName,
		RD_Accessory_FirstAvailRow
	};

	enum ColumDescription_Accessory
	{
		CD_Accessory_FirstColumn = 1,
		CD_Accessory_Left,
		CD_Accessory_Right,
		CD_Accessory_LastColumn
	};

	typedef enum PMWQPrefs_Status
	{
		PMWQPref_Unchecked = 0,
		PMWQPref_Checked
	};

	CRViewSSelPM();
	virtual ~CRViewSSelPM();

	void SetParent( CWnd *pclParent ) { m_pclParent = pclParent; }

	// To allow to get access the protected 'IsSelectReady' method.
	bool IsSelectionAvailable( void ) { return IsSelectionReady(); }

	// Allow to retrieve the current product selected in a specific sheet description.
	bool GetProductSelected( UINT uiSheetDescriptionID, UINT uiSDGroupID, CDB_Product **ppclProduct, long *plRow = NULL, LPARAM *plParam = NULL );

	// Returns the current vessel selected (expansion vessel, expansion vessel membrane, primary Compresso vessel or primary Transfero vessel) or NULL if any.
	bool GetVesselSelected( CDB_Vessel **ppclVessel, UINT *puiSheetDescriptionID = NULL, long *plRow = NULL, LPARAM *plParam = NULL );

	bool GetExpansionVesselSelected( CDB_Vessel **ppclVessel, long *plRow = NULL, LPARAM *plParam = NULL );

	bool GetExpansionVesselMembraneSelected( CDB_Vessel **ppclVessel, long *plRow = NULL, LPARAM *plParam = NULL );
	
	bool GetCompressoVesselSelected( CDB_Vessel **ppclVessel, long *plRow = NULL, LPARAM *plParam = NULL );

	bool GetCompressoSecondaryVesselSelected( CDB_Vessel **ppclVessel, long *plRow = NULL, LPARAM *plParam = NULL );
	
	bool GetTransferoVesselSelected( CDB_Vessel **ppclVessel, long *plRow = NULL, LPARAM *plParam = NULL );

	// Returns the current Compresso selected or NULL if any.
	bool GetTecBoxSelected( CDB_TecBox **ppclTechBox, UINT *puiSheetDescriptionID = NULL, long *plRow = NULL, LPARAM *plParam = NULL );
	
	bool GetTecBoxCompressoSelected( CDB_TecBox **ppclTechBox, long *plRow = NULL, LPARAM *plParam = NULL );
	
	bool GetTecBoxTransferoSelected( CDB_TecBox **ppclTechBox, long *plRow = NULL, LPARAM *plParam = NULL );

	// HYS-872: Get the selected and saved Buffer vessel 
	bool GetBufferVesselTecBoxSelected(LPARAM *plParam);

	// Returns the current intermediate vessel selected or NULL if any.
	bool GetIntermediateVesselSelected( CDB_Vessel **ppclIntermediateVessel, long *plRow = NULL, LPARAM *plParam = NULL );

	bool GetTecBoxVentoSelected( CDB_TBPlenoVento **ppclVento, UINT uiSDGroupID, long *plRow = NULL, LPARAM *plParam = NULL );

	bool GetTecBoxPlenoSelected( CDB_TBPlenoVento **ppclPleno, UINT uiSDGroupID, long *plRow = NULL, LPARAM *plParam = NULL );

	bool GetTecBoxPlenoWMProtectionSelected( CDB_TBPlenoVento **ppclPleno, UINT uiSDGroupID, long *plRow = NULL, LPARAM *plParam = NULL );

	bool GetPlenoRefillSelected( CDB_PlenoRefill **ppclPlenoRefill, UINT uiSDGroupID, long *plRow = NULL, LPARAM *plParam = NULL );

	bool GetTransferoAccessoryList( CAccessoryList *pclAccessoryListToFill );
	bool GetTransferoVesselAccessoryList( CAccessoryList *pclAccessoryListToFill );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding the 'CMultiSpreadBase' public virtual methods.
	virtual void Reset( void );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS public virtual methods.

	// Called by 'DlgIndSelXXX' the user click 'Suggest' button.
	// Param: 'pclProductSelectionParameters' contains all needed data.
	virtual void Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam = NULL );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Override 'CRViewSSelSS public pure virtual methods.
	// Remark: for pressurization maintenance we don't need these two methods because 'CDS_SSelPMaint' don't inherited from 'CDS_SelProd'.
	virtual void FillInSelected( CDS_SelProd *pSelectedProductToFill )
	{
		/* DO NOTHING */
	}

	// End of overriding CRViewSSelSS public pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FillInSelected( CDS_SSelPMaint *pclSelectedPMFill );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS public virtual methods.

	// Allow to initialize some internal variables.
	// Remark: called by 'CDlgIndSelSeparator::OnNewDocument'.
	virtual void OnNewDocument( CDS_IndSelParameter *pclIndSelParameter );

	// Allow to save some internal variables.
	// Remark: called by 'CDlgIndSelSeparator::SaveSelectionParameters'.
	virtual void SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter );

	// End of overriding CRViewSSelSS public virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Helpers for the 'DlgIndSelPMExcludedProducts'.
	void InitHeadersHelper( CSheetDescription *pclSheetDescription, CSelectPMList::ProductType eProductType );

	long FillRowHelper( CSheetDescription *pclSheetDescription, CSelectPMList::ProductType eProductType, long lRow, 
			CSelectedPMBase* pclSelectedPM, CPMInputUser *pclPMInputUser );

// Protected members
protected:

	// HYS-872: to save the combo selection
	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnComboSelChange(WPARAM wParam, LPARAM lParam);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding the 'CMultiSpreadBase' protected pure virtual methods.

	// Returns 'true' if current method has displayed a tooltip.
	virtual bool OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, TCHAR *pstrTipText,
			BOOL *pbShowTip );

	// End of overriding CMultiSpreadBase protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS protected virtual methods.
	virtual CCellDescriptionProduct* FindCDProductByUserParam( long lRow, LPARAM lpUserParam, CSheetDescription *pclSheetDescription );

	// This method is called by 'CRViewSSelSS::OnCellClicked' if the cell description is a link.
	virtual void OnCellDescriptionLinkClicked( CSheetDescription *pclSheetDescription, long lColumn, long lRow, LPARAM lpParam );

	// When right clicking on a cell and it's a 'CCellDescriptionProduct' product, if 'GetProduct' returns NULL, we can ask inherited class to interpret
	// user param if exist and it the class can do it.
	virtual CDB_Product *RetrieveProductFromUserParam( LPARAM lpParam );

	// End of overriding CRViewSSelSS protected virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CRViewSSelSS protected pure virtual methods.

	// This method is called when user click on a product (and not on button or accessories).
	virtual bool OnClickProduct( CSheetDescription *pclSheetDescriptionPM, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow );

	// This method is called when user click on a title group (to reinitialize default column width).
	virtual bool ResetColumnWidth( short nSheetDescriptionID );

	virtual bool IsSelectionReady( void );

	virtual void OnExpandCollapeGroupButtonClicked( CCDButtonExpandCollapseGroup *pCDBExpandCollapseGroup, CSheetDescription *pclSSheetDescription );

	// End of overriding CRViewSSelSS protected pure virtual methods.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to set the new cell description containing current pressure maintenance product selected.
	void SetCurrentPMSelected( UINT uiSheetDescriptionID, UINT uiSDGroupID, CCellDescriptionProduct *pclCDCurrentPMSelected );

// Private methods.
private:

	// Allow to fill the 'Computed data' sheet when user has clicked the 'Suggest' button in the 'CDlgIndSelPM' dialog.
	void _InitAndFillComputedDataRows();

	// Allow to fill the 'Price index' sheet when the PM price index is used.
	void _InitPriceIndexGroup( void );


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// EXPANSION VESSEL

	// Allow to fill the 'Expansion vessel' sheet when user has clicked the 'Suggest' button in the 'CDlgIndSelPM' dialog.
	// Returns -1 if no current edition or the row where is the current selected product.
	// Remark: 'uiSheetID' can be 'SD_ExpansionVessel' or 'SD_ExpansionVesselMembrane'.
	long _FillExpansionVesselSheet( UINT uiSheetID, CDS_SSelPMaint *pclEditedSSelPM = NULL );

	void _InitExpansionVesselHeaders( UINT uiSheetID, CSheetDescription *pclSDExpansionVessel );

	// The 'pclSelectedBestProduct' correspond to the object that belongs to the best price index.
	long _FillExpansionVesselRow( CSheetDescription *pclSDExpansionVessel, long lRow, CSelectedVssl *pclSelectedExpansionVessel, CPMInputUser *pclPMInputUser,
			CDS_SSelPMaint *pclEditedSSelPM = NULL, CSelectedPMBase* pclSelectedBestPriceProduct = NULL, bool bForRejectedProduct = false );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to complete the 'Expansion vessel' with the curves.
	bool _FillExpansionVesselCurves( CSelectedVssl *pclExpansionVesselSelected );


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// COMPRESSO

	// Allow to fill the 'Compresso' sheet when user has clicked the 'Suggest' button in the 'CDlgIndSelPM' dialog.
	// Returns -1 if no current edition or the row where is the current selected product.
	long _FillTecBoxCompressoSheet( CDS_SSelPMaint *pclEditedSSelPM = NULL );

	void _InitTecBoxCompressoHeaders( CSheetDescription *pclSDTecBoxCompresso );

	long _FillTecBoxCompressoRow( CSheetDescription *pclSDTecBoxCompresso, long lRow, CSelectedCompresso *pclSelectedTecBoxCompresso, CPMInputUser *pclPMInputUser, 
			bool bCheckPriority, CDS_SSelPMaint *pclEditedSSelPM = NULL, CSelectedPMBase* pclSelectedBestPriceProduct = NULL, bool bForRejectedProduct = false );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Allow to complete the 'Compresso' with the curves.
	bool _FillCompressoCurves( CDB_TecBox *pclCompressoSelected, int iNbrOfDevices );

	// Will call '_FillCompressoVsslSheet' or '_FillCompressoSecondaryVsslSheet' in regards to the Compresso type.
	void _FillPreCompressoVesselSheet( CSelectedPMBase *pclSelectedCompresso, UINT uiSDGroupID );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// COMPRESSO VESSEL

	// Fill primary and secondary vessel associated to selected Compresso.
	void _FillCompressoVsslSheet( UINT uiSDGroupID );

	void _InitCompressoVsslHeaders( CSheetDescription *pclSDCompressoVssl, CPMInputUser *pclPMInputUser );

	long _FillCompressoVsslRow( CSheetDescription *pclSDCompressoVssl, long lRow, CSelectedVssl *pclSelCompressoVssl, 
			CPMInputUser *pclPMInputUser, CSelectedPMBase* pclSelectedBestPriceProduct = NULL, bool bForRejectedProduct = false );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// COMPRESSO SECONDARY VESSEL

	// Fill secondary vessel associated to selected Compresso.
	// Remark: Usually the secondary vessel are shown in the same line of the primary vessel.
	//         But sometimes it can happen that the primary vessel is integrated with a TecBox (Simply Compresso for example).
	//         In that case we don't have the 'Primary vessel sheet' and we show only the secondary vessel sheet just after the Compresso sheet.
	void _FillCompressoSecondaryVsslSheet( UINT uiSDGroupID );

	void _InitCompressoSecondaryVsslHeaders( CSheetDescription *pclSDCompressoVssl );

	long _FillCompressoSecondaryVsslRow( CSheetDescription *pclSDCompressoSecondaryVssl, long lRow, CSelectedVssl *pclSelCompressoSecondaryVssl, 
			CPMInputUser *pclPMInputUser, CSelectedPMBase* pclSelectedBestPriceProduct = NULL );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TRANSFERO

	// Allow to fill the 'Transfero' sheet when user has clicked the 'Suggest' button in the 'CDlgIndSelPM' dialog.
	// Returns -1 if no current edition or the row where is the current selected product.
	long _FillTecBoxTransferoSheet( CDS_SSelPMaint *pclEditedSSelPM = NULL );

	void _InitTecBoxTransferoHeaders( CSheetDescription *pclSDTecBoxTransfero );

	long _FillTecBoxTransferoRow( CSheetDescription *pclSDTecBoxTransfero, long lRow, CSelectedTransfero *pclSelectedTecBoxTransfero, CPMInputUser *pclPMInputUser, 
			CDS_SSelPMaint *pclEditedSSelPM = NULL, CSelectedPMBase* pclSelectedBestPriceProduct = NULL, bool bForRejectedProduct = false );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Allow to complete the 'Transfero' with the curves.
	bool _FillTransferoCurves( CDB_TecBox *pclTransferoSelected );


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TRANSFERO VESSEL

	// Fill primary and secondary vessel associated to selected Transfero.
	void _FillTransferoVsslSheet( UINT uiSDGroupID, CDS_SSelPMaint *pclEditedSSelPM );

	void _InitTransferoVsslHeaders( CSheetDescription *pclSDTransferoVssl );

	long _FillTransferoVsslRow( CSheetDescription *pclSDTransferoVssl, long lRow, CSelectedVssl *pclSelTransferoVssl, CPMInputUser *pclPMInputUser, 
			CSelectedPMBase* pclSelectedBestPriceProduct = NULL, bool bForRejectedProduct = false );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Allow to fill accessory sheet.
	// Param: 'uiSheetDescriptionID' is the accessory sheet description ID we want to add.
	// Param: 'uiSDGroupID' is to defined in which group belongs the accessory sheet.
	// Param: 'uiSDParentID' is to defined who is the parent sheet of the accessory sheet.
	// Param: 'prList': if defined, we use the accessories in this list.
	// Remark: For a description of group and parent ID see above.
	void _FillAccessoryRows( UINT uiSheetDescriptionID, UINT uiSDGroupID, UINT uiSDParentID, int iIDSSheetTitle, CDB_Product *pProd, CRank *prList = NULL, 
			CDB_RuledTable *pclRuledTable = NULL );


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// INTERMEDIATE VESSEL

	// Allow to fill the 'Intermediate' sheet when user has clicked selected a vessel.
	long _FillIntermediateVsslSheet( UINT uiSheetDescriptionID, UINT uiSDGroupID, UINT uiSDParentID, CSelectedVssl *pclSelectedVessel );

	void _InitIntermediateVsslHeaders( CSheetDescription *pclSDIntermVssl );

	long _FillIntermediateVsslRow( CSheetDescription *pclSDIntermVssl, long lRow, CSelectedVssl *pclSelectedIntermVssl, 
			CPMInputUser *pclPMInputUser, bool bForRejectedProduct = false );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// VENTO

	// Allow to fill the 'Vento' sheet when user has clicked the 'Suggest' button in the 'CDlgIndSelPM' dialog or has clicked on other tecbox.
	// Param: 'uiSDGroupID' -> ID of the group sheet description (Expansion vessel, Compresso, Transfero) or -1 if 'Pleno' is selected without pressurization.
	// Param: 'pclSelectedPM' -> if defined, Vento is searched in regards to the expansion vessel, Compresso or Transfero. If not defined, Vento and
	//                           is selected alone.
	// Returns -1 if no current edition or the row where is the current selected product.
	long _FillTecBoxVentoSheet( UINT uiSDGroupID = -1, CSelectedPMBase *pclSelectedPM = NULL, CDS_SSelPMaint *pclEditedSSelPM = NULL,
			bool bForceRedForTitleBkg = false );

	// Param: 'pclSelectedPM' -> may be an expansion vessel, Compresso or Transfero. It is just to know what is the background color for the title. We set
	//                           a dark gray if Pleno is selected alone and light gray if not alone.
	void _InitTecBoxVentoHeaders( CSheetDescription *pclSDTecBoxVento, CSelectedPMBase *pclSelectedPM = NULL, bool bForceRedForTitleBkg = false );

	long _FillTecBoxVentoRow( CSheetDescription *pclSDTecBoxVento, long lRow, CSelectedVento *pclSelectedTBVento, CPMInputUser *pclPMInputUser,
			CDS_SSelPMaint *pclEditedSSelPM = NULL, CSelectedPMBase* pclSelectedBestPriceProduct = NULL, bool bForRejectedProduct = false );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PLENO
	void _VerifyPlenoSheet( CSelectedPMBase *pclSelectedProduct, UINT uiSDGroupID );
	// HYS-1121: This function will check if the refill sheet must be updated
	void _VerifyPlenoRefillSheet( UINT uiSDGroupID );

	// Allow to fill the 'Pleno' sheet when user has clicked the 'Suggest' button in the 'CDlgIndSelPM' dialog or has clicked on other tecbox.
	// Param: 'uiSDGroupID' -> ID of the group sheet description (Expansion vessel, Compresso, Transfero) or -1 if 'Pleno' is selected without pressurization.
	// Param: 'pclSelectedPM' -> if defined, Pleno is searched in regards to the expansion vessel, Compresso or Transfero. If not defined, Pleno and
	//                           is selected alone.
	// Param: bOnlyWMProtecModule : HYS-1121 if the sheet is for protection module
	// Returns -1 if no current edition or the row where is the current selected product.
	long _FillTecBoxPlenoSheet( UINT uiSDGroupID = -1, CSelectedPMBase *pclSelectedPM = NULL, bool bOnlyWMProtecModule = false, CDS_SSelPMaint *pclEditedSSelPM = NULL,
			bool bForceRedForTitleBkg = false );

	// Param: 'pclSelectedPM' -> may be an expansion vessel, Compresso or Transfero. It is just to know what is the background color for the title. We set
	//                           a dark gray if Pleno is selected alone and light gray if not alone.
	void _InitTecBoxPlenoHeaders( CSheetDescription *pclSDTecBoxPleno, CSelectedPMBase *pclSelectedPM = NULL, bool bForceRedForTitleBkg = false, bool bOnlyWMProtecModule = false );

	long _FillTecBoxPlenoRow( CSheetDescription *pclSDTecBoxPleno, long lRow, CSelectedPMBase *pclSelectedTBPleno, CPMInputUser *pclPMInputUser,
			CDS_SSelPMaint *pclEditedSSelPM = NULL, CSelectedPMBase* pclSelectedBestPriceProduct = NULL, bool bForRejectedProduct = false );

	void _PrepareTBPlenoAccessories( CDB_Set *pclWTCombination, CRank *prList, CDB_RuledTable **pclRuledTable );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PLENO REFILL

	// Allow to fill the 'Pleno Refill' sheet when user has clicked on a Pleno.
	// Param: 'uiSDGroupID' -> ID of the group sheet description.
	// Returns -1 if no current edition or the row where is the current selected product.
	long _FillPlenoRefillSheet( UINT uiSDGroupID );

	void _InitPlenoRefillHeaders( CSheetDescription *pclSDPlenoRefill );

	long _FillPlenoRefillRow( CSheetDescription *pclSDPlenoRefill, long lRow, CSelectedPMBase *pclSelectedPlenoRefill, CPMInputUser *pclPMInputUser, bool bForRejectedProduct = false );

	// HYS-1445: Statico + P BA 4 + Pleno Refill must have the WM 24 automatically selected.
	void _VerifyPlenoRefillAccessories( UINT uiSDGroupID );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Allow to add a special row where for the rejected products.
	long _AddExcludedProductsLink( CSheetDescription *pclSheetDescription, long lRow, CSelectPMList::ProductType eProductType );

	// Allow to retrieve the current group in which there is a product (Expansion vessel, Compresso or Transfero) that is selected.
	// Returns 0 if no current selected product.
	void _GetCurrentSheetDescriptionAndGroupID( UINT *puiSheetDescriptionID, UINT *puiSDGroupID );

	UINT _ConvertSD2SDG( UINT uiSDIDToConvert, UINT uiSDIDGroup );

	// Allow to remove current selection before to select a new pressure maintenance product.
	void _ClearPreviousSelectedProduct( UINT uiProductSheetDescriptionID, UINT uiProductSDGroupID );

	// Get the accessories sheet description ID list linked to the sheet containing the pressure maintenance products.
	void _GetAccessorySheetDescriptionID( UINT uiProductSheetDescriptionID, std::vector<UINT> &vecAccessorySheetDescriptionIDList );

	// Get 

	// Allow to retrieve the accessory list with the corresponding sheet description ID.
	vecCDCAccessoryList *_GetvecCDAccessoryList( UINT uiSheetDescriptionID );

	// Return associated Primary/Secondary Vessel sheet ID linked to the sheet containing the pressure maintenance products.
	// Remarks: return 0xFFFFFFFF if not found.
	int _GetVesselSheetDescriptionID( UINT uiProductSheetDescriptionID );

	// Check after which sheet we can insert the new one defined by 'uiNewSheetDescriptionID'.
	// Remarks: return 0xFFFFFFFF if not found.
	UINT _GetAfterSheetDescriptionID( UINT uiNewSheetDescriptionID, UINT uiSDGroupID );

	void _CollapseOtherGroups( UINT uiSheetDescriptionID );

	CString _OnTextTipFetchTemperatureHelper( CSelectedPMBase *pclSelectedPM, CDB_Product *pclPMProduct, CPMInputUser *pclInputUser );

	void _ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );
	void _WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter );

	void _SetPMGraphsOutputContext( bool bShowGraphs );

	// Allow us to update if needed the price index of the current selection.
	void _UpdateCurrentSelectionPriceIndex( void );
	void _ResetCurrentSelectionPriceIndex( void );

	// Check if at least one device has the vacuum degassing function.
	bool _IsAtLeastOneSelectedDeviceHasVacuumDegassingFunction( UINT uiSDGroupID );

	// Allow to get the sum of all index available in text format.
	CString _GetSumOfIndexes( int iStartIndex, int iEndIndex, CString strPrefix = _T("") );

	// Private variables.
private:
	CIndSelPMParams *m_pclIndSelPMParams;

	std::map<UINT, CCDButtonExpandCollapseRows *> m_mapButtonExpandCollapseRows;
	std::map<UINT, CCDButtonExpandCollapseGroup *> m_mapButtonExpandCollapseGroup;
	std::map<UINT, CCDButtonShowAllPriorities *> m_mapButtonShowAllPriorities;
	std::map<UINT, vecCDCAccessoryList> m_mapAccessoryList;

	LPARAM m_arlpSelectedUserParam[SD_Last];
	long m_lSelectedRow;
	long m_lProductTotalCount;
	long m_lProductNotPriorityCount;
	bool m_bShowAllPrioritiesShown;
	long m_lShowAllPrioritiesButtonRow;
	double m_dTotalHardnessOfSystem;
	bool m_bIsPMTypeAll;

	CWnd *m_pclParent;
};

extern CRViewSSelPM *pRViewSSelPM;
