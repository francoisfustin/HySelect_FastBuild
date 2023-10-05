#pragma once


// We now have more than on connection for a pipe.
// It's stricly temporary! Because we don't have yet bypass primary and secondary pipe.
// We just have the case of the bypass at the primary side for the auto-adapting variable flow decoupling circuit.
// See the 'CDS_HydroMod::_VerifyConnection' method to have an overview of all the connections.
#define SINGULARITY_CONNECT_ID				( 0 )
#define SINGULARITY_FIRSTACCESSORY_ID		( SINGULARITY_CONNECT_ID + 10 )
#define SINGULARITY_MAXACCESSORY			( SINGULARITY_FIRSTACCESSORY_ID + 99 )
#define SINGULARITY_NOMOREAVAILABLE			( -1 )

class CPipes
{
	friend class CDS_HydroMod;

public:
	CPipes( CDS_HydroMod *pHM, CDS_HydroMod::eHMObj eLocated );
	virtual ~CPipes();

	class CSingularity
	{
	public:
		short m_nID;						// Unique Singularity Identification 
		IDPTR m_IDPtr;						// Refer to a singularity defined in TADB or a CDB_Pipe
		CString m_strDescription;			//
		CDS_HydroMod::eDpType m_eDpType;	// Type of Dp
		CDS_HydroMod::_uDpVal m_uDpVal;
		IDPTR GetSingulIDPtr();
		CString GetDescription();
	};

public:

	// 'bResetDiversityFactor' is set to 'false' if we must copy diversity factor value (it's the case for example when we do a perfect copy of the 
	// hydromod in the 'CDlgDiversityFactor::_TakeSnapshot' method).
	void Copy( CPipes *pHMpipe, bool bResetDiversityFactor = true );

	// Returns 'true' if both pipes are the same.
	bool Compare( CPipes *pPipes );
	
	CDS_HydroMod *GetpParent() { return m_pParent; }
	IDPTR GetIDPtr();
	CTable *GetPipeSeries();
	bool IsDiversityApplied() { return m_bIsDiversityApplied; }
	IDPTR GetDiversityIDPtr();
	double GetLength() { return m_dLength; }
	double GetRealQ() const { return m_dRealQ; }
	double GetPartialPendingQ() const { return m_dPartialPendingQ; }
	double GetTotalQ() const { return m_dTotalQ; }
	IDPTR GetBestPipeIDPtr();
	CDS_HydroMod::eHMObj GetLocate() { return m_eLocated; }
	double GetPipeDp();
	double GetLinDp();
	double GetVelocity();
	double GetVelocity( double dQ );

	// HYS-1716.
	// Remark: we have two variables why ? Because we have only one pipe definition for example circuit at the primary side,
	// one circuit pipe at secondary side and one distribution supply pipe when we are in the direct return mode.
	CWaterChar *GetpWaterChar( CAnchorPt::PipeLocation ePipeLocation );
	double GetTemperature( CAnchorPt::PipeLocation ePipeLocation );

	void SetPipeSeries( CTable *pSerieTab, bool bSignal = true );
	void SetLocate( CDS_HydroMod::eHMObj eLocate ) { if( m_eLocated != eLocate ) { m_eLocated = eLocate; m_pParent->Modified(); } }
	void SetIDPtr( IDPTR idptr, bool bSignal = true );
	void SetDiversityAppliedFlag( bool bIsDiversityApplied );
	void SetDiversityIDPtr( IDPTR idptr ) { m_DiversityIDPtr = idptr; }
	void SetLength( double dLength, bool bSignal = true );
	void SetRealQ( double dRealQ ); 
	void SetTotalQ( double dTotalQ );
	void SetPartialPendingQ( double dPartialPendingQ ) { m_dPartialPendingQ = dPartialPendingQ; }
	void SetBestPipeIDPtr( IDPTR IDPtr ) { m_IDPtrBestPipe = IDPtr; }
	void SetLinDp( double dLinDp ) { m_dLinDp = dLinDp; }
	void SetVelocity( double dVelocity ) { m_dVelocity = dVelocity; }

