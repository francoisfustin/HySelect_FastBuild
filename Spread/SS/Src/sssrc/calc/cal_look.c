//--------------------------------------------------------------------
//
//  File: cal_look.c
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

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_iseven(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
 
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetError(lpResult);
    else
    {
      double dfNumber = ValGetDouble(lpArg);
      ValSetBool(lpResult, fmod(ApproxFloor(fabs(dfNumber)), 2.0) == 0.0);
    }
  }
  else  
    ValSetError(lpResult);
  return TRUE;
}
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_isodd(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else
    {
      double dfNumber = ValGetDouble(lpArg);
      ValSetBool(lpResult, fmod(ApproxFloor(fabs(dfNumber)), 2.0) != 0.0);
    }
  }
  else  
    ValSetError(lpResult);
  return TRUE;
}
#endif

//--------------------------------------------------------------------
 
#ifdef SS_V30
BOOL CALLBACK Calc_isnumber(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;

  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
    ValSetBool(lpResult, bAnyDouble || bAnyLong);
  else
    ValSetBool(lpResult, FALSE);
  return TRUE;
}
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_istext(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;

  if (CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr))
    ValSetBool(lpResult, bAnyStr);
  else
    ValSetBool(lpResult, FALSE);
  return TRUE;
}
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_isnontext(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;

  if (CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr))
    ValSetBool(lpResult, !bAnyStr);
  else
    ValSetBool(lpResult, TRUE);
  return TRUE;
}
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_isref(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  if (CheckAllCellOrRange(lpArg, nArgs))
    ValSetBool(lpResult, TRUE);
  else
    ValSetBool(lpResult, FALSE);
  return TRUE;
}
#endif
