#pragma once

#include <map>
#include <vector>

#define IMPORTEXPORTHM_VERSION	1.002

// Defines all node names for import/export from Json/XML format.
#define IMPORTEXPORTHM_NODE_HYDRAULIC_NETWORK								_T("hydraulic_network")			// hydraulic_network
#define IMPORTEXPORTHM_NODE_HN_VERSION										_T("version")					// hydraulic_network\version
#define IMPORTEXPORTHM_NODE_HN_DIRECTION									_T("direction")					// hydraulic_network\direction

#define IMPORTEXPORTHM_NODE_HN_PARAMETERS									_T("parameters")				// hydraulic_network\parameters
#define IMPORTEXPORTHM_NODE_HN_P_MODE										_T("mode")						// hydraulic_network\parameters\mode
#define IMPORTEXPORTHM_NODE_HN_P_FLUIDCHARACTERISTIC						_T("fluid_characteristic")		// hydraulic_network\parameters\fluid_characteristic
#define IMPORTEXPORTHM_NODE_HN_P_FC_INSUPPLYTEMPERATURE						_T("in_temperature_supply")		// hydraulic_network\parameters\fluid_characteristic\in_temperature_supply
#define IMPORTEXPORTHM_NODE_HN_P_FC_INRETURNTEMPERATURE						_T("in_temperature_return")		// hydraulic_network\parameters\fluid_characteristic\in_temperature_return
#define IMPORTEXPORTHM_NODE_HN_P_FC_INADDITIVEFAMILY						_T("in_additive_family")		// hydraulic_network\parameters\fluid_characteristic\in_additive_family
#define IMPORTEXPORTHM_NODE_HN_P_FC_INADDITIVENAME							_T("in_additive_name")			// hydraulic_network\parameters\fluid_characteristic\in_additive_name
#define IMPORTEXPORTHM_NODE_HN_P_FC_INADDITIVEWEIGHT						_T("in_additive_weight")		// hydraulic_network\parameters\fluid_characteristic\in_additive_weight

#define IMPORTEXPORTHM_NODE_HN_HYDRAULICCIRCUITLIST							_T("hydraulic_circuit_list")	// hydraulic_network\hydraulic_circuit_list
#define IMPORTEXPORTHM_NODE_HN_HCL_HYDRAULIC_CIRCUIT						_T("hydraulic_circuit")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_INNAME								_T("in_name")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\in_name
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_INTYPE								_T("in_type")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\in_type
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_INCIRCUITTYPEID						_T("in_circuit_type_id")		// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\in_circuit_type_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_INPOSITION							_T("in_position")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\in_position
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_INRETURNMODE							_T("in_return_mode")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\in_return_mode
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNINLETSUPPLY		_T("in_temperature_design_inlet_supply")		// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\in_temperature_design_inlet_supply
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNINLETRETURN		_T("in_temperature_design_inlet_return")		// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\in_temperature_design_inlet_return
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNOUTLETSUPPLY		_T("in_temperature_design_outlet_supply")		// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\in_temperature_design_outlet_supply
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_INTEMPERATUREDESIGNOUTLETRETURN		_T("in_temperature_design_outlet_return")		// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\in_temperature_design_outlet_return

#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TERMINALUNITLIST						_T("terminal_unit_list")		// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TERMINALUNIT						_T("terminal_unit")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_INEXTERNALID					_T("in_external_id")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit\terminal_unit\in_external_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_INFLOW							_T("in_flow")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit\terminal_unit\in_flow
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_TYPE						_T("type")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit\terminal_unit\in_flow\type
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_FLOW						_T("flow")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit\terminal_unit\in_flow\flow
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_POWER						_T("power")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit\terminal_unit\in_flow\power
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IF_DT							_T("dt")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit\terminal_unit\in_flow\dt
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_INPRESSUREDROP					_T("in_pressure_drop")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit\terminal_unit\in_pressure_drop
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_TYPE						_T("type")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit\terminal_unit\in_pressure_drop\type
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_PRESSUREDROP				_T("pressure_drop")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit\terminal_unit\in_pressure_drop\pressure_drop
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_FLOW_REF					_T("flow_ref")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit\terminal_unit\in_pressure_drop\flow_ref
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_PRESSURE_DROP_REF			_T("pressure_drop_ref")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit\terminal_unit\in_pressure_drop\pressure_drop_ref
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_KV							_T("kv")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit\terminal_unit\in_pressure_drop\kv
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_TUL_TU_IPD_CV							_T("cv")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\terminal_unit_list\terminal_unit\terminal_unit\in_pressure_drop\cv

