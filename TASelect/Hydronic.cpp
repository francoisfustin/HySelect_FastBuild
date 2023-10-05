#include "stdafx.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>

#ifndef TACBX
#include "DataBase.h"
#include "DataBObj.h"
#else
#include "DBInclude.h"
#endif

#include "Hydronic.h"

const double _PHI = 0.70;
const double _FRT = 0.86;
const double _DELTA = 0.4;


/////////////////////////////////////////////////////////////////////////////
// BASIC CALCULATIONS
/////////////////////////////////////////////////////////////////////////////

// Compute hydraulic resistance based on Kv value
double CalcR( double dKv, double dRho )
{
	if( dKv <= 0.0 || dRho <= 0.0 )
	{
		return 0.0;
	}
	
	return ( 36000.0 * 36000.0 * dRho ) / ( dKv * dKv );
}

double CalcKv( double dR, double dRho )
{
	if( dR <= 0.0 || dRho <= 0.0 )
	{
		return 0.0;
	}
	
	return ( 36000.0 * sqrt( dRho / dR ) );
}

// Compute the turbulent water flow rate.
double CalcqT(double dKv, double dDp, double dRho)
{
	if( dKv <= 0.0 || dDp <= 0.0 || dRho <= 0.0 )
	{
		return 0.0;
	}
	
	return dKv / 36000.0 * sqrt( dDp /dRho );
}

// Compute the Dp from the turbulent water flow rate and Kv.
double CalcDp( double dQ, double dKv, double dRho )
{
	if( dQ < 0.0 || dKv <= 0.0 || dRho <= 0.0 )
	{
		return 0.0;
	}
	
	return dRho * pow( dQ * 36000.0 / dKv, 2.0 );
}

// Compute the Kv from the turbulent water flow rate and Dp.
double CalcKv( double dQ, double dDp, double dRho )
{
	if( dQ <= 0.0 || dDp <= 0.0 || dRho <= 0.0 )
	{
		return 0.0;
	}

	if( 0.0 == dDp )
	{
		return DBL_MAX;
	}
	
	return dQ * 36000.0 * sqrt( dRho / dDp );
}

// Compute the power from a flow and DT.
double CalcP( double dQ, double dDT, double dRho, double dcp )
{
	if( dQ <= 0.0 || dDT <= 0.0 || dRho <= 0.0 || dcp <= 0.0 )
	{
		return 0.0;
	}
	
	return dRho * dcp * dQ * dDT;
}

// Compute the flow from a power and DT.
double CalcqFromPDT(double dP, double dDT, double dRho, double dcp )
{
	if( dP < 0.0 || dDT <= 0.0 || dRho <= 0.0 || dcp <= 0.0 )
	{
		return 0.0;
	}
	
	return dP / ( dRho * dcp * dDT );
}

// Compute the DT from a flow and power.
double CalcDT( double dP, double dQ, double dRho, double dcp )
{
	if( dP < 0.0 || dQ <= 0.0 || dRho <= 0.0 || dcp <= 0.0 )
		return 0.0;
	
	return dP / ( dRho * dcp * dQ );
}

// Compute the Dp from the Dzeta coefficient speed and density rho
double CalcDpFromDzeta( double dDzeta, double dU, double dRho )
{
	if( dDzeta <= 0.0 || dU <= 0.0 || dRho <= 0.0 )
	{
		return 0.0;
	}
	
	return 500.0 * dDzeta * dRho / 1000.0 * dU * dU;
}

// Compute the Kv coefficient from the Dzeta coefficient and internal 
// pipe diameter
double CalcKvFromDzeta( double dDzeta, double dDiameter )
{
	if( dDzeta <= 0.0 || dDiameter < 0.0 )
	{
		return 0.0;
	}

	return ( dDzeta > 0.0 ) ? 18e+3 * M_PI * dDiameter * dDiameter / sqrt( 2.0 * dDzeta ) : 0.0;
}

/////////////////////////////////////////////////////////////////////////////
// PIPE CALCULATIONS
/////////////////////////////////////////////////////////////////////////////

