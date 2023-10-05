#pragma once


#include "MyToolTipCtrl.h"
#include "ExtNumEdit.h"
#include "ExtComboBox.h"
#include "ExtStatic.h"
#include "SelectPM.h"
#include "ProductSelectionParameters.h"

class CDlgWizardPM_RightViewInput_Base : public CDialogEx, public CExtNumEdit::INotificationHandler
{
public:
	typedef enum _RightViewInputDialog
	{
		Undefined,
		ProjectType,
		HeatingTemperature,
		HeatingTemperatureAdvance,
		HeatingPressure,
		HeatingPressureAdvance,
		HeatingSystemVolumePower,
		HeatingSystemVolumePowerSWKI,
		HeatingGenerators,
		HeatingConsumers,
		HeatingPressurisationMaintenanceType,
		HeatingRequirement,
		HeatingRequirement2,
		CoolingTemperature,
		CoolingTemperatureAdvance,
		CoolingPressure,
		CoolingPressureAdvance,
		CoolingSystemVolumePower,
		CoolingSystemVolumePowerSWKI,
		CoolingGenerators,
		CoolingConsumers,
		CoolingPressurisationMaintenanceType,
		CoolingRequirement,
		CoolingRequirement2,
		SolarTemperature,
		SolarTemperatureAdvance,
		SolarPressure,
		SolarPressureAdvance,
		SolarSystemVolumePower,
		SolarSystemVolumePowerSWKI,
		SolarSystemVolumePowerSWKIAdvance,
		SolarPressurisationMaintenanceType,
		SolarRequirement,
		SolarRequirement2,
		LastElement,
	}RightViewInputDialog;

	typedef enum _ErrorCodes
	{
		Error_Temp_SafetyTempLimiter			= 0x00000001,
		Error_Temp_Supply						= ( Error_Temp_SafetyTempLimiter << 1 ),
		Error_Temp_Return						= ( Error_Temp_Supply << 1 ),
		Error_Temp_Min							= ( Error_Temp_Return << 1 ),
		Error_Temp_Max							= ( Error_Temp_Min << 1 ),
		Error_Temp_Fill							= ( Error_Temp_Max << 1 ),
		Error_Temp_WaterChar					= ( Error_Temp_Fill << 1 ),
		Error_Pressure_Pz						= ( Error_Temp_WaterChar << 1 ),
		Error_Pressure_SafetyVRP				= ( Error_Pressure_Pz << 1 ),
		Error_Pressure_PumpHead					= ( Error_Pressure_SafetyVRP << 1 ),
		Error_Pressure_WaterMakeUpNetworkPN		= ( Error_Pressure_PumpHead << 1 ),
		Error_Installation_SystemVolume			= ( Error_Pressure_WaterMakeUpNetworkPN << 1 ),
		Error_Installation_StorageTankMaxTemp	= ( Error_Installation_SystemVolume << 1 ),
		Error_Installation_SolarContent			= ( Error_Installation_StorageTankMaxTemp << 1 ),
		Error_Installation_SolarContentFactor	= ( Error_Installation_SolarContent << 1 ),
		Error_Installation_InstalledPower		= ( Error_Installation_SolarContentFactor << 1 ),
		Error_Pressure_SVLocation			= ( Error_Installation_InstalledPower << 1 ),
	}ErrorCodes;

	CDlgWizardPM_RightViewInput_Base( CWizardSelPMParams *pclWizardSelPMParams, int iRightViewInputID, UINT iDialogID, CWnd *pclParent );
	~CDlgWizardPM_RightViewInput_Base();

	// Allow to enable or disable the notification when user edit fields.
	// It helps us when we need to update value in fields but we don't want notification.
	void EnableOnEnChangeNotification( bool bEnable ) { m_bOnEnChangeEnabled = bEnable; }

	void EnableBackButton( bool bEnable );
	void EnableNextButton( bool bEnable );

	virtual void FillPMInputUser() {}

	// Called when for example the 'CDlgWizardSelPM' class receives the 'OnWaterChange' message and needs to apply
	// an update of some values in the current 'CDlgWizardPM_RightViewInput_XXX' class. It's the case when for example user 
	// changes the fluid temperature in the ribbon bar.
	// Because changes are directly applied in in the 'CPMInputUser' variable of the 'CDlgWizardSelPM' class and the
	// 'CRViewWizardSelPM' class has a pointer on this variable, and because the 'CRViewWizardSelPM' class creates the different
	// 'CDlgWizardPM_RightViewInput_XXX' classes with the same pointer, there is no need to pass the 'CPMInputUser' as argument.
	virtual void ApplyPMInputUserUpdated( bool bWaterCharUpdated = false, bool bShowErrorMsg = true ) {}

