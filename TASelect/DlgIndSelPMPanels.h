#pragma once


#include "DialogExt.h"
#include "MyToolTipCtrl.h"
#include "XGroupBox.h"
#include "ExtNumEdit.h"
#include "ExtStatic.h"
#include "afxwin.h"
#include "ExtComboBox.h"

#define DLGINDSELPMINTERFACE_GETLINKEDRIGHTVIEW( pInterface ) ( ( *pInterface->m_pclParent.*pInterface->m_pfnGetLinkedRightView )( ) )
#define DLGINDSELPMINTERFACE_CLEARALL( pInterface ) ( ( *pInterface->m_pclParent.*pInterface->m_pfnClearAll )( ) )

class CDlgIndSelPMPanelBase : public CDialogExt, public CDlgSelectionTabHelper, public CExtNumEdit::INotificationHandler
{
public:
	typedef enum 
	{
		ePanelFirst				= ProjectType::Heating,
		ePanelHeating			= ePanelFirst,
		ePanelCooling			= ProjectType::Cooling,
		ePanelSolar				= ProjectType::Solar,
		ePanelHeatingSWKI		= ePanelSolar + 1,
		ePanelCoolingSWKI		= ePanelHeatingSWKI + 1,
		ePanelSolarSWKI			= ePanelCoolingSWKI + 1,
		ePanelNoPressMaint		= ePanelSolarSWKI + 1,
		ePanelLast
	}PanelType;

	CDlgIndSelPMPanelBase( CIndSelPMParams *pclIndSelParams, PanelType ePanelType, UINT nID = 0, CWnd *pParent = NULL );
	virtual ~CDlgIndSelPMPanelBase() {}

	PanelType GetPanelType() { return m_ePanelType; }

	void SetInterface( LPARAM pclInterface ) { m_pclInterface = pclInterface; }

	// Allow to enable or disable the notification when user edit fields.
	// It helps us when we need to update value in fields but we don't want notification.
	void EnableOnEnChangeNotification( bool bEnable ) { m_bOnEnChangeEnabled = bEnable; }

	virtual void OnNormChanged() {}

	virtual void EnableDegassing( bool bEnable, bool bWaterMakupChecked );
	virtual void EnableWaterMakeUp( bool bEnable, bool bDegassingChecked );
	
	virtual void SetPz( double dPz ) {}
	virtual void SetPressurOn( PressurON ePressurON ) {}
	virtual void SetPumpHead( double dPumpHead ) {}
	virtual void SetMaxWidth( double dMaxWidth ) {}
	virtual void SetMaxHeight( double dMaxHeight ) {}

	virtual void SaveSelectionParameters() {}

