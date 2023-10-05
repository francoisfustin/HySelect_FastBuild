#pragma once


#include <afxtempl.h>

#include "SerialCom.h"
#include "wizard.h"
#include "PanelCBIPlant.h"
#include "PanelCBILog.h"
#include "MyToolTipCtrl.h"
#include "DlgListBox.h"
#include "XGroupBox.h"


/////////////////////////////////////////////////////////////////////////////
// CBI communication
/*
#define SENDCBI2SOFTVERSION		":00000085"		// Checksum 7B
#define ERASESAVEDDATA			":00000086"  	// CheckSum 7A
#define ERASELOGGEDDATA 		":00000087"		// Checksum 79
#define SENDSAVEDDATA			":02000088"		// DDDD	:	 	DD0:Pressure [0->4]
												//				DD1:Flow [0->5]
#define SENDLOGGEDDATA			":03000089"		// DDDDDD : 	DD0:Pressure [0->4]
												//				DD1:Flow [0->5]
												//				DD2:Temperature [0->1]
#define ADDSITE					":38000090"		// DD...DD: 	DD0:Flow [0->5]
												//				DD1..DD21 Site name (NTZ)
#define ADDMODULE				":0B000091"		// DD...DD: 	DD0..DD4 Module name + blanks if < 5(NTZ)

#define ADDVALVE				":37000092"		// DD...DD: 	DD0: 			valve type 0=normal, 1=alternative
															//	DD1: 			valve number
															//	DD2..DD12: 	valve name	 
															//	DD13..DD24: valve type
															//	DD25..DD30:	valve size
															//	DD31..DD41:	desired flow
															//	DD42..DD53: position KV optional
#define ERASESITE				":15000093"		// DD..DD		DD0..DD20 Site name (NTZ)
*/
#define HEADERSIZE				9
static const BYTE SENDCBI2SOFTVERSION[] = {0x3A,0x30,0x30,0x30,0x30,0x30,0x30,0x38,0x35};
static const BYTE ERASESAVEDDATA[]		= {0x3A,0x30,0x30,0x30,0x30,0x30,0x30,0x38,0x36};
static const BYTE ERASELOGGEDDATA[]		= {0x3A,0x30,0x30,0x30,0x30,0x30,0x30,0x38,0x37};
static const BYTE SENDSAVEDDATA[]		= {0x3A,0x30,0x32,0x30,0x30,0x30,0x30,0x38,0x38};
static const BYTE SENDLOGGEDDATA[]		= {0x3A,0x30,0x33,0x30,0x30,0x30,0x30,0x38,0x39};
static const BYTE ADDSITE[]				= {0x3A,0x33,0x38,0x30,0x30,0x30,0x30,0x39,0x30};
static const BYTE ADDMODULE[]			= {0x3A,0x30,0x42,0x30,0x30,0x30,0x30,0x39,0x31};
static const BYTE ADDVALVE[]			= {0x3A,0x33,0x37,0x30,0x30,0x30,0x30,0x39,0x32};
static const BYTE ERASESITE[]			= {0x3A,0x31,0x35,0x30,0x30,0x30,0x30,0x39,0x33};

/////////////////////////////////////////////////////////////////////////////
#define		RXTO		1
#define		TXTO		2

#define		TIMEOUT		1000
#define		BUFFERSIZE	256

#define		FNONE		0x0000
#define		FTO			0x0001
#define 	FACK		0x0002
#define		FMSG		0x0004
#define 	FERROR 		0x0008


#define		MAXCOMNUMBER	64

//typedef unsigned char	BYTE;
//typedef unsigned short	WORD;


class CWizardCBI;

/////////////////////////////////////////////////////////////////////////////
// CPanelCBI1 dialog

class CPanelCBI1 : public CDlgWizard
{
public:
	enum { IDD = IDD_PANELCBI1 };
	CPanelCBI1( CWnd* pParent = NULL);
	virtual ~CPanelCBI1();

	virtual BOOL Create( CWizardManager* pParent );

	void SetComboBox();
	bool IsAreaCompatible(){return m_fAreaCompatibility;};

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );

	LRESULT OnInitComPort( WPARAM wParam, LPARAM lParam );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizard' protected virtual methods.
	virtual bool OnActivate();
	virtual bool OnAfterActivate();
	virtual BOOL OnWizNext();
	virtual int OnWizButFinishTextID() { return IDS_WIZBUT_CANCEL; }
	virtual void OnGetMinMaxInfo( MINMAXINFO* lpMMI ) { lpMMI->ptMinTrackSize.x = 400; lpMMI->ptMinTrackSize.y = 400; }
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	CComboBox m_ComboPort;
	CWizardCBI *m_pclWizardCBI;
	CBrush m_brWhiteBrush;
	bool m_fAreaCompatibility;
};

