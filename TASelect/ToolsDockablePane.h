#pragma once


#include "afxwin.h"

class CDlgOutput;
class CDlgHMCompilationOutput;
class CDlgBatchSelectionOutput;
class CDlgPMGraphsOutput;

#define DLGTDP_MINHEIGHT	100

class CToolsDockablePane : public CDockablePane
{
	DECLARE_DYNAMIC( CToolsDockablePane )

public:
	enum { IDD = IDD_TOOLSDOCKABLEPANE };

	// Allow to link ID with a specific output when calling 'OutputMessage' method.
	typedef enum TabID
	{
		OutputID_First			= 0,
		OutputID_HMCompilation	= OutputID_First,
		OutputID_BatchSelection	= OutputID_HMCompilation + 1,
		OutputID_PMGraphs		= OutputID_BatchSelection + 1,
		OutputID_Last			= OutputID_PMGraphs + 1
	};

	typedef struct _ContextParameters
	{
		int m_iContextID;
		bool m_bIsPaneVisible;
		BOOL m_bIsPaneAutoHide;
		BOOL m_bIsGripperEnabled;
		bool m_bIsPaneFullScreen;
		bool m_bIsContextMenuEnabled;
		long m_lHeight;
		DWORD m_dwAlignement;
		bool m_arIsTabVisible[TabID::OutputID_Last];
		int m_iLinkedRightView;

		// This variable allow to set how many times in ms before the panel is hidden.
		// By default, this variable is set to 700 in the 'afxdockablepane.h'.
		int m_nTimeOutBeforeAutoHide;
	}ContextParameters;
	
	CToolsDockablePane();
	virtual ~CToolsDockablePane();

	CDlgHMCompilationOutput *GetpDlgHMCompilationOutput( void ) { return m_pDlgHMCompilationOutput; }
	CDlgBatchSelectionOutput *GetpDlgBatchSelectionOutput( void ) { return m_pDlgBatchSelectionOutput; }
	CDlgPMGraphsOutput *GetpDlgPMGraphsOutput( void ) { return m_pDlgPMGraphsOutput; }
	
	ContextParameters *RegisterContext();

	// Allow to restore the state of a precise context.
	// Param: 'bForce' by default to 'false' -> do nothing if we are already in the same context.
	//        Set to 'true' if you need to change the actual context.
	// Param: 'bMainFrameResized' is set to 'true' when it is the 'OnMainFrameResize' method that is calling the 'RestoreContext' method.
	bool RestoreContext( int iContextID, bool bForce = false, bool bMainFrameResized = false );
	ContextParameters *GetContextByID( int iContextID );

	// Allow to adapt the pane height in regards to the new desired height for the output in it.
	bool ChangeDlgOutputHeight( int iContextID, long lNewHeight );

	// Allow the main frame to prevent when user is resizing the application.
	// Remark: we need it for the dockable pane. Because if we decrease the main frame window height, the dockable will automatically
	// decrease in the same time. But not when we increase back the frame window height.
	void BlockSavingDockSize( bool bBlock );
	void OnMainFrameSizing( UINT bwSide, LPRECT pRect );
	void OnMainFrameSize( UINT nType, int cx, int cy );

	// Overrides 'CDockablePane' public method.
	virtual CMFCAutoHideBar *SetAutoHideMode( BOOL bMode, DWORD dwAlignment, CMFCAutoHideBar *pCurrAutoHideBar = NULL, BOOL bUseTimer = TRUE);

// Public variables.
public:
	CMFCTabCtrl	m_wndTabs;
	CDlgHMCompilationOutput *m_pDlgHMCompilationOutput;
	CDlgBatchSelectionOutput *m_pDlgBatchSelectionOutput;
	CDlgPMGraphsOutput *m_pDlgPMGraphsOutput;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual void OnAfterDock( CBasePane *pBar, LPCRECT lpRect, AFX_DOCK_METHOD dockMethod );
	virtual BOOL OnShowControlBarMenu( CPoint point );
	virtual void OnSlide( BOOL bSlideOut );

// Private methods.
private:
	void _ShowTab( TabID eTabID, bool bShow, bool bHideOther );
	//HYS-1046: To force output to be resizing when no modification is applying to the dockpane
	void forceResize();

// Private variables.
private:
	bool m_bInitialized;
	typedef std::map<int, CDlgOutput *>::iterator iterOutputList;
	std::map<int, CDlgOutput *> m_mapOutputList;
	std::map<int, int> m_mapOutputCaption;
	std::map<int, bool> m_mapFreezeHeightSaving;
	std::map<int, int> m_mapHeightToRestore;
	bool m_bContextMenuEnabled;
	bool m_bBlockSavingState;
	CArray<ContextParameters *> m_arContextList;
	int m_iCurrentContextID;
};
