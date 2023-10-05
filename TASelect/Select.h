#pragma once


#include <map>

//////////////////////////////////////////////////////////////////////////////////
class CSelectPipe
{
public:
	CSelectPipe( CProductSelelectionParameters *pclProductSelParams );
	virtual ~CSelectPipe() {}

public:
	CString GetPipeID() { return m_strPipeID; }
	CDB_Pipe *GetpPipe();

	CProductSelelectionParameters *GetProductSelParams() { return m_pclProductSelParams; }
	double GetLinDp() { return m_dLinDp; }
	bool GetLinDpFlag()	{ return m_LinDpFlag; }
	double GetU() { return m_dU; }
	bool GetUFlag() { return m_UFlag; }
	bool GetTpFlag() { return m_TpFlag;	}
	int GetDNKey() { return m_iPipeDNKey; }

	void SetPipeID( CString strPipeID ) { m_strPipeID = strPipeID; }
	void SetLinDp( double dLinDp ) { m_dLinDp = dLinDp;	}
	void SetLinDpFlag( bool LinDpFlag ) { m_LinDpFlag = LinDpFlag; }
	void SetU( double dU ) { m_dU = dU; }
	void SetUFlag( bool UFlag ) { m_UFlag = UFlag; }
	void SetTpFlag( bool TpFlag ) { m_TpFlag = TpFlag; }
	void SetDNKey( int iDNKey ) { m_iPipeDNKey = iDNKey; }

	virtual void Clean();

	void CopyFrom( CSelectPipe *pPipe );
	void Copy( CSelectPipe *pTo ) { pTo->CopyFrom( this ); }
	
	// Disk IO.
	void Write( OUTSTREAM outf );
	void Read( INPSTREAM inpf );

// Protected variables
protected:
	CProductSelelectionParameters *m_pclProductSelParams;
	CString m_strPipeID;
	double m_dLinDp;		// Linear Dp
	bool m_LinDpFlag;		// Linear Dp validity flag
	double m_dU;			// Water velocity
	bool m_UFlag;			// Water velocity validity flag
	bool m_TpFlag;			// Temperature and pressure validity flag
	int m_iPipeDNKey;
};

class CProductSelelectionParameters;
class CSelectPipeList
{
public:
	typedef std::pair<int, CSelectPipe> PipePair;
	typedef std::multimap<int, CSelectPipe, std::greater<int> > PipeMMap;

public:
	CSelectPipeList();
	virtual ~CSelectPipeList();

	// HYS-1147: if 'pclWaterChar' is defined we use this fluid characteristics instead of the ones found in the 'pclProductSelectionParameters'
	int SelectPipes( CProductSelelectionParameters *pclProductSelectionParameters, double dFlow, bool bLimitToMinLinDpAndMinVelocity = true, 
			bool bBestPipeOnly = true, int iPipeSizeAbove = 0, int iPipeSizeBelow = 0, CWaterChar *pclWaterChar = NULL );

	int GetPipeNumbers( void ) { return (int)m_PipeMMap.size(); }

	CSelectPipe *GetFirstSelectPipe( void );
	CSelectPipe *GetNextSelectPipe( void );

	CSelectPipe *GetBestSelectPipe( void );
	CDB_Pipe *GetBestPipe( void );

	bool GetPipeSizeBound( IDPTR *pSmallestPipeIDPtr = NULL, IDPTR *pLargestPipeIDPtr = NULL );

	// Process to a GetPipeSizeBound() to update these values.
	IDPTR GetSmallestPipe() { return m_SmallestPipeIDPtr; }
	CDB_Pipe *GetpSmallestPipe() { return dynamic_cast<CDB_Pipe *>( ( CData * )m_SmallestPipeIDPtr.MP ); }

	IDPTR GetLargestPipe() { return m_LargestPipeIDPtr;	}
	CDB_Pipe *GetpLargestPipe() { return dynamic_cast<CDB_Pipe *>( ( CData * )m_LargestPipeIDPtr.MP ); }

	void GetMatchingPipe( int iSizeKey, CSelectPipe &selPipe );
	int GetMatchingPipeSize( int iSizeKey );

	virtual void Clean();

	PipeMMap *GetpPipeMap() { return &m_PipeMMap; }

	void CopyFrom( CSelectPipeList *pclSelectPipeList );

// Private variables.
private:
	PipeMMap m_PipeMMap;
	PipeMMap::iterator m_Iter;
	IDPTR m_SmallestPipeIDPtr;
	IDPTR m_LargestPipeIDPtr;
	CSelectPipe *m_pclBestPipe;
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected product base
//////////////////////////////////////////////////////////////////////////////////
class CSelectedBase
{
public:
	enum SelectedBaseType
	{
		SBT_Valve,
		SBT_Separator,
		SBT_SafetyValve,
		SBT_BlowTank
	};

	CSelectedBase( SelectedBaseType eSelectedBaseType );
	virtual ~CSelectedBase() {}

	typedef enum Flags
	{
		eCommonFlagFirst =		0x00000001,
		eBest =					eCommonFlagFirst,					// Best technical choice.
		eDp =	( eBest << 1 ),										// Pressure drop validity flag.
		eDpSignal =	( eDp << 1 ),									// Dp signal validity flag.
		eTemperature =	( eDpSignal << 1 ),							// Temperature validity flag.
		ePN =	( eTemperature << 1 ),								// PN (Static pressure) validity flag.
		eNotPriority =	( ePN << 1 ),								// This selection is not a priority.
		eCommonFlagLast =	( eNotPriority ),

		eValveFlagsFirst =	( eCommonFlagLast << 1 ),
		eValveSetting =	( eValveFlagsFirst ),						// Setting validity flag
		eValveMaxSetting =	( eValveSetting << 1 ),					// True if the setting is the max setting of the valve
		eValveHalfODp =	( eValveMaxSetting << 1 ),					// Pressure drop Half Open
		eValveFullODp =	( eValveHalfODp << 1 ),						// Pressure drop Fully Open flag
		eValveDpMin =	( eValveFullODp << 1 ),						// DpController: Dp min not found or give a flow too big!
		eValveDpToSmall =	( eValveDpMin << 1 ),					// DpController
		eValveDpToLarge =	( eValveDpToSmall << 1 ),				// DpController
		eValveDpAlternative =	( eValveDpToLarge << 1 ),			// DpController: Dp to small or to large but alternative can exist.
		eValveDpCHide =	( eValveDpAlternative << 1 ),				// DpController: If Dp is not correct and technical parameter is set to hide which does not satisfy the Min. Dp limit
		eValveBatchAlter = ( eValveDpCHide << 1 ),					// Batch selection: set to 1 if result is an alternative solution in regards to user choice with combos.
		eValveBatchDpCForceMvInSec = ( eValveBatchAlter << 1 ),		// Batch selection: DpC selection for Mv to be in secondary.
		eValveFlagsLast =	( eValveBatchDpCForceMvInSec ),
		eValveGlobal =	( eValveSetting | ePN | eTemperature | eValveDpMin | eValveDpToSmall | eValveDpToLarge ),

		eSeparatorFlagsFirst =	( eValveFlagsLast << 1 ),
		eSeparatorFlagsLast =	( eSeparatorFlagsFirst ),

		efAll =					0xFFFFFFFF
	};

