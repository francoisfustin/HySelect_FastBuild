#include <windows.h>
#include <ctype.h>
#include <toolbox.h>

#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))
//GAB 8/19/04 Bug 15052 Make the buffer size the same for both configurations.
//#if defined(_UNICODE)
//#define CVTBUFSIZE (350) //GAB 1/13/2004
//#else
#define CVTBUFSIZE (309+40) /* # of digits in max. dp value + slop */
//#endif

int   cdecl _cftof(double *, TCHAR *, int);

#define  MAX_STRLEN  800

#ifndef  NULL
#define  NULL  0l
#endif

typedef  struct
{
   int   signflag;
   int   plusflag;
   int   blankflag;
   int   poundflag;
   int   upperflag;
   int   precision;
   int   farflag;
   long  ival;
} FLAGS;

typedef  FLAGS far *LPFLAGS;

#if !defined(va_start) && !defined(va_arg) && !defined(va_end)
typedef char far *va_list;
#define va_start(ap,v) ap = (va_list)&v + sizeof(v)
#define va_arg(ap,t) ((t far *)(ap += sizeof(t)))[-1]
#define va_end(ap) ap = NULL
#endif

int   far pascal TBatoi(const TCHAR far  * far *format);
int   far pascal _output(LPTSTR, LPCTSTR, va_list FAR*);
void  far pascal _eint(TCHAR far *, int, LPFLAGS);
void  far pascal _dtoa(unsigned, TCHAR far * far *);
void  far pascal _htoa(unsigned, TCHAR far * far *, int);
void  far pascal _otoa(unsigned, TCHAR far * far *);
void  far pascal _elong(TCHAR far *, int, LPFLAGS);
void  far pascal _ldtoa(unsigned long, TCHAR far * far *);
void  far pascal _lhtoa(unsigned long, TCHAR far * far *, int);
void  far pascal _lotoa(long, TCHAR far * far *);
int   far pascal _mputc(int, TCHAR far * far *);
int   far pascal _mwrite(TCHAR far * far *, TCHAR far *, int);

extern   double   val;
