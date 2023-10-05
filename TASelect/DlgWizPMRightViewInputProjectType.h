#pragma once

#include "ExtNumEdit.h"
#include "DialogExt.h"
#include "SelectPM.h"
#include "DlgWizPMRightViewInputBase.h"

class CDlgWizardPM_RightViewInput_ProjectType : public CDlgWizardPM_RightViewInput_Base
{

public:
	enum { IDD = IDD_DLGWIZPM_RIGHTVIEWINPUT_PROJECTTYPE };

	CDlgWizardPM_RightViewInput_ProjectType( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent );
	~CDlgWizardPM_RightViewInput_ProjectType() {}

protected:
	virtual BOOL OnInitDialog();
};