	// Allow to link a flag with a error code (to facilitate management with 'OnTextTipFetch' in each RViewSSelXXX class).
	typedef std::map< Flags, int > mapFlagsInt;
	typedef mapFlagsInt::iterator mapFlagsIntIter;

	IDPTR GetProductIDPtr() { return m_IDPtr; }
	CData *GetpData() { return ( CData * )m_IDPtr.MP; }
	bool IsFlagSet( Flags flag ) { return ( GetFlag( flag ) ? 1 : 0 ); }
	
	void SetProductIDPtr( IDPTR idptr ) { m_IDPtr = idptr; }

	// Allow to set a flag to 'true' or 'false'.
	// Param: 'iErrorCode': if different of -1, it contains an error code linked with this flag.
	// Remark: 'iErrorCode' is to facilitate management with 'OnTextTipFetch' in each RViewSSelXXX class.
	void SetFlag( Flags eFlag, bool bSet, int iErrorCode = -1 );

	// Param: 'iErrorCode': if defined and if an error code exists for this flag then method returns error code with this pointer.
	bool GetFlag( Flags eFlag, int *piErrorCode = NULL );
	int GetFlagError( Flags eFlag );
	void ClearAllFlags( void ) { m_ulFlags = 0; }

	// Setter.
	void SetDp( double dDp ) { m_dDp = dDp; }

	// Getter.
	double GetDp( void ) { return m_dDp; }
	
	virtual void CopyFrom( CSelectedBase *pFromSelectedProduct );
	void Copy( CSelectedBase *pFromSelectedProduct ) { pFromSelectedProduct->CopyFrom( (CSelectedBase *)this ); }

	mapFlagsInt *GetpmapFlagsInt() { return &m_mapFlagsErrorCode; }

	SelectedBaseType GetSelectedBaseType( void ) { return m_eSelectedBaseType; }

protected:
	SelectedBaseType m_eSelectedBaseType;

	IDPTR m_IDPtr;		// Product IDPTR.
	unsigned long m_ulFlags;

	double m_dDp;

	mapFlagsInt m_mapFlagsErrorCode;
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected valve.
//////////////////////////////////////////////////////////////////////////////////
class CSelectedValve : public CSelectedBase
{
public:
	CSelectedValve();
	virtual ~CSelectedValve() {}

public:
	// Setter.
	void SetDpSignal( double dDp ) { m_dDpSignal = dDp; }
	void SetDpQuarterOpen( double dDpQuarterOpen ) { m_dDpQuarterOpen = dDpQuarterOpen; }
	void SetDpHalfOpen( double dDpHalfOpen ) { m_dDpHalfOpen = dDpHalfOpen; }
	void SetDpFullOpen( double dDpFullOpen ) { m_dDpFullOpen = dDpFullOpen; }
	void SetH( double dh ) { m_dh = dh; }
	void SetBestDelta( double dBestDelta ) { m_dBestDelta = dBestDelta; }

	// Setter for DpController (see CSelectDpCList::SelectDpC()).
	void SetDpMin( double dDpMin ) { m_dDpMin = dDpMin;	}
	void SetRequiredDpMv( double dReqDpMv )	{ m_dReqDpMv = dReqDpMv; }

	// Setter for combined DpC and Bal. & ctrl. valve (See CSelectDpCBCVList::Select()).
	void SetHMin( double dHMin ) { m_dHMin = dHMin;	}

	// Getter
	double GetDpSignal( void ) { return m_dDpSignal; }
	double GetDpQuarterOpen( void )	{ return m_dDpQuarterOpen; }
	double GetDpHalfOpen( void ) { return m_dDpHalfOpen; }
	double GetDpFullOpen( void ) { return m_dDpFullOpen; }
	double GetH( void ) { return m_dh; }
	double GetBestDelta( void ) { return m_dBestDelta; }

	// Getter for DpController (see CSelectDpCList::SelectDpC()).
	double GetDpMin( void ) { return m_dDpMin; }
	double GetRequiredDpMv( void ) { return m_dReqDpMv;	}

	// Getter for combined DpC and Bal. & ctrl. valve (See CSelectDpCBCVList::Select()).
	double GetHMin( void ) { return m_dHMin; }

	// Overrides 'CSelectedBase' methods.
	virtual void CopyFrom( CSelectedBase *pFromSelectedProduct );

protected:
	double m_dDpSignal;
	double m_dDpQuarterOpen;
	double m_dDpHalfOpen;
	double m_dDpFullOpen;
	double m_dh;
	double m_dBestDelta;

	// Variables needed by DpController selection (see CSelectDpCList::SelectDpC())
	double m_dDpMin;
	double m_dReqDpMv;		// Dp required on valve (in case of Dp can't alone assume Dp required on branch)

	// Variables needed by combined DpC and Bal. & ctrl. valve selection (see CSelectDpCBCVList::SelectDpC())
	double m_dHMin;
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected separator.
//////////////////////////////////////////////////////////////////////////////////
class CSelectedSeparator : public CSelectedBase
{
public:
	CSelectedSeparator() : CSelectedBase( CSelectedBase::SBT_Separator ){}
	virtual ~CSelectedSeparator() {}

public:
	// Overrides 'CSelectedBase' methods.
	virtual void CopyFrom( CSelectedBase *pFromSelectedProduct );

protected:
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected safety valve.
//////////////////////////////////////////////////////////////////////////////////
class CSelectedSafetyValve : public CSelectedBase
{
public:
	CSelectedSafetyValve();
	virtual ~CSelectedSafetyValve() {}

