//--------------------------------------------------------------------
//
//  File: cal_fina.c
//
//  Description: Built-in financial functions
//

#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "calc.h"
#include "cal_dde.h"
#include "cal_func.h"
#include "cal_expr.h"
#ifdef SS_V80
#include "..\..\..\include\toolbox.h"
#include "..\edit\editdate\calendar.h"
#include "..\spread\ftdattim.h"
#endif

#ifdef SS_V80
extern int annual_year_basis (double date, int basis);
extern int days_monthly_basis (double date_i, double date_m, int basis);
extern int days_between_BASIS_MSRB_30_360 (double from, double to);
extern int days_between_basis (double from,  double to, int basis);

#endif

extern BOOL CheckAllNumOrEmpty(LPCALC_VALUE lpArg, short nArgs,
                               BOOL FAR* pbAnyDouble, BOOL FAR* pbAnyLong,
                               BOOL FAR* pbAnyEmpty);

#ifdef SS_V30
BOOL CALLBACK Calc_sln(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double cost,salvage,life,sln;
 
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else
    {
      errno = 0;
      cost=ValGetDouble(lpArg);
      salvage=ValGetDouble(lpArg+1);
      if (salvage < 0)
      {
        ValSetError(lpResult);
        return TRUE;
      }
      life=ValGetDouble(lpArg+2);
      if (life !=0)  
        sln=(cost-salvage)/life;
      ValSetDouble(lpResult,sln);
    }
  }
  else
    ValSetError(lpResult);

  return TRUE;
}      
#endif
 
#ifdef SS_V30
BOOL CALLBACK Calc_syd(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double cost;
  double salvage;
  double life;
  double per;
  double syd;
 
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else
    {
      cost = ValGetDouble(lpArg);
      salvage = ValGetDouble(lpArg+1);
      life = ValGetDouble(lpArg+2);
      per = ValGetDouble(lpArg+3);
      if (0.0 <= salvage && 0.0 < per && per <= life)
      {
        syd = ((cost - salvage) * (life - per + 1.0) * 2.0) / (life * (life + 1.0));  
        ValSetDouble(lpResult, syd);
      }
      else
        ValSetError(lpResult);
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
} 
#endif

#ifdef SS_V30
BOOL CALLBACK Calc_fv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double rate;
  double nper;
  double pmt;
  double pv;
  double type;
  double fv;
 
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    if (bAnyEmpty)
      ValSetEmpty(lpResult);
    else
    {
      errno = 0;
      rate = ValGetDouble(lpArg);
      nper = ValGetDouble(lpArg+1);
      pmt = ValGetDouble(lpArg+2);
      pv = nArgs > 3 ? ValGetDouble(lpArg+3) : 0.0;
      type = nArgs > 4 ? ValGetDouble(lpArg+4) : 0.0;
      if (rate == 0.0)
        fv = -(pv + pmt * nper);
      else
      {
        double term = pow((1.0 + rate), nper);
        if (type == 0.0)
          fv = -((pv * term) + pmt * (term - 1.0) / rate);
        else 
          fv = -((pv * term) + pmt * (1.0 + rate) * (term - 1.0) / rate);
      }   
      ValSetDouble(lpResult,fv);
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

#ifdef SS_V30
BOOL CALLBACK Calc_pv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double rate;
  double nper;
  double pmt;
  double fv;
  double type;
  double pv;
 
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    if (bAnyEmpty)
      ValSetEmpty(lpResult);
    else
    {
      errno = 0;
      rate = ValGetDouble(lpArg);
      nper = ValGetDouble(lpArg+1);
      pmt = ValGetDouble(lpArg+2);
      fv = nArgs > 3 ? ValGetDouble(lpArg+3) : 0.0;
      type = nArgs > 4 ? ValGetDouble(lpArg+4) : 0.0;
      if (rate == 0.0)
        pv = -(fv + pmt * nper);
      else
      {
        double term;
        term = pow((1.0 + rate), nper);
        if (type == 0.0)
          pv = -(pmt * (term - 1.0) / rate + fv) / term;  
        else 
          pv = -(pmt * (1.0 + rate) * (term - 1.0) / rate + fv) / term;  
      }
      ValSetDouble(lpResult,pv);
    }
  }
  else
    ValSetError(lpResult);

  return TRUE;
}      
#endif

#ifdef SS_V30
BOOL CALLBACK Calc_nper(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double rate;
  double nper;
  double pmt;
  double pv;
  double type;
  double fv;
 
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    if (bAnyEmpty)
      ValSetEmpty(lpResult);
    else
    {
      errno = 0;
      rate = ValGetDouble(lpArg);
      pmt = ValGetDouble(lpArg+1);
      pv = ValGetDouble(lpArg+2);
      fv = nArgs > 3 ? ValGetDouble(lpArg+3) : 0.0;
      type = nArgs > 4 ? ValGetDouble(lpArg+4) : 0.0;
      if (rate == 0.0 && pmt == 0.0)
        ValSetError(lpResult);
      else
      {
        if (rate == 0.0)
          nper = -(pv + fv) / pmt;
        else if (type == 0.0)
          nper = log(-(rate * fv - pmt)/(rate * pv + pmt)) / log(1.0 + rate);
        else
          nper = log(-(rate * fv - pmt * (1.0 + rate))/(rate * pv + pmt * (1.0 + rate))) / log(1.0 + rate);
        ValSetDouble(lpResult,nper);
      }
    }
  }
  else
    ValSetError(lpResult);

  return TRUE;
}
#endif
      