	// Copy 'pclPMInputUser' into the input fields.
	// Param: 'bDontShowErrorMessage': the 'VerifyPMInputUser' method is internally called. This variable specify
	//        if we can show a popup window with message if there are errors.
	virtual bool ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Param: 'bDontShowErrorMessage': This variable specify if we can show a popup window with message if there are errors.
	virtual bool VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bDontShowErrorMessage );

	virtual void OnPMTypeChanged( CPMInputUser *pclPMInputUser ) {}

	// Not called in reaction to the 'WM_USER_NEWDOCUMENT' message. Because we need that 'OnNewDocument' of the
	// 'DlgIndSekPressureMaintenance' be called first. In this method, we call 'ApplyPMInputUser' that fill inputs
	// and validate them with sometimes by setting some field with a red border to signal an error.
	// If 'OnNewDocument' here is called after, we reset the tooltip and errors.
	virtual void OnNewDocument();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'DlgSelectionTabHelper' public virtual methods.
	// This method is called when left tab dialog has no more the focus while user presses TAB or SHIFT+TAB.
	// Param: if 'fNext' is 'true' that means user presses 'TAB', otherwise it's 'SHIFT+TAB'.
	// Remark: for the pressure maintenance panel, it's a particular case. We have the 'CDlgIndSelPM' inherited directly from 'CDlgSelectionBase'.
	//         In this dialog, we have just a few controls common to all application types (heating, cooling and solar). Below these controls, there
	//         is one other dialog (CDlgIndSelPMPanels), one dialog for one application type. 
	//         At start, the focus is set on the control in the 'CDlgIndSelPM'. If user presses and reaches the last control, we don't want to go
	//         in the right view but first in the 'CDlgIndSelPMPanels'. When last control is reached in this dialog while pressing TAB key, then we can go
	//         to the right view.
	
	virtual bool LeftTabKillFocus( bool fNext );
	
	// End of overriding 'DlgSelectionTabHelper' public virtual methods.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	// Overrides 'CExtNumEdit::INotificationHandler'.
	virtual void OnSpecialValidation( CWnd *pWnd, int iVirtualKey );
	// HYS-1041: Disabled ENTER key
	virtual void EnablePMSelection( bool Enable );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK() { /* DO NOTHING */ }
	virtual void OnCancel() { /* DO NOTHING */ }
	virtual BOOL PreTranslateMessage( MSG *pMsg );
	virtual afx_msg LRESULT OnPipeChange( WPARAM wParam = 0, LPARAM lParam = 0 ) { return 0; }
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 ) { return 0; }

	virtual afx_msg void OnEnChangeEdit();

	virtual afx_msg void OnEnSetFocusDegassingMaxTemp();
	virtual afx_msg void OnEnSetFocusWaterMakeUpNetworkPN();
	virtual afx_msg void OnEnSetFocusWaterMakeUpWaterTemp();

	virtual afx_msg void OnKillFocusDegassingMaxTemp();
	virtual afx_msg void OnKillFocusWaterMakeUpNetworkPN();
	virtual afx_msg void OnKillFocusWaterMakeUpWaterTemp();

	void InitNumericalEdit( CExtNumEdit *pEdit, ePHYSTYPE phystype );
	void ClearRightSheet();
	virtual void ResetToolTipsErrorBorder( CWnd *pWnd = NULL, bool bToolTips = true, bool bErrorBorder = true );
	virtual void InitToolTips( CWnd* pWnd = NULL ) {}

	enum MapErrorID
	{
		MEID_Undefined				= 0x0000,
		MEID_Pz						= 0x0001,
		MEID_WaterContent			= 0x0002,
		MEID_SolarContent			= 0x0004,
		MEID_InstalledPower			= 0x0008,
		MEID_PSVLocation			= 0x0010,
		MEID_SafetyPressureValve	= 0x0020,
		MEID_SafetyTempLimiter		= 0x0040,
		MEID_MaxTemperature			= 0x0080,
		MEID_SupplyTemperature		= 0x0100,
		MEID_ReturnTemperature		= 0x0200,
		MEID_MinTemperature			= 0x0400,
		MEID_FillTemperature		= 0x0800,
		MEID_PumpHead				= 0x100,
		MEID_DegassingMaxTemp		= 0x2000,
		MEID_WaterMakeUpPSN			= 0x4000,
		MEID_WaterMakeUpMaxTemp		= 0x8000,
	};

	bool IsFluidTemperatureOK( CExtNumEdit *pclCurrentEdit, CString strPrefix, bool bInsertInErrorMap = false, MapErrorID eMapErrorID = MEID_Undefined );
	bool IsDegassingMaxTempOK( bool bInsertInErrorMap = false );
	bool IsWaterMakeUpTempOK( bool bInsertInErrorMap = false );

// protected variables.
protected:
	CIndSelPMParams *m_pclIndSelPMParams;
	PanelType m_ePanelType;
	LPARAM m_pclInterface;
	
	bool m_bOnEnChangeEnabled;
	CXGroupBox m_GroupDegassing;
	CExtNumEdit m_EditDegassingMaxTemp;
	CXGroupBox m_GroupWaterMakeUp;
	CExtNumEdit m_EditWaterMakeUpNetworkPN;
	CExtNumEdit m_EditWaterMakeUpWaterTemp;
	CMyToolTipCtrl m_ToolTip;
	CRect m_OrgClient;
	CRect m_OrgPosGroupDegassing;
	CRect m_OrgPosGroupWaterMakeUp;
	double m_dDegassingMaxTempSaved;
	double m_dWaterMakeUpNetworkPNSaved;
	double m_dWaterMakeUpTempSaved;

	// For error message.
	std::vector<std::pair<int,int>> m_vecErrorList;
	std::map<int, CString> m_mapErrorMsg;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelHCSBase - Common to Heating / Cooling / Solar.
class CDlgIndSelPMPanelHCSBase : public CDlgIndSelPMPanelBase
{
public:
	CDlgIndSelPMPanelHCSBase( CIndSelPMParams *pclIndSelParams, PanelType ePanelType, UINT nID = 0, CWnd *pParent = NULL );
	virtual ~CDlgIndSelPMPanelHCSBase() {}
	
	// Overrides 'CDlgIndSelPMPanels' public virtual methods.
	virtual void SaveSelectionParameters();

	virtual void SetPz( double dPz );
	virtual void SetPressurOn( PressurON ePressurON );
	virtual void SetPumpHead( double dPumpHead );
	virtual void SetMaxWidth( double dMaxWidth );
	virtual void SetMaxHeight( double dMaxHeight );