// Compute the the linear pressure drop.
// Parameters are: flow (q), fluid density (rho), fluid viscosity (nu), 
// pipe roughness (rough), internal diameter (di) and arithmetic accuracy.
double CalcPipeDp( double dQ, double dRho, double dNu, double dRoughness, double dDiameter, double dAccuracy )
{
	const double dRe1 = 2300.0;
	const double dRe2 = 3500.0;
	
	if( dQ <= 0.0 || dRho <= 0.0 || dNu <= 0.0 || dRoughness <= 0.0 || dDiameter <= 0.0 || dAccuracy < 1.E-15 )
	{
		return 0.0;
	}

	const double dU = dQ / ( 0.785398163398 * dDiameter * dDiameter );
	const double dRe = dU * dDiameter / dNu;
	double dLambda;
	
	if( dRe > dRe1 )
	{
		const double dC1 = dRoughness / dDiameter / 3.71;
		const double dC2 = 2.51 / dRe;
		const double dC3 = 2.0 / log( 10.0 ) * dC2;
		dAccuracy = 0.5 * dAccuracy;
		double dOldX;
		double dX = 3.0;
		int i = 100;	// Maximum allowed iteration
		
		do	// Newton-Raphson
		{
			dOldX = dX;
			dX = dC1 + dC2 * dX; 
			dX = dOldX - ( dOldX + 2.0 * log10( dX ) ) / ( 1.0 + dC3 / dX );			ASSERT( dX > 0.0 );
		}while( ( fabs( 1.0 - dOldX / dX ) > dAccuracy) && ( i-- > 0 ) );
		
		ASSERT( i >= 0 );

		// Turbulent flow conditions.
		dLambda = 1.0 / ( dX * dX);
		
		// Intermediary flow conditions.
		if( dRe < dRe2 )
		{
			dLambda = ( 64.0 * ( dRe2 - dRe ) / dRe1 + dLambda * ( dRe - dRe1 ) ) / ( dRe2 - dRe1 );
		}
	}
	else
	{
		// Laminar flow conditions.
		dLambda = 64.0 / dRe;
	}
	
	return 0.5 * dLambda * dRho * dU * dU / dDiameter;
}

/////////////////////////////////////////////////////////////////////////////
// VALVE CALCULATIONS
/////////////////////////////////////////////////////////////////////////////

// Compute Tau = the ratio between the turbulent Reynolds number and the 
// transitional Reynolds number.
double CalcTau( double dqT, double dNu, double dKv, double dLambda, double dReStar )
{
#ifdef TACBX
	ASSERT( dqT >= 0.0 && dNu > 0.0 && dKv >= 0.0 && dLambda > 0.0 && dReStar > 0.0 );
#endif

	if( dqT <= 0.0 || dNu <= 0.0 || dKv <= 0.0 || dLambda <= 0.0 || dReStar <= 0.0 )
	{
		return 0.0;
	}
	
	if( 0.0 == dKv )
	{
		if( 0.0 == dqT )
		{
			return 0.0;
		}
		else
		{
			return DBL_MAX;
		}
	}
	
	return 214.094893938 * dqT * pow( _PHI, 1.0 / dLambda ) / ( dNu * sqrt( dKv ) * dReStar );
}

// Give Tau and laminar slope (lambda), return the correction factor Fr.
double CalcLowReCorr( double dTau, double dLambda, double dAccuracy )
{
#ifdef TACBS
	ASSERT( dTau >= 0.0 && dLambda > 0.0 && dAccuracy >= 1.E-15 && dAccuracy < 1.0 );
#endif
	
	if( dTau <= 0.0 || dLambda <= 0.0 || dAccuracy < 1.E-15 || dAccuracy >= 1.0 )
	{
		return 1.0;
	}
		
	const double dC1 = 4.0 / ( 1 - _PHI );
	const double dC2 = 2.0 / _DELTA;
	const double dC3 = 0.5 * dLambda;
	double dFr, dOldFr, dGam, dLam, dEps;
	
	int i = 100;	// Maximum allowed iteration
	dFr = min( 0.9, dTau );

	do
	{
		dOldFr = dFr;
		dGam = 0.5 * ( 1.0 + tanh( dC1 * ( dFr - _PHI ) ) );
		dLam = dC3 * ( 1.0 - tanh( dC2 * ( dFr - _FRT ) ) );
		dFr = dTau * dFr;
		dFr = pow( dFr / ( 1.0 + dGam * dFr ), dLam );

#ifndef TACBX
		ASSERT( dFr > 0.0 );
#endif
		
		dEps = fabs( dFr - dOldFr ) / dFr;
	}
	while( ( dEps > dAccuracy ) && ( i-- > 0 ) );

#ifndef TACBX
	ASSERT( dFr > 0.0 && dFr < 1.1 );
#endif
	
	if( i < 0 )
	{
		return 1.0; // Convergence not achieved
	}

	return min( 1.0, max( dFr, DBL_MIN ) );
} 