#ifdef SS_V30
BOOL CALLBACK Calc_db(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double cost;
  double salvage;
  long life;
  long period;
  long month;
  long lastPeriod;
  long i;
  double rate;
  double total = 0.0;
  double db = 0.0;
 
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else
    {
      cost = ValGetDouble(lpArg);
      salvage = ValGetDouble(lpArg+1);
      life = ValGetLong(lpArg+2);
      period = ValGetLong(lpArg+3);
      month = nArgs > 4 ? ValGetLong(lpArg+4) : 12;
      lastPeriod = month < 12 ? life + 1 : life;
      if (0.0 <= cost && 0.0 <= salvage && 1 <= life && 1 <= period && period <= lastPeriod && 1 <= month && month <= 12)
      {
        if (cost == 0.0)
        {
          ValSetDouble(lpResult, 0.0);
        }
        else
        {
          rate = ApproxRound(1 - pow(salvage / cost, 1.0 / (double)life), 3); 
          for (i = 1; i <= period; i++)
          {
            if (i == 1)
              db = cost * rate * (double)month / 12.0;
            else if (i == life + 1)
              db = (cost - total) * rate * (12.0 - (double)month) / 12.0;
            else
              db = (cost - total) * rate;
            total += db;
          }
          ValSetDouble(lpResult, db);
        }
      }
      else
        ValSetError(lpResult);
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

#ifdef SS_V30
BOOL CALLBACK Calc_ddb(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double cost,salvage,life,period,factor,ddb,temp,totdb,tdb;
 
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else
    {
      errno = 0;
      temp=1;
      cost=ValGetDouble(lpArg);
      salvage=ValGetDouble(lpArg+1);
      if (salvage < 0)
      {
        ValSetError(lpResult);
        return TRUE;
      }
      life=ValGetDouble(lpArg+2);
      period=ValGetDouble(lpArg+3);
      if (period > life)
        ValSetError(lpResult);
      else
      {
        if (nArgs == 5)
          factor=ValGetDouble(lpArg+4);
        else
          factor=2;    
        if (cost < 0 || salvage < 0 || life < 0 || period < 0 || factor < 0 )
        {
          ValSetError(lpResult);
          return TRUE;
        }
        ddb=cost*(factor/life); 
        totdb=ddb;
        while (temp < period)
        {     
          ddb=(cost- totdb)*(factor/life);     
          tdb=totdb;
          totdb+=ddb;
          temp+=1;    
        }    
        if ((cost-totdb) < salvage)
          ddb=(cost-tdb)-salvage;
      
        if (ddb < 0)
          ddb = 0;

        ValSetDouble(lpResult,ddb);
      }
    }
  }
  else
    ValSetError(lpResult);

  return TRUE;
}       
#endif
#ifdef SS_V80

BOOL CALLBACK Calc_accrint(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double maturity, first_interest, settlement, rate, par;
  double a, d, coefficient, x;
  long freq;
  long basis = 0;

  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    maturity = ValGetDouble(lpArg);
    first_interest = ValGetDouble(lpArg+1);
	settlement = ValGetDouble(lpArg+2);
    rate = ValGetDouble(lpArg+3);
	par = ValGetDouble(lpArg+4);
	freq = ValGetLong(lpArg+5);
	if (nArgs > 6)
		basis = ValGetLong(lpArg+6);

    if (rate <= 0.0 || par <= 0.0) 
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (basis < 0 || 4 < basis)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (!(freq == 1 || freq == 2 || freq == 4))
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
      if (maturity >= settlement)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
      a = days_monthly_basis(maturity, settlement, basis);
      d = annual_year_basis(maturity, basis);
      if (a < 0 || d <= 0) 
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
      coefficient = par * rate / (double)freq;
      x = a / d;
      ValSetDouble(lpResult, coefficient * (double)freq * x);
	  return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
BOOL CALLBACK Calc_accrintm(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double maturity, issue, rate;
  double a, d;
  long basis = 0;
  double par = 1000.0;

  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    issue = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
    rate = ValGetDouble(lpArg+2);
	if (nArgs > 3)
		par = ValGetDouble(lpArg+3);
	if (nArgs > 4)
		basis = ValGetLong(lpArg+4);
    
	if (rate <= 0.0 || par <= 0.0 || basis < 0 || 4 < basis)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (issue > maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
      a = days_monthly_basis(issue, maturity, basis);
      d = annual_year_basis(issue, basis);
      if (a < 0 || d <= 0) 
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
      ValSetDouble(lpResult, par * rate * a/d);
	  return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_nominal(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double rate;
  double nper;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    rate = ValGetDouble(lpArg);
    nper = ValGetDouble(lpArg+1);
    if (rate <= 0.0 || nper < 1.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}
	else 
		dfResult = nper * (pow(1.0 + rate, 1.0 / nper) - 1.0);

    if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_effect(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double rate;
  double nper;
  double dfResult = 0.0;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    rate = ValGetDouble(lpArg);
    nper = ValGetDouble(lpArg+1);
    if (rate <= 0.0 || nper < 1.0)
	{
		ValSetError(lpResult);
        return TRUE;
	}
    else 
    {
		dfResult = pow ((1.0 + rate / nper), nper) - 1.0;
	}

	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
BOOL CALLBACK Calc_npv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  BOOL bAnyEmpty = FALSE;
  long lCol;
  long lRow;

  double dRate = 0.0;
  double dNpv = 0.0;
  int i = 1;

  if (nArgs > 0)
  {
  if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
  else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        dRate = (double)lpArg->Val.ValLong;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        dRate = lpArg->Val.ValDouble;		
        bAnyDouble = TRUE;
      }
	}
  lpArg++;
  nArgs--;
  }
  else 
	  bAnyError = TRUE;
  errno = 0;
  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        dNpv += (double)lpArg->Val.ValLong / pow(1.0 + dRate, i);                                             
        bAnyLong = TRUE;
		i++;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        dNpv += lpArg->Val.ValDouble / pow(1.0 + dRate, i);                                             
        bAnyDouble = TRUE;
		i++;
      }
      else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
				dNpv += (double)lpArg->Val.ValLong / pow(1.0 + dRate, i);                                             
				bAnyDouble = TRUE;
				i++;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				dNpv += lpArg->Val.ValDouble / pow(1.0 + dRate, i);                                             
				bAnyDouble = TRUE;
				i++;
              }
            }
            ValFree(&value);
          }
      }
    }
  }
  
  if( bAnyError )
    ValSetError(lpResult);
  else
    ValSetDouble(lpResult, dNpv);
  return TRUE;
}

static double calculate_pvif (double rate, double nper)
      {
        double ret = (pow (1.0 + rate, nper));
        return ret;
      }

static double calculate_fvifa (double rate, double nper)
      {
        if (rate == 0.0)
          return nper;
        else
        {
          double x = pow (1.0 + rate, nper);
          double y = x - 1.0;
          double ret = y / rate;
          return ret;
        }
      }
static double calculate_interest_part (double pv, double pmt, double rate, double per)
      {
        double x = pow (1.0 + rate, per);
        double y = x - 1.0;
        double ret = -(pv * x * rate + pmt * y);
        return ret;

      }

static double calculate_pmt (double rate, double nper, double pv, double fv, int type)
      {
        double pvif, fvifa;
        double x,y,z,ret;

        pvif = calculate_pvif (rate, nper);
        fvifa = calculate_fvifa (rate, nper);
        
        x = -pv * pvif - fv;
        y = 1.0 + rate * (double)type;
        z = y * fvifa;
        ret = x / z;
        return ret;
      }

