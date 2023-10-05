#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "HydroMod.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabSelP.h"
#include "RViewDescription.h"
#include "SelProdArticle.h"
#include "SelProdPageBase.h"
#include "SelProdPageHub.h"

CSelProdPageHub::CSelProdPageHub( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::HUB, pclArticleGroupList )
{
	m_pSelected = NULL;
}

void CSelProdPageHub::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPageHub::HasSomethingToDisplay( HMvector& vecHMList )
{
	return false;
}

bool CSelProdPageHub::Init( bool fResetOrder, bool fPrint )
{
	return false;
}