// Give pressure drop (Dp), fluid viscosity and density (rho and nu),
// catalogue valve Kv (KvCat), valve laminar slope (lambda), valve transition 
// Reynolds (ReStar) and test valve Kv (KvTst). Return the flow.
double CalcValvQ(double dDp, double dRho, double dNu, double dKvCat, double dLambda, double dReStar, double dKvTst, double dAccuracy )
{
#ifdef TACBX
	ASSERT( dDp >= 0.0 && dRho > 0.0 && dNu > 0.0 && dKvCat >= 0.0 && dLambda >= 0.0 && dReStar >= 0.0 && dKvTst >= 0.0 && dAccuracy >= 1.E-15 && dAccuracy <1.0 );
#endif
	
	if( dDp <= 0.0 || dRho <= 0.0 || dNu <= 0.0 || dKvCat <= 0.0 || dLambda <= 0.0 || dReStar <= 0.0 || dKvTst <= 0.0 || dAccuracy < 1.E-15 || dAccuracy >= 1)
	{
		return 0.0;
	}
	
	if( dReStar < 1.0 || dLambda < 0.01 )
	{
		return CalcqT( dKvCat, dDp, dRho );
	}
	else 
	{
	 	return CalcLowReCorr( CalcTau( CalcqT( dKvTst, dDp, dRho ), dNu, dKvTst, dLambda, dReStar ), dLambda, dAccuracy )
				/ CalcLowReCorr( CalcTau( CalcqT( dKvTst, _NORMAL_DP, _NORMAL_RHO ), _NORMAL_NU, dKvTst, dLambda, dReStar ), dLambda, dAccuracy )
				* dKvCat / 36000.0 * sqrt( dDp / dRho );
	}
}

// Give the flow (q), fluid viscosity and density (nu and rho),
// catalogue valve Kv (KvCat), valve laminar slope (lambda), valve transition 
// Reynolds (ReStar) and test valve Kv (KvTst). Return the pressure drop.
double CalcValvDp( double dQ, double dRho, double dNu, double dKvCat, double dLambda, double dReStar, double dKvTst )
{
#ifndef TACBX
	ASSERT( dQ >= 0.0 && dRho > 0.0 && dNu > 0.0 && dKvCat >= 0.0 && dLambda >= 0.0 && dReStar >= 0.0 && dKvTst >= 0.0);
#endif
	
	if( dQ <= 0 || dRho <= 0.0 || dNu <= 0.0 || dKvCat <= 0.0 || dLambda <= 0.0 || dReStar <= 0.0 || dKvTst <= 0.0 )
	{
		return 0.0;
	}

	const double dAccuracy = 1.0e-4;
	const double dEpsilon = 0.01 * dAccuracy;
	const double dOpEpsilon = 1.0 + dEpsilon;

	if( dReStar < 1.0 || dLambda < 0.01 )
	{
		return CalcDp( dQ, dKvCat, dRho);
	}
	else 
	{
		double dQCalc, ddQCalc;
		double dEps, dDp, dOldDp;
		dDp = dRho * pow( dQ *36000.0 / dKvCat, 2.0 );
		int i = 2 * (int)( -log( dAccuracy ) / log(2.0 ) ) + 1;			// maximum iteration.						 
		
		do
		{
			dOldDp = dDp;
			dQCalc = CalcValvQ( dDp, dRho, dNu, dKvCat, dLambda, dReStar, dKvTst,dEpsilon );
			ddQCalc = ( CalcValvQ( dDp * dOpEpsilon, dRho, dNu, dKvCat,dLambda, dReStar, dKvTst, dEpsilon) - dQCalc ) / dEpsilon;
			dDp = ( ddQCalc > 0.0 )? dDp * ( 1.0 + ( dQ - dQCalc ) / ddQCalc ) : dOldDp;
			if( dDp <= 0.0 )
				dDp = 0.5 * dOldDp;
			dEps = fabs( dDp - dOldDp ) / dOldDp;
		}while( ( dEps > dAccuracy ) && ( i-- > 0 ) );
		
		return dDp;
	}
}