BOOL CALLBACK Calc_ipmt(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double rate;
  double per;
  double nper;
  double pv;
  double fv = 0.0;
  int type = 0;
  double dfResult = 0.0;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    rate = ValGetDouble(lpArg);
    per = ValGetDouble(lpArg+1);
    nper = ValGetDouble(lpArg+2);
    pv = ValGetDouble(lpArg+3);
	if (nArgs > 4)
		fv = ValGetDouble(lpArg+4);
	if (nArgs > 5)
		type = ValGetLong(lpArg+5);

    if (per < 1.0 || per >= nper + 1.0 || nper < 1.0)
	{
		ValSetError(lpResult);
        return TRUE;
	}
    else 
      {
        double pmt = calculate_pmt(rate, nper, pv, fv, type);
        dfResult = calculate_interest_part (pv, pmt, rate, per - 1.0);
      }
  }
  else
  {
	  ValSetError(lpResult);
	  return TRUE;
  }
	  

	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}




BOOL CALLBACK Calc_ppmt(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfResult = 0.0;
  double rate;
  double per;
  double nper;
  double pv;
  double fv = 0.0;
  double pmt,ipmt;
  BOOL type = FALSE;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    rate = ValGetDouble(lpArg);
    per = ValGetDouble(lpArg+1);
    nper = ValGetDouble(lpArg+2);
    pv = ValGetDouble(lpArg+3);
	if (nArgs > 4)
		fv = ValGetDouble(lpArg+4);
	if (nArgs > 5)
		type = ValGetBool(lpArg+5);

	if (per < 1.0 || per >= nper + 1.0)
	{
		ValSetError(lpResult);
        return TRUE;
	}
    else 
      {
        pmt = calculate_pmt (rate, nper, pv, fv, type == TRUE ? 1 : 0);
        ipmt = calculate_interest_part (pv, pmt, rate, per - 1.0);
        dfResult = pmt - ipmt;   
      }

	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

static double getGDA (double x, double y, double z, double period, double factor)
    {
      double gda, ff, aa, nn;

      ff = factor / z;
      if (ff >= 1.0) 
      {
        ff = 1.0;
        if (period == 1.0)
          aa = x;
        else
          aa = 0.0;
      } 
      else
        aa = x * pow(1.0 - ff, period - 1.0);
      nn = x * pow (1.0 - ff, period);

      if (nn < y)
        gda = aa - y;
      else
        gda = aa - nn;
      if (gda < 0.0)
        gda = 0.0;
      return gda;
    }

static double interVDB (double cost, double salvage, double life, double life1, double period, double factor)
    {
      double fVdb       = 0;
      double fIntEnd    = ceil (period);
      int        nLoopEnd   = (int)fIntEnd;

      double fTerm, fLia;
      double rr  = cost - salvage;
      BOOL   bNowLia    = FALSE;

      double fGda;
      int        i;

      fLia = 0;
      for ( i = 1; i <= nLoopEnd; i++ ) 
      {
        if (!bNowLia) 
        {
          fGda = getGDA (cost, salvage, life, i, factor);
          fLia = rr / (life1 - (double)(i - 1));

          if (fLia > fGda) 
          {
            fTerm   = fLia;
            bNowLia = TRUE;
          } 
          else 
          {
            fTerm      = fGda;
            rr -= fGda;
          }
        } 
        else
          fTerm = fLia;

        if ( i == nLoopEnd)
          fTerm *= ( period + 1.0 - fIntEnd );

        fVdb += fTerm;
      }
      return fVdb;
    }

    static double get_vdb (double cost, double salvage, double life,
      double start_period, double end_period, double factor,
      BOOL flag)
    {
      double fVdb;
      double fIntStart = floor(start_period);
      double fIntEnd   = ceil(end_period);
      int        i;
      int        nLoopStart = (int) fIntStart;
      int        nLoopEnd   = (int) fIntEnd;

      fVdb      = 0.0;

      if ( flag ) 
      {
        for (i = nLoopStart + 1; i <= nLoopEnd; i++) 
        {
          double fTerm;

          fTerm = getGDA (cost, salvage, life, i, factor);
          if ( i == nLoopStart+1 )
            fTerm *= ( min( end_period, fIntStart + 1.0 )
              - start_period );
          else if ( i == nLoopEnd )
            fTerm *= ( end_period + 1.0 - fIntEnd );
          fVdb += fTerm;
        }
      } 
      else 
      {
        double life1 = life;
        double fPart;

        if ( start_period != floor(start_period) )
          if (factor > 1.0) 
          {
            if (start_period >= life / 2.0) 
            {
              fPart        = start_period - life / 2.0;
              start_period = life / 2.0;
              end_period  -= fPart;
              life1       += 1.0;
            }
          }

        cost -= interVDB (cost, salvage, life, life1, start_period,
          factor);
        fVdb = interVDB (cost, salvage, life, life - start_period,
          end_period - start_period, factor);
      }
      return fVdb;
    }

BOOL CALLBACK Calc_vdb(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double cost;
  double salvage;
  int life;
  double start;
  double end;
  double factor = 2.0;
  BOOL noswitch = FALSE;
  double dfResult = 0.0;
	  
	if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    cost = ValGetDouble(lpArg);
    salvage = ValGetDouble(lpArg+1);
    life = ValGetLong(lpArg+2);
    start = ValGetDouble(lpArg+3);
    end = ValGetDouble(lpArg+4);
	if (nArgs > 5)
		factor = ValGetDouble(lpArg+5);
	if (nArgs > 6)
		noswitch = ValGetBool(lpArg+6);

    if (cost < 0.0 || salvage < 0.0 || life < 0 || start < 0 || end < 0 || end < start)
 	{
		ValSetError(lpResult);
        return TRUE;
	}
    else 
      {
		if (cost < salvage && start == 0.0 && end == 1.0)
			dfResult =  cost - salvage;
		else
		    dfResult = get_vdb (cost, salvage, (double)life, (double)start, (double)end, factor, noswitch);
      }
	}
	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}