	PressurON GetPressurOn( void );

	// Copy 'pclPMInputUser' into the input fields.
	// Param: 'bDontShowErrorMessage': the 'VerifyPMInputUser' method is internally called. This variable specify
	//        if we can show a popup window with message if there are errors.
	virtual bool ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Param: 'bDontShowErrorMessage': This variable specify if we can show a popup window with message if there are errors.
	virtual bool VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	virtual void OnPMTypeChanged( CPMInputUser *pclPMInputUser );

	// Not called in reaction to the 'WM_USER_NEWDOCUMENT' message. Because we need that 'OnNewDocument' of the
	// 'DlgIndSekPressureMaintenance' be called first. In this method, we call 'ApplyPMInputUser' that fill inputs
	// and validate them with sometimes by setting some field with a red border to signal an error.
	// If 'OnNewDocument' here is called after, we reset the tooltip and errors.
	virtual void OnNewDocument();

	// HYS-1041: Disabled ENTER key
	virtual void EnablePMSelection( bool Enable );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	virtual afx_msg LRESULT OnWaterChange( WPARAM wParam = 0, LPARAM lParam = 0 );

	virtual afx_msg void OnCbnSelChangePressOn();

	virtual afx_msg void OnEnSetFocusStaticHeight();
	virtual afx_msg void OnEnSetFocusPz();
	virtual afx_msg void OnEnSetFocusSystemVolume();
	virtual afx_msg void OnEnSetFocusInstalledPower();

	// HYS-1083: We now have also the possibility to put the height difference between the pSV and the vessel location.
	virtual afx_msg void OnEnSetFocuspSVLocation();

	virtual afx_msg void OnEnSetFocusSafetyVRP();
	virtual afx_msg void OnEnSetFocusSupplyTemp();
	virtual afx_msg void OnEnSetFocusReturnTemp();
	virtual afx_msg void OnEnSetFocusMinTemp();
	
	// HYS-1054: fill temperature field is now in common with the cooling, heating, solar mode
	virtual afx_msg void OnEnSetFocusFillTemp();
	
	virtual afx_msg void OnEnSetFocusPumpHead();
	virtual afx_msg void OnEnSetFocusMaxWidth();
	virtual afx_msg void OnEnSetFocusMaxHeight();

	virtual afx_msg void OnEnChangeStaticHeight();
	virtual afx_msg void OnEnChangeInstalledPower();
	virtual afx_msg void OnEnChangePumpHead();

	virtual afx_msg void OnKillFocusStaticHeight();
	virtual afx_msg void OnKillFocusPz();
	virtual afx_msg void OnKillFocusSystemVolume();
	virtual afx_msg void OnKillFocusInstalledPower();
	virtual afx_msg void OnKillFocuspSVLocation();				// HYS-1083.
	virtual afx_msg void OnKillFocusSafetyVRP();
	virtual afx_msg void OnKillFocusPumpHead();
	virtual afx_msg void OnKillFocusMaxWidth();
	virtual afx_msg void OnKillFocusMaxHeight();

	virtual afx_msg void OnBnClickedCheckPz();
	virtual afx_msg void OnBnClickedCheckpSVLocation();			// HYS-1083.

	virtual void ResetToolTipsErrorBorder( CWnd *pWnd = NULL, bool bToolTips = true, bool bErrorBorder = true );
	virtual void InitToolTips( CWnd *pWnd = NULL );
	virtual void UpdateP0();
	virtual void UpdateVolumeSystemStatus( BOOL bEnableSysVol, BOOL bEnableInstPower ) {}

	bool CheckPz();
	bool CheckSafetyVRP();

	void FillComboPressOn( PressurON ePressOn = PressurON::poPumpSuction );

	// HYS-933: replace &= by |= to activate error flag. Also activate error flag when bMinBadFluidError is true and bMaxBadFluidError = true
	// HYS-1054: Fill temperature is added, fill temperature field is now in common with the cooling, heating, solar mode
	UINT AreHeatingSolarTemperaturesOK( CExtNumEdit *pclSftyEdit, CExtNumEdit *pclSupplyEdit, CExtNumEdit *pclReturnEdit, CExtNumEdit *pclMinEdit, CExtNumEdit *pclFillEdit, bool bInsertInErrorMap = false );
	UINT AreCoolingTemperaturesOK( CExtNumEdit *pclMaxEdit, CExtNumEdit *pclSupplyEdit, CExtNumEdit *pclReturnEdit, CExtNumEdit *pclMinEdit, INT iCheckMinTemp, CExtNumEdit *pclFillEdit, bool bInsertInErrorMap = false );

