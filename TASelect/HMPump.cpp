#include "StdAfx.h"
#include "HMInclude.h"

CPump::CPump( CDS_HydroMod *pHM )
{
	m_pParent = pHM;
	m_PumpChar.clear();
	m_dHmin = 0.0;
	m_dHpump = 0.0;
}

void CPump::Copy( CPump *pPump )
{
	pPump->SetDescription( GetDescription() );
	pPump->SetHmin( GetHmin() );
	pPump->SetHpump( GetHpump(), false );
	std::map<double, double>::iterator It;
	for (It = m_PumpChar.begin(); It != m_PumpChar.end(); ++It)
	{
		pPump->SetPumpCharPoint(It->first, It->second);
	}
}

double CPump::GetHmin() const
{
/*	if (NULL != m_pParent)
	{
		m_pParent->AdjustPumpHmin();
	}
*/	return m_dHmin;
}

void CPump::DropData( OUTSTREAM outf, int* piLevel )
{
	CString strTab = _T("");
	for( int i = *piLevel; i; i-- )
		strTab += _T("\t");

	CString str;
	str.Format( _T("m_pParent name = %s\n"), m_pParent->GetHMName() );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_Description = %s\n"), m_Description );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dHmin = %f\n"), WriteCUDouble( _U_DIFFPRESS, m_dHmin, true ) );
	WriteFormatedStringA2( outf, str, strTab );
	str.Format( _T("m_dHpump = %f\n"), WriteCUDouble( _U_DIFFPRESS, m_dHpump, true ) );
	WriteFormatedStringA2( outf, str, strTab );
	WriteFormatedStringA2( outf, _T("\n"), strTab );
}

double CPump::GetHforCircuit()
{
	CDS_HydroMod::CBV *pBv = NULL;

	if( m_pParent->GetpSchcat() != NULL && true == m_pParent->GetpSchcat()->IsPump() )
	{
		pBv = m_pParent->GetpBv();
	}
	else
	{
		pBv = m_pParent->GetpSecBv();	
	}
	
	double dH =  GetHAvail();
	
	// Balancing valve exist.
	if( pBv != NULL )
	{
		dH -= pBv->GetDp();		
	}

	return dH; 
}

#define CPUMP_VERSION	1
void CPump::Write( OUTSTREAM outf )
{
	// Version.
	BYTE Version = CPUMP_VERSION;
	outf.write( (char*)&Version, sizeof( Version ) );

	// Info.
	WriteString( outf, (LPCTSTR)m_Description );
	outf.write( (char*)&m_dHmin, sizeof( m_dHmin ) );
	outf.write( (char*)&m_dHpump, sizeof( m_dHpump ) );
	int iSize = m_PumpChar.size();
	outf.write( (char*)&iSize, sizeof( iSize ) );
	std::map<double, double>::iterator It;
	if ( iSize > 0 )
	{
		for(It = m_PumpChar.begin(); It!=m_PumpChar.end(); ++It)
		{
			outf.write( (char*)&It->first, sizeof( double ) );
			outf.write( (char*)&It->second, sizeof( double ) );
		}
	}
}

bool CPump::Read( INPSTREAM inpf )
{
	// Version.
	BYTE Version = CPUMP_VERSION;
	inpf.read( (char*)&Version, sizeof( Version ) );
	if( Version < 1 || Version > CPUMP_VERSION )
		return false;

	// Info.
	TCHAR buf[CSTRING_LINE_SIZE_MAX];
	ReadString( inpf, buf, sizeof( buf ) );
	m_Description=buf;
	inpf.read( (char*)&m_dHmin, sizeof( m_dHmin ) );
	inpf.read( (char*)&m_dHpump, sizeof( m_dHpump ) );
	int iSize = 0;
	m_PumpChar.clear();
	inpf.read( (char*)&iSize, sizeof( iSize ) );

	for ( ; (--iSize) >= 0; )
	{
		double dq, dh;
		inpf.read( (char*)&dq, sizeof( dq ) );
		inpf.read( (char*)&dh, sizeof( dh ) );
		m_PumpChar[dq] = dh;
	}
	return true;
}

void CPump::SetHpump( double val, bool fCompute )
{
	m_dHpump = val;
	if (true == fCompute)
		m_pParent->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
	m_pParent->Modified();
}

void CPump::SetHmin( double val )
{
	m_dHmin = val;
	m_pParent->Modified();
}