BOOL CALLBACK Calc_ispmt(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double rate;
  double per;
  double nper;
  double pv;
  double dfResult = 0.0;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    rate = ValGetDouble(lpArg);
    per = ValGetDouble(lpArg+1);
    nper = ValGetDouble(lpArg+2);
    pv = ValGetDouble(lpArg+3);

    if (nper == 0.0)
	{
		ValSetError(lpResult);
        return TRUE;
	}
    else 
      {
        dfResult = pv * rate * ((double)per / (double)nper - 1.0);
      }

	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
BOOL CALLBACK Calc_fvschedule(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  double fv;
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  BOOL bAnyEmpty = FALSE;
  long lCol;
  long lRow;
  double dfResult = 0.0;

  if (nArgs > 0)
  {
  if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
  else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        fv = (double)lpArg->Val.ValLong;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        fv = lpArg->Val.ValDouble;		
        bAnyDouble = TRUE;
      }
	}
  lpArg++;
  nArgs--;
  }
  else 
	  bAnyError = TRUE;
  errno = 0;
  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
				fv *= 1.0 + (double)value.Val.ValLong;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				fv *= 1.0 + value.Val.ValDouble;
              }
            }
            ValFree(&value);
          }
      }
	  else
		  bAnyError = TRUE;
    }
  }
  
  if( bAnyError )
    ValSetError(lpResult);
  else
    ValSetDouble(lpResult, fv);
  return TRUE;
}