// Give the flow (q), pressure drop (Dp), fluid viscosity and density 
// (nu and rho) and valve characteristic (valveChar). Return the valve opening.
bool CalcValvh( double dQ, double dDp, double *pdH, double dRho, double dNu, CDB_ValveCharacteristic *pValveCharacteristic, int iRounding )
{
#ifndef TACBX
	ASSERT( dQ > 0.0 && dDp > 0.0 && dRho > 0.0 && dNu > 0.0 );
#endif
	
	if( dQ <= 0.0 || dDp <= 0.0 || NULL == pdH || dRho <= 0.0 || dNu <= 0.0 || NULL == pValveCharacteristic )
	{
		return false;
	}
	
	const double dAccuracy = 1.0e-4;
	const double dEpsilon = 0.01 * dAccuracy;

	// Rounding cases:
	// Continuous opening - case 1
	// Discrete opening rounded towards closest position - case 2
	// Discrete opening rounded towards the bigger Dp - case 3 (STAM)
	// Discrete opening rounded towards the smaller Dp - case 4 (TRV)
	*pdH = 0.0;

	if( true == pValveCharacteristic->IsViscCorrExist() )
	{
		// With viscosity correction.
		double dHmin = pValveCharacteristic->GetOpeningMin();
		double dHmax = pValveCharacteristic->GetOpeningMax();
		double dDeltaH = 1.0 / ( dHmax - dHmin );

		double dFmin = dQ - CalcValvQ( dDp, dRho, dNu, pValveCharacteristic->GetKv( dHmin ), pValveCharacteristic->GetLambda( dHmin ),
				pValveCharacteristic->GetReStar( dHmin ), pValveCharacteristic->GetKvTst( dHmin ), dEpsilon );

		double dFmax = dQ - CalcValvQ( dDp, dRho, dNu, pValveCharacteristic->GetKv( dHmax ), pValveCharacteristic->GetLambda( dHmax ),
				pValveCharacteristic->GetReStar( dHmax ), pValveCharacteristic->GetKvTst( dHmax ), dEpsilon );
		
		if( dFmin * dFmax > 0.0 )
		{
			// No possible solution.
			return false;
		}

		if( 0.0 == dFmin )
		{
			*pdH = dHmin;
			return true;
		}
		
		double dF, dEps;
		int i = 2 * (int)( -log( dAccuracy ) / log( 2.0 ) );	// Maximum iteration.
		double dOldH;
		*pdH = dHmax;

		do
		{
			dOldH = *pdH;

#ifndef TACBX			
			ASSERT( dFmin != 0.0 );
#endif

			*pdH = dHmin + ( dHmax - dHmin ) / ( 1.0 + fabs( dFmax / dFmin ) );
			dF = dQ - CalcValvQ( dDp, dRho, dNu, pValveCharacteristic->GetKv( *pdH ), pValveCharacteristic->GetLambda( *pdH ),
					pValveCharacteristic->GetReStar( *pdH ), pValveCharacteristic->GetKvTst( *pdH ), dEpsilon );
			
			if( 0.0 == dF )
			{
				dOldH = *pdH;
			}
			else if( dF * dFmin > 0.0 )
			{
				dHmin = *pdH;
				dFmin = dF;
			}
			else
			{
				dHmax = *pdH;
				dFmax = dF;
			}
			dEps = fabs( *pdH - dOldH ) * dDeltaH;
		
		}while( dEps > dAccuracy && ( i-- ) > 0 );
	}
	else
	{
		// Without viscosity correction.
		if( false == pValveCharacteristic->GetOpening( CalcKv( dQ, dDp, dRho ), pdH ) )
		{
			return false;
		}
	}
	
	if( eb3True == pValveCharacteristic->IsDiscrete() )
	{
		switch( iRounding )
		{
			// Rounding == 2 => Rounding to closest int.
			case 2:
				*pdH = max( 1.0, floor( *pdH + 0.5 ) );
				break;
		
			// Rounding == 3 => Decimal part truncation.
			case 3:

				if( *pdH < 1.0 )
				{
					// If opening is between 0 and 1 with rounding equal to 3 we reject.
					return false;
				}
				
				*pdH = floor( *pdH );
				break;

			// Rounding == 4 => Rounding to upper int.
			case 4:

				if( *pdH < 1.0 )
				{
					// If opening is between 0 and 1 with rounding equal to 4 we reject.
					return false;
				}
				
				*pdH = ceil( *pdH );
				break;
		}
	}
	
	return true;
}