	// Allow the 'CDlgWizardSelPM' class through the 'CRViewWizardSelPM' class to know if there is an error in the input values.
	// Normally this method is called for 'CDlgWizardPM_RightViewInput_XXX' that are not visible. It happens in the case for example the
	// user is in the cooling mode and at the 'Pressures' step. If he changes the fluid temperature in the ribbon bar, 'CDlgWizardSelPM'
	// needs to validate this temperature to know if it must set the step in red (the text 'Temperatures' in red and change the icon in red cross)
	// or not.
	// Remark: this method is never called by the inherited classes. Thus if this method is not inherited, we set the two variables to -1
	// to signal that there is no verification.
	virtual int IsAtLeastOneError( int &iErrorMaskNormal, int &iErrorMaskAdvanced ) { iErrorMaskNormal = -1, iErrorMaskAdvanced = -1; return 0; }

	int GetRightViewInputID() { return m_iRightViewInputID; }

	void SetMainTitle( CString strText );

	// Overrides 'CExtNumEdit::INotificationHandler'.
	virtual void OnSpecialValidation( CWnd *pWnd, int iVirtualKey );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();

	virtual afx_msg void OnPaint();
	virtual afx_msg BOOL OnEraseBkgnd( CDC *pDC );
	virtual afx_msg void OnSize( UINT nType, int cx, int cy ) { CDialogEx::OnSize( nType, cx, cy ); }
	virtual afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual afx_msg LRESULT OnPipeChange( WPARAM wParam = 0, LPARAM lParam = 0 ) { return 0; }
	virtual afx_msg LRESULT OnTechParChange( WPARAM wParam = 0, LPARAM lParam = 0 ) { return 0; }
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 ) { return 0; }
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 ) { return 0; }

	void InitNumericalEdit( CExtNumEdit *pEdit, ePHYSTYPE phystype );
	void ResetToolTipAndErrorBorder( CExtNumEdit *pclExtEdit = NULL, bool bToolTips = true, bool bErrorBorder = true );

	virtual void GetExtEditList( CArray<CExtNumEdit*> &arExtEditList ) {}
	virtual void InitToolTips( CWnd *pWnd = NULL ) {}

	void FillComboNorm( CExtNumEditComboBox *pclComboPressNorm, CString strNormID );
	void FillComboPressOn( CExtNumEditComboBox *pclComboPressOn, PressurON ePressOn );
	void FillComboPressMaintType( CExtNumEditComboBox *pclComboPMType, CString strPressureTypeID, bool bNoNone );
	void FillComboWaterMakeUpType( CExtNumEditComboBox *pclComboWaterMakeUpType, CString strWaterMakeUpID );

	CString GetNormID( CExtNumEditComboBox *pComboNorm );
	void SetNormID( CExtNumEditComboBox *pComboNorm, CString strNormID );
	
	PressurON GetPressurOn( CExtNumEditComboBox *pComboPressOn );
	void SetPressurOn( CExtNumEditComboBox *pComboPressOn, PressurON ePressurOn );
	
	bool IsFluidTemperatureOK( CExtNumEdit *pclCurrentEdit, CString strPrefix );
	int IsWaterMakeUpNetworkPNOK( CExtNumEdit *pclCurrentEdit );
	int IsPumpHeadOK( CExtNumEdit *pclCurrentEdit, CExtNumEditComboBox *pclComboPressOn );
	int IsHeatingCoolingSystemVolumeOK( CExtNumEdit *pclCurrentEdit );
	int IsSolarSystemVolumeOK( CExtNumEdit *pclEditSystemVolume, CExtNumEdit *pclEditSolarContent );
	int IsSolarSolarContentOK( CExtNumEdit *pclEditSolarContent, CExtNumEdit *pclEditSystemVolume );
	int IsSolarSolarContentMultiplierFactorOK( CExtNumEdit *pclEditSolarContentMultiplierFactor );
	int IsStorageTankMaxTempOK( CExtNumEdit *pclEditStorageTankMaxTemp );
	int IsInstalledPowerOK( CExtNumEdit *pclInstalledPower );

	int IsHeatingSolarSafetyTempLimiterOK( CExtNumEdit *pclSftyEdit, CExtNumEditComboBox *pComboNorm = NULL, bool bShowErrorMsg = true );
	
	// HYS-1054: Fill temperature is added.
	int AreHeatingSolarTemperaturesOK( CExtNumEdit *pclSftyEdit, CExtNumEdit *pclSupplyEdit, CExtNumEdit *pclReturnEdit, CExtNumEdit *pclMinEdit, 
			CExtNumEdit *pclFillEdit, CExtNumEditComboBox *pComboNorm = NULL, bool bShowErrorMsg = true );

	// After to have verify temperatures, we can call this method to save what it is possible in the 'CPMInputUser' class.
	void UpdateOtherHeatingSolarTemperatures( int iErrorCode, CExtNumEdit *pclSftyEdit, CExtNumEdit *pclSupplyEdit = NULL, CExtNumEdit *pclReturnEdit = NULL, CExtNumEdit *pclMinEdit = NULL, 
			CExtNumEdit *pclFillEdit = NULL );

	int AreCoolingMaxSupplyReturnFillTempOK( CExtNumEdit *pclMaxEdit, CExtNumEdit *pclSupplyEdit, CExtNumEdit *pclReturnEdit, CExtNumEdit *pclFillEdit );
	int AreCoolingTemperaturesOK( CExtNumEdit *pclMaxEdit, CExtNumEdit *pclSupplyEdit, CExtNumEdit *pclReturnEdit, CExtNumEdit *pclFillEdit, CExtNumEdit *pclMinEdit, 
			INT iCheckMinTemp );
	
	// After to have verify temperatures, we can call this method to save what it is possible in the 'CPMInputUser' class.
	void UpdateOtherCoolingTemperatures( int iErrorCode, CExtNumEdit *pclMaxEdit, CExtNumEdit *pclSupplyEdit, CExtNumEdit *pclReturnEdit, CExtNumEdit *pclFillEdit,
			CExtNumEdit *pclMinEdit = NULL, INT iCheckMinTemp = 0 );

	// It's a special mode. These methods are called to check with values in 'm_pclInputUser' instead of the value in the edit field.
	// It's the case for example when user changes supply temperature in the ribbon and the 'CDlgWizardSelPM' needs to know with steps in the left dialog
	// to set in red.
	int IsHeatingSolarSafetyTempLimiterOK( void );
	int AreHeatingSolarTemperaturesOK( void );
	int AreCoolingMaxSupplyReturnFillTempOK( void );
	int AreCoolingTemperaturesOK( void );

	bool CheckPz( CButton *pBtnCheckPz, CExtNumEdit *pclPzEdit );
	bool CheckSafetyVRP( CExtNumEdit *pclEditSafetyPSV, CExtStatic *pclStaticSafetyPSVMinimum );
	bool IspSVLocationOK( CExtNumEdit *pclEditpSVLocation ); // HYS-1537

	typedef enum
	{
		Draw_AdvancedMode,
		Draw_NormalMode,
	}DrawWhichMode;
	void DrawGoAdvancedNormalModeButtton( DrawWhichMode eDrawWhichMode, CRect &rectPos );

	void VerifyFluidCharacteristic( CExtButton *pclButton );
	void VerifyFluidCharacteristicHelper( bool &bEnable, CString &strMsg );

