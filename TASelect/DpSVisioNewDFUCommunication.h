#pragma once




class CDpSVisioNewDFUInformation : public CDpSVisioBaseInformation
{
public:
	virtual void Reset( void )
	{
		CDpSVisioBaseInformation::Reset();

		m_bIsConnected = false;
		m_strDeviceName = _T("Unknown");
		m_strIdentifier = _T("Unknown");
		m_strSerialNumber = _T("Unknown");
		m_strFirmwareRevision = _T("Unknown");
		m_strShortFirmwareRevision = _T( "" );
		m_dFirmwareVersion = 0.0;
	}
};

class CDpSVisioNewDFUCommunication : public CDpSVisioBaseCommunication
{
public:
	enum _ProcessCode
	{
		PC_NothingRunning,
		PC_DFUErase,
		PC_DFUUpgrade
	};

	enum _ProcessStateEngine
	{
		PSE_NotStarted,
		
		PSE_RunOneTargetInFile,
		PSE_CheckAlternateSetting,
		PSE_InitOneElement,
		
		PSE_EraseOneElementOneChunk,
		PSE_EraseOneElementOneChunkOnePage,
		PSE_EraseOneElementOneChunkLastPage,
		
		PSE_UpgradeOneElementOneChunk,
		PSE_UpgradeOneElementOneChunkFirstGetStatus,
		PSE_UpgradeOneElementOneChunkWaitEndOfWritting,
		PSE_UpgradeOneElementOneChunkFinished,
		
		PSE_RunAllTargetsInFileFinished,
		PSE_Finished,
		PSE_UpdateError,
		PSE_WaitingToBeKilled
	};

	enum ProcessError
	{
		PE_NoError,
		PE_DeviceDisconnected,
		PE_DeviceChanged,
		PE_DFUFileNoValidTargetSignature,
		PE_CantSetAlternateSetting,
		PE_DFUDeviceLastPageNotWritable,
		PE_InternalError,
	};

	enum ProcessInternalError
	{
		PIE_DFUSeMemcpyError,
		PIE_FileTooSmallForElementSize,
		PIE_DFUCantExecuteSpecialCommand,
		PIE_DFUCantSegmentNotErasable,
		PIE_DFUCantWriteChunkElement,
		PIE_DFUErrorWhenReadingState,
		PIE_DFUErrorWhenReadingStatus,
		PIE_DFUErrorWhenClearingState,
		PIE_DFUErrorToManyRetryWhenUpgrading,
		PIE_DFUErrorWhenWritingChunkElement,
		PIE_DFUErrorBadFirmwareStateMachine,
	};

public:
	CDpSVisioNewDFUCommunication( CWnd *pParent, _DFUUtilWorkVariables *prNewDFUSeVariables );
	virtual ~CDpSVisioNewDFUCommunication();

	// Overrides pure public virtual methods of 'CDpSVisioBaseCommunication'.
	virtual void DpSVisioConnected( CString strDeviceName );
	virtual void DpSVisioDisconnected( CString strDeviceName );

	virtual int ReadDpSVisioCommand( CListBox *pOutputList = NULL ) { return -1; }
	virtual int SendDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize = 0, UINT8 *pData = NULL, CListBox *pOutputList = NULL ) { return -1; }
	virtual int SendDpSVisioCommandLP( UINT16 uiCommand, LPARAM lpParam = (LPARAM)0, CListBox *pOutputList = NULL );
	virtual int SendAndReadDpSVisioCommand( UINT16 uiCommand, UINT8 uiSize = 0, UINT8 *pData = NULL, CListBox *pOutputList = NULL ) { return -1; }

	static UINT ThreadProcessRunning( LPVOID pParam );

// Protected methods.
private:
	// Overrides pure protected virtual methods of 'CDpSVisioBaseCommunication'.
	virtual void Reset();
	virtual bool RetrieveDpSVisioInformation( CString strDeviceName );

// Private methods.
private:
	int _DFUStartErase( CString *pstrDFUFileName, CListBox *pOutputList );
	int _DFUCheckErase( BYTE *pPercentCompleted, CListBox *pOutputList );
	int _DFUStartUpgrade( CListBox *pOutputList );
	int _DFUCheckUpgrade( BYTE *pPercentCompleted, CListBox *pOutputList );
	int _DFUDetach( CListBox *pOutputList );

	void _ProcessRunning( void );
	void _StopThreadProcessRunning( void );
	_ProcessStateEngine _SetProcessError( ProcessError eProcessError, DWORD dwInternalError, _ProcessStateEngine eProcessStateEngine = PSE_UpdateError );
	unsigned int _quad2uint( unsigned char *p );
	bool _CheckNumberOfPages( void );

// Private variables.
private:
	CDpSVisioDFUCommand m_clDpSVisioDFUCommand;
	CDpSVisioNewDFUInformation m_clDpSVisioNewDFUInformation;
	CString m_strFileName;
	_DFUUtilWorkVariables *m_prNewDFUSeVariables;
	struct dfu_file m_DFUFile;
	
	int m_iOperationCode;
	uint64_t m_ui64StartTime;
	int m_ifunc_dfu_transfer_size;
	int m_iBytesRemaining;
	int m_iStartBytesRemaining;
	uint8_t *m_pdata;
	uint8_t *m_pDataStart;
	uint8_t m_elementheader[8];
	uint8_t m_targetprefix[274];
	int m_dwNbElements;
	int m_ielement;
	unsigned int m_dwElementAddress;
	unsigned int m_dwElementSize;
	int m_p;
	int m_iProgressPercent;
	int m_iPreviousProgressPercent;
	struct dfu_status m_dst;
	int m_iTargets;
	int m_iLoopTarget;
	int m_iPageSize;
	int m_iChunkSize;
	unsigned int m_uiAddress;
	unsigned int m_uiEraseAddress;
	unsigned int m_uiLastErasedPage;
	struct dfu_if *m_padif;
	unsigned int m_uiCurrentPage;
	unsigned int m_uiTotalPagesToErase;
	unsigned int m_uiTotalChunksToWrite;
	DWORD m_PollTime;

	CWinThread *m_pProcessThread;
	CEvent *m_pclStopThreadEvent;
	CEvent *m_pclThreadStoppedEvent;
	CRITICAL_SECTION m_CriticalSection;
	_ProcessStateEngine m_eProcessStateEngine;

	struct ProgressStatus
	{
		bool m_bProcessStatus;
		ProcessError m_eProcessError;
		DWORD m_dwDFUInternalErrorCode;
	};

	ProgressStatus m_rProcessStatus;
};
