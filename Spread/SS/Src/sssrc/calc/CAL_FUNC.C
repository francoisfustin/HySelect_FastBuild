//--------------------------------------------------------------------
//
//  File: cal_func.c
//
//  Description: Built-in functions
//

#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <float.h>
//#include <time.h>
#include <string.h>
#include <limits.h>

#include "calc.h"
#include "cal_dde.h"
#include "cal_func.h"
#include "cal_expr.h"

// Calc_median() needs QuickSortHuge()
#include "..\..\..\include\toolbox.h"

#ifdef SS_V35
#include "..\..\..\..\fplibs\fpdb\src\fpdbutil.h"
#include "..\spread\ftdattim.h"
#ifdef FP_DLL
#include "ole2.h"
#endif
#include "fptools.h"
#endif
#ifdef SS_V80
#include "..\edit\editdate\calendar.h"

extern double CalcStringToDouble(LPCTSTR lpText);

#endif
//--------------------------------------------------------------------
// Calc_fina.c
extern BOOL CALLBACK Calc_sln(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
extern BOOL CALLBACK Calc_syd(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
extern BOOL CALLBACK Calc_fv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
extern BOOL CALLBACK Calc_pv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
extern BOOL CALLBACK Calc_nper(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
extern BOOL CALLBACK Calc_db(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
extern BOOL CALLBACK Calc_ddb(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
// Calc_look.c
extern BOOL CALLBACK Calc_iseven(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
extern BOOL CALLBACK Calc_isodd(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
extern BOOL CALLBACK Calc_isnumber(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
extern BOOL CALLBACK Calc_istext(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
extern BOOL CALLBACK Calc_isnontext(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
extern BOOL CALLBACK Calc_isref(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);

BOOL CALLBACK Calc_abs(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_acos(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_add(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_and(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_asin(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_atan(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_atan2(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_average(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_ceiling(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs); 
BOOL CALLBACK Calc_combin(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs); 
BOOL CALLBACK Calc_cos(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_cosh(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_degrees(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_even(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_exp(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_fact(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_false(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_floor(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_if(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_int(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_inverse(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_isempty(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_ln(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_log(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_log10(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_max(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_median(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_min(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_mod(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_mode(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_neg(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_not(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_odd(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_or(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_permut(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_pi(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_pmt(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_power(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_product(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_radians(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_rand(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_rank(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_roundup(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_round(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_sign(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_sin(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_sinh(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_sqrt(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_square(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_stdev(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_stdevp(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_sum(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_sumsq(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_tan(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_true(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_tanh(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_rounddown(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_var(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_varp(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_xroot(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_url (LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
#ifdef SS_V35
BOOL CALLBACK Calc_acosh(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_asinh(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_atanh(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_char(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_clean(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_code(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_concatenate(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_exact(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_find(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_isblank(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
//BOOL CALLBACK Calc_islogical(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);

BOOL CALLBACK Calc_date(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_day(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_hour(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_left(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_len(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_lower(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_mid(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_minute(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_month(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_now(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_proper(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_replace(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_rept(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_right(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
//BOOL CALLBACK Calc_search(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_second(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_substitute(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_today(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_time(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_trim(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_trunc(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_upper(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_weekday(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_year(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);

void sjis2jis( unsigned char *c1, unsigned char *c2);
void jis2sjis( unsigned char *c1, unsigned char *c2);
#endif
#ifdef SS_V80
BOOL CALLBACK Calc_avedev(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_binomdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_critbinom(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_negbinomdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_sqrtpi(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_quotient(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_devsq(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_npv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_ipmt(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_ppmt(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_vdb(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_ispmt(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_fvschedule(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_effect(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_gcd(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_lcm(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_nominal(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_count(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_counta(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_subtotal(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_sumproduct(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_seriessum(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_multinomial(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_betadist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_fdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_normsdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_normdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_normsinv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_norminv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_tdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_tinv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_skew(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_poisson(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_weibull(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_gammaln(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_gammadist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_gammainv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_factdouble(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_fisher(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_fisherinv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_lognormdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_loginv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_betainv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_finv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_standardize(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_expondist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_confidence(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_hypgeomdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_chidist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_chiinv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_days360(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_accrint(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_accrintm(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_yearfrac(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_disc(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_intrate(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_received(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_tbilleq(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_tbillprice(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_tbillyield(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_price(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_pricedisc(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_pricemat(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_cumipmt(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_cumprinc(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_coupdaybs(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_coupdays(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_coupdaysnc(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_coupncd(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_coupnum(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_couppcd(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_irr(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_mirr(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_countif(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_sumif(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_pmtxl(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
// 24477 -scl
BOOL CALLBACK Calc_average_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals);
BOOL CALLBACK Calc_max_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals);
BOOL CALLBACK Calc_min_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals);
BOOL CALLBACK Calc_product_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals);
BOOL CALLBACK Calc_stdev_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals);
BOOL CALLBACK Calc_stdevp_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals);
BOOL CALLBACK Calc_sum_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals);
BOOL CALLBACK Calc_var_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals);
BOOL CALLBACK Calc_varp_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals);
BOOL CALLBACK Calc_count_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals);
BOOL CALLBACK Calc_counta_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals);
BOOL variance_info_ex(LPCALC_VALUE lpArg, short nArgs, double FAR *lpdfSum, double FAR *lpdfSum2, double FAR *lpdfCount, BOOL bIncludeSubtotals);
#endif
//--------------------------------------------------------------------
//
//  The FuncTable[] array represents all built-in functions.  Function
//  names must be less than CALC_MAX_NAME_LEN characters (including
//  terminating NULL character).
//
//  Note: The order of the FuncTable table must match the order of the
//        CALC_FUNC_? constants which are defined in CAL_OPER.H file.
//        This allows quicker lookups.
//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//        This is also the table table that the Excel conversion uses for
//        function conversions.
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
//  Note: Functions are always represented by nonzero id codes. Thus,
//        FuncTable[0] is not used.
//

#define CALC_MAX_PARAMS 30

const CALC_FUNCTION FuncTable[] =
{
  {_T(""), NULL, 0, 0, 0},
  {_T("abs"), Calc_abs, 1, 1, 0},
  {_T("add"), Calc_add, 2, 2, 0},
#ifndef SS_V35
  {_T("and"), Calc_and, 2, 2, 0},
#else
  {_T(""), NULL, 2, 2, 0},
#endif
#if SS_V80
  {_T("if"), Calc_if, 2, 3, 0},
#else
  {_T("if"), Calc_if, 3, 3, 0},
#endif
  {_T("isempty"), Calc_isempty, 1, 1, 0},
  {_T("max"), Calc_max, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("min"), Calc_min, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("neg"), Calc_neg, 1, 1, 0},
  {_T("not"), Calc_not, 1, 1, 0},
#ifndef SS_V35
  {_T("or"), Calc_or, 2, 2, 0},
#else
  {_T(""), NULL, 2, 2, 0},
#endif
  {_T("round"), Calc_round, 2, 2, 0},
  {_T("roundup"), Calc_roundup, 2, 2, 0},
  {_T("sum"), Calc_sum, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("truncate"), Calc_rounddown, 2, 2, 0},
  {_T("pmt"), Calc_pmt, 4, 4, 0},
#ifdef SS_DDE
  {_T("ddelink"), NULL, 3, 5, 0}, // psuedo function for DDE links
#else
  {_T(""), NULL, 3, 3, 0},
#endif
#ifdef SS_V30
  {_T("acos"), Calc_acos, 1, 1, 0},
  {_T("asin"), Calc_asin, 1, 1, 0},
  {_T("atan"), Calc_atan, 1, 1, 0},
  {_T("atan2"), Calc_atan2, 2, 2, 0},
  {_T("average"), Calc_average, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("cos"), Calc_cos, 1, 1, 0},
  {_T("cosh"), Calc_cosh, 1, 1, 0},
  {_T("degrees"), Calc_degrees, 1, 1, 0},
  {_T("exp"), Calc_exp, 1, 1, 0},
  {_T("fact"), Calc_fact, 1, 1, 0},
  {_T("inverse"), Calc_inverse, 1, 1, 0},
  {_T("ln"), Calc_ln, 1, 1, 0},
  {_T("log"), Calc_log, 1, 2, 0},
  {_T("log10"), Calc_log10, 1, 1, 0},
  {_T("median"), Calc_median, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("mod"), Calc_mod, 2, 2, 0},
  {_T("pi"), Calc_pi, 0, 0, 0},
  {_T("power"), Calc_power, 2, 2, 0},
  {_T("radians"), Calc_radians, 1, 1, 0},
  {_T("rank"), Calc_rank, 2, 3, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("sin"), Calc_sin, 1, 1, 0},
  {_T("sinh"), Calc_sinh, 1, 1, 0},
  {_T("sqrt"), Calc_sqrt, 1, 1, 0},
  {_T("square"), Calc_square, 1, 1, 0},
  {_T("stdev"), Calc_stdev, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("tan"), Calc_tan, 1, 1, 0},
  {_T("tanh"), Calc_tanh, 1, 1, 0},
  {_T("xroot"), Calc_xroot, 2, 2, 0},
  {_T("combin"), Calc_combin, 2, 2, 0},
  {_T("ceiling"), Calc_ceiling, 2, 2, 0},
  {_T("floor"), Calc_floor, 2, 2, 0},
  {_T("product"), Calc_product, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("even"), Calc_even, 1, 1, 0},
  {_T("odd"), Calc_odd, 1, 1, 0},
  {_T("int"), Calc_int, 1, 1, 0},
  {_T("rand"), Calc_rand, 0, 0, 0},
  {_T("mode"), Calc_mode, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("stdevp"), Calc_stdevp, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("var"), Calc_var, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("varp"), Calc_varp, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("sln"), Calc_sln, 3, 3, 0},
  {_T("syd"), Calc_syd, 4, 4, 0},
  {_T("sumsq"), Calc_sumsq, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("fv"), Calc_fv, 3, 5, 0},
  {_T("pv"), Calc_pv, 3, 5, 0},
  {_T("nper"), Calc_nper, 3, 5, 0},
  {_T("db"), Calc_db, 4, 5, 0},
  {_T("ddb"), Calc_ddb, 4, 5, 0},
  {_T("true"), Calc_true, 0, 0, 0},
  {_T("false"), Calc_false, 0, 0, 0},
  {_T("sign"), Calc_sign, 1, 1, 0},
  {_T("permut"), Calc_permut, 2, 2, 0},
  {_T("iseven"), Calc_iseven, 1, 1, 0},
  {_T("isodd"), Calc_isodd, 1, 1, 0},
  {_T("isnumber"), Calc_isnumber, 1, 1, 0},
  {_T("istext"), Calc_istext, 1, 1, 0},
  {_T("isnontext"), Calc_isnontext, 1, 1, 0},
  {_T("isref"), Calc_isref, 1, 1, CALC_WANTRANGEREF | CALC_WANTCELLREF | CALC_WANTDDEREF},
  {_T("url"),  Calc_url, 1, 2, 0 },
#ifdef SS_V35
  {_T("acosh"), Calc_acosh, 1, 1, 0 },
  {_T("and"), Calc_and, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF },
  {_T("asinh"), Calc_asinh, 1, 1, 0 },
  {_T("atanh"), Calc_atanh, 1, 1, 0 },
  {_T("char"), Calc_char, 1, 1, 0 },
  {_T("clean"), Calc_clean, 1, 1, 0 },
  {_T("code"), Calc_code, 1, 1, 0 },
  {_T("concatenate"), Calc_concatenate, 1, CALC_MAX_PARAMS, 0 },
  {_T("exact"), Calc_exact, 2, 2, 0 },
  {_T("find"), Calc_find, 2, 3, 0 },
  {_T("isblank"), Calc_isblank, 1, 1, 0},
//  {_T("islogical"), Calc_islogical, 1, 1, 0},
  {_T("date"), Calc_date, 3, 3, 0 },
  {_T("day"), Calc_day, 1, 1, 0 },
  {_T("hour"), Calc_hour, 1, 1, 0 },
  {_T("left"), Calc_left, 1, 2, 0 },
  {_T("len"), Calc_len, 1, 1, 0 },
  {_T("lower"), Calc_lower, 1, 1, 0 },
  {_T("mid"), Calc_mid, 3, 3, 0 },
  {_T("minute"), Calc_minute, 1, 1, 0 },
  {_T("month"), Calc_month, 1, 1, 0 },
  {_T("now"), Calc_now, 0, 0, 0 },
  {_T("or"), Calc_or, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF },
  {_T("proper"), Calc_proper, 1, 1, 0 },
  {_T("replace"), Calc_replace, 4, 4, 0 },
  {_T("rept"), Calc_rept, 2, 2, 0 },
  {_T("right"), Calc_right, 1, 2, 0 },
  {_T("rounddown"), Calc_rounddown, 2, 2, 0 },
//  {_T("search"), Calc_search, 2, 3, 0 },
  {_T("second"), Calc_second, 1, 1, 0 },
  {_T("substitute"), Calc_substitute, 3, 4, 0 },
  {_T("time"), Calc_time, 3, 3, 0 },
  {_T("today"), Calc_today, 0, 0, 0 },
  {_T("trim"), Calc_trim, 1, 1, 0 },
  {_T("trunc"), Calc_trunc, 1, 2, 0 },
  {_T("upper"), Calc_upper, 1, 1, 0 },
  {_T("weekday"), Calc_weekday, 1, 2, 0 },
  {_T("year"), Calc_year, 1, 1, 0 },
#endif
  #endif
#ifdef SS_V80
  {_T("avedev"), Calc_avedev, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("binomdist"), Calc_binomdist, 4, 4, 0},
  {_T("critbinom"), Calc_critbinom, 3, 3, 0},
  {_T("negbinomdist"), Calc_negbinomdist, 3, 3, 0},
  {_T("sqrtpi"), Calc_sqrtpi, 1, 1, 0},
  {_T("quotient"), Calc_quotient, 2, 2, 0 },
  {_T("devsq"), Calc_devsq, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("npv"), Calc_npv, 2, CALC_MAX_PARAMS, CALC_WANTRANGEREF},
  {_T("ipmt"), Calc_ipmt, 4, 6, 0},
  {_T("ppmt"), Calc_ppmt, 4, 6, 0},
  {_T("vdb"), Calc_vdb, 5, 7, 0},
  {_T("ispmt"), Calc_ispmt, 4, 4, 0},
  {_T("fvschedule"), Calc_fvschedule, 2, 2, CALC_WANTRANGEREF},
  {_T("effect"), Calc_effect, 2, 2, 0},
  {_T("gcd"), Calc_gcd, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF},
  {_T("lcm"), Calc_lcm, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF},
  {_T("nominal"), Calc_nominal, 2, 2, 0},
  {_T("count"), Calc_count, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("counta"), Calc_counta, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("subtotal"), Calc_subtotal, 2, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("sumproduct"), Calc_sumproduct, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("seriessum"), Calc_seriessum, 4, 4, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("multinomial"), Calc_multinomial, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("betadist"), Calc_betadist, 3, 5, 0},
  {_T("fdist"), Calc_fdist, 3, 3, 0},
  {_T("normsdist"), Calc_normsdist, 1, 1, 0},
  {_T("normdist"), Calc_normdist, 4, 4, 0},
  {_T("normsinv"), Calc_normsinv, 1, 1, 0},
  {_T("norminv"), Calc_norminv, 3, 3, 0},
  {_T("tdist"), Calc_tdist, 3, 3, 0},
  {_T("tinv"), Calc_tinv, 2, 2, 0},
  {_T("skew"), Calc_skew, 1, CALC_MAX_PARAMS, CALC_WANTRANGEREF | CALC_WANTDDEREF},
  {_T("poisson"), Calc_poisson, 3, 3, 0},
  {_T("weibull"), Calc_weibull, 4, 4, 0},
  {_T("gammaln"), Calc_gammaln, 1, 1, 0},
  {_T("gammadist"), Calc_gammadist, 4, 4, 0},
  {_T("gammainv"), Calc_gammainv, 3, 3, 0},
  {_T("factdouble"), Calc_factdouble, 1, 1, 0},
  {_T("fisher"), Calc_fisher, 1, 1, 0},
  {_T("fisherinv"), Calc_fisherinv, 1, 1, 0},
  {_T("lognormdist"), Calc_lognormdist, 3, 3, 0},
  {_T("loginv"), Calc_loginv, 3, 3, 0},
  {_T("betainv"), Calc_betainv, 3, 5, 0},
  {_T("finv"), Calc_finv, 3, 3, 0},
  {_T("standardize"), Calc_standardize, 3, 3, 0},
  {_T("expondist"), Calc_expondist, 3, 3, 0},
  {_T("confidence"), Calc_confidence, 3, 3, 0},
  {_T("hypgeomdist"), Calc_hypgeomdist, 4, 4, 0},
  {_T("chidist"), Calc_chidist, 2, 2, 0},
  {_T("chiinv"), Calc_chiinv, 2, 2, 0},
  {_T("days360"), Calc_days360, 2, 3, 0},
  {_T("accrint"), Calc_accrint, 6, 7, 0},
  {_T("accrintm"), Calc_accrintm, 3, 5, 0},
  {_T("yearfrac"), Calc_yearfrac, 2, 3, 0},
  {_T("disc"), Calc_disc, 4, 5, 0},
  {_T("intrate"), Calc_intrate, 4, 5, 0},
  {_T("received"), Calc_received, 4, 5, 0},
  {_T("tbilleq"), Calc_tbilleq, 3, 3, 0},
  {_T("tbillprice"), Calc_tbillprice, 3, 3, 0},
  {_T("tbillyield"), Calc_tbillyield, 3, 3, 0},
  {_T("price"), Calc_price, 6, 7, 0},
  {_T("pricedisc"), Calc_pricedisc, 4, 5, 0},
  {_T("pricemat"), Calc_pricemat, 5, 6, 0},
  {_T("cumipmt"), Calc_cumipmt, 6, 6, 0},
  {_T("cumprinc"), Calc_cumprinc, 6, 6, 0},
  {_T("coupdaybs"), Calc_coupdaybs, 3, 4, 0},
  {_T("coupdays"), Calc_coupdays, 3, 4, 0},
  {_T("coupdaysnc"), Calc_coupdaysnc, 3, 4, 0},
  {_T("coupncd"), Calc_coupncd, 3, 4, 0},
  {_T("coupnum"), Calc_coupnum, 3, 4, 0},
  {_T("couppcd"), Calc_couppcd, 3, 4, 0},
  {_T("irr"), Calc_irr, 1, 2, CALC_WANTRANGEREF },
  {_T("mirr"), Calc_mirr, 3, 3, CALC_WANTRANGEREF },
  {_T("countif"), Calc_countif, 2, 2, CALC_WANTRANGEREF },
  {_T("sumif"), Calc_sumif, 2, 3, CALC_WANTRANGEREF },
  {_T("pmtxl"), Calc_pmtxl, 3, 5, 0},
#endif
};

const int sizeFuncTable = sizeof(FuncTable) / sizeof(FuncTable[0]);

//--------------------------------------------------------------------
//
//  The FuncLookup() function determines if the given name is a
//  built-in function.  If so, the function returns the Id code of
//  the builtin fucntion.  Otherwise, the function return zero.

int FuncLookup(LPCTSTR lpszName)
{
  int i;
  
  for( i = 0; i < sizeFuncTable; i++ )
    if( lstrcmpi(lpszName, FuncTable[i].lpszText) == 0 )
      return i;
  return 0;
}

//--------------------------------------------------------------------
//
//  The FuncFirst() function retrieves the first built-in function.
//

int FuncFirst(void)
{
  return 1;
}

//--------------------------------------------------------------------
//
//  The FuncNext() function retrieves the next built-in function.
//

int FuncNext(LPCTSTR lpszName)
{
  int i;

  for( i = 0; i < sizeFuncTable; i++ )
    if( lstrcmpi(lpszName, FuncTable[i].lpszText) < 0 )
      return i;;
  return 0;
}

//--------------------------------------------------------------------
//
//  The FuncGetProc() function retrieves the routine used to evaluate
//  the built-in function.
//

CALC_EVALPROC FuncGetProc(int nId)
{
  if( 0 < nId && nId < sizeFuncTable )
    return FuncTable[nId].lpfnProc;
  else
    return NULL;
}

//--------------------------------------------------------------------
//
//  The FuncGetText() function retrieves the text representation of
//  the built-in function.
//

int FuncGetText(int nId, LPTSTR lpszText, int nLen)
{
  int nFuncLen;
  int iResult = 0;
  
  if( 0 < nId && nId < sizeFuncTable )
  { 
    nFuncLen = lstrlen(FuncTable[nId].lpszText);
    if( nFuncLen < nLen )
    {
      lstrcpy(lpszText, FuncTable[nId].lpszText);
      iResult = nFuncLen;
    }
  }
  if( 0 == iResult && nLen > 0 )
    lstrcpy(lpszText, _T(""));
  return iResult;
}

//--------------------------------------------------------------------
//
//  The FuncGetTextLen() function retrieves the length of the text
//  representation of the built-in function.
//

int FuncGetTextLen(int nId)
{
  int iResult = 0;
  
  if( 0 < nId && nId < sizeFuncTable )
    iResult = lstrlen(FuncTable[nId].lpszText);
  return iResult;
}

//--------------------------------------------------------------------
//
//  The FuncGetMinArgs() function determines the minimum number of
//  parameters that the built-in function will accept.
//

short FuncGetMinArgs(int nId)
{
  if( 0 < nId && nId < sizeFuncTable )
    return FuncTable[nId].nMinArgs;
  else
    return 0;
}

//--------------------------------------------------------------------
//
//  The FuncGetMaxArgs() function determines the maximum number of
//  parameters that the built-in function will accept.
//

short FuncGetMaxArgs(int nId)
{
  if( 0 < nId && nId < sizeFuncTable )
    return FuncTable[nId].nMaxArgs;
  else
    return 0;
}

//--------------------------------------------------------------------
//
//  The FuncGetFlags() function returns flags which determine if the
//  built-in function will accept cell and/or range references.
//

long FuncGetFlags(int nId)
{
  if( 0 < nId && nId < sizeFuncTable )
    return FuncTable[nId].lFlags;
  else
    return FALSE;
}

//--------------------------------------------------------------------

BOOL CheckAnyError(LPCALC_VALUE lpArg, short nArgs)
{
  for( ; nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------

BOOL CheckAnyEmpty(LPCALC_VALUE lpArg, short nArgs)
{
  for( ; nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_EMPTY == lpArg->Status )
      return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------

BOOL CheckAllNum(LPCALC_VALUE lpArg, short nArgs, BOOL FAR* pbAnyDouble)
{
  *pbAnyDouble = FALSE;
  for( ; nArgs > 0; nArgs--, lpArg++ )
  {
    switch( lpArg->Status )
    {
      case CALC_VALUE_STATUS_OK:
        if( lpArg->Type == CALC_VALUE_TYPE_DOUBLE )
          *pbAnyDouble = TRUE;
        else if( lpArg->Type != CALC_VALUE_TYPE_LONG )
          return FALSE;
        break;
      default:
        return FALSE;
    }
  }
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CheckAllBoolOrEmpty(LPCALC_VALUE lpArg, short nArgs, BOOL FAR* pbAnyBool, BOOL FAR* pbAnyEmpty)
{
  *pbAnyBool = FALSE;
  *pbAnyEmpty = FALSE;
  for( ; nArgs > 0; nArgs--, lpArg++ )
  {
    switch( lpArg->Status )
    {
      case CALC_VALUE_STATUS_OK:
        if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
          *pbAnyBool = TRUE;
        else if( CALC_VALUE_TYPE_LONG == lpArg->Type )
          *pbAnyBool = TRUE;
        else
          return FALSE;
        break;
      case CALC_VALUE_STATUS_EMPTY:
        *pbAnyEmpty = TRUE;
        break;
      default:
        return FALSE;
    }
  }
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CheckAllNumOrEmpty(LPCALC_VALUE lpArg, short nArgs, BOOL FAR* pbAnyDouble, BOOL FAR* pbAnyLong, BOOL FAR* pbAnyEmpty)
{
  *pbAnyDouble = FALSE;
  *pbAnyLong = FALSE;
  *pbAnyEmpty = FALSE;
  for( ; nArgs > 0; nArgs--, lpArg++ )
  {
    switch( lpArg->Status )
    {
      case CALC_VALUE_STATUS_OK:
        if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
          *pbAnyDouble = TRUE;
        else if( CALC_VALUE_TYPE_LONG == lpArg->Type )
          *pbAnyLong = TRUE;
        else
          return FALSE;
        break;
      case CALC_VALUE_STATUS_EMPTY:
        *pbAnyEmpty = TRUE;
        break;
      default:
        return FALSE;
    }
  }
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CheckAllStrOrEmpty(LPCALC_VALUE lpArg, short nArgs, BOOL FAR* pbAnyStr)
{
  *pbAnyStr = FALSE;
  for( ; nArgs > 0; nArgs--, lpArg++ )
  {
    switch( lpArg->Status )
    {
      case CALC_VALUE_STATUS_OK:
        if( CALC_VALUE_TYPE_STR == lpArg->Type  )
          *pbAnyStr = TRUE;
        else
          return FALSE;
        break;
      case CALC_VALUE_STATUS_EMPTY:
        break;
      default:
        return FALSE;
    }
  }
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CheckAllCellOrRange(LPCALC_VALUE lpArg, short nArgs)
{
  int i;
  
  for( i = 0; i < nArgs; i++ )
  {
    switch( lpArg[i].Status )
    {
      case CALC_VALUE_STATUS_OK:
        if( lpArg[i].Type != CALC_VALUE_TYPE_CELL
            && lpArg[i].Type != CALC_VALUE_TYPE_RANGE )
          return FALSE;
        break;
      default:
        return FALSE;
    }
  }
  return TRUE;
}

//--------------------------------------------------------------------

void ValFree(LPCALC_VALUE lpValue)
{
  if( CALC_VALUE_STATUS_OK == lpValue->Status &&
      CALC_VALUE_TYPE_STR == lpValue->Type &&
      lpValue->Val.hValStr )
    GlobalFree(lpValue->Val.hValStr);
}

//--------------------------------------------------------------------

BOOL ValGetBool(LPCALC_VALUE lpValue)
{
  BOOL bRet = FALSE;

  if( CALC_VALUE_STATUS_OK == lpValue->Status )
  {
    if( CALC_VALUE_TYPE_LONG == lpValue->Type )
      bRet = lpValue->Val.ValLong != 0;
    else if( CALC_VALUE_TYPE_DOUBLE == lpValue->Type )
      bRet = lpValue->Val.ValDouble != 0.0;
  }
  return bRet;
}

//--------------------------------------------------------------------

long ValGetLong(LPCALC_VALUE lpValue)
{
  long lRet = 0;

  if( CALC_VALUE_STATUS_OK == lpValue->Status )
  {
    if( CALC_VALUE_TYPE_LONG == lpValue->Type )
      lRet = lpValue->Val.ValLong;
    else if( CALC_VALUE_TYPE_DOUBLE == lpValue->Type )
      lRet = (long)lpValue->Val.ValDouble;
  }
  return lRet;
}

//--------------------------------------------------------------------

double ValGetDouble(LPCALC_VALUE lpValue)
{
  double dfRet = 0.0;

  if( CALC_VALUE_STATUS_OK == lpValue->Status )
  {
    if( lpValue->Type == CALC_VALUE_TYPE_LONG )
      dfRet = (double)lpValue->Val.ValLong;
    else if( lpValue->Type == CALC_VALUE_TYPE_DOUBLE )
      dfRet = lpValue->Val.ValDouble;
  }
  return dfRet;
}

//--------------------------------------------------------------------

HGLOBAL ValGetStringHandle(LPCALC_VALUE lpValue)
{
  HGLOBAL hRet = 0;

  if( CALC_VALUE_STATUS_OK == lpValue->Status )
    if( CALC_VALUE_TYPE_STR == lpValue->Type )
      hRet = lpValue->Val.hValStr;
  return hRet;
}

//--------------------------------------------------------------------

void ValGetCell(LPCALC_VALUE lpValue, LPCALC_SHEET FAR* lplpSheet, LPLONG lplCol, LPLONG lplRow)
{
  if( CALC_VALUE_STATUS_OK == lpValue->Status )
  {
    if( CALC_VALUE_TYPE_CELL == lpValue->Type )
    {
      *lplpSheet = (LPCALC_SHEET)lpValue->Val.ValCell.u.lInfo;
      *lplCol =  lpValue->Val.ValCell.Col;
      *lplRow =  lpValue->Val.ValCell.Row;
    }
    else if( CALC_VALUE_TYPE_RANGE == lpValue->Type )
    {
      *lplpSheet = (LPCALC_SHEET)lpValue->Val.ValRange.u.lInfo;
      *lplCol = lpValue->Val.ValRange.Col1;
      *lplRow = lpValue->Val.ValRange.Row1;
    }
    else
    {
      *lplpSheet = NULL;
      *lplCol = 0;
      *lplRow = 0;
    }
  }
  else
  {
    *lplpSheet = NULL;
    *lplCol = 0;
    *lplRow = 0;
  }
}

//--------------------------------------------------------------------

void ValGetRange(LPCALC_VALUE lpValue, LPCALC_SHEET FAR* lplpSheet, LPLONG lplCol1, LPLONG lplRow1, LPLONG lplCol2, LPLONG lplRow2)
{
  if( CALC_VALUE_STATUS_OK == lpValue->Status )
  {
    if( CALC_VALUE_TYPE_CELL == lpValue->Type )
    {
      *lplpSheet = (LPCALC_SHEET)lpValue->Val.ValCell.u.lInfo;
      *lplCol1 = lpValue->Val.ValCell.Col;
      *lplRow1 = lpValue->Val.ValCell.Row;
      *lplCol2 = lpValue->Val.ValCell.Col;
      *lplRow2 = lpValue->Val.ValCell.Row;
    }
    else if( CALC_VALUE_TYPE_RANGE == lpValue->Type )
    {
      *lplpSheet = (LPCALC_SHEET)lpValue->Val.ValRange.u.lInfo;
      *lplCol1 = lpValue->Val.ValRange.Col1;
      *lplRow1 = lpValue->Val.ValRange.Row1;
      *lplCol2 = lpValue->Val.ValRange.Col2;
      *lplRow2 = lpValue->Val.ValRange.Row2;
    }
    else
    {
      *lplpSheet = NULL;
      *lplCol1 = 0;
      *lplCol2 = 0;
      *lplRow1 = 0;
      *lplRow2 = 0;
    }
  }
  else
  {
    *lplpSheet = NULL;
    *lplCol1 = 0;
    *lplCol2 = 0;
    *lplRow1 = 0;
    *lplRow2 = 0;
  }
}

//--------------------------------------------------------------------

void ValSetError(LPCALC_VALUE lpResult)
{
  lpResult->Status = CALC_VALUE_STATUS_ERROR;
}

//--------------------------------------------------------------------

void ValSetEmpty(LPCALC_VALUE lpResult)
{
  lpResult->Status = CALC_VALUE_STATUS_EMPTY;
}

//--------------------------------------------------------------------

void ValSetValue(LPCALC_VALUE lpResult, LPCALC_VALUE lpValue)
{
  LPTSTR lpszValue;
  LPTSTR lpszResult;
  long lResultLen;

  _fmemcpy(lpResult, lpValue, sizeof(*lpValue));
  if( CALC_VALUE_STATUS_OK == lpValue->Status &&
      CALC_VALUE_TYPE_STR == lpValue->Type &&
      lpValue->Val.hValStr )
  {
    lpszValue = (LPTSTR)GlobalLock(lpValue->Val.hValStr);
    lResultLen = (1 + lstrlen(lpszValue)) * sizeof(TCHAR);
    lpResult->Val.hValStr = GlobalAlloc(GHND, lResultLen);
    if( lpResult->Val.hValStr )
    {
      lpszResult = (LPTSTR)GlobalLock(lpResult->Val.hValStr);
      lstrcpy(lpszResult, lpszValue);
      GlobalUnlock(lpResult->Val.hValStr);
    }
    GlobalUnlock(lpValue->Val.hValStr);
  }
}

//--------------------------------------------------------------------

void ValSetBool(LPCALC_VALUE lpResult, BOOL bValue)
{
  lpResult->Status = CALC_VALUE_STATUS_OK;
  lpResult->Type = CALC_VALUE_TYPE_LONG;
  lpResult->Val.ValLong = bValue ? 1 : 0;
}

//--------------------------------------------------------------------

void ValSetLong(LPCALC_VALUE lpResult, long lValue)
{
  lpResult->Status = CALC_VALUE_STATUS_OK;
  lpResult->Type = CALC_VALUE_TYPE_LONG;
  lpResult->Val.ValLong = lValue;
}

//--------------------------------------------------------------------

void ValSetDouble(LPCALC_VALUE lpResult, double dValue)
{
  if( _finite(dValue) )
  {
    lpResult->Status = CALC_VALUE_STATUS_OK;
    lpResult->Type = CALC_VALUE_TYPE_DOUBLE;
    lpResult->Val.ValDouble = dValue;
  }
  else
  {
    ValSetError(lpResult);
  }
}

//--------------------------------------------------------------------

void ValSetStringHandle(LPCALC_VALUE lpResult, HGLOBAL hValue)
{
  if( hValue )
  {
    lpResult->Status = CALC_VALUE_STATUS_OK;
    lpResult->Type = CALC_VALUE_TYPE_STR;
    lpResult->Val.hValStr = hValue;
  }
  else
    ValSetEmpty(lpResult);
}

//--------------------------------------------------------------------

#if defined(WIN32) && !defined(_fwcstombs)
  #define _fwcstombs wcstombs
#endif
#if defined(WIN32) && !defined(_fwcslen)
  #define _fwcslen wcslen
#endif

#if defined(SS_DDE)
void ValSetStringW(LPCALC_VALUE lpResult, LPWSTR lpszValue)
{
  #ifdef _UNICODE

  ValSetString(lpResult, lpszValue);

  #else

  HGLOBAL hTemp;
  LPTSTR lpszTemp;
  long lTempLen;

  if( lpszValue )
  {
    lTempLen = (1 + _fwcslen(lpszValue)) * sizeof(TCHAR);
    hTemp = GlobalAlloc(GHND, lTempLen);
    if( hTemp && (lpszTemp = (LPTSTR)GlobalLock(hTemp)) )
    {
      _fwcstombs(lpszTemp, lpszValue, lTempLen);
      ValSetStringHandle(lpResult, hTemp);
      GlobalUnlock(hTemp);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);

  #endif
}
#endif

//--------------------------------------------------------------------

void ValSetString(LPCALC_VALUE lpResult, LPTSTR lpszValue)
{
  HGLOBAL hTemp;
  LPTSTR lpszTemp;
  long lTempLen;

  if( lpszValue )
  {  
    lTempLen = (1 + lstrlen(lpszValue)) * sizeof(TCHAR);
    hTemp = GlobalAlloc(GHND, lTempLen);
    if( hTemp && (lpszTemp = (LPTSTR)GlobalLock(hTemp)) )
    {
      lstrcpy(lpszTemp, lpszValue);
      ValSetStringHandle(lpResult, hTemp);
      GlobalUnlock(hTemp);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
}

//--------------------------------------------------------------------

void ValSetCell(LPCALC_VALUE lpResult, LPCALC_SHEET lpSheet, long lCol, long lRow)
{
  lpResult->Status = CALC_VALUE_STATUS_OK;
  lpResult->Type = CALC_VALUE_TYPE_CELL;
  #ifdef SS_V70
  lpResult->Val.ValCell.nSheet = lpSheet->lpfnGetIndex(lpSheet->hSS);
  #endif
  lpResult->Val.ValCell.Col = lCol;
  lpResult->Val.ValCell.Row = lRow;
#if defined(_WIN64) || defined(_IA64)
  lpResult->Val.ValCell.u.lInfo = (LONG_PTR)lpSheet;
#else
  lpResult->Val.ValCell.u.lInfo = (long)lpSheet;
#endif
}

//--------------------------------------------------------------------

void ValSetRange(LPCALC_VALUE lpResult, LPCALC_SHEET lpSheet, long lCol1, long lRow1, long lCol2, long lRow2)
{
  lpResult->Status = CALC_VALUE_STATUS_OK;
  lpResult->Type = CALC_VALUE_TYPE_RANGE;
  #ifdef SS_V70
  lpResult->Val.ValRange.nSheet = lpSheet->lpfnGetIndex(lpSheet->hSS);
  #endif
  lpResult->Val.ValRange.Col1 = lCol1;
  lpResult->Val.ValRange.Row1 = lRow1;
  lpResult->Val.ValRange.Col2 = lCol2;
  lpResult->Val.ValRange.Row2 = lRow2;
#if defined(_WIN64) || defined(_IA64)
  lpResult->Val.ValRange.u.lInfo = (LONG_PTR)lpSheet;
#else
  lpResult->Val.ValRange.u.lInfo = (long)lpSheet;
#endif
}

//--------------------------------------------------------------------

void ValSetDde(LPCALC_VALUE lpResult, CALC_HANDLE hDde, long lCol, long lRow)
{
  lpResult->Status = CALC_VALUE_STATUS_OK;
  lpResult->Type = CALC_VALUE_TYPE_DDE;
  lpResult->Val.ValDde.hDde = hDde;
  lpResult->Val.ValDde.lCol = lCol;
  lpResult->Val.ValDde.lRow = lRow;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_range(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet1, lpSheet2;
  long lCol1, lCol2, lCol3, lCol4;
  long lRow1, lRow2, lRow3, lRow4;
  
  if( CheckAllCellOrRange(lpArg, nArgs) )
  {
    ValGetRange(lpArg, &lpSheet1, &lCol1, &lRow1, &lCol2, &lRow2);
    ValGetRange(lpArg+1, &lpSheet2, &lCol3, &lRow3, &lCol4, &lRow4);
    if( lpSheet1 == lpSheet2)
    {
      lCol1 = min(lCol1, lCol3);
      lRow1 = min(lRow1, lRow3);
      lCol2 = max(lCol2, lCol4);
      lRow2 = max(lRow2, lRow4);
      ValSetRange(lpResult, lpSheet1, lCol1, lRow1, lCol2, lRow2);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------
//
//  Returns 10 raised to the specified power.
//

double Pow10(int n)
{
  static double p10[] = {1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16};
  if (0 <= n && n <= 16)
    return p10[n];
  return pow(10.0, (double)n);
}

//--------------------------------------------------------------------
//
//  Tests the equalness of two numbers with an accuracy of the magnitude
//  of the given values scaled by 2^-48 (4 bits stripped).  This is used
//  by the rounding and truncating methods to iron out small roundoff
//  errors.
//

BOOL ApproxEqual(double x, double y)
{
  if (x == y)
    return TRUE;
  return fabs(x - y) < fabs(x) / (16777216.0 * 16777216.0);
}

//--------------------------------------------------------------------
//
//  Rounds towards negative infinity taking ApproxEqual into account.
//

double ApproxFloor(double x)
{
  double r = floor(x);
  if (ApproxEqual(x, r + 1.0))
    return r + 1.0;
  return r;
}

//--------------------------------------------------------------------
//
//  Rounds towards positive infinity taking ApproxEqual into account.
//

double ApproxCeiling(double x)
{
  double r = ceil(x);
  if (ApproxEqual(x, r - 1.0))
    return r - 1.0;
  return r;
}

//--------------------------------------------------------------------
//
//  Rounds towards nearest neighbor taking ApproxEqual into account.
//  If both neighbors are equidistant then rounds away from zero.
//

double ApproxRound(double x, int digits)
{
  double power = Pow10(abs(digits));
  if (digits < 0)
    x /= power;
  else
    x *= power;
  if (x < 0.0)
    x = ApproxCeiling(x - 0.5);
  else
    x = ApproxFloor(x + 0.5);
  if (digits < 0)
    x *= power;
  else
    x /= power;
  return x;
}

//--------------------------------------------------------------------
//
//  Tests the equalness of two numbers with an accuracy of the magnitude
//  of the given values scaled by 2^-51 (1 bit stripped).  This is used
//  by the rounding and truncating methods to iron out small roundoff
//  errors.
//

BOOL ApproxEqual2(double x, double y)
{
  if (x == y)
    return TRUE;
  return fabs(x - y) < fabs(x) / (16777216.0 * 16777216.0 * 8.0);
}

//--------------------------------------------------------------------
//
//  Rounds towards negative infinity taking ApproxEqual2 into account.
//

double ApproxFloor2(double x)
{
  double r = floor(x);
  if (ApproxEqual2(x, r + 1.0))
    return r + 1.0;
  return r;
}

//--------------------------------------------------------------------
//
//  Rounds towards positive infinity taking ApproxEqual2 into account.
//

double ApproxCeiling2(double x)
{
  double r = ceil(x);
  if (ApproxEqual2(x, r - 1.0))
    return r - 1.0;
  return r;
}

//--------------------------------------------------------------------
//
//  Rounds towards nearest neighbor taking ApproxEqual2 into account.
//  If both neighbors are equidistant then rounds away from zero.
//

double ApproxRound2(double x, int digits)
{
  double power = Pow10(abs(digits));
  if (digits < 0)
    x /= power;
  else
    x *= power;

  // RFW - 11/4/05 - 17533
  if (x > -100000000000000.0 && x < 100000000000000.0)
  {
    if (x < 0.0)
      x = ApproxCeiling2(x - 0.5);
    else
      x = ApproxFloor2(x + 0.5);
  }

  if (digits < 0)
    x *= power;
  else
    x /= power;
  return x;
}

//--------------------------------------------------------------------
//
//  Rounds towards zero taking ApproxEqual into account.
//

double ApproxRoundDown(double x, int digits)
{
  double power = Pow10(abs(digits));
  if (digits < 0)
    x /= power;
  else
    x *= power;
  if (x < 0.0)
    x = ApproxCeiling(x);
  else
    x = ApproxFloor(x);
  if (digits < 0)
    x *= power;
  else
    x /= power;
  return x;
}

//--------------------------------------------------------------------
//
//  Rounds away from zero taking ApproxEqual into account.
//

double ApproxRoundUp(double x, int digits)
{
  double power = Pow10(abs(digits));
  if (digits < 0)
    x /= power;
  else
    x *= power;
  if (x < 0.0)
    x = ApproxFloor(x);
  else
    x = ApproxCeiling(x);
  if (digits < 0)
    x *= power;
  else
    x /= power;
  return x;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_neg(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else if( bAnyDouble )
      ValSetDouble(lpResult, - ValGetDouble(lpArg));
    else
      ValSetLong(lpResult, - ValGetLong(lpArg));
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_mult(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  double dfVal;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else if( bAnyDouble )
      ValSetDouble(lpResult, ValGetDouble(lpArg) * ValGetDouble(lpArg+1));
    else
    {
      dfVal = ValGetDouble(lpArg) * ValGetDouble(lpArg+1);
      if( LONG_MIN - 0.5 <= dfVal && dfVal <= LONG_MAX + 0.5)
        ValSetLong(lpResult, ValGetLong(lpArg) * ValGetLong(lpArg+1));
      else
        ValSetDouble(lpResult, dfVal);
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_div(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  long lArg1, lArg2, lResult;
  double dfArg1, dfArg2;
    
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else if( bAnyDouble )
    {
      dfArg1 = ValGetDouble(lpArg);
      dfArg2 = ValGetDouble(lpArg+1);
      if( dfArg2 == 0.0 )
        ValSetError(lpResult);
      else
        ValSetDouble(lpResult, dfArg1 / dfArg2);
    }
    else
    {
      lArg1 = ValGetLong(lpArg);
      lArg2 = ValGetLong(lpArg+1);
      if( 0 == lArg2 )
        ValSetError(lpResult);
      else
      {
        lResult = lArg1 / lArg2;
        if( lArg1 == lResult * lArg2 )
          ValSetLong(lpResult, lResult);
        else
        {
          dfArg1 = ValGetDouble(lpArg);
          dfArg2 = ValGetDouble(lpArg+1);
          ValSetDouble(lpResult, dfArg1 / dfArg2);
        }
      }
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_add(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  double dfVal;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyDouble )
      ValSetDouble(lpResult, ValGetDouble(lpArg) + ValGetDouble(lpArg+1));
    else if( bAnyLong )
    {
      dfVal = ValGetDouble(lpArg) + ValGetDouble(lpArg+1);
      if( LONG_MIN - 0.5 <= dfVal && dfVal <= LONG_MAX + 0.5)
        ValSetLong(lpResult, ValGetLong(lpArg) + ValGetLong(lpArg+1));
      else
        ValSetDouble(lpResult, dfVal);
    }
    else
      ValSetEmpty(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_sub(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  double dfVal;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyDouble )
      ValSetDouble(lpResult, ValGetDouble(lpArg) - ValGetDouble(lpArg+1));
    else if( bAnyLong )
    {
      dfVal = ValGetDouble(lpArg) - ValGetDouble(lpArg+1);
      if( LONG_MIN - 0.5 <= dfVal && dfVal <= LONG_MAX + 0.5)
        ValSetLong(lpResult, ValGetLong(lpArg) - ValGetLong(lpArg+1));
      else
        ValSetDouble(lpResult, dfVal);
    }
    else
      ValSetEmpty(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_expon(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
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
      double dfArg1 = ValGetDouble(lpArg);
      double dfArg2 = ValGetDouble(lpArg+1);
      double dfResult;
      errno = 0;
      dfResult = pow(dfArg1, dfArg2);
      if( errno != 0 )
      {
        ValSetError(lpResult);
        errno = 0;
      }
      else if( !bAnyDouble && dfArg2 >= 0.0 &&
               LONG_MIN + 0.5 < dfResult && dfResult < LONG_MAX - 0.5 )
      {
        dfResult += dfResult < 0 ? -0.5 : 0.5;
        ValSetLong(lpResult, (long)dfResult);
      }
      else
        ValSetDouble(lpResult, dfResult);
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_concat(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  HGLOBAL hArg1;
  HGLOBAL hArg2;
  HGLOBAL hResult;
  LPTSTR lpszArg1;
  LPTSTR lpszArg2;
  LPTSTR lpszResult;
  BOOL bReturn = FALSE;
  int nLen;

  if( CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr) )
  {
    hArg1 = ValGetStringHandle(lpArg);
    hArg2 = ValGetStringHandle(lpArg+1);
    if( hArg1 && hArg2 )
    {
      lpszArg1 = (LPTSTR)GlobalLock(hArg1);
      lpszArg2 = (LPTSTR)GlobalLock(hArg2);
      if( lpszArg1 && lpszArg2 )
      {
        nLen = (1 + lstrlen(lpszArg1) + lstrlen(lpszArg2)) * sizeof(TCHAR);
        hResult = GlobalAlloc(GHND, nLen);
        if( hResult && (lpszResult = (LPTSTR)GlobalLock(hResult)) )
        {
          lstrcpy(lpszResult, lpszArg1);
          lstrcat(lpszResult, lpszArg2);
          GlobalUnlock(hResult);
        }
      }
      GlobalUnlock(hArg1);
      GlobalUnlock(hArg2);
      ValSetStringHandle(lpResult, hResult);
    }
    else if( hArg1 )
    {
      lpszArg1 = (LPTSTR)GlobalLock(hArg1);
      ValSetString(lpResult, lpszArg1);
      GlobalUnlock(hArg1);
    }
    else if( hArg2 )
    {
      lpszArg2 = (LPTSTR)GlobalLock(hArg2);
      ValSetString(lpResult, lpszArg2);
      GlobalUnlock(hArg2);
    }
    else
      ValSetEmpty(lpResult);
    bReturn = TRUE;
  }
  return bReturn;
}

//--------------------------------------------------------------------

int ValStrCmp(LPCALC_VALUE lpArg1, LPCALC_VALUE lpArg2)
{
  HGLOBAL hArg1;
  HGLOBAL hArg2;
  LPTSTR lpszArg1;
  LPTSTR lpszArg2;
  int iReturn;

  hArg1 = ValGetStringHandle(lpArg1);
  hArg2 = ValGetStringHandle(lpArg2);
  if( hArg1 && hArg2 )
  {
    lpszArg1 = GlobalLock(hArg1);
    lpszArg2 = GlobalLock(hArg2);
    if( lpszArg1 && lpszArg2 )
      iReturn = lstrcmp(lpszArg1, lpszArg2);
    else if( lpszArg1 )
      iReturn = 1;
    else if( lpszArg2 )
      iReturn = -1;
    else
      iReturn = 0;
    GlobalUnlock(hArg1);
    GlobalUnlock(hArg2);
  }
  else if( hArg1 )
    iReturn = 1;
  else if( hArg2 )
    iReturn = -1;
  else
    iReturn = 0;
  return iReturn;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_equal(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  BOOL bAnyStr;
  
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else if( bAnyDouble )
    {
      double x = ValGetDouble(lpArg);
      double y = ValGetDouble(lpArg+1);
      ValSetBool(lpResult, ApproxEqual(x, y));
    }
    else
      ValSetBool(lpResult, ValGetLong(lpArg) == ValGetLong(lpArg+1));
  }
  else if( CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr) )
    ValSetBool(lpResult, ValStrCmp(lpArg, lpArg+1) == 0 );
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_notequal(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  BOOL bAnyStr;
  
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    if( bAnyDouble )
    {
      double x = ValGetDouble(lpArg);
      double y = ValGetDouble(lpArg+1);
      ValSetBool(lpResult, !ApproxEqual(x, y));
    }
    else
      ValSetBool(lpResult, ValGetLong(lpArg) != ValGetLong(lpArg+1));
  }
  else if( CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr) )
    ValSetBool(lpResult, ValStrCmp(lpArg, lpArg+1) != 0 );
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_less(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  BOOL bAnyStr;
    
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else if( bAnyDouble )
    {
      double x = ValGetDouble(lpArg);
      double y = ValGetDouble(lpArg+1);
      ValSetBool(lpResult, x < y && !ApproxEqual(x, y));
    }
    else
      ValSetBool(lpResult, ValGetLong(lpArg) < ValGetLong(lpArg+1));
  }
  else if( CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr) )
    ValSetBool(lpResult, ValStrCmp(lpArg, lpArg+1) < 0 );
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_greater(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  BOOL bAnyStr;
  
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else if( bAnyDouble )
    {
      double x = ValGetDouble(lpArg);
      double y = ValGetDouble(lpArg+1);
      ValSetBool(lpResult, x > y && !ApproxEqual(x, y));
    }
    else
      ValSetBool(lpResult, ValGetLong(lpArg) > ValGetLong(lpArg+1));
  }
  else if( CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr) )
    ValSetBool(lpResult, ValStrCmp(lpArg, lpArg+1) > 0 );
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_lesseq(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  BOOL bAnyStr;
  
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else if( bAnyDouble )
    {
      double x = ValGetDouble(lpArg);
      double y = ValGetDouble(lpArg+1);
      ValSetBool(lpResult, x < y || ApproxEqual(x, y));
    }
    else
      ValSetBool(lpResult, ValGetLong(lpArg) <= ValGetLong(lpArg+1));
  }
  else if( CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr) )
    ValSetBool(lpResult, ValStrCmp(lpArg, lpArg+1) <= 0 );
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_greatereq(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  BOOL bAnyStr;
  
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else if( bAnyDouble )
    {
      double x = ValGetDouble(lpArg);
      double y = ValGetDouble(lpArg+1);
      ValSetBool(lpResult, x > y || ApproxEqual(x, y));
    }
    else
      ValSetBool(lpResult, ValGetLong(lpArg) >= ValGetLong(lpArg+1));
  }
  else if( CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr) )
    ValSetBool(lpResult, ValStrCmp(lpArg, lpArg+1) >= 0 );
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CALLBACK Calc_abs(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg,
                       short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  
  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyDouble )
    {
      double dfValue = ValGetDouble(lpArg);
      if( dfValue < 0 )
        dfValue = -dfValue;
      ValSetDouble(lpResult, dfValue);
    }
    else if( bAnyLong )
    {
      long lValue = ValGetLong(lpArg);
      if( lValue < 0 )
        lValue = -lValue;
      ValSetLong(lpResult, lValue);
    }
    else
      ValSetEmpty(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_acos(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = acos(dfArg);
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
#endif

//--------------------------------------------------------------------
#ifndef SS_V35
BOOL CALLBACK Calc_and(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{ 
  BOOL bAnyBool;
  BOOL bAnyEmpty;

  if( CheckAllBoolOrEmpty(lpArg, nArgs, &bAnyBool, &bAnyEmpty) )
  {
    if( bAnyBool )
    {
      if( bAnyEmpty )  // one parameter is boolean, the other is empty
        ValSetBool(lpResult, ValGetBool(lpArg) || ValGetBool(lpArg+1)); 
      else  // both parameters are boolean
        ValSetBool(lpResult, ValGetBool(lpArg) && ValGetBool(lpArg+1)); 
    }
    else
      ValSetEmpty(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif
//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_asin(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = asin(dfArg);
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
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_atan(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = atan(dfArg);
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
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_atan2(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg1;
  double dfArg2;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg1 = ValGetDouble(lpArg);
    dfArg2 = ValGetDouble(lpArg+1);
    dfResult = atan2(dfArg2, dfArg1);
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
#endif

//--------------------------------------------------------------------

#if SS_V80 // 24477 -scl
BOOL CALLBACK Calc_average_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfSum = 0.0;
  long lSum = 0;
  long lCount = 0;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        dfSum += lpArg->Val.ValLong;
        lSum += lpArg->Val.ValLong;
        lCount += 1;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        dfSum += lpArg->Val.ValDouble;
        lCount += 1;
        bAnyDouble = TRUE;
      }
      else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            if( bIncludeSubtotals || !CalcIsSubtotal(lpSheet, lCol, lRow) )
            {
               lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
               if( CALC_VALUE_STATUS_OK == value.Status )
               {
                 if( CALC_VALUE_TYPE_LONG == value.Type )
                 {
                   dfSum += value.Val.ValLong;
                   lSum += value.Val.ValLong;
                   lCount += 1;
                   bAnyLong = TRUE;
                 }
                 else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
                 {
                   dfSum += value.Val.ValDouble;
                   lCount += 1;
                   bAnyDouble = TRUE;
                 }
               }
               ValFree(&value);
            }
          }
      }
    }
  }
  if( bAnyError )
  {
    ValSetError(lpResult);
    return TRUE;
  }
  else if( bAnyDouble )
    ValSetDouble(lpResult, dfSum / lCount);
  else if( bAnyLong )
  {
    if( lSum == lSum / lCount * lCount )
      ValSetLong(lpResult, lSum / lCount);
    else
      ValSetDouble(lpResult, dfSum / lCount);
  }
  else
    ValSetEmpty(lpResult);
  return TRUE;
}
BOOL CALLBACK Calc_average(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
   return Calc_average_ex(lpResult, lpArg, nArgs, TRUE);
}
#elif SS_V30
BOOL CALLBACK Calc_average(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfSum = 0.0;
  long lSum = 0;
  long lCount = 0;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        dfSum += lpArg->Val.ValLong;
        lSum += lpArg->Val.ValLong;
        lCount += 1;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        dfSum += lpArg->Val.ValDouble;
        lCount += 1;
        bAnyDouble = TRUE;
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
                dfSum += value.Val.ValLong;
                lSum += value.Val.ValLong;
                lCount += 1;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                dfSum += value.Val.ValDouble;
                lCount += 1;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
    }
  }
  if( bAnyError )
  {
    ValSetError(lpResult);
    return TRUE;
  }
  else if( bAnyDouble )
    ValSetDouble(lpResult, dfSum / lCount);
  else if( bAnyLong )
  {
    if( lSum == lSum / lCount * lCount )
      ValSetLong(lpResult, lSum / lCount);
    else
      ValSetDouble(lpResult, dfSum / lCount);
  }
  else
    ValSetEmpty(lpResult);
  return TRUE;
}
#endif


//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_cos(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = cos(dfArg);
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
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_cosh(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = cosh(dfArg);
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
#endif

//--------------------------------------------------------------------

#define PI 3.14159265358979

#ifdef SS_V30
BOOL CALLBACK Calc_degrees(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    dfArg = ValGetDouble(lpArg);
    dfResult = 180.0 * dfArg / PI;
    ValSetDouble(lpResult, dfResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_exp(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = exp(dfArg);
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
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_fact(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  long lArg;
  long lResult;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    lArg = ValGetLong(lpArg);
    if( 0 <= lArg && lArg <= 12 )
    {
      for( lResult = 1; lArg > 1; lArg-- )
        lResult *= lArg;
      ValSetLong(lpResult, lResult);
    }
    else if( 12 < lArg )
    {
      for( dfResult = 1.0; lArg > 1; lArg-- )
        dfResult *= lArg;
      ValSetDouble(lpResult, dfResult);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

//--------------------------------------------------------------------

BOOL CALLBACK Calc_if(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{ 
  BOOL bAnyBool;
  BOOL bAnyEmpty;

  if( CheckAllBoolOrEmpty(lpArg, 1, &bAnyBool, &bAnyEmpty) )
  { 
    if( bAnyEmpty )
      ValSetError(lpResult);
    else if( ValGetBool(lpArg) )
      ValSetValue(lpResult, lpArg+1);
#if SS_V80
    else if( nArgs > 1 )
      ValSetValue(lpResult, lpArg+2);
    else
       ValSetBool(lpResult, FALSE);
#else
    else
      ValSetValue(lpResult, lpArg+2);
#endif
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_inverse(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    dfArg = ValGetDouble(lpArg);
    if (0.0 == dfArg)
      ValSetError(lpResult);
    else
    {
      errno = 0;
      dfResult = 1.0/dfArg;
      if( 0 == errno )
        ValSetDouble(lpResult, dfResult);
      else
        ValSetError(lpResult);
      errno = 0;
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

//--------------------------------------------------------------------

BOOL CALLBACK Calc_isempty(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  if( CheckAnyError(lpArg, nArgs) )
    ValSetError(lpResult);
  else
    ValSetBool(lpResult, CheckAnyEmpty(lpArg, nArgs));
  return TRUE;
}

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_ln(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = log(dfArg);
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
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_log(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg1, dfArg2;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg1 = ValGetDouble(lpArg);
    if( nArgs < 2 )
      dfArg2 = 10.0;
    else
      dfArg2 = ValGetDouble(lpArg+1);
    if (log10(dfArg2) == 0)
    {
      ValSetError(lpResult);
      return TRUE;
    }  
    dfResult = log10(dfArg1) / log10(dfArg2);
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
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_log10(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = log10(dfArg);
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
#endif

//--------------------------------------------------------------------

#if SS_V80 // 24477 -scl
BOOL CALLBACK Calc_max_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfMax;
  long lMax;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        if( !bAnyLong || lpArg->Val.ValLong > lMax )
          lMax = lpArg->Val.ValLong;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        if( !bAnyDouble || lpArg->Val.ValDouble > dfMax )
          dfMax = lpArg->Val.ValDouble;
        bAnyDouble = TRUE;
      }
      else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            if( bIncludeSubtotals || !CalcIsSubtotal(lpSheet, lCol, lRow) )
            {
               lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
               if( CALC_VALUE_STATUS_OK == value.Status )
               {
                 if( CALC_VALUE_TYPE_LONG == value.Type )
                 {
                   if( !bAnyLong || value.Val.ValLong > lMax )
                     lMax = value.Val.ValLong;
                   bAnyLong = TRUE;
                 }
                 else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
                 {
                   if( !bAnyDouble || value.Val.ValDouble > dfMax )
                     dfMax = value.Val.ValDouble;
                   bAnyDouble = TRUE;
                 }
               }
               ValFree(&value);
            }
          }
      }
      #if defined(SS_DDE)
      else if(CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        CALC_HANDLE hDde = lpArg->Val.ValDde.hDde;
        long lMaxCols = DdeMaxCol(hDde);
        long lMaxRows = DdeMaxRow(hDde);
        for( lCol = 0; lCol < lMaxCols; lCol++ )
          for( lRow = 0; lRow < lMaxRows; lRow++ )
          {
            DdeGetValue(hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                if( !bAnyLong || value.Val.ValLong > lMax )
                  lMax = value.Val.ValLong;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                if( !bAnyDouble || value.Val.ValDouble > dfMax )
                  dfMax = value.Val.ValDouble;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #endif // defined(SS_DDE)
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if( bAnyDouble && bAnyLong )
    ValSetDouble(lpResult, lMax > dfMax ? lMax : dfMax);
  else if( bAnyDouble )
    ValSetDouble(lpResult, dfMax);
  else if( bAnyLong )
    ValSetLong(lpResult, lMax);
  else
    ValSetEmpty(lpResult);
  return TRUE;
}
BOOL CALLBACK Calc_max(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
   return Calc_max_ex(lpResult, lpArg, nArgs, TRUE);
}
#else
BOOL CALLBACK Calc_max(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfMax;
  long lMax;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        if( !bAnyLong || lpArg->Val.ValLong > lMax )
          lMax = lpArg->Val.ValLong;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        if( !bAnyDouble || lpArg->Val.ValDouble > dfMax )
          dfMax = lpArg->Val.ValDouble;
        bAnyDouble = TRUE;
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
                if( !bAnyLong || value.Val.ValLong > lMax )
                  lMax = value.Val.ValLong;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                if( !bAnyDouble || value.Val.ValDouble > dfMax )
                  dfMax = value.Val.ValDouble;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #if defined(SS_DDE)
      else if(CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        CALC_HANDLE hDde = lpArg->Val.ValDde.hDde;
        long lMaxCols = DdeMaxCol(hDde);
        long lMaxRows = DdeMaxRow(hDde);
        for( lCol = 0; lCol < lMaxCols; lCol++ )
          for( lRow = 0; lRow < lMaxRows; lRow++ )
          {
            DdeGetValue(hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                if( !bAnyLong || value.Val.ValLong > lMax )
                  lMax = value.Val.ValLong;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                if( !bAnyDouble || value.Val.ValDouble > dfMax )
                  dfMax = value.Val.ValDouble;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #endif // defined(SS_DDE)
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if( bAnyDouble && bAnyLong )
    ValSetDouble(lpResult, lMax > dfMax ? lMax : dfMax);
  else if( bAnyDouble )
    ValSetDouble(lpResult, dfMax);
  else if( bAnyLong )
    ValSetLong(lpResult, lMax);
  else
    ValSetEmpty(lpResult);
  return TRUE;
}
#endif // !SS_V80

//--------------------------------------------------------------------

#ifdef SS_V30
int FAR PASCAL CompareDouble(HPVOID lpElem1, HPVOID lpElem2, long lUserData)
{
  if( *(double FAR*)lpElem1 < *(double FAR*)lpElem2 )
    return 1;
  else if( *(double FAR*)lpElem1 > *(double FAR*)lpElem2 )
    return -1;
  else
    return 0; 
}
#endif

#ifdef SS_V80
BOOL CALLBACK Calc_median_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyError = FALSE;
  CALC_HANDLE hElem = NULL;
  double FAR* lpElem = NULL;
  long lElemCnt = 0;
  long lAllocCnt = 0;
  long lAllocInc = 10;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( lElemCnt >= lAllocCnt )
    {
      if( hElem )
        CalcMemUnlock(hElem);
      if( hElem )
        hElem = CalcMemReAlloc(hElem, (lAllocCnt + lAllocInc) * sizeof(double));
      else
        hElem = CalcMemAlloc(lAllocInc * sizeof(double));
      if( hElem && (lpElem = (double FAR*)CalcMemLock(hElem)) )
        lAllocCnt += lAllocInc;
      else
      {
        bAnyError = TRUE;
        break;
      }
    }
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        lpElem[lElemCnt++] = lpArg->Val.ValLong;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        lpElem[lElemCnt++] = lpArg->Val.ValDouble;
      }
      else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; !bAnyError && lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; !bAnyError && lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            if( bIncludeSubtotals || !CalcIsSubtotal(lpSheet, lCol, lRow) )
            {
               if( lElemCnt >= lAllocCnt )
               {
                 if( hElem )
                   CalcMemUnlock(hElem);
                 if( hElem )
                   hElem = CalcMemReAlloc(hElem, (lAllocCnt + lAllocInc) * sizeof(double));
                 else
                   hElem = CalcMemAlloc(lAllocInc * sizeof(double));
                 if( hElem && (lpElem = (double FAR*)CalcMemLock(hElem)) )
                   lAllocCnt += lAllocInc;
                 else
                 {
                   bAnyError = TRUE;
                   break;
                 }
               }
               lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
               if( CALC_VALUE_STATUS_OK == value.Status )
               {
                 if( CALC_VALUE_TYPE_LONG == value.Type )
                 {
                   lpElem[lElemCnt++] = value.Val.ValLong;
                 }
                 else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
                 {
                   lpElem[lElemCnt++] = value.Val.ValDouble;
                 }
               }
               ValFree(&value);
            }
          }
      }
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if( lElemCnt > 0 )
  {
    QuickSortHuge((LPBYTE)lpElem, lElemCnt, sizeof(double), 0, CompareDouble);
    if( 0 == lElemCnt % 2 )
      ValSetDouble(lpResult, (lpElem[lElemCnt/2-1] + lpElem[lElemCnt/2]) / 2.0);
    else
      ValSetDouble(lpResult, lpElem[lElemCnt/2]);
  }
  else
    ValSetEmpty(lpResult);
  if( hElem )
  {
    CalcMemUnlock(hElem);
    CalcMemFree(hElem);
  }
  return TRUE;
}
BOOL CALLBACK Calc_median(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
   return Calc_median_ex(lpResult, lpArg, nArgs, TRUE);
}
#elif SS_V30
BOOL CALLBACK Calc_median(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyError = FALSE;
  CALC_HANDLE hElem = NULL;
  double FAR* lpElem = NULL;
  long lElemCnt = 0;
  long lAllocCnt = 0;
  long lAllocInc = 10;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( lElemCnt >= lAllocCnt )
    {
      if( hElem )
        CalcMemUnlock(hElem);
      if( hElem )
        hElem = CalcMemReAlloc(hElem, (lAllocCnt + lAllocInc) * sizeof(double));
      else
        hElem = CalcMemAlloc(lAllocInc * sizeof(double));
      if( hElem && (lpElem = (double FAR*)CalcMemLock(hElem)) )
        lAllocCnt += lAllocInc;
      else
      {
        bAnyError = TRUE;
        break;
      }
    }
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        lpElem[lElemCnt++] = lpArg->Val.ValLong;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        lpElem[lElemCnt++] = lpArg->Val.ValDouble;
      }
      else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; !bAnyError && lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; !bAnyError && lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            if( lElemCnt >= lAllocCnt )
            {
              if( hElem )
                CalcMemUnlock(hElem);
              if( hElem )
                hElem = CalcMemReAlloc(hElem, (lAllocCnt + lAllocInc) * sizeof(double));
              else
                hElem = CalcMemAlloc(lAllocInc * sizeof(double));
              if( hElem && (lpElem = (double FAR*)CalcMemLock(hElem)) )
                lAllocCnt += lAllocInc;
              else
              {
                bAnyError = TRUE;
                break;
              }
            }
            lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                lpElem[lElemCnt++] = value.Val.ValLong;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                lpElem[lElemCnt++] = value.Val.ValDouble;
              }
            }
            ValFree(&value);
          }
      }
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if( lElemCnt > 0 )
  {
    QuickSortHuge((LPBYTE)lpElem, lElemCnt, sizeof(double), 0, CompareDouble);
    if( 0 == lElemCnt % 2 )
      ValSetDouble(lpResult, (lpElem[lElemCnt/2-1] + lpElem[lElemCnt/2]) / 2.0);
    else
      ValSetDouble(lpResult, lpElem[lElemCnt/2]);
  }
  else
    ValSetEmpty(lpResult);
  if( hElem )
  {
    CalcMemUnlock(hElem);
    CalcMemFree(hElem);
  }
  return TRUE;
}
#endif // !SS_V80

//--------------------------------------------------------------------

#if SS_V80 // 24477 -scl
BOOL CALLBACK Calc_min_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfMin;
  long lMin;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        if( !bAnyLong || lpArg->Val.ValLong < lMin )
          lMin = lpArg->Val.ValLong;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        if( !bAnyDouble || lpArg->Val.ValDouble < dfMin )
          dfMin = lpArg->Val.ValDouble;
        bAnyDouble = TRUE;
      }
      else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status)
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                if( !bAnyLong || value.Val.ValLong < lMin )
                  lMin = value.Val.ValLong;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                if( !bAnyDouble || value.Val.ValDouble < dfMin )
                  dfMin = value.Val.ValDouble;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        CALC_HANDLE hDde = lpArg->Val.ValDde.hDde;
        long lMaxCol = DdeMaxCol(hDde);
        long lMaxRow = DdeMaxRow(hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(hDde, lCol, lRow, &value);
            if( CALC_VALUE_TYPE_LONG == value.Type )
            {
              if( !bAnyLong || value.Val.ValLong < lMin )
                lMin = value.Val.ValLong;
              bAnyLong = TRUE;
            }
            else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
            {
              if( !bAnyDouble || value.Val.ValDouble < dfMin )
                dfMin = value.Val.ValDouble;
              bAnyDouble = TRUE;
            }
            ValFree(&value);
          }
      }
      #endif
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if( bAnyDouble && bAnyLong)
    ValSetDouble(lpResult, lMin < dfMin ? lMin : dfMin);
  else if( bAnyDouble )
    ValSetDouble(lpResult, dfMin);
  else if( bAnyLong )
    ValSetLong(lpResult, lMin);
  else
    ValSetEmpty(lpResult);
  return TRUE;
}
BOOL CALLBACK Calc_min(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
   return Calc_min_ex(lpResult, lpArg, nArgs, TRUE);
}
#else // !SS_V80
BOOL CALLBACK Calc_min(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfMin;
  long lMin;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        if( !bAnyLong || lpArg->Val.ValLong < lMin )
          lMin = lpArg->Val.ValLong;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        if( !bAnyDouble || lpArg->Val.ValDouble < dfMin )
          dfMin = lpArg->Val.ValDouble;
        bAnyDouble = TRUE;
      }
      else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status)
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                if( !bAnyLong || value.Val.ValLong < lMin )
                  lMin = value.Val.ValLong;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                if( !bAnyDouble || value.Val.ValDouble < dfMin )
                  dfMin = value.Val.ValDouble;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        CALC_HANDLE hDde = lpArg->Val.ValDde.hDde;
        long lMaxCol = DdeMaxCol(hDde);
        long lMaxRow = DdeMaxRow(hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(hDde, lCol, lRow, &value);
            if( CALC_VALUE_TYPE_LONG == value.Type )
            {
              if( !bAnyLong || value.Val.ValLong < lMin )
                lMin = value.Val.ValLong;
              bAnyLong = TRUE;
            }
            else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
            {
              if( !bAnyDouble || value.Val.ValDouble < dfMin )
                dfMin = value.Val.ValDouble;
              bAnyDouble = TRUE;
            }
            ValFree(&value);
          }
      }
      #endif
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if( bAnyDouble && bAnyLong)
    ValSetDouble(lpResult, lMin < dfMin ? lMin : dfMin);
  else if( bAnyDouble )
    ValSetDouble(lpResult, dfMin);
  else if( bAnyLong )
    ValSetLong(lpResult, lMin);
  else
    ValSetEmpty(lpResult);
  return TRUE;
}
#endif // !SS_V80

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_mod(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;

  if ( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else
    {
      double dfNumber = ValGetDouble(lpArg);
      double dfDivisor = ValGetDouble(lpArg+1);
      if( dfDivisor == 0.0 )
        ValSetError(lpResult);
      else
        ValSetDouble(lpResult, dfNumber - dfDivisor * ApproxFloor(dfNumber / dfDivisor));
    }
  }
  return TRUE;
}
#endif

//--------------------------------------------------------------------

BOOL CALLBACK Calc_not(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{ 
  BOOL bAnyBool;
  BOOL bAnyEmpty;

  if( CheckAllBoolOrEmpty(lpArg, nArgs, &bAnyBool, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else
      ValSetBool(lpResult, !ValGetBool(lpArg)); 
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------
#ifndef SS_V35
BOOL CALLBACK Calc_or(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{ 
  BOOL bAnyBool;
  BOOL bAnyEmpty;

  if( CheckAllBoolOrEmpty(lpArg, nArgs, &bAnyBool, &bAnyEmpty) )
  {
    if( bAnyBool )
      ValSetBool(lpResult, ValGetBool(lpArg) || ValGetBool(lpArg+1)); 
    else
      ValSetEmpty(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif
//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_pi(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    dfResult = PI;
    ValSetDouble(lpResult, dfResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

//--------------------------------------------------------------------
                    
BOOL CALLBACK Calc_pmt(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
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
      CALC_VALUE val[2];
      double dAmt = ValGetDouble(lpArg);
      double dInterest = ValGetDouble(lpArg+1);
      double dTerm = ValGetDouble(lpArg+2);
      double dTermsPerYear = ValGetDouble(lpArg+3);
      double dIr;
      double dPmt;
      if( dAmt < 0.0 || dInterest < 0.0 || dTerm <= 0.0 ||
          dTermsPerYear <= 0.0 )
        ValSetError(lpResult);
      else
      {
        if( 0.0 == dInterest )
          dPmt = dAmt / dTerm;
        else
        {
          dIr = dInterest / 100.0 / dTermsPerYear;
          dPmt = (dIr / (pow(1.0 + dIr, dTerm) - 1.0) + dIr) * dAmt;
        }
        ValSetDouble(val, dPmt);
        ValSetLong(val+1, 2);
        Calc_roundup(lpResult, val, 2);
      }
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------
#if SS_V80
BOOL CALLBACK Calc_pmtxl(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
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
	 double fv = 0.0;
	 double type = 0.0;

      double dRate = ValGetDouble(lpArg);
      double dNper = ValGetDouble(lpArg+1);
      double dPv = ValGetDouble(lpArg+2);
	  if (nArgs > 3)
		fv = ValGetDouble(lpArg+3);
	  if (nArgs > 4)
		type = ValGetDouble(lpArg+4);

      if (type != 0.0)
      {
        type = 1.0f;
      }
      if (dRate == 0.0)
      {
        if (dNper == 0.0)
	        ValSetError(lpResult);
        ValSetDouble(lpResult, -(dPv + fv) / dNper);
      }
      else
        ValSetDouble(lpResult, -(dPv * pow(1.0 + dRate, dNper) + fv) / ((1.0 + dRate * type) * (pow(1.0 + dRate, dNper) - 1.0) / dRate));
    }

  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif
//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_power(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg1;
  double dfArg2;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg1 = ValGetDouble(lpArg);
    dfArg2 = ValGetDouble(lpArg+1);
    dfResult = pow(dfArg1, dfArg2);
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
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_radians(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    dfArg = ValGetDouble(lpArg);
    dfResult = PI * dfArg / 180.0;
    ValSetDouble(lpResult, dfResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_rank(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bError;
  BOOL bFound = FALSE;
  double dfArg1, dfArg2;
  long lArg3 = 0;
  long lResult = 1;
  long lCol;
  long lRow;

  if( !(bError = CheckAnyError(lpArg, 3)) )
  {
    dfArg1 = ValGetDouble(lpArg);
    if( nArgs < 3 )
      lArg3 = 0;
    else
      lArg3 = ValGetLong(lpArg+2);
    if( CALC_VALUE_STATUS_OK == (lpArg+1)->Status )
    {
      if( CALC_VALUE_TYPE_LONG == (lpArg+1)->Type )
      {
        dfArg2 = ValGetDouble(lpArg+1);
        if (dfArg1 == dfArg2)
        {
          bFound = TRUE;
        }
      }
      else if( CALC_VALUE_TYPE_DOUBLE == (lpArg+1)->Type )
      {
        dfArg2 = ValGetDouble(lpArg+1);
        if (dfArg1 == dfArg2)
        {
          bFound = TRUE;
        }
      }
      else if( CALC_VALUE_TYPE_RANGE == (lpArg+1)->Type )
      {
        lpSheet = (LPCALC_SHEET)(lpArg+1)->Val.ValRange.u.lInfo;
        for( lCol = (lpArg+1)->Val.ValRange.Col1; lCol <= (lpArg+1)->Val.ValRange.Col2; lCol++ )
          for( lRow = (lpArg+1)->Val.ValRange.Row1; lRow <= (lpArg+1)->Val.ValRange.Row2; lRow++ )
          {
            lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                dfArg2 = ValGetDouble(&value);
                if (dfArg1 == dfArg2)
                {
                  bFound = TRUE;
                  continue;
                }
                if (lArg3 && dfArg2 < dfArg1) //1,2,3,4,...
                  lResult++;
                else if (!lArg3 && dfArg2 > dfArg1) //...,4,3,2,1
                  lResult++;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                dfArg2 = ValGetDouble(&value);
                if (dfArg1 == dfArg2)
                {
                  bFound = TRUE;
                  continue;
                }
                if (lArg3 && dfArg2 < dfArg1) //1,2,3,4,...
                  lResult++;
                else if (!lArg3 && dfArg2 > dfArg1) //...,4,3,2,1
                  lResult++;
              }
            }
            ValFree(&value);
          }
      }
    }
  }
  if( bError )
    ValSetError(lpResult);
  else if( bFound )
    ValSetLong(lpResult, lResult);
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

//--------------------------------------------------------------------
                    
BOOL CALLBACK Calc_round(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfValue;
  long lNumDigits;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else
    {
      dfValue = ValGetDouble(lpArg);
      lNumDigits = ValGetLong(lpArg+1);
      ValSetDouble(lpResult, ApproxRound(dfValue, lNumDigits));
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------
                    
BOOL CALLBACK Calc_roundup(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfValue;
  long lNumDigits;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else
    {
      dfValue = ValGetDouble(lpArg);
      lNumDigits = ValGetLong(lpArg+1);
      ValSetDouble(lpResult, ApproxRoundUp(dfValue, lNumDigits));
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_sin(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = sin(dfArg);
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
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_sinh(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = sinh(dfArg);
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
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_square(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = dfArg * dfArg;
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
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_sqrt(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = sqrt(dfArg);
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
#endif

//--------------------------------------------------------------------

#if SS_V80
BOOL variance_info_ex(LPCALC_VALUE lpArg, short nArgs, double FAR *lpdfSum, double FAR *lpdfSum2, double FAR *lpdfCount, BOOL bIncludeSubtotals)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyError = FALSE;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        *lpdfSum += lpArg->Val.ValLong;
        *lpdfSum2 += lpArg->Val.ValLong * lpArg->Val.ValLong;
        (*lpdfCount)++;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        *lpdfSum += lpArg->Val.ValDouble;
        *lpdfSum2 += lpArg->Val.ValDouble * lpArg->Val.ValDouble;
        (*lpdfCount)++;
      }
      else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
        {
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            if( bIncludeSubtotals || !CalcIsSubtotal(lpSheet, lCol, lRow) )
            {
               lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
               if( CALC_VALUE_STATUS_OK == value.Status )
               {
                 if( CALC_VALUE_TYPE_LONG == value.Type )
                 {
                   *lpdfSum += value.Val.ValLong;
                   *lpdfSum2 += value.Val.ValLong * value.Val.ValLong;
                   (*lpdfCount)++;
                 }
                 else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
                 {
                   *lpdfSum += value.Val.ValDouble;
                   *lpdfSum2 += value.Val.ValDouble * value.Val.ValDouble;
                   (*lpdfCount)++;
                 }
               }
               ValFree(&value);
            }
          }
        }
      }
    }
  }
  return bAnyError;
}
BOOL variance_info(LPCALC_VALUE lpArg, short nArgs, double FAR *lpdfSum, double FAR *lpdfSum2, double FAR *lpdfCount)
{
  return variance_info_ex(lpArg, nArgs, lpdfSum, lpdfSum2, lpdfCount, TRUE);
}
#else // !SS_V80
BOOL variance_info(LPCALC_VALUE lpArg, short nArgs, double FAR *lpdfSum, double FAR *lpdfSum2, double FAR *lpdfCount)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyError = FALSE;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        *lpdfSum += lpArg->Val.ValLong;
        *lpdfSum2 += lpArg->Val.ValLong * lpArg->Val.ValLong;
        (*lpdfCount)++;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        *lpdfSum += lpArg->Val.ValDouble;
        *lpdfSum2 += lpArg->Val.ValDouble * lpArg->Val.ValDouble;
        (*lpdfCount)++;
      }
      else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
        {
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                *lpdfSum += value.Val.ValLong;
                *lpdfSum2 += value.Val.ValLong * value.Val.ValLong;
                (*lpdfCount)++;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                *lpdfSum += value.Val.ValDouble;
                *lpdfSum2 += value.Val.ValDouble * value.Val.ValDouble;
                (*lpdfCount)++;
              }
            }
            ValFree(&value);
          }
        }
      }
    }
  }
  return bAnyError;
}
#endif // !SS_V80

#ifdef SS_V30
BOOL CALLBACK Calc_sumsq(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  double dfSum = 0.0;
  double dfSum2 = 0.0;
  double dfCount = 0.0;

  if( variance_info(lpArg, nArgs, &dfSum, &dfSum2, &dfCount) )
    ValSetError(lpResult);
  else
    ValSetDouble(lpResult, dfSum2);
  return TRUE;
}
#endif

#if SS_V80 // 24477 -scl
BOOL CALLBACK Calc_varp_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals)
{
  double dfSum = 0.0;
  double dfSum2 = 0.0;
  double dfCount = 0.0;

  if( variance_info_ex(lpArg, nArgs, &dfSum, &dfSum2, &dfCount, bIncludeSubtotals) )
    ValSetError(lpResult);
  else if( dfCount > 0.0 )
  {
    double varp = (dfCount * dfSum2 - dfSum * dfSum) / (dfCount * dfCount);
    ValSetDouble(lpResult, varp < 0.0 ? 0.0 : varp);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
BOOL CALLBACK Calc_varp(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
   return Calc_varp_ex(lpResult, lpArg, nArgs, TRUE);
}
#elif SS_V30
BOOL CALLBACK Calc_varp(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  double dfSum = 0.0;
  double dfSum2 = 0.0;
  double dfCount = 0.0;

  if( variance_info(lpArg, nArgs, &dfSum, &dfSum2, &dfCount) )
    ValSetError(lpResult);
  else if( dfCount > 0.0 )
  {
    double varp = (dfCount * dfSum2 - dfSum * dfSum) / (dfCount * dfCount);
    ValSetDouble(lpResult, varp < 0.0 ? 0.0 : varp);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

#if SS_V80
BOOL CALLBACK Calc_var_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals)
{
  double dfSum = 0.0;
  double dfSum2 = 0.0;
  double dfCount = 0.0;

  if( variance_info(lpArg, nArgs, &dfSum, &dfSum2, &dfCount) )
    ValSetError(lpResult);
  else if( dfCount > 1.0 )
  {
    double var = (dfCount * dfSum2 - dfSum * dfSum) / (dfCount * (dfCount - 1.0));
    ValSetDouble(lpResult, var < 0.0 ? 0.0 : var);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
BOOL CALLBACK Calc_var(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
   return Calc_var_ex(lpResult, lpArg, nArgs, TRUE);
}
#elif SS_V30
BOOL CALLBACK Calc_var(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  double dfSum = 0.0;
  double dfSum2 = 0.0;
  double dfCount = 0.0;

  if( variance_info(lpArg, nArgs, &dfSum, &dfSum2, &dfCount) )
    ValSetError(lpResult);
  else if( dfCount > 1.0 )
  {
    double var = (dfCount * dfSum2 - dfSum * dfSum) / (dfCount * (dfCount - 1.0));
    ValSetDouble(lpResult, var < 0.0 ? 0.0 : var);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

#if SS_V80 // 24477 -scl
BOOL CALLBACK Calc_stdevp_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals)
{
  double dfSum = 0.0;
  double dfSum2 = 0.0;
  double dfCount = 0.0;

  if( variance_info(lpArg, nArgs, &dfSum, &dfSum2, &dfCount) )
    ValSetError(lpResult);
  else if( dfCount > 0.0 )
  {
    double varp = (dfCount * dfSum2 - dfSum * dfSum) / (dfCount * dfCount);
    ValSetDouble(lpResult, varp < 0.0 ? 0.0 : sqrt(varp));
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
BOOL CALLBACK Calc_stdevp(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
   return Calc_stdevp_ex(lpResult, lpArg, nArgs, TRUE);
}
#elif SS_V30
BOOL CALLBACK Calc_stdevp(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  double dfSum = 0.0;
  double dfSum2 = 0.0;
  double dfCount = 0.0;

  if( variance_info(lpArg, nArgs, &dfSum, &dfSum2, &dfCount) )
    ValSetError(lpResult);
  else if( dfCount > 0.0 )
  {
    double varp = (dfCount * dfSum2 - dfSum * dfSum) / (dfCount * dfCount);
    ValSetDouble(lpResult, varp < 0.0 ? 0.0 : sqrt(varp));
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif


#if SS_V80 // 24477 -scl
BOOL CALLBACK Calc_stdev_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals)
{
  double dfSum = 0.0;
  double dfSum2 = 0.0;
  double dfCount = 0.0;

  if( variance_info_ex(lpArg, nArgs, &dfSum, &dfSum2, &dfCount, bIncludeSubtotals) )
    ValSetError(lpResult);
  else if( dfCount > 1.0 )
  {
    double var = (dfCount * dfSum2 - dfSum * dfSum) / (dfCount * (dfCount - 1.0));
    ValSetDouble(lpResult, var < 0.0 ? 0.0 : sqrt(var));
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
BOOL CALLBACK Calc_stdev(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
   return Calc_stdev_ex(lpResult, lpArg, nArgs, TRUE);
}
#elif SS_V30
BOOL CALLBACK Calc_stdev(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  double dfSum = 0.0;
  double dfSum2 = 0.0;
  double dfCount = 0.0;

  if( variance_info(lpArg, nArgs, &dfSum, &dfSum2, &dfCount) )
    ValSetError(lpResult);
  else if( dfCount > 1.0 )
  {
    double var = (dfCount * dfSum2 - dfSum * dfSum) / (dfCount * (dfCount - 1.0));
    ValSetDouble(lpResult, var < 0.0 ? 0.0 : sqrt(var));
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

//--------------------------------------------------------------------

#if SS_V80 // 24477 -scl
BOOL CALLBACK Calc_sum_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfSum = 0.0;
  long lSum = 0;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        dfSum += lpArg->Val.ValLong;
        lSum += lpArg->Val.ValLong;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        dfSum += lpArg->Val.ValDouble;
        bAnyDouble = TRUE;
      }
      else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
             if( bIncludeSubtotals || !CalcIsSubtotal(lpSheet, lCol, lRow) )
             {
               lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
               if( CALC_VALUE_STATUS_OK == value.Status )
               {
                 if( CALC_VALUE_TYPE_LONG == value.Type )
                 {
                   dfSum += value.Val.ValLong;
                   lSum += value.Val.ValLong;
                   bAnyLong = TRUE;
                 }
                 else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
                 {
                   dfSum += value.Val.ValDouble;
                   bAnyDouble = TRUE;
                 }
               }
               ValFree(&value);
             }
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                dfSum += value.Val.ValLong;
                lSum += value.Val.ValLong;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                dfSum += value.Val.ValDouble;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #endif
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if( bAnyDouble )
    ValSetDouble(lpResult, dfSum);
  else if( bAnyLong )
  {
    if( LONG_MIN - 0.5 <= dfSum && dfSum <= LONG_MAX + 0.5)
      ValSetLong(lpResult, lSum);
    else
      ValSetDouble(lpResult, dfSum);
  }
  else
    ValSetEmpty(lpResult);
  return TRUE;
}
BOOL CALLBACK Calc_sum(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
   return Calc_sum_ex(lpResult, lpArg, nArgs, TRUE);
}
#else
BOOL CALLBACK Calc_sum(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfSum = 0.0;
  long lSum = 0;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        dfSum += lpArg->Val.ValLong;
        lSum += lpArg->Val.ValLong;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        dfSum += lpArg->Val.ValDouble;
        bAnyDouble = TRUE;
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
                dfSum += value.Val.ValLong;
                lSum += value.Val.ValLong;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                dfSum += value.Val.ValDouble;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                dfSum += value.Val.ValLong;
                lSum += value.Val.ValLong;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                dfSum += value.Val.ValDouble;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #endif
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if( bAnyDouble )
    ValSetDouble(lpResult, dfSum);
  else if( bAnyLong )
  {
    if( LONG_MIN - 0.5 <= dfSum && dfSum <= LONG_MAX + 0.5)
      ValSetLong(lpResult, lSum);
    else
      ValSetDouble(lpResult, dfSum);
  }
  else
    ValSetEmpty(lpResult);
  return TRUE;
}
#endif // !SS_V80

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_tan(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = tan(dfArg);
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
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_tanh(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = tanh(dfArg);
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
#endif

//--------------------------------------------------------------------
                    
BOOL CALLBACK Calc_rounddown(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfValue;
  long lNumDigits;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else
    {
      dfValue = ValGetDouble(lpArg);
      lNumDigits = ValGetLong(lpArg+1);
      ValSetDouble(lpResult, ApproxRoundDown(dfValue, lNumDigits));
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CALLBACK Calc_xroot(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg1;
  double dfArg2;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    dfArg1 = ValGetDouble(lpArg);
    dfArg2 = ValGetDouble(lpArg+1);
    if (0.0 == dfArg2)
      ValSetError(lpResult);
    else
    {
      if (!fmod(dfArg2, 2) && (dfArg1 < 0))
      {
        ValSetError(lpResult);
        return TRUE;
      }
      errno = 0;
      dfResult = pow(fabs(dfArg1), (1.0/dfArg2));
      if (dfArg1 < 0)
        dfResult *= -1.0;
      if( 0 == errno )
        ValSetDouble(lpResult, dfResult);
      else
        ValSetError(lpResult);
      errno = 0;
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

#ifdef SS_V30
BOOL CALLBACK Calc_combin(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs) 
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double n;
  double k;
  double dfResult;
  double i;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else
    {
      n = ApproxFloor(ValGetDouble(lpArg));
      k = ApproxFloor(ValGetDouble(lpArg+1));
      if (n < 0.0 || k < 0.0 || n < k || (double)LONG_MAX < n)
        ValSetError(lpResult);
      else
      {
        dfResult = 1.0;
        k = min(n - k, k);
        for (i = 1.0; i <= k; i++)
        {
          dfResult *= n - i + 1.0;
          dfResult /= i;
        }
        if (dfResult <= (double)LONG_MAX)
          ValSetLong(lpResult, (long)dfResult);
        else
          ValSetDouble(lpResult, dfResult); 
      }
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif   

#ifdef SS_V30
BOOL CALLBACK Calc_ceiling(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{    
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg1;
  double dfArg2;
  double dfResult;

  if ( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else 
    {
      dfArg1=ValGetDouble(lpArg);
      dfArg2=ValGetDouble(lpArg+1);
      if (dfArg1 == 0.0 || dfArg2 == 0.0)
        ValSetDouble(lpResult, 0.0);
      else if ((dfArg1 < 0.0 && 0.0 < dfArg2) || (dfArg2 < 0.0 && 0.0 < dfArg1))
        ValSetError(lpResult);
      else
      {
        dfResult = ApproxCeiling(dfArg1 / dfArg2) * dfArg2;
        ValSetDouble(lpResult, dfResult);
      }
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif


#ifdef SS_V30
BOOL CALLBACK Calc_floor(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{ 
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg1;
  double dfArg2;
  double dfResult;

  if ( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else 
    {
      dfArg1=ValGetDouble(lpArg);
      dfArg2=ValGetDouble(lpArg+1);
      if (dfArg1 == 0.0 || dfArg2 == 0.0)
        ValSetDouble(lpResult, 0.0);
      else if ((dfArg1 < 0.0 && 0.0 < dfArg2) || (dfArg2 < 0.0 && 0.0 < dfArg1))
        ValSetError(lpResult);
      else
      {
        dfResult = ApproxFloor(dfArg1 / dfArg2) * dfArg2;
        ValSetDouble(lpResult, dfResult);
      }
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
} 
#endif

#if SS_V80 // 24477 -scl
BOOL CALLBACK Calc_product_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfProduct = 1.0;
  long lProduct = 1;
  long lCol;
  long lRow;

  errno = 0;
  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        dfProduct *= lpArg->Val.ValLong;
        lProduct *= lpArg->Val.ValLong;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        dfProduct *= lpArg->Val.ValDouble;
        bAnyDouble = TRUE;
      }
      else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            if( bIncludeSubtotals || !CalcIsSubtotal(lpSheet, lCol, lRow) )
            {
               lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
               if( CALC_VALUE_STATUS_OK == value.Status )
               {
                 if( CALC_VALUE_TYPE_LONG == value.Type )
                 {
                   dfProduct *= value.Val.ValLong;
                   lProduct *= value.Val.ValLong;
                   bAnyLong = TRUE;
                 }
                 else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
                 {
                   dfProduct *= value.Val.ValDouble;
                   bAnyDouble = TRUE;
                 }
               }
               ValFree(&value);
            }
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                dfProduct *= value.Val.ValLong;
                lProduct *= value.Val.ValLong;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                dfProduct *= value.Val.ValDouble;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #endif
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if( bAnyDouble )
    ValSetDouble(lpResult, dfProduct);
  else if( bAnyLong )
  {
    if( LONG_MIN - 0.5 <= dfProduct && dfProduct <= LONG_MAX + 0.5)
      ValSetLong(lpResult, lProduct);
    else
      ValSetDouble(lpResult, dfProduct);
  }
  else
    ValSetEmpty(lpResult);
  return TRUE;
}
BOOL CALLBACK Calc_product(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
   return Calc_product_ex(lpResult, lpArg, nArgs, TRUE);
}
#elif SS_V30
BOOL CALLBACK Calc_product(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfProduct = 1.0;
  long lProduct = 1;
  long lCol;
  long lRow;

  errno = 0;
  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        dfProduct *= lpArg->Val.ValLong;
        lProduct *= lpArg->Val.ValLong;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        dfProduct *= lpArg->Val.ValDouble;
        bAnyDouble = TRUE;
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
                dfProduct *= value.Val.ValLong;
                lProduct *= value.Val.ValLong;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                dfProduct *= value.Val.ValDouble;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                dfProduct *= value.Val.ValLong;
                lProduct *= value.Val.ValLong;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                dfProduct *= value.Val.ValDouble;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #endif
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if( bAnyDouble )
    ValSetDouble(lpResult, dfProduct);
  else if( bAnyLong )
  {
    if( LONG_MIN - 0.5 <= dfProduct && dfProduct <= LONG_MAX + 0.5)
      ValSetLong(lpResult, lProduct);
    else
      ValSetDouble(lpResult, dfProduct);
  }
  else
    ValSetEmpty(lpResult);
  return TRUE;
}
#endif

#ifdef SS_V30
BOOL CALLBACK Calc_even(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfNumber;

  if ( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else 
    {
      dfNumber = ValGetDouble(lpArg);
      if( dfNumber < 0.0 )
      {
        dfNumber = ApproxFloor(dfNumber);
        if( fmod(dfNumber, 2.0) != 0.0 )
          dfNumber -= 1.0;
      }
      else
      {
        dfNumber = ApproxCeiling(dfNumber);
        if( fmod(dfNumber, 2.0) != 0.0)
          dfNumber += 1.0;
      }
      ValSetDouble(lpResult, dfNumber);
    }
  }
  else
    ValSetError(lpResult);

  return TRUE;
}
#endif
      
#ifdef SS_V30
BOOL CALLBACK Calc_odd(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfNumber;

  if ( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else 
    {
      dfNumber = ValGetDouble(lpArg);
      if( dfNumber < 0.0)
      {
        dfNumber = ApproxFloor(dfNumber);
        if( fmod(dfNumber, 2.0) == 0.0 )
          dfNumber -= 1.0;
      }
      else
      {
        dfNumber = ApproxCeiling(dfNumber);
        if( fmod(dfNumber, 2.0) == 0.0 )
          dfNumber += 1.0;
      }
      ValSetDouble(lpResult, dfNumber);
    }
  }
  else
    ValSetError(lpResult);

  return TRUE;
}
#endif

#ifdef SS_V30
BOOL CALLBACK Calc_int(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg1;
  long lResult;

  if ( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else 
    {
      dfArg1 = ValGetDouble(lpArg);
      lResult = (long)ApproxFloor(dfArg1);
      ValSetLong(lpResult, lResult);
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

double random(double x)
{ 
  SYSTEMTIME sysTime;
  int seed;
  double d = 0.0;

  GetSystemTime(&sysTime);
  seed=sysTime.wHour+sysTime.wMinute+sysTime.wSecond*sysTime.wMilliseconds;
  srand((int)seed);
  d = (double)rand();
  d = (double)d/10000.0;
  d = (double)(d*seed/sysTime.wMilliseconds);
  d = fmod(d, 1);

  return d;
}
 
BOOL CALLBACK Calc_rand(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{ 
 double randnum;
 
 randnum=random(0);
 ValSetDouble(lpResult,randnum);
 return TRUE;
 
 }       
 
#ifdef SS_V30
BOOL CALLBACK Calc_mode(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyError = FALSE;
  CALC_HANDLE hElem = NULL;
  double FAR* lpElem = NULL;
  long lElemCnt = 0;
  long lAllocCnt = 0;
  long lAllocInc = 10;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( lElemCnt >= lAllocCnt )
    {
      if( hElem )
        CalcMemUnlock(hElem);
      if( hElem )
        hElem = CalcMemReAlloc(hElem, (lAllocCnt + lAllocInc) * sizeof(double));
      else
        hElem = CalcMemAlloc(lAllocInc * sizeof(double));
      if( hElem && (lpElem = (double FAR*)CalcMemLock(hElem)) )
        lAllocCnt += lAllocInc;
      else
      {
        bAnyError = TRUE;
        break;
      }
    }
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        lpElem[lElemCnt++] = lpArg->Val.ValLong;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        lpElem[lElemCnt++] = lpArg->Val.ValDouble;
      }
      else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; !bAnyError && lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; !bAnyError && lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            if( lElemCnt >= lAllocCnt )
            {
              if( hElem )
                CalcMemUnlock(hElem);
              if( hElem )
                hElem = CalcMemReAlloc(hElem, (lAllocCnt + lAllocInc) * sizeof(double));
              else
                hElem = CalcMemAlloc(lAllocInc * sizeof(double));
              if( hElem && (lpElem = (double FAR*)CalcMemLock(hElem)) )
                lAllocCnt += lAllocInc;
              else
              {
                bAnyError = TRUE;
                break;
              }
            }
            lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                lpElem[lElemCnt++] = value.Val.ValLong;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                lpElem[lElemCnt++] = value.Val.ValDouble;
              }
            }
            ValFree(&value);
          }
      }
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if( lElemCnt > 0 )
  {
    int    i,j;
    double dfMode = 0;
    double dfElement = 0;
    long   lModeCount = 0;
    BOOL   fMoreThanOne = FALSE;
    long   lCount = 0;

    for (i=0; i<lElemCnt; i++)
    {
      for(j=0; j<lElemCnt; j++)
      {
        if (j == i)
          continue;
        if (lpElem[j] == lpElem[i])
          lCount++;
      }
      if (lCount > 0)
      {
        fMoreThanOne = TRUE;
        if ((lCount == lModeCount) && (lpElem[i] < dfMode))
        {
          dfMode = lpElem[i];
        }
        else if (lCount > lModeCount)
        {
          lModeCount = lCount;
          dfMode = lpElem[i];
        }
      }
      lCount = 0;
    }
    if (!fMoreThanOne)
      ValSetError(lpResult);
    else
      ValSetDouble(lpResult, dfMode);
  }
/*
    for (i=0; i<lElemCnt; i++)
    {
      dfElement = lpElem[i];
      lCount = 1;
      for(j=i+1; j<lElemCnt; j++)
      {
        if (lpElem[j] == dfElement)
          lCount++;
      }
      if (lCount > lModeCount)
      {
        dfMode = dfElement;
        lModeCount = lCount;
        fDupe = FALSE;
      }
      if (lCount == lModeCount)
      {
        fDupe = TRUE;
      }
    }
    if (fDupe)
    {
      ValSetError(lpResult);
    }

    ValSetDouble(lpResult, dfMode);
  }
  else
    ValSetEmpty(lpResult);
*/
  if( hElem )
  {
    CalcMemUnlock(hElem);
    CalcMemFree(hElem);
  }
  return TRUE;
}
#endif

#ifdef SS_V30
BOOL CALLBACK Calc_true(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    dfResult = TRUE;
    ValSetDouble(lpResult, dfResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

#ifdef SS_V30
BOOL CALLBACK Calc_false(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    dfResult = FALSE;
    ValSetDouble(lpResult, dfResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif

#ifdef SS_V30
BOOL CALLBACK Calc_sign(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfResult = 0;
  double dfArg1;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else 
    {
      dfArg1=ValGetDouble(lpArg);
      if (dfArg1 < 0)
        dfResult = -1;
      else if (dfArg1 > 0)
        dfResult = 1;
      ValSetDouble(lpResult, dfResult);
    }
  }
  else
    ValSetError(lpResult);

  return TRUE;
}
#endif

#ifdef SS_V30
BOOL CALLBACK Calc_permut(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double n;
  double k;
  double dfResult;
  double i;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else
    {
      n = ApproxFloor(ValGetDouble(lpArg));
      k = ApproxFloor(ValGetDouble(lpArg+1));
      if (n < 0.0 || k < 0.0 || n < k || (double)LONG_MAX < n)
        ValSetError(lpResult);
      else
      {
        dfResult = 1.0;
        for (i = n - k + 1.0; i <= n; i++)
          dfResult *= i;
        if (dfResult <= (double)LONG_MAX)
          ValSetLong(lpResult, (long)dfResult);
        else
          ValSetDouble(lpResult, dfResult); 
      }
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
#endif 

#ifdef SS_V30

// jb - 1 dec 98 -----------------------------------------------------------------
//
// <A HREF="www.fpoint.com">FarPoint</A>
// <A HREF="www.fpoint.com">www.fpoint.com</A>
//
void urlanchor (LPTSTR lpszResult, LPTSTR lpszValue)
{
	lstrcpy (lpszResult, lpszValue);
	return;
}

// jb - 1 dec 98 --- URL --------------------------------------------------------------
//
// The Anchor tag will use the attribute HREF to enable the user to export a URL with the 
// appropriate HTML tags.  There are 2 ways they may want to do this.  www.fpoint.com OR FarPoint
//
// A1 = www.fpoint.com, A2 = FarPoint
//
// ex: formula for A3, URL (A1).     The result is <A HREF="http://www.fpoint.com">www.fpoint.com</A>
// ex: formula for A4, URL (A1, A2). The result is <A HREF="http://www.fpoint.com>FarPoint</A>
//
// Note: on the nLen, we add 64 to take care of the extra characters needed for HTML info.
// For the URL function that takes only one argument, we also need the size of the arg twice since 
// it will be our default anchor tag.
//
BOOL CALLBACK Calc_url (LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
	BOOL bAnyStr;
	HGLOBAL hArg1;
	HGLOBAL hArg2;
	HGLOBAL hResult;
	LPTSTR lpszArg1;
	LPTSTR lpszArg2;
	LPTSTR lpszResult;
	BOOL bReturn = FALSE;
	int nLen;

	if (CheckAllStrOrEmpty (lpArg, nArgs, &bAnyStr))
	{	hArg1 = ValGetStringHandle (lpArg);
		hArg2 = ValGetStringHandle (lpArg+1);

		// This is URL (A1, A2) 'Farpoint' will point to www.fpoint.com
		//
		if (hArg1 && hArg2)
		{	lpszArg1 = (LPTSTR)GlobalLock (hArg1);
			lpszArg2 = (LPTSTR)GlobalLock (hArg2);

			if (lpszArg1 && lpszArg2)
			{	nLen = (1 + lstrlen (lpszArg2)) * sizeof(TCHAR);
				hResult = GlobalAlloc (GHND, nLen);

				if (hResult && (lpszResult = (LPTSTR)GlobalLock (hResult)))
				{	urlanchor (lpszResult, lpszArg2);
					GlobalUnlock (hResult);
					bReturn = TRUE;
				}
			}

			GlobalUnlock (hArg1);
			GlobalUnlock (hArg2);
			ValSetStringHandle (lpResult, hResult);
		}

		// This is URL (A1)  www.fpoint.com
		//
		else if (hArg1)
		{	lpszArg1 = (LPTSTR)GlobalLock (hArg1);

			if (lpszArg1)
			{	nLen = (1 + lstrlen (lpszArg1) * sizeof(TCHAR));
				hResult = GlobalAlloc (GHND, nLen);

				if (hResult && (lpszResult = (LPTSTR)GlobalLock (hResult)))
				{	urlanchor (lpszResult, lpszArg1);
					GlobalUnlock (hResult);
					bReturn = TRUE;
				}
			}

			GlobalUnlock (hArg1);
			ValSetStringHandle (lpResult, hResult);
		}
	}

	// Need to write something to the log file indicating that we had an error.
	//
	else
	{	ValSetError(lpResult);
	}

	return bReturn;
}

#ifdef SS_V35

#ifdef FP_DLL
// special version of fpConvertTSTRtoType for the DLL control,
// just does VT_DATE.
int fpConvertTSTRtoType( LPTSTR tstr, LPVOID lpDest, USHORT usDestType)
{
  int ret = -1;
  VARIANT v, vDest;

  VariantInit(&v);
  VariantInit(&vDest);
  v.vt = VT_BSTR;
#ifdef _UNICODE
  v.bstrVal = SysAllocString(tstr);
#else
  v.bstrVal = SysAllocStringByteLen(tstr, lstrlen(tstr));
#endif

  if( v.bstrVal )
  {
    if( VT_DATE == usDestType )
    {
      if( SUCCEEDED(VariantChangeType(&vDest, &v, 0, VT_DATE)) )
      {
        *(double *)lpDest = vDest.dblVal;
        ret = sizeof(double);
      }
    }
    SysFreeString(v.bstrVal);
  }
  return ret;
}
#endif

BOOL CALLBACK Calc_acosh(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = log(dfArg + sqrt(dfArg * dfArg - 1.0));
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

BOOL CALLBACK Calc_and(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  BOOL bAnyBool = FALSE;
  BOOL bAnyError = FALSE;
  SS_COORD c, r;
  CALC_VALUE val;
  BOOL bResult;

// this was removed because it conflicts with the fix-up code to '&' operator changing it to use AND() -scl
//#if SS_V80 // make '&' work as concat operator is all args are string or empty -scl
//  BOOL bAnyStr;
//  if( CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr) )
//     return Calc_concatenate(lpResult, lpArg, nArgs);
//#endif
  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++)
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      switch( lpArg->Type )
      {
        case CALC_VALUE_TYPE_DOUBLE:
          if( bAnyBool )
            bResult &= (ValGetDouble(lpArg) ? TRUE : FALSE);
          else
            bResult = (ValGetDouble(lpArg) ? TRUE : FALSE);
          bAnyBool = TRUE;
          break;
        case CALC_VALUE_TYPE_LONG:
          if( bAnyBool )
            bResult &= (ValGetLong(lpArg) ? TRUE : FALSE);
          else
            bResult = (ValGetLong(lpArg) ? TRUE : FALSE);
          bAnyBool = TRUE;
          break;
        case CALC_VALUE_TYPE_RANGE:
          lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
          for( r = lpArg->Val.ValRange.Row1; r <= lpArg->Val.ValRange.Row2; r++ )
          {
            for( c = lpArg->Val.ValRange.Col1; c <= lpArg->Val.ValRange.Col2; c++ )
            {
              lpSheet->lpfnGetData(lpSheet->hSS, c, r, &val);
              if( CALC_VALUE_STATUS_ERROR == val.Status )
                bAnyError = TRUE;
              else if( CALC_VALUE_STATUS_OK == val.Status )
              {
                switch( val.Type )
                {
                  case CALC_VALUE_TYPE_DOUBLE:
                    if( bAnyBool )
                      bResult &= (ValGetDouble(&val) ? TRUE : FALSE);
                    else
                      bResult = (ValGetDouble(&val) ? TRUE : FALSE);
                    bAnyBool = TRUE;
                    break;
                  case CALC_VALUE_TYPE_LONG:
                    if( bAnyBool )
                      bResult &= (ValGetLong(&val) ? TRUE : FALSE);
                    else
                      bResult = (ValGetLong(&val) ? TRUE : FALSE);
                    bAnyBool = TRUE;
                    break;
                }
              }
              ValFree(&val);
            }
          }
          break;
      }
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if( bAnyBool )
    ValSetBool(lpResult, bResult);
  else
    ValSetEmpty(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_asinh(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = log(dfArg + sqrt(dfArg * dfArg + 1.0));
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

BOOL CALLBACK Calc_atanh(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
    dfResult = log((1.0 + dfArg) / (1.0 - dfArg)) / 2.0;
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

BOOL CALLBACK Calc_char(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  long lArg;
  LPTSTR pszResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    lArg = ValGetLong(lpArg);
#if defined(_MBCS) && !defined(UNICODE) && !defined(_UNICODE)
    if( 1 <= lArg && lArg <= 255 )
    {
      HGLOBAL hResult = GlobalAlloc(GHND, 2 * sizeof(TCHAR));
      if( hResult && (pszResult = (LPTSTR)GlobalLock(hResult)) )
        {
          pszResult[0] = (TCHAR)lArg;
          pszResult[1] = 0;
          ValSetString(lpResult, pszResult);
          GlobalUnlock(hResult);
          GlobalFree(hResult);
        }
    }
    else
    {
      char szResult[3];

      szResult[0] = (BYTE)(lArg >> 8);
      szResult[1] = (BYTE)lArg;
      szResult[2] = '\0';
      jis2sjis(&szResult[0], &szResult[1]);

      if (_TIsDBCSLeadByte(szResult[0]))
        ValSetString(lpResult, szResult);
      else
        ValSetError(lpResult);
    }
#else
    if( lArg > 0 )
    {
      HGLOBAL hResult = GlobalAlloc(GHND, 2 * sizeof(TCHAR));
      if( hResult && (pszResult = (LPTSTR)GlobalLock(hResult)) )
      {
        pszResult[0] = (TCHAR)lArg;
        pszResult[1] = 0;
        ValSetString(lpResult, pszResult);
        GlobalUnlock(hResult);
        GlobalFree(hResult);
      }
    }
    else
      ValSetError(lpResult);
#endif
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_clean(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  LPTSTR lpszResult;
  HGLOBAL hResult;
  LPTSTR lpszArg;
  HGLOBAL hArg;
  int  nLen;
  int  nArgLen;
  int  i, j;

  if (CheckAllStrOrEmpty (lpArg, nArgs, &bAnyStr) &&
      (hArg = ValGetStringHandle (lpArg)))

  {
    if (lpszArg = (LPTSTR)GlobalLock (hArg))
    {
      nArgLen = (int)_tcslen(lpszArg);
      for( nLen = 0, i = 0; i < nArgLen; i++ )
      {
        if ((!iscntrl(lpszArg[i]))  ||                      // 8954
            (_istprint(lpszArg[i])) || 
            (isprint(lpszArg[i])))                          // 8954
          nLen++;
      }

      if (hResult = GlobalAlloc(GHND, (nLen+1)*sizeof(TCHAR)))
      {
        if (lpszResult = GlobalLock(hResult))
        {
           for( i = 0, j = 0; i < nArgLen; i++ )
           {
             if ((!iscntrl(lpszArg[i]))  ||                 // 8954
                 (_istprint(lpszArg[i])) ||
                 (isprint(lpszArg[i])))                     // 8954
                lpszResult[j++] = lpszArg[i];
           }
           lpszResult[j] = 0;
           ValSetString(lpResult, lpszResult);
           GlobalUnlock(hResult);
        }
        GlobalFree(hResult);
      }
      GlobalUnlock(hArg);
    }
    else
      ValSetString(lpResult, _T(""));
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_code(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  LPTSTR lpszArg;
  HGLOBAL hArg;


  if (CheckAllStrOrEmpty (lpArg, nArgs, &bAnyStr) &&
      (hArg = ValGetStringHandle (lpArg)))

  {
    if (lpszArg = (LPTSTR)GlobalLock (hArg))
    {
      if (_tcslen(lpszArg))
      {
        unsigned short unResult = (TCHAR)*lpszArg;

#if defined(_MBCS) && !defined(UNICODE) && !defined(_UNICODE)
        if (_TIsDBCSLeadByte(*lpszArg))
        {
//          unResult = (unsigned short)_tcsnextc(lpszArg);
          unsigned char Buffer[2] = {*lpszArg, *(lpszArg+1)};
          sjis2jis( &Buffer[0], &Buffer[1]);

          unResult = (unsigned short)(((Buffer[0]) << 8) | Buffer[1]);
        }
//        unResult = ((int)lpszArg[0]) < 0 ? 256 + lpszArg[0] : lpszArg[0];
#endif
        ValSetLong(lpResult, unResult);
      }
      GlobalUnlock(hArg);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_concatenate(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  HGLOBAL hArg;
  HGLOBAL hResult;
  LPTSTR lpszArg;
  LPTSTR lpszResult;
  int nLen = 0;
  int i;

  if (CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr))
  {
    for (i = 0; i < nArgs; i++)
    {
      hArg = ValGetStringHandle(lpArg+i);
      if (hArg && (lpszArg = (LPTSTR)GlobalLock(hArg)))
      {
        nLen += lstrlen(lpszArg);
        GlobalUnlock(hArg);
      }
    }
    hResult = GlobalAlloc(GHND, (nLen + 1) * sizeof(TCHAR));
    if (hResult && (lpszResult = (LPTSTR)GlobalLock(hResult)))
    {
      lstrcpy(lpszResult, _T(""));
      for (i = 0; i < nArgs; i++)
      {
        hArg = ValGetStringHandle(lpArg+i);
        if (hArg && (lpszArg = (LPTSTR)GlobalLock(hArg)))
        {
          lstrcat(lpszResult, lpszArg);
          GlobalUnlock(hArg);
        }
      }
      ValSetStringHandle(lpResult, hResult);
      GlobalUnlock(hResult);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_exact(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  LPTSTR lpszArg1, lpszArg2;
  HGLOBAL hArg1, hArg2;

  if (CheckAllStrOrEmpty (lpArg, nArgs, &bAnyStr))
  {
    hArg1 = ValGetStringHandle (lpArg);
    hArg2 = ValGetStringHandle (lpArg+1);

    if (!hArg1 && !hArg2)
      ValSetBool(lpResult, TRUE);

    else if (!hArg1 || !hArg2)
      ValSetBool(lpResult, FALSE);

    else
    {
      if ((lpszArg1 = (LPTSTR)GlobalLock (hArg1)) &&
          (lpszArg2 = (LPTSTR)GlobalLock (hArg2)))
      {
        if (0 == _tcscmp(lpszArg1, lpszArg2))
          ValSetBool(lpResult, TRUE);
        else
          ValSetBool(lpResult, FALSE);

        GlobalUnlock(hArg1);
        GlobalUnlock(hArg2);
      }
      else
        ValSetError(lpResult);
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_find(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  HGLOBAL hFindText;
  HGLOBAL hWithinText;
  LPTSTR pszFindText = NULL;
  LPTSTR pszWithinText = NULL;
  long lFindTextLen;
  long lWithinTextLen;
  long lStartPos = 1;
  long i;

  if( CheckAllStrOrEmpty(lpArg, 2, &bAnyStr) &&
      (nArgs <= 2 || CheckAllNumOrEmpty(lpArg + 2, 1, &bAnyDouble, &bAnyLong, &bAnyEmpty)) )
  {
    hFindText = ValGetStringHandle(lpArg);
    hWithinText = ValGetStringHandle(lpArg+1);
    if (hFindText)
      pszFindText = (LPTSTR)GlobalLock(hFindText);
    if (hWithinText)
      pszWithinText = (LPTSTR)GlobalLock(hWithinText);
    if ( nArgs > 2 )
      lStartPos = ValGetLong((lpArg + 2));

    lFindTextLen = pszFindText ? (long)_tcsclen(pszFindText) : 0;
    lWithinTextLen = pszWithinText ? (long)_tcsclen(pszWithinText) : 0;
    if( 1 <= lStartPos && lStartPos + lFindTextLen <= lWithinTextLen + 1 )
    {
      for( i = 0; i <= lWithinTextLen - lFindTextLen; i++, pszWithinText = CharNext(pszWithinText) )
      {
        if( i >= lStartPos - 1 && 0 == _tcsnccmp(pszFindText, pszWithinText, lFindTextLen) )
          break;
      }
      if( i >= lStartPos - 1 && i <= lWithinTextLen - lFindTextLen )
        ValSetLong(lpResult, i + 1);
      else
        ValSetError(lpResult);
    }
    else
      ValSetError(lpResult);
    if (hFindText)
      GlobalUnlock(hFindText);
    if (hWithinText)
      GlobalUnlock(hWithinText);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_isblank(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  return Calc_isempty(lpResult, lpArg, nArgs);
}

/*
BOOL CALLBACK Calc_islogical(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyBool;
  BOOL bAnyEmpty;

  if( CheckAllBoolOrEmpty(lpArg, nArgs, &bAnyBool, &bAnyEmpty) )
    ValSetBool(lpResult, TRUE);
  else
    ValSetBool(lpResult, FALSE);
  return TRUE;
}
*/

#define DATE_1899_12_30 0.0
#define DATE_9999_12_31 2958465.0

BOOL CALLBACK Calc_date(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  long lYear;
  long lMonth;
  long lDay;
  double dfDate;

  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    lYear = ValGetLong(lpArg);
    lMonth = ValGetLong(lpArg + 1);
    lDay = ValGetLong(lpArg + 2);
    if (0 <= lYear && lYear < 100)
      lYear += 1900;
    if (0 < lMonth)
    {
      lYear = lYear + (lMonth - 1) / 12;
      lMonth = (lMonth - 1) % 12 + 1;
    }
    else
    {
      lYear = lYear - (-lMonth) / 12 - 1;
      lMonth = 12 - (-lMonth) % 12;
    }
    ftDateToDouble((short)lYear, (short)lMonth, 1, &dfDate);
    dfDate = dfDate + (lDay - 1);
    if (DATE_1899_12_30 <= dfDate && dfDate <= DATE_9999_12_31)
      ValSetDouble(lpResult, dfDate);
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_day(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfDate;
  short nDay;

  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    dfDate = ValGetDouble(lpArg);
    if (DATE_1899_12_30 <= dfDate && dfDate < DATE_9999_12_31 + 1.0)
    {
      ftDoubleToDate(dfDate, NULL, NULL, &nDay);
      ValSetLong(lpResult, nDay);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_hour(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  HGLOBAL hDate;
  LPTSTR lpszDate;
  double dfDate;
  short nHour;

  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    dfDate = ValGetDouble(lpArg);
    if (DATE_1899_12_30 <= dfDate && dfDate < DATE_9999_12_31 + 1.0)
    {
      ftDoubleToTime(dfDate, &nHour, NULL, NULL);
      ValSetLong(lpResult, nHour);
    }
    else
      ValSetError(lpResult);
  }
  else if (CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr))
  {
    hDate = ValGetStringHandle(lpArg);
    if (hDate && (lpszDate = (LPTSTR)GlobalLock(hDate)))
    {
      if (-1 != fpConvertTSTRtoType(lpszDate, &dfDate, VT_DATE))
      {
        ftDoubleToTime(dfDate, &nHour, NULL, NULL);
        ValSetLong(lpResult, nHour);
      }
      else
        ValSetError(lpResult);
      GlobalUnlock(hDate);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_left(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  HGLOBAL hText;
  HGLOBAL hResult;
  LPTSTR pszText;
  LPTSTR pszResult;
  long lTextLen;
  long lNumChars = 1;
  long lNumBytes;

  if( CheckAllStrOrEmpty(lpArg, 1, &bAnyStr) &&
      (nArgs <= 1 || CheckAllNumOrEmpty(lpArg + 1, 1, &bAnyDouble, &bAnyLong, &bAnyEmpty)) )
  {
    hText = ValGetStringHandle(lpArg);
    if( nArgs > 1 )
      lNumChars = ValGetLong(lpArg + 1);
    if( lNumChars < 0 )
      ValSetError(lpResult);
    else if( hText && (pszText = (LPTSTR)GlobalLock(hText)) )
    {
      lTextLen = (long)_tcslen(pszText);
      lNumChars = min(lTextLen, lNumChars);
      lNumBytes = (long)_tcsnbcnt (pszText, lNumChars);
      if( lNumBytes >= 0 && lNumBytes <= lTextLen &&
          (hResult = GlobalAlloc(GHND, (lNumBytes + 1) * sizeof(TCHAR))))
      {
        pszResult = (LPTSTR)GlobalLock(hResult);
        _tcsncpy(pszResult, pszText, lNumBytes);
        pszResult[lNumBytes] = 0;
        GlobalUnlock(hResult);
        ValSetStringHandle (lpResult, hResult);
      }
      else
        ValSetError(lpResult);
      GlobalUnlock(hText);
    }
    else
      ValSetString(lpResult, _T(""));
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_len(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  HGLOBAL hText;
  LPTSTR pszText;
  long lLen = 0;

  if( CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr) )
  {
    hText = ValGetStringHandle(lpArg);
    if (hText && (pszText = (LPTSTR)GlobalLock(hText)))
    {
      lLen = (long)_tcsclen(pszText);
      GlobalUnlock(hText);
    }
    ValSetLong(lpResult, lLen);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_lower(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  HGLOBAL hText;
  HGLOBAL hResult = NULL;
  LPTSTR pszText;
  LPTSTR pszResult;

  if( CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr) )
  {
    hText = ValGetStringHandle(lpArg);
    if (hText && (pszText = (LPTSTR)GlobalLock(hText)))
    {
      hResult = GlobalAlloc(GHND, (lstrlen(pszText) + 1) * sizeof(TCHAR));
      if (hResult && (pszResult = (LPTSTR)GlobalLock(hResult)))
      {
        lstrcpy(pszResult, pszText);
        CharLower(pszResult);
        GlobalUnlock(hResult);
        ValSetStringHandle(lpResult, hResult);
      }
      else
        ValSetError(lpResult);
      GlobalUnlock(hText);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_mid(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  HGLOBAL hText;
  HGLOBAL hResult;
  LPTSTR pszText;
  LPTSTR pszResult;
  long lTextLen;
  long lStartPos;
  long lNumChars;

  if( CheckAllStrOrEmpty(lpArg, 1, &bAnyStr) &&
      CheckAllNumOrEmpty(lpArg + 1, 2, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    hText = ValGetStringHandle(lpArg);
    lStartPos = ValGetLong(lpArg + 1);
    lNumChars = ValGetLong(lpArg + 2);
    if( lStartPos < 1 || lNumChars < 0 )
      ValSetError(lpResult);
    else if( hText && (pszText = (LPTSTR)GlobalLock(hText)) )
    {
      lTextLen = (long)_tcsclen(pszText);
      lStartPos = min(lTextLen + 1, lStartPos);
      lNumChars = min(lTextLen - lStartPos + 1, lNumChars);
      hResult = GlobalAlloc(GHND, (lNumChars + 1) * max(2, sizeof(TCHAR)));
      if( hResult && (pszResult = (LPTSTR)GlobalLock(hResult)) )
      {
        //_tcsncpy(pszResult, pszText + lStartPos - 1, lNumChars);
        //pszResult[lNumChars] = 0;
        LPTSTR pszEnd;

        for ( ; *pszText && lStartPos > 1; pszText = CharNext(pszText), lStartPos--)
          ;

        for (pszEnd = pszText ; *pszEnd && lNumChars > 0; pszEnd = CharNext(pszEnd), lNumChars--)
          ;

        _tcsncpy(pszResult, pszText, pszEnd - pszText);
        pszEnd = 0;
        GlobalUnlock(hResult);
        ValSetStringHandle (lpResult, hResult);
      }
      else
        ValSetError(lpResult);
      GlobalUnlock(hText);
    }
    else
      ValSetString(lpResult, _T(""));
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_minute(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  HGLOBAL hDate;
  LPTSTR lpszDate;
  double dfDate;
  short nMinute;

  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    dfDate = ValGetDouble(lpArg);
    if (DATE_1899_12_30 <= dfDate && dfDate < DATE_9999_12_31 + 1.0)
    {
      ftDoubleToTime(dfDate, NULL, &nMinute, NULL);
      ValSetLong(lpResult, nMinute);
    }
    else
      ValSetError(lpResult);
  }
  else if (CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr))
  {
    hDate = ValGetStringHandle(lpArg);
    if (hDate && (lpszDate = (LPTSTR)GlobalLock(hDate)))
    {
      if (-1 != fpConvertTSTRtoType(lpszDate, &dfDate, VT_DATE))
      {
        ftDoubleToTime(dfDate, NULL, &nMinute, NULL);
        ValSetLong(lpResult, nMinute);
      }
      else
        ValSetError(lpResult);
      GlobalUnlock(hDate);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_month(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfDate;
  short nMonth;

  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    dfDate = ValGetDouble(lpArg);
    if (DATE_1899_12_30 <= dfDate && dfDate < DATE_9999_12_31 + 1.0)
    {
      ftDoubleToDate(dfDate, NULL, &nMonth, NULL);
      ValSetLong(lpResult, nMonth);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_now(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  SYSTEMTIME t;
  double dfDate;
  double dfTime;

  GetLocalTime(&t);
  ftDateToDouble(t.wYear, t.wMonth, t.wDay, &dfDate);
  ftTimeToDouble(t.wHour, t.wMinute, t.wSecond, &dfTime);
  ValSetDouble(lpResult, dfDate + dfTime);
  return TRUE;
}

BOOL CALLBACK Calc_or(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  BOOL bAnyBool = FALSE;
  BOOL bAnyError = FALSE;
  SS_COORD c, r;
  CALC_VALUE val;
  BOOL bResult;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++)
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      switch( lpArg->Type )
      {
        case CALC_VALUE_TYPE_DOUBLE:
          if( bAnyBool )
            bResult |= (ValGetDouble(lpArg) ? TRUE : FALSE);
          else
            bResult = (ValGetDouble(lpArg) ? TRUE : FALSE);
          bAnyBool = TRUE;
          break;
        case CALC_VALUE_TYPE_LONG:
          if( bAnyBool )
            bResult |= (ValGetLong(lpArg) ? TRUE : FALSE);
          else
            bResult = (ValGetLong(lpArg) ? TRUE : FALSE);
          bAnyBool = TRUE;
          break;
        case CALC_VALUE_TYPE_RANGE:
          lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
          for( r = lpArg->Val.ValRange.Row1; r <= lpArg->Val.ValRange.Row2; r++ )
          {
            for( c = lpArg->Val.ValRange.Col1; c <= lpArg->Val.ValRange.Col2; c++ )
            {
              lpSheet->lpfnGetData(lpSheet->hSS, c, r, &val);
              if( CALC_VALUE_STATUS_ERROR == val.Status )
                bAnyError = TRUE;
              else if( CALC_VALUE_STATUS_OK == val.Status )
              {
                switch( val.Type )
                {
                  case CALC_VALUE_TYPE_DOUBLE:
                    if( bAnyBool )
                      bResult |= (ValGetDouble(&val) ? TRUE : FALSE);
                    else
                      bResult = (ValGetDouble(&val) ? TRUE : FALSE);
                    bAnyBool = TRUE;
                    break;
                  case CALC_VALUE_TYPE_LONG:
                    if( bAnyBool )
                      bResult |= (ValGetLong(&val) ? TRUE : FALSE);
                    else
                      bResult = (ValGetLong(&val) ? TRUE : FALSE);
                    bAnyBool = TRUE;
                    break;
                }
              }
              ValFree(&val);
            }
          }
          break;
      }
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if( bAnyBool )
    ValSetBool(lpResult, bResult);
  else
    ValSetEmpty(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_proper(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  HGLOBAL hText;
  HGLOBAL hResult;
  LPTSTR pszText;
  LPTSTR pszResult;
  BOOL bLastWasAlpha = FALSE;

  if( CheckAllStrOrEmpty(lpArg, 1, &bAnyStr) )
  {
    hText = ValGetStringHandle(lpArg);
    if( hText && (pszText = (LPTSTR)GlobalLock(hText)) )
    {
      hResult = GlobalAlloc(GHND, (_tcslen(pszText) + 1) * sizeof(TCHAR));
      if( hResult && (pszResult = (LPTSTR)GlobalLock(hResult)) )
      {
        wint_t iWord;
        lstrcpy(pszResult, pszText);

        for ( ; *pszResult; pszResult = CharNext(pszResult))
        {
          if( bLastWasAlpha )
            CharLower(pszResult);
          else
            CharUpper(pszResult);

          iWord = (TUCHAR)*pszResult;
          if (_TIsDBCSLeadByte((BYTE)*pszResult))
            iWord = (wint_t)((iWord << 8) | (BYTE)*(pszResult + 1));

          bLastWasAlpha = _istalpha(iWord);
        }
/*
        for( ; *pszArg; pszArg++, pszResult++ )
        {
          if( bLastWasAlpha )
            *pszResult = _totlower(*pszArg);
          else
            *pszResult = _totupper(*pszArg);
          bLastWasAlpha = _istalpha(*pszArg);
        }
*/
        *pszResult = 0;
        GlobalUnlock(hResult);
        ValSetStringHandle (lpResult, hResult);
      }
      GlobalUnlock(hText);
    }
    else
      ValSetString(lpResult, _T(""));
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_replace(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  HGLOBAL hOldText;
  HGLOBAL hNewText;
  HGLOBAL hResult;
  LPTSTR pszOldText = NULL;
  LPTSTR pszNewText = NULL;
  LPTSTR pszResult;
  long lOldTextBytes;
  long lNewTextBytes;
  long lStartChar;
  long lNumChars;
  long lStartByte = 0;
  long lNumBytes = 0;

  if( CheckAllStrOrEmpty(lpArg, 1, &bAnyStr)
      && CheckAllNumOrEmpty(lpArg + 1, 2, &bAnyDouble, &bAnyLong, &bAnyEmpty) 
      && CheckAllStrOrEmpty(lpArg + 3, 1, &bAnyStr) )
  {
    hOldText = ValGetStringHandle(lpArg);
    lStartChar = ValGetLong(lpArg + 1);
    lNumChars = ValGetLong(lpArg + 2);
    hNewText = ValGetStringHandle(lpArg + 3);
    if (hOldText)
      pszOldText = (LPTSTR)GlobalLock(hOldText);
    if (hNewText)
      pszNewText = (LPTSTR)GlobalLock(hNewText);
    lOldTextBytes = pszOldText ? (long)_tcslen(pszOldText) : 0;
    lNewTextBytes = pszNewText ? (long)_tcslen(pszNewText) : 0;

    if( lStartChar >= 1 && lNumChars >= 0 )
    {
      if (pszOldText)
      {
        lStartByte = min((long)_tcsnbcnt(pszOldText, lStartChar - 1), lOldTextBytes);
        lNumBytes = min((long)_tcsnbcnt(pszOldText + lStartByte, lNumChars), lOldTextBytes - lStartByte);
      }

      hResult = GlobalAlloc(GHND, (lOldTextBytes - lNumBytes + lNewTextBytes + 1) * sizeof(TCHAR));
      if( hResult && (pszResult = (LPTSTR)GlobalLock(hResult)) )
      {
        if( pszOldText )
          _tcsncpy(pszResult, pszOldText, lStartByte);
        if( pszNewText )
          _tcsncpy(pszResult + lStartByte, pszNewText, lNewTextBytes);
        if( pszOldText )
          _tcsncpy(pszResult + lStartByte + lNewTextBytes, pszOldText + lStartByte + lNumBytes, lOldTextBytes - lStartByte + 1 - lNumBytes);
        pszResult[lOldTextBytes - lNumBytes + lNewTextBytes] = 0;
        GlobalUnlock(hResult);
        ValSetStringHandle (lpResult, hResult);
      }
      else
        ValSetError(lpResult);
    }
    else
      ValSetError(lpResult);
    if (hOldText)
      GlobalUnlock(hOldText);
    if (hNewText)
      GlobalUnlock(hNewText);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_rept(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  HGLOBAL hText;
  HGLOBAL hResult;
  LPTSTR pszText;
  LPTSTR pszResult;
  long lTextLen;
  long lNumTimes;

  if( CheckAllStrOrEmpty(lpArg, 1, &bAnyStr)
      && CheckAllNumOrEmpty(lpArg + 1, 1, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    hText = ValGetStringHandle(lpArg);
    lNumTimes = ValGetLong(lpArg + 1);
    if( lNumTimes < 0 )
     ValSetError(lpResult);
    else if( hText && (pszText = (LPTSTR)GlobalLock(hText)) )
    {
      lTextLen = (long)_tcslen(pszText);
      hResult = GlobalAlloc(GHND, (lTextLen * lNumTimes + 1) * sizeof(TCHAR));
      if( hResult && (pszResult = (LPTSTR)GlobalLock(hResult)) )
      {
        for( ; lNumTimes > 0; lNumTimes--, pszResult += lTextLen )
          _tcscpy(pszResult, pszText);
        GlobalUnlock(hResult);
        ValSetStringHandle (lpResult, hResult);
      }
      else
        ValSetError(lpResult);
      GlobalUnlock(hText);
    }
    else
      ValSetString(lpResult, _T(""));
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_right(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  HGLOBAL hText;
  HGLOBAL hResult;
  LPTSTR pszText;
  LPTSTR pszResult;
  LPTSTR pszRight;
  long lTextLen;
  long lNumChars = 1;
  long lNumBytes;
  long i;

  if( CheckAllStrOrEmpty(lpArg, 1, &bAnyStr) &&
      (nArgs <= 1 || CheckAllNumOrEmpty(lpArg + 1, 1, &bAnyDouble, &bAnyLong, &bAnyEmpty)) )
  {
    hText = ValGetStringHandle(lpArg);
    if( nArgs > 1 )
      lNumChars = ValGetLong(lpArg + 1);
    if( lNumChars < 0 )
      ValSetError(lpResult);
    else if( hText && (pszText = (LPTSTR)GlobalLock(hText)) )
    {
      lTextLen = (long)_tcslen(pszText);

      for (pszRight = pszText + lTextLen, i = 0; pszRight && i < lNumChars; i++, pszRight = CharPrev(pszText, pszRight))
        ;

      if (!pszRight)
        pszRight = pszText;

      lNumBytes = min(lTextLen, (long)_tcslen(pszRight)); 
      if (lNumBytes >= 0)
      {
        hResult = GlobalAlloc(GHND, (lNumBytes + 1) * sizeof(TCHAR));
        if( hResult && (pszResult = (LPTSTR)GlobalLock(hResult)) )
        {
          lstrcpy(pszResult, pszRight);
          GlobalUnlock(hResult);
          ValSetStringHandle (lpResult, hResult);
        }
        else
          ValSetError(lpResult);
      }
      else
        ValSetError(lpResult);

      GlobalUnlock(hText);
    }
    else
      ValSetString(lpResult, _T(""));
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_trunc(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfValue;
  long lNumDigits;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    if( bAnyEmpty )
      ValSetEmpty(lpResult);
    else
    {
      dfValue = ValGetDouble(lpArg);
      if( nArgs < 2 )
        lNumDigits = 0;
      else
        lNumDigits = ValGetLong(lpArg+1);
      ValSetDouble(lpResult, ApproxRoundDown(dfValue, lNumDigits));
    }
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

/*
// Add this function back in when wildcard support (*, ?) is added
BOOL CALLBACK Calc_search(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  LPTSTR pszFindText;
  LPTSTR pszWithinText;
  long lFindTextLen;
  long lWithinTextLen;
  long lStartPos = 1;
  long i;

  if( CheckAllStrOrEmpty(lpArg, 2, &bAnyStr) &&
      (nArgs <= 2 || CheckAllNumOrEmpty(lpArg + 2, 1, &bAnyDouble, &bAnyLong, &bAnyEmpty)) )
  {
    pszFindText = (LPTSTR)GlobalLock(lpArg->Val.hValStr);
    pszWithinText = (LPTSTR)GlobalLock((lpArg + 1)->Val.hValStr);
    if( nArgs > 2 )
      lStartPos = ValGetLong(lpArg + 2);
    lFindTextLen = pszFindText ? _tcslen(pszFindText) : 0;
    lWithinTextLen = pszWithinText ? _tcslen(pszWithinText) : 0;
    if( 1 <= lStartPos && lStartPos + lFindTextLen <= lWithinTextLen + 1 )
    {
      for( i = lStartPos - 1; i <= lWithinTextLen - lFindTextLen; i++ )
      {
        if( 0 == _tcsnicmp(pszFindText, pszWithinText + i, lFindTextLen) )
          break;
      }
      if( i <= lWithinTextLen - lFindTextLen )
        ValSetLong(lpResult, i + 1);
      else
        ValSetError(lpResult);
    }
    else
      ValSetError(lpResult);
    GlobalUnlock(lpArg->Val.hValStr);
    GlobalUnlock((lpArg + 1)->Val.hValStr);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
*/

BOOL CALLBACK Calc_second(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  HGLOBAL hDate;
  LPTSTR lpszDate;
  double dfDate;
  short nSecond;

  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    dfDate = ValGetDouble(lpArg);
    if (DATE_1899_12_30 <= dfDate && dfDate < DATE_9999_12_31 + 1.0)
    {
      ftDoubleToTime(dfDate, NULL, NULL, &nSecond);
      ValSetLong(lpResult, nSecond);
    }
    else
      ValSetError(lpResult);
  }
  else if (CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr))
  {
    hDate = ValGetStringHandle(lpArg);
    if (hDate && (lpszDate = (LPTSTR)GlobalLock(hDate)))
    {
      if (-1 != fpConvertTSTRtoType(lpszDate, &dfDate, VT_DATE))
      {
        ftDoubleToTime(dfDate, NULL, NULL, &nSecond);
        ValSetLong(lpResult, nSecond);
      }
      else
        ValSetError(lpResult);
      GlobalUnlock(hDate);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_substitute(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  HGLOBAL hText;
  HGLOBAL hOldText;
  HGLOBAL hNewText;
  LPTSTR pszResult;
  LPTSTR pszText = NULL;
  LPTSTR pszOldText = NULL;
  LPTSTR pszNewText = NULL;
  HGLOBAL hResult;
  long lTextLen;
  long lOldTextLen;
  long lNewTextLen;
  long lResultLen;
  long lInstNum = 1;
  long lInst;
  long i, j;

  if( CheckAllStrOrEmpty(lpArg, 3, &bAnyStr) &&
      (nArgs <= 3 || CheckAllNumOrEmpty(lpArg + 3, 1, &bAnyDouble, &bAnyLong, &bAnyEmpty)) )
  {
    hText = ValGetStringHandle(lpArg);
    hOldText = ValGetStringHandle(lpArg+1);
    hNewText = ValGetStringHandle(lpArg+2);
    if( nArgs > 3 )
      lInstNum = ValGetLong(lpArg+3);
    if (hText)
      pszText = (LPTSTR)GlobalLock(hText);
    if (hOldText)
      pszOldText = (LPTSTR)GlobalLock(hOldText);
    if (hNewText);
      pszNewText = (LPTSTR)GlobalLock(hNewText);
    lTextLen = pszText ? (long)_tcslen(pszText) : 0;
    lOldTextLen = pszOldText ? (long)_tcslen(pszOldText) : 0;
    lNewTextLen = pszNewText ? (long)_tcslen(pszNewText) : 0;
    if( nArgs <= 3 || lInstNum >= 1 )
    {
      if( lTextLen > 0 && lOldTextLen > 0 )
      {
        for( lResultLen = 0, lInst = 0, i = 0; i < lTextLen; )
        {
          if( 0 == _tcsncmp(pszText+i, pszOldText, lOldTextLen) )
          {
            lInst += 1;
            if( nArgs <= 3 || lInst == lInstNum )
            {
              lResultLen += lNewTextLen;
              i += lOldTextLen;
            }
            else
            {
              lResultLen += lOldTextLen;
              i += lOldTextLen;
            }
          }
          else
          {
            lResultLen += 1;
            i += 1;
          }
        }
        hResult = GlobalAlloc(GHND, (lResultLen + 1) * sizeof(TCHAR));
        if( hResult && (pszResult = (LPTSTR)GlobalLock(hResult)) )
        {
          for( lInst = 0, i = 0, j = 0; i < lTextLen; )
          {
            if( 0 == _tcsncmp(pszText + i, pszOldText, lOldTextLen) )
            {
              lInst += 1;
              if( nArgs <= 3 || lInst == lInstNum )
              {
                _fmemcpy(pszResult + j, pszNewText, lNewTextLen * sizeof(TCHAR));
                i += lOldTextLen;
                j += lNewTextLen;
              }
              else
              {
                _fmemcpy(pszResult + j, pszText + i, lOldTextLen * sizeof(TCHAR));
                i += lOldTextLen;
                j += lOldTextLen;
              }
            }
            else
            {
              _fmemcpy(pszResult + j, pszText + i, sizeof(TCHAR));
              i += 1;
              j += 1;
            }
          }
          pszResult[lResultLen] = 0;
          GlobalUnlock(hResult);
          ValSetStringHandle (lpResult, hResult);
        }
        else
          ValSetError(lpResult);
      }
      else
        ValSetString(lpResult, pszText);
    }
    else
      ValSetError(lpResult);
    if (hText)
      GlobalUnlock(hText);
    if (hOldText)
      GlobalUnlock(hOldText);
    if (hNewText)
      GlobalUnlock(hNewText);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_today(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  SYSTEMTIME t;
  double dfDate;

  GetLocalTime(&t);
  ftDateToDouble(t.wYear, t.wMonth, t.wDay, &dfDate);
  ValSetDouble(lpResult, dfDate);
  return TRUE;
}

BOOL CALLBACK Calc_time(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  long lHour;
  long lMinute;
  long lSecond;
  double dfTime;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    lHour = ValGetLong(lpArg);
    lMinute = ValGetLong(lpArg+1);
    lSecond = ValGetLong(lpArg+2);
    ftTimeToDouble((short)lHour, (short)lMinute, (short)lSecond, &dfTime);
    ValSetDouble(lpResult, dfTime);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_trim(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  BOOL bHaveSpace = FALSE;
  BOOL bHaveWord = FALSE;
  HGLOBAL hText;
  HGLOBAL hResult = NULL;
  LPTSTR pszText;
  LPTSTR pszResult;

  if( CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr) )
  {
    hText = ValGetStringHandle(lpArg);
    if (hText && (pszText = (LPTSTR)GlobalLock(hText)))
    {
      hResult = GlobalAlloc(GHND, (lstrlen(pszText) + 1) * sizeof(TCHAR));
      if (hResult && (pszResult = (LPTSTR)GlobalLock(hResult)))
      {
        lstrcpy(pszResult, pszText);
        StrTrimAll(pszResult);
        GlobalUnlock(hResult);
        ValSetStringHandle (lpResult, hResult);
      }
      else
        ValSetError(lpResult);
      GlobalUnlock(hText);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_upper(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyStr;
  HGLOBAL hText;
  HGLOBAL hResult = NULL;
  LPTSTR pszText;
  LPTSTR pszResult;

  if( CheckAllStrOrEmpty(lpArg, nArgs, &bAnyStr) )
  {
    hText = ValGetStringHandle(lpArg);
    if (hText && (pszText = (LPTSTR)GlobalLock(hText)))
    {
      hResult = GlobalAlloc(GHND, (lstrlen(pszText) + 1) * sizeof(TCHAR));
      if (hResult && (pszResult = (LPTSTR)GlobalLock(hResult)))
      {
        lstrcpy(pszResult, pszText);
        CharUpper(pszResult);
        GlobalUnlock(hResult);
        ValSetStringHandle (lpResult, hResult);
      }
      else
        ValSetError(lpResult);
      GlobalUnlock(hText);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_weekday(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfDate;
  long lRetType = 1;
  short nWeekday;
  BOOL bOK = FALSE;

  if ((CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty)))
  {
    dfDate = ValGetDouble(lpArg);
    if (nArgs > 1)
      lRetType = ValGetLong(lpArg+1);
    if (DATE_1899_12_30 <= dfDate && dfDate < DATE_9999_12_31 + 1.0)
    {
      ftDoubleToWeekday(dfDate, &nWeekday);
      switch (lRetType)
      {
        case 1: ValSetLong(lpResult, nWeekday + 1); break;
        case 2: ValSetLong(lpResult, (nWeekday + 6) % 7 + 1); break;
        case 3: ValSetLong(lpResult, (nWeekday + 6) % 7); break;
        default: ValSetError(lpResult); break;
      }
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_year(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfDate;
  short nYear;

  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    dfDate = ValGetDouble(lpArg);
    if (DATE_1899_12_30 <= dfDate && dfDate < DATE_9999_12_31 + 1.0)
    {
      ftDoubleToDate(dfDate, &nYear, NULL, NULL);
      ValSetLong(lpResult, nYear);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}


void sjis2jis( unsigned char *c1, unsigned char *c2)
{
if( *c2 < 0x9f )
   {
   if( *c1 < 0xa0 )
      {
      *c1 -= 0x81;
      *c1 *= 2;
      *c1 += 0x21;
      }
   else
      {
      *c1 -= 0xe0;
      *c1 *= 2;
      *c1 += 0x5f;
      }
   if( *c2 > 0x7f )
      -- *c2;
   *c2 -= 0x1f;
   }

else
   {
   if( *c1 < 0xa0 )
      {
      *c1 -= 0x81;
      *c1 *= 2;
      *c1 += 0x22;
      }
   else
      {
      *c1 -= 0xe0;
      *c1 *= 2;
      *c1 += 0x60;
      }
   *c2 -= 0x7e;
   }
}


void jis2sjis( unsigned char *c1, unsigned char *c2)
{
/* Conversion of 2nd byte */

if( ( *c1 % 2 ) == 0 )
   *c2 += 0x7d;
else
   *c2 += 0x1f;

if( *c2 > 0x7e )
   ++ *c2;

/* Conversion of 1st byte */

if( *c1 < 0x5f )
   {
   ++ *c1;
   *c1 /= 2;
   *c1 += 0x70;
   }
else
   {
   ++ *c1;
   *c1 /= 2;
   *c1 += 0xb0;
   }
}

#endif // SS_V35

#endif // SS_V30

#ifdef SS_V80
BOOL CALLBACK Calc_avedev(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfSum = 0.0;
  double dfMean = 0.0;
  double dfSumDev = 0.0;
  long lMean = 0;
  long lSum = 0;
  long lSumDev = 0;
  long lCount = 0;
  long lCol;
  long lRow;

  short args = nArgs;
  LPCALC_VALUE lpArgTemp = lpArg;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        dfSum += lpArg->Val.ValLong;
        lSum += lpArg->Val.ValLong;
        lCount += 1;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        dfSum += lpArg->Val.ValDouble;
        lCount += 1;
        bAnyDouble = TRUE;
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
                dfSum += value.Val.ValLong;
                lSum += value.Val.ValLong;
                lCount += 1;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                dfSum += value.Val.ValDouble;
                lCount += 1;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                dfSum += value.Val.ValLong;
                lSum += value.Val.ValLong;
                lCount += 1;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                dfSum += value.Val.ValDouble;
                lCount += 1;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #endif

    }
  }

  dfMean = dfSum / lCount;
  lMean = lSum / lCount;

  lpArg = lpArgTemp;
  nArgs = args;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        dfSumDev += abs(lpArg->Val.ValLong - lMean);
 	    lSumDev += abs(lpArg->Val.ValLong - lMean);
     }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        dfSumDev += fabs(lpArg->Val.ValDouble - dfMean);
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
				dfSumDev += abs(value.Val.ValLong - lMean);
 				lSumDev += abs(value.Val.ValLong - lMean);
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				dfSumDev += fabs(value.Val.ValDouble - dfMean);
              }
            }
            ValFree(&value);
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
				dfSumDev += abs(value.Val.ValLong - lMean);
 				lSumDev += abs(value.Val.ValLong - lMean);
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				dfSumDev += fabs(value.Val.ValDouble - dfMean);
              }
            }
            ValFree(&value);
          }
      }
      #endif

    }
  }


  if( bAnyError )
  {
    ValSetError(lpResult);
    return TRUE;
  }
  else if( bAnyDouble )
    ValSetDouble(lpResult, dfSumDev / lCount);
  else if( bAnyLong )
  {
    if( lSum == lSum / lCount * lCount )
      ValSetLong(lpResult, lSumDev / lCount);
    else
      ValSetDouble(lpResult, dfSumDev / lCount);
  }
  else
    ValSetEmpty(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_binomdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  long x;
  long n;
  double p;
  BOOL cum;
  int i;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    x = ValGetLong(lpArg);
    n = ValGetLong(lpArg+1);
	p = ValGetDouble(lpArg+2);
	cum = ValGetBool(lpArg+3);

    if (x < 0 || n < 0 || n < x || p < 0.0 || 1.0 < p)
	{
		ValSetError(lpResult);
        return TRUE;
	}
    if (!cum)
      {
        double q = 1.0 - p;
        double factor = pow(q, n);
        if (factor == 0.0)
        {
          factor = pow(p, n);
          if (factor == 0.0)
		  {
			ValSetError(lpResult);
		    return TRUE;
		  }
          else
          {
            for (i = 0; i < n - x && factor > 0.0; i++)
              factor *= (double)(n-i) / (double)(i+1) * q / p;
            ValSetDouble(lpResult, factor);
			return TRUE;
          }
        }
        else
        {
          for (i = 0; i < x && factor > 0.0; i++)
            factor *= (double)(n-i) / (double)(i+1) * p / q;
          ValSetDouble(lpResult, factor);
		  return TRUE;
        }
      }
      else
      {
        if (n == x)
		{
          ValSetDouble(lpResult, 1.0);
		  return TRUE;
		}
        else
        {
          double q = 1.0 - p;
          double factor = pow(q, n);
          if (factor == 0.0)
          {
            factor = pow(p, n);
            if (factor == 0.0)
			{
				ValSetError(lpResult);
                return TRUE;
			}
            else
            {
              double sum = 1.0 - factor;
              for (i = 0; i < n - x && factor > 0.0; i++)
              {
                factor *= (double)(n-i) / (double)(i+1) * q / p;
                sum -= factor;
              }
              if (sum < 0.0)
			  {
				ValSetDouble(lpResult, 0.0);
				return TRUE;
			  }
              else
			  {
				ValSetDouble(lpResult, sum);
				return TRUE;
			  }
            }
          }
          else
          {
            double sum = factor;
            for (i = 0; i < x && factor > 0.0; i++)
            {
              factor *= (double)(n-i) / (double)(i+1) * p / q;
              sum += factor;
            }
			ValSetDouble(lpResult, sum);
			return TRUE;
          }
        }
      }

  }
  else
    ValSetError(lpResult);
  return TRUE;
}


BOOL CALLBACK Calc_critbinom(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  long n;
  double p;
  double alpha;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    n = ValGetLong(lpArg);
    p = ValGetDouble(lpArg+1);
	alpha = ValGetDouble(lpArg+2);

      if (n < 0 || p < 0.0 || 1.0 < p || alpha <= 0.0 || 1.0 <= alpha )
	  {
		ValSetError(lpResult);
        return TRUE;
	  }
      else
      {
        double q = 1.0 - p;
        double factor = pow(q, n);
        if (factor == 0.0)
        {
          factor = pow(p, n);
          if (factor == 0.0)
		  {
			ValSetError(lpResult);
            return TRUE;
		  }
          else
          {
            double sum = 1.0 - factor;
            int i;
            for (i = 0; i < n && sum >= alpha; i++)
            {
              factor *= (double)(n-i) / (double)(i+1) * q / p;
              sum -= factor;
            }
			ValSetDouble(lpResult, (double)(n-i));
            return TRUE;
          }
        }
        else
        {
          double sum = factor;
          int i;
          for (i = 0; i < n && sum < alpha; i++)
          {
            factor *= (double)(n-i) / (double)(i+1) * p / q;
            sum += factor;
          }
          ValSetDouble(lpResult,(double)(i));
		  return TRUE;
        }
      }
	}
  return TRUE;
}

BOOL CALLBACK Calc_negbinomdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  long x;
  long r;
  double p, pt1, pt2, pt3;
  CALC_VALUE args[2];

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    x = ValGetLong(lpArg);
    r = ValGetLong(lpArg+1);
	p = ValGetDouble(lpArg+2);

      if (p < 0.0 || p >= 1.0)
	  {
		ValSetError(lpResult);
        return TRUE;
	  }
      if (x + r - 1 <= 0)
	  {
        ValSetError(lpResult);
		return TRUE;
	  }
	  ValSetLong(args, x+r-1);
	  ValSetLong(args+1, r-1);
      Calc_combin(lpResult, args, 2);
      if (lpResult->Status == CALC_VALUE_STATUS_ERROR)
		  return TRUE;
      pt1 = ValGetDouble(lpResult);
      pt2 = pow(p, r);
      pt3 = pow(1.0 - p, x);
	  ValSetDouble(lpResult, (double)(pt1 * pt2 * pt3));
      return TRUE; 
	}
  return TRUE;
}

BOOL CALLBACK Calc_sqrtpi(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;
  double dfResult;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    dfArg = ValGetDouble(lpArg);
	if (dfArg <= 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}
    dfResult = sqrt(dfArg*PI);
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
BOOL CALLBACK Calc_quotient(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double n;
  double d;
  double dfResult = 0.0;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    errno = 0;
    n = ValGetDouble(lpArg);
	d = ValGetDouble(lpArg+1);
	if (d == 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}
    dfResult = n/d;
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

BOOL CALLBACK Calc_devsq(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  BOOL bAnyEmpty = FALSE;
  double dfSum = 0.0;
  double dfMean = 0.0;
  double dfSumDevSq = 0.0;
  long lMean = 0;
  long lSum = 0;
  long lSumDevSq = 0;
  long lCount = 0;
  long lCol;
  long lRow;

  short args = nArgs;
  LPCALC_VALUE lpArgTemp = lpArg;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        dfSum += lpArg->Val.ValLong;
        lSum += lpArg->Val.ValLong;
        lCount += 1;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        dfSum += lpArg->Val.ValDouble;
        lCount += 1;
        bAnyDouble = TRUE;
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
                dfSum += value.Val.ValLong;
                lSum += value.Val.ValLong;
                lCount += 1;
                bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                dfSum += value.Val.ValDouble;
                lCount += 1;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                dfSum += value.Val.ValLong;
                lSum += value.Val.ValLong;
                lCount += 1;
                 bAnyLong = TRUE;
             }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                dfSum += value.Val.ValDouble;
                lCount += 1;
                bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #endif
    }
  }

  dfMean = dfSum / lCount;
  lMean = lSum / lCount;

  lpArg = lpArgTemp;
  nArgs = args;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        dfSumDevSq += (lpArg->Val.ValLong - lMean) * (lpArg->Val.ValLong - lMean);
 	    lSumDevSq += (lpArg->Val.ValLong - lMean) * (lpArg->Val.ValLong - lMean);
     }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        dfSumDevSq += (lpArg->Val.ValDouble - dfMean)*(lpArg->Val.ValDouble - dfMean);
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
				dfSumDevSq += (value.Val.ValLong - lMean)*(value.Val.ValLong - lMean);
 				lSumDevSq += (value.Val.ValLong - lMean)*(value.Val.ValLong - lMean);
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				dfSumDevSq += (value.Val.ValDouble - dfMean)*(value.Val.ValDouble - dfMean);
              }
            }
            ValFree(&value);
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
				dfSumDevSq += (value.Val.ValLong - lMean)*(value.Val.ValLong - lMean);
 				lSumDevSq += (value.Val.ValLong - lMean)*(value.Val.ValLong - lMean);
             }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				dfSumDevSq += (value.Val.ValDouble - dfMean)*(value.Val.ValDouble - dfMean);
              }
            }
            ValFree(&value);
          }
      }
      #endif

    }
  }


  if( bAnyError )
  {
    ValSetError(lpResult);
    return TRUE;
  }
  else if( bAnyDouble )
    ValSetDouble(lpResult, dfSumDevSq);
  else if( bAnyLong )
  {
    if( lSum == lSum / lCount * lCount )
      ValSetLong(lpResult, lSumDevSq );
    else
      ValSetDouble(lpResult, dfSumDevSq);
  }
  else
    ValSetEmpty(lpResult);
  return TRUE;
}




 static long gcd(long a, long b)
 {
	 long r;
     while (b != 0) 
      {
        r = a % b;
        a = b;
        b = r;
      }
      return a;
 }  

BOOL CALLBACK Calc_gcd(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyError = FALSE;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyEmpty = FALSE;
  long lGcd = 0;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
		if (lpArg->Val.ValLong < 0)
		{
			ValSetError(lpResult);
			return TRUE;
		}
        lGcd = gcd(lGcd, lpArg->Val.ValLong);
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
		if (lpArg->Val.ValDouble < 0.0)
		{
			ValSetError(lpResult);
			return TRUE;
		}
        lGcd = gcd(lGcd, (long)lpArg->Val.ValDouble);
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
 				if (lpArg->Val.ValLong < 0)
				{
					ValSetError(lpResult);
					return TRUE;
				}
               lGcd = gcd(lGcd, value.Val.ValLong);
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				if (lpArg->Val.ValDouble < 0.0)
				{
					ValSetError(lpResult);
					return TRUE;
				}
                lGcd = gcd(lGcd, (long)value.Val.ValDouble);
              }
            }
            ValFree(&value);
          }
      }
    }
  }

  if( bAnyError )
  {
    ValSetError(lpResult);
    return TRUE;
  }
  ValSetLong(lpResult, lGcd);
  return TRUE;
}
BOOL CALLBACK Calc_lcm(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyError = FALSE;
  long result = 1;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyEmpty = FALSE;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
		if (lpArg->Val.ValLong < 0)
		{
			ValSetError(lpResult);
			return TRUE;
		}
        if (result != 0)
        {
           result /= gcd(result, lpArg->Val.ValLong);
           result *= lpArg->Val.ValLong;
        }
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
		if (lpArg->Val.ValDouble < 0.0)
		{
			ValSetError(lpResult);
			return TRUE;
		}
        if (result != 0)
        {
           result /= gcd(result, (long)lpArg->Val.ValDouble);
           result *= (long)lpArg->Val.ValDouble;
        }
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
 				if (lpArg->Val.ValLong < 0)
				{
					ValSetError(lpResult);
					return TRUE;
				}
				if (result != 0)
				{
					result /= gcd(result, lpArg->Val.ValLong);
					result *= lpArg->Val.ValLong;
				}
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				if (lpArg->Val.ValDouble < 0.0)
				{
					ValSetError(lpResult);
					return TRUE;
				}
				if (result != 0)
				{
					result /= gcd(result, (long)lpArg->Val.ValDouble);
					result *= (long)lpArg->Val.ValDouble;
				}
              }
            }
            ValFree(&value);
          }
      }
    }
  }
  
  if( bAnyError )
  {
    ValSetError(lpResult);
    return TRUE;
  }
  ValSetLong(lpResult, result);
  return TRUE;
}

#if SS_V80 // 24477 -scl
BOOL CALLBACK Calc_count_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfSum = 0.0;
  long lSum = 0;
  long lCount = 0;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        lCount += 1;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        lCount += 1;
      }
      else if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            if( bIncludeSubtotals || !CalcIsSubtotal(lpSheet, lCol, lRow) )
            {
               lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
               if( CALC_VALUE_STATUS_OK == value.Status )
               {
                 if( CALC_VALUE_TYPE_LONG == value.Type )
                 {
                   lCount += 1;
                 }
                 else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
                 {
                   lCount += 1;
                 }
               }
               ValFree(&value);
            }
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                lCount += 1;
             }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                lCount += 1;
              }
            }
            ValFree(&value);
          }
      }
      #endif

    }
  }
  if( bAnyError )
  {
    ValSetError(lpResult);
    return TRUE;
  }
  else
    ValSetLong(lpResult, lCount);
  return TRUE;
}
BOOL CALLBACK Calc_count(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  return Calc_count_ex(lpResult, lpArg, nArgs, TRUE);
}
BOOL CALLBACK Calc_counta_ex(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs, BOOL bIncludeSubtotals)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfSum = 0.0;
  long lSum = 0;
  long lCount = 0;
  long lCol;
  long lRow;

  for( ; nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_EMPTY == lpArg->Status )
      ;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
	  if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            if( bIncludeSubtotals || !CalcIsSubtotal(lpSheet, lCol, lRow) )
            {
               lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
               if( CALC_VALUE_STATUS_EMPTY != value.Status )
				   lCount += 1;
            }
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_EMPTY != value.Status )
            {
				lCount += 1;
            }
            ValFree(&value);
          }
      }
      #endif
	  else
		  lCount += 1;
    }
	else 
		lCount += 1;
  }
  ValSetLong(lpResult, lCount);
  return TRUE;
}
BOOL CALLBACK Calc_counta(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
   return Calc_counta_ex(lpResult, lpArg, nArgs, TRUE);
}
#else // !SS_V80
BOOL CALLBACK Calc_count(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfSum = 0.0;
  long lSum = 0;
  long lCount = 0;
  long lCol;
  long lRow;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        lCount += 1;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        lCount += 1;
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
                lCount += 1;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                lCount += 1;
              }
            }
            ValFree(&value);
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
                lCount += 1;
             }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
                lCount += 1;
              }
            }
            ValFree(&value);
          }
      }
      #endif

    }
  }
  if( bAnyError )
  {
    ValSetError(lpResult);
    return TRUE;
  }
  else
    ValSetLong(lpResult, lCount);
  return TRUE;
}

BOOL CALLBACK Calc_counta(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfSum = 0.0;
  long lSum = 0;
  long lCount = 0;
  long lCol;
  long lRow;

  for( ; nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_EMPTY == lpArg->Status )
      ;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
	  if( CALC_VALUE_TYPE_RANGE == lpArg->Type )
      {
        lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
        for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_EMPTY != value.Status )
				lCount += 1;
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_EMPTY != value.Status )
            {
				lCount += 1;
            }
            ValFree(&value);
          }
      }
      #endif
	  else
		  lCount += 1;
    }
	else 
		lCount += 1;
  }
  ValSetLong(lpResult, lCount);
  return TRUE;
}
#endif // !SS_V80

BOOL CALLBACK Calc_sumproduct(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet1;
  CALC_VALUE value;
  long lCol1, lCol2;
  long lRow1, lRow2;
  long lRowCount, lColCount;
  double product;
  int i, r, c;
  double sum = 0.0;

  if( CheckAllCellOrRange(lpArg, nArgs) )
  {
    ValGetRange(lpArg, &lpSheet1, &lCol1, &lRow1, &lCol2, &lRow2);
	lRowCount = lRow2 - lRow1;
	lColCount = lCol2 - lCol1;
    
   for(i = 1 ; i < nArgs; i++)
   {
		ValGetRange(lpArg+i, &lpSheet1, &lCol1, &lRow1, &lCol2, &lRow2);
		if ((lRowCount != (lRow2 - lRow1)) || (lColCount != (lCol2-lCol1)))
		{
			ValSetError(lpResult);
			return TRUE;
		}
   }

   for (r = 0; r <= lRowCount; r++)
      {
        for (c = 0; c <= lColCount; c++)
        {
          product = 1.0;
          for (i = 0; i < nArgs; i++)
          {
			ValGetRange(lpArg+i, &lpSheet1, &lCol1, &lRow1, &lCol2, &lRow2);
            lpSheet1->lpfnGetData(lpSheet1->hSS, lCol1+c, lRow1+r, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
				  product *= (double)value.Val.ValLong;
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
				  product *= value.Val.ValDouble;
            }
            ValFree(&value);
		  }
  		sum += product;
        }
      }

  }
  else
  {
    ValSetError(lpResult);
	return TRUE;
  }

  ValSetDouble(lpResult, sum);

  return TRUE;
}

#if SS_V80
BOOL CALLBACK Calc_subtotal(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  long index = 0;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  BOOL bAnyEmpty = FALSE;

  if (nArgs > 0)
	{
		if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
			bAnyError = TRUE;
		else if( CALC_VALUE_STATUS_OK == lpArg->Status )
		{
			if( CALC_VALUE_TYPE_LONG == lpArg->Type )
				index = lpArg->Val.ValLong;
			else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
				index = (long)lpArg->Val.ValDouble;	
			else 
				bAnyError = TRUE;
		}
	lpArg++;
	nArgs--;
	}
  else 
	  bAnyError = TRUE;

  if( bAnyError )
    ValSetError(lpResult);
  else
  {
	  switch (index)
	  {
        case 1: 
        case 101:
            return Calc_average_ex(lpResult, lpArg, nArgs, FALSE);
        case 2:
        case 102:
            return Calc_count_ex(lpResult, lpArg, nArgs, FALSE);
        case 3: 
        case 103:
            return Calc_counta_ex(lpResult, lpArg, nArgs, FALSE);
        case 4: 
        case 104:
            return Calc_max_ex(lpResult, lpArg, nArgs, FALSE);
        case 5: 
        case 105:
            return Calc_min_ex(lpResult, lpArg, nArgs, FALSE);
        case 6: 
        case 106:
            return Calc_product_ex(lpResult, lpArg, nArgs, FALSE);
        case 7: 
        case 107:
            return Calc_stdev_ex(lpResult, lpArg, nArgs, FALSE);
        case 8: 
        case 108:
            return Calc_stdevp_ex(lpResult, lpArg, nArgs, FALSE);
        case 9: 
        case 109:
            return Calc_sum_ex(lpResult, lpArg, nArgs, FALSE);
        case 10: 
        case 110:
            return Calc_var_ex(lpResult, lpArg, nArgs, FALSE);
        case 11: 
        case 111:
            return Calc_varp_ex(lpResult, lpArg, nArgs, FALSE);
	  }
  }

  return TRUE;
}
#else
BOOL CALLBACK Calc_subtotal(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  long index = 0;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  BOOL bAnyEmpty = FALSE;

  if (nArgs > 0)
	{
		if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
			bAnyError = TRUE;
		else if( CALC_VALUE_STATUS_OK == lpArg->Status )
		{
			if( CALC_VALUE_TYPE_LONG == lpArg->Type )
				index = lpArg->Val.ValLong;
			else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
				index = (long)lpArg->Val.ValDouble;	
			else 
				bAnyError = TRUE;
		}
	lpArg++;
	nArgs--;
	}
  else 
	  bAnyError = TRUE;

  if( bAnyError )
    ValSetError(lpResult);
  else
  {
	  switch (index)
	  {
        case 1: 
        case 101:
            return Calc_average(lpResult, lpArg, nArgs);
        case 2:
        case 102:
            return Calc_count(lpResult, lpArg, nArgs);
        case 3: 
        case 103:
            return Calc_counta(lpResult, lpArg, nArgs);
        case 4: 
        case 104:
            return Calc_max(lpResult, lpArg, nArgs);
        case 5: 
        case 105:
            return Calc_min(lpResult, lpArg, nArgs);
        case 6: 
        case 106:
            return Calc_product(lpResult, lpArg, nArgs);
        case 7: 
        case 107:
            return Calc_stdev(lpResult, lpArg, nArgs);
        case 8: 
        case 108:
            return Calc_stdevp(lpResult, lpArg, nArgs);
        case 9: 
        case 109:
            return Calc_sum(lpResult, lpArg, nArgs);
        case 10: 
        case 110:
            return Calc_var(lpResult, lpArg, nArgs);
        case 11: 
        case 111:
            return Calc_varp(lpResult, lpArg, nArgs);
	  }
  }

  return TRUE;
}
#endif // !SS_V80

BOOL CALLBACK Calc_seriessum(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  double x;
  long n,m;
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  BOOL bAnyEmpty = FALSE;
  long lCol;
  long lRow;
  double sum = 0.0;
  int i = 0;

  if( CheckAllNumOrEmpty(lpArg, (short)(nArgs-1), &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    x = ValGetDouble(lpArg);
    n = ValGetLong(lpArg+1);
	m = ValGetLong(lpArg+2);
  }
  else
    ValSetError(lpResult);

  lpArg = lpArg+3;
  nArgs = 1;

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
				sum += (double)value.Val.ValLong * pow(x, n + i * m);
				i++;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				sum += value.Val.ValDouble * pow(x, n + i * m);
				i++;
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
    ValSetDouble(lpResult, sum);
  return TRUE;
}

BOOL CALLBACK Calc_multinomial(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfProduct = 1.0;
  long sum = 0;
  long lCol;
  long lRow;
  CALC_VALUE factArg[1];
  
  errno = 0;
  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {		 
         if (lpArg->Val.ValLong < 0 || 170 < lpArg->Val.ValLong)
		 {
            ValSetError(lpResult);
			return TRUE;
		 }
         sum += lpArg->Val.ValLong;
		 ValSetLong(factArg, lpArg->Val.ValLong);
		 if (Calc_fact(lpResult, factArg, 1))
			dfProduct *= ValGetDouble(lpResult);
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
         if (lpArg->Val.ValDouble < 0.0 || 170.0 < lpArg->Val.ValDouble)
		 {
            ValSetError(lpResult);
			return TRUE;
		 }
         sum += (long)lpArg->Val.ValDouble;
 		 ValSetLong(factArg, (long)lpArg->Val.ValDouble);
		 if (Calc_fact(lpResult, factArg, 1))
			dfProduct *= ValGetDouble(lpResult);
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
				if (value.Val.ValLong < 0 || 170 < value.Val.ValLong)
				{
					ValSetError(lpResult);
					return TRUE;
				}
				sum += value.Val.ValLong;
				ValSetLong(factArg, value.Val.ValLong);
				if (Calc_fact(lpResult, factArg, 1))
					dfProduct *= ValGetDouble(lpResult);
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				if (value.Val.ValDouble < 0.0 || 170.0 < value.Val.ValDouble)
				{
					ValSetError(lpResult);
					return TRUE;
				}
				sum += (long)value.Val.ValDouble;
				ValSetLong(factArg, (long)value.Val.ValDouble);
				if (Calc_fact(lpResult, factArg, 1))
					dfProduct *= ValGetDouble(lpResult);
              }
            }
            ValFree(&value);
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
				if (value.Val.ValLong < 0 || 170 < value.Val.ValLong)
				{
					ValSetError(lpResult);
					return TRUE;
				}
				sum += value.Val.ValLong;
				ValSetLong(factArg, value.Val.ValLong);
				if (Calc_fact(lpResult, factArg, 1))
					dfProduct *= ValGetDouble(lpResult);
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				if (value.Val.ValDouble < 0.0 || 170.0 < value.Val.ValDouble)
				{
					ValSetError(lpResult);
					return TRUE;
				}
				sum += (long)value.Val.ValDouble;
				ValSetLong(factArg, (long)value.Val.ValDouble);
				if (Calc_fact(lpResult, factArg, 1))
					dfProduct *= ValGetDouble(lpResult);
              }
            }
            ValFree(&value);
          }
      }
      #endif
    }
  }
  if( bAnyError )
    ValSetError(lpResult);
  else if (sum < 0 || 170 < sum)
  {
      ValSetError(lpResult);  
	  return TRUE;
  }
  else
  {
	  ValSetLong(factArg, sum);
	  if (Calc_fact(lpResult, factArg, 1))
		 ValSetDouble(lpResult, ValGetDouble(lpResult) / dfProduct);
	  else
		  ValSetError(lpResult);
  }
  return TRUE;
}

  static double Lanczos(double p) 
    {
      double x, tmp, ser, pt1;
  
      x = p;
      tmp = x + 5.5;
      tmp = tmp - (x + .5) * log(tmp);
      ser = 1.000000000190015 + 76.18009172947146 / ( p + 1.0 );
      ser -= 86.50532032941678 / ( p + 2.0 );
      ser += 24.01409824083091 / ( p + 3.0 );
      ser -= 1.231739572450155 / ( p + 4.0 );
      ser += .001208650973866179 / ( p + 5.0 );
      ser -= 5.395239384953E-06 / ( p + 6.0 );
      pt1 = log(2.506628274631001 * ser / x);
      return pt1 - tmp;
    }

   static double betacf(double a, double b, double x ) 
    {
      int m, m2;
      double retval, aa, c, d, del, qab, qam, qap;
      int ITMAX = 300;
      double FPMIN = 1.0E-50;
      double EPSILON = 1.0E-20;
  
      qab = a + b;
      qap = a + 1.0;
      qam = a - 1.0;
      c = 1.0;
      d = 1.0 - qab * x / qap;
      if (fabs(d) < FPMIN) d = FPMIN;
      d = 1.0 / d;
      retval = d;
      for (m=1; m<=ITMAX; m++) 
      {
        m2 = m + m;
        aa = (b-m) * m * x / ( (qam+m2) * (a+m2) );
        d = 1.0 + aa * d;
        if (fabs(d) < FPMIN) d = FPMIN;
        c = 1.0 + aa / c;
        if (fabs(c) < FPMIN) c = FPMIN;
        d = 1.0 / d;
        retval *= d * c;
        aa = 0.0 - (a+m) * (qab+m) * x / ( (a+m2) * (qap+m2) );
        d = 1.0 + aa * d;
        if (fabs(d) < FPMIN) d = FPMIN;
        c = 1.0 + aa / c;
        if (fabs(c) < FPMIN) c = FPMIN;
        d = 1.0 / d;
        del = d * c;
        retval *= del;
        if (fabs(del - 1.0) < EPSILON) break;
      }
      return (retval);
    }

BOOL CALLBACK Calc_betadist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double prob;
  double alpha;
  double beta;
  double bt,x,pt1,pt2,pt3,pt4,pt5;
  double a = 0;
  double b = 1;
  BOOL badBetacf = FALSE;
  double dfResult = 0.0;

  
	if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    prob = ValGetDouble(lpArg);
    alpha = ValGetDouble(lpArg+1);
    beta = ValGetDouble(lpArg+2);
	if (nArgs > 3)
		a = ValGetDouble(lpArg+3);
	if (nArgs > 4)
		b = ValGetDouble(lpArg+4);

      if (alpha <= 0.0 || beta <= 0.0)
 		{
		ValSetError(lpResult);
        return TRUE;
		}
	  if (prob < a || b < prob || a == b) 
 		{
		ValSetError(lpResult);
        return TRUE;
		}

      x = (prob - a) / (b - a);
      pt1 = Lanczos(alpha + beta);
      pt2 = Lanczos(alpha);
      pt3 = Lanczos(beta);
      pt4 = log(x);
      pt5 = log(1.0 - x);
      bt = exp(pt1 - pt2 - pt3 + alpha * pt4 + beta * pt5);
      if (x < (alpha + 1.0) / (alpha + beta + 2.0))
        dfResult = bt * betacf(alpha, beta, x) / alpha;
      else
        dfResult = 1.0 - bt * betacf(beta, alpha, 1.0 - x) / beta;
      if (badBetacf)
        dfResult = -1.0;
	}
	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}


BOOL CALLBACK Calc_fdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double x;
  double df1;
  double df2;
  double dfResult = 0.0;
  CALC_VALUE val[3];

  
	if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    x = ValGetDouble(lpArg);
    df1 = ValGetDouble(lpArg+1);
    df2 = ValGetDouble(lpArg+2);
    if(x < 0.0 || df1 < 1.0 || df1 >= pow(10.0,10.0) || df2 < 1.0 || df2 >= pow(10.0,10.0))
 		{
		ValSetError(lpResult);
        return TRUE;
		}
	
	  ValSetDouble(val, df1 * x /( df1 * x + df2));
      ValSetDouble(val+1, 0.5 * df1);
      ValSetDouble(val+2, 0.5 * df2);
	  Calc_betadist(lpResult, val, 3);
	  dfResult = ValGetDouble(lpResult);  

	}
	if( 0 == errno )
      ValSetDouble(lpResult, 1.0 - dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}

BOOL CALLBACK Calc_normsdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double w,x,y,z;
  double Z_MAX = 6.0;
  double dfResult = 0.0;

  
	if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    z = ValGetDouble(lpArg);
    if (z == 0.0) 
      {
        x = 0.0;
      } 
    else 
      {
        y = 0.5 * fabs(z);
        if (y >= (Z_MAX * 0.5)) 
        {
          x = 1.0;
        } 
        else if (y < 1.0) 
        {
          w = y * y;
          x = ((((((((0.000124818987 * w
            - 0.001075204047) * w + 0.005198775019) * w
            - 0.019198292004) * w + 0.059054035642) * w
            - 0.151968751364) * w + 0.319152932694) * w
            - 0.531923007300) * w + 0.797884560593) * y * 2.0;
        } 
        else 
        {
          y -= 2.0;
          x = (((((((((((((-0.000045255659 * y
            + 0.000152529290) * y - 0.000019538132) * y
            - 0.000676904986) * y + 0.001390604284) * y
            - 0.000794620820) * y - 0.002034254874) * y
            + 0.006549791214) * y - 0.010557625006) * y
            + 0.011630447319) * y - 0.009279453341) * y
            + 0.005353579108) * y - 0.002141268741) * y
            + 0.000535310849) * y + 0.999936657524;
        }
      }
      dfResult =  z > 0.0 ? ((x + 1.0) * 0.5) : ((1.0 - x) * 0.5);
	}
	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}
BOOL CALLBACK Calc_normdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double x;
  double mean;
  double stdev;
  BOOL cum;
  double dfResult = 0.0;
  CALC_VALUE val[1];

  
	if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    x = ValGetDouble(lpArg);
    mean = ValGetDouble(lpArg+1);
    stdev = ValGetDouble(lpArg+2);
	cum = ValGetBool(lpArg+3);

    if(stdev <= 0.0)
 		{
		ValSetError(lpResult);
        return TRUE;
		}
	
      if (cum)
      {
		ValSetDouble(val, (x-mean)/stdev);
		Calc_normsdist(lpResult, val, 1);
		dfResult = ValGetDouble(lpResult);
      }
      else
      {
        dfResult = (exp(-((x - mean) * (x - mean)) / (2.0 * stdev * stdev)) / (sqrt(2 * PI) * stdev));
      }

	}
	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}

BOOL CALLBACK Calc_normsinv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double prob;
  double dfResult = 0.0;
  CALC_VALUE val[3];

  
	if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    prob = ValGetDouble(lpArg);

      if (prob < 0.0 || 1.0 < prob)
	  {
		ValSetError(lpResult);
		return TRUE;
	  }

      ValSetDouble(val, prob);
	  ValSetDouble(val+1,0.0);
	  ValSetDouble(val+2, 1.0);
	  Calc_norminv(lpResult, val, 3);
	  dfResult = ValGetDouble(lpResult);	
	}
	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}
BOOL CALLBACK Calc_norminv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double prob;
  double mean;
  double stdev;
  double q;
  double r;
  double val;
  double x;
  double denom;
  double dfResult;
  CALC_VALUE value[4];

  
	if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    prob = ValGetDouble(lpArg);
    mean = ValGetDouble(lpArg+1);
    stdev = ValGetDouble(lpArg+2);
    if (prob < 0.0 || 1.0 < prob) 
	{
		ValSetError(lpResult);
		return TRUE;
	}
    if (stdev <= 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}
      q = prob - 0.5;
      if (fabs(q) <= 0.42) 
      {
        r = q * q;
        val = q * (((-25.44106049637 * r + 41.39119773534) * r
          - 18.61500062529) * r + 2.50662823884)
          / ((((3.13082909833 * r - 21.06224101826) * r
          + 23.08336743743) * r + -8.47351093090) * r + 1.0);
      }
      else 
      {
  
        r = prob;
        if (q > 0.0)
          r = 1.0 - prob;
        if(r > 2.2204460492503131e-016) 
        {
          r = sqrt(-log(r));
          val = (((2.32121276858 * r + 4.85014127135) * r
            - 2.29796479134) * r - 2.78718931138)
            / ((1.63706781897 * r + 3.54388924762) * r + 1.0);
          if (q < 0.0)
            val = -val;
        }
        else if(r > 1e-300) 
        {
          val = -2.0 * log(prob);
          r = log(6.283185307179586476925286766552 * val);
          r = r/val + (2.0 - r)/(val * val)
            + (-14.0 + 6.0 * r - r * r)/(2.0 * val * val * val);
          val = sqrt(val * (1 - r));
          if(q < 0.0)
            val = -val;         
		  ValSetDouble(lpResult, val);
		  return TRUE;
        }
        else 
        {
          if(q < 0.0) 
          {
		  ValSetDouble(lpResult, DBL_MIN);
		  return TRUE;
          }
          else 
          {
		  ValSetDouble(lpResult, DBL_MAX);
		  return TRUE;
          }
        }
      }

	  ValSetDouble(value, val);
	  ValSetDouble(value+1, 0.0);
	  ValSetDouble(value+2, 1.0);
	  ValSetBool(value+3, TRUE);
	  Calc_normdist(lpResult, value, 4);

      x = (val - 0.0) / 1.0;
      denom = 0.398942280401432677939946059934 * exp(-0.5 * x * x) / 1.0;

      val = val - (ValGetDouble(lpResult) - prob) / denom;
      dfResult =  mean + stdev * val;

	}
	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}

BOOL CALLBACK Calc_tinv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double val;
  int ndf;
  double dfResult = 0.0;
  double a, b, c, d, prob, P, q, x, y;
  int neg;
  double p, p2;
  CALC_VALUE value[3];
  double eps = 1.0e-12;

  
	if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    val = ValGetDouble(lpArg);
	ndf = ValGetLong(lpArg+1);

    if (val < 0.0 || 1.0 < val)
	{
		ValSetError(lpResult);
        return TRUE;
	}
    if (ndf < 1 || ndf > pow(10,10))
	{
		ValSetError(lpResult);
        return TRUE;
	}

    p = val/2;
    p2 = p;

    if (ndf > 1.0e20)
    {
        ValSetDouble(value, p);
		Calc_normsinv(lpResult, value, 1);
		return TRUE;
    }
    if(p2 < 0.5)
    {
        neg = 0;
        P = 2.0 * p2;
    }
    else
    {
        neg = 1; P = 2.0 * (1.0-p2);
    }
    if (fabs(ndf - 2.0) < eps)
    {
        if (P > 0.0)
          q = sqrt(2.0 / (P * (2.0 - P)) - 2.0);
        else
          q = DBL_MAX;
    }
    else if ((double)ndf < 1.0 + eps)
    {
        if (P > 0.0)
        {
          prob = (P+1.0) * 1.57079632679489661923;
          q = - tan(prob);
        }
        else
        {
          q = DBL_MAX;
        }
    }
    else
    {
        a = 1.0 / (ndf - 0.5);
        b = 48.0 / (a * a);
        c = ((20700.0 * a / b - 98.0) * a - 16.0) * a + 96.36;
        d = ((94.5 / (b + c) - 3.0) / b + 1.0) * sqrt(a * 1.57079632679489661923) * ndf;
        y = pow(d * P, 2.0 / ndf);
        if (y > 0.05 + a)
        {
		  ValSetDouble(value, 0.5*P);
		  Calc_normsinv(lpResult, value, 1);
          x = ValGetDouble(lpResult);
          y = x * x;
          if (ndf < 5.0)
            c = c + 0.3 * (ndf - 4.5) * (x + 0.6);
          c = (((0.05 * d * x - 5.0) * x - 7.0) * x - 2.0) * x + b + c;
          y = (((((0.4 * y + 6.3) * y + 36) * y + 94.5) / c - y - 3.0) / b + 1.0) * x;
          y = a * y * y;
          if (y > 0.002)
            y = exp(y) - 1.0;
          else
          {
            y = 0.5 * y * y + y;
          }
        } 
        else
        {
          y = ((1.0 / (((ndf + 6.0) / (ndf * y) - 0.089 * d - 0.822)
            * (ndf + 2.0) * 3.0) + 0.5 / (ndf + 4.0))
            * y - 1.0) * (ndf + 1.0) / (ndf + 2.0) + 1.0 / y;
        }
        q = sqrt(ndf * y);
    }
      if (neg != 0.0)
        q = -q;
	
	  dfResult = q;
	
	}
	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}
BOOL CALLBACK Calc_tdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double x;
  long df;
  long tails;
  double f,a,b,im2,ioe,s,c,ks,fk;
  double dfResult;
  int i;
  
	if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    x = ValGetDouble(lpArg);
    df = ValGetLong(lpArg+1);
    tails = ValGetLong(lpArg+2);
    if (df < 1)
	{
		ValSetError(lpResult);
        return TRUE;
	}
    if (tails != 1 && tails != 2)
	{
		ValSetError(lpResult);
        return TRUE;
	}
    f = df;
    a = x/sqrt(f);
    b = f/(f + (x*x));
    im2 = f - 2.0;
    ioe = (long)f % 2;
    s = 1.0;
    c = 1.0;
    f = 1.0;
    ks = 2.0 + ioe;
    fk = ks;
    if (im2 >= 2.0)
      {
        for (i=(int)ks; i <= im2; i=i+2)
        {
          c= c*b*(fk-1.0)/fk;
          s= s+c;
          if (s == f)
            break;
          f=s;
          fk = fk+2;
        }
      }
    if (ioe != 1.0)
        dfResult =  tails * (1.0 -(0.5 + 0.5 * a * sqrt(b) * s));
	else
	{
    if (df == 1.0)
        s = 0.0;
    dfResult =  tails * (1.0 - (0.5 + (a * b * s + atan(a)) * 0.3183098862));
	}

	}
	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}

BOOL CALLBACK Calc_skew(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  BOOL bAnyDouble = FALSE;
  BOOL bAnyLong = FALSE;
  BOOL bAnyError = FALSE;
  double dfSum = 0.0;
  double dfSumx2 = 0.0;
  double sumskew = 0.0;
  double dfMeanX;
  double stdev;
  long lCount = 0;
  long lCol;
  long lRow;

  short args = nArgs;
  LPCALC_VALUE lpArgTemp = lpArg;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
        dfSum += lpArg->Val.ValLong;
		dfSumx2 += lpArg->Val.ValLong * lpArg->Val.ValLong;
        lCount += 1;
        bAnyLong = TRUE;
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
        dfSum += lpArg->Val.ValDouble;
		dfSumx2 += lpArg->Val.ValDouble * lpArg->Val.ValDouble;
        lCount += 1;
        bAnyDouble = TRUE;
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
				dfSum += value.Val.ValLong;
				dfSumx2 += value.Val.ValLong * value.Val.ValLong;
				lCount += 1;
				bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				dfSum += value.Val.ValDouble;
				dfSumx2 += value.Val.ValDouble * value.Val.ValDouble;
				lCount += 1;
				bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
				dfSum += value.Val.ValLong;
				dfSumx2 += value.Val.ValLong * value.Val.ValLong;
				lCount += 1;
				bAnyLong = TRUE;
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				dfSum += value.Val.ValDouble;
				dfSumx2 += value.Val.ValDouble * value.Val.ValDouble;
				lCount += 1;
				bAnyDouble = TRUE;
              }
            }
            ValFree(&value);
          }
      }
      #endif

    }
  }

  if (lCount <= 2)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }

  dfMeanX = dfSum / (double)lCount;
  stdev = sqrt(((double)lCount * dfSumx2 - dfSum * dfSum) / ((double)lCount * ((double)lCount - 1.0)));

  if (stdev == 0.0)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }

  lpArg = lpArgTemp;
  nArgs = args;

  for( ; !bAnyError && nArgs > 0; nArgs--, lpArg++ )
  {
    if( CALC_VALUE_STATUS_ERROR == lpArg->Status )
      bAnyError = TRUE;
    else if( CALC_VALUE_STATUS_OK == lpArg->Status )
    {
      if( CALC_VALUE_TYPE_LONG == lpArg->Type )
      {
              sumskew += pow(((double)lpArg->Val.ValLong - dfMeanX) / stdev, 3.0);
      }
      else if( CALC_VALUE_TYPE_DOUBLE == lpArg->Type )
      {
              sumskew += pow((lpArg->Val.ValDouble - dfMeanX) / stdev, 3.0);
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
				sumskew += pow(((double)value.Val.ValLong - dfMeanX) / stdev, 3.0);
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				sumskew += pow((value.Val.ValDouble - dfMeanX) / stdev, 3.0);
              }
            }
            ValFree(&value);
          }
      }
      #if defined(SS_DDE)
      else if( CALC_VALUE_TYPE_DDE == lpArg->Type )
      {
        long lMaxCol = DdeMaxCol(lpArg->Val.ValDde.hDde);
        long lMaxRow = DdeMaxRow(lpArg->Val.ValDde.hDde);
        for( lCol = 0; lCol < lMaxCol; lCol++ )
          for( lRow = 0; lRow < lMaxRow; lRow++ )
          {
            DdeGetValue(lpArg->Val.ValDde.hDde, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
              if( CALC_VALUE_TYPE_LONG == value.Type )
              {
				sumskew += pow(((double)value.Val.ValLong - dfMeanX) / stdev, 3.0);
              }
              else if( CALC_VALUE_TYPE_DOUBLE == value.Type )
              {
				sumskew += pow((value.Val.ValDouble - dfMeanX) / stdev, 3.0);
              }
            }
            ValFree(&value);
          }
      }
      #endif

    }
  }


  if( bAnyError )
  {
    ValSetError(lpResult);
    return TRUE;
  }

  ValSetDouble(lpResult, ((double)lCount * sumskew) / (((double)lCount - 1.0) * ((double)lCount - 2.0)));
  return TRUE;
}

BOOL CALLBACK Calc_poisson(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  BOOL cum;
  double mean;
  int x; 
  double dfResult = 0.0;
  int i;
  CALC_VALUE factArg[1];
  
	if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    x = ValGetLong(lpArg);
    mean = ValGetDouble(lpArg+1);
    cum = ValGetBool(lpArg+2);

    if (x <= 0 || mean <= 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}
    if (cum)
      {
        for (i = 0; i <= x; i++)
        {
		  ValSetLong(factArg, i);
		  Calc_fact(lpResult, factArg, 1);
          dfResult += (exp(-mean) * pow(mean, i)) / ValGetDouble(lpResult);
        }
      }
    else
      {
		ValSetLong(factArg, x);
		Calc_fact(lpResult, factArg, 1);
        dfResult = (exp(-mean) * pow(mean, x)) / ValGetDouble(lpResult);
      }


	}
	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}

BOOL CALLBACK Calc_weibull(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double x, alpha, beta;
  BOOL cum;
  double dfResult;
  
	if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    x = ValGetDouble(lpArg);
    alpha = ValGetDouble(lpArg+1);
    beta = ValGetDouble(lpArg+2);
	cum = ValGetBool(lpArg+3);

    if (x < 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}
    if (alpha <= 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}
    if (beta <= 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}
    if (cum)
       dfResult = 1.0 - exp(-pow((x / beta), alpha));
    else
       dfResult = (alpha / pow(beta, alpha)) * pow(x, alpha - 1.0) * exp(-pow((x / beta), alpha));
	}
	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}


static double gamma(double x)
    {
      int i,k,m;
      double ga,gr,z;
      double r = 1.0;

      double g[] = {
                     1.0,
                     0.5772156649015329,
                     -0.6558780715202538,
                     -0.420026350340952e-1,
                     0.1665386113822915,
                     -0.421977345555443e-1,
                     -0.9621971527877e-2,
                     0.7218943246663e-2,
                     -0.11651675918591e-2,
                     -0.2152416741149e-3,
                     0.1280502823882e-3,
                     -0.201348547807e-4,
                     -0.12504934821e-5,
                     0.1133027232e-5,
                     -0.2056338417e-6,
                     0.6116095e-8,
                     0.50020075e-8,
                     -0.11812746e-8,
                     0.1043427e-9,
                     0.77823e-11,
                     -0.36968e-11,
                     0.51e-12,
                     -0.206e-13,
                     -0.54e-14,
                     0.14e-14};

      if (x > 171.0) return 1e308;    // This value is an overflow flag.
      if (x == (int)x) 
      {
        if (x > 0.0) 
        {
          ga = 1.0;               // use factorial
          for (i=2;i<x;i++) 
          {
            ga *= i;
          }
        }
        else
          ga = 1e308;
      }
      else 
      {
        if (fabs(x) > 1.0) 
        {
          z = fabs(x);
          m = (int)z;
          r = 1.0;
          for (k=1;k<=m;k++) 
          {
            r *= (z-k);
          }
          z -= m;
        }
        else
          z = x;
        gr = g[24];
        for (k=23;k>=0;k--) 
        {
          gr = gr*z+g[k];
        }
        ga = 1.0/(gr*z);
        if (fabs(x) > 1.0) 
        {
          ga *= r;
          if (x < 0.0) 
          {
            ga = -PI/(x*ga*sin(PI*x));
          }
        }
      }
      return ga;
}

/*static*/ double lgamma(double x)
   {
     double x0,x2,xp,gl,gl0;
     int k;
     int n = 0;
     double a[] = 
    {
      8.333333333333333e-02,
      -2.777777777777778e-03,
      7.936507936507937e-04,
      -5.952380952380952e-04,
      8.417508417508418e-04,
      -1.917526917526918e-03,
      6.410256410256410e-03,
      -2.955065359477124e-02,
      1.796443723688307e-01,
      -1.39243221690590};
    
     x0 = x;
     if (x <= 0.0) return 1e308;
     else if ((x == 1.0) || (x == 2.0)) return 0.0;
     else if (x <= 7.0) 
     {
       n = (int)(7-x);
       x0 = x+n;
     }
     x2 = 1.0/(x0*x0);
     xp = 2.0*PI;
     gl0 = a[9];
     for (k=8;k>=0;k--) 
     {
       gl0 = gl0*x2 + a[k];
     }
     gl = gl0/x0+0.5*log(xp)+(x0-0.5)*log(x0)-x0;
     if (x <= 7.0) 
     {
       for (k=1;k<=n;k++) 
       {
         gl -= log(x0-1.0);
         x0 -= 1.0;
       }
     }
     return gl;
}

BOOL CALLBACK Calc_gammaln(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double x;
  double dfResult;
  
	if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    x = ValGetDouble(lpArg);
    if (x <= 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}

	dfResult = lgamma(x);
	}
	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}

BOOL CALLBACK Calc_gammadist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double x, alpha, beta;
  BOOL cum;
  double dfResult;
  CALC_VALUE gammaLnArgs[1];
  CALC_VALUE normdistArgs[4];
  
	if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    x = ValGetDouble(lpArg);
	alpha = ValGetDouble(lpArg+1);
	beta = ValGetDouble(lpArg+2);
	cum = ValGetBool(lpArg+3);

    if (x < 0.0 || alpha <= 0.0 || beta <= 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}

    if (cum == FALSE)
      {
        double pt1 = pow(beta,alpha);
        //if (double.IsNaN(pt1) || double.IsInfinity(pt1))
          //return CalcError.DivideByZero;
        double pt2 = 1.0/(pt1*gamma(alpha));
        double pt3 = pow(x,alpha-1.0);
        double pt4 = exp(-(x/beta));
        double pt5 = pt3*pt4;
        dfResult =  pt2*pt5;
    
      }
      else
      {
        double pn1, pn2, pn3, pn4, pn5, pn6, arg, c, rn, a, b, an;
        double sum;
        double third = 1.0 / 3.0;
        double xbig = 1.0e+8;
        double oflo = 1.0e+37;
        double plimit = 1000.0e0;
        double elimit = -88.0e0;
        x = x / beta;
        if (x <= 0.0)
		{
			ValSetError(lpResult);
			return TRUE;
		}
        if (alpha > plimit) 
        {
          pn1 = sqrt(alpha) * 3.0 * (pow((x/alpha), third) + 1.0 / (alpha * 9.0) - 1.0);
		  ValSetDouble(normdistArgs, pn1);
		  ValSetDouble(normdistArgs+1, 0.0);
		  ValSetDouble(normdistArgs+2, 1.0);
		  ValSetBool(normdistArgs+3, TRUE);
		  Calc_normdist(lpResult, normdistArgs, 4);
		  return TRUE;
        }
        if (x > xbig)
		{
		   ValSetDouble(lpResult, 1.0);
		   return TRUE;
		}
        if (x <= 1.0 || x < alpha) 
        {
		  ValSetDouble(gammaLnArgs, alpha + 1.0);
		  Calc_gammaln(lpResult, gammaLnArgs, 1);
          arg = alpha * log(x) - x - ValGetDouble(lpResult);
          c = 1.0;
          sum = 1.0;
          a = alpha;
          do 
          {
            a = a + 1.0;
            c = c * x / a;
            sum = sum + c;
          } while (c > 2.2204460492503131e-016);
          arg = arg + log(sum);
          sum = 0.0;
          if (arg >= elimit)
            sum = exp(arg);
        } 
        else 
        {
		  ValSetDouble(gammaLnArgs, alpha);
		  Calc_gammaln(lpResult, gammaLnArgs, 1);
          arg = alpha * log(x) - x - ValGetDouble(lpResult);
          a = 1.0 - alpha;
          b = a + x + 1.0;
          c = 0.0;
          pn1 = 1.0;
          pn2 = x;
          pn3 = x + 1.0;
          pn4 = x * b;
          sum = pn3 / pn4;
          for (;;) 
          {
            a = a + 1.0;
            b = b + 2.0;
            c = c + 1.0;
            an = a * c;
            pn5 = b * pn3 - an * pn1;
            pn6 = b * pn4 - an * pn2;
            if (fabs(pn6) > 0.0) 
            {
              rn = pn5 / pn6;
              if (fabs(sum - rn) <= min(2.2204460492503131e-016, 2.2204460492503131e-016 * rn))
                break;
              sum = rn;
            }
            pn1 = pn3;
            pn2 = pn4;
            pn3 = pn5;
            pn4 = pn6;
            if (fabs(pn5) >= oflo) 
            {
              pn1 = pn1 / oflo;
              pn2 = pn2 / oflo;
              pn3 = pn3 / oflo;
              pn4 = pn4 / oflo;
            }
          }
          arg = arg + log(sum);
          sum = 1.0;
          if (arg >= elimit)
            sum = 1.0 - exp(arg);
        }
        dfResult = sum;
      }
	
	
	}
	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}

BOOL CALLBACK Calc_gammainv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double prob, alpha, beta;
  double dfResult;
  CALC_VALUE gammaLnArgs[1];
  CALC_VALUE norminvArgs[3];
  CALC_VALUE gammaDistArgs[4];
  double a, b, c, ch, g, p1, v;
  double p2, q, s1, s2, s3, s4, s5, s6, t, x;
  int i;  
  double C7 = 4.67;
  double C8 = 6.66;
  double C9 = 6.73;
  double C10 = 13.32;
  double C11 = 60.0;
  double C12 = 70.0;
  double C13 = 84.0;
  double C14 = 105.0;
  double C15 = 120.0;
  double C16 = 127.0;
  double C17 = 140.0;
  double C18 = 1175.0;
  double C19 = 210.0;
  double C20 = 252.0;
  double C21 = 2264.0;
  double C22 = 294.0;
  double C23 = 346.0;
  double C24 = 420.0;
  double C25 = 462.0;
  double C26 = 606.0;
  double C27 = 672.0;
  double C28 = 707.0;
  double C29 = 735.0;
  double C30 = 889.0;
  double C31 = 932.0;
  double C32 = 966.0;
  double C33 = 1141.0;
  double C34 = 1182.0;
  double C35 = 1278.0;
  double C36 = 1740.0;
  double C37 = 2520.0;
  double C38 = 5040.0;
  double EPS0 = 5.0e-7;
  double EPS1 = 1.0e-2;
  double EPS2 = 5.0e-7;
  double MAXIT = 20.0;
  double pMIN = 0.000002;
  double pMAX = 0.999998;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
	{
    errno = 0;
    prob = ValGetDouble(lpArg);
	alpha = ValGetDouble(lpArg+1);
	beta = ValGetDouble(lpArg+2);


    if (prob < 0.0 || 1.0 < prob || alpha <= 0.0 || beta <= 0.0) 
	{
		ValSetError(lpResult);
		return TRUE;
	}
    if (prob < pMIN)
	  {
		ValSetDouble(lpResult, 0.0);
        return TRUE;
	  }
    if (prob > pMAX)
	  {
		ValSetDouble(lpResult, DBL_MAX);
        return TRUE;
	  }
    v = 2.0*alpha;
    c = alpha-1.0;
	ValSetDouble(gammaLnArgs, alpha);
	Calc_gammaln(lpResult, gammaLnArgs, 1);
    g = ValGetDouble(lpResult);
    if(v < (-1.24)*log(prob)) 
      {
        ch = pow(prob*alpha*exp(g+alpha*0.69314718055994530942), 1.0/alpha);
        if(ch < EPS0) 
        {
			//should return double.nan
          ValSetError(lpResult);
		  return TRUE;
        }
      } 
    else if(v > 0.32) 
      {
		ValSetDouble(norminvArgs,prob);
		ValSetDouble(norminvArgs+1, 0.0);
		ValSetDouble(norminvArgs+2, 1.0);
		Calc_norminv(lpResult, norminvArgs, 3);
		x = ValGetDouble(lpResult);
        p1 = 0.222222/v;
        ch = v*pow(x*sqrt(p1)+1.0-p1, 3.0);
        if( ch > 2.2*v + 6 )
          ch = -2.0*(log(1.0-prob) - c*log(0.5*ch) + g);
      } 
    else 
      {
        ch = 0.4;
        a = log(1.0-prob) + g + c*0.69314718055994530942;
        do 
        {
          q = ch;
          p1 = 1.0+ch*(C7+ch);
          p2 = ch*(C9+ch*(C8+ch));
          t = -0.5 +(C7+2*ch)/p1 - (C9+ch*(C10+3.0*ch))/p2;
          ch -= (1.0- exp(a+0.5*ch)*p2/p1)/t;
        } while(fabs(q/ch - 1.0) > EPS1);
      }
    for( i=1 ; i <= MAXIT ; i++ ) 
      {
        q = ch;
        p1 = 0.5*ch;
		ValSetDouble(gammaDistArgs, p1);
		ValSetDouble(gammaDistArgs+1, alpha);
		ValSetDouble(gammaDistArgs+2, 1.0);
		ValSetBool(gammaDistArgs+3, TRUE);
		Calc_gammadist(lpResult, gammaDistArgs, 4);
        p2 = prob - ValGetDouble(lpResult);
        t = p2*exp(alpha*0.69314718055994530942+g+p1-c*log(ch));
        b = t/ch;
        a = 0.5*t-b*c;
        s1 = (C19+a*(C17+a*(C14+a*(C13+a*(C12+C11*a)))))/C24;
        s2 = (C24+a*(C29+a*(C32+a*(C33+C35*a))))/C37;
        s3 = (C19+a*(C25+a*(C28+C31*a)))/C37;
        s4 = (C20+a*(C27+C34*a)+c*(C22+a*(C30+C36*a)))/C38;
        s5 = (C13+C21*a+c*(C18+C26*a))/C37;
        s6 = (C15+c*(C23+C16*c))/C38;
        ch = ch+t*(1+0.5*t*s1-b*c*(s1-b*(s2-b*(s3-b*(s4-b*(s5-b*s6))))));
        if(fabs(q/ch-1) > EPS2)
		{
			ValSetDouble(lpResult, 0.5*beta*ch);
			return TRUE;
		}
      }
      dfResult = 0.5*beta*ch;
	
	}
	if( 0 == errno )
      ValSetDouble(lpResult, dfResult);
    else
      ValSetError(lpResult);
    errno = 0;
  return TRUE;
}
BOOL CALLBACK Calc_factdouble(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  long lArg;
  long lResult = 1;
  double dfResult = 1.0;
  int i;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    lArg = ValGetLong(lpArg);
	if (lArg < 0 || lArg > 300)
	{
		ValSetError(lpResult);
		return TRUE;
	}
    if( 0 <= lArg && lArg <= 12 )
    {
      for( i = lArg; i > 1; i-=2 )
        lResult *= i;
      ValSetLong(lpResult, lResult);
    }
    else if( 12 < lArg )
    {
      for( i = lArg; i > 1; i-=2 )
        dfResult *= i;
      ValSetDouble(lpResult, dfResult);
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}


BOOL CALLBACK Calc_fisher(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    dfArg = ValGetDouble(lpArg);
    if (dfArg <= -1.0 || 1.0 <= dfArg) 
	{
		ValSetError(lpResult);
		return TRUE;
	}
    ValSetDouble(lpResult, log((1.0 + dfArg) / (1.0 - dfArg)) / 2.0);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

BOOL CALLBACK Calc_fisherinv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfArg, pt1, pt2;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    dfArg = ValGetDouble(lpArg);
    pt1  = exp(2.0 * dfArg) - 1.0;
    pt2 =  exp(2.0 * dfArg) + 1.0;

    ValSetDouble(lpResult, pt1/pt2);
  }
  else
    ValSetError(lpResult);
  return TRUE;

}

BOOL CALLBACK Calc_lognormdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double x, mean, stdev;
  CALC_VALUE val[1];

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    x = ValGetDouble(lpArg);
	mean = ValGetDouble(lpArg+1);
	stdev = ValGetDouble(lpArg+2);
     
	if (x <= 0.0 || stdev <= 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}
	ValSetDouble(val, (log(x) - mean) / stdev);
	Calc_normsdist(lpResult, val, 1);
		
  }
  else
	ValSetError(lpResult);
  return TRUE;

}

BOOL CALLBACK Calc_loginv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double prob, mean, stdev, dfResult; 
  CALC_VALUE val[1];

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    prob = ValGetDouble(lpArg);
	mean = ValGetDouble(lpArg+1);
	stdev = ValGetDouble(lpArg+2);
     
    if (stdev <= 0.0 || prob < 0.0 || prob > 1.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}
	ValSetDouble(val, prob);
	Calc_normsinv(lpResult, val, 1);
	dfResult = exp(mean + stdev * ValGetDouble(lpResult));
		
  }
  else
	ValSetError(lpResult);
  ValSetDouble(lpResult, dfResult);
  return TRUE;

}

   static double d1mach (int i)
    {
      switch (i) 
      {
        case 1: return 2.2250738585072014e-308;
        case 2: return 1.7976931348623158e+308;
        case 3: return pow (2.0, -53.0);
        case 4: return pow (2.0, 1.0 - 53.0);
        case 5: return log10 (2.0);
        default:return 0.0;
      }
    }    

    static int chebyshev_init(double dos[], int nos, double eta)
    {
      int i, ii;
      double err;
    
      if (nos < 1)
        return 0;
    
      err = 0.0;
      i = 0;
      for (ii=1; ii<=nos; ii++) 
      {
        i = nos - ii;
        err += fabs(dos[i]);
        if (err > eta) 
        {
          return i;
        }
      }
      return i;
    }

    static double chebyshev_eval(double x, double a[], int n)
    {
      double b0, b1, b2, twox;
      int i;
      if (n < 1 || n > 1000) 
        return -DBL_MAX;
      if (x < -1.1 || x > 1.1) 
        return -DBL_MAX;
      twox = x * 2;
      b2 = b1 = 0;
      b0 = 0;
      for (i = 1; i <= n; i++) 
      {
        b2 = b1;
        b1 = b0;
        b0 = twox * b1 - b2 + a[n - i];
      }
      return (b0 - b2) * 0.5;
    }

    static double lgammacor(double x)
    {
      double tmp;
      int nalgm = 0;
      double xbig = 0.0;
      double xmax = 0.0;
      double algmcs[15];
      algmcs[0] = +.1666389480451863247205729650822e+0;
      algmcs[1] = -.1384948176067563840732986059135e-4;
      algmcs[2] = +.9810825646924729426157171547487e-8;
      algmcs[3] = -.1809129475572494194263306266719e-10;
      algmcs[4] = +.6221098041892605227126015543416e-13;
      algmcs[5] = -.3399615005417721944303330599666e-15;
      algmcs[6] = +.2683181998482698748957538846666e-17;
      algmcs[7] = -.2868042435334643284144622399999e-19;
      algmcs[8] = +.3962837061046434803679306666666e-21;
      algmcs[9] = -.6831888753985766870111999999999e-23;
      algmcs[10] = +.1429227355942498147573333333333e-24;
      algmcs[11] = -.3547598158101070547199999999999e-26;
      algmcs[12] = +.1025680058010470912000000000000e-27;
      algmcs[13] = -.3401102254316748799999999999999e-29;
      algmcs[14] = +.1276642195630062933333333333333e-30;
      if (nalgm == 0) 
      {
        nalgm = chebyshev_init(algmcs, 15, d1mach(3));
        xbig = 1.0 / sqrt(d1mach(3));
        xmax = exp(min(log(d1mach(2) / 12.0), -log(12.0 * d1mach(1))));
      }
      if (x < 10.0)
        return -DBL_MAX;
      else if (x >= xmax) 
        return 2.2204460492503131e-016*2.2204460492503131e-016;
      else if (x < xbig) 
      {
        tmp = 10.0 / x;
        return chebyshev_eval(tmp * tmp * 2.0 - 1.0, algmcs, nalgm) / x;
      }
      else return 1.0 / (x * 12.0);
    }

    static double logrelerr(double x)
    {
      double alnrcs[43];
      int nlnrel = 0;
      double xmin = 0.0;
      alnrcs[0] = +.10378693562743769800686267719098e+1;
      alnrcs[1] = -.13364301504908918098766041553133e+0;
      alnrcs[2] = +.19408249135520563357926199374750e-1;
      alnrcs[3] = -.30107551127535777690376537776592e-2;
      alnrcs[4] = +.48694614797154850090456366509137e-3;
      alnrcs[5] = -.81054881893175356066809943008622e-4;
      alnrcs[6] = +.13778847799559524782938251496059e-4;
      alnrcs[7] = -.23802210894358970251369992914935e-5;
      alnrcs[8] = +.41640416213865183476391859901989e-6;
      alnrcs[9] = -.73595828378075994984266837031998e-7;
      alnrcs[10] = +.13117611876241674949152294345011e-7;
      alnrcs[11] = -.23546709317742425136696092330175e-8;
      alnrcs[12] = +.42522773276034997775638052962567e-9;
      alnrcs[13] = -.77190894134840796826108107493300e-10;
      alnrcs[14] = +.14075746481359069909215356472191e-10;
      alnrcs[15] = -.25769072058024680627537078627584e-11;
      alnrcs[16] = +.47342406666294421849154395005938e-12;
      alnrcs[17] = -.87249012674742641745301263292675e-13;
      alnrcs[18] = +.16124614902740551465739833119115e-13;
      alnrcs[19] = -.29875652015665773006710792416815e-14;
      alnrcs[20] = +.55480701209082887983041321697279e-15;
      alnrcs[21] = -.10324619158271569595141333961932e-15;
      alnrcs[22] = +.19250239203049851177878503244868e-16;
      alnrcs[23] = -.35955073465265150011189707844266e-17;
      alnrcs[24] = +.67264542537876857892194574226773e-18;
      alnrcs[25] = -.12602624168735219252082425637546e-18;
      alnrcs[26] = +.23644884408606210044916158955519e-19;
      alnrcs[27] = -.44419377050807936898878389179733e-20;
      alnrcs[28] = +.83546594464034259016241293994666e-21;
      alnrcs[29] = -.15731559416479562574899253521066e-21;
      alnrcs[30] = +.29653128740247422686154369706666e-22;
      alnrcs[31] = -.55949583481815947292156013226666e-23;
      alnrcs[32] = +.10566354268835681048187284138666e-23;
      alnrcs[33] = -.19972483680670204548314999466666e-24;
      alnrcs[34] = +.37782977818839361421049855999999e-25;
      alnrcs[35] = -.71531586889081740345038165333333e-26;
      alnrcs[36] = +.13552488463674213646502024533333e-26;
      alnrcs[37] = -.25694673048487567430079829333333e-27;
      alnrcs[38] = +.48747756066216949076459519999999e-28;
      alnrcs[39] = -.92542112530849715321132373333333e-29;
      alnrcs[40] = +.17578597841760239233269760000000e-29;
      alnrcs[41] = -.33410026677731010351377066666666e-30;
      alnrcs[42] = +.63533936180236187354180266666666e-31;
      if (nlnrel == 0) 
      {
        nlnrel = chebyshev_init(alnrcs, 43, 0.1 * d1mach(3));
        xmin = -1.0 + sqrt(d1mach(4));
      }
      if (x <= -1)
        return DBL_MAX;
      if (fabs(x) <= 0.375)
        return x * (1.0 - x * chebyshev_eval(x / 0.375, alnrcs, nlnrel));
      else
        return log(x + 1.0);
    }

    static double lbeta(double a, double b)
    {
      double corr, p, q;
      p = q = a;
      if(b < p) p = b;
      if(b > q) q = b;
      if (p < 0)
        return -DBL_MAX;
      else if (p == 0) 
      {
        return DBL_MAX;
      }
      if (p >= 10.0)
      {
        corr = lgammacor(p) + lgammacor(q) - lgammacor(p + q);
        return log(q) * -0.5 + 0.918938533204672741780329736406 + corr
          + (p - 0.5) * log(p / (p + q)) + q * logrelerr(-p / (p + q));
      }
      else if (q >= 10) 
      {
        corr = lgammacor(q) - lgammacor(p + q);
        return lgamma(p) + corr + p - p * log(p + q)
          + (q - 0.5) * logrelerr(-p / (p + q));
      }
      else
      {
        double a1 = gamma(p);
        double a2 = gamma(q);
        double a3 = gamma(p+q);
        return log(a1 * (a2 / a3));
      }
    }


    static double pbeta_raw(double x, double pin, double qin)
    {
      double ans, c, finsum, p, ps, p1, q, term, xb, xi, y;
      int n, i, ib;
      double eps = 0.0;
      double alneps = 0.0;
      double sml = 0.0;
      double alnsml = 0.0;
      if (eps == 0.0)
      {
        eps = d1mach(3);
        alneps = log(eps);
        sml = d1mach(1);
        alnsml = log(sml);
      }
      y = x;
      p = pin;
      q = qin;
      if (p / (p + q) < x) 
      {
        y = 1.0 - y;
        p = qin;
        q = pin;
      }
      if ((p + q) * y / (p + 1.0) < eps)
      {
    
        ans = 0.0;
        xb = p * log(max(y, sml)) - log(p) - lbeta(p, q);
        if (xb > alnsml && y != 0.0)
          ans = exp(xb);
        if (y != x || p != pin)
          ans = 1.0 - ans;
      }
      else 
      {
        ps = q - floor(q);
        if (ps == 0.0)
          ps = 1.0;
        xb = p * log(y) - lbeta(ps, p) - log(p);
        ans = 0.0;
        if (xb >= alnsml)
        {
          ans = exp(xb);
          term = ans * p;
          if (ps != 1.0)
          {
            n = (int)max(alneps/log(y), 4.0);
            for(i=1 ; i<= n ; i++) 
            {
              xi = i;
              term = term * (xi - ps) * y / xi;
              ans = ans + term / (p + xi);
            }
          }
        }
        if (q > 1.0)
        {
          xb = p * log(y) + q * log(1.0 - y) - lbeta(p, q) - log(q);
          ib = (int)max(xb / alnsml, 0.0);
          term = exp(xb - ib * alnsml);
          c = 1.0 / (1.0 - y);
          p1 = q * c / (p + q - 1.0);
    
          finsum = 0;
          n = (int)q;
          if (q == n)
            n = n - 1;
          for(i=1 ; i<=n ; i++) 
          {
            if (p1 <= 1 && term / eps <= finsum)
              break;
            xi = i;
            term = (q - xi + 1.0) * c * term / (p + q - xi);
            if (term > 1.0)
            {
              ib = ib - 1;
              term = term * sml;
            }
            if (ib == 0)
              finsum = finsum + term;
          }
          ans = ans + finsum;
        }
        if (y != x || p != pin)
          ans = 1.0 - ans;
        ans = max(min(ans, 1.0), 0.0);
      }
      return ans;
    }
    static double pbeta(double x, double pin, double qin)
    {
      if (pin <= 0 || qin <= 0) 
      {
        return DBL_MAX;
      }
      if (x <= 0)
        return 0.0;
      if (x >= 1)
        return 1.0;
      return pbeta_raw(x, pin, qin);
    }

BOOL CALLBACK Calc_betainv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double prob, alpha, beta, dfResult; 
  int swap_tail, i_pb, i_inn;
  double a, adj, logbeta, g, h, pp, prev, qq, r, s, t, tx, w, y, yprev;
  double acu;
  double xinbta, xtrunc;
  double aa = 0.0;
  double bb = 1.0;
  double const1 = 2.30753;
  double const2 = 0.27061;
  double const3 = 0.99229;
  double const4 = 0.04481;
  double zero = 0.0;
  double fpu = 3.0e-308;
  double acu_min = 1.0e-300;
  double lower = fpu;
  double upper = 1.0-2.22e-16;
  double five = 5.0;
  double six = 6.0;
  double two = 2.0;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    prob = ValGetDouble(lpArg);
	alpha = ValGetDouble(lpArg+1);
	beta = ValGetDouble(lpArg+2);
	if (nArgs > 3)
		aa = ValGetDouble(lpArg+3);
	if (nArgs > 4)
		bb = ValGetDouble(lpArg+4);
     
    if (prob <= 0.0 || 1.0 <= prob) 
	{
		ValSetError(lpResult);
		return TRUE;
	}
      if (alpha <= 0.0 || beta <= 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}

      xinbta = prob;
      if(alpha < zero || beta < zero || prob < zero || prob > 1.0) 
      {
		ValSetError(lpResult);
		return TRUE;
      }
      if (prob == zero || prob == 1.0)
	  {
		  ValSetDouble(lpResult, prob);
		  return TRUE;
	  }
      logbeta = lbeta(alpha, beta);
      if (prob <= 0.5) 
      {
        a = prob;
        pp = alpha;
        qq = beta;
        swap_tail = 0;
      } 
      else 
      {
        a = 1.0 - prob;
        pp = beta;
        qq = alpha;
        swap_tail = 1;
      }
      r = sqrt(-log(a * a));
      y = r - (const1 + const2 * r) / (1.0 + (const3 + const4 * r) * r);
      if (pp > 1.0 && qq > 1.0) 
      {
        r = (y * y - 3.0) / 6.0;
        s = 1.0 / (pp + pp - 1.0);
        t = 1.0 / (qq + qq - 1.0);
        h = 2.0 / (s + t);
        w = y * sqrt(h + r) / h - (t - s) * (r + five / six - two / (3.0 * h));
        xinbta = pp / (pp + qq * exp(w + w));
      } 
      else 
      {
        r = qq + qq;
        t = 1.0 / (9.0 * qq);
        t = r * pow(1.0 - t + y * sqrt(t), 3.0);
        if (t <= zero)
          xinbta = 1.0 - exp((log((1.0 - a) * qq) + logbeta) / qq);
        else 
        {
          t = (4.0 * pp + r - two) / t;
          if (t <= 1.0)
            xinbta = exp((log(a * pp) + logbeta) / pp);
          else
            xinbta = 1.0 - two / (t + 1.0);
        }
      }
      r = 1.0 - pp;
      t = 1.0 - qq;
      yprev = zero;
      adj = 1.0;
      if (xinbta < lower)
        xinbta = lower;
      else if (xinbta > upper)
        xinbta = upper;
      acu = max(acu_min, pow(10.0, -13.0 - 2.5/(pp * pp) - 0.5/(a * a)));
      tx = prev = zero;
      for (i_pb=0; i_pb < 1000; i_pb++) 
      {
        y = pbeta_raw(xinbta, pp, qq);
        y = (y - a) *
          exp(logbeta + r * log(xinbta) + t * log(1.0 - xinbta));
        if (y * yprev <= zero)
          prev = max(fabs(adj),fpu);
        g = 1.0;
        for (i_inn=0; i_inn < 1000;i_inn++) 
        {
          adj = g * y;
          if (fabs(adj) < prev)
          {
            tx = xinbta - adj;
            if (tx >= zero && tx <= 1.0) 
            {
              if (prev <= acu)
                goto L_converged;
              if (fabs(y) <= acu)
                goto L_converged;
              if (tx != zero && tx != 1.0)
                break;
            }
          }
          g /= 3.0;
        }
        xtrunc = tx;
        if (xtrunc == xinbta)
          goto L_converged;
        xinbta = tx;
        yprev = y;
      }
      L_converged:
        if (swap_tail != 0)
          xinbta = 1.0 - xinbta;
      dfResult = (bb - aa) * xinbta + aa;	  	  
	  	
  }
  else
	ValSetError(lpResult);
  ValSetDouble(lpResult, dfResult);
  return TRUE;

}

BOOL CALLBACK Calc_finv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double f, df1, df2, dfResult, x; 
  CALC_VALUE val[3];

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    f = ValGetDouble(lpArg);
	df1 = ValGetDouble(lpArg+1);
	df2 = ValGetDouble(lpArg+2);
     
    if (f < 0.0 || 1.0 < f || df1 < 1.0 || df1 >= pow(10.0,10.0) || df2 < 1.0 || df2 >= pow(10.0,10.0))
	{
		ValSetError(lpResult);
		return TRUE;
	}

	x = 1.0 - f;

	ValSetDouble(val, 1.0 - x);
	ValSetDouble(val+1, df2 / 2.0);
	ValSetDouble(val+2, df1 / 2.0);
	Calc_betainv(lpResult, val, 1);
    dfResult =  (1.0 / ValGetDouble(lpResult) - 1.0) * (df2 / df1);
		
  }
  else
	ValSetError(lpResult);
  ValSetDouble(lpResult, dfResult);
  return TRUE;

}

BOOL CALLBACK Calc_standardize(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double x, mean, stdev, dfResult; 

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    x = ValGetDouble(lpArg);
	mean = ValGetDouble(lpArg+1);
	stdev = ValGetDouble(lpArg+2);
     
    if (stdev <= 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}

    dfResult = (x - mean) / stdev;
		
  }
  else
	ValSetError(lpResult);
  ValSetDouble(lpResult, dfResult);
  return TRUE;

}
BOOL CALLBACK Calc_expondist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double x, lambda, dfResult, ex; 
  BOOL cum;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    x = ValGetDouble(lpArg);
	lambda = ValGetDouble(lpArg+1);
	cum = ValGetBool(lpArg+2);
     
    if (x < 0.0) 
	{
		ValSetError(lpResult);
		return TRUE;
	}

    if (lambda <= 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}
    ex = exp(-lambda * x);
    dfResult = cum ? 1.0 - ex : lambda * ex;
		
  }
  else
	ValSetError(lpResult);
  ValSetDouble(lpResult, dfResult);
  return TRUE;

}
BOOL CALLBACK Calc_confidence(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double x, stdev, dfResult; 
  long size;
  CALC_VALUE val[1];

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
    x = ValGetDouble(lpArg);
	stdev = ValGetDouble(lpArg+1);
	size = ValGetLong(lpArg+2);
     
    if (x <= 0.0 || x >= 1.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}

    if (stdev <= 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}

    if (size < 1)
	{
		ValSetError(lpResult);
		return TRUE;
	}

	ValSetDouble(val, x / 2.0);
	Calc_normsinv(lpResult, val, 1);

    dfResult = -ValGetDouble(lpResult) * (stdev / sqrt(size));
		
  }
  else
	ValSetError(lpResult);
  ValSetDouble(lpResult, dfResult);
  return TRUE;

}
BOOL CALLBACK Calc_hypgeomdist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  long a1,a2,a3,a4;
  double pt1,pt2,pt3, dfResult;
  CALC_VALUE val[2];

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
	a1 = ValGetLong(lpArg);
	a2 = ValGetLong(lpArg+1);
	a3 = ValGetLong(lpArg+2);
	a4 = ValGetLong(lpArg+3);
     
    if (a1 < 0.0 || a1 > min(a2, a3))
	{
		ValSetError(lpResult);
		return TRUE;
	}

    if (a1 < max(0.0, a2 - a4 + a3))
	{
		ValSetError(lpResult);
		return TRUE;
	}

    if (a2 < 0.0 || a2 > a4)
	{
		ValSetError(lpResult);
		return TRUE;
	}

    if (a3 < 0.0 || a3 > a4)
	{
		ValSetError(lpResult);
		return TRUE;
	}

    if (a4 < 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}

     ValSetLong(val, a3);
	 ValSetLong(val+1,a1);
	 Calc_combin(lpResult, val, 2);
     pt1 = ValGetDouble(lpResult);
     ValSetLong(val, a4-a3);
	 ValSetLong(val+1,a2-a1);
	 Calc_combin(lpResult, val, 2);
     pt2 = ValGetDouble(lpResult);
     ValSetLong(val, a4);
	 ValSetLong(val+1,a2);
	 Calc_combin(lpResult, val, 2);
     pt3 = ValGetDouble(lpResult);
     dfResult = (pt1 * pt2) / pt3;		
  }
  else
	ValSetError(lpResult);
  ValSetDouble(lpResult, dfResult);
  return TRUE;

}

BOOL CALLBACK Calc_chiinv(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double x,df,p;
  CALC_VALUE val[3];


  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
	x = ValGetDouble(lpArg);
	df = ValGetDouble(lpArg+1);
     
	if (x < 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}

    if (df < 1.0 || df > pow(10.0,10.0))
	{
		ValSetError(lpResult);
		return TRUE;
	}

     p = 1.0 - x;
	 ValSetDouble(val, p);
	 ValSetDouble(val+1, 0.5*df);
	 ValSetDouble(val+2, 2.0);
	 Calc_gammainv(lpResult, val, 3);
	 return TRUE;
  }
  else
  {
	ValSetError(lpResult);
	return TRUE;
  }

}
BOOL CALLBACK Calc_chidist(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double x,df;
  double e, s, z, c, a, x1, zz;
  BOOL even;
  CALC_VALUE val[1];
  double LOG_SQRT_PI = log(sqrt(PI));
  double I_SQRT_PI = 1.0 / sqrt(PI);
  double y = 0.0;

  if( CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty) )
  {
	x = ValGetDouble(lpArg);
	df = ValGetDouble(lpArg+1);
     
	if (x < 0.0)
	{
		ValSetError(lpResult);
		return TRUE;
	}

    if (df < 1.0 || df > pow(10.0,10.0))
	{
		ValSetError(lpResult);
		return TRUE;
	}

    x1 = x;
    a = 0.5 * x1;
    if ((long)df % 2 == 0)
		even = TRUE;
	else
		even = FALSE;
    if (df > 1.0)
        y = exp(-a);

	ValSetDouble(val, -sqrt(x1));
	Calc_normsdist(lpResult, val, 1);
    zz = ValGetDouble(lpResult);
    s = (even ? y : (2.0 * zz));
    if (df > 2)
    {
        x1 = 0.5 * (df - 1.0);
        z = (even ? 1.0 : 0.5);
        if (a > 20.0)
        {
            e = (even ? 0.0 : LOG_SQRT_PI);
            c = log(a);
            while (z <= x1)
            {
                e = log(z) + e;
                s += exp(c * z - a - e);
                z += 1.0;
            }
			ValSetDouble(lpResult, s);
			return TRUE;
        }
        else
        {
            e = (even ? 1.0 : (I_SQRT_PI / sqrt(a)));
            c = 0.0;
            while (z <= x1)
            {
                e = e * (a / z);
                c = c + e;
                z += 1.0;
            }
			ValSetDouble(lpResult, c * y + s);
            return TRUE;
        }
    }
    else
    {
        ValSetDouble(lpResult, s);
		return TRUE;
    }

  }
  else
  {
	ValSetError(lpResult);
	return TRUE;
  }

}

BOOL CALLBACK Calc_days360(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double dfStartDate, dfEndDate;
  BOOL method = FALSE;
  short nStartMonth, nStartYear, nStartDay;
  short nEndMonth, nEndYear, nEndDay;
  long lResult;

  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    dfStartDate = ValGetDouble(lpArg);
    dfEndDate = ValGetDouble(lpArg+1);
	if (nArgs > 2)
		method = ValGetBool(lpArg+2);

    if (DATE_1899_12_30 <= dfStartDate && dfStartDate < DATE_9999_12_31 + 1.0 &&
		DATE_1899_12_30 <= dfEndDate && dfEndDate < DATE_9999_12_31 + 1.0)
    {
 
     ftDoubleToDate(dfStartDate, &nStartYear, &nStartMonth, &nStartDay);
     ftDoubleToDate(dfEndDate, &nEndYear, &nEndMonth, &nEndDay);
   
     if (method)
      {
        if (nEndDay == 31)
          nEndDay = 30;
        if (nStartDay == 31)
          nStartDay = 30;
      }
     else
      {
        if (nStartDay == 31)
           nStartDay = 30;
        if (nEndDay == 31)
        {
          if (nStartDay < 30)
          {
            nEndDay = 1;
            nEndMonth++;
            if (nEndMonth > 12)
            {
              nEndMonth = 1;
              nEndYear++;
            }
          }
          else
            nEndDay = 30;
        }
      }
      lResult = ((nEndYear - nStartYear) * 12 + (nEndMonth - nStartMonth)) * 30 + (nEndDay - nStartDay);
      ValSetLong(lpResult, lResult);
	  return TRUE;
    }
    else
      ValSetError(lpResult);
  }
  else
    ValSetError(lpResult);
  return TRUE;
}

static BOOL isLeapYear(short Year)
{
if (((Year % 4) == 0 && (Year % 100) != 0) ||
    ((Year % 100) == 0 && (Year % 400) == 0))
   return TRUE;
else
   return FALSE;
}
  int annual_year_basis (double date, int basis)
    {
      BOOL leap_year;
	  short nYear;
      ftDoubleToDate(date, &nYear, NULL, NULL);

      switch (basis) 
      {
        case 0:
          return 360;
        case 1:
          leap_year = isLeapYear(nYear);
          return leap_year ? 366 : 365;
        case 2:
          return 360;
        case 3:
          return 365;
        case 4:
          return 360;
        default:
          return -1;
      }
    }
    int days_monthly_basis (double date_i, double date_m, int basis)
    {
      short      issue_day, issue_month, issue_year;
      short      maturity_day, maturity_month, maturity_year;
      int      months, days, years;
      BOOL     leap_year;
      CALC_VALUE val[2];
	  CALC_VALUE result[1];

      ftDoubleToDate(date_i, &issue_year, &issue_month, &issue_day);
      ftDoubleToDate(date_m, &maturity_year, &maturity_month, &maturity_day);

      years = maturity_year - issue_year;
      months = maturity_month - issue_month;
      days = maturity_day - issue_day;

      months = years * 12 + months;
      leap_year = isLeapYear(issue_year);

      switch (basis) 
      {
        case 0:
          ValSetDouble(val, date_i);
		  ValSetDouble(val+1, date_m);
		  Calc_days360(result, val, 2);
          return ValGetLong(result);
        case 1:
        case 2:
        case 3:
          return (int)(date_m - date_i);
        case 4:
          return months * 30 + days;
        default:
          return -1;
      }
    }   


  int days_between_BASIS_MSRB_30_360 (double from, double to)
    {
      short y1, m1, d1, y2, m2, d2;
      SS_DATE  fromDt, toDt;


 	  ftDoubleToDate(from, &y1, &m1, &d1);
	  ftDoubleToDate(to, &y2, &m2, &d2);
	  fromDt.nDay = d1;
	  fromDt.nMonth = m1;
	  fromDt.nYear = y1;
	  toDt.nDay = d2;
	  toDt.nMonth = m2;
	  toDt.nYear = y2;

      //km added for bug 16110. 
		if ((m1 == 2 && DateGetDaysInMonth(&fromDt) == d1) && (m2 == 2 && DateGetDaysInMonth(&toDt) == d2))
		{
			d1 = 30;
			d2 = 30;
		}
      if (d2 == 31 && d1 >= 30)
        d2 = 30;
      if (d1 == 31)
        d1 = 30;

      return (y2 - y1) * 360 + (m2 - m1) * 30 + (d2 - d1);
    }

    static  int days_between_BASIS_MSRB_30_360_SYM (double from, double to)
    {
      short y1, m1, d1, y2, m2, d2;
      SS_DATE  fromDt, toDt;


 	  ftDoubleToDate(from, &y1, &m1, &d1);
	  ftDoubleToDate(to, &y2, &m2, &d2);
	  fromDt.nDay = d1;
	  fromDt.nMonth = m1;
	  fromDt.nYear = y1;
	  toDt.nDay = d2;
	  toDt.nMonth = m2;
	  toDt.nYear = y2;

      if (m1 == 2 && DateGetDaysInMonth(&fromDt) == d1)
        d1 = 30;
      if (m2 == 2 && DateGetDaysInMonth(&toDt) == d2)
        d2 = 30;
      if (d2 == 31 && d1 >= 30)
        d2 = 30;
      if (d1 == 31)
        d1 = 30;

      return (y2 - y1) * 360 + (m2 - m1) * 30 + (d2 - d1);
    }

    static  int days_between_BASIS_30E_360 (double from, double to)
    {
      short y1, m1, d1, y2, m2, d2;

 	  ftDoubleToDate(from, &y1, &m1, &d1);
	  ftDoubleToDate(to, &y2, &m2, &d2);

      if (d1 == 31)
        d1 = 30;
      if (d2 == 31)
        d2 = 30;

      return (y2 - y1) * 360 + (m2 - m1) * 30 + (d2 - d1);
    }

    static  int days_between_BASIS_30Ep_360 (double from, double to)
    {
      short y1, m1, d1, y2, m2, d2;
      
	  ftDoubleToDate(from, &y1, &m1, &d1);
	  ftDoubleToDate(to, &y2, &m2, &d2);

      if (d1 == 31)
        d1 = 30;
      if (d2 == 31) 
      {
        d2 = 1;
        m2++;
      }

      return (y2 - y1) * 360 + (m2 - m1) * 30 + (d2 - d1);
    }

    int days_between_basis (double from,  double to, int basis)
    {
      int sign = 1;

      if (from > to) 
      {
        double tmp = from;
        from = to;
        to = tmp;
        sign = -1;
      }

      switch (basis) 
      {
        case 1:
        case 2:
        case 3:
          return sign * (int)(to - from);
        case 4:
          return sign * days_between_BASIS_30E_360 (from, to);
        case 5:
          return sign * days_between_BASIS_30Ep_360 (from, to);
        case 6:
          return sign * days_between_BASIS_MSRB_30_360_SYM (from, to);
        case 0:
        default:
          return sign * days_between_BASIS_MSRB_30_360 (from, to);
      }
    }	
	static double yearfrac (double from,  double to, int basis)
    {
      double peryear;
	  short y1, y2, m1, m2, dy1, dy2;
	  double d1, d2, d;
      int feb29s, years;
      int days = days_between_basis (from, to, basis);

      if (days < 0) 
      {
        double tmp;
        days = -days;
        tmp = from; from = to; to = tmp;
      }

	  ftDoubleToDate(from, &y1, &m1, &dy1);
	  ftDoubleToDate(to, &y2, &m2, &dy2);
      switch (basis) 
      {
        case 1: 
        {
		  ftDateToDouble(y1+1, m1, dy1, &d1);
          if (to > d1) 
          {
            /* More than one year.  */
            years = y2 + 1 - y1;
            ftDateToDouble(y1,1,1, &d1);
            ftDateToDouble(y2+1, 1, 1, &d2);

            feb29s = (int)(d2 - d1) -
              365 * (int)(y2 + 1 - y1);
          } 
          else 
          {
            /* Less than one year.  */
            years = 1;

            if ((isLeapYear(y1) && m1 < 3) ||
              (isLeapYear(y2) &&
              (m2 * 0x100 + dy2 >= 2 * 0x100 + 29)))
              feb29s = 1;
            else
              feb29s = 0;			    
          }

          d =  (double)feb29s / (double)years;
          peryear = 365.0 + d;
        }

          break;

        default:
			{
			  SYSTEMTIME t;
			  double dfDate;

			  GetLocalTime(&t);
			  ftDateToDouble(t.wYear, t.wMonth, t.wDay, &dfDate);
			  peryear = annual_year_basis (dfDate, basis);
			}
          break;
      }

      return days / peryear;
    }

BOOL CALLBACK Calc_yearfrac(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  BOOL bAnyDouble;
  BOOL bAnyLong;
  BOOL bAnyEmpty;
  double start, end;
  long basis = 0;

  if (CheckAllNumOrEmpty(lpArg, nArgs, &bAnyDouble, &bAnyLong, &bAnyEmpty))
  {
    start = ValGetDouble(lpArg);
	end = ValGetDouble(lpArg+1);
	if (nArgs > 2)
		basis = ValGetLong(lpArg+2);
    
    if (basis < 0 || basis > 4)
	  {
		  ValSetError(lpResult);
		  return TRUE;
	  }
 
      ValSetDouble(lpResult, yearfrac(start, end, basis));
	  return TRUE;
  }
  else
    ValSetError(lpResult);
  return TRUE;
}
  
static int parseCriteria(LPTSTR criteria)
    {

      if (_tcsnicmp(criteria, _T("<=") , 2) == 0) 
        return 0;
      else if (_tcsnicmp(criteria, _T(">="), 2) == 0) 
        return 1;
      else if (_tcsnicmp(criteria, _T("<>"), 2) == 0) 
        return 2;
      else if (_tcsnicmp(criteria, _T("<"),1) == 0)
        return 3;
      else if (_tcsnicmp(criteria, _T("="), 1) == 0)
        return 4;
      else if (_tcsnicmp(criteria, _T(">"), 1) == 0)
        return 5;
      else
        return -1;

    }

 static BOOL checkCondition(int crit, LPTSTR critVal, LPCALC_VALUE value)
    {
      double val = -1;
      double chkVal = -1; 
	  BOOL ret = FALSE;
      if (crit != -1)
      {
        if (value->Status == CALC_VALUE_STATUS_EMPTY)
          return FALSE;
        val = CalcStringToDouble(critVal);
		if (value->Type == SS_VALUE_TYPE_LONG || value->Type == SS_VALUE_TYPE_DOUBLE)
			chkVal = ValGetDouble(value);
		else
			return FALSE;
      }

      switch (crit)
      {
        case 0:
          if (chkVal <= val)
            return TRUE;          
          break;
        case 1:
          if (chkVal >= val)
            return TRUE;
          break;
        case 2:
          if (chkVal != val)
            return TRUE;
          break;
        case 3:
          if (chkVal < val)
            return TRUE;
          break;
        case 4:
          if (chkVal == val)
            return TRUE;
          break;
        case 5:
          if (chkVal > val)
            return TRUE;
          break;
        default:
         {
            LPTSTR lpszVal = NULL; // 24919 -scl
            HGLOBAL	hCrit = ValGetStringHandle(value);
            if (hCrit)
               lpszVal = (LPTSTR)GlobalLock(hCrit);

            if (lpszVal && _tcscmp(critVal, lpszVal) == 0)
            {
               ret = TRUE;
               GlobalUnlock(hCrit);
            }
         }
         break;
      }
      return ret;
    }


BOOL CALLBACK Calc_countif(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value;
  long lCol, lRow;
  long count = 0;
  TCHAR Buffer[330];
  HGLOBAL hCrit;
  LPTSTR lpszCrit = NULL;
  double checkVal;
  int crit;
  int len = -1;

  _fmemset(Buffer, '\0', 330 * sizeof(TCHAR));
  if ((lpArg+1)->Type == CALC_VALUE_TYPE_STR)
  {
	hCrit = ValGetStringHandle(lpArg+1);
	if (hCrit)
		lpszCrit = (LPTSTR)GlobalLock(hCrit);
	else
	{
	  ValSetError(lpResult);
	  return TRUE;
	}
	if( lpszCrit )
      {
		crit = parseCriteria(lpszCrit);
		len = (crit >= 0 && crit <= 2) ? 2 : crit > 2 ? 1 : -1; 
		if (len != -1)
			lpszCrit = _tcsninc(lpszCrit, len);
      }
    GlobalUnlock(hCrit);
  }
  else
  {
	  crit = 4;
	  checkVal = ValGetDouble(lpArg+1);
      fpDoubleToString (Buffer, checkVal, 15, (TCHAR)'.', 0, FALSE, FALSE,
         TRUE, FALSE, 0, NULL);

  }
  if (lpArg->Status == CALC_VALUE_STATUS_OK && lpArg->Type == CALC_VALUE_TYPE_RANGE)
  {
       lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
       for( lCol = lpArg->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2; lCol++ )
          for( lRow = lpArg->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2; lRow++ )
          {
            lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
            if( CALC_VALUE_STATUS_OK == value.Status )
            {
  
				len = lstrlen(lpszCrit);
			    _tcsncpy(Buffer, lpszCrit, len);
				if (checkCondition(crit, Buffer, &value))
					count++;
            }
            ValFree(&value);
          }

   ValSetLong(lpResult, count);
  }
  else
	  ValSetError(lpResult);

  return TRUE;
}

BOOL CALLBACK Calc_sumif(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs)
{
  LPCALC_SHEET lpSheet;
  CALC_VALUE value, value2;
  long lCol, lRow, lCol2, lRow2;
  TCHAR Buffer[330];
  HGLOBAL hCrit;
  LPTSTR lpszCrit = NULL;
  double checkVal;
  double sum = 0.0;
  int crit;
  int len = -1;
  LPCALC_VALUE lpSumRange;

  _fmemset(Buffer, '\0', 330 * sizeof(TCHAR));
  if ((lpArg+1)->Type == CALC_VALUE_TYPE_STR)
  {
	hCrit = ValGetStringHandle(lpArg+1);
	if (hCrit)
		lpszCrit = (LPTSTR)GlobalLock(hCrit);
	else
	{
	  ValSetError(lpResult);
	  return TRUE;
	}
	if( lpszCrit )
      {
		crit = parseCriteria(lpszCrit);
		len = (crit >= 0 && crit <= 2) ? 2 : crit > 2 ? 1 : -1; 
		if (len != -1)
			lpszCrit = _tcsninc(lpszCrit, len);
      }
    GlobalUnlock(hCrit);
  }
  else
  {
	  crit = 4;
	  checkVal = ValGetDouble(lpArg+1);
      fpDoubleToString (Buffer, checkVal, 15, (TCHAR)'.', 0, FALSE, FALSE,
         TRUE, FALSE, 0, NULL);

  }

  if (nArgs > 2)
	  lpSumRange = lpArg+2;
  else
	  lpSumRange = lpArg;

  if (lpArg->Status == CALC_VALUE_STATUS_OK && lpArg->Type == CALC_VALUE_TYPE_RANGE)
  {
	   if ((lpArg->Val.ValRange.Col2 - lpArg->Val.ValRange.Col1 != lpSumRange->Val.ValRange.Col2 - lpSumRange->Val.ValRange.Col1) ||
		   (lpArg->Val.ValRange.Row2 - lpArg->Val.ValRange.Row1 != lpSumRange->Val.ValRange.Row2 - lpSumRange->Val.ValRange.Row1))
	   {
		   ValSetError(lpResult);
		   return TRUE;
	   }

       lpSheet = (LPCALC_SHEET)lpArg->Val.ValRange.u.lInfo;
       for( lCol = lpArg->Val.ValRange.Col1, lCol2 = lpSumRange->Val.ValRange.Col1; lCol <= lpArg->Val.ValRange.Col2, lCol2 <= lpSumRange->Val.ValRange.Col2; lCol++, lCol2++ )
          for( lRow = lpArg->Val.ValRange.Row1, lRow2 = lpSumRange->Val.ValRange.Row1; lRow <= lpArg->Val.ValRange.Row2, lRow2 <= lpSumRange->Val.ValRange.Row2; lRow++, lRow2++ )
          {
            lpSheet->lpfnGetData(lpSheet->hSS, lCol, lRow, &value);
            lpSheet->lpfnGetData(lpSheet->hSS, lCol2, lRow2, &value2);
            if( CALC_VALUE_STATUS_OK == value.Status && CALC_VALUE_STATUS_OK == value2.Status)
            {
  
				len = lstrlen(lpszCrit);
			    _tcsncpy(Buffer, lpszCrit, len);
				if (checkCondition(crit, Buffer, &value))
					sum+=ValGetDouble(&value2);;
            }
            ValFree(&value);
          }

   ValSetDouble(lpResult, sum);
  }
  else
	  ValSetError(lpResult);

  return TRUE;
}
#endif
