#pragma once

#include "hydromod.h"
#include "TerminalUnit.h"
#include "ssheetpanelcirc2.h"
#include "ExtComboBox.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "SetDpi.h"
#include "CbiRestrEdit.h"

class CPanelCirc2 : public CDlgWizard
{
public:
	enum { IDD = IDD_PANELCIRC2 };

	CPanelCirc2( CWnd* pParent = NULL );
	virtual ~CPanelCirc2();

	bool IsHMExist() { return m_pHM != NULL; }
	void Print();
	void EnableButtonNext( bool fEnable );
	void DeleteCircuit( CDS_HydroMod *pHM = NULL );
	CSSheetPanelCirc2 *GetFPSpreadSheet() { return m_pFPSheet; }
	bool IsAtLeastOneCircuitAdded( void ) { return m_bAtLeastOneCircuitAdded; }
	CTable* GetTableWhereLastInsertOccured( void ) { return m_pTab; }

// Protected members.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeComboParent();
	afx_msg void OnEnKillFocusEditHMName();
	afx_msg void OnEnKillFocusEditHMDescription();
	afx_msg void OnSelChangeComboPos();
	afx_msg void OnTimer( UINT_PTR nIDEvent );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizard' virtual methods.
	virtual BOOL OnWizCancel();
	virtual BOOL OnWizNext();
	virtual BOOL OnWizBack();
	virtual BOOL OnWizFinish();
	
	// Return text for Wizard buttons.
	virtual int OnWizButFinishTextID();
	virtual int OnWizButNextTextID() { return IDS_PANELCIRC2_BUTADD; }
	virtual int OnWizButBackTextID() { return IDS_PANELCIRC2_BUTPREVIEWS; }

	// Called when the panel is activated for the first time.
	virtual bool OnInitialActivate();

	// Called when the panel is activated.
	virtual bool OnActivate();
	virtual bool OnAfterActivate();

	virtual bool OnEscapeKeyPressed( void ) { return ( TRUE == OnWizFinish() ) ? true : false; }

	virtual void OnSize( CRect rectParentSize );

	virtual void OnGetMinMaxInfo( MINMAXINFO* lpMMI );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ResetComboParent();
	void FillComboParent( CDS_HydroMod *pHM );
	void CreateNewCircuit( bool fWithTU );
	void FillLocalControls();

	// Duplicate current HM, new created HM becomes current m_pHM.
	void DuplicateHM();

	void FillComboPos( int iMaxPos, int iCurPos );

// Private methods.
private:
	void _VerifyInjectionTemperaturesBeforeInsert( CDS_HydroMod *pclObjectToInsert, CTable *pclTableWhereToInsert );
	void _CorrectInjectionTemperaturesAfterInsert( CDS_HydroMod *pclObjectInserted, CTable *pclTableWhereInserted );

// Protected variables.
protected:
	// Pointer to the new created object, it will be cast when needed.
	// Point to a circuit object CDS_Hm2W, CDS_Hm3W,...
	CDS_HydroMod *m_pHM;
	CCbiRestrEdit m_EditHMName;
	CEdit m_EditDescription;
	int m_iPrevCompoParentSelection;
	CExtNumEditComboBox m_ComboParent;
	CExtNumEditComboBox m_ComboPos;
	CSetDPI dpi;
	bool m_bEditEntry;
	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;
	CDB_CircuitScheme *m_pSch;
	CSSheetPanelCirc2 *m_pFPSheet;
	CTable *m_pTab;
	UINT_PTR m_nTimer;
	bool m_bAtLeastOneCircuitAdded;

	// Working variables for the both "_VerifyInjectionTemperaturesBeforeInsert" and "_CorrectInjectionTemperaturesAfterInsert" methods.
	std::multimap<CDS_HydroMod *, CTable *> m_mmapInjectionHydraulicCircuitWithTemperatureError;
	std::vector<CDS_HydroMod *> m_vecAllInjectionCircuitsWithTemperatureError;
	int m_iDlgInjectionErrorReturnCode;
};