	UINT IsInstalledPowerOK( CExtNumEdit *pclInstalledPower, bool bInsertInErrorMap = false );

	// HYS-1083.
	UINT IspSVLocationOK( CExtNumEdit *pclpSVLocation, bool bInsertInErrorMap = false );

	void UpdateOtherHeatingSolarTemperatures( UINT uiError, CExtNumEdit *pclSftyEdit, CExtNumEdit *pclSupplyEdit, CExtNumEdit *pclReturnEdit, CExtNumEdit *pclMinEdit, CExtNumEdit *pclFillEdit );
	void UpdateOtherCoolingTemperatures( UINT uiError, CExtNumEdit *pclMaxEdit, CExtNumEdit *pclSupplyEdit, CExtNumEdit *pclReturnEdit, CExtNumEdit *pclMinEdit, INT iCheckMinTemp, CExtNumEdit *pclFillEdit );

	virtual double GetSafetyTempLimiter() = 0;

// Protected variables.
protected:

	CButton m_BtnWaterContent;
	CButton m_BtnCheckPz;
	CButton m_BtnCheckpSVLocation;		// HYS-1083

	CExtStatic m_StaticStaticHeight;
	CExtStatic m_StaticP0;
	CExtStatic m_StaticPz;
	CExtStatic m_StaticSystemVolume;
	CExtStatic m_StaticInstalledPower;
	CExtStatic m_StaticpSVLocation;		// HYS-1083
	CExtStatic m_StaticSafetyValveResponsePressure;
	CExtStatic m_StaticMinPSV;
	CExtStatic m_StaticSupplyTemp;
	CExtStatic m_StaticReturnTemp;
	CExtStatic m_StaticMinTemp;
	CExtStatic m_StaticFillTemp;
	CExtStatic m_StaticPressureOn;
	CExtStatic m_StaticPumpHead;
	CExtStatic m_StaticMaxWidth;
	CExtStatic m_StaticMaxHeight;

	CExtNumEdit m_EditStaticHeight;
	CExtNumEdit m_EditPz;
	CExtNumEdit m_EditWaterContent;
	CExtNumEdit m_EditInstalledPower;
	CExtNumEdit m_EditpSVLocation;			// HYS-1083.
	CExtNumEdit m_EditSafetyValveResponsePressure;
	CExtNumEdit m_EditSupplyTemp;			// Common to H/C/S
	CExtNumEdit m_EditReturnTemp;			// Common to H/C/S
	CExtNumEdit m_EditMinTemp;				// Common to H/C/S
	CExtNumEdit m_EditFillTemp;			// Common to H/C/S
	CExtNumEditComboBox m_ComboPressOn;
	CExtNumEdit m_EditPumpHead;
	CExtNumEdit m_EditMaxWidth;
	CExtNumEdit m_EditMaxHeight;

	double m_dStaticHeightSaved;
	double m_dPzSaved;
	double m_dSystemVolumeSaved;
	double m_dInstalledPowerSaved;
	double m_dSafetyValveResponsePressureSaved;
	double m_dpSVLocationSaved;
	double m_dSupplyTempSaved;
	double m_dReturnTempSaved;
	double m_dMinTempSaved;
	double m_dFillTempSaved;
	double m_dPumpHeadSaved;
	double m_dMaxWidthSaved;
	double m_dMaxHeightSaved;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelHeatingBase dialog
// HYS-1534: Base class for heating. For the moment, we have a common heating class for all norm (CDlgIndSelPMPanelHeating)
//           and a specific one for the SWKI HE301-01 norm (CDlgIndSelPMPanelHeatingSWKI).
class CDlgIndSelPMPanelHeatingBase : public CDlgIndSelPMPanelHCSBase
{
public:
	CDlgIndSelPMPanelHeatingBase( CIndSelPMParams *pclIndSelParams, PanelType ePanelType, UINT nID = 0, CWnd *pParent = NULL );
	virtual ~CDlgIndSelPMPanelHeatingBase() {}

	virtual void OnNormChanged();

	// Copy 'pclPMInputUser' into the input fields.
	// Param: 'bDontShowErrorMessage': the 'VerifyPMInputUser' method is internally called. This variable specify
	//        if we can show a popup window with message if there are errors.
	virtual bool ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Param: 'bDontShowErrorMessage': This variable specify if we can show a popup window with message if there are errors.
	virtual bool VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );
	
	// Not called in reaction to the 'WM_USER_NEWDOCUMENT' message. Because we need that 'OnNewDocument' of the
	// 'DlgIndSekPressureMaintenance' be called first. In this method, we call 'ApplyPMInputUser' that fill inputs
	// and validate them with sometimes by setting some field with a red border to signal an error.
	// If 'OnNewDocument' here is called after, we reset the tooltip and errors.
	virtual void OnNewDocument();

