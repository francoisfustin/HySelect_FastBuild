#pragma once


#include "afxwin.h"
#include "MyToolTipCtrl.h"
#include "DlgOutput.h"

#define DLGHMCOID							0x01

#define DLGHMCO_BUTTONID_ERROR				0x01
#define DLGHMCO_BUTTONID_WARNING			0x02
#define DLGHMCO_BUTTONID_INFO				0x03
#define DLGHMCO_BUTTONID_CLEAR				0x04

// It's for debug purpose
#define DLGHMCO_BUTTONID_WRITEPACKET		0x05
#define DLGHMCO_BUTTONID_BLOCKOUTPUT		0x06
#define DLGHMCO_BUTTONID_RELEASEOUTPUT		0x07
#define DLGHMCO_BUTTONID_WRITEMESSAGE		0x08
#define DLGHMCO_BUTTONID_SCROLLLASTLINE		0x09

class CDlgOutput;
class CDlgHMCompilationOutput : public CDlgOutput
{
	DECLARE_DYNAMIC( CDlgHMCompilationOutput )

public:
	enum ColumnID
	{
		Icon = 0x01,
		HydroModName,
		Problem,
		Solution,
		Parameter
	};
	enum MessageType
	{
		Undefined = -1,
		First = 1,
		Error = First,
		Warning,
		Info,
		Comment,
		Last
	};

	CDlgHMCompilationOutput();
	
private:
	class CHMMessage;
public:
	class CHMInterface : public CBaseInterface
	{
	public:
		CHMInterface();
		virtual ~CHMInterface();
		
		virtual bool Init( void );

		bool FillAndSendMessage( CString strMessage, MessageType eMessageType, CDS_HydroMod* pclHydroMod = NULL, CString strSolution = _T(""), CString strArg1 = _T(""), CString strArg2 = _T(""), CString strArg3 = _T(""), int iOutputID = -1 );
		bool FillAndSendMessage( int iMessageID, MessageType eMessageType, CDS_HydroMod* pclHydroMod = NULL, CString strSolution = _T(""), CString strArg1 = _T(""), CString strArg2 = _T(""), CString strArg3 = _T(""), int iOutputID = -1 );

	private:
		CDlgHMCompilationOutput* m_pDlgHMCompilationOutput;
		CHMMessage *m_pclHMMessage;
	};

	// Allows to send a message with parameters passed each as argument.
	void SendMessage( int iMessageID, MessageType eMessageType, CDS_HydroMod* pclHydroMod = NULL, CString strSolution = _T(""), CString strArg1 = _T(""), CString strArg2 = _T(""), CString strArg3 = _T(""), int iOutputID = -1 );

	// Allows to send a message with parameters passed as a class.
	// Remark: It's up to the caller to delete 'pclHMMessage'.
	void SendMessage( CHMMessage* pclHMessage, int iOutputID = -1 );

	// Overrides 'DlgOutput' methods.
	virtual void OnHeaderButtonClicked( int iButtonID, CVButton::State eState );

	// Overrides 'CDlgOutputSpreadContainer::IOutputSpreadNotificationHandler'
	virtual void OnOutputSpreadContainerCellDblClicked( int iOutputID, int iColumnID, long lRow, CPoint ptMouse, DlgOutputHelper::vecCCellParam* pVecParamList );

	virtual bool ClearOutput( bool fAlsoHeader = false, int iOutputID = -1 );

protected:
	virtual BOOL OnInitDialog();

	// Overrides 'CDlgOutput' protected virtual methods.
	virtual void OnDlgOutputAdded( int iOutputID ,CDlgOutputSpreadContainer* pclDlgOutputSpreadContainer );
	
// Private methods.
private:
	void _UpdateButtonCaption( void );

// Private class.
private:
	class CHMMessage : public DlgOutputHelper::CMessageBase
	{
	public:
		CHMMessage() { m_eMessageType = MessageType::Undefined; m_pDlgHMCompilationOutput = NULL; }
		CHMMessage( CDlgHMCompilationOutput* pDlgHMCompilationOutput );
		CHMMessage &operator=( CHMMessage &clHMMessage );
		void FillMessage( CString strMessage, MessageType eMessageType, CDS_HydroMod* pclHydroMod = NULL, CString strSolution = _T(""), CString strArg1 = _T(""), CString strArg2 = _T(""), CString strArg3 = _T("") );
		void FillMessage( int iMessageID, MessageType eMessageType, CDS_HydroMod* pclHydroMod = NULL, CString strSolution = _T(""), CString strArg1 = _T(""), CString strArg2 = _T(""), CString strArg3 = _T("") );
		bool FillAndSendMessage( CString strMessage, MessageType eMessageType, CDS_HydroMod* pclHydroMod = NULL, CString strSolution = _T(""), CString strArg1 = _T(""), CString strArg2 = _T(""), CString strArg3 = _T(""), int iOutputID = -1 );
		bool FillAndSendMessage( int iMessageID, MessageType eMessageType, CDS_HydroMod* pclHydroMod = NULL, CString strSolution = _T(""), CString strArg1 = _T(""), CString strArg2 = _T(""), CString strArg3 = _T(""), int iOutputID = -1 );
		
		bool SendMessage( int iOutputID = -1 );
		
		void SetMessageType( MessageType eMessageType );
		void SetHydroModPointer( CDS_HydroMod* pclHydroMod );
		void SetMessageString( CString strMessage );
		void SetMessageID( int iMessageID );
		void SetSolutionString( CString strSolution );
		void SetSolutionID( int iSolutionID );
		void SetArguments( CString strArg1, CString strArg2 = _T(""), CString strArg3 = _T("") );

		MessageType GetMessageType( void ) { return m_eMessageType; }
		void SetpDlgHMCompilationOutput( CDlgHMCompilationOutput* pDlgHMCompilationOutput ) { m_pDlgHMCompilationOutput = pDlgHMCompilationOutput; }
		CDlgHMCompilationOutput* GetpDlgHMCompilationOutput( void ) const { return m_pDlgHMCompilationOutput; }
	
	protected:
		virtual void Copy( CHMMessage* pclHMMessage );
	
	private:
		MessageType m_eMessageType;
		CDlgHMCompilationOutput* m_pDlgHMCompilationOutput;
	};

// Private variables.
private:
	DlgOutputHelper::CFilterList m_clFilterList;
	std::map<short, long> m_mapCounters;
};
