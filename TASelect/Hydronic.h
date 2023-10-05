//////////////////////////////////////////////////////////////////////////
//  Hydronic.h : header file
//	Version: 1.0							Date: 10/04/08
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "global.h"

// Flow conditions for which one should retrieve the catalogue values.
// Pressure drop, viscosity, density:
#define _NORMAL_DP		10000.0
#define _NORMAL_NU		1.0e-6
#define _NORMAL_RHO		1000.0

class CDB_ValveCharacteristic;

/////////////////////////////////////////////////////////////////////////////
// BASIC CALCULATIONS
/////////////////////////////////////////////////////////////////////////////

// Compute hydraulic resistance based on Kv value.
double CalcR( double dKv, double dRho );

// Compute Kv value based on hydraulic resistance value.
double CalcKv( double dR, double dRho );

// Compute the turbulent water flow rate from the Dp and Kv.
double CalcqT( double dKv, double dDp, double dRho );

// Compute the Dp from the turbulent water flow rate and Kv.
double CalcDp( double dQ, double dKv, double dRho );

// Compute the Kv from the turbulent water flow rate and Dp.
double CalcKv( double dQ, double dDp, double dRho );

// Compute the power from a flow and DT.
double CalcP( double dQ, double dDT, double dRho, double dcp );

// Compute the flow from a power and DT.
double CalcqFromPDT( double dP, double dDT, double dRho, double dcp );

// Compute the DT from a flow and power.
double CalcDT( double dP, double dQ, double dRho, double dcp );

// Compute the Dp from the Dzeta, coefficient speed and density rho.
double CalcDpFromDzeta( double dDzeta, double dU, double dRho ); 

// Compute the Kv coefficient from the Dzeta coefficient and internal pipe diameter.
double CalcKvFromDzeta( double dDzeta, double dDiamater );

/////////////////////////////////////////////////////////////////////////////
// PIPE CALCULATIONS
/////////////////////////////////////////////////////////////////////////////

// Compute the linear pressure drop.
// Parameters are: flow (dQ), fluid density(dRho), fluid viscosity (dNu), pipe roughness (dRoughness),
// internal diameter(dDiameter) and arithmetic accuracy.
double CalcPipeDp( double dQ, double dRho, double dNu, double dRoughness, double dDiameter, double dAccuracy = 1e-8 );

/////////////////////////////////////////////////////////////////////////////
// VALVE CALCULATIONS
/////////////////////////////////////////////////////////////////////////////

// Compute Tau = the ratio between the turbulent Reynolds number and the transitional Reynolds number.
double CalcTau( double dqT, double dNu, double dKv, double dLambda, double dReStar );

// Give Tau and laminar slope (dLambda), return the correction factor Fr.
double CalcLowReCorr( double dTau, double dLambda, double dAccuracy );

// Give pressure drop (dDp), fluid density and viscosity (dRho and dNu), valve Kv (real : Kv, catalog : dKvCat), 
// valve laminar slope (dLambda) and valve transition Reynolds (dReStar).
// Return the flow.
double CalcValvQ( double dDp, double dRho, double dNu, double dKvCat, double dLambda, double dReStar, double dKvTst, double dAccuracy = 1e-4 );

// Give the flow (dQ), fluid density and viscosity (dRho and dNu), catalog valve Kv (dKvCat), valve laminar slope (dLambda),
// valve transition Reynolds (dReStar) and test valve Kv (dKvTst). Return the pressure drop.
double CalcValvDp( double dQ, double dRho, double dNu, double dKvCat, double dLambda, double dReStar, double dKvTst );

// Give the flow (dQ), pressure drop (dDp), fluid density and viscosity (dRho and dNu) and valve CurvFit (pValveCharacteristic).
// Return the valve opening.
bool CalcValvh( double dQ, double dDp, double *pdH, double dRho, double dNu, CDB_ValveCharacteristic *pValveCharacteristic, int iRounding );
