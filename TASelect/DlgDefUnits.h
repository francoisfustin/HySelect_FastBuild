#pragma once


#include "afxwin.h"
#include "ExtComboBox.h"
#include "XGroupBox.h"

class CDlgDefUnits : public CDialogEx
{
public:
	enum { IDD = IDD_DLGDEFUNITS };

	CDlgDefUnits( CWnd* pParent = NULL );

	// Opens the window to modify the *pUdb unit database.
	// Returns the same value as domodal().
	int Display( CUnitDatabase *pUdb );

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelChangeDifferentialPressure();
	afx_msg void OnCbnSelChangeStaticPressure();
	afx_msg void OnCbnSelChangeLinearPressureDrop();
	afx_msg void OnCbnSelChangeFlow();
	afx_msg void OnCbnSelChangeVelocity();
	afx_msg void OnCbnSelChangeTemperature();
	afx_msg void OnCbnSelChangeTemperatureDifference();
	afx_msg void OnCbnSelChangePower();
	afx_msg void OnCbnSelChangeElecPower();
	afx_msg void OnCbnSelChangeWaterHardness();
	afx_msg void OnCbnSelChangeDensity();
	afx_msg void OnCbnSelChangeKinematicViscosity();
	afx_msg void OnCbnSelChangeSpecificHeat();
	afx_msg void OnRadioValveCoefficientKv();
	afx_msg void OnRadioValveCoefficientCv();
	afx_msg void OnCbnSelChangeLength();
	afx_msg void OnCbnSelChangeDiameter();
	afx_msg void OnCbnSelChangeRoughness();
	afx_msg void OnCbnSelChangeThickness();
	afx_msg void OnCbnSelChangeArea();
	afx_msg void OnCbnSelChangeVolume();
	afx_msg void OnCbnSelChangeMass();
	afx_msg void OnCbnSelChangeThermalConductivity();
	afx_msg void OnCbnSelChangeHeatTransfer();
	afx_msg void OnCbnSelChangeForce();
	afx_msg void OnCbnSelChangeTorque();	
	afx_msg void OnCbnSelChangeEnergy();	
	afx_msg void OnBnClickedResetToDefault();

protected:
	CExtNumEditComboBox m_ComboDiffPress;
	CExtNumEditComboBox m_ComboPressure;
	CExtNumEditComboBox m_ComboLinDp;
	CExtNumEditComboBox m_ComboFlow;
	CExtNumEditComboBox m_ComboVelocity;
	CExtNumEditComboBox m_ComboTemp;
	CExtNumEditComboBox m_ComboDiffTemp;
	CExtNumEditComboBox m_ComboPower;
	CExtNumEditComboBox m_ComboElecPower;
	CExtNumEditComboBox m_ComboWaterHardness;
	CExtNumEditComboBox m_ComboDensity;
	CExtNumEditComboBox m_ComboKinVisc;
	CExtNumEditComboBox m_ComboSpecifHeat;
	CXGroupBox m_GroupKvCv;
	CButton m_RadioWorkForControlValvesKvsValues;
	CButton m_RadioCv;
	CExtNumEditComboBox m_ComboLength;
	CExtNumEditComboBox m_ComboDiameter;
	CExtNumEditComboBox m_ComboRoughness;
	CExtNumEditComboBox m_ComboThickness;
	CExtNumEditComboBox m_ComboArea;
	CExtNumEditComboBox m_ComboVolume;
	CExtNumEditComboBox m_ComboMass;
	CExtNumEditComboBox m_ComboThermCond;
	CExtNumEditComboBox m_ComboHeatTransf;
	CExtNumEditComboBox m_ComboForce;
	CExtNumEditComboBox m_ComboTorque;
	CExtNumEditComboBox m_ComboEnergy;
	CExtNumEditComboBox m_ButtonDefault;

// Private methods.
private:
	// Fill combo box corresponding to physical type PhysType with the unit database info.
	void _FillCombo( int iPhysType );

// Private variables.
private:
	CUnitDatabase m_UnitDB;
};