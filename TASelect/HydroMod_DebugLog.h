#pragma once

#include <atlsimpstr.h>


#define CDS_HYDROMOD_COMPUTEALLLOG_DEFAULTFILENAME							_T("c:\\temp\\cds_hydromod_computealllog.txt")
#define CDS_HYDROMOD_COMPUTEALLLOG_TABULATION								_T("  ")


#ifdef _DEBUG
	#define CDSHM_CAL_STARTLOG( class )										class.ComputeAllLog_OpenLogfile()
	#define CDSHM_CAL_STOPLOG( class )										class.ComputeAllLog_CloseLogfile()
	#define CDSHM_CAL_CHANGETABINDENT( class, TabFct )						class.ComputeAllLog_ChangeTabIndent( TabFct )
	#define CDSHM_CAL_WRITESTRING( class, strText, ... )					class.ComputeAllLog_WriteString( CDS_HydroMod_DebugLog::TIF_Nothing, strText, __VA_ARGS__ )
	#define CDSHM_CAL_WRITESTRING_POSTINC( class, strText, ... )			class.ComputeAllLog_WriteString( CDS_HydroMod_DebugLog::TIF_PoI, strText, __VA_ARGS__ )
	#define CDSHM_CAL_WRITESTRING_PREINC( class, strText, ... )				class.ComputeAllLog_WriteString( CDS_HydroMod_DebugLog::TIF_PrI, strText, __VA_ARGS__ )
	#define CDSHM_CAL_WRITESTRING_POSTDEC( class, strText, ... )			class.ComputeAllLog_WriteString( CDS_HydroMod_DebugLog::TIF_PoD, strText, __VA_ARGS__ )
	#define CDSHM_CAL_WRITESTRING_PREDEC( class, strText, ... )				class.ComputeAllLog_WriteString( CDS_HydroMod_DebugLog::TIF_PrD, strText, __VA_ARGS__ )
	#define CDSHM_CAL_WRITESTRING_NOTAB( class, strText, ... )				class.ComputeAllLog_WriteString( CDS_HydroMod_DebugLog::TIF_NoTab, strText, __VA_ARGS__ )
	#define CDSHM_CAL_WRITEVARBOOL( class, Name )							class.ComputeAllLog_WriteVariable( CDS_HydroMod_DebugLog::VT_Bool, CString(#Name), (LPARAM)&Name )
	#define CDSHM_CAL_WRITEVARINT( class, Name )							class.ComputeAllLog_WriteVariable( CDS_HydroMod_DebugLog::VT_Int, CString(#Name), (LPARAM)&Name )
	#define CDSHM_CAL_WRITEVARDOUBLE( class, Name )							class.ComputeAllLog_WriteVariable( CDS_HydroMod_DebugLog::VT_Double, CString(#Name), (LPARAM)&Name )
	#define CDSHM_CAL_WRITEVARCSTRING( class, Name )						class.ComputeAllLog_WriteVariable( CDS_HydroMod_DebugLog::VT_CString, CString(#Name), (LPARAM)&Name )
	#define CDSHM_CAL_WRITEBYPASSMODULELIST( class, pRootHM, pBypassMap )	class.ComputeAllLog_WriteBypassModuleList( pRootHM, pBypassMap )
	#define CDSHM_CAL_WRITEMODULATINGCVLIST( class, pRootHM, pModCVVector )	class.ComputeAllLog_WriteModulatingCvList( pRootHM, pModCVVector )