////////////////////////////////////////////////////////////////////////////
// CPanelCBI2 dialog
class CPanelCBI2 : public CDlgWizard
{
public:
	enum { IDD = IDD_PANELCBI2 };
	CPanelCBI2( CWnd* pParent = NULL );
	virtual ~CPanelCBI2() {}

	virtual BOOL Create( CWizardManager* pParent );
	
	BOOL GetCheckCollectData() { return m_CheckCollectData; }
	BOOL GetCheckReadProj() { return m_CheckReadProj; }
	BOOL GetCheckSendProj(){ return m_CheckSendProj; }

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckcollectdata();
	afx_msg void OnCheckreadproj();
	afx_msg void OnChecksendproj();
	afx_msg void OnButeraselogdata();
	afx_msg void OnButerasesaveddata();
	afx_msg void OnButcannotsend();

	enum enum_Column
	{
		Problem = 0,
		ModuleName = 1,
		ValveName = 2,
		Valve = 3,
		ValveIndex = 4
	};
	
	enum enum_CheckProjectRetCode
	{
		ProjectOK,
		ProjectNotOK,
		ProjectMaybeOK
	};
	
	enum enum_ProbDescription
	{
		UndefValv = 0,
		EmptyMod = 1,
		NotInCBI = 2,
		UndetValve = 3,
		TooManyWaterChar = 4,
		LastProb
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizard' protected virtual methods.
	virtual BOOL OnWizCancel();
	virtual BOOL OnWizNext(){return true;};
	virtual bool OnActivate();
	virtual bool OnAfterActivate();
	virtual int OnWizButtons() { return ( CWizardManager::WizButFinish | CWizardManager::WizButNext ); }
	virtual int OnWizButFinishTextID() { return IDS_WIZBUT_CANCEL; }
	virtual void OnGetMinMaxInfo( MINMAXINFO* lpMMI ) { lpMMI->ptMinTrackSize.x = 400; lpMMI->ptMinTrackSize.y = 400; }
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Check the project coherence, count the valve number.
	// Return true if OK.
	enum enum_CheckProjectRetCode CheckProject( CDS_HydroMod *pHM, int &count );

// Private variables.
private:
	CWizardCBI* m_pclWizardCBI;
	CButton	m_CheckSendProjBut;
	CMFCButton m_ButCannotSend;
	CMFCButton m_ButEraseLogData;
	CMFCButton m_ButEraseSavedData;
	CEdit m_EditVersion;
	BOOL m_CheckCollectData;
	BOOL m_CheckReadProj;
	BOOL m_CheckSendProj;
	CXGroupBox m_GroupBox;

	CTADatastruct *m_pTADS;
	CMyToolTipCtrl m_ToolTip;
	CString	m_strCBIVerComp;
	CString m_strCBIVersion;
	int	m_iCBICompat;
	bool m_fButIgnoreVisible;
	// m_strProblemPos used to store the string POSITION of related problem in the DialogCannotSend Item
	POSITION m_strProblemPos[8];
	CDlgListBox	m_DlgCannotSend;
};

/////////////////////////////////////////////////////////////////////////////
// CPanelCBI3 dialog

class CPanelCBI3 : public CDlgWizard
{
public:
	enum { IDD = IDD_PANELCBI3 };
	CPanelCBI3( CWnd* pParent = NULL );
	virtual ~CPanelCBI3();

	virtual BOOL Create( CWizardManager* pParent );

	void DeleteTmpSavedData();

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CDlgWizard' protected virtual methods.
	virtual BOOL OnWizNext();
	virtual BOOL OnWizFinish();
	virtual bool OnActivate();
	virtual bool OnAfterActivate();
	virtual int OnWizButtons() { return ( CWizardManager::WizButFinish | CWizardManager::WizButNext ); }
	virtual int OnWizButFinishTextID() { return IDS_WIZBUT_CANCEL; }
	virtual void OnGetMinMaxInfo( MINMAXINFO* lpMMI ) { lpMMI->ptMinTrackSize.x = 400; lpMMI->ptMinTrackSize.y = 400; }
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Count the valve number.
	void CountValves( CDS_HydroMod *pHM, int &count );
	
	// Send the current project to the CBI.
	// Return -1 in case of "ERROR" message from CBI.
	// Return 0	in case of transmission error.
	// Return 1 if OK.
	int SiteBuilder();
	
	// Send current pHM module and all valve included.
	bool SendModule( CDS_HydroMod* pHM, int &ProgressPos );