// Protected variables.
protected:
	CWizardSelPMParams *m_pclWizardSelPMParams;
	CDS_TechnicalParameter *m_pclTechParam;
	CWnd *m_pclParent;
	int m_iRightViewInputID;
	CString m_strMainTitle;
	CFont m_clFontMaintTitle;
	CBrush m_clBrushBackground;
	CMyToolTipCtrl m_ToolTip;
	CArray<CExtNumEdit*> m_arExtEditList;
	CFont m_clGoTextFont;
	CFont m_clBackNextTextFont;
	bool m_bBackEnable;
	bool m_bNextEnable;
	CString m_strBackText;
	CString m_strNextText;
	CRect m_rectBackClickableArea;
	CRect m_rectNextClickableArea;
	bool m_bOnEnChangeEnabled;

	// Allow to save input field before user changes it.
	double m_dSafetyTempLimiterSaved;
	double m_dSupplyTempSaved;
	double m_dReturnTempSaved;
	double m_dFillTempSaved;
	double m_dMinTempSaved;
	double m_dMaxTempSaved;
	double m_dWaterMakeUpTempSaved;
	double m_dStaticHeightSaved;
	double m_dPzSaved;
	double m_dSafetyVRPSaved;
	double m_dPumpHeadSaved;
	double m_dWaterMakeUpNetworkPNSaved;
	double m_dSystemVolumeSaved;
	double m_dStorageTankVolumeSaved;
	double m_dStorageTankMaxTempSaved;
	double m_dInstalledPowerSaved;
	double m_dMaxWidthSaved;
	double m_dMaxHeightSaved;
	double m_dpSVLocationSaved; // HYS-1537
};