	int GetQuantityNeeded() const	{ return m_iQuantityNeeded; }
	void SetQuantityNeeded( int val )	{ m_iQuantityNeeded = val; }

public:
	// Overrides 'CSelectedBase' methods.
	virtual void CopyFrom( CSelectedBase *pFromSelectedProduct );

private:
	int	m_iQuantityNeeded;
};

//////////////////////////////////////////////////////////////////////////////////
// Base class for all 'CSelectList'.
//////////////////////////////////////////////////////////////////////////////////
class CSelectedProductList
{
public:
	CSelectedProductList() {}
	virtual ~CSelectedProductList();

	int GetCount();

	typedef std::pair<double, CSelectedBase *> ProductPair;
	typedef std::multimap<double, CSelectedBase *, std::greater<double> > ProductMMapKeyDecreasing;
	void CreateKeyAndInsertInMap( ProductMMapKeyDecreasing *pMap, CDB_TAProduct *pSourceProduct, CSelectedBase *pclProductToInsert );

	CSelectPipeList *GetSelectPipeList( void ) { return &m_clSelectPipeList; }

	virtual void Clean();
	virtual void CopyFrom( CSelectedProductList *pclSelectedProductList );

	ProductMMapKeyDecreasing m_MMapKeyDecreasing;
	ProductMMapKeyDecreasing m_PreselMMapKeyDecreasing;
	ProductMMapKeyDecreasing::const_iterator m_cIter;

	template<typename T>
	T *GetFirst()
	{
		m_cIter = m_MMapKeyDecreasing.begin();
		return GetNext<T>();
	}

	template<typename T>
	T *GetNext()
	{
		T *pSelectedTAP = ( T * )NULL;

		if( m_cIter != m_MMapKeyDecreasing.end() )
		{
			pSelectedTAP = dynamic_cast<T *>( m_cIter->second );
			m_cIter++;
		}

		return pSelectedTAP;
	}

	template<typename T>
	T *GetTAP( IDPTR ProductIDPtr )
	{
		T *pReturn = NULL;

		for( ProductMMapKeyDecreasing::const_iterator iter = m_MMapKeyDecreasing.begin(); iter != m_MMapKeyDecreasing.end() && NULL == pReturn; ++iter )
		{
			if( 0 == IDcmp( ProductIDPtr.ID, iter->second->GetProductIDPtr().ID ) )
			{
				pReturn = dynamic_cast<T *>( iter->second );
			}
		}

		return pReturn;
	}

	virtual void FillProductListInRankEx( CRankEx *pList ) {}

	ProductMMapKeyDecreasing *GetpValveMap() { return &m_MMapKeyDecreasing; }

// Public variables.
public:
	// mapPriority store for each size the minimum priority level (most important)
	std::map<int, int> m_mapPriority;
	
// Protected variables.
protected:
	CSelectPipeList m_clSelectPipeList;
};

//////////////////////////////////////////////////////////////////////////////////
//	Base class for all individual selections.
//////////////////////////////////////////////////////////////////////////////////
class CIndividualSelectBaseList : public CSelectedProductList
{
public:
	CIndividualSelectBaseList();
	virtual ~CIndividualSelectBaseList();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Override the 'CSelectedProductList' virtual public methods.
	// This method erases all data that are no more needed included the pipe list in the 'CSelectedProductList' base class.
	virtual void Clean();

	virtual void CopyFrom( CSelectedProductList *pclSelectedProductList );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected Balancing Valve container contain all balancing valve that match selection criterion
//////////////////////////////////////////////////////////////////////////////////
class CIndSelBVParams;
class CSelectList : public CIndividualSelectBaseList
{
public:
	// enum to facilitate management with 'OnTextTipFetch' in each RViewSSelXXX class (see 'CSelectedTAP::SetFlag').
	enum BvFlagError
	{
		DpBelowMinDp = 0,			// Valve found but Dp is below the minimal pressure drop (defined in technical parameter). (IDS_SSHEETSSEL_DPERRORL)
		DpAboveMaxDp,				// Valve found but Dp is above the maximal pressure drop (defined in technical parameter). (IDS_SSHEETSSEL_DPERRORH)
		DpToReachTooLow,			// User has entered a Dp but it's impossible to reach this value with valve. (IDS_SSHEETSSEL_DPERRORNOTF)
		DpQOTooLow,					// User has not entered a Dp and Dp at quarter position is already below the minimal recommended pressure drop.
		//   That means by closing valve, value will decrease below 3KPa. (IDS_SSHEETSSELBV_DPQOTOOLOWERROR)
		DpFOTooHigh,				// User has not entered a Dp, Dp at quarter position is above 3KPa, Dp at full position is bigger that 3KPa and
		//   we have not found opening giving 3KPa. (IDS_SSHEETSSELBV_DPFOTOOLOWERROR)
		BvFlagErrorLast
	};

	CSelectList();
	virtual ~CSelectList();
	virtual void Clean();

	// Returns 0 if no result.
	int GetManBvList( CIndSelBVParams *pclIndSelBVParam, bool bForceFullOpening = false );
	
	// Needed for 'CSelectDpCList' to specify where must be located the balancing valve.
	void SetMvLoc( eMvLoc val ) { m_eSelMvLoc = val; }

	// Temporary helper to convert a selection made by HydroCalc mode to a selection made by individual selection.
	// Remark: it's for the batch selection. We launch a selection in hydro calc mode but there is more than one solution. User double clicks
	//         on the valve, we must display list of valves in the 'RView'. But the right view works on a 'CSelectList'.
	void AddTAP( CDB_TAProduct *pSourceProduct, CSelectedValve *pclSelectedTAP );

// Protected methods.
protected:
	void CleanPreselectionList( CProductSelelectionParameters *pclProductSelectionParameters );

// Protected variables.
protected:
	double m_dPpn;
	bool m_bForceFullOpening;

	// Used for Hub Selection
	bool m_bForHub;
	bool m_bForHubStation;
	
	// Mv location for DpC selection.
	eMvLoc m_eSelMvLoc;

	// Used for Trv Selection
	bool *m_pbDiffDpTrv;
	double *m_pdMaxDpTrv;
	double m_dSelDpTot;
	bool m_bUseWithThrmHead;

	struct sBest
	{
		union
		{
			double dBestDp;								// Largest Dp below ValvMin2Dp (6 kPa)
			double dBestKvs;
		};
		double dBestDpBelowMin;							// Largest Dp below ValvMinDp (3 kPa in principle)
		double dBestDpAboveMin2;						// Smallest Dp above ValvMin2Dp (6 kPa)
		double dBestDelta;
		struct sBest()
		{
			dBestDp = 0.0;
			dBestDpBelowMin = 0.0;
			dBestDpAboveMin2 = DBL_MAX;
			dBestDelta = DBL_MAX;
		}
		CSelectedValve *pSelV;
	};
	sBest m_rBest;

// Private methods.
private:
	int _GetList( CIndSelBVParams *pclIndSelBVParam, bool bForceFullOpening, bool bForTrv );

