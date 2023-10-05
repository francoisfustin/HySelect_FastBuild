#pragma once
class CPump
{
	friend class CDS_HydroMod;
public:
	CPump( CDS_HydroMod *pHM );
	virtual ~CPump(void) {}

	void Copy( CPump *CPump );

	CString GetDescription() const { return m_Description; }
	double GetHmin() const;
	double GetHpump() const { return m_dHpump; }
	double GetHAvail() const { return max(m_dHmin, m_dHpump); }
	double GetHforCircuit();

	void SetDescription(CString val) { m_Description = val;  m_pParent->Modified();}
	void SetPumpCharPoint(double dq, double dH) { m_PumpChar[dq] = dH; m_pParent->Modified();}
	void SetHpump(double val, bool fCompute = true);
	void SetHmin(double val); 

	virtual void DropData( OUTSTREAM outf, int* piLevel );

	// Protected methods.
protected:
	void Write( OUTSTREAM outf );
	bool Read( INPSTREAM inpf );

	// Protected variables.
protected:

	// Private variables.
private:
	CDS_HydroMod				*m_pParent;				// Pointer on the parent module
	CString						m_Description;			// Description/Name of the pump
	std::map<double, double>	m_PumpChar;				// map of double describing pump curve (x[flow],y[Head])
	double						m_dHmin;				// H needed by the circuit
	double						m_dHpump;				// H provided by the pump
};

