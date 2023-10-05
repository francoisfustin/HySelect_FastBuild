#include "fptools.h"
#include <ctype.h>

// workaround for bug in strtod() in MSVCRT.DLL
double FPLIB fpstrtod(LPCTSTR str, char FAR ** ppchar, const char decimal)
{
	double  val = 0.0;
	BOOL    neg;
	int     digits = 0;

	while (isspace (*str))
		str++;

	str += (neg = *str == '-'); 

	while(isdigit (*str))
	{
		  val = val*10 + (int)(*str++ - '0');
		  digits++;
	}

	*ppchar = (char FAR *)str;
	if (*str && *str++ != decimal)
		return 0;
	else if (*str)
	{	// process decimal digits
		double place = 10.0;
		while (isdigit(*str) && digits < 16)	// 15 digit max for double data type
		{
			val = val + (((double)((int)*str++ - '0')) / place);
			place *= 10.0;
			digits++;
		}
		*ppchar = (char FAR *)str;
	}
	*ppchar = (char FAR *)str;
	return neg ? -val : val;
}