	bool _SelectBV( CIndSelBVParams *pclIndSelBVParam, CDB_TAProduct *pTAP, bool &bValidFound );
	bool _SelectCS( CIndSelBVParams *pclIndSelBVParam, CDB_TAProduct *pTAP, bool &bValidFound );
	bool _SelectVV( CIndSelBVParams *pclIndSelBVParam, CDB_TAProduct *pTAP, bool &bValidFound );
	bool _SelectFO( CIndSelBVParams *pclIndSelBVParam, CDB_TAProduct *pTAP, bool &bValidFound );
	bool _SelectTrv( CIndSelBVParams *pclIndSelBVParam, CDB_TAProduct *pTAP, bool &bValidFound );

	void _ComputeBestBV( CIndSelBVParams *pclIndSelBVParam );
	void _ComputeBestCS( CIndSelBVParams *pclIndSelBVParam );
	void _ComputeBestVV( CIndSelBVParams *pclIndSelBVParam );
	void _ComputeBestFO( CIndSelBVParams *pclIndSelBVParam );
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected control valve container contains all control valves for a given Q/Dp/...
//////////////////////////////////////////////////////////////////////////////////
class CSelectCtrlList : public CSelectList
{
public:
	CSelectCtrlList() : CSelectList() {}
	virtual ~CSelectCtrlList() {}
};

class CIndSelCVParams;
class CSelectCVList : public CSelectCtrlList
{
public:
	CSelectCVList();
	virtual ~CSelectCVList() {}

	// Pipe Selection must be done before CV selection
	// Select control valve for a given Q, Kvs or Dp
	// based on a rank list that contains CV matching selection parameters as Type, Connection, ...
	// Boolean bSizeShiftProblem is set to true if there is a problem with allowed valves sizes boundaries.
	int SelectQ( CIndSelCVParams *pclIndSelCVParams, bool *pbSizeShiftProblem );
	int SelectCvKvs( CIndSelCVParams *pclIndSelCVParams, bool *pbSizeShiftProblem );
	int SelectCvDp( CIndSelCVParams *pclIndSelCVParams, bool *pbSizeShiftProblem );

	double GetMinDpMinCv( void ) { return m_dMinDpMinCv; }

// Private methods.
private:
	int _Select( CIndSelCVParams *pclIndSelCVParams, bool *pbSizeShiftProblem );

// Private variables.
private:
	double m_dMinDpMinCv;
};

class CIndSelPIBCVParams;
class CSelectPICVList : public CSelectCtrlList
{
public:
	CSelectPICVList() : CSelectCtrlList() {}
	virtual ~CSelectPICVList() {}

	// Pipe Selection must be done before PICV selection
	// Select control valve for a given Q
	// based on a rank list that contains PICV matching selection parameters as Type, Connection, ...
	// Boolean bSizeShiftProblem is set to true if there is a problem with allowed valves sizes boundaries.
	int SelectQ( CIndSelPIBCVParams *pclIndSelPIBCVParams, bool *pbSizeShiftProblem );
};

class CIndSelBCVParams;
class CSelectBCVList : public CSelectCtrlList
{
public:
	CSelectBCVList();
	virtual ~CSelectBCVList() {}

	virtual void Clean();

	// Pipe Selection must be done before BCV selection.
	// Select control valve for a given Q.
	// Based on a rank list that contains BCV matching selection parameters as Type, Connection, ...
	// Boolean pbSizeShiftProblem is set to true if there is a problem with allowed valves sizes boundaries.

	// Allow to select balancing and control valve for a given flow.
	// Param: 'pBCVList' is the list in which will be saved valves.
	// Param: 'pbSizeShiftProblem': is set to 'true' if there is a problem with allowed valves sizes boundaries.
	// Param: 'CtrlType' determine what type of control user wants to use with valve (On/Off, proportional or 3 points).
	// Param: 'dSelDp' is the Dp that user wants on the valve or -1 if we must use technical parameters to do the selection.
	// Param: 'fForceFullOpening' set to true if we want only
	int SelectQ( CIndSelBCVParams *pclIndSelBCVParams, bool *pbSizeShiftProblem, bool bForceFullOpening = false );

// Private variables.
private:
	double m_dPpn;
	bool m_bForceFullOpening;
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected smart control valve container contains all smart control valves for a given Q/Dp/...
//////////////////////////////////////////////////////////////////////////////////
class CIndSelSmartControlValveParams;
class CSelectSmartControlValveList : public CIndividualSelectBaseList
{
public:
	CSelectSmartControlValveList();
	virtual ~CSelectSmartControlValveList() {}

	// Pipe Selection must be done before smart control valve selection.
	// Select smart control valve for a given Q.
	// Based on a rank list that contains smart control valve matching selection parameters as body material, connection, ...
	// Boolean bSizeShiftProblem is set to true if there is a problem with allowed valves sizes boundaries.
	int SelectSmartControlValve( CIndSelSmartControlValveParams *pclIndSelSmartControlValveParams, bool *pbSizeShiftProblem, bool &bBestFound );

// Private variables.
private:
	int m_iSizeShiftUp;
	int m_iSizeShiftDown;
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected smart differential controller container contains all smart DpC for a given Q/Dp/...
//  HYS-1937: TA-Smart Dp - 04 - Individual selection: left tab.
//////////////////////////////////////////////////////////////////////////////////
class CIndSelSmartDpCParams;
class CSelectSmartDpCList : public CIndividualSelectBaseList
{
public:
	CSelectSmartDpCList();
	virtual ~CSelectSmartDpCList() {}

	// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view
	virtual void Clean();

	// HYS-1938: TA-Smart Dp - 05 - Individual selection: right view
	// Pipe Selection must be done before smart DpC selection.
	// Select smart DpC for a given Q.
	// Based on a rank list that contains smart DpC matching selection parameters as body material, connection, ...
	// Boolean bSizeShiftProblem is set to true if there is a problem with allowed valves sizes boundaries.
	int SelectSmartDpC( CIndSelSmartDpCParams *pclIndSelSmartDpCParams, bool *pbSizeShiftProblem, bool &bBestFound );

	// Select Dp sensor products linked to the selected smart Dp controller by a set.
	int SelectDpSensorSet( CIndSelSmartDpCParams *pclIndSelSmartDpCParams, std::set<CDB_Set *> *pSet, CSelectedValve *pSelSmartDpC );

