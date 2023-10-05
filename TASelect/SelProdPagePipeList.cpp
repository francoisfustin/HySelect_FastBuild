#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "HydroMod.h"
#include "HMPipes.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabSelP.h"
#include "RViewDescription.h"
#include "SelProdArticle.h"
#include "SelProdPageBase.h"
#include "SelProdPagePipeList.h"

CSelProdPagePipeList::CSelProdPagePipeList( CArticleGroupList *pclArticleGroupList ) 
	: CSelProdPageBase( CDB_PageSetup::enCheck::PIPELIST, pclArticleGroupList )
{
}

void CSelProdPagePipeList::Reset( void )
{
	CSelProdPageBase::Reset();
}

bool CSelProdPagePipeList::PreInit( HMvector &vecHMList )
{
	// Something for individual or direct selection ?
	CTable *pclTable = TASApp.GetpTADS()->GetpHydroModTable();

	if( NULL == pclTable )
	{
		ASSERT( 0 );
		return false;
	}

	m_pclSelectionTable = ( _T('\0') != *pclTable->GetFirst().ID ) ? pclTable : NULL;

	if( NULL == m_pclSelectionTable )
	{
		return false;
	}
	
	_FillPipeList( m_pclSelectionTable );
	return true;
}

bool CSelProdPagePipeList::Init( bool fResetOrder, bool fPrint )
{
	if( NULL == m_pclSelectionTable || 0 == m_mapPipes.size() )
		return false;

	// Call base class method to get a new 'Sheet'.
	CSheetDescription* pclSheetDescriptionPipes = CMultiSpreadBase::CreateSSheet( SD_PipeList );
	if( NULL == pclSheetDescriptionPipes || NULL == pclSheetDescriptionPipes->GetSSheetPointer() )
		return false;
	
	CSSheet* pclSheet = pclSheetDescriptionPipes->GetSSheetPointer();
	if( false == CSelProdPageBase::PrepareSheet( pclSheetDescriptionPipes, ColumnDescriptionPipeList::Pointer, fPrint ) )
	{
		m_ViewDescription.RemoveOneSheetDescription( SD_PipeList );
		return false;
	}

	// Initialize column width for header and footer column.
	if( false == m_bForPrint )
	{
		SetColWidth( pclSheetDescriptionPipes, ColumnDescriptionPipeList::Header, DefaultColumnWidth::DCW_Header );
		SetColWidth( pclSheetDescriptionPipes, ColumnDescriptionPipeList::Footer, DefaultColumnWidth::DCW_Footer );
	}
	else
	{
		pclSheet->ShowCol( ColumnDescriptionPipeList::Header, FALSE );
		pclSheet->ShowCol( ColumnDescriptionPipeList::Footer, FALSE );
	}
	
	SetColWidth( pclSheetDescriptionPipes, ColumnDescriptionPipeList::PipeSerie, 40 ); 
	SetColWidth( pclSheetDescriptionPipes, ColumnDescriptionPipeList::PipeSize, 25); 
	SetColWidth( pclSheetDescriptionPipes, ColumnDescriptionPipeList::TotalLength, 15 );  
	SetColWidth( pclSheetDescriptionPipes, ColumnDescriptionPipeList::FluidVolume, 15 );  
	SetColWidth( pclSheetDescriptionPipes, ColumnDescriptionPipeList::Pointer, 0 );
	pclSheet->ShowCol( ColumnDescriptionPipeList::Pointer, FALSE );
	CRect rect = pclSheet->GetSheetSizeInPixels( false );
	m_mapSheetInfos[pclSheetDescriptionPipes].m_dPageWidth = rect.Width();

	// Page title.
	SetPageTitle( pclSheetDescriptionPipes, IDS_RVIEWSELP_TITLE_PIPELIST );

	// Init column header.
	_InitColHeader( pclSheetDescriptionPipes );

	// HYS-1571: We can optimize the column width now.
	// See Jira card to have more details about why we do that here.
	CRect rectClient;
	GetClientRect( &rectClient );
	OptimizePageSize( rectClient.Width(), rectClient.Height() );

	/////////////////////////////////////////////////////
	// Fill each line 
	/////////////////////////////////////////////////////
	
	long lRow = pclSheet->GetMaxRows();
	CTable* pPipeSerie = NULL;
	double dVolumeTot = 0.0;
	for( std::map<CString, PipeInfo>::iterator iter = m_mapPipes.begin(); iter != m_mapPipes.end(); iter++ )
	{
		CDB_Pipe* pPipe = iter->second.m_pclPipe;
		double dLength = iter->second.m_dLength;
		CTable* pNewTabSerie = (CTable *)( pPipe->GetIDPtr().PP );						ASSERT( NULL != pNewTabSerie );

		// Compare previous and new pipe serie; used to insert a blank line.
		if( NULL != pPipeSerie && ( pPipeSerie != pNewTabSerie ) )
		{
			AddStaticText( pclSheetDescriptionPipes, ColumnDescriptionPipeList::PipeSerie, lRow, _T("") );
			AddCellSpanW( pclSheetDescriptionPipes, ColumnDescriptionPipeList::PipeSerie, lRow, ColumnDescriptionPipeList::Footer - ColumnDescriptionPipeList::PipeSerie, 1 );
			pclSheet->SetCellParam( ColumnDescriptionPipeList::PipeSerie, lRow, 0 );
	
			// Draw line below.
			pclSheet->SetCellBorder( ColumnDescriptionPipeList::PipeSerie, lRow, ColumnDescriptionPipeList::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM );
			lRow++;
		}
		pPipeSerie = pNewTabSerie;
		long lRowPipe = _FillRow( pclSheetDescriptionPipes, lRow, pPipeSerie, pPipe, dLength, dVolumeTot);
	
		// Draw line below.
		pclSheet->SetCellBorder( ColumnDescriptionPipeList::PipeSerie, lRowPipe, ColumnDescriptionPipeList::Footer - 1, lRowPipe, true, SS_BORDERTYPE_BOTTOM );
		lRow = ++lRowPipe;
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	CString str = WriteCUDouble( _U_VOLUME, dVolumeTot, true );
	AddStaticText( pclSheetDescriptionPipes, ColumnDescriptionPipeList::FluidVolume, lRow, str );

	// Move sheet to correct position.
	SetSheetSize();
	Invalidate();
	UpdateWindow();

	return true;
}

bool CSelProdPagePipeList::HasSomethingToDisplay( void )
{
	if( NULL == m_pclSelectionTable || 0 == m_mapPipes.size() )
		return false;
	return true;
}

void CSelProdPagePipeList::_FillPipeList( CTable* pTab )
{
	if( NULL == pTab )
		return;
	
	for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{	
		CDS_HydroMod* pHM = ( CDS_HydroMod *)IDPtr.MP;
		if( NULL == pHM )
			continue;
		
		for( int i = 0; i < 4; i++ )
		{
			CPipes* pclHMPipe = NULL;
			switch( i )
			{
				case 0:
					pclHMPipe = pHM->GetpDistrSupplyPipe();
					break;

				case 1:
					pclHMPipe = pHM->GetpCircuitPrimaryPipe();
					break;

				case 2:
					pclHMPipe = pHM->GetpCircuitSecondaryPipe();
					break;

				case 3:
					pclHMPipe = pHM->GetpDistrReturnPipe();
					break;
			}
		
			if( NULL != pclHMPipe && NULL != pclHMPipe->GetIDPtr().MP && pclHMPipe->GetLength() > 0.0 )
			{
				CDB_Pipe* pclPipe = (CDB_Pipe*)( pclHMPipe->GetIDPtr().MP );
				CTable* pTabSerie = (CTable *)( pclHMPipe->GetIDPtr().PP );
				CString str;
				CString IntDiameter = WriteCUDouble( _U_LENGTH, pclPipe->GetIntDiameter() );
				str.Format( _T("%s %s"), pTabSerie->GetName(), IntDiameter );

				if( 0 != m_mapPipes.count( str ) )
				{
					std::map<CString, PipeInfo>::iterator iter = m_mapPipes.find( str );
					iter->second.m_dLength += pclHMPipe->GetLength();
				}
				else
					m_mapPipes[str] = PipeInfo( pclPipe, pclHMPipe->GetLength() );
			}
		}
		
		if( true == pHM->IsaModule() )
			_FillPipeList( pHM );
	}
}

void CSelProdPagePipeList::_InitColHeader( CSheetDescription* pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		return;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	CString str;

	// Format columns header.
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_WHITE );
	
	str = TASApp.LoadLocalizedString( IDS_SSELPRODHDRCOL_PIPESERIE );
	AddStaticText( pclSheetDescription, ColumnDescriptionPipeList::PipeSerie, CSelProdPageBase::HR_RowHeader, str );
	
	str = TASApp.LoadLocalizedString( IDS_SSELPRODHDRCOL_PIPESIZE );
	AddStaticText( pclSheetDescription, ColumnDescriptionPipeList::PipeSize, CSelProdPageBase::HR_RowHeader, str );
	
	str = TASApp.LoadLocalizedString( IDS_SSELPRODHDRCOL_TOTALLENGTH );
	CString str1; 
	str1.Format( _T("%s (%s)"), str, m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	AddStaticText( pclSheetDescription, ColumnDescriptionPipeList::TotalLength, CSelProdPageBase::HR_RowHeader, str1 );
	
	str = TASApp.LoadLocalizedString( IDS_SSELPRODHDRCOL_FLUIDVOLUME );
	str1.Format( _T("%s (%s)"), str, m_pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str() );
	AddStaticText( pclSheetDescription, ColumnDescriptionPipeList::FluidVolume, CSelProdPageBase::HR_RowHeader, str1 );

	// Draw border.
	pclSheet->SetCellBorder( ColumnDescriptionPipeList::PipeSerie, CSelProdPageBase::HR_RowHeader, ColumnDescriptionPipeList::Footer - 1, CSelProdPageBase::HR_RowHeader, true, SS_BORDERTYPE_BOTTOM | SS_BORDERTYPE_TOP );

	// Freeze row header.
	pclSheet->SetFreeze( 0, CSelProdPageBase::HR_RowHeader );
}

long CSelProdPagePipeList::_FillRow( CSheetDescription* pclSheetDescription, long lRow, CTable* pTabSerie, CDB_Pipe* pPipe, double dLength, double &dVolumeTot )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pTabSerie || NULL == pPipe )
		return lRow;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	AddStaticText( pclSheetDescription, ColumnDescriptionPipeList::PipeSerie, lRow, pTabSerie->GetName() );

	// Save pointer on pipe series.
	pclSheet->SetCellParam( ColumnDescriptionPipeList::PipeSerie, lRow, (long)pTabSerie );
	AddStaticText( pclSheetDescription, ColumnDescriptionPipeList::PipeSize, lRow, pPipe->GetName() );
	
	CString str = WriteCUDouble( _U_LENGTH, dLength, false );
	AddStaticText( pclSheetDescription, ColumnDescriptionPipeList::TotalLength, lRow, str );

	// Compute and display volume.
	double dVolume = dLength * pPipe->GetFluidVolumeByMeter();
	dVolumeTot += dVolume;
	str = WriteCUDouble( _U_VOLUME, dVolume, false );
	AddStaticText( pclSheetDescription, ColumnDescriptionPipeList::FluidVolume, lRow, str );

	return pclSheet->GetMaxRows();
}
