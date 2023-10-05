#pragma once
#include <string>
#include <stdarg.h>
#include <stdexcept>


using namespace std;
std::string FormatString( const char *szFormat, va_list &arg_ptr );
std::string FormatString( const char *szFormat, ... );
std::wstring FormatString(const wchar_t *szFormat,va_list &arg_ptr );
std::wstring FormatString(const wchar_t *szFormat,... );

template <typename tplstring = string, typename T = char >
class stringex : public tplstring
{
public:
	void FormatString(const T *szFormat, ...)
	{
		if(szFormat==NULL) AfxThrowUserException();
	
		va_list arg_ptr;
		va_start(arg_ptr,szFormat);	
	
		swap(::FormatString(szFormat,arg_ptr ));
		va_end(arg_ptr);
	}
};
