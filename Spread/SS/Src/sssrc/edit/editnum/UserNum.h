// usernum.h

#define SSNUM_DECCHAR_SIZE  3
#define SSNUM_SEPCHAR_SIZE  3
#define SSNUM_GROUPING_SIZE 10
#define SSNUM_CURCHAR_SIZE  5
#define SSNUM_NEGSIGN_SIZE  4
#define SSNUM_POSSIGN_SIZE  4

typedef struct tagSSNUM_INTLSETTINGS
	{
	TCHAR decimal_point[SSNUM_DECCHAR_SIZE + 1];
	TCHAR thousands_sep[SSNUM_SEPCHAR_SIZE + 1];
	TCHAR grouping[SSNUM_GROUPING_SIZE + 1];
	TCHAR currency_symbol[SSNUM_CURCHAR_SIZE + 1];
	TCHAR mon_decimal_point[SSNUM_DECCHAR_SIZE + 1];
	TCHAR mon_thousands_sep[SSNUM_SEPCHAR_SIZE + 1];
	TCHAR mon_grouping[SSNUM_GROUPING_SIZE + 1];
	TCHAR positive_sign[SSNUM_POSSIGN_SIZE + 1];
	TCHAR negative_sign[SSNUM_NEGSIGN_SIZE + 1];
	BYTE  fLeadingZero;         // T/F
	char  frac_digits;          // 0 - 9
	BYTE  bNegStyle;            // 0 - 4
	BYTE  bNegCurrencyStyle;    // 0 - 15
	BYTE  bPosCurrencyStyle;    // 0 - 4
	} SSNUM_INTLSETTINGS, *LPSSNUM_INTLSETTINGS;

typedef struct tagSSNUM_FORMAT
	{
	TCHAR  szDecimal[SSNUM_DECCHAR_SIZE + 1];
	TCHAR  szSeparator[SSNUM_SEPCHAR_SIZE + 1];
	TCHAR  szCurrency[SSNUM_CURCHAR_SIZE + 1];
	BYTE   bLeadingZero;         // 0 - 2
	short  bFractionalWidth;     // 0 - 10
	BYTE   bNegCurrencyStyle;    // 0 - 16
	BYTE   bPosCurrencyStyle;    // 0 - 5
	BYTE   bNegStyle;            // 0 - 5
	BYTE   fCurrency;            // T/F
	BYTE   fShowSeparator;       // T/F
	BYTE   fShowCurrencySymbol;  // T/F
	BYTE   fSpin;                // T/F
	BYTE   fSpinWrap;            // T/F
	double dfSpinInc;
	} SSNUM_FORMAT, *LPSSNUM_FORMAT;

#ifdef __cplusplus
extern "C" {
#endif
BOOL NumSetRange(HWND hWnd, double Min, double Max);
BOOL NumGetValue(HWND hWnd, double *lpDouble);
BOOL NumSetValue(HWND hWnd, double dfDouble);
BOOL NumSetFormat(HWND hWnd, LPSSNUM_FORMAT lpff);
BOOL NumStringToValue(LPSSNUM_FORMAT lpFormat, LPCTSTR szString, double *lpdfVal);
BOOL NumValueToString(LPSSNUM_FORMAT lpFormat, double dfVal, LPTSTR szString);
void NumGetDisplayFormat(LPSSNUM_FORMAT lpFormatSrc, LPSSNUM_FORMAT lpFormat);
void NumGetSystemIntlSettings(LPSSNUM_INTLSETTINGS lpIntlSettings);
BOOL NumReadSystemIntlSettings(LPSSNUM_INTLSETTINGS lpIntlSettings);
#ifdef SS_V80
void NumSetSpinStyle (HWND hWnd, BOOL fEnhanced);
#endif
#ifdef __cplusplus
}
#endif
