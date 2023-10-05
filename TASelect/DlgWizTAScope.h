#pragma once

#include "wizard.h"
#include "servicemanager.h"
#include "TAScopeUtil.h"

#include "DlgProgressBar.h"
#include "DlgPanelTAScopeStart.h"
#include "DlgPanelTAScopeDownload.h"
#include "DlgPanelTAScopeMaintenance.h"
#include "DlgPanelTAScopeUpload.h"
#include "DlgPanelTAScopeUpdate.h"


using namespace TASCOPE;
#define MAXCOMNUMBER	32
#define COMSPEED		115200
#define METAFILE		_T("MetaFile.tmd")
#define TADSXFILE		_T("tadsx.tsc")

class CFileTable;

class CDlgWizTAScope : public CWizardManager
{
public:
	enum { IDD = IDD_DLGWIZTASCOPE };

	enum eSoftwareVersion
	{
		eswvHH,
		eswvDB,
		eswvWireless,
		eswvWirelessDresden,
		eswvDPS,
		eswvDBKey,
		eswLast
	};
	
	enum eTAScopeUpdate
	{
		eupdFail,
		eupdUpdateAvailable,				// An Update is available
		eupdNoUpdateNeeded,					// HFT and SFT checked no difference
		eupdLocalHftMissing,				// No local SFT available, no update possible
		eupdTASCOPEHftMissing,				// TA SCOPE HFT missing, Old version?
		eupdIncorrectCRC,					// Problem between SFT and TASCOPE files
		eupdLast
	};

	enum ePanel
	{
		epanStart,
		epanDownLoad,
		epanUpLoad,
		epanMaintenance,
		epanUpdate,
		epanLast
	};

	enum WirelessModuleType
	{
		eWirelessModuleUnknown,
		eHandheldWirelessAtmel,
		eHandheldWirelessDresden,
		eSensor1Wireless
	};

	enum ProgressBarType
	{
		PBT_Download,
		PBT_Upload
	};

	CDlgWizTAScope( CWnd* pParent = NULL );
	virtual ~CDlgWizTAScope();

	CFileTable *GetHftTable();

	WirelessModuleType GetHHWirelessModuleType() { return m_eHHWirelessModuleType; }
	bool IsNewDpSVisioFirmware() { return m_bNewDpSVisioFirmware; }

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Creates the progress bar. </summary>
	///
	/// <remarks>	Display a window popup progress bar 
	/// 			Alen, 30/09/2010. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void CreateProgressBar( ProgressBarType eProgressBarType );
	void DeleteProgressBar();
	double GetProgressValue() { return m_dProgress; }
	void SetProgressValue( double d ) { m_dProgress = d; }

	// Communication function.
	bool IsInBootMode() { return m_fBootMode; }
	bool IsComPortBuzy() { return m_fComPortBusy; }
	void SetComPortBuzy( bool b ) { m_fComPortBusy = b; }
	int GetComNumber() { return m_iComNumber; }
	int FoundTAScopePortCom();
	bool IsTAScopeAvailable();
	void ActivateRefresh( bool bRefresh ) { m_fRefreshVersion = bRefresh; }
	bool GetRefreshVersion() { return m_fRefreshVersion; }
	CSerialPort	*GetComPort() { return m_pCom; }

	CArray<CString,CString> *GetpArCurrentVersion() { return &m_CurrentVersion; }
	double GetVersionAsDouble( eSoftwareVersion esvIndex );
	unsigned int GetTAScopeDSVersion() { return m_uiDSversion; }
	void ClearSoftwareVersion();
	bool RequestSoftwareVersion();
	bool GetMetaData();
	bool GetTADSFile();
	bool RebootDevice( eSoftwareVersion eDevice );
	void WaitMilliSec( unsigned int uiDelay );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Update ZigBee software. </summary>
	///
	/// <remarks>	Alen, 30/09/2010. </remarks>
	///
	/// <param name="bHH">	'eWhichWirelessModule' to tell which module to update </param>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool UpdateZBsw( WirelessModuleType eWhichWirelessModule );
	bool GetFileList( CArray<CString> *parList, CArray<UINT32> *parSize );
	bool DischargeCurrentProject();
	bool DeleteAllQMandLOG();
	
	bool DeleteTASCOPEFile( CArray<CString> *parList );
	bool DeleteTASCOPEFile( CString fn );
	bool DeleteTASCOPEFile( const char * pfn );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Backup tascope files. </summary>
	///
	/// <remarks>	Backup tascope files *.tsc and *.bak  </remarks>
	///
	/// <param name="fAll">	true to all. false only the tadsx.tsc is backup</param>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	bool BackupTASCOPEFiles( bool fAll );

	bool ReadFile( CString strSrcFn, CString strTrgFn, bool fUseProgressBarWnd = true );
	bool WriteFile( CString strSrcFn, bool fSecure = false, bool fUseProgressBarWnd = true );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Check for update. </summary>
	///
	/// <remarks>	Alen, 30/09/2010. </remarks>
	///
	/// <param name="bForce">	true to force. </param>
	///
	/// <returns> eTAScopeUpdate
	/// 	eupdFail,
	///		eupdUpdateAvailable,				// An Update is available
	///		eupdNoUpdateNeeded,					// HFT and SFT checked no difference
	///		eupdLocalHftMissing,				// No local SFT available, no update possible
	///		eupdTASCOPEHftMissing,				// TA SCOPE HFT missing, Old version?
	///		eupdIncorrectCRC,					// Problem between SFT and TASCOPE files
	///		eupdLast
	///	</returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	eTAScopeUpdate CheckForUpdate( bool fForce = false );
	
	bool CheckHFTIntegrity( CFileTable *pSFT, CList<CString, CString&> *errorList );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg void OnSize( UINT nType, int cx, int cy );

	// Overrides 'CDlgWizardManager' protected virtual methods.
	virtual bool OnWizFinish();

	bool PortOpen( int i = 0 );
	void PortClose();
	void CleanHFTs();

// Public variables.
public:
	CDlgPanelTAScopeStart		m_DlgPanelStart;
	CDlgPanelTAScopeDownload	m_DlgPanelDownload;
	CDlgPanelTAScopeMaintenance	m_DlgPanelMaintenance;
	CDlgPanelTAScopeUpload		m_DlgPanelUpload;
	CDlgPanelTAScopeUpdate		m_DlgPanelUpdate;
	bool IsComForcedOffLine() const { return m_fTAScopeOffLineForced; }
	void SetComForcedOffLine(bool val) { m_fTAScopeOffLineForced = val; }

// Protected variables.
protected:
	CDlgProgressBar *m_pDlgProgressBar;
	ProgressBarType m_eProgressBarType;

	bool m_fComPortBusy;
	bool m_fRefreshVersion;
	bool m_fInterruptRequested;
	int m_iComNumber;
	bool m_fBootMode;
	CSerialPort	*m_pCom;
	CArray<BYTE> m_arData;
	double m_dProgress;
	static int m_iComIndex;
	bool m_fTAScopeOffLineForced;
	// Version strings.
	CArray<CString, CString> m_CurrentVersion;
	unsigned int m_uiDSversion;
	ServiceManager::eDPSComStatus m_DpsComStatus;
	UINT_PTR m_nTimer;
	
	WirelessModuleType m_eHHWirelessModuleType;
	bool m_bNewDpSVisioFirmware;

	CFileTable *m_pSFT;		// TA-SCOPE file table into the PC
	CFileTable *m_pHFT;		// TA-SCOPE file table into the TASCOPE
	CFileTable *m_pFT;		// File table with new files that should be transfered into the TA-SCOPE
};
