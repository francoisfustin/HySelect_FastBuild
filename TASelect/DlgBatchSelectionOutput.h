#pragma once


#include "afxwin.h"
#include "MyToolTipCtrl.h"
#include "ExtMenu.h"
#include "DlgOutput.h"

#define DLGBSOOID								0x01

#define DLGBSO_BUTTONID_CLEARALL				0x01
#define DLGBSO_BUTTONID_CLEARRESULTS			0x02

class CDlgOutput;
class CDlgBatchSelectionOutput : public CDlgOutput
{
	DECLARE_DYNAMIC( CDlgBatchSelectionOutput )

public:
	class IBSNotificationHandler : public IBaseNotificationHandler
	{
	public:
		enum
		{
			BSNH_First					= IBaseNotificationHandler::NH_Last,
			BSNH_OnButtonClearAll		= ( BSNH_First << 1 ),
			BSNH_OnButtonClearResults	= ( BSNH_OnButtonClearAll << 1 ),
			BSNH_OnPasteData			= ( BSNH_OnButtonClearResults << 1 ),
			BSNH_OnDeleteRow			= ( BSNH_OnPasteData << 1 ),
			BSNH_OnGetColumnList		= ( BSNH_OnDeleteRow << 1 ),
			BSNH_OnAddColumn			= ( BSNH_OnGetColumnList << 1 ),
			BSNH_OnRemoveColumn			= ( BSNH_OnAddColumn << 1 ),
			BSNH_OnKeyboardVirtualKey	= ( BSNH_OnRemoveColumn << 1 ),
			BSNH_OnIsDataExist			= ( BSNH_OnKeyboardVirtualKey << 1 ),
			BSNH_OnIsResultExist		= ( BSNH_OnIsDataExist << 1 ),
			BSNH_OnIsCatalogExist		= ( BSNH_OnIsResultExist << 1 ),
			BSNH_OnDelete				= ( BSNH_OnIsCatalogExist << 1 ),
			BSNH_OnEdit					= ( BSNH_OnDelete << 1 ),
			BSNH_OnSuggest				= ( BSNH_OnEdit << 1 ),
			BSNH_OnValidate				= ( BSNH_OnSuggest << 1 ),
			BSNH_Last					= BSNH_OnValidate,
			BSNH_All					= ( BSNH_OnButtonClearAll | BSNH_OnButtonClearResults | BSNH_OnPasteData | BSNH_OnDeleteRow | BSNH_OnGetColumnList | 
											BSNH_OnAddColumn | BSNH_OnRemoveColumn | BSNH_OnKeyboardVirtualKey | BSNH_OnIsDataExist | BSNH_OnIsResultExist | 
											BSNH_OnIsCatalogExist | BSNH_OnDelete | BSNH_OnEdit | BSNH_OnSuggest | BSNH_OnValidate )
		};
		IBSNotificationHandler() {}
		virtual ~IBSNotificationHandler() {}
		virtual void OnBatchSelectionButtonClearAll() = 0;
		virtual void OnBatchSelectionButtonClearResults() = 0;
		virtual void OnBatchSelectionPasteData() = 0;
		virtual void OnBatchSelectionDeleteRow( long lStartRowRelative, long lEndRowRelative ) = 0;
		virtual bool OnBatchSelectionGetColumnList( mapColData &mapColumnList ) = 0;
		virtual void OnBatchSelectionAddColumn( int iColumnID ) = 0;
		virtual void OnBatchSelectionRemoveColumn( int iColumnID ) = 0;
		virtual bool OnBatchSelectionKeyboardVirtualKey( int iVirtualKey ) = 0;
		virtual bool OnBatchSelectionIsDataExist( bool &fDataExist, bool &fDataValidated ) = 0;
		virtual bool OnBatchSelectionIsResultExist( bool &fResultExist, bool &fResultValidated ) = 0;
		virtual bool OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &fCatExist, CData* &pclProduct ) = 0;
		virtual bool OnBatchSelectionDelete( int iStartColID, int iEndColID, long lStartRowRelative, long lEndRowRelative ) = 0;
		virtual bool OnBatchSelectionEdit( long lRowRelative ) = 0;
		virtual bool OnBatchSelectionSuggest( void ) = 0;
		virtual bool OnBatchSelectionValidate( void ) = 0;
	};

	CDlgBatchSelectionOutput();