#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PIPELIST								_T("pipe_list")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_PIPE								_T("pipe")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INEXTERNALID						_T("in_external_id")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\in_external_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INTYPE							_T("in_type")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\in_type
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INSIZEID							_T("in_size_id")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\in_size_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_ININTERNALDIAMETER				_T("in_internal_diameter")		// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\in_internal_diameter
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INROUGHNESS						_T("in_roughness")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\in_roughness
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INLENGTH							_T("in_length")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\in_length
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INTEMPERATURESUPPLY				_T("in_temperature_supply")		// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\in_temperature_supply
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INTEMPERATURERETURN				_T("in_temperature_return")		// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\in_temperature_return
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_INLOCKED							_T("in_locked")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\in_locked
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_ACCESSORYLIST					_T("accessory_list")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\accessory_list
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_INACCESSORYLIST				_T("in_accessory_list")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\accessory_list\in_accessory_list
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_ACCESSORY					_T("accessory")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\accessory_list\in_accessory_list\accessory
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_EXTERNALID				_T("externnal_id")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\accessory_list\in_accessory_list\accessory\external_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE					_T("type")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\accessory_list\in_accessory_list\accessory\type
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_ZETA					_T("zeta")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\accessory_list\in_accessory_list\accessory\zeta
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_PRESSUREDROP			_T("pressure_drop")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\accessory_list\in_accessory_list\accessory\pressure_drop
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_FLOWREF					_T("flow_ref")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\accessory_list\in_accessory_list\accessory\flow_ref
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_PRESSUREDROPREF			_T("pressure_drop_ref")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\accessory_list\in_accessory_list\accessory\pressure_drop_ref
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_KV						_T("kv")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\accessory_list\in_accessory_list\accessory\kv
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PL_P_AL_IAL_A_CV						_T("cv")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pipe_list\pipe\accessory_list\in_accessory_list\accessory\cv

#define IMPORTEXPORTHM_NODE_HN_HCL_HC_BALANCINGVALVELIST					_T("balancing_valve_list")		// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\balancing_valve_list
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BALANCINGVALVE					_T("balancing_valve")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\balancing_valve_list\balancing_valve
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INEXTERNALID					_T("in_external_id")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\balancing_valve_list\balancing_valve\in_external_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INLOCATION						_T("in_location")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\balancing_valve_list\balancing_valve\in_location
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INSIZEID						_T("in_size_id")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\balancing_valve_list\balancing_valve\in_size_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INFLOW							_T("in_flow")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\balancing_valve_list\balancing_valve\in_flow
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_BVL_BV_INLOCKED						_T("in_locked")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\balancing_valve_list\balancing_valve\in_locked

#define IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCONTROLLERVALVELIST					_T("dp_controller_valve_list")	// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\dp_controller_valve_list
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCONTROLLERVALVE				_T("dp_controller_valve")		// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\dp_controller_valve_list\dp_controller_valve
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INEXTERNALID				_T("in_external_id")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\dp_controller_valve_list\dp_controller_valve\in_external_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INSIZEID					_T("in_size_id")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\dp_controller_valve_list\dp_controller_valve\in_size_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INFLOW						_T("in_flow")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\dp_controller_valve_list\dp_controller_valve\in_flow
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_DPCVL_DPCV_INLOCKED					_T("in_locked")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\dp_controller_valve_list\dp_controller_valve\in_locked