	// HYS-1992: Select DpSensor fictif set.
	int SelectDpSensor( CIndSelSmartDpCParams* pclIndSelSmartDpCParams, CTable* pTableSensorGr, CSelectedValve* pSelSmartDpC );

	// These two methods are called by 'RViewSSelSmartDpC' to adapt 'm_dDpToStab' with the user choice for regulating valve.
	void SetDpToStab( double dDpToStab ) { m_dDpToStab = dDpToStab; }
	double GetDpToStab() const { return m_dDpToStab; }
	bool IsGroupDpbranchChecked() { return m_bIsGroupDpbranchChecked; }
	
	CRank *GetDpSensorSelected() { return &m_DpSensorList; }

// Private variables.
private:
	int m_iSizeShiftUp;
	int m_iSizeShiftDown;
	bool m_bIsGroupDpbranchChecked;
	double m_dDpToStab;
	CRank m_DpSensorList;
};

class CSelectCvTypeLimits
{
public:
	CSelectCvTypeLimits()
	{
		m_pCv = NULL;
		m_bAbove = INT_MAX;
		m_bBelow = 0.0;
	}
	
	~CSelectCvTypeLimits() {}
	
	double GetLimitAbove() { return m_bAbove; }
	double GetLimitBelow() { return m_bBelow; }
	
	void SetLimitAbove( double dAbove ) { m_bAbove = dAbove; }
	void SetLimitBelow( double dBelow ) { m_bBelow = dBelow; }

// Public variables.
public:
	CDB_ControlValve *m_pCv;
	double m_bAbove;
	double m_bBelow;
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected DpC Controller container
//////////////////////////////////////////////////////////////////////////////////
class CIndSelDpCParams;
class CIndSelBVParams;
class CSelectDpCList : public CSelectList
{
public:
	CSelectDpCList();
	virtual ~CSelectDpCList();

	virtual void Clean();

	// Pipe Selection must be done before DpC selection
	int SelectDpC( CIndSelDpCParams *pclIndSelDpCParams, bool &bValidFound, bool bTestDplmin = true );
	
	int SelectManBV( CIndSelDpCParams *pclIndSelDpCParams, CSelectedValve *pSelDpC );
	int SelectManBVSet( CIndSelDpCParams *pclIndSelDpCParams, std::set<CDB_Set *> *pSet, CSelectedValve *pSelDpC );

	CSelectList *GetBvSelected();

	// These two methods are called by 'RViewSSelDpC' to adapt 'm_dDpToStab' with the user choice for regulating valve.
	void SetDpToStab( double dDpToStab ) { m_dDpToStab = dDpToStab; }
	double GetDpToStab() const { return m_dDpToStab; }

// Private methods.
private:
	void _SearchBv( CIndSelDpCParams *pclIndSelDpCParams, CDB_DpController *pclDpController, double dRequiredDpOnMv );

// Private variables.
private:
	int m_iSizeShiftUp;
	int m_iSizeShiftDown;
	double m_dDpToStab;
	double m_dMinRequiredDpOnMv;
	CIndSelBVParams *m_pclIndSelBVParams;
};


//////////////////////////////////////////////////////////////////////////////////
//	Selected combined DpController, control and balancing valve container
//////////////////////////////////////////////////////////////////////////////////
class CSelectShutoffList;
class CIndSelDpCBCVParams;
class CSelectDpCBCVList : public CSelectCtrlList
{
public:
	CSelectDpCBCVList();
	virtual ~CSelectDpCBCVList();

	virtual void Clean();

	// Pipe selection must be done before DpCBCV selection.
	// 'dDpFullOpen' is the Dp of the fully opened valve if it exist with the terminal unit (Can be a STAD, STS).
	int SelectDpCBCV( CIndSelDpCBCVParams *pclIndSelDpCBCVParams, bool *pbValidFound, bool *pbSizeShiftProblem, bool bTestDplmin = true );

	int SelectShutoffValve( CIndSelDpCBCVParams *pclIndSelDpCBCVParams, CSelectedValve *pSelDpCBCV );

	CSelectShutoffList *GetSvSelected() { return m_pSelectedSvList; }

// Private variables.
private:
	CSelectShutoffList *m_pSelectedSvList;
	int m_iSizeShiftUp;
	int m_iSizeShiftDown;
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected Thermostatic regulating valve container
//////////////////////////////////////////////////////////////////////////////////
class CIndSelTRVParams;
class CSelectTrvList : public CSelectList
{
public:
	// enum to facilitate management with 'OnTextTipFetch' in each RViewSSelXXX class (see 'CSelectedTAP::SetFlag').
	enum FlagError
	{
		DpNotFound = 0,
		DpAuthority,
		DpMaxReached,
		DpAboveUser = BvFlagErrorLast
	};
	
	CSelectTrvList();
	virtual ~CSelectTrvList();

	virtual void Clean();

	// Param: 'fInsert' is to tell if we must to work on 'TRVALV_TAB' or 'TRINSERT_TAB'.
	int SelectSupplyValve( CIndSelTRVParams *pclIndSelTRVParams, double dDpRequiredOnSupplyValve, bool &bValidFound, bool *pbSizeShiftProblem, 
			bool bForceFullOpen = false );

	int SelectFlowLimitedControlValve( CIndSelTRVParams *pclIndSelTRVParams, bool &bValidFound, bool *pbSizeShiftProblem, bool bForceFullOpen = false );

	int SelectReturnValve( CIndSelTRVParams *pclIndSelTRVParams, CString strSelRVTypeID, CString strSelRVFamilyID, CString strSelRVConnectID, 
			CString strSelRVVersionID, double dDpRequiredOnReturnValve, bool bForceFullOpening = false );

	CSelectList *GetReturnValveList() { return m_pReturnValveList; }

	double GetDpSV() const { return m_dSelDpSV; }
	double GetDpRV() const { return m_dSelDpRV; }
	bool GetDiffDpSV() const { return m_bDiffDpSV; }
	double GetMaxDpSupplyValve() const { return m_dMaxDpSV; }

// Private variables.
private:
	CSelectList *m_pReturnValveList;
	int m_iSizeShiftUp;
	int m_iSizeShiftDown;
	double m_dSelDpSV;		// It's the Dp wanted on the supply valve.
	double m_dSelDpRV;		// It's the Dp wanted on the return valve.
	bool m_bDiffDpSV;		// 'true' if there is Trv with different pressure drop computed.
	double m_dMaxDpSV;		// Contains the biggest Dp of selected supply valve.
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected separator container
//////////////////////////////////////////////////////////////////////////////////
class CIndSelSeparatorParams;
class CSelectSeparatorList : public CIndividualSelectBaseList
{
public:
	CSelectSeparatorList();
	virtual ~CSelectSeparatorList() {}

