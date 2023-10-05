#pragma once


#include "afxcmn.h"
#include "DialogExt.h"


class CDS_HydroMod;
class CDlgHMTree : public CDialogExt
{
public:
	enum { IDD = IDD_DLGHMTREE };

	CDlgHMTree( CWnd* pParent = NULL );
	virtual ~CDlgHMTree();
	BOOL Create();

	void FillHMTreeView( CTable* pcTable, HTREEITEM hParentItem );
	void SetVisible( bool fflag );
	bool IsVisible();
	void InitTree( CDS_HydroMod *pHM, bool fEditMode = true );

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnMove( int x, int y );
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnTvnSelchangingTree( NMHDR *pNMHDR, LRESULT *pResult );

// Protected variables.
protected:
	enum OverlayMaskIndex
	{
		OMI_Undefined		= 0,
		OMI_EditCircuit		= 1,
		OMI_NewCircuit		= 2
	};

	CWnd			*m_pParent;
	CTADatastruct	*m_pTADS;
	CDS_HydroMod	*m_pSelHM;
	bool			m_bInitialized;		// Set to true when OnInitDialog performed.
	bool			m_fEditMode;		// Set to true when OnInitDialog performed.
	CString			m_strSectionName;		// Name of the section in the registry.
	CImageList*		m_pclTreeImageList;
	CTreeCtrl		m_Tree;
	HTREEITEM		m_hSelItem;
	bool			m_fInitRunning;
};