#else
	#define CDSHM_CAL_STARTLOG( class )
	#define CDSHM_CAL_STOPLOG( class )
	#define CDSHM_CAL_CHANGETABINDENT( class, TabFct )
	#define CDSHM_CAL_WRITESTRING( class, strText, ... )
	#define CDSHM_CAL_WRITESTRING_POSTINC( class, strText, ... )
	#define CDSHM_CAL_WRITESTRING_PREINC( class, strText, ... )
	#define CDSHM_CAL_WRITESTRING_POSTDEC( class, strText, ... )
	#define CDSHM_CAL_WRITESTRING_PREDEC( class, strText, ... )
	#define CDSHM_CAL_WRITESTRING_NOTAB( class, strText, ... )
	#define CDSHM_CAL_WRITEVARBOOL( class, Name )
	#define CDSHM_CAL_WRITEVARINT( class, Name )
	#define CDSHM_CAL_WRITEVARDOUBLE( class, Name )
	#define CDSHM_CAL_WRITEVARCSTRING( class, Name )
	#define CDSHM_CAL_WRITEBYPASSMODULELIST( class, pRootHM, pBypassMap )
	#define CDSHM_CAL_WRITEMODULATINGCVLIST( class, pRootHM, pModCVMap )
#endif

#ifdef _DEBUG

class CDS_HydroMod;
class CDS_HydroMod_DebugLog
{
public:
	CDS_HydroMod_DebugLog();
	~CDS_HydroMod_DebugLog();

	enum TabIndentFct
	{
		TIF_Nothing		= 0x0000,
		TIF_Inc			= 0x0001,
		TIF_Dec			= 0x0002,
		TIF_NoTab		= 0x0004,
		TIF_IDMask		= 0x000F,
		TIF_Post		= 0x0010,
		TIF_Pre			= 0x0020,
		TIF_PPMask		= 0x00F0,
		TIF_CRBef		= 0x0100,
		TIF_CRAfter		= 0x0200,
		TIF_CRMask		= 0x0F00,
		TIF_PoI		= ( TIF_Inc | TIF_Post ),
		TIF_PrI		= ( TIF_Inc | TIF_Pre ),
		TIF_PoD		= ( TIF_Dec | TIF_Post ),
		TIF_PrD		= ( TIF_Dec | TIF_Pre ),
		TIF_PoICRb	= ( TIF_PoI | TIF_CRBef ),
		TIF_PoICRa	= ( TIF_PoI | TIF_CRAfter ),
		TIF_PrICRb	= ( TIF_PrI | TIF_CRBef ),
		TIF_PrICRa	= ( TIF_PrI | TIF_CRAfter ),
		TIF_PoDCRb	= ( TIF_PoD | TIF_CRBef ),
		TIF_PoDCRa	= ( TIF_PoD | TIF_CRAfter ),
		TIF_PrDCRb	= ( TIF_PrD | TIF_CRBef ),
		TIF_PrDCRa	= ( TIF_PrD | TIF_CRAfter )
	};

	enum VariableType
	{
		VT_Bool,
		VT_Int,
		VT_Double,
		VT_CString
	};

	void ComputeAllLog_Activate( bool bActivate );
	void ComputeAllLog_SetLogFileName( CString strLogFileName );

	void ComputeAllLog_OpenLogfile();
	void ComputeAllLog_CloseLogfile();
	void ComputeAllLog_ChangeTabIndent( TabIndentFct eTabIndentFct );
	void ComputeAllLog_WriteString( TabIndentFct eTabIndentFct, CString strText, ... );
	void ComputeAllLog_WriteVariable( VariableType eVariableType, CString strVariableName, LPARAM lpParam );
	void ComputeAllLog_WriteBypassModuleList( CDS_HydroMod *pRootHM, std::multimap<int, CDS_HydroMod *> *pBypassModuleMap );
	void ComputeAllLog_WriteModulatingCvList( CDS_HydroMod *pRootHM, std::vector<CDS_HydroMod *> *pModCVVector );

// Private members.
private:
	void _TabIndent( TabIndentFct eTabIndentFct );

// Private variables.
private:
	bool m_bActivated;
	CFile m_clFile;
	CString m_strFileName;
	CString m_strTabIndent;
	int m_iTabLength;
};

#else

class CDS_HydroMod_DebugLog
{
public:
	CDS_HydroMod_DebugLog() {}
	~CDS_HydroMod_DebugLog() {}
};

#endif