	virtual void Clean();

	// Allow to select separator for a given flow and Dp.
	// Param: 'pSeparatorList' is the list in which are the separators.
	// Param: 'pbSizeShiftProblem': is set to 'true' if there is a problem with allowed product sizes boundaries.
	// Param: 'dSelFlow' is the flow that must pass through.
	// Param: 'fBestFound' set to 'true' if at least one product is set as the best.
	int SelectSeparator( CIndSelSeparatorParams *pclIndSelSeparatorParams, bool *pbSizeShiftProblem, bool &bBestFound );

// Private variables.
private:
	int m_iSizeShiftUp;
	int m_iSizeShiftDown;
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected shut-off valve container
//////////////////////////////////////////////////////////////////////////////////
class CIndSelSVParams;
class CSelectShutoffList : public CIndividualSelectBaseList
{
public:
	CSelectShutoffList();
	virtual ~CSelectShutoffList() {}

	virtual void Clean();

	// Allow to select shut-off for a given flow.
	// Param: 'pclShutoffList' is the list in which are the shut-off valves.
	// Param: 'pbSizeShiftProblem': is set to 'true' if there is a problem with allowed product sizes boundaries.
	// Param: 'dSelFlow' is the flow that must pass through.
	// Param: 'fBestFound' set to 'true' if at least one product is set as the best.
	int SelectShutoffValve( CIndSelSVParams *pclIndSelSVParams, bool *pbSizeShiftProblem, bool &bBestFound );

// Private variables.
private:
	struct sBest
	{
		double dBestDp;
		double dBestDelta;

		struct sBest()
		{
			dBestDp = 0.0;
			dBestDelta = DBL_MAX;
		}
		CSelectedValve *pSelV;
	};
	sBest m_rBest;

	int m_iSizeShiftUp;
	int m_iSizeShiftDown;
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected safety valve container
//////////////////////////////////////////////////////////////////////////////////
class CIndSelSafetyValveParams;
class CSelectSafetyValveList : public CIndividualSelectBaseList
{
public:
	CSelectSafetyValveList();
	virtual ~CSelectSafetyValveList();

	virtual void Clean();

	// Allow to select safety valve.
	// Param: 'pclIndSelSafetyValveParams' is the list in which are the separators.
	// Param: 'pbSizeShiftProblem': is set to 'true' if there is a problem with allowed product sizes boundaries.
	// Param: 'fBestFound' set to 'true' if at least one product is set as the best.
	int SelectSafetyValve( CIndSelSafetyValveParams *pclIndSelSafetyValveParams, bool &bBestFound );

	int SelectBlowTank( CIndSelSafetyValveParams *pclIndSelSafetyValveParams, CSelectedSafetyValve *pclSelectedSafetyValve );

	CSelectList *GetBlowTankSelected() { return m_pclBlowTankList; }

	// Overrides 'CSelectedProduct'.
	virtual void FillResultsInRankEx( CRankEx *pList );

// Private method.
private:
	int _SelectSafetyValveHeating( CIndSelSafetyValveParams *pclIndSelSafetyValveParams, bool &bBestFound );
	int _SelectSafetyValveCooling( CIndSelSafetyValveParams *pclIndSelSafetyValveParams, bool &bBestFound );
	int _SelectSafetyValveSolar( CIndSelSafetyValveParams *pclIndSelSafetyValveParams, bool &bBestFound );

 // Private variables
private:
	CSelectList *m_pclBlowTankList;
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected 6-Way valve container
//////////////////////////////////////////////////////////////////////////////////
class CIndSel6WayValveParams;
class CSelect6WayValveList : public CSelectCtrlList
{
public:
	CSelect6WayValveList();
	virtual ~CSelect6WayValveList();

	virtual void Clean();

	// Pipe Selection must be done before 6-Way selection.
	// Based on a rank list that contains BCV matching selection parameters as Type, Connection, ...
	// Boolean pbSizeShiftProblem is set to true if there is a problem with allowed valves sizes boundaries.

	// TA-6-Way valve selection
	// ++++++++++++++++++++++++
	// 
	// We will use 'Chart2' sheet. Between several curves we will take the lower.
	// For example:
	// 
	// at 600 l/h the 'TA-6-Way DN 20' curve is always below the 'TA-6-Way DN 15/20' curve. We keep the DN 20.
	// at 550 l/h the 'TA-6-Way DN 20' is now above and the we take thus the 'TA-6-Way DN 15/20'.
	//
	// To have the graph in HySelect, we have Kvs and Flow, we can deduce Dp!!!
	// 
	// Then to have the curve in 'Chart2' we subtract all the value by the Dp pivot value (Here $S$2 that is 3 kPa).

	int Select6WayValve( CIndSel6WayValveParams *pclIndSel6WayValveParams, bool *pbSizeShiftProblem );

	// The control valve will depend of the application type chosen by the user.
	//
	// TA-Compact-P and TA-Modulator selection
	// +++++++++++++++++++++++++++++++++++++++
	// 
	// For application type 1 and 2 (See page 3 of the 'ta-6-way_valve_en_low.pdf' file).
	//
	// First, we need to verify that the Qmax of each valve is not too low. For that check the 'Chart1' sheet.
	// For example: if we have 1100 l/h we can take the 'TA-Compact-P 20' but not the 'TA-Modulator 20'.
	// 
	// Secondly, if we have to discriminate between two valves, we will use the 'Chart4' sheet.
	// For example: if we have 400 l/h, potentially we can take either the 'TA-Compact-P 15' or the 'TA-Modulator 15'.
	// If we go in the 'Chart4' we will take the one that have the bigger setting. In this case the 'TA-Modulator DN 15'.
	// 
	// To have the graph in HySelect, we need to have the curve qmax in regards to the setting for each TA-Compact-P and TA-Modulator valves.
	// 
	// We will then take the ratio between the cooling and heating application type (Ratio between the flow). Here in Excel it's 0,25.
	// And we take the sum between the setting from the interpolated curve for cooling at full flow and the setting from the interpolated curve for heating at 0.25 flow.
	// 'eSideDefinition': Because we have different flow in heating and cooling, we need to select 2 pressure independent balancing & control valves.
	int SelectPIBCValve( CIndSel6WayValveParams *pclIndSel6WayValveParams, CSelectedValve *pclSelected6WayValve, SideDefinition eSideDefinition, bool *pbSizeShiftProblem );

