#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"

#include "SelectPM.h"
#include "DlgWizPMRightViewInputProjectType.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_ProjectType::CDlgWizardPM_RightViewInput_ProjectType( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, RightViewInputDialog::ProjectType, IDD, pclParent )
{
}

BOOL CDlgWizardPM_RightViewInput_ProjectType::OnInitDialog()
{
	CDlgWizardPM_RightViewInput_Base::OnInitDialog();

	// TODO

	return TRUE;
}