	// HYS-1041: Disabled ENTER key.
	virtual void EnablePMSelection( bool bEnable );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );

	virtual afx_msg void OnBnClickedWaterContent();

	virtual afx_msg void OnEnSetFocusSafetyTempLimiter();
	virtual afx_msg void OnEnSetFocusWaterHardness();

	virtual afx_msg void OnEnChangeSafetyTempLimiter();

	virtual afx_msg void OnKillFocusEditSafetyTempLimiter();
	virtual afx_msg void OnKillFocusEditSupplyTemp();
	virtual afx_msg void OnKillFocusEditReturnTemp();
	virtual afx_msg void OnKillFocusEditMinTemp();
	virtual afx_msg void OnKillFocusEditFillTemp();
	virtual afx_msg void OnKillFocusEditWaterHardness();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelPMPanelBase' protected virtual methods.
	virtual void ResetToolTipsErrorBorder( CWnd *pWnd = NULL, bool bToolTips = true, bool bErrorBorder = true );
	virtual void UpdateVolumeSystemStatus( BOOL bEnableSysVol, BOOL bEnableInstPower );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelPMPanelHCSBase' protected pure virtual methods.
	virtual double GetSafetyTempLimiter() { return m_EditSafetyTLim.GetCurrentValSI(); }
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	void _UpdateInstalledPowerWaterContent( void );

// Private variables.
private:
	CExtStatic m_StaticSafetyTLim;
	CExtNumEdit m_EditSafetyTLim;
	CExtNumEdit m_EditWaterHardness;
	double m_dSafetyTempLimiterSaved;
	double m_dWaterHardnessSaved;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelHeating dialog
// HYS-1534: Common heating class for all norm.
class CDlgIndSelPMPanelHeating : public CDlgIndSelPMPanelHeatingBase
{
public:
	enum { IDD = IDD_DLGINDSELPRESSMAINTHEATING };
	CDlgIndSelPMPanelHeating( CIndSelPMParams *pclIndSelParams, CWnd *pParent = NULL );
	virtual ~CDlgIndSelPMPanelHeating() {}

	// Copy 'pclPMInputUser' into the input fields.
	// Param: 'bDontShowErrorMessage': the 'VerifyPMInputUser' method is internally called. This variable specify
	//        if we can show a popup window with message if there are errors.
	virtual bool ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Param: 'bDontShowErrorMessage': This variable specify if we can show a popup window with message if there are errors.
	virtual bool VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelHeatingSWKI dialog
// HYS-1534: specific heating class for the SWKI HE301-01 norm.
class CDlgIndSelPMPanelHeatingSWKI : public CDlgIndSelPMPanelHeatingBase
{
public:
	enum { IDD = IDD_DLGINDSELPRESSMAINTHEATINGSWKI };
	CDlgIndSelPMPanelHeatingSWKI( CIndSelPMParams *pclIndSelParams, CWnd *pParent = NULL );
	virtual ~CDlgIndSelPMPanelHeatingSWKI() {}

	// Copy 'pclPMInputUser' into the input fields.
	// Param: 'bDontShowErrorMessage': the 'VerifyPMInputUser' method is internally called. This variable specify
	//        if we can show a popup window with message if there are errors.
	virtual bool ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Param: 'bDontShowErrorMessage': This variable specify if we can show a popup window with message if there are errors.
	virtual bool VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Not called in reaction to the 'WM_USER_NEWDOCUMENT' message. Because we need that 'OnNewDocument' of the
	// 'DlgIndSekPressureMaintenance' be called first. In this method, we call 'ApplyPMInputUser' that fill inputs
	// and validate them with sometimes by setting some field with a red border to signal an error.
	// If 'OnNewDocument' here is called after, we reset the tooltip and errors.
	virtual void OnNewDocument();

	// HYS-1041: Disabled ENTER key
	virtual void EnablePMSelection( bool bEnable );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );

