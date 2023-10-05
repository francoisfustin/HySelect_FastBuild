#include "stdafx.h"


#include "MainFrm.h"
#include "TASelect.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "ImportExportHMNodeDefinitions.h"
#include "ImportHMBase.h"
#include "ImportHMFromXML.h"

bool CImportHMFromXML::OpenFile( CString strFileName, CString &strErrorMsg )
{
	strErrorMsg = _T("");
	return true;
}

CString CImportHMFromXML::GetNodeText( NodeDefinition &nodeMain )
{
	return _T("");
}

bool CImportHMFromXML::GetSubNode( NodeDefinition &nodeMain, CString strNode, NodeDefinition &nodeSub )
{
	return true;
}