#define IMPORTEXPORTHM_NODE_HN_HCL_HC_CONTROLVALVELIST						_T("control_valve_list")		// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\control_valve_list
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CONTROLVALVE						_T("control_valve")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\control_valve_list\control_valve
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INEXTERNALID					_T("in_external_id")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\control_valve_list\control_valve\in_external_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INCONTROLTYPE					_T("in_control_type")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\control_valve_list\control_valve\in_control_type
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INSIZEID						_T("in_size_id")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\control_valve_list\control_valve\in_size_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INFLOW							_T("in_flow")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\control_valve_list\control_valve\in_flow
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_CVL_CV_INLOCKED						_T("in_locked")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\control_valve_list\control_valve\in_locked

#define IMPORTEXPORTHM_NODE_HN_HCL_HC_SHUTOFFVALVELIST						_T("shutoff_valve_list")		// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\shutoff_valve_list
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SHUTOFFVALVE						_T("shutoff_valve")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\shutoff_valve_list\shutoff_valve
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INEXTERNALID					_T("in_external_id")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\shutoff_valve_list\shutoff_valve\in_external_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INSIZEID						_T("in_size_id")				// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\shutoff_valve_list\shutoff_valve\in_size_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INFLOW							_T("in_flow")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\shutoff_valve_list\shutoff_valve\in_flow
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_SVL_SV_INLOCKED						_T("in_locked")					// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\shutoff_valve_list\shutoff_valve\in_locked

#define IMPORTEXPORTHM_NODE_HN_HCL_HC_PUMP									_T("pump")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pump
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_P_INEXTERNALID						_T("in_external_id")			// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pump\in_external_id
#define IMPORTEXPORTHM_NODE_HN_HCL_HC_P_HMIN								_T("hmin")						// hydraulic_network\hydraulic_circuit_list\hydraulic_circuit\pump\hmin

// Defines all node values for import/export from Json/XML format.
#define IMPORTEXPORTHM_VALUE_HN_P_MODE_HEATING								_T("heating")
#define IMPORTEXPORTHM_VALUE_HN_P_MODE_COOLING								_T("cooling")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_TYPE_CIRCUIT							_T("circuit")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_TYPE_MODULE							_T("module")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_RETURNMODE_DIRECT					_T("direct")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_RETURNMODE_REVERSE					_T("reverse")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IF_TYPE_FLOW					_T("flow")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IF_TYPE_POWERDT				_T("power_dt")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_PRESSUREDROP			_T("dp")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_QREFDPREF			_T("qref_dpref")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_KV					_T("kv")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_TUL_TU_IPD_TYPE_CV					_T("cv")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_DISTRIBUTIONSUPPLY		_T("distribution_supply")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_DISTRIBUTIONRETURN		_T("distribution_return")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_CIRCUITPRIMARY			_T("circuit_primary")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_INTYPE_CIRCUITSECONDARY			_T("circuit_secondary")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_ELBOW				_T("elbow")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_TEE				_T("tee")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_ZETA				_T("zeta")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_DP				_T("dp")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_QREFDPREF			_T("qref_dpref")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_KV				_T("kv")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_PL_P_AL_IAL_A_TYPE_CV				_T("cv")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_PRIMARY			_T("primary")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_SECONDARY			_T("secondary")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_BVL_BV_INLOCATION_BYPASS				_T("bypass")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_CVL_CV_INCONTROLTYPE_ONOFF			_T("on_off")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_CVL_CV_INCONTROLTYPE_3POINTS			_T("3-points")
#define IMPORTEXPORTHM_VALUE_HN_HCL_HC_CVL_CV_INCONTROLTYPE_PROPORTIONAL	_T("proportional")


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class that will contain all the data from the import/export file.
class CImportExportHMData
{
public:
	CImportExportHMData() {}
	~CImportExportHMData() {}
	
	class CHydraulicCircuitData;
	class CPumpData
	{
	public:
		CPumpData() { m_bExist = false; m_strInExternalID = _T(""); m_dHMin = 0.0; m_pclHydraulicCircuitParent = NULL; }