	// STAD
	// ++++
	//
	// For application type 3 (See page 3 of the 'ta-6-way_valve_en_low.pdf' file).
	//
	// 'eSideDefinition': Because we have different flow in heating and cooling, we need to select 2 balancing valves.
	int SelectBalancingValve( CIndSel6WayValveParams *pclIndSel6WayValveParams, CSelectedValve *pclSelected6WayValve, SideDefinition eSideDefinition, bool *pbSizeShiftProblem );

	bool IsPIBCValveExist( SideDefinition eSideDefinition );
	CSelectPICVList *GetPICVList( SideDefinition eSideDefinition );

	bool IsBalancingValveExist( SideDefinition eSideDefinition );
	CSelectList *GetBVList( SideDefinition eSideDefinition );

// Private methods.
private:
	// HYS-1877: Select PIBCV for 6-way valve set. 
	int _SelectPIBCVSetEQMControl( CIndSel6WayValveParams* pclIndSel6WayValveParams, CSelectedValve* pclSelected6WayValve, bool* pbSizeShiftProblem );
	int _SelectPIBCValveEQMControl( CIndSel6WayValveParams *pclIndSel6WayValveParams, CSelectedValve *pclSelected6WayValve, SideDefinition eSideDefinition, bool *pbSizeShiftProblem );
	int _SelectPIBCValveOnOffControl( CIndSel6WayValveParams *pclIndSel6WayValveParams, CSelectedValve *pclSelected6WayValve, SideDefinition eSideDefinition, bool *pbSizeShiftProblem );

// Private variables.
private:
	CSelectPICVList *m_pSelectedPICVList1;		// Application type 1 & 2: will contain TA-Modulator or TA-Compact in the case of change-over use.
	CSelectPICVList *m_pSelectedPICVList2;		// Application type 1 & 2: will contain TA-Modulator or TA-Compact in the case of change-over use.
	CSelectList *m_pSelectedBVList1;			// Application type 3: will contain STAD.
	CSelectList *m_pSelectedBVList2;			// Application type 3: will contain STAD.
	int m_iHighestSize;
	int m_iLowestSize;
};

//////////////////////////////////////////////////////////////////////////////////
//	Base class for all batch selections.
//////////////////////////////////////////////////////////////////////////////////
class CBatchSelectionParameters;
class CBatchSelectBaseList : public CSelectedProductList
{
public:
	typedef std::multimap<int, CSelectedBase *> mmapKeyIntSelectedProduct;

	typedef enum _BatchReturn
	{
		BR_FirstFlagResult		= 1,
		BR_NotFound				= BR_FirstFlagResult,					// 1
		BR_FoundOneBest			= ( BR_NotFound << 1 ),					// 2
		BR_FoundButNotBest		= ( BR_FoundOneBest << 1 ),				// 4
		BR_FoundAlernative		= ( BR_FoundButNotBest << 1 ),			// 8
		BR_LastFlagResult		= ( BR_FoundAlernative ),				// 8

		BR_FirstFlagError		= ( BR_LastFlagResult << 1 ),			// 16
		BR_FirstError			= ( BR_FirstFlagError ),				// 16
		BR_BatchSelParamsNULL	= ( BR_FirstError << 1 ),				// 32
		BR_BadArguments			= ( BR_BatchSelParamsNULL << 1 ),		// 64
		BR_NoPipeFound			= ( BR_BadArguments << 1 ),				// 128
		BR_DpBelowMinDp			= ( BR_NoPipeFound << 1 ),				// 256
		BR_DpAboveMaxDp			= ( BR_DpBelowMinDp << 1 ),				// 512
		BR_LastFlagError		= ( BR_DpAboveMaxDp )

	}BatchReturn;

	CBatchSelectBaseList();
	virtual ~CBatchSelectBaseList();

	// This method cleans only the product found with a previous search without clearing all the rest.
	void CleanVecObjects();

	virtual void TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList );

	CSelectedBase *GetBestProduct( void ) { return m_pclBestProduct; }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Override the 'CSelectedProductList' virtual public methods.
	// This method erases all data that are no more needed included the pipe list in the 'CSelectedProductList' base class.
	virtual void Clean();

	virtual void CopyFrom( CSelectedProductList *pclSelectedProductList );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Protected methods.
protected:
	void BuildSortingKeys( CBatchSelectionParameters *pclBatchSelectionParameters, CTAPSortKey *psKeyTechParamBlw65 , CTAPSortKey *psKeyTechParamAbv50 );
	bool GetPipeSizeShift( CBatchSelectionParameters *pclBatchSelectionParameters, int &iHighestSize, int &iLowestSize );

	typedef mmapKeyIntSelectedProduct::iterator mmapKeyIntSelectedProductIter;

// Protected variables.
protected:
	int m_iSizeShiftUp;
	int m_iSizeShiftDown;
	CSelectedBase *m_pclBestProduct;
	CDB_Pipe *m_pclBestPipe;

	typedef std::multimap<double, CSelectedBase *> mmapKeyDoubleSelectedProduct;

	// This multimap will contain all products that are matching with size above/below criterion.
	mmapKeyIntSelectedProduct m_mmapProductList;

	// Because we are working in other way for batch selection, it easier to have only one vector in which we insert
	// all the 'CSelectedValve' object and this vector is cleared when no more needed.
	std::vector<CSelectedBase*> m_vecObjects;
};

//////////////////////////////////////////////////////////////////////////////////
// Selected control valve container base for batch mode for balancing and control
// valves and pressure independent balancing and control valves.
//////////////////////////////////////////////////////////////////////////////////
class CBatchSelCtrlParamsBase;
class CBatchSelectCtrlListBase: public CBatchSelectBaseList
{
public:
	CBatchSelectCtrlListBase() : CBatchSelectBaseList() {}
	virtual ~CBatchSelectCtrlListBase() {}