	// HYS-1716.
	// Remark: we have supply and return temperatures in the same 'CHMPipes' object because for the moment we have
	// only the distribution supply pipe for the direct return mode (While for the reverse return mode we have 
	// both the distribution supply and return pipe separately).
	// HYS-1724: For a future development, we will try to always have a distribution return pipe even if we are in the
	// direct return mode.
	void SetWaterChar( CWaterChar *pclWaterChar, CAnchorPt::PipeLocation ePipeLocation );
	void SetTemperature( double dTemperature, CAnchorPt::PipeLocation ePipeLocation );

	IDPTR SelectBestPipe(double dQ);
	
	// If user opens a tsp project that contains pipes that are not yet in HySelect, he has two choices: either import new pipes in the
	// 'PIPE_TAB' or temporary use these pipes that will be no more available as soon as HySelect is closed. For this second option, we
	// completely reset the 'PIPE_TAB', fill it with default HySelect pipes and pipes from tsp project. By 'filling' we mean, we create
	// new object and we insert it in 'PIPE_TAB'. That means all IDPTR.MP for pipe are no more correct.
	void UpdateAllPipesIDPtr();

	// 'IsPipeUsed' returns true if the pipe is used, otherwise false.
	// If no pipe size is enter, the function only takes account to the pipe series. If the pipe size exist, its take account for the pipe
	// size for the current pipe series.
	bool IsPipeUsed( IDPTR IDPtrPipeSerie, IDPTR IDPtrPipeSize = _NULL_IDPTR );
	bool IsCompletelyDefined();
	bool IsBestPipe( CData *pData );
	bool IsLocked();
	void SetLock( bool bFlag, bool bResizeNow = true);

	// Used to determine Dp relative to connexion singularity (always first pipe singularity).
	// pPipe is the pipe where the singularity must be computed.
	double GetConnectDp( CDS_HydroMod *pNextHM = NULL );

	// Allow to retrieve the description of the connection.
	// Param: 'bWithConnectionSuffix' set to 'true' to write 'for connection' after the name of the connection.
	// Remarks: description returns here will differ from the content of 'sSingularity::m_strDescription'. In the most case, pipe has only one
	//          connection singularity. Since reverse return mode is possible, we have separated distribution pipe in two 'CPipes' objects, one
	//          for the supply and one for the return. But it's not the case at now for the circuit pipe. There is only one 'CPipes' object and thus
	//          only one connection singularity available (with uid set to 0).
	//          In direct return mode, circuit pipe has the same connection singularity at the inlet and outlet. But it is no more the case for
	//          the reverse return mode. Where circuit pipe can have one of these combinations: "TEE in - ELBOW out"; "TEE in - TEE out" or "ELBOW in - TEE out".
	//          We thus need this method to retrieve the correct description for connection singularity.
	void GetConnectDescription( CString &strDescription, bool bWithConnectionSuffix = false );
	
	eBool3 CheckLinDpMinMax();
	eBool3 CheckVelocityMinMax();

	// Verify type of singularity used for the connection.
	// Can be changed by adding a new circuit in the current branch (Elbow becomes tee) by drag and drop operation.
	void CheckPipeConnection( CString strID );

	// Add an accessory.
	// SingulDpType:
	//   Dp, Kv, or Cv -> only use dValue1.
	//   dzeta         -> dValue1 = dzeta, dValue2 = length.
	//   QDpref        -> dValue1 = Qref, dValue2 = Dpref.
	short AddAccessorySingularity( CString strDescription, CDS_HydroMod::eDpType eSingulDpType, double dValue1, double dValue2 = 0.0 );

	void AddAccessorySingularities( std::vector<CSingularity *> *pvecSingularities );
	
	// Add an accessory.
	// The length is used to calculate the equivalent pipe pressure drop.
	short AddPipeSingularity( CString strDescription, IDPTR PipeIDPtr, double dLength );

	// Add a singularity from TADB.
	short AddConnectionSingularity( LPCTSTR SingulID );

	void RemoveSingularity( short nID );
	void RemoveAllSingularities( bool bExceptConnection = false );