		bool m_bExist;
		CString m_strInExternalID;
		double m_dHMin;
		CHydraulicCircuitData *m_pclHydraulicCircuitParent;
	};

	class CShutoffValveData
	{
	public:
		CShutoffValveData() { Reset(); }
		void Reset() { m_strInExternalID = _T(""); m_strInSizeID = _T(""); m_dFlow = 0.0; m_bInIsLocked = false; m_pclHydraulicCircuitParent = NULL; }

		CString m_strInExternalID;
		CString m_strInSizeID;
		double m_dFlow;
		bool m_bInIsLocked;
		CHydraulicCircuitData *m_pclHydraulicCircuitParent;
	};

	class CControlValveData
	{
	public:
		CControlValveData() { Reset(); }
		void Reset() { m_strInExternalID = _T(""); m_eControlType = CDB_ControlProperties::CvCtrlType::eCvNU; m_strInSizeID = _T(""); m_dFlow = 0.0; 
				m_bInIsLocked = false; m_pclHydraulicCircuitParent = NULL; }

		CString m_strInExternalID;
		CDB_ControlProperties::CvCtrlType m_eControlType;
		CString m_strInSizeID;
		double m_dFlow;
		bool m_bInIsLocked;
		CHydraulicCircuitData *m_pclHydraulicCircuitParent;
	};

	class CDpControllerValveData
	{
	public:
		CDpControllerValveData() { Reset(); }
		void Reset() { m_strInExternalID = _T(""); m_strInSizeID = _T(""); m_dFlow = 0.0; m_bInIsLocked = false; m_pclHydraulicCircuitParent = NULL; }

		CString m_strInExternalID;
		CString m_strInSizeID;
		double m_dFlow;
		bool m_bInIsLocked;
		CHydraulicCircuitData *m_pclHydraulicCircuitParent;
	};

	class CBalancingValveData
	{
	public:
		CBalancingValveData() { Reset(); }
		void Reset() { m_strInExternalID = _T(""); m_eInLocation = CDS_HydroMod::eHMObj::eNone; m_strInSizeID = _T(""); m_dFlow = 0.0; 
				m_bInIsLocked = false; m_pclHydraulicCircuitParent = NULL; }

		CString m_strInExternalID;
		CDS_HydroMod::eHMObj m_eInLocation;
		CString m_strInSizeID;
		double m_dFlow;
		bool m_bInIsLocked;
		CHydraulicCircuitData *m_pclHydraulicCircuitParent;
	};

	class CPipeData;
	class CPipeAccessoryData
	{
	public:
		CPipeAccessoryData() { Reset(); }
		void Reset() { m_strInExternalID = _T(""); m_eType = CDS_HydroMod::eDpType::None; m_bIsElbow = false; m_dZetaValue = 0.0; m_dPressureDrop = 0.0; 
				m_dFlowRef = 0.0; m_dPressureDropRef = 0.0; m_dKv = 0.0; m_dCv = 0.0; m_pclPipeParent = NULL; }

		CString m_strInExternalID;
		CDS_HydroMod::eDpType m_eType;
		bool m_bIsElbow;					// If 'm_eType' is none, 'bIsElbow' is 'true' for an elbow otherwise if 'false' for a tee.
		double m_dZetaValue;
		double m_dPressureDrop;
		double m_dFlowRef;
		double m_dPressureDropRef;
		double m_dKv;
		double m_dCv;
		CPipeData *m_pclPipeParent;
	};

	class CPipeData
	{
	public:
		CPipeData() { Reset(); }
		~CPipeData() { Reset(); }
		void Reset();

		CString m_strInExternalID;
		CDS_HydroMod::eHMObj m_eInType;
		CString m_strInSizeID;
		double m_dInInternalDiameter;
		double m_dInRoughness;
		double m_dInLength;
		double m_dSupplyTemperature;
		double m_dReturnTemperature;
		bool m_bInIsLocked;
		std::vector<CPipeAccessoryData*> m_vecpInPipeAccessoryList;
		CHydraulicCircuitData *m_pclHydraulicCircuitParent;
	};