	CDB_Actuator *SelectActuator( CBatchSelCtrlParamsBase *pclBatchSelCtrlParamsBase, CDB_ControlValve *pclControlValve, double dDp, int* sp_iReturn );

// Private methods.
private:
	int _GetCOCCompliantActuatorList( CBatchSelCtrlParamsBase *pclBatchSelCtrlParamsBase, CDB_ControlValve *pclControlValve, double dDp, std::multimap< double, CDB_Actuator *> *pmmapActuatorList, 
			bool bWorkingForASet );
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected balancing & control valve container for batch mode
//////////////////////////////////////////////////////////////////////////////////
class CBatchSelBCVParams;
class CBatchSelectBCVList : public CBatchSelectCtrlListBase
{
public:
	CBatchSelectBCVList( );
	virtual ~CBatchSelectBCVList() {}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Override the 'CSelectedProductList' virtual public methods.
	virtual void TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to select a balancing & control valve for a given flow and Dp.
	int SelectBCV( CBatchSelBCVParams *pclBatchSelBCVParams, double dFlow, double dDp );

// Private methods.
private:
	bool _SelectBestTACV( CBatchSelBCVParams *pclBatchSelBCVParams, double dTargetKvs, double dFlow, double dDp );
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected pressure & independent control valve container for batch mode
//////////////////////////////////////////////////////////////////////////////////
class CBatchSelPIBCVParams;
class CBatchSelectPICvList : public CBatchSelectCtrlListBase
{
public:
	CBatchSelectPICvList();
	virtual ~CBatchSelectPICvList() {}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Override the 'CSelectedProductList' virtual public methods.
	virtual void TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to select a pressure & independent control valve for a given flow and Dp.
	int SelectPICv( CBatchSelPIBCVParams *pclBatchSelPIBCVParams, double dFlow );

// Private methods.
private:
	bool _SelectBestPICV( CBatchSelPIBCVParams *pclBatchSelPIBCVParams, double dFlow );
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected balancing valve container for batch mode
//////////////////////////////////////////////////////////////////////////////////
class CBatchSelBVParams;
class CBatchSelectBvList : public CBatchSelectBaseList
{
public:
	CBatchSelectBvList();
	virtual ~CBatchSelectBvList() {}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Override the 'CSelectedProductList' virtual public methods.
	virtual void TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to select a balancing & control valves for a given flow and Dp.
	int SelectBv( CBatchSelBVParams *pclBatchSelBVParams, double dFlow, double dDpRequired, CRankEx *pclBvList = NULL );

// Private methods.
private:
	bool _SelectBv( CBatchSelBVParams *pclBatchSelBVParams, double dFlow, double dDpRequired, CRankEx *pclBvList = NULL );
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected separator container for batch mode
//////////////////////////////////////////////////////////////////////////////////
class CBatchSelSeparatorParams;
class CBatchSelectSeparatorList : public CBatchSelectBaseList
{
public:
	CBatchSelectSeparatorList();
	virtual ~CBatchSelectSeparatorList() {}

	// Allow to select a separator for a given flow.
	int SelectSeparator( CBatchSelSeparatorParams *pclBatchSelSeparatorParams, double dFlow );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Override the 'CSelectedProductList' virtual public methods.
	virtual void Clean();
	virtual void TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Private methods.
private:
	bool _SelectSeparator( CBatchSelSeparatorParams *pclBatchSelSeparatorParams, double dFlow );

// Private variables.
private:
	bool m_bBestFound;
	bool m_bSizeShiftProblem;
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected Dp controller valve container for batch mode
//////////////////////////////////////////////////////////////////////////////////
class CBatchSelDpCParams;
class CBatchSelectDpCList : public CBatchSelectBaseList
{
public:
	CBatchSelectDpCList();
	virtual ~CBatchSelectDpCList();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CSelectedProductList' virtual public method.
	virtual void Clean();
	virtual void TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to select a dp controller valve for a given flow and Dp.
	int SelectDpC( CBatchSelDpCParams *pclBatchSelDpCParams, double dFlow, double dDpBranch, double dKvs );

	CBatchSelectBvList *GetBvSelected() { return m_pSelectedBatchBvList; }

// Private methods.
private:
	bool _SelectBestDpC( CBatchSelDpCParams *pclBatchSelDpCParams, double dFlow, double dDpBranch, double dKvs );
	bool _SelectBestBv( CBatchSelDpCParams *pclBatchSelDpCParams, double dFlow, double dDpBranch, double dKvs );
	double _GetDpToStab( CBatchSelDpCParams *pclBatchSelDpCParams, double dFlow, double dDpBranch, double dKvs, int iForceMvLoc );

// Private variables.
private:
	CBatchSelectBvList *m_pSelectedBatchBvList;
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected smart control valve container for batch mode
//////////////////////////////////////////////////////////////////////////////////
class CBatchSelSmartControlValveParams;
class CBatchSelectSmartControlValveList : public CBatchSelectBaseList
{
public:
	CBatchSelectSmartControlValveList();
	virtual ~CBatchSelectSmartControlValveList() {}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Override the 'CSelectedProductList' virtual public methods.
	virtual void Clean();
	virtual void TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to select a smart control valve for a given flow and Dp.
	int SelectSmartControlValve( CBatchSelSmartControlValveParams *pclBatchSelSmartControlValveParams, double dFlow );

// Private methods.
private:
	bool _SelectSmartControlValve( CBatchSelSmartControlValveParams *pclBatchSelSmartControlValveParams, double dFlow );

// Private variables.
private:
	bool m_bBestFound;
	bool m_bSizeShiftProblem;
};

//////////////////////////////////////////////////////////////////////////////////
//	Selected smart differential pressure controller container for batch mode
//////////////////////////////////////////////////////////////////////////////////
class CBatchSelSmartDpCParams;
class CBatchSelectSmartDpCList : public CBatchSelectBaseList
{
public:
	CBatchSelectSmartDpCList();
	virtual ~CBatchSelectSmartDpCList() {}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Override the 'CSelectedProductList' virtual public methods.
	virtual void Clean();
	virtual void TransferResultsToIndividualSelectList( CIndividualSelectBaseList *pclIndividualSelectList );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Allow to select a smart differential pressure controller for a given flow and Dp.
	int SelectSmartDpC( CBatchSelSmartDpCParams *pclBatchSelSmartDpCParams, double dFlow );

	CDB_DpSensor *GetBestDpSensor( CBatchSelSmartDpCParams *pclBatchSelSmartDpCParams, double dDpBranch );
	CDB_Product *GetBestDpSensorSet( CBatchSelSmartDpCParams *pclBatchSelSmartDpCParams, double dDpBranch );

// Private methods.
private:
	bool _SelectSmartDpC( CBatchSelSmartDpCParams *pclBatchSelSmartDpCParams, double dFlow );
	CDB_DpSensor *_SelectBestDpSensor( CBatchSelSmartDpCParams *pclBatchSelSmartDpCParams, double dDpBranch, bool bForSet );

// Private variables.
private:
	bool m_bBestFound;
	bool m_bSizeShiftProblem;
	CDB_Product *m_pclBestDpSensor;
};