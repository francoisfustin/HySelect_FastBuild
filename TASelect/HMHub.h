#pragma once


class CDS_HydroMod;
class CDS_HmHub: public CDS_HydroMod
{
	DEFINE_DATA( CDS_HmHub )
public:
	typedef enum Casing
	{
		None,		
		Insulated
	};

	typedef enum MainFeedPos
	{
		Left,	
		Right	
	};

	typedef enum EndOption
	{
		Drain
	};

	typedef enum SelMode
	{
		Q,
		QDpLoop,
		LoopDetails,
		Direct
	};

	typedef enum Application
	{
		Heating = 1,
		Cooling	= 2
	};

	typedef enum ExtConn
	{
		eHubPartner = 1,
		eConnectName = 2,
		eTAISConnType = 3,
		eConnOrder = 4
	};

public:
	CDS_HmHub( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDS_HmHub();
	
	void Init( CString strBalTypeID, CTable *plcParent = NULL );

	// Spread any change for other elements.
	virtual void Copy( CDS_HydroMod *pHMHub, bool bValveDataOnly = false, bool bCreateLinkedDevice = true, bool bToClipboard = false );
	
	virtual bool IsForHub() { return true; }
	
	// Return true if all defined HM objects are defined.
	virtual bool IsCompletelyDefined();

	virtual CString RenameMe(bool bForceAuto = false );
	
	virtual void ResizeShutOffValves();
	
	void SetpWC( CString strID_WC = CString( _T("PROJ_WC") ) );
	CWaterChar *GetpWC();
	
	// Return IDPTR of TADB valve measured into the CBI.
	virtual IDPTR GetTADBValveIDPtr();

	UINT GetNumberOfStations() { return GetItemCount(); }
	Casing GetCasing() { return m_eCasing; }
	MainFeedPos GetMainFeedPos() { return m_eMainFeedPos; }
	bool IsBypassExist() { return m_bBypass; }
	EndOption GetEndOption() { return m_eEndOption; }
	CString GetBalTypeID(){ return GetBalTypeIDPtr().ID; }
	IDPTR GetBalTypeIDPtr( bool bForBVSizing = false );
	SelMode GetSelMode() { return m_eSelMode; }
	CString GetSupplyValveID() { return m_SupplyValveIDPtr.ID; }
	IDPTR GetSupplyValveIDPtr();
	CString GetReturnValveID() { return m_ReturnValveIDPtr.ID; }
	IDPTR GetReturnValveIDPtr();	
	IDPTR GetPartnerConnectIDPtr();
	IDPTR GetStationsConnectIDPtr();

	Application GetApplication() { return m_eApplication; }

	void SetCasing( Casing eCasing ) { if( m_eCasing != eCasing ) { m_eCasing = eCasing; Modified();}};
	void SetMainFeedPos( MainFeedPos eMainFeedPos ) { if( m_eMainFeedPos != eMainFeedPos ) { m_eMainFeedPos = eMainFeedPos; Modified(); } }
	void SetBypass( bool fBypass ) { if( m_bBypass != fBypass ) { m_bBypass = fBypass; Modified(); } }
	void SetEndOption(EndOption eEndOption) { if( m_eEndOption != eEndOption ) { m_eEndOption = eEndOption; Modified(); } }
	void SetSelMode(SelMode eSelMode) { if( m_eSelMode != eSelMode ) { m_eSelMode= eSelMode; Modified(); } }
	bool SetBalTypeID( CString strBalTypeID );
	void SetSupplyValveID( CString strSupplyValveID );
	void SetSupplyValveIDPtr( IDPTR SupplyValveIDPtr );
	void SetReturnValveID( CString strReturnValveID );
	void SetPartnerConnectIDPtr( IDPTR PartnerConnectIDPtr ) { m_PartnerConnectIDPtr = PartnerConnectIDPtr;} 
	void SetStationsConnectIDPtr( IDPTR StationsConnectIDPtr ) { m_StationsConnectIDPtr = StationsConnectIDPtr; }
	void SetApplication( Application eApplication ) { if( m_eApplication != eApplication ) { m_eApplication = eApplication; Modified(); } }

	IDPTR FindHubPtnValve( CDB_HubValv::eSupplyReturnValv SupRet, IDPTR SelValveIDPtr );
	virtual CSelectedInfos *GetpSelectedInfos() { return m_pSelectedInfos; }

	void RenameStations();
	
	// Return warning string if the total flow is greater than the maximum allowed flow.
	CString	CheckMaxTotalFlow();

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' public virtual methods.

	// Allow to compare a selection inherited class (as CDS_SSelDpC) with an object of the same type in regards 
	// to the key passed as argument.
	virtual int CompareSelectionTo( CData *pclSelectionCompareWith, int iKey );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Overrides 'CData' protected virtual methods.

	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	////////////////////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CWaterChar *m_pWC;
	CString	m_WaterCharID;
	bool m_bBypass;
	Casing m_eCasing;
	MainFeedPos m_eMainFeedPos;
	EndOption m_eEndOption;
	SelMode m_eSelMode;
	Application m_eApplication;
	CSelectedInfos *m_pSelectedInfos;
	IDPTR m_BalTypeIDPtr;
	IDPTR m_SupplyValveIDPtr;
	IDPTR m_ReturnValveIDPtr;
	IDPTR m_PartnerConnectIDPtr;
	IDPTR m_StationsConnectIDPtr;
};

////////////////////////////////////////////////////////////////////////////////////////////
class CDS_HmHubStation: public CDS_HydroMod
{
	DEFINE_DATA(CDS_HmHubStation)
public:
	typedef enum eCVTAKV
	{
		eNull,
		eNone,
		eKv,
		eTAValve
	};
protected:
	// Saved variables
	eCVTAKV				m_CvTaKv;
	IDPTR				m_BalTypeIDPtr;
	IDPTR				m_SupplyValveIDPtr;
	IDPTR				m_ReturnValveIDPtr;
	IDPTR				m_ActuatorGroupIDPtr;
	IDPTR				m_ReturnActuatorIDPtr;
	bool				m_bPlugged;

public:
	CDS_HmHubStation(CDataBase* pDataBase, LPCTSTR ID);
	virtual ~CDS_HmHubStation(){};
	void Init(CString BalTypeID, CTable *plcParent = NULL, eCVTAKV CvTaKv = eCVTAKV::eNone);
	// Compute a new setting point for balancing device in function of Havail.
	virtual void ResizeBalDevForH(double Havail, bool bForPending, CAnchorPt::CircuitSide eCircuitSide );
	// Return minimum H for the circuit 
	virtual double GetHmin();
	// Return current Dp for the circuit
	virtual double GetDp(bool fDpMin = false);
	virtual bool IsForHub(){return true;};
	// Spread any change for other elements
	virtual void Copy(CDS_HydroMod *pHMHub, bool bValveDataOnly = false, bool bCreateLinkedDevice=true, bool bToClipboard=false);
	virtual void ResizeShutOffValves();
	CWaterChar *GetpWC();

