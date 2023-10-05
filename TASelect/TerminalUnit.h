#pragma once


class CTermUnit
{
	friend class CDS_HydroMod;

public:
	enum _QType
	{
		Undefined = -1,
		Q,
		PdT,
		PTsTr			// No more used ! Need to keep to read old project.
	};

	// Structure just to facilitate work with 'CDS_HydroMod::_uDpVal'.
	// In 'DlgComboBoxHM' we need to keep previous values entered by user. For example, when user choose to introduce Dp in a 'QRef/DpRef' mode 
	// and coming back for directly input Dp, DpRef is overwritten due to the fact that 'CDS_HydroMod::_uDpVal' is an union. Thus if user want to come back
	// again in a 'DpRef/QRef' mode, previous values can't be recuperated.
	// This structure is not wrote/read!
	struct _DpVal
	{
		double m_dDT;
		double m_dTs;
		double m_dTr;
		double m_dKv;
		double m_dCv;
		double m_dQRef;
		double m_dDpRef;
		double m_dDzeta;
		double m_dDiameter;
		struct _DpVal()
		{
			Reset();
		};
		void Reset()
		{
			m_dDT = 0.0;
			m_dTs = 0.0;
			m_dTr = 0.0;
			m_dKv = 0.0;
			m_dCv = 0.0;
			m_dQRef = 0.0;
			m_dDpRef = 0.0;
			m_dDzeta = 0.0;
			m_dDiameter = 0.0;
		}
	};

	class _STermUnit				// struct Terminal Unit
	{
	public:
		void CopyTo( _STermUnit *pDest );
		CString m_strDescription;
		_QType	m_eQType;
		double m_dQ;			// Flow
		double m_dP;			// Power
		double m_dDT;			// delta T
		CDS_HydroMod::eDpType m_eDpType;			// Type of Dp
		CDS_HydroMod::_uDpVal m_uDpVal;
	};

	CTermUnit( CDS_HydroMod *pHM );
	void SetDescription( CString strDescription ) { m_TermUnit.m_strDescription = strDescription; m_pParent->Modified(); }
	
	// A terminal unit is virtual when it doesn't exist, variables of terminal unit are used to store children flow and Dp.
	void SetVirtual( bool flag ) { m_bVirtual = flag; }
	bool IsVirtual() { return m_bVirtual; }

	void SetQType( enum _QType eQType );
	void SetQ( double dQ, bool bComputeHM = true );
	// HYS-1872: Add a booleen to tell if the compute is necessary or not
	void SetPdT( double dP, double dDT, bool bComputeHM = true );				// To set only one value set other value to DBL_MAX
	void SetDT( double dDT ) { m_TermUnit.m_dDT = dDT; }

	// HYS-1882: Set DT when we are in flow mode.
	void SetDTFlowMode( double dDT, bool bComputeHM = true );
	
	// Set the values but do nothing else like the "SetPdT" method.
	void SetPdTOnlyValues( double dP, double dDT );				// To set only one value set other value to DBL_MAX
	
	// eDpType = Dp      -> dVal1 = Dp
	//			 Kv      -> dVal1 = Kv
	//			 Cv      -> dVal1 = Cv
	//			 dzeta   -> dVal1 = dzeta, dVal2 = d		; To set only one value set other value to DBL_MAX
	//			 QDpref		dVal1 = DpRef, dVal2 = Qref		; To set only one value set other value to DBL_MAX		
	void SetDp( CDS_HydroMod::eDpType eDpType, double dVal1, double dVal2 = 0.0, bool bCompute = true );
	void SetDp( double dVal1, bool bCompute = true ) { SetDp( CDS_HydroMod::eDpType::Dp, dVal1, 0.0, bCompute ); }

	enum _QType GetQType() { return m_TermUnit.m_eQType; }
	double GetQ();
	
	// return P=f(q;Ts;Tr)
	double GetP();
	double GetDT();

	// HYS-1882: Return DT if we are in flow mode. Else return 0.0. bNeedsInitValue if we need to need to show a value != 0 in TU box.
	double GetDTFlowMode( bool bNeedsInitValue = false );
	
	CDS_HydroMod::eDpType GetDpType() { return m_TermUnit.m_eDpType; }
	double GetDp();
	
	// peDpType = Dp     -> pdVal1 = Dp
	// peDpType = Kv     -> pdVal1 = Kv
	// peDpType = Cv     -> pdVal1 = Cv
	// peDpType = dzeta  -> pdVal1 = dzeta, pdVal2 = d
	// peDpType = QDpref -> pdVal1 = DpRef, pdVal2 = Qref
	double GetDp( CDS_HydroMod::eDpType* peDpType, double* pdVal1, double* pdVal2 );
	double GetKv();
	double GetCv();
	double GetDpRef();
	double GetQRef();
	void GetDpQRef( double &dDpRef, double &dQRef );

	CString GetFlowString();
	CString GetPowerString();
	CString GetDTString();
	CString GetDpString();
	CString GetKvString();
	CString GetCvString();
	CString GetDpRefString();
	CString GetQRefString();

	_DpVal *GetpDpWorkStructure( void ) { return &m_rDpValues; }
	_STermUnit *GetTerminalUnitData() {return &m_TermUnit; }
	
	double DpToDpref();
	CString GetDescription() { return m_TermUnit.m_strDescription; }
	void CopyTo( CTermUnit *pclDestTerminalUnit );
	bool IsCompletelyDefined();
	
	double GetPartialPendingQ() const { return m_dPartialQ; }
	void SetPartialPendingQ(double val) { m_dPartialQ = val; }

	double GetTotQwoDiversity() const { return m_dTotQwoDiversity; }
	void SetTotQwoDiversity(double val) { m_dTotQwoDiversity = val; }

	// HYS-1882: Set DT with dDtToSet or set init value.
	double UpdateDT( double dDtToSet = 0.0 );

	virtual void DropData( OUTSTREAM outf, int* piLevel );

// Protected methods.
protected:
	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );

// Protected variables.
protected:
	bool m_bVirtual;	
	CTADatastruct *m_pTADS;

// Private variables.
private:
	CDS_HydroMod *m_pParent;

	_STermUnit m_TermUnit;
	_DpVal m_rDpValues;

	// m_dPartialQ is used to maintain during computing flow that is flowing through unit/module,
	// When the circuit is "normal" m_dPartialQ = m_dQ
	// When the circuit is pending m_dPartialQ = 0;
	// When the circuit is a module m_dPartialQ = sum of all m_dPartialQ
	double m_dPartialQ;
	
	// m_dTotQwoDiversity is used to maintain during computing flow that is flowing through unit/module,
	// When the circuit is "normal" m_dTotQwoDiversity = m_dQ
	// When the circuit is a module with diversity m_dTotQwoDiversity = Q with diversity computed during ascending phase
	double m_dTotQwoDiversity;
};
