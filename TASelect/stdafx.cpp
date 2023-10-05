// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// stdafx.cpp : source file that includes just the standard includes
// TASelect.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

/*#include<list>
typedef struct
{
	DWORD	address;
	DWORD	size;
	CString file;
	DWORD	line;
}ALLOC_INFOS;

typedef std::list<ALLOC_INFOS*> AllocList;
AllocList *allocList;

void AddTrack(DWORD addr,  DWORD asize,  const char *fname, DWORD lnum)
{
	ALLOC_INFOS *info;
	if(!allocList)
	{
		allocList = new(AllocList);
	}
	info = new(ALLOC_INFOS);
	info->address = addr;
	info->line = lnum;
	info->file = fname;
	info->size = asize;
	allocList->insert(allocList->begin(), info);
};
void RemoveTrack(DWORD addr)
{
	AllocList::iterator i;
	if(!allocList)
		return;
	for(i = allocList->begin(); i != allocList->end(); i++)
	{
		if((*i)->address == addr)
		{
			allocList->remove((*i));
			break;
		}
	}
};
#ifdef _DEBUG
void DumpUnfreed()
{
	CString str;
	AllocList::iterator i;
	DWORD totalSize = 0;
	
	if(!allocList)
		return;
	for(i = allocList->begin(); i != allocList->end(); i++)
	{
		str.Format(_T("%-50s:\t\tLINE %d,\t\tADDRESS %d\t%d unfreed\n"),(*i)->file, (*i)->line, (*i)->address, (*i)->size);
		OutputDebugString((LPCWSTR)str);
		totalSize += (*i)->size;
	}
	str = _T("-----------------------------------------------------------\n");
	OutputDebugString((LPCWSTR)str);
	str.Format(_T("Total Unfreed: %d bytes\n"),totalSize);
	OutputDebugString((LPCWSTR)str);
};
void * __cdecl operator new(unsigned int size, const char *file, int line)
{
	void *ptr = (void *)malloc(size);
	AddTrack((DWORD)ptr, size, file, line);
	return(ptr);
};
void __cdecl operator delete(void *p)
{
	RemoveTrack((DWORD)p);
	free(p);
};
#endif*/