	CString GetSingularityTypeString( CSingularity *pSingularity );

	// Allows to return description of all singularities in a short format (simple line).
	// Remark: - if there is only one singularity, we show its description.
	//         - if there is more than one singularity, we show only the total count.
	// Param: 'bWithConnection' set to 'true' if we must take in count the connection singularity if exist and defined.
	// Param: 'bWithConnectionSuffix' set to 'true' to write 'for connection' after the name of the connection.
	CString GetSingularitiesDescriptionShort( bool bWithConnection = true, bool bWithConnectionSuffix = false );

	// Allow to return the input data for all singularities in a short format (simple line).
	// Remark: - if there is no connection and only one accessory, we show input data for this accessory.
	//         - if there is more than one singularity, we show nothing.
	CString GetSingularitiesInputDataShort( void );

	// Allows to return a list of all singularities in a full format.
	// Param: 'bDescription' set to 'true' if we must add the description input by user.
	// Param: 'bInputData' set to 'true' if we must add input (Dp, Cv/Kv, dZeta, QRef/DpRef or pipe length).
	// Param: 'bDp' set to 'true' if we must show pressure drop in the accessories.
	// Param: 'bWithConnection' set to 'true' if we must show also connection (accessory with the 'SINGULARITY_CONNECT_ID' id).
	// Param: 'iLimitOutput' set to a number to limit the output ('...' will be add on the last line).
	// Remark: output will contain one line for one singularity with details asked (description, input data or/and pressure drop).
	//         If there is more than one line, these ones will be prefixed with a bullet (-> ) to be clearer whern showing.
	CString GetSingularityListFull( bool bDescription, bool bInputData, bool bDp, bool bWithConnection = true, int iLimitOutput = -1 );

	// Allow to retrieve all needed details about a specific singularity in separate strings.
	bool GetSingularityDetails( CSingularity *pSingularity, CString *pstrDescription = NULL, CString *pstrInputData = NULL, CString *pstrDp = NULL );

	void FormatSingularityString( CString &FSstr, CSingularity *pSingularity = NULL, bool bDescription = true, bool bInputData = true, bool bDp = true );
	
	bool GetSingularity( short nID, CSingularity *pSingulularity );
	int GetSingularityCount();
	
	// Compute pressure drop on one singularity.
	// Remark: 'pNextHM' will only needed if singularity is a connection. In this case, the method will call 'GetConnectDp'.
	double GetSingularityDp( CSingularity *pSingularity, CDS_HydroMod *pNextHM = NULL );
	
	// Allow to compute pressure drop on all singularities on the pipe.
	// Param: 'bWithConnection' -> 'true' if we must include connection singularity (see remark) or only other singularities.
	// Param: 'pNextHM' -> is set to allow to improve computing of connection singularity (take into account eventual restriction).
	// Remark: Keep in mind that the first singularity (uid = 0) is the tee connection accessory if next distribution pipe exists 
	//         or an elbow if there is no more distribution pipe after.
	double GetSingularityTotalDp( bool bWithConnection, CDS_HydroMod *pNextHM = NULL );
	int GetFirstSingularity( CSingularity *pSingularity );
	int GetNextSingularity( int iPos, CSingularity *pSingularity );

	// Fill a vector with all singularities.
	// PAY ATTENTION: It's up to you to properly clean vector (with deleting all 'sSingularity' in it) when we have no more need of it.
	//                To help you can call 'ClearSingularityVector' method just below.
	void GetSingularityList( std::vector<CSingularity *> *pvecSingularities, bool bExceptConnect = true );

	// Clear a vector with all singularities in it.
	void ClearSingularityVector( std::vector<CSingularity *> *pvecSingularities );
	
	// Copy all singularities (except connection) to the 'pclDestPipe'.
	void CopyAllSingularities( CPipes *pclDestPipe );

