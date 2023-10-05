#pragma once


// CDlgWizCircuit dialog
#include "afxwin.h"
#include "wizard.h"
#include "PanelCirc1.h"
#include "PanelCirc2.h"
#include "SetDpi.h"
#include "afxcmn.h"

class CDlgHMTree;
class CDlgWizCircuit : public CWizardManager
{
	DECLARE_DYNAMIC( CDlgWizCircuit )

public:
	enum { IDD = IDD_DLGWIZCIRCUIT };

	CDlgWizCircuit( CWnd* pParent = NULL );
	virtual ~CDlgWizCircuit();

	bool Init( CTable *pTab = NULL, bool fEdit = false, CTable **ppTab = NULL );
	CTable *GetpInitialTab() { return m_pInitialTab; }
	CDS_HydroMod *GetpCurrentHM() { return m_pCurrentHM; }
	void SetpCurrentHM( CDS_HydroMod *pHM ) { m_pCurrentHM = pHM; if( m_ppTab ) *m_ppTab = (CTable *)pHM; }
	bool IsModeEdit() { return m_bEditMode; }
	CSpinButtonCtrl *GetpSpin() { return &m_Spin; }
	void SetFocusToSSheet();
	bool IsAtLeastOneCircuitAdded( void );
	CTable* GetTableWhereLastInsertOccured( void );
	
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual void ChangeWizard( PANEL* pNewWiz, PANEL* pOldWiz = NULL, bool fCallAfterActivate = true );
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnBnClickedButtonprint();
	afx_msg void OnMove( int x, int y );
	afx_msg void OnSize( UINT nType, int cx, int cy );

// Private methods
private:
	void _UpdateLayout( void );

// Public variables.
public:
	bool m_bModified;
	CPanelCirc1	m_PanelCirc1;				// Selection between different control loop
	CPanelCirc2	m_PanelCirc2;				// Control Loop

// Protected variables.
protected:
	bool m_bEditMode;
	// called when the panel is activated for the first time
	CSetDPI m_dpi;
	CTable *m_pInitialTab, **m_ppTab;
	CDS_HydroMod *m_pCurrentHM;
	CString m_SectionName;								// Name of the section in the registry
	bool m_bInitialized;
	CButton m_ButtonPrint;
	CStatic m_StaticNumAdd;
	CEdit m_EditNumAdd;
	CSpinButtonCtrl m_Spin;
};

extern CDlgWizCircuit *pDlgWizCircuit;