public:
	class CBSMessage;
	class CBSInterface : public CBaseInterface
	{
	public:
		CBSInterface();
		virtual ~CBSInterface();

		virtual bool Init( void );

		// 'true' if 'Paste data' item in context menu must be enable.
		bool EnableCtxtMenuPasteData( bool fEnable, int iOutputID = -1 );

		// 'true' if 'Add column' item in context menu must be enable.
		bool EnableCtxtMenuAddColumn( bool fEnable, int iOutputID = -1 );

		CBSMessage *GetMessage( void );
		bool RegisterNotificationHandler( IBSNotificationHandler *pclHandler, int iNotificationHandlerFlags, int iOutputID = -1 );
		bool UnregisterNotificationHandler( IBSNotificationHandler *pclHandler, int iOutputID = -1 );

	private:
		CDlgBatchSelectionOutput *m_pDlgBatchSelectionOutput;
		CBSMessage *m_pclBSMessage;
	};

	class CBSMessage : public DlgOutputHelper::CMessageBase
	{
	public:
		CBSMessage()
		{
			m_pDlgBatchSelectionOutput = NULL;
		}
		CBSMessage( CDlgBatchSelectionOutput *pDlgBatchSelectionOutput );
		CBSMessage &operator=( CBSMessage &clHMMessage );
		void SetpDlgBatchSelectionOutput( CDlgBatchSelectionOutput *pDlgBatchSelectionOutput )
		{
			m_pDlgBatchSelectionOutput = pDlgBatchSelectionOutput;
		}
		CDlgBatchSelectionOutput *GetpDlgBatchSelectionOutput( void ) const
		{
			return m_pDlgBatchSelectionOutput;
		}
		bool SendMessage( long lRow = -1, bool fScrollOnTheLastLine = true, int iOutputID = -1 );

	protected:
		virtual void Copy( CBSMessage *pclBSMessage );

	private:
		CDlgBatchSelectionOutput *m_pDlgBatchSelectionOutput;
	};

	// Completely clear the batch selection interface.
	virtual bool Reset( int iOutputID = -1 );

	// 'true' if 'Paste data' item in context menu must be enable.
	bool EnableCtxtMenuPasteData( bool fEnable, int iOutputID = -1 );

	// 'true' if 'Add column' item in context menu must be enable.
	bool EnableCtxtMenuAddColumn( bool fEnable, int iOutputID = -1 );

	// Allows to send a message with parameters passed as a class.
	// Remark: It's up to the caller to delete 'pclHMMessage'.
	bool SendMessage( CBSMessage *pclBSMessage, long lRow = -1, bool fScrollOnTheLastLine = true, int iOutputID = -1 );

	// Overrides 'DlgOutput' methods.
	virtual void OnHeaderButtonClicked( int iButtonID, CVButton::State eState );

	// Overrides 'CDlgOutputSpreadContainer::INotificationHandler'
	// Remark: these methods are inherited by the base class 'CDlgOutput'.
	virtual void OnOutputSpreadContainerCellRClicked( int iOutputID, int iColumnID, long lRow, CPoint ptMouse, DlgOutputHelper::vecCCellParam *pVecParamList );
	virtual void OnOutputSpreadContainerKeyboardShortcut( int iOutputID, int iKeyboardShortcut );
	virtual void OnOutputSpreadContainerKeyboardVirtualKey( int iOutputID, int iKeyboardVirtualKey );

	bool RegisterNotificationHandler( IBSNotificationHandler *pclHandler, int iNotificationHandlerFlags, int iOutputID = -1 );
	bool UnregisterNotificationHandler( IBSNotificationHandler *pclHandler, int iOutputID = -1 );

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdateMenuText( CCmdUI *pCmdUI );
	afx_msg void OnFltMenuCopy();
	afx_msg void OnFltMenuPaste();
	afx_msg void OnFltMenuDelete();
	afx_msg void OnFltMenuEdit();
	afx_msg void OnFltMenuDelRow();
	afx_msg void OnFltMenuInsRow();
	afx_msg void OnFltMenuSelectAll();
	afx_msg void OnFltMenuClearAll();
	afx_msg void OnFltMenuSuggest();
	afx_msg void OnFltMenuValidate();
	afx_msg void OnFltMenuEditResult();
	afx_msg void OnFltMenuClearResults();
	afx_msg void OnFltMenuGetFullCat();
	afx_msg void OnFltMenuAddColumn( UINT nID );
	afx_msg void OnFltMenuRemoveColumn( UINT nID );

	afx_msg LRESULT OnShowPopupMenu( WPARAM wParam, LPARAM lParam );

	// Overrides 'CDlgOutput' protected virtual methods.
	virtual void OnDlgOutputAdded( int iOutputID , CDlgOutputSpreadContainer *pclDlgOutputSpreadContainer );
	virtual void OnDlgOutputActivated( int iOutputID );

	// Private methods.
private:
	// Allows to copy data to the output.
	void _PasteData( bool fWithKeyboard = false );

	// Private variables.
private:
	typedef std::map<int, IBSNotificationHandler *> mapIntBSNH;
	typedef mapIntBSNH::iterator mapIntBSNHIter;
	typedef struct _BSOutputContextParams
	{
		bool m_fCtxtMenuPasteDataEnabled;
		bool m_fCtxtMenuAddColumnEnabled;
		DlgOutputHelper::CFilterList m_clFilterList;
		int m_iLastColumnIDRightClicked;
		long m_lLastRowRightClicked;
		CPoint m_clLastMousePointRightClicked;
		CData* m_pclProductForQRSCat;
		mapColData m_mapColumnList;
		mapIntBSNH m_mapBSNotificationHandler;
		struct _BSOutputContextParams()
		{
			m_fCtxtMenuPasteDataEnabled = true;
			m_fCtxtMenuAddColumnEnabled = true;
			m_iLastColumnIDRightClicked = -1;
			m_lLastRowRightClicked = -1;
			m_clLastMousePointRightClicked = CPoint( -1, -1 );
			m_pclProductForQRSCat = NULL;
		}
	} BSOutputContextParams;
	typedef std::map<int, BSOutputContextParams> mapBSOutputContext;
	typedef mapBSOutputContext::iterator iterBSOutputContext;
	mapBSOutputContext m_mapBSOutputContextParams;
	CExtMenu m_clAddColExtMenu;
	CExtMenu m_clRemoveColExtMenu;
};
