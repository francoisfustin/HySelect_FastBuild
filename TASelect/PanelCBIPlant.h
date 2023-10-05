#pragma once

class CWizardCBI;
class CPanelCBIPlant : public CDlgWizard
{
public:
	enum { IDD = IDD_PANELCBIPLANT };
	CPanelCBIPlant( CWnd* pParent = NULL );
	virtual ~CPanelCBIPlant();

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
	afx_msg void OnClickList( NMHDR* pNMHDR, LRESULT* pResult );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizard' protected virtual methods.
	virtual BOOL OnWizNext() { return OnWizFinish(); }
	virtual BOOL OnWizFinish();
	virtual bool OnActivate();
	virtual bool OnAfterActivate();
	virtual bool OnEscapeKeyPressed( void );
	virtual int OnWizButtons(); 
	virtual int OnWizButFinishTextID();
	virtual void OnGetMinMaxInfo( MINMAXINFO* lpMMI ) { lpMMI->ptMinTrackSize.x = 400; lpMMI->ptMinTrackSize.y = 400; }
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Check the selected plant and load it in TADS after user confirmation.
	// Return true if the processing is completed and we can go to the next panel.
	bool ProcessSelectedPlant();

	// Move selected plant pPlant from temporary tab to the TADS.
	void MoveSelPlantToTADS( CDB_MultiString* pPlant );
	
	// Append info from selected plant pPlant of temporary tab in the pTmpPipTab table in TADS.
	void AppendSelPlantToTADS( CDB_MultiString* pPlant, CTable* pTmpPipTab, bool* pfWarningOldVersion, CTable* pOrgTab );

	// Copy valve data from TmpSavedData to an existing HM in the TADS.
	// if bNew = true keep Uid if different of zero.
	void CopyValvData( CDB_MultiString* pTmpValv, CDS_HydroMod* pHM, bool fNew = false, unsigned short Uid = 0 );

	// Reset recursively HydroMod treatment to ett_Delete.
	void ResetHMTreatment( CTable* pTab );
	
	// Delete recursively valve only CDS_HydroMod tagged as ett_Delete.
	void DeleteTaggedValves( CTable* pHM );
	
	// Copy the media data from TmpSavedData to the Proj CDS_WaterCharacteristic.
	void CopyMediaData( CDB_MultiString* pPlant );

// Private variables.
private:
	CListCtrl m_List;
	CEdit m_EditText;
	CTADatastruct* m_pTADS;
	CTADatabase* m_pTADB;
	CWizardCBI* m_pParent;
	CBrush m_brWhiteBrush;
	CImageList	m_ImageList;
};