	// Add a Valve in the current module.
	bool AddValve( CDS_HydroMod* pHM, BYTE *pTx, int &ProgressPos );
	
	// Copy a string from source to target, fill empty space (difference between sizeof(src) and len with space
	// or specific character ,if fNTS = true put a 0 at the end of the Trg.
	void CopyTextWithSpace( BYTE *pTrg, const TCHAR *pSrc, BYTE len, bool fNTS = true, int fillchar =  0x20 /*' '*/);

	// Collect all saved data from the CBI.
	// Save these data into the Temporary SaveDataTab.
	bool CollectSavedData();
	
	// Collect all logged data from the CBI.
	// Save Data into the Temporary LogData Tab
	bool CollectLoggedData();

	LRESULT OnDataTransferReady( WPARAM wParam, LPARAM lParam );

// Private variables.
private:
	CEdit m_EditInfo;
	CProgressCtrl m_Progress;
	CTADatastruct* m_pTADS;
	CUnitDatabase* m_pUnitDB;
	CWizardCBI* m_pclWizardCBI;
	CBitmap m_BitmapArrow;
	CBrush m_brWhiteBrush;
	int m_iDTState;				// Data transfer state; used to recover to current state of 
									// transmission after a transmission problem
	bool m_bUserAborted;
};


/////////////////////////////////////////////////////////////////////////////
// CWizardCBI dialog
class CWizardCBI : public CWizardManager
{
// Construction
public:
	enum { IDD = IDD_DLGWIZARD };
	CWizardCBI( CWnd* pParent = NULL );
	virtual ~CWizardCBI() {}

	struct s_Msg 
	{
		struct s_Msg()
		{
			Reset();
		}
		void Reset()
		{
			Length = 0;
			ZeroMemory( Buffer, sizeof( Buffer ) );
		}
		WORD Length;
		BYTE Buffer[2*BUFFERSIZE];
	};

	struct s_TabUid
	{
		IDPTR	TabIDPtr;
		WORD	FirstUid;
	};

#define MAXVALVES 1000
	struct s_TabFirstUID
	{
		const static WORD PipingTab = 1;
		const static WORD SSelHub = MAXVALVES;
	} m_TabFirstUID;

	int HexAsciiToInt( BYTE *pa, int len );
	int GetAnswer( s_Msg *pMsg );
	void SendMessage( BYTE *pMsg2CBI );
	
	/////////////////////////////////////////////////////////////////////
	// If *pCom==0 or pCom==NULL Scan all communication port (1-4) to find a CBI.
	// If the CBI is located return the com number and the software version of the CBI.
	// If *pCom != 0 scan this port.
	// If a CBI is located leave the port open; member variable m_HPortCom hold the Handle of the port opened.
	// If a communication port already exist check if the CBI is always connected.
	CString FindCBIComPort( int *pCom = NULL );
	
	// Ask the version number to the CBI.
	bool IsCBIThere( s_Msg *pBuf );
	
	// Return if a communication port could be opened.
	// WARNING the current port is closed !!!!
	bool IsPortAvailable( int com ) { if( com < MAXCOMNUMBER && com > 0 ) return ( m_PortAvailable[com - 1] ); else return false; }
	
	// Return value of US flag.
	bool GetUSflag() { return m_USflag; }

	// Convert a CBI answer (buffer of byte) to a TCHAR buffer.
	LPCTSTR Byte2TChar( BYTE *pBuf, int MaxLen );
	
	CWaterChar *GetpWaterChar() { return &m_WaterChar; }
	
	// Return the piping Tab where the uid object is stored.
	CTable *GetPipingTab( UINT uid );

// Public variables.
public:
	CArray<s_TabUid> m_aTableUID;
	CPanelCBI1 m_PanelCBI1;
	CPanelCBI2 m_PanelCBI2;
	CPanelCBI3 m_PanelCBI3;
	CPanelCBIPlant m_PanelCBIPlant;
	CPanelCBILog m_PanelCBILog;
	
// Protected methods.
protected:
	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	LRESULT OnInitComPort( WPARAM wParam, LPARAM lParam );

protected:
	s_Msg m_CBIRxMsg;
	CList<s_Msg,s_Msg&> m_MsgList;
	CCom m_Com;
	BYTE m_HexAsciiTxMsg[2 * BUFFERSIZE], *m_pHATx;	
	BYTE m_HexAsciiRxMsg[2 * BUFFERSIZE], *m_pHARx;	
	bool m_PortAvailable[MAXCOMNUMBER];
	bool m_USflag;
	int m_HPortCom;
	BYTE m_TxBuf[200];
	BYTE m_RxBuf[200];
	CWaterChar m_WaterChar;
};