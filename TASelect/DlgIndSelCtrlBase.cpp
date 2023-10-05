#include "stdafx.h"
#include <errno.h>

#include "Taselect.h"
#include "MainFrm.h"

#include "RViewSSelSS.h"

#include "DlgIndSelBase.h"
#include "DlgIndSelCtrlBase.h"

CDlgIndSelCtrlBase::CDlgIndSelCtrlBase( CIndSelCtrlParamsBase &clIndSelCtrlParams, UINT nID, CWnd *pParent )
	: CDlgIndSelBase( clIndSelCtrlParams, nID, pParent )
{
	m_pclIndSelCtrlParams = &clIndSelCtrlParams;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void CDlgIndSelCtrlBase::FillCombo2w3w( CString strTableID, CDB_ControlProperties::CV2W3W eCv2w3w )
{
	CExtNumEditComboBox *pCB2w3w = ( CExtNumEditComboBox * )GetDlgItem( IDC_COMBO2W3W );
	CTable *pTab = ( CTable * )( m_pclIndSelCtrlParams->m_pTADB->Get( ( LPCTSTR )strTableID ).MP );

	if( NULL == pCB2w3w || NULL == pTab )
	{
		ASSERT_RETURN;
	}

	pCB2w3w->ResetContent();

	// Fill an array with all possibilities.
	bool ar[CDB_ControlProperties::CV2W3W::LastCV2W3W];
	memset( ar, 0, sizeof( ar ) );

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( IDPtr.MP );

		if( NULL == pclControlValve )
		{
			ASSERT( 0 );
			continue;
		}

		if( false == pclControlValve->IsSelectable( true ) )
		{
			continue;
		}

		CDB_ControlProperties::CV2W3W CtrlProp2w3w = ( ( CDB_ControlProperties * )pclControlValve->GetCtrlPropIDPtr().MP )->GetCv2w3w();
		ASSERT( CtrlProp2w3w < CDB_ControlProperties::CV2W3W::LastCV2W3W );
		ar[CtrlProp2w3w] = true;
		bool fAllChecked = true;

		for( int i = 0; i < CDB_ControlProperties::CV2W3W::LastCV2W3W && true == fAllChecked; i++ )
		{
			if( false == ar[i] )
			{
				fAllChecked = false;
			}
		}

		if( true == fAllChecked )
		{
			break;
		}
	}

	int iSelPos = 0;

	for( int i = 0; i < CDB_ControlProperties::CV2W3W::LastCV2W3W; i++ )
	{
		if( true == ar[i] )
		{
			CString str = CDB_ControlProperties::GetCv2W3WStr( ( CDB_ControlProperties::CV2W3W )i ).c_str();
			int pos = pCB2w3w->AddString( ( LPCTSTR )str );
            pCB2w3w->SetItemData( pos, ( DWORD_PTR )i );

			if( i == eCv2w3w )
			{
				iSelPos = pos;
			}
		}
	}

	pCB2w3w->SetCurSel( iSelPos );
	m_pclIndSelCtrlParams->m_eCV2W3W = (CDB_ControlProperties::CV2W3W)pCB2w3w->GetItemData( iSelPos );

	if( pCB2w3w->GetCount() <= 1 )
	{
		pCB2w3w->EnableWindow( false );
	}
	else
	{
		pCB2w3w->EnableWindow( true );
	}
}

void CDlgIndSelCtrlBase::FillComboCtrlType( CTADatabase::CvTargetTab eTargetTab, CDB_ControlProperties::CvCtrlType eCvCtrlType )
{
	CExtNumEditComboBox *pCBCtrlType = ( CExtNumEditComboBox * )GetDlgItem( IDC_COMBOCTRLTYPE );
	CExtNumEditComboBox *pCBType = ( CExtNumEditComboBox * )GetDlgItem( IDC_COMBOTYPE );

	if( NULL == pCBCtrlType || NULL == pCBType )
	{
		ASSERT_RETURN;
	}

	CRankEx CtrlTypeList;
	m_pclIndSelCtrlParams->m_pTADB->GetTACVCtrlTypeList( &CtrlTypeList, eTargetTab, false, m_pclIndSelCtrlParams->m_eCV2W3W, pCBType->GetCBCurSelIDPtr().ID, 
			CDB_ControlProperties::LastCVFUNC, m_pclIndSelCtrlParams->m_eFilterSelection );

	pCBCtrlType->ResetContent();
	CtrlTypeList.Transfer( pCBCtrlType );
	ASSERT( 0 != pCBCtrlType->GetCount() );

	int iSelPos = 0;

	for( int i = 0; i < pCBCtrlType->GetCount(); i++ )
	{
        if( eCvCtrlType == ( CDB_ControlProperties::CvCtrlType )(DWORD_PTR)pCBCtrlType->GetItemDataPtr( i ) )
		{
			iSelPos = i;
			break;
		}
	}

	pCBCtrlType->SetCurSel( iSelPos );
	m_pclIndSelCtrlParams->m_eCvCtrlType = (CDB_ControlProperties::CvCtrlType)pCBCtrlType->GetItemData( iSelPos );

	if( pCBCtrlType->GetCount() <= 1 )
	{
		pCBCtrlType->EnableWindow( false );
	}
	else
	{
		pCBCtrlType->EnableWindow( true );
	}
}

void CDlgIndSelCtrlBase::ActivateLeftTabDialog()
{
	CDlgIndSelBase::ActivateLeftTabDialog();

	// HYS-1263 : Added to update application type if it is necessary
	CDS_TechnicalParameter *pTechP = m_pclIndSelCtrlParams->m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_pclIndSelCtrlParams->m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}
}
