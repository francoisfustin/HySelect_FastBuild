/*    PortTool v2.2     FPTOOL30.C          */

//
// International Settings  (Currency & Numeric)
//
//   iCurrency=0
//   iCurrDigits=2
//   iNegCurr=1
//   sCurrency=$
//
//   iLzero=0
//   iDigits=2
//   sThousand=,
//   sDecimal=.
//   sList=,
//
#include "fptools.h"
#include <tchar.h>

static unsigned char fIsLoaded = 0L;

#define  SCURRENCY        0x01   // "sCurrency"
#define  ICURRENCY        0x02   // "iCurrency"
#define  ICURRDIGITS      0x04   // "iCurrDigits"
#define  INEGCURR         0x08   // "iNegCurr"

#define  STHOUSAND        0x10   // "sThousand"
#define  SDECIMAL         0x20   // "sDecimal"
#define  IDIGITS          0x40   // "iDigits"
#define  ILZERO           0x80   // "iLzero"
#if (WINVER >= 0x0400)  //WIN95 or NT
#define	 SMONTHOUSAND     0x100 // "sMonThousand"
#define  SMONDECIMAL      0x200 // "sMonDecimal"
#endif

// International Currency Settings
//
void FPLIB fpResetIntlNumeric()
{
	fIsLoaded = 0L;
}
// Currency Symbol String
//
static TCHAR sCurrency[8];
LPTSTR FPLIB fpGetIntl_sCurrency()
{
   if ((fIsLoaded & SCURRENCY) == 0)
      {
      GetProfileString (_T("intl"), _T("sCurrency"), _T("$"), sCurrency, STRING_SIZE(sCurrency));
      fIsLoaded |= SCURRENCY;
      }
   return sCurrency;
}

// Currency Symbol Placement
//
static int iCurrency; 
int FPLIB fpGetIntl_iCurrency()
{
   if ((fIsLoaded & ICURRENCY) == 0)
      {
      iCurrency = GetProfileInt (_T("intl"), _T("iCurrency"), 0);
      fIsLoaded |= ICURRENCY;
      }
   return iCurrency;
}

// Currency Fractional Digits
static int  iCurrDigits;
int FPLIB fpGetIntl_iCurrDigits()
{
   if ((fIsLoaded & ICURRDIGITS) == 0)
      {
      iCurrDigits = GetProfileInt (_T("intl"), _T("iCurrDigits"), 3);
      fIsLoaded |= ICURRDIGITS;
      }
   return iCurrDigits;
}

// Currency Negative Format
static int iNegCurr;
int FPLIB fpGetIntl_iNegCurr()
{
   if ((fIsLoaded & INEGCURR) == 0)
      {
      iNegCurr = GetProfileInt (_T("intl"), _T("iNegCurr"), 1);
      fIsLoaded |= INEGCURR;
      }
   return iNegCurr;
}


// International Numeric Settings
//

// Thousands Separator String
static TCHAR sThousand[2];
LPTSTR FPLIB fpGetIntl_sThousand()
{
   if ((fIsLoaded & STHOUSAND) == 0)
      {
      GetProfileString (_T("intl"), _T("sThousand"), _T(","), sThousand, STRING_SIZE(sThousand));
      fIsLoaded |= STHOUSAND;
      }
   return sThousand;
}

// Decimal Point String
static TCHAR sDecimal[2];
LPTSTR FPLIB fpGetIntl_sDecimal()
{
   if ((fIsLoaded & SDECIMAL) == 0)
      {
      GetProfileString (_T("intl"), _T("sDecimal"), _T("."), sDecimal, STRING_SIZE(sDecimal));
      fIsLoaded |= SDECIMAL;
      }
   return sDecimal;
}

#if (WINVER >= 0x0400)  //WIN95 or NT
// Thousands Separator String
static TCHAR sMonThousand[2];
LPTSTR FPLIB fpGetIntl_sMonThousandSep()
{
   if ((fIsLoaded & SMONTHOUSAND) == 0)
      {
      GetProfileString (_T("intl"), _T("sMonThousandSep"), _T(","), sMonThousand, STRING_SIZE(sMonThousand));
      fIsLoaded |= SMONTHOUSAND;
      }
   return sMonThousand;
}

// Decimal Point String
static TCHAR sMonDecimal[2];
LPTSTR FPLIB fpGetIntl_sMonDecimalSep()
{
   if ((fIsLoaded & SMONDECIMAL) == 0)
      {
      GetProfileString (_T("intl"), _T("sMonDecimalSep"), _T("."), sMonDecimal, STRING_SIZE(sMonDecimal));
      fIsLoaded |= SMONDECIMAL;
      }
   return sMonDecimal;
}
#endif

// Fractional Digits
static int iDigits;
int FPLIB fpGetIntl_iDigits()
{
   if ((fIsLoaded & IDIGITS) == 0)
      {
      iDigits = GetProfileInt (_T("intl"), _T("iDigits"), 2);
      fIsLoaded |= IDIGITS;
      }
   return iDigits;
}

// Leading Zero Indicator
static int iLZero;
int FPLIB fpGetIntl_iLZero()
{
   if ((fIsLoaded & ILZERO) == 0)
      {
      iLZero = GetProfileInt (_T("intl"), _T("iLZero"), 2);
      fIsLoaded |= ILZERO;
      }
   return iLZero;
}