	class CTerminalUnitData
	{
	public:
		CTerminalUnitData() { Reset(); }
		void Reset() { m_strInExternalID = _T(""); m_eFlowType = CTermUnit::_QType::Undefined ; m_dFlow = 0.0; m_dPower = 0.0; m_dDT = 0.0; 
				m_ePressureDropType = CDS_HydroMod::eDpType::None; m_dPressureDrop = 0.0; m_dFlowRef = 0.0; m_dPressureDropRef = 0.0; m_dKv = 0.0; 
				m_dCv = 0.0; m_pclHydraulicCircuitParent = NULL; }

		CString m_strInExternalID;
		CTermUnit::_QType m_eFlowType;
		double m_dFlow;
		double m_dPower;
		double m_dDT;
		CDS_HydroMod::eDpType m_ePressureDropType;
		double m_dPressureDrop;
		double m_dFlowRef;
		double m_dPressureDropRef;
		double m_dKv;
		double m_dCv;
		CHydraulicCircuitData *m_pclHydraulicCircuitParent;
	};

	class CHydraulicCircuitListData;
	class CHydraulicNetworkData;
	class CHydraulicCircuitData
	{
	public:
		CHydraulicCircuitData() { m_pclHydraulicChildrenCircuitList = NULL; Reset(); } 
		~CHydraulicCircuitData() { Reset(); }
		void Reset();

		CString m_strInName;
		bool m_bInModule;
		CDB_CircuitScheme *m_pclCircuitScheme;
		int m_iInPosition;
		CDS_HydroMod::ReturnType m_eInReturnMode;
		double m_dTemperatureDesignInletSupply;
		double m_dTemperatureDesignInletReturn;
		double m_dTemperatureDesignOutletSupply;
		double m_dTemperatureDesignOutletReturn;
		std::vector<CTerminalUnitData *> m_vecpTerminalUnitList;
		std::vector<CPipeData *> m_vecpPipeList;
		std::vector<CBalancingValveData *> m_vecpBalancingValveList;
		std::vector<CDpControllerValveData *> m_vecpDpControllerValveList;
		std::vector<CControlValveData *> m_vecpControlValveList;
		std::vector<CShutoffValveData *> m_vecpShutoffValveList;
		CPumpData m_Pump;
		CHydraulicCircuitListData *m_pclHydraulicChildrenCircuitList;
		CHydraulicCircuitListData *m_pclHydraulicCircuitParent;
		CHydraulicNetworkData *m_pclHydraulicNetworkData;
	};

	class CHydraulicCircuitListData
	{
	public:
		CHydraulicCircuitListData() { m_pclHydraulicCircuitParent = NULL; m_pclHydraulicNetworkData = NULL; }
		~CHydraulicCircuitListData() { Reset(); }
		void Reset();
		
		std::vector<CHydraulicCircuitData *> m_vecpHydraulicCircuitList;
		CHydraulicCircuitListData *m_pclHydraulicCircuitParent;
		CHydraulicNetworkData *m_pclHydraulicNetworkData;
	};

	class CHydraulicNetworkData;
	class CParametersData
	{
	public:
		CParametersData() { Reset(); }
		void Reset() { m_eMode = ProjectType::InvalidProjectType; m_dReturnTemperature = 0.0; m_pclHydraulicNetworkData = NULL; }

		ProjectType m_eMode;
		CWaterChar m_WC;
		double m_dReturnTemperature;
		CHydraulicNetworkData *m_pclHydraulicNetworkData;
	};

	class CHydraulicNetworkData
	{
	public:
		CHydraulicNetworkData() { m_dVersion = 0.0; m_strDirection = _T(""); }
		void Reset() { m_clParameters.Reset(); m_clHydraulicCircuitList.Reset(); }

		double m_dVersion;
		CString m_strDirection;
		CParametersData m_clParameters;
		CHydraulicCircuitListData m_clHydraulicCircuitList;
	};

	CHydraulicNetworkData m_clHydraulicNetwork;
};