BOOL CALLBACK Calc_disc(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity, par, redemption;
  double dsm, b;
  long basis = 0;

  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
    par = ValGetDouble(lpArg+2);
	redemption = ValGetDouble(lpArg+3);
	if (nArgs > 4)
		basis = ValGetLong(lpArg+4);
    
	if (settlement > maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (par <= 0.0 || redemption <= 0.0 || basis < 0 || 4 < basis)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    b = annual_year_basis(settlement, basis);
    dsm = days_monthly_basis(settlement, maturity, basis);
    if (dsm <= 0 || b <= 0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    ValSetDouble(lpResult, (redemption - par) / redemption * (b / dsm));
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_intrate(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity, investment, redemption;
  double a, d;
  long basis = 0;

  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
    investment = ValGetDouble(lpArg+2);
	redemption = ValGetDouble(lpArg+3);
	if (nArgs > 4)
		basis = ValGetLong(lpArg+4);
    
	if (settlement > maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (investment <= 0.0 || redemption <= 0.0 || basis < 0 || 4 < basis)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    a = days_monthly_basis(settlement, maturity, basis);
    d = annual_year_basis(settlement, basis);
    if (a <= 0 || d <= 0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    ValSetDouble(lpResult, (redemption - investment) / investment * (d / a));
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_received(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity, investment, discount;
  double a, d, n;
  long basis = 0;

  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
    investment = ValGetDouble(lpArg+2);
	discount = ValGetDouble(lpArg+3);
	if (nArgs > 4)
		basis = ValGetLong(lpArg+4);
    
	if (settlement > maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (investment <= 0.0 || discount <= 0.0 || basis < 0 || 4 < basis)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    a = days_monthly_basis(settlement, maturity, basis);
    d = annual_year_basis(settlement, basis);
    if (a <= 0 || d <= 0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    n = 1.0 - (discount * a/d);
    if (n <= 0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }

    ValSetDouble(lpResult, investment / n);
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}


BOOL CALLBACK Calc_tbilleq(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity, discount, dsm, p1, p2;

  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
	discount = ValGetDouble(lpArg+2);
    
	if (settlement > maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (discount <= 0.0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    dsm = maturity - settlement;
    if (dsm > 365.0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    p1 = 365.0 * discount;
    p2 = (360.0 - discount * dsm);
    if (p2 == 0.0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    else if (p2 < 0.0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }

    ValSetDouble(lpResult, p1 / p2);
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_tbillprice(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity, discount, dsm;

  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
	discount = ValGetDouble(lpArg+2);
    
	if (settlement > maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (discount <= 0.0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    dsm = maturity - settlement;
    if (dsm > 365.0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    ValSetDouble(lpResult, 100.0 * (1.0 - (discount * dsm) / 360.0));
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_tbillyield(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity, pr, dsm;

  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
	pr = ValGetDouble(lpArg+2);
    
	if (settlement > maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (pr <= 0.0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    dsm = maturity - settlement;
    if (dsm > 365.0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }

    ValSetDouble(lpResult, ((100.0 - pr) / pr) * (360.0 / dsm));
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

static double AddMonths(double date, int months)
{
	int newMonth, days;
	short nYear, nMonth, nDay;
	SS_DATE dt;

	ftDoubleToDate(date, &nYear, &nMonth, &nDay);

	newMonth = nMonth + months;
	if (newMonth < 1)
	{
		nYear -= abs(months/12);
		if (abs(months % 12) >= nMonth)
			nYear--;
		newMonth = nMonth + abs(months % 12);
		if (newMonth > 12)
			newMonth = newMonth - 12;
	}
	else if (newMonth > 12)
	{
		nYear += abs(months/12);
		if (nMonth >= abs(months%12))
			nYear++;
		newMonth = 12 - (months % 12);
		if (newMonth < 1)
			newMonth = 12 + newMonth;
	}

	dt.nDay = nDay;
	dt.nMonth = newMonth;
	dt.nYear = nYear;

	days = DateGetDaysInMonth(&dt);

	if (nDay > days)
		ftDateToDouble(nYear, newMonth, days, &date);
	else
		ftDateToDouble(nYear, newMonth, nDay, &date);

	return date;

}
  static int coupnum (double settlement, double maturity, int freq )
  {
    int months;
	short matYear, matMon, matDay;
	short setYear, setMon, setDay;
	short coupYear, coupMon, coupDay;
    double   coupondate = maturity;
    SS_DATE dt, coupDt;

	ftDoubleToDate(maturity, &matYear, &matMon, &matDay);
	ftDoubleToDate(settlement, &setYear, &setMon, &setDay);

    months = matMon - setMon + 12 *  (matYear - setYear);
    coupondate = AddMonths(coupondate, -months);

	ftDoubleToDate(coupondate, &coupYear, &coupMon, &coupDay);

	dt.nDay = matDay;
	dt.nMonth = matMon;
	dt.nYear = matYear;

    coupDt.nDay = coupDay;
	coupDt.nMonth = coupMon;
	coupDt.nYear = coupYear;

    if (matDay == DateGetDaysInMonth(&dt))
		coupDt.nDay = DateGetDaysInMonth(&coupDt);


    if (setDay >= coupDt.nDay)
      months--;

    return (1 + months / (12 / freq));
  }

static double coup_cd ( double settlement, double maturity, int freq, BOOL next)
{
  int   ndays, months, periods;
  BOOL   is_eom_special;
  short nMatYear, nMatDay, nMatMon;
  short nSetYear, nSetDay, nSetMon;
  short nResYear, nResDay, nResMon;
  double result = 0.0;
  SS_DATE matDt, resultDt;

  ftDoubleToDate(settlement, &nSetYear, &nSetMon, &nSetDay);
  ftDoubleToDate(maturity, &nMatYear, &nMatMon, &nMatDay);
  matDt.nDay = nMatDay;
  matDt.nMonth = nMatMon;
  matDt.nYear = nMatYear;

  is_eom_special = (nMatDay == DateGetDaysInMonth(&matDt));

  months = 12 / freq;
  periods = (nMatYear - nSetYear);
	if (periods > 0)
		periods = (periods - 1) * freq;

	do {
		result = maturity;
		periods++;
		result = AddMonths(result, -(periods * months));
		if (is_eom_special) 
		{
			ftDoubleToDate(result, &nResYear, &nResMon, &nResDay);
			resultDt.nDay = nResDay;
			resultDt.nMonth = nResMon;
			resultDt.nYear = nResYear;
			ndays = DateGetDaysInMonth(&resultDt);
			ftDateToDouble(nResYear, nResMon, ndays, &result);
		}
	} while (settlement < result );

	if (next)
  {
    result = maturity;
    periods--;
		result = AddMonths(result, -(periods * months));
		if (is_eom_special)
    {
			ftDoubleToDate(result, &nResYear, &nResMon, &nResDay);
			resultDt.nDay = nResDay;
			resultDt.nMonth = nResMon;
			resultDt.nYear = nResYear;
			ndays = DateGetDaysInMonth(&resultDt);
			ftDateToDouble(nResYear, nResMon, ndays, &result);
    }
	}
  return result;
 }

static double coupdays (double settlement, double maturity, int freq, int basis)
{
	double prev, next;

    switch (basis) 
        {
	case 0:
	case 2:
        case 4:
        case 5:
		return 360 / freq;
	case 3:
		return 365 / freq;
	case 1:
	default:
		next = coup_cd (settlement, maturity, freq, TRUE);
		prev = coup_cd (settlement, maturity, freq, FALSE);
		return days_between_basis (prev, next, 1);
        }
}

static double coupdaybs (double settlement, double maturity, int freq, int basis)
{
	double prev_coupon;
	prev_coupon = coup_cd (settlement, maturity, freq, FALSE);
	return days_between_basis (prev_coupon, settlement, basis);
}

static double coupdaysnc (double settlement, double maturity, int freq, int basis)
{
	double next_coupon;
	next_coupon = coup_cd (settlement, maturity, freq, TRUE);
	return days_between_basis (settlement, next_coupon, basis);
}
static double couppcd ( double settlement, double maturity, int freq)
{
  double date;
  date = coup_cd (settlement, maturity, freq, FALSE);
  return date;
}

static double coupncd (double settlement, double maturity, int freq)
{
  double date;
  date = coup_cd (settlement, maturity, freq, TRUE);
  return date;
}

static double price (double settlement, double maturity, double rate, double yield, double redemption, int freq, int basis)
{
  double a, d, e, sum, den, based, exponent, first_term, last_term;
  int       k, n;

  a = coupdaybs (settlement, maturity, freq, basis);
  d = coupdaysnc (settlement, maturity, freq, basis);
  e = coupdays (settlement, maturity, freq, basis);
  n = (int)coupnum (settlement, maturity, freq);

  sum = 0.0;
  den = 100.0 * rate / freq;
  based = 1.0 + yield / freq;
  exponent = d / e;
  for (k = 0; k < n; k++)
  sum += den / pow(based, exponent + k);

  first_term = redemption / pow (based, (n - 1.0 + d / e));
  last_term = a / e * den;

  return (first_term + sum - last_term);
}

BOOL CALLBACK Calc_price(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double maturity, settlement, rate, yield, redem;
  long frequency;
  long basis = 0;

  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
    maturity = ValGetDouble(lpArg+1);
	rate = ValGetDouble(lpArg+2);
    yield = ValGetDouble(lpArg+3);
	redem = ValGetDouble(lpArg+4);
	frequency = ValGetLong(lpArg+5);
	if (nArgs > 6)
		basis = ValGetLong(lpArg+6);

    if (yield < 0.0 || rate < 0.0 || redem == 0.0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (basis < 0 || 4 < basis)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (frequency != 1 && frequency !=2 && frequency != 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
      if (settlement > maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
      ValSetDouble(lpResult, price(settlement, maturity, rate, yield, redem, frequency, basis));
	  return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_pricedisc(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity, redemption, discount;
  double a, d;
  long basis = 0;

  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
    discount = ValGetDouble(lpArg+2);
	redemption = ValGetDouble(lpArg+3);
	if (nArgs > 4)
		basis = ValGetLong(lpArg+4);


	if (settlement > maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (discount <= 0.0 || redemption <= 0.0 || basis < 0 || 4 < basis)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    a = days_monthly_basis(settlement, maturity, basis);
    d = annual_year_basis(settlement, basis);
    if (a <= 0 || d <= 0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    ValSetDouble(lpResult, redemption - discount * redemption * a/d);
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_pricemat(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity, issue , rate, yield;
  double a, b, dsm, dim, n;
  long basis = 0;
  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
    issue = ValGetDouble(lpArg+2);
	rate = ValGetDouble(lpArg+3);
	yield = ValGetDouble(lpArg+4);
	if (nArgs > 5)
		basis = ValGetLong(lpArg+5);

    if (settlement > maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (rate < 0.0 || yield < 0.0 || basis < 0 || 4 < basis)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
      dsm = days_monthly_basis(settlement, maturity, basis);
      dim = days_monthly_basis(issue, maturity, basis);
      a   = days_monthly_basis(issue, settlement, basis);
      b   = annual_year_basis(settlement, basis);
      if (a <= 0.0 || b <= 0.0 || dsm <= 0.0 || dim <= 0.0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
      n = 1.0 + ((dsm/b) * yield);
      if (n == 0.0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    ValSetDouble(lpResult, ((100.0 + ((dim/b) * rate * 100.0)) / (n)) - ((a/b) * rate * 100.0));
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

static double GetRmz ( double fZins, double fZzr, double fBw, double fZw,
      int nF )
    {
      double fRmz;

      if ( fZins == 0.0 )
        fRmz = ( fBw + fZw ) / fZzr;
      else 
      {
        double fTerm = pow ( 1.0 + fZins, fZzr );		
        if ( nF > 0 )
          fRmz = ( fZw * fZins / ( fTerm - 1.0 ) + fBw * fZins /
            ( 1.0 - 1.0 / fTerm ) ) / ( 1.0 + fZins );
        else
          fRmz = fZw * fZins / ( fTerm - 1.0 ) + fBw * fZins /
            ( 1.0 - 1.0 / fTerm );
      }

      return -fRmz;
    }

static double GetZw ( double fZins, double fZzr, double fRmz, double fBw,
      int nF )
    {
      double fZw;

      if ( fZins == 0.0 )
        fZw = fBw + fRmz * fZzr;
      else 
      {
        double fTerm = pow ( 1.0 + fZins, fZzr );
        if ( nF > 0 )
          fZw = fBw * fTerm + fRmz * ( 1.0 + fZins ) *
            ( fTerm - 1.0 ) / fZins;
        else
          fZw = fBw * fTerm + fRmz * ( fTerm - 1.0 ) / fZins;
      }

      return -fZw;
    }

static double get_cumipmt (double fRate, int nNumPeriods, double fVal,
      int nStart, int nEnd, int nPayType)
    {
      double fRmz, fZinsZ;
      int       i;
	  
	  int c = 0;
	  double m = 1.0;
	  if (fRate > 1.0)
	  {
		TCHAR    szNum[25];
		int val = (int)floor(fRate);
		IntToString(val, szNum);
	    c = lstrlen(szNum);
	    m = pow(10, c);
	    fRate = fRate/m;
	  }
      fRmz = GetRmz ( fRate, nNumPeriods, fVal, 0.0, nPayType );

      fZinsZ = 0.0;

      if ( nStart == 1 ) 
      {
        if ( nPayType <= 0 )
          fZinsZ = -fVal;

        nStart++;
      }

      for ( i = nStart ; i <= nEnd ; i++ ) 
      {
        if ( nPayType > 0 )
          fZinsZ += GetZw ( fRate, ( i - 2 ), fRmz, fVal, 1 ) 
            - fRmz;
        else
          fZinsZ += GetZw ( fRate, ( i - 1 ), fRmz, fVal, 0 );
      }

      fZinsZ *= fRate;

      return fZinsZ * m;
    }

BOOL CALLBACK Calc_cumipmt(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double rate;
  int pers;
  double val;
  int startPer;
  int endPer;
  int payType;
  double dfResult = 0.0;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    rate = ValGetDouble(lpArg);
	pers = ValGetLong(lpArg+1);
	val = ValGetDouble(lpArg+2);
	startPer = ValGetLong(lpArg+3);
	endPer = ValGetLong(lpArg+4);
	payType = ValGetLong(lpArg+5);

    if ( startPer < 1 || endPer < startPer || rate <= 0.0
        || endPer > pers || pers <= 0
        || val <= 0.0 || (payType != 0 && payType != 1) ) 
	{
		ValSetError(lpResult);
		return TRUE;
	}

    dfResult = get_cumipmt (rate, pers, val, startPer, endPer, payType);
	ValSetDouble(lpResult, dfResult);
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

static double get_cumprinc  (double fRate, int nNumPeriods, double fVal,
      int nStart, int nEnd, int nPayType)
    {
      double fRmz, fKapZ;
      int       i;

      fRmz = GetRmz ( fRate, nNumPeriods, fVal, 0.0, nPayType );

      fKapZ = 0.0;

      if ( nStart == 1 ) 
      {
        if ( nPayType <= 0 )
          fKapZ = fRmz + fVal * fRate;
        else
          fKapZ = fRmz;

        nStart++;
      }

      for ( i = nStart ; i <= nEnd ; i++ ) 
      {
        if ( nPayType > 0 )
          fKapZ += fRmz - ( GetZw ( fRate, ( i - 2 ), fRmz,
            fVal, 1 ) - fRmz ) * fRate;
        else
          fKapZ += fRmz - GetZw( fRate, ( i - 1 ), fRmz, fVal,
            0 ) * fRate;
      }

      return  fKapZ;
    }

BOOL CALLBACK Calc_cumprinc(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double rate;
  int pers;
  double val;
  int startPer;
  int endPer;
  int payType;
  double dfResult = 0.0;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    rate = ValGetDouble(lpArg);
	pers = ValGetLong(lpArg+1);
	val = ValGetDouble(lpArg+2);
	startPer = ValGetLong(lpArg+3);
	endPer = ValGetLong(lpArg+4);
	payType = ValGetLong(lpArg+5);

    if ( startPer < 1 || endPer < startPer || rate <= 0.0
        || endPer > pers || pers <= 0
        || val <= 0.0 || (payType != 0 && payType != 1) ) 
	{
		ValSetError(lpResult);
		return TRUE;
	}

    dfResult = get_cumprinc (rate, pers, val, startPer, endPer, payType);
	ValSetDouble(lpResult, dfResult);
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_coupdaybs(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity;
  long frequency;
  long basis = 0;
  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
    frequency = ValGetLong(lpArg+2);
	if (nArgs > 3)
		basis = ValGetLong(lpArg+3);
    
	if (settlement >= maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (basis < 0 || basis > 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (frequency != 1 && frequency !=2 && frequency != 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }

    ValSetDouble(lpResult, coupdaybs(settlement, maturity, frequency, basis));
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_coupdays(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity;
  long frequency;
  long basis = 0;
  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
    frequency = ValGetLong(lpArg+2);
	if (nArgs > 3)
		basis = ValGetLong(lpArg+3);
    
	if (settlement >= maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (basis < 0 || basis > 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (frequency != 1 && frequency !=2 && frequency != 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }

    ValSetDouble(lpResult, coupdays(settlement, maturity, frequency, basis));
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_coupdaysnc(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity;
  long frequency;
  long basis = 0;
  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
    frequency = ValGetLong(lpArg+2);
	if (nArgs > 3)
		basis = ValGetLong(lpArg+3);
    
	if (settlement >= maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (basis < 0 || basis > 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (frequency != 1 && frequency !=2 && frequency != 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }

    ValSetDouble(lpResult, coupdaysnc(settlement, maturity, frequency, basis));
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_coupncd(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity;
  long frequency;
  long basis = 0;
  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
    frequency = ValGetLong(lpArg+2);
	if (nArgs > 3)
		basis = ValGetLong(lpArg+3);
    
	if (settlement >= maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (basis < 0 || basis > 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (frequency != 1 && frequency !=2 && frequency != 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }

    ValSetDouble(lpResult, coupncd(settlement, maturity, frequency));
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_couppcd(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity;
  long frequency;
  long basis = 0;
  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
    frequency = ValGetLong(lpArg+2);
	if (nArgs > 3)
		basis = ValGetLong(lpArg+3);
    
	if (settlement >= maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (basis < 0 || basis > 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (frequency != 1 && frequency !=2 && frequency != 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }

    ValSetDouble(lpResult, couppcd(settlement, maturity, frequency));
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_coupnum(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double settlement, maturity;
  long frequency;
  long basis = 0;
  
  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    settlement = ValGetDouble(lpArg);
	maturity = ValGetDouble(lpArg+1);
    frequency = ValGetLong(lpArg+2);
	if (nArgs > 3)
		basis = ValGetLong(lpArg+3);
    
	if (settlement >= maturity)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (basis < 0 || basis > 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
    if (frequency != 1 && frequency !=2 && frequency != 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }

    ValSetDouble(lpResult, coupnum(settlement, maturity, frequency));
	return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

static void goal_seek_initialise (LPGOAL_SEEK data)
    {
      data->havexpos = FALSE;
	  data->havexneg = FALSE;
      data->xmin = -1.0e10;
      data->xmax = +1.0e10;
      data->precision = 1.0e-10;
    }    
static BOOL update_data (double x, double y, LPGOAL_SEEK data)
    {
      if (y > 0) 
      {
        if (data->havexpos) 
        {
          if (data->havexneg) 
          {
            if (fabs (x - data->xneg) < fabs (data->xpos - data->xneg)) 
            {
              data->xpos = x;
              data->ypos = y;
            }
          } 
          else if (y < data->ypos) 
          {
            /* We have pos only and our neg y is closer to zero.  */
            data->xpos = x;
            data->ypos = y;
          }
        } 
        else 
        {
          data->xpos = x;
          data->ypos = y;
          data->havexpos = TRUE;
        }
        return FALSE;
      } 
      else if (y < 0) 
      {
        if (data->havexneg) 
        {
          if (data->havexpos) 
          {
            if (fabs (x - data->xpos) < fabs (data->xpos - data->xneg)) 
            {
              data->xneg = x;
              data->yneg = y;
            }
          } 
          else if (-y < -data->yneg) 
          {
            data->xneg = x;
            data->yneg = y;
          }

        } 
        else 
        {
          data->xneg = x;
          data->yneg = y;
          data->havexneg = TRUE;
        }
        return FALSE;
      } 
      else 
      {
        data->root = x;
        return TRUE;
      }
    }   

    static BOOL irr_npv (double rate, double* y, double FAR* vals, int n)
    {
        double sum;
        int i;

        sum = 0.0;
        for (i = 0; i < n; i++)
            sum += vals[i] * pow(1.0 + rate, (double)(n - i));

        *y = sum;
        return TRUE;

    }

    static BOOL irr_npv_df(double rate, double* y, double FAR* vals, int n)
    {
        double sum;
        int i;

        sum = 0.0;
        for (i = 0; i < n - 1; i++)
            sum += vals[i] * (((double)(n - i)) * pow(1.0 + rate, (double)(n - i - 1.0)));

        *y = sum;
        return TRUE;
    }

   static BOOL fake_df(double x, double* lpdfx, double xstep, LPGOAL_SEEK data, double FAR* vals, int count)
    {
        double xl, xr;
        BOOL status;
        double yl = 0.0;
        double yr = 0.0;

        xl = x - xstep;
        if (xl < data->xmin)
            xl = x;

        xr = x + xstep;
        if (xr > data->xmax)
            xr = x;

        if (xl == xr)
            return FALSE;

        status = irr_npv(xl, &yl, vals, count);
        if (status != TRUE)
            return status;

        status = irr_npv(xr, &yr, vals, count);
        if (status != TRUE)
            return status;

        *lpdfx = (yr - yl) / (xr - xl);
        return TRUE;
    }

    static BOOL goal_seek_newton(LPGOAL_SEEK data, double FAR* vals, double x0, int count)
    {
        int iterations;
        double precision = data->precision / 2.0;

        for (iterations = 0; iterations < 20; iterations++)
        {
            double x1, stepsize;
            BOOL status;
            double y0 = 0.0;
            double df0 = 0.0;

            /* Check whether we have left the valid interval.  */
            if (x0 < data->xmin || x0 > data->xmax)
                return FALSE;

            status = irr_npv(x0, &y0, vals, count);
            if (status != TRUE)
                return status;

            if (update_data(x0, y0, data))
                return TRUE;

            status = irr_npv_df(x0, &df0, vals, count);
            if (status != TRUE)
                return status;

            if (df0 == 0)
                return FALSE;

            x1 = x0 - 1.000001 * y0 / df0;
            if (x1 == x0)
            {
                data->root = x0;
                return TRUE;
            }

            stepsize = fabs(x1 - x0) / (fabs(x0) + fabs(x1));

            x0 = x1;

            if (stepsize < precision)
            {
                data->root = x0;
                return TRUE;
            }
        }

        return FALSE;
    }


    static BOOL goal_seek_point(LPGOAL_SEEK data, double FAR* vals, double x0, int count)
    {
        BOOL status;
        double y0 = 0.0;

        if (x0 < data->xmin || x0 > data->xmax)
            return FALSE;

        status = irr_npv(x0, &y0, vals, count);
        if (status != TRUE)
            return status;

        if (update_data(x0, y0, data))
            return TRUE;

        return FALSE;
    }

    static BOOL goal_seek_bisection(LPGOAL_SEEK data, double FAR* vals, int count)
    {
        int iterations;
        double stepsize;
        int newton_submethod = 0;

        if (!data->havexpos || !data->havexneg)
            return FALSE;

        stepsize = fabs(data->xpos - data->xneg)
          / (fabs(data->xpos) + fabs(data->xneg));

        for (iterations = 0; iterations < 100 + 15 * 4; iterations++)
        {
            double xmid;
            double ymid = 0.0;
            BOOL status;
            int method = 0;

            method = (iterations % 4 == 0)
              ? 1
              : ((iterations % 4 == 2)
              ? 2
              : 3);

        again:
            switch (method)
            {
                default:
                    return FALSE;

                case 0:
                    xmid = data->xpos - data->ypos *
                      ((data->xneg - data->xpos) /
                      (data->yneg - data->ypos));
                    break;

                case 1:
                    {
                        double det;

                        xmid = (data->xpos + data->xneg) / 2.0;
                        status = irr_npv(xmid, &ymid, vals, count);
                        if (status != TRUE)
                            continue;
                        if (ymid == 0)
                        {
                            update_data(xmid, ymid, data);
                            return TRUE;
                        }

                        det = sqrt(ymid * ymid - data->ypos * data->yneg);
                        if (det == 0)
                            continue;

                        xmid += (xmid - data->xpos) * ymid / det;
                        break;
                    }

                case 2:
                    xmid = (data->xpos + data->xneg) / 2.0;
                    break;

                case 3:
                    {
                        double x0, xstep;
                        double y0 = 0.0;
                        double df0 = 0.0;
                        if (stepsize > 0.1)
                        {
                            method = 3;
                            goto again;
                        }

                        switch (newton_submethod++ % 4)
                        {
                            case 0: x0 = data->xpos; x0 = data->ypos; break;
                            case 2: x0 = data->xneg; y0 = data->yneg; break;
                            default:
                            case 3:
                            case 1:
                                x0 = (data->xpos + data->xneg) / 2.0;

                                status = irr_npv(x0, &y0, vals, count);
                                if (status != TRUE)
                                    continue;
                                break;
                        }

                        xstep = fabs(data->xpos - data->xneg) / 1.0e6;
                        status = fake_df(x0, &df0, xstep, data, vals, count);
                        if (status != TRUE)
                            continue;

                        if (df0 == 0)
                            continue;

                        xmid = x0 - 1.01 * y0 / df0;
                        if ((xmid < data->xpos && xmid < data->xneg) ||
                          (xmid > data->xpos && xmid > data->xneg))
                            continue;
                    }
                    break;
            }

            status = irr_npv(xmid, &ymid, vals, count);
            if (status != TRUE)
                continue;


            if (update_data(xmid, ymid, data))
            {
                return TRUE;
            }

            stepsize = fabs(data->xpos - data->xneg)
              / (fabs(data->xpos) + fabs(data->xneg));

            if (stepsize < data->precision)
            {
                if (data->yneg < ymid)
                    ymid = data->yneg; xmid = data->xneg;

                if (data->ypos < ymid)
                    ymid = data->ypos; xmid = data->xpos;

                data->root = xmid;
                return TRUE;
            }
        }
        return FALSE;
    }

 
BOOL CALLBACK Calc_irr(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  long lCol, lRow;
  int count;
  double guess = 0.1;
  double FAR* lpValues = NULL;
  CALC_HANDLE hElem = NULL;
  int index = 0;
  BOOL posVal = FALSE;
  BOOL negVal = FALSE;
  BOOL status;
  GOAL_SEEK data;

  data.xmin = 0.0;
  data.xmax = 0.0;
  data.precision = 0.0;
  data.havexpos = FALSE;
  data.xpos = 0.0;
  data.ypos = 0.0;
  data.havexneg = FALSE;
  data.xneg = 0.0;
  data.yneg = 0.0;
  data.root = 0.0;
  goal_seek_initialise(&data);

  if (nArgs > 1)
	  guess = ValGetDouble(lpArg+1);
  
  if (lpArg->Status == CALC_VALUE_STATUS_OK && lpArg->Type == CALC_VALUE_TYPE_RANGE)
  {
       lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
	   count = ((lpArg->Val.ValRange.Col2-lpArg->Val.ValRange.Col1)+1)*((lpArg->Val.ValRange.Row2-lpArg->Val.ValRange.Row1)+1);
       hElem = CalcMemAlloc(count * sizeof(double));
       if( hElem && (lpValues = (double FAR*)CalcMemLock(hElem)) )
	   {
       for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type || CALC_VALUE_TYPE_DOUBLE == value.Type)
              {
				  lpValues[index] = ValGetDouble(&value);
				  if (lpValues[index] > 0)
					  posVal = TRUE;
				  else if (lpValues[index] < 0)
					  negVal = TRUE;
				  index++;
              }
            }
            ValFree(&value);
          }
	   }
		if (index < 2 || posVal == FALSE || negVal == FALSE)
			status = FALSE;
		else
		{
        data.xmin = max(data.xmin,
          -pow(1.7976931348623158e+308 / 1.0e10, 1.0 / (double)index) + 1.0);
        data.xmax = min(data.xmax,
          pow(1.7976931348623158e+308 / 1.0e10, 1.0 / (double)index) - 1.0);

       
        status = goal_seek_newton(&data, lpValues, guess, index);
		}
        if (status == TRUE)
            ValSetDouble(lpResult, data.root);
        else
            ValSetError(lpResult);

		if( hElem )
		{
			CalcMemUnlock(hElem);
			CalcMemFree(hElem);
		}
		return TRUE;

  }
  else
	  ValSetError(lpResult);

  return TRUE;
}

BOOL CALLBACK Calc_mirr(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  long lCol, lRow;
  CALC_HANDLE hElem = NULL;
  double FAR* lpValues = NULL;
  int index = 0;
  int i, count;
  double frate,rrate;
  int pos = 0;
  int neg = 0;
  double n = 0;
  double posnpv = 0.0;
  double negnpv = 0.0;


  if (nArgs > 1)
	frate = ValGetDouble(lpArg+1);
  if (nArgs > 2)
	rrate = ValGetDouble(lpArg+2);
  
  if (lpArg->Status == CALC_VALUE_STATUS_OK && lpArg->Type == CALC_VALUE_TYPE_RANGE)
  {
       lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
	   count = ((lpArg->Val.ValRange.Col2-lpArg->Val.ValRange.Col1)+1)*((lpArg->Val.ValRange.Row2-lpArg->Val.ValRange.Row1)+1);
       hElem = CalcMemAlloc(count * sizeof(double));
       if( hElem && (lpValues = (double FAR*)CalcMemLock(hElem)) )
	   {

       for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type || CALC_VALUE_TYPE_DOUBLE == value.Type)
              {
				  lpValues[index] = ValGetDouble(&value);
				  if (lpValues[index] > 0)
					  pos++;
				  else if (lpValues[index] < 0)
					  neg++;
				  index++;
              }
            }
            ValFree(&value);
          }
		if (index < 2 || pos == 0 || neg == 0)
		{
			ValSetError(lpResult);
			return TRUE;
		}
       
	    n = (double)(neg + pos);
        for (i = 0; i < n; i++) 
		{
        double v = (double)lpValues[i];
        if (v >= 0.0)
          posnpv += v / pow (1.0 + rrate, (double)i);
        else
          negnpv += v / pow (1.0 + frate, (double)i);
		}

		if( hElem )
		{
			CalcMemUnlock(hElem);
			CalcMemFree(hElem);
		}
        if (negnpv == 0.0 || posnpv == 0.0 || rrate <= -1.0) 
		{
		  ValSetError(lpResult);
          return TRUE;
		}
		ValSetDouble(lpResult,pow ((-posnpv * pow (1.0 + rrate, n)) / (negnpv * (1.0 + rrate)), (1.0 / (n - 1.0))) - 1.0);
		return TRUE;
		}
		else
		   ValSetError(lpResult);

  }
  else
	  ValSetError(lpResult);

  return TRUE;
}
#endif
            