	// Return IDPTR of TADB valve measured into the CBI 
	virtual IDPTR GetTADBValveIDPtr();

	CString			GetBalTypeID()		{return	GetBalTypeIDPtr().ID;};
	IDPTR			GetBalTypeIDPtr();
	CString			GetSupplyValveID()		{return m_SupplyValveIDPtr.ID;};
	CString			GetReturnValveID()		{return m_ReturnValveIDPtr.ID;};
	CString			GetActuatorGroupID()	{return m_ActuatorGroupIDPtr.ID;};
	CString			GetReturnActuatorID()	{return m_ReturnActuatorIDPtr.ID;};
	IDPTR			GetSupplyValveIDPtr();	
	IDPTR			GetReturnValveIDPtr();	
	IDPTR			GetActuatorGroupIDPtr();
	IDPTR			GetReturnActuatorIDPtr();
	eCVTAKV			GetCvTaKv()				{return m_CvTaKv;};
	CString			GetCvTypeString(eCVTAKV CvTaKv, bool bStrShort = true);
	bool			SetControlValve(eCVTAKV CvTaKv, CDB_ControlProperties::CvCtrlType CvCtrlType, CDB_StringID* pConnect=NULL, CDB_StringID* pVersion=NULL, CDB_StringID* pPN=NULL);
	bool			IsPlugged(){return m_bPlugged;};

	void			SetSupplyValveID(CString SupplyValveID);			
	void			SetSupplyValveIDPtr(IDPTR SupplyValveIDPtr);
	void			SetReturnValveID(CString ReturnValveID);			
	//	void			SetActuatorGroupID(CString ActuatorGroupID);		
	void			SetReturnActuatorID(CString ReturnActuatorID);	
	void			SetPlugged(bool bFlag=true){if (bFlag!=m_bPlugged){m_bPlugged = bFlag; Modified();};};
	IDPTR			FindHubStaValve(CDB_HubStaValv::eSupplyReturnValv SupRet,IDPTR SelValveIDPtr);
private:
protected:
	bool		 SetBalTypeID(CString BalTypeID);
	// Called by Database object before its destruction
	virtual void OnDatabaseDestroy(){CDS_HydroMod::OnDatabaseDestroy();};
	// Disk IO
	virtual void Write(OUTSTREAM outf);
	void PatchActr();
	virtual bool Read(INPSTREAM inpf);
};