	virtual afx_msg void OnBnClickedStorageTankMaxTemp();
	virtual afx_msg void OnEnSetFocusStorageTankVolume();
	virtual afx_msg void OnKillFocusStorageTankVolume();

// Protected variables.
protected:
	CButton m_BtnStorageTankMaxTemp;
	CExtStatic m_StaticStorageTankVolume;
	CExtNumEdit m_EditStorageTankVolume;
	double m_dStorageTankVolumeSaved;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelCooling dialog
// HYS-1534: Base class for cooling. For the moment, we have a common cooling class for all norm (CDlgIndSelPMPanelCooling)
//           and a specific one for the SWKI HE301-01 norm (CDlgIndSelPMPanelCoolingSWKI).
class CDlgIndSelPMPanelCoolingBase : public CDlgIndSelPMPanelHCSBase
{
public:
	CDlgIndSelPMPanelCoolingBase( CIndSelPMParams *pclIndSelParams, PanelType ePanelType, UINT nID = 0, CWnd *pParent = NULL );
	virtual ~CDlgIndSelPMPanelCoolingBase() {}

	virtual void OnNormChanged();

	// Copy 'pclPMInputUser' into the input fields.
	// Param: 'bDontShowErrorMessage': the 'VerifyPMInputUser' method is internally called. This variable specify
	//        if we can show a popup window with message if there are errors.
	virtual bool ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Param: 'bDontShowErrorMessage': This variable specify if we can show a popup window with message if there are errors.
	virtual bool VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Not called in reaction to the 'WM_USER_NEWDOCUMENT' message. Because we need that 'OnNewDocument' of the
	// 'DlgIndSekPressureMaintenance' be called first. In this method, we call 'ApplyPMInputUser' that fill inputs
	// and validate them with sometimes by setting some field with a red border to signal an error.
	// If 'OnNewDocument' here is called after, we reset the tooltip and errors.
	virtual void OnNewDocument();
	
	// HYS-1041: Disabled ENTER key.
	virtual void EnablePMSelection( bool bEnable );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	
	virtual afx_msg void OnEnSetFocusMaxTemp();

	virtual afx_msg void OnBnClickedCheckMinTemp();
	virtual afx_msg void OnBnClickedWaterContent();
	
	virtual afx_msg void OnKillFocusEditMaxTemp();
	virtual afx_msg void OnKillFocusEditSupplyTemp();
	virtual afx_msg void OnKillFocusEditReturnTemp();
	virtual afx_msg void OnKillFocusEditMinTemp();
	virtual afx_msg void OnKillFocusEditFillTemp();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelPMPanelBase' protected methods.
	virtual void ResetToolTipsErrorBorder( CWnd *pWnd = NULL, bool bToolTips = true, bool bErrorBorder = true );
	virtual void InitToolTips( CWnd *pWnd = NULL );
	virtual void UpdateVolumeSystemStatus(BOOL bEnableSysVol, BOOL bEnableInstPower);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelPMPanelHCSBase' protected pure virtual methods.
	virtual double GetSafetyTempLimiter() { return 0; }
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods:
private:
	void _UpdateInstalledPowerWaterContent( void );
	UINT _UpdateMinTemperature( void );

private:
	CExtNumEdit m_EditMaxTemp;
	CButton m_clCheckMinTemp;
	double m_dMaxTempSaved;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelCooling dialog
// HYS-1534: Common cooling class for all norm.
class CDlgIndSelPMPanelCooling : public CDlgIndSelPMPanelCoolingBase
{
public:
	enum { IDD = IDD_DLGINDSELPRESSMAINTCOOLING };
	CDlgIndSelPMPanelCooling( CIndSelPMParams *pclIndSelParams, CWnd *pParent = NULL );
	virtual ~CDlgIndSelPMPanelCooling() {}

	// Copy 'pclPMInputUser' into the input fields.
	// Param: 'bDontShowErrorMessage': the 'VerifyPMInputUser' method is internally called. This variable specify
	//        if we can show a popup window with message if there are errors.
	virtual bool ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Param: 'bDontShowErrorMessage': This variable specify if we can show a popup window with message if there are errors.
	virtual bool VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelCooling dialog
// HYS-1534: specific cooling class for the SWKI HE301-01 norm.
class CDlgIndSelPMPanelCoolingSWKI : public CDlgIndSelPMPanelCoolingBase
{
public:
	enum { IDD = IDD_DLGINDSELPRESSMAINTCOOLINGSWKI };
	CDlgIndSelPMPanelCoolingSWKI( CIndSelPMParams *pclIndSelParams, CWnd *pParent = NULL );
	virtual ~CDlgIndSelPMPanelCoolingSWKI() {}

	// Copy 'pclPMInputUser' into the input fields.
	// Param: 'bDontShowErrorMessage': the 'VerifyPMInputUser' method is internally called. This variable specify
	//        if we can show a popup window with message if there are errors.
	virtual bool ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Param: 'bDontShowErrorMessage': This variable specify if we can show a popup window with message if there are errors.
	virtual bool VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Not called in reaction to the 'WM_USER_NEWDOCUMENT' message. Because we need that 'OnNewDocument' of the
	// 'DlgIndSekPressureMaintenance' be called first. In this method, we call 'ApplyPMInputUser' that fill inputs
	// and validate them with sometimes by setting some field with a red border to signal an error.
	// If 'OnNewDocument' here is called after, we reset the tooltip and errors.
	virtual void OnNewDocument();

