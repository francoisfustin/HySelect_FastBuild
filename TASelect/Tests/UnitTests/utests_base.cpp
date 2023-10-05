#include "stdafx.h"
#include "utests_base.h"

void utests_base::setUp()
{
	m_CurrentLocale = GetThreadLocale();

	// Prefer the 'SetThreadLocale' function instead of the '_tsetlocale' function. Because the first function will set the DEFAULT
	// regional settings corresponding to the LCID and not the one that the user can change in the Windows settings.
	SetThreadLocale( 0x0409 );
}

void utests_base::tearDown()
{
	SetThreadLocale( m_CurrentLocale );
}
