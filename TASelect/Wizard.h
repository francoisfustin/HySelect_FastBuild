#pragma once


#include "SetDpi.h"
#include "ExtNumEdit.h"
#include "EnBitmap.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgWizard dialog
//
// Base wizard dialog class 
// To create a new panel in a wizard, create a standard CDialogEx and manually replace all 
// 'CDialogEx' occurrence by 'CDlgWizard'

class CWizardManager;
class CDlgWizard : public CDialogEx
{

friend class CWizardManager;

public:
	CDlgWizard( UINT DiagID, CWnd* pParent = NULL );

	// Create child panel with its parent
	virtual BOOL Create( CWizardManager* pParent );

// User Functions
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage( MSG* pMsg );

	// React on cancel, next, back and finish buttons.
	// If the return code is 0 keep the current active panel otherwise leave the panel.
	virtual BOOL OnWizCancel() { m_bInitialized = false; return TRUE; }
	virtual BOOL OnWizNext() { return TRUE; }
	virtual BOOL OnWizBack() { return TRUE; }
	virtual BOOL OnWizFinish() { m_bInitialized = false; return TRUE; }

	// Called when the panel is activated for the first time.
	virtual bool OnInitialActivate() { return OnActivate(); }

	// Called when the panel is activated.
	virtual bool OnActivate() { return true; }

	// Called after activation, gives possibility to disable buttons.
	virtual bool OnAfterActivate() { m_bInitialized = true; return true; }
	
	// Called by 'CDlgWizardManager' when user presses the [ESCAPE] key.
	// If method returns 'true', that means we can exit 'CDlgWizarManager' and all its 'CDlgWizard' panels.
	virtual bool OnEscapeKeyPressed( void ) { m_bInitialized = false; return true; }

	// Called when we leave the panel.
	// methods 'OnWizCancel', 'OnWizNext', 'OnWizBack', 'OnWizFinish' are called first.
	virtual void OnQuitPane() {}
	
	// Display buttons by default.
	// Button 'Finish' is always displayed.
	// First Panel hide button back.
	// Last Panel hide button next.
	// Called after the panel activation.
	virtual int OnWizButtons() { return 1; }
	
	// Return text for Wizard buttons.
	virtual int OnWizButFinishTextID() { return IDS_WIZBUT_FINISH; }
	virtual int OnWizButNextTextID() { return IDS_WIZBUT_NEXT; }
	virtual int OnWizButBackTextID() { return IDS_WIZBUT_BACK; }

	// Return Wizard Manager pointer.
	CWizardManager *GetWizMan();

	// By default remove Cancel function.
	virtual void OnCancel();

	// By default remove Ok function.
	virtual void OnOK() {}

	// Called by 'CWizardManager' when 'WM_SIZE' has been called for the parent dialog.
	virtual void OnSize( CRect rectParentSize ) {}

	virtual void OnGetMinMaxInfo( MINMAXINFO* lpMMI ) = 0;
	
// Protected variables.
protected:
	bool m_bInitialized;
	CWizardManager* m_pParent;
	UINT m_ResID;						// panel identification.
	CExtNumEdit m_EditTitle;
};


/////////////////////////////////////////////////////////////////////////////
// CWizardManager dialog
//
// manage panels from a wizard

class CWizardManager : public CDialogEx
{
	DECLARE_DYNAMIC( CWizardManager )

public:
	// specify Title resource ID and Bitmap resource ID.
	CWizardManager( UINT uiTitleRes, UINT IDD );
	virtual ~CWizardManager();

	// Add a panel at the end of list.
	void Add( CDlgWizard *pDlg, int iID = -1 );

	// Change the current panel.
	// Remove 'pOldWiz' and replace by 'pNewWiz'.
	struct PANEL;
	virtual void ChangeWizard( PANEL *pNewWiz, PANEL *pOldWiz = NULL, bool bCallAfterActivate = true );
	
	// Display panels.
	void DisplayWizard();

	// Called by 'CDlgWizard' when user has pressed the [ESCAPE] key.
	void EscapeKeyPressed( void );
	
	// Called by friend class CDlgWizard do next, back and finish operations.
	void DoNext();
	void DoBack();
	void Cancel();
	void Finish();
	
	// Called to jump to a specific PANEL, return true for success.
	bool JumpToPanel( int i );

	// Enable or disable buttons.
	void EnableButtons( int iButton );
	void DisableButtons( int iButton );
	
	// nCmdShow like CWnd::ShowWindow parameters.
	void ShowButtons( int iButton, int nCmdShow );
	
	enum enum_WizBut;
	void SetButtonText( enum_WizBut eButton, CString *pstrText );
	void SetButtonText( enum_WizBut eButton, int ids );
	PANEL *GetCurrentPanel();
	
// Protected members.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnFinish() {}
	virtual afx_msg void OnWizB();
	virtual afx_msg void OnWizF();
	virtual afx_msg void OnWizN();
	virtual afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual afx_msg void OnGetMinMaxInfo( MINMAXINFO* lpMMI );
	virtual afx_msg void OnClose();
	virtual afx_msg LRESULT OnDoBack( WPARAM wParam, LPARAM lParam );

	virtual bool OnWizFinish() { return true; }
	
// Private methods.
private:
	void _UpdateLayoutPanel( void );
	void _UpdateLayoutBelowPanel( void );

// Public variables.
public:
	enum enum_WizBut
	{
		WizButDefault	= 1,
		WizButFinish	= 2,
		WizButNext		= 4,
		WizButBack		= 8
	};

	struct PANEL
	{
		int m_iID; 
		bool m_bCreated;
		bool m_bInitialized;
		CPoint m_ptInitialSize;
		CDlgWizard* m_pPanel;
	};

	CStatic	m_StaticSeparator;
	CButton	m_ButWizNext;
	CButton	m_ButWizFinish;
	CButton	m_ButWizBack;

// Protected variables.
protected:
	std::vector<PANEL *> m_vecPanelList;			// List of all panels.
	int m_iPanelCount;			// Panels numbers.
	int m_iPanelActive;			// Index of active panel.
	UINT m_uiTitleRes;			// Title resource ID 
	CSetDPI m_dpi;
	UINT m_uiIDD;
	int m_iAvailableHeightBelowPanel;
	int m_iButtonReferencePoint;
};
