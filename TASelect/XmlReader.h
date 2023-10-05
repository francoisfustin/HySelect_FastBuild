#pragma once
#include "stdlib.h"
#include "stdio.h"
#include <iostream>
#include <fstream>
#include <string>
#include "global.h"

class CXmlReader
{
protected:
	std::ifstream m_inpf;
	std::wstring m_field;
	std::wstring FindNextBalise();
	char *m_pBuf; 
	WCHAR *m_pPosBuf;
public:
	CXmlReader(void);
	~CXmlReader(void);

	int	Open(std::string filename);
	void	Close();
	WCHAR * GetStringTo(WCHAR wcMark);
	// return text included between <BaliseID> and <\BaliseID>
	WCHAR * GetElementContent(WCHAR * pElement, WCHAR *pEndTable=NULL);
	// return find next balise, return text included and BaliseID
	WCHAR * GetNextElement();
	WCHAR ProcessXMLSpecChar(WCHAR **pos);
	// Function that make a special treatment for '\n','\r','\t'
	void ProcessNewLineChar(WCHAR** pos, WCHAR** posBuf);

};