	// HYS-1041: Disabled ENTER key.
	virtual void EnablePMSelection( bool bEnable );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );
	
	virtual afx_msg void OnBnClickedStorageTankMaxTemp();
	virtual afx_msg void OnEnSetFocusStorageTankVolume();
	virtual afx_msg void OnKillFocusStorageTankVolume();

private:
	CButton m_BtnStorageTankMaxTemp;
	CExtStatic m_StaticStorageTankVolume;
	CExtNumEdit m_EditStorageTankVolume;
	double m_dStorageTankVolumeSaved;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelSolarBase dialog
// HYS-1534: Base class for solar. For the moment, we have a common solar class for all norm (CDlgIndSelPMPanelSolar)
//           and a specific one for the SWKI HE301-01 norm (CDlgIndSelPMPanelSolarSWKI).
class CDlgIndSelPMPanelSolarBase : public CDlgIndSelPMPanelHCSBase
{
public:
	enum { IDD = IDD_DLGINDSELPRESSMAINTSOLAR };
	CDlgIndSelPMPanelSolarBase( CIndSelPMParams *pclIndSelParams, PanelType ePanelType, UINT nID = 0, CWnd *pParent = NULL );
	virtual ~CDlgIndSelPMPanelSolarBase() {}

	virtual void OnNormChanged();

	// Copy 'pclPMInputUser' into the input fields.
	// Param: 'bDontShowErrorMessage': the 'VerifyPMInputUser' method is internally called. This variable specify
	//        if we can show a popup window with message if there are errors.
	virtual bool ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Param: 'bDontShowErrorMessage': This variable specify if we can show a popup window with message if there are errors.
	virtual bool VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Not called in reaction to the 'WM_USER_NEWDOCUMENT' message. Because we need that 'OnNewDocument' of the
	// 'DlgIndSekPressureMaintenance' be called first. In this method, we call 'ApplyPMInputUser' that fill inputs
	// and validate them with sometimes by setting some field with a red border to signal an error.
	// If 'OnNewDocument' here is called after, we reset the tooltip and errors.
	virtual void OnNewDocument();

	// HYS-1041: Disabled ENTER key.
	virtual void EnablePMSelection( bool bEnable );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );

	virtual afx_msg void OnEnSetFocusSolarCollector();
	virtual afx_msg void OnEnSetFocusSafetyTempLimiter();

	virtual afx_msg void OnEnChangeSafetyTempLimiter();

	virtual afx_msg void OnKillFocusSolarCollector();
	virtual afx_msg void OnKillFocusEditSafetyTempLimiter();
	virtual afx_msg void OnKillFocusEditSupplyTemp();
	virtual afx_msg void OnKillFocusEditReturnTemp();
	virtual afx_msg void OnKillFocusEditMinTemp();
	virtual afx_msg void OnKillFocusEditFillTemp();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelPMPanelBase' protected virtual methods.
	virtual void ResetToolTipsErrorBorder( CWnd *pWnd = NULL, bool bToolTips = true, bool bErrorBorder = true );
	virtual void InitToolTips( CWnd *pWnd = NULL );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgIndSelPMPanelHCSBase' protected pure virtual methods.
	virtual double GetSafetyTempLimiter() { return m_EditSafetyTLim.GetCurrentValSI(); }
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CExtNumEdit m_EditSolarContent;
	CExtNumEdit m_EditSafetyTLim;
	double m_dSolarContentSaved;
	double m_dSafetyTempLimiterSaved;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelSolar dialog
// HYS-1534: Common solar class for all norm.
class CDlgIndSelPMPanelSolar : public CDlgIndSelPMPanelSolarBase
{
public:
	enum { IDD = IDD_DLGINDSELPRESSMAINTSOLAR };
	CDlgIndSelPMPanelSolar( CIndSelPMParams *pclIndSelParams, CWnd *pParent = NULL );
	virtual ~CDlgIndSelPMPanelSolar() {}