	// Copy all singularities in a raw buffer.
	// Param: 'pbBuffer' is NULL then method returns the size needed to save all singularities.
	//                   is not NULL then method write all singularities in the buffer.
	// Param: 'bExceptConnect' is 'true' if we don't want the connection singularity.                  
	// Remark: It's up to you to allocate (with respect to the size returns by the method) AND deallocate memory .
	int WriteAllSingularitiesToBuffer( BYTE *pbBuffer, bool bExceptConnect = true );

	// Read all singularities from the buffer and add it to the pipe.
	// Remark: original IDs will be changed when added in the pipe in regards to the already existing singularities.
	int ReadAllSingularitiesFromBuffer( BYTE *pbBuffer );

	void ChangeReturnType( CDS_HydroMod::ReturnType eReturnType ) { m_eReturnType = eReturnType; }

	virtual void DropData( OUTSTREAM outf, int *piLevel );

// Public variables.
public:	
	typedef std::map<short, CSingularity *> mapSingularity;
	typedef mapSingularity::iterator SingularityIter;
	std::map<short, CSingularity *> m_mapSingularityList;

protected:
	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );
		
	short _GetFirstFreeID();
	short _AddSingularity( short nID, LPCTSTR SingulID );
	bool _CopySingularity( CSingularity *pSrcSingularity );
	CString _FormatSingularityStringHelper( CSingularity *pSingularity, bool bDescription, bool bInputData, bool bDp, bool bWithBullet );
	CString _GetSingularityDescription( CSingularity *pSingularity );
	CString _GetSingularityInputData( CSingularity *pSingularity );
	CString _GetSingularityDp( CSingularity *pSingularity );
	void _FreeBuffer( CMemFile *pclMemFile );
	CWaterChar *_GetCurrentWaterChar();
	// HYS-1734: For 3-way dividing circuit we could have a valve on the bypass and we should take the supply information.
	bool _IsParentHMScheme3Wdividing();

protected:
	CDS_HydroMod *m_pParent;
	CTADatastruct *m_pTADS;
	IDPTR m_IDPtr;
	IDPTR m_DiversityIDPtr;					// pipe Diversity IDPtr (pipe saved before applying diversity factor)
	CTable *m_pPipeSerie;
	IDPTR m_IDPtrBestPipe;
	double m_dLength;
	double m_dRealQ;						// dRealQ can be lower than TotalQ in the case of pending circuit exist.
	double m_dTotalQ;						// dTotalQ contains flow when diversity is set to -1 (no diversity !)
	double m_dPartialPendingQ;				// Used during computing when some pending circuits exists.
	double m_dLinDp;
	double m_dVelocity;
	CDS_HydroMod::eHMObj m_eLocated;

	// Before, we checked the difference between real and total Q. But for double precision limitation due to
	// the compiler (the way it transforms binary value into readable double value), we had sometime a lack of
	// precision (below 16th decimal). Even if real and total Q were the same, a little decimal for example at the 18th 
	// position could change the test.
	bool m_bIsDiversityApplied;

	// Remark about this variable. Now we have the possibility to work in a direct or a reverse return mode. Previously and 
	// specifically for a distribution pipe we computed the pressure drop for one connection and we multiplied by 2. So if a 
	// circuit A2 had a tee connection with the next circuit A3 for example, the total pressure drop through the connection 
	// was in fact the pressure drop through the tee multiply by 2. It is always the case if we work in direct return mode.
	// But for the reverse return mode, distribution pipe are well separated into two 'CPipes' object. Each one has its own internal
	// variables. That means if we need pressure drop through the tee connection for a distribution supply pipe, it's only for it.
	// If we need the pressure drop through the tee in the return path, you must call method for the distribution return pipe.
	CDS_HydroMod::ReturnType m_eReturnType;

	// HYS-1716: we need to add sypply and return temperatures in the pipe.
	// This allows us to correcly compute the power when needed (For example on a module with a TA-Smart in power control mode).
	// Remark: we have two variables why ? Because we have only one pipe definition for example circuit at the primary side,
	// one circuit pipe at secondary side and one distribution supply pipe when we are in the direct return mode.
	CWaterChar m_clSupplyWaterChar;
	CWaterChar m_clReturnWaterChar;
};