	// Copy 'pclPMInputUser' into the input fields.
	// Param: 'bDontShowErrorMessage': the 'VerifyPMInputUser' method is internally called. This variable specify
	//        if we can show a popup window with message if there are errors.
	virtual bool ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Param: 'bDontShowErrorMessage': This variable specify if we can show a popup window with message if there are errors.
	virtual bool VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMPanelSolarSWKI dialog
// HYS-1534: specific solar class for the SWKI HE301-01 norm.
class CDlgIndSelPMPanelSolarSWKI : public CDlgIndSelPMPanelSolarBase
{
public:
	enum { IDD = IDD_DLGINDSELPRESSMAINTSOLARSWKI };
	CDlgIndSelPMPanelSolarSWKI( CIndSelPMParams *pclIndSelParams, CWnd *pParent = NULL );
	virtual ~CDlgIndSelPMPanelSolarSWKI() {}

	// Copy 'pclPMInputUser' into the input fields.
	// Param: 'bDontShowErrorMessage': the 'VerifyPMInputUser' method is internally called. This variable specify
	//        if we can show a popup window with message if there are errors.
	virtual bool ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Param: 'bDontShowErrorMessage': This variable specify if we can show a popup window with message if there are errors.
	virtual bool VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Not called in reaction to the 'WM_USER_NEWDOCUMENT' message. Because we need that 'OnNewDocument' of the
	// 'DlgIndSekPressureMaintenance' be called first. In this method, we call 'ApplyPMInputUser' that fill inputs
	// and validate them with sometimes by setting some field with a red border to signal an error.
	// If 'OnNewDocument' here is called after, we reset the tooltip and errors.
	virtual void OnNewDocument();

	// HYS-1041: Disabled ENTER key.
	virtual void EnablePMSelection( bool bEnable );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );

	virtual afx_msg void OnBnClickedSolarContentMultiplierFactor();

	virtual afx_msg void OnBnClickedStorageTankMaxTemp();
	virtual afx_msg void OnEnSetFocusStorageTankVolume();
	virtual afx_msg void OnKillFocusStorageTankVolume();

// Protected variables.
protected:
	CButton m_BtnCheckSolarContentMultiplierFactor;

	CButton m_BtnStorageTankMaxTemp;
	CExtStatic m_StaticStorageTankVolume;
	CExtNumEdit m_EditStorageTankVolume;
	double m_dStorageTankVolumeSaved;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CDlgIndSelPMNo dialog

class CDlgIndSelPMNo : public CDlgIndSelPMPanelBase
{
public:
	enum { IDD = IDD_DLGINDSELPRESSMAINTNO };
	CDlgIndSelPMNo( CIndSelPMParams *pclIndSelParams, CWnd *pParent = NULL );
	virtual ~CDlgIndSelPMNo() {}

	// Copy 'pclPMInputUser' into the input fields.
	// Param: 'bDontShowErrorMessage': the 'VerifyPMInputUser' method is internally called. This variable specify
	//        if we can show a popup window with message if there are errors.
	virtual bool ApplyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	// Param: 'bDontShowErrorMessage': This variable specify if we can show a popup window with message if there are errors.
	virtual bool VerifyPMInputUser( CPMInputUser *pclPMInputUser, bool bAllowShowErrorMessage );

	virtual void OnPMTypeChanged( CPMInputUser *pclPMInputUser );

	// Not called in reaction to the 'WM_USER_NEWDOCUMENT' message. Because we need that 'OnNewDocument' of the
	// 'DlgIndSekPressureMaintenance' be called first. In this method, we call 'ApplyPMInputUser' that fill inputs
	// and validate them with sometimes by setting some field with a red border to signal an error.
	// If 'OnNewDocument' here is called after, we reset the tooltip and errors.
	virtual void OnNewDocument();

	// HYS-1041: Disabled ENTER key.
	virtual void EnablePMSelection( bool bEnable );

	virtual void EnableDegassing( bool bEnable, bool bWaterMakupChecked );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg LRESULT OnUnitChange( WPARAM wParam = 0, LPARAM lParam = 0 );

	virtual afx_msg void OnEnSetFocusDegPress();
	virtual afx_msg void OnEnSetFocusSystemVolume();
	virtual afx_msg void OnEnSetFocusWaterHardness();

	virtual afx_msg void OnKillFocusEditDegPress();
	virtual afx_msg void OnKillFocusEditSystemVolume();
	virtual afx_msg void OnKillFocusEditWaterHardness();

	virtual double GetSafetyTempLimiter() { return 0; }

// Protected variables.
protected:
	CExtNumEdit m_EditDegassingPressure;
	CExtNumEdit m_EditSystemVol;
	CExtNumEdit m_EditWaterHardness;

	double m_dDegassingPressureConnectPointSaved;
	double m_dSystemVolSaved;
	double m_dWaterHardnessSaved;

	// To not interfere with 'm_ToolTip' in the base class that is only used for errors.
	CMyToolTipCtrl m_ToolTipOther;
};
