#include "stdafx.h"


#include <map>
#include <vector>
#include <algorithm>
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "DataBObj.h"
#include "HydroMod.h"
#include "HMPipes.h"
#include "HubHandler.h"
#include "RViewDescription.h"
#include "SSheet.h"
#include "SelProdArticle.h"
#include "SelProdPageBase.h"


CProductParam::CProductParam() { Clear(); }

CProductParam::CProductParam( CSheetDescription* pclSheetDescription, long lSelectionTopRow, long lSelectionBottomRow, LPARAM lpSelectionContainer, CData* pclProduct )
{
	Clear();
	m_pclSheetDescription = pclSheetDescription;
	AddRange( lSelectionTopRow, lSelectionBottomRow, pclProduct );
	m_lpSelectionContainer = lpSelectionContainer;
}

CProductParam::CProductParam( const CProductParam& rProductParam )
{
	m_pclSheetDescription = rProductParam.m_pclSheetDescription;
	m_mapSelectionRangeList = rProductParam.m_mapSelectionRangeList;
	m_lSelectionTopRow = rProductParam.m_lSelectionTopRow;
	m_lSelectionBottomRow = rProductParam.m_lSelectionBottomRow;
	m_lpSelectionContainer = rProductParam.m_lpSelectionContainer;
	m_vecLinkedProductParam = rProductParam.m_vecLinkedProductParam;
	m_lScrollTopRow = rProductParam.m_lScrollTopRow;
	m_lScrollBottomRow = rProductParam.m_lScrollBottomRow;
}

bool CProductParam::operator == ( const CProductParam &rProductParam )
{
	if( m_pclSheetDescription != rProductParam.m_pclSheetDescription || m_lSelectionBottomRow != rProductParam.m_lSelectionTopRow 
			|| m_lSelectionTopRow != rProductParam.m_lSelectionTopRow || m_lpSelectionContainer != rProductParam.m_lpSelectionContainer 
			|| m_mapSelectionRangeList.size() != rProductParam.m_mapSelectionRangeList.size() || m_vecLinkedProductParam.size() != rProductParam.m_vecLinkedProductParam.size() 
			|| m_lScrollTopRow != rProductParam.m_lScrollTopRow || m_lScrollBottomRow != rProductParam.m_lScrollBottomRow )
	{
		return false;
	}

	mapPairLLCData::iterator iter1 = m_mapSelectionRangeList.begin();
	mapPairLLCData::const_iterator iter2 = rProductParam.m_mapSelectionRangeList.begin();
	
	for( ; iter1 != m_mapSelectionRangeList.end(); iter1++, iter2++ )
	{
		if( iter1->first.first != iter2->first.first || iter1->first.second != iter2->first.second || iter1->second != iter2->second )
		{
			return false;
		}
	}

	for( int i = 0 ; i < (int)m_vecLinkedProductParam.size(); i++ )
	{
		if( ! (m_vecLinkedProductParam[i] == rProductParam.m_vecLinkedProductParam[i] ) )
		{
			return false;
		}
	}

	return true;
}

void CProductParam::Clear( void )
{
	m_pclSheetDescription = NULL;
	m_mapSelectionRangeList.clear();
	m_lSelectionTopRow = -1;
	m_lSelectionBottomRow = -1;
	m_lpSelectionContainer = 0;
	m_vecLinkedProductParam.clear();
	m_lScrollTopRow = -1;
	m_lScrollBottomRow = -1;
}

void CProductParam::AddRange( long lRowStart, long lRowEnd, CData *pclProduct )
{
	m_mapSelectionRangeList[std::pair<long, long>( lRowStart, lRowEnd )] = pclProduct;

	if( -1 == m_lSelectionTopRow || lRowStart < m_lSelectionTopRow )
	{
		m_lSelectionTopRow = lRowStart;
	}
	
	if( -1 == m_lSelectionBottomRow || lRowEnd > m_lSelectionBottomRow )
	{
		m_lSelectionBottomRow = lRowEnd;
	}
}

void CProductParam::UpdateRange( long lRowStartToUpdate, long lRowEndToUpdate, long lRowStartUpdate, long lRowEndUpdate )
{
	mapPairLLCData::iterator iter = m_mapSelectionRangeList.find( std::pair<long, long>( lRowStartToUpdate, lRowEndToUpdate ) );

	if( iter == m_mapSelectionRangeList.end() ) 
	{
		return;
	}
	
	CData *pclProduct = iter->second;
	m_mapSelectionRangeList.erase( iter );
	AddRange( lRowStartUpdate, lRowEndUpdate, pclProduct );
}

bool CProductParam::isRowInRange( long lRow )
{
	bool bInRange = false;

	for( mapPairLLCData::iterator iter = m_mapSelectionRangeList.begin(); iter != m_mapSelectionRangeList.end(); iter++ )
	{
		if( lRow >= iter->first.first && lRow <= iter->first.second ) 
		{
			bInRange = true;
		}
	}
	
	return bInRange;
}

CData *CProductParam::GetProduct( long lRow )
{
	if( 0 == (int)m_mapSelectionRangeList.size() )
	{
		return NULL;
	}

	CData *pclProduct = NULL;

	for( mapPairLLCData::iterator iter = m_mapSelectionRangeList.begin(); iter != m_mapSelectionRangeList.end(); iter++ )
	{
		if( lRow >= iter->first.first && lRow <= iter->first.second )
		{
			pclProduct = iter->second;
			break;
		}
	}

	return pclProduct;
}

void CProductParam::GetScrollRange( long &lScrollTopRow, long &lScrollBottomRow )
{
	lScrollTopRow = ( -1 != m_lScrollTopRow ) ? m_lScrollTopRow : m_lSelectionTopRow;
	lScrollBottomRow = ( -1 != m_lScrollBottomRow ) ? m_lScrollBottomRow : m_lSelectionBottomRow;
}

bool CProductParam::GetFirstSelectioRange( long &lFirstRow, long &lLastRow )
{
	m_mapSelectionIter = m_mapSelectionRangeList.begin();
	return GetNextSelectionRange( lFirstRow, lLastRow );
}

bool CProductParam::GetNextSelectionRange( long &lFirstRow, long &lLastRow )
{
	bool bReturn = false;

	if( m_mapSelectionRangeList.end() != m_mapSelectionIter )
	{
		lFirstRow = m_mapSelectionIter->first.first;
		lLastRow = m_mapSelectionIter->first.second;
		m_mapSelectionIter++;
		bReturn = true;
	}
	
	return bReturn;
}

CSelProdPageBase::CSelProdPageBase( CDB_PageSetup::enCheck ePage, CArticleGroupList *pclArticleGroupList, bool fUseOnlyOneSpread ) : CMultiSpreadInDialog( fUseOnlyOneSpread )
{
	Reset();
	m_pclArticleGroupList = pclArticleGroupList;
	m_ePageNum = ePage;
}

void CSelProdPageBase::Reset( void )
{
	m_bInitialized = false;
	m_vecProductSelected.clear();
	m_iProductSelectedIndex = -1;
	m_mapSheetInfos.clear();
	m_bForPrint = false;
	m_dRowHeight = 12.75;
	m_vecHMList.clear();
	m_pclSelectionTable = NULL;

	m_pTADB = TASApp.GetpTADB();
	ASSERT( NULL != m_pTADB );

	m_pTADS = TASApp.GetpTADS();
	ASSERT( NULL != m_pTADS );

	m_pTADSPageSetup = m_pTADS->GetpPageSetup();
	ASSERT( NULL != m_pTADSPageSetup );

	m_pUnitDB = CDimValue::AccessUDB();
	ASSERT( NULL != m_pUnitDB );

	m_pclTechParam = TASApp.GetpTADS()->GetpTechParams();
	ASSERT( NULL != m_pclTechParam );

	m_mapTooltipMessages.clear();

	CMultiSpreadInDialog::Reset();
}

void CSelProdPageBase::OptimizePageSize( int iWidth, int iHeight )
{
	if( 0 == m_mapSheetInfos.size() )
	{
		return;
	}

	CRect rectSheetTotalSize = GetTotalSize();

	for( mapSheetInfoIter iter = m_mapSheetInfos.begin(); iter != m_mapSheetInfos.end(); iter++ )
	{
		if( NULL == iter->first || NULL == iter->first->GetSSheetPointer() )
		{
			continue;
		}

		CSheetDescription *pclSheetDescription = iter->first;
		CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
		CRect rectSheet = pclSheet->GetSheetSizeInPixels();

		// Check if a vertical scroll bar exist.
		int iWidthPage = iWidth;

		if( rectSheetTotalSize.Height() > iHeight )
		{
			iWidthPage -= GetSystemMetrics( SM_CXVSCROLL );
		}

		// First step: make a list of all visible column .
		std::vector<int> vecColumnList;
		long lPageWidth = 0;

		for( int iLoopColumn = 1; iLoopColumn <= m_mapSheetInfos[pclSheetDescription].m_iColNum - 1; iLoopColumn++ )
		{
			if( FALSE == pclSheet->IsColHidden( iLoopColumn ) )
			{
				vecColumnList.push_back( iLoopColumn );
				lPageWidth += m_mapSheetInfos[pclSheetDescription].m_mapColWidth[iLoopColumn];
			}
		}

		// Second step: compute the factor to apply.
		double dFactor = (double)iWidthPage / lPageWidth;
		dFactor = max( 1, dFactor );
		int iColumnListSize = (int)vecColumnList.size();

		if( dFactor > 1.0 )
		{
			// Adapt width to the dialog container.
			int iHeightToSet = ( true == UseOnlyOneSpread() ) ? iHeight : rectSheet.Height();
			pclSheet->SetWindowPos( NULL, -1, -1, iWidth, iHeightToSet, SWP_NOMOVE | SWP_NOZORDER );

			int iTotalWidth = 0;

			// Third step: we set the header and footer column. To have the same width around the content.
			// Remark: header column must be set to the first position (index 1).
			//         pointer column is the last (=m_iColNum).
			//         footer column must be set just before the pointer column (m_iColNum-1).
			int iColumnWidth = (int)( (double)m_mapSheetInfos[pclSheetDescription].m_mapColWidth[1] * dFactor );
			pclSheet->SetColWidthInPixels( 1, iColumnWidth );
			pclSheet->SetColWidthInPixels( m_mapSheetInfos[pclSheetDescription].m_iColNum - 1, iColumnWidth );
			iTotalWidth += ( 2 * iColumnWidth );

			// Fourth step: we can now optimize column width between header and footer to perfectly match the actual width.
			for( int iIndex = 1; iIndex < iColumnListSize - 1; iIndex++ )
			{
				int iColumn = vecColumnList[iIndex];
				iColumnWidth = (int)( (double)m_mapSheetInfos[pclSheetDescription].m_mapColWidth[iColumn] * dFactor );

				if( iIndex < iColumnListSize - 2 )
				{
					iTotalWidth += iColumnWidth;
				}
				else
				{
					// The last column before the footer will take the difference to perfectly match the actual width.
					iColumnWidth = iWidthPage - iTotalWidth;
				}

				pclSheet->SetColWidthInPixels( iColumn, iColumnWidth );
			}
		}
		else
		{
			// Adapt width to the real width of the spread.
			int iRectSheetWidth = rectSheet.Width();

			if( rectSheetTotalSize.Height() > iHeight )
			{
				iRectSheetWidth += GetSystemMetrics( SM_CXVSCROLL );
			}

			int iHeightToSet = ( true == UseOnlyOneSpread() ) ? iHeight : rectSheet.Height();
			pclSheet->SetWindowPos( NULL, -1, -1, iRectSheetWidth, iHeightToSet, SWP_NOMOVE | SWP_NOZORDER );

			for( int iIndex = 0; iIndex < iColumnListSize; iIndex++ )
			{
				int iColumn = vecColumnList[iIndex];
				pclSheet->SetColWidthInPixels( iColumn, m_mapSheetInfos[pclSheetDescription].m_mapColWidth[iColumn] );
			}
		}
	}
}

LPARAM CSelProdPageBase::BackupSelectedProduct( void )
{
	BackupPosSel* pBackup = new BackupPosSel;
	if( NULL == pBackup )
		return (LPARAM)0;

	if( 0 != m_vecProductSelected.size() && -1 != m_iProductSelectedIndex && m_iProductSelectedIndex < (int)m_vecProductSelected.size() )
	{
		if( NULL != m_vecProductSelected[m_iProductSelectedIndex].m_pclSheetDescription )
		{
			pBackup->m_uiSheetDescriptionID = m_vecProductSelected[m_iProductSelectedIndex].m_pclSheetDescription->GetSheetDescriptionID();
			pBackup->m_lpParam = m_vecProductSelected[m_iProductSelectedIndex].m_lpSelectionContainer;
		}
	}
	pBackup->m_iCheckMemID = 0x12435687;
	return (LPARAM)pBackup;
}

bool CSelProdPageBase::RestoreSelectedProduct( LPARAM lpParam )
{
	if( NULL == lpParam )
		return false;
	
	// Verify memory.
	BackupPosSel* pBackup = (BackupPosSel*)lpParam;
	if( 0x12435687 != pBackup->m_iCheckMemID )
		return false;
	
	// Verify if product is exist.
	CProductParam clProductParam;
	CSheetDescription* pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( pBackup->m_uiSheetDescriptionID );
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || false == _IsProductRegistered( pclSheetDescription, pBackup->m_lpParam, clProductParam ) )
		return false;

	// Select it.
	_SelectCell( clProductParam );

	// Do the scrolling.
	// 'NULL' to specify that 'rectPixel' coordinates are relative to the window and not the current sheet description.
	CRect rectPixel = _GetScrollingRange( clProductParam );
	SetScrollPosition( ScrollPosition::ScrollToCompute, NULL, rectPixel, true );

	return true;
}

void CSelProdPageBase::FreeSelectedProduct( LPARAM lpParam )
{
	BackupPosSel* pBackup = (BackupPosSel*)lpParam;
	if( NULL != pBackup && 0x12435687 == pBackup->m_iCheckMemID )
		delete pBackup;
}

int CSelProdPageBase::GetSSelSelectedNumbers( void )
{
	return (int)m_vecProductSelected.size();
}

bool CSelProdPageBase::GetFirstSSelSelected( CString& strClassName, LPARAM& lpObj )
{
	bool fReturnValue = false;
	if( m_vecProductSelected.size() > 0 )
	{
		CProductParam clProductParam;
		clProductParam.Clear();
		for( vecProductSelectedIter iter = m_vecProductSelected.begin(); iter != m_vecProductSelected.end(); iter++ )
		{
			if( NULL == clProductParam.m_pclSheetDescription )
				clProductParam = *iter;
			else if( (*iter).m_lSelectionBottomRow < clProductParam.m_lSelectionTopRow )
				clProductParam = *iter;
		}
		
		if( NULL != clProductParam.m_pclSheetDescription )
		{
			lpObj = clProductParam.m_lpSelectionContainer;
			CData* pData = (CData*)lpObj;
			if( NULL != pData )
			{
				strClassName = pData->GetClassName();
			}
			fReturnValue = true;
		}
	}
	return fReturnValue;
}

int CSelProdPageBase::GetAllSSelSelected( CString& strClassName, CArray<LPARAM>& arlpObj )
{
	int iCount = 0;
	for( vecProductSelectedIter iter = m_vecProductSelected.begin(); iter != m_vecProductSelected.end(); iter++ )
	{
		if( (LPARAM)0 != (*iter).m_lpSelectionContainer )
		{
			iCount++;
			arlpObj.Add( (*iter).m_lpSelectionContainer );
			if( true == strClassName.IsEmpty() )
			{
				CData* pData = (CData*)( (*iter).m_lpSelectionContainer );
				if( NULL != pData )
				{
					strClassName = pData->GetClassName();
				}
			}
		}
	}
	return iCount;
}

bool CSelProdPageBase::GetSSelUnderMouse( CPoint ptMousePosScreen, CData** ppclSSelObject, CData** ppclProduct )
{
	if( NULL == ppclSSelObject )
		return false;

	*ppclSSelObject = NULL;
	if( NULL != ppclProduct )
		*ppclProduct = NULL;

	CSheetDescription* pclSheetDescription;
	long lColumn;
	long lRow;
	GetInfoUnderMouse( ptMousePosScreen, pclSheetDescription, lColumn, lRow );

	bool fReturn = false;
	if( NULL != pclSheetDescription && -1 != lRow )
	{
		CProductParam clProductParam;
		if( true == _GetProduct( pclSheetDescription, lRow, clProductParam ) )
		{
			*ppclSSelObject = (CData*)clProductParam.m_lpSelectionContainer;
			if( NULL != ppclProduct )
				*ppclProduct = clProductParam.GetProduct( lRow );
			fReturn = true;
		}
	}
	return fReturn;
}

void CSelProdPageBase::ClickOnSSel( LPARAM lpObj )
{
	CProductParam clProductParam;
	if( true == _IsProductExist( lpObj, clProductParam ) )
	{
		LockWindowUpdate();
		_SelectCell( clProductParam );
		UnlockWindowUpdate();
	}
}

void CSelProdPageBase::ClearAllProductSelected( void )
{
	m_vecProductSelected.clear();
}

void CSelProdPageBase::SelectFirstProduct( CSheetDescription *pclSheetDescription )
{
	CSheetDescription *pclSearchSD = NULL;

	if( NULL == pclSheetDescription )
	{
		pclSearchSD = m_ViewDescription.GetTopSheetDescription();
	}
	else
	{
		pclSearchSD = pclSheetDescription;
	}

	while( NULL != pclSearchSD )
	{
		if( 0 != m_mapSheetInfos.count( pclSearchSD ) && 0 != m_mapSheetInfos[pclSearchSD].m_mapProductRegistered.size() )
		{
			_SelectCell( m_mapSheetInfos[pclSearchSD].m_mapProductRegistered.begin()->second );
			pclSearchSD = NULL;
		}
		else
		{
			if( NULL != pclSheetDescription )
			{
				pclSearchSD = NULL;
			}
			else
			{
				pclSearchSD = m_ViewDescription.GetNextSheetDescription( pclSearchSD );
			}
		}
	}
}

void CSelProdPageBase::SSheetOnAfterPaint()
{
	_DrawSelectionBorders();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Spread DLL message handlers
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSelProdPageBase::OnCellClicked( CSheetDescription* pclSheetDescription, long lClickedCol, long lClickedRow )
{
	if( false == m_bInitialized )
		return;

	// Variable.
	CSSheet::_SSKeyBoardPressed eKeyPressed;

	if( GetKeyState( VK_LSHIFT ) < 0 || GetKeyState( VK_RSHIFT ) < 0 )
	{
		// In case the Shift key were pressed.
		eKeyPressed = CSSheet::_SSKeyBoardPressed::ShiftKey;
	}
	else if( GetKeyState( VK_LCONTROL ) < 0 || GetKeyState( VK_RCONTROL ) < 0 )
	{
		// In case the Ctrl key were pressed.
		eKeyPressed = CSSheet::_SSKeyBoardPressed::CtrlKey;
	}
	else
	{
		// In case no key where pressed.
		eKeyPressed = CSSheet::_SSKeyBoardPressed::NoKey;
	}

	if( NULL != pclSheetDescription )
	{
		LockWindowUpdate();
		CProductParam clProductParam;
		_GetProduct( pclSheetDescription, lClickedRow, clProductParam );
		_SelectCell( clProductParam, eKeyPressed );
		UnlockWindowUpdate();
	}
}

void CSelProdPageBase::OnCellDblClicked( CSheetDescription* pclSheetDescription, long lClickedCol, long lClickedRow )
{
	if( false == m_bInitialized || NULL == pclSheetDescription )
		return;
	
	CProductParam clProductParam;
	_GetProduct( pclSheetDescription, lClickedRow, clProductParam );

	if( m_pTADB->CanEditSelection( (CData*)clProductParam.m_lpSelectionContainer ) )
	{
		pMainFrame->PostMessageToDescendants( WM_USER_SELPRODEDITPRODUCT, clProductParam.m_lpSelectionContainer );
	}
}

BOOL CSelProdPageBase::OnKeyDown( CSheetDescription *pclSheetDescription, WPARAM wKeyPressed )
{
	if( false == m_bInitialized || NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return FALSE;
	}

	// Do nothing if user press Up or Down with control key (bad behavior).
	if( GetKeyState( VK_LCONTROL ) < 0 || GetKeyState( VK_RCONTROL ) < 0 )
	{
		return FALSE;
	}

	ScrollPosition eScrollPosition = ScrollPosition::NoScroll;

	int iSheetDescriptionID = pclSheetDescription->GetSheetDescriptionID();
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();

	// By default, don't dispatch the message.
	BOOL bReturn = TRUE;
	long lActiveColumn = pclSheet->GetActiveCol();
	long lActiveRow = pclSheet->GetActiveRow();
	bool bTopPreference = true;
	long lRowReference = 0;
	CProductParam clDestProductParam;

	switch( wKeyPressed )
	{
		case VK_DOWN:
			{
				CProductParam clFromProductParam;

				if( 0 == m_vecProductSelected.size() || -1 == m_iProductSelectedIndex || m_iProductSelectedIndex >= (int)m_vecProductSelected.size() )
				{
					_GetProduct( pclSheetDescription, lActiveRow, clFromProductParam );
				}
				else
				{
					bool bShift = ( GetKeyState( VK_LSHIFT ) < 0 || GetKeyState( VK_RSHIFT ) < 0 );
					int iIndex = m_iProductSelectedIndex;
					
					if( true == bShift )
					{
						iIndex = ( 0 == m_iProductSelectedIndex ) ? m_vecProductSelected.size() - 1 : 0;
					}
					
					clFromProductParam = m_vecProductSelected[iIndex];
				}

				_GetNextProduct( clFromProductParam, clDestProductParam );
				
				CSheetDescription *pclDestSheetDescription = clDestProductParam.m_pclSheetDescription;

				if( NULL != pclDestSheetDescription && NULL != pclDestSheetDescription->GetSSheetPointer() &&
						0 != clDestProductParam.m_lSelectionTopRow && 0 != clDestProductParam.m_lSelectionBottomRow && (LPARAM)0 != clDestProductParam.m_lpSelectionContainer )
				{
					OnCellClicked( pclDestSheetDescription, lActiveColumn, clDestProductParam.m_lSelectionTopRow );
					lRowReference = clDestProductParam.m_lScrollBottomRow;
					// pclDestSheetDescription->GetSSheetPointer()->SetActiveCell( lActiveColumn, clDestProductParam.m_lRowUpper );
					eScrollPosition = ScrollPosition::ScrollToCompute;
					bTopPreference = false;
				}
				else
				{
					// If we don't find any more product, we click on the same one and force to scroll to the bottom.
					OnCellClicked( pclSheetDescription, lActiveColumn, clFromProductParam.m_lSelectionTopRow );
					eScrollPosition = ScrollPosition::ScrollToBottom;
				}
			}
			break;

		case VK_UP:
			{
				CProductParam clFromProductParam;

				if( 0 == m_vecProductSelected.size() || -1 == m_iProductSelectedIndex || m_iProductSelectedIndex >= (int)m_vecProductSelected.size() )
				{
					_GetProduct( pclSheetDescription, lActiveRow, clFromProductParam );
				}
				else
				{
					bool bShift = ( GetKeyState( VK_LSHIFT ) < 0 || GetKeyState( VK_RSHIFT ) < 0 );
					int iIndex = m_iProductSelectedIndex;

					if( true == bShift )
					{
						iIndex = ( 0 == m_iProductSelectedIndex ) ? m_vecProductSelected.size() - 1 : 0;
					}
					clFromProductParam = m_vecProductSelected[iIndex];
				}

				_GetPrevProduct( clFromProductParam, clDestProductParam );
				
				CSheetDescription *pclDestSheetDescription = clDestProductParam.m_pclSheetDescription;

				if( NULL != pclDestSheetDescription && NULL != pclDestSheetDescription->GetSSheetPointer() 
						&& 0 != clDestProductParam.m_lSelectionTopRow && 0 != clDestProductParam.m_lSelectionBottomRow && (LPARAM)0 != clDestProductParam.m_lpSelectionContainer )
				{
// 					if( true == fIsTheFirst )
// 					{
// 						// Remark: Left top cell is not necessarily the cell that is in the first position (first row, first col) in the view.
// 						//         If there is some cols or rows frozen, and no current scroll, left top cell can be set to the row 5 for example.
// 						SS_COORD lColsFrozen, lRowsFrozen;
// 						pclDestSheetDescription->GetSSheetPointer()->GetFreeze( &lColsFrozen, &lRowsFrozen );
// 						
// 						OnCellClicked( pclDestSheetDescription, lActiveColumn, clDestProductParam.m_lSelectionBottomRow );
//
// 						pclDestSheetDescription->GetSSheetPointer()->SetActiveCell( lActiveColumn, clDestProductParam.m_lSelectionTopRow );
// 						pclDestSheetDescription->GetSSheetPointer()->ShowCell( lColsFrozen + 1, lRowsFrozen + 1, SS_SHOW_TOPLEFT );
// 					}
// 					else
// 					{
						OnCellClicked( pclDestSheetDescription, lActiveColumn, clDestProductParam.m_lSelectionBottomRow );
//						pclDestSheetDescription->GetSSheetPointer()->SetActiveCell( lActiveColumn, clDestProductParam.m_lSelectionTopRow );
//					}
					lRowReference = clDestProductParam.m_lSelectionTopRow;
					eScrollPosition = ScrollPosition::ScrollToCompute;
				}
				else
				{
					// If we don't find any more product, we click on the same one and force to scroll to the top.
					OnCellClicked( pclSheetDescription, lActiveColumn, clFromProductParam.m_lSelectionBottomRow );
					eScrollPosition = ScrollPosition::ScrollToTop;
				}
			}
			break;

		case VK_HOME:
			{
				// Scrolling has already be done in the 'MultiSpreadBase' base class.
				CProductParam clFirstProductParam;
				_GetFirstProduct( clFirstProductParam, NULL );
				CSheetDescription *pclDestSheetDescription = clFirstProductParam.m_pclSheetDescription;

				if( NULL != pclDestSheetDescription && NULL != pclDestSheetDescription->GetSSheetPointer() 
						&& 0 != clFirstProductParam.m_lSelectionTopRow && 0 != clFirstProductParam.m_lSelectionBottomRow && (LPARAM)0 != clFirstProductParam.m_lpSelectionContainer )
				{
					// Remark: Left top cell is not necessarily the cell that is in the first position (first row, first col) in the view.
					//         If there is some cols or rows frozen, and no current scroll, left top cell can be set to the row 5 for example.
					SS_COORD lColsFrozen, lRowsFrozen;
					pclDestSheetDescription->GetSSheetPointer()->GetFreeze( &lColsFrozen, &lRowsFrozen );
						
					OnCellClicked( pclDestSheetDescription, lActiveColumn, clFirstProductParam.m_lSelectionBottomRow );

					pclDestSheetDescription->GetSSheetPointer()->SetActiveCell( lActiveColumn, clFirstProductParam.m_lSelectionTopRow );
					pclDestSheetDescription->GetSSheetPointer()->ShowCell( lColsFrozen + 1, lRowsFrozen + 1, SS_SHOW_TOPLEFT );
				}
			}
			
			break;

		case VK_END:
			{
				// Scrolling has already be done in the 'MultiSpreadBase' base class.
				CProductParam clLastProductParam;
				_GetLastProduct( clLastProductParam, NULL );
				CSheetDescription *pclDestSheetDescription = clLastProductParam.m_pclSheetDescription;
				
				if( NULL != pclDestSheetDescription && NULL != pclDestSheetDescription->GetSSheetPointer()
						&& 0 != clLastProductParam.m_lSelectionTopRow && 0 != clLastProductParam.m_lSelectionBottomRow && (LPARAM)0 != clLastProductParam.m_lpSelectionContainer )
				{
					OnCellClicked( pclDestSheetDescription, lActiveColumn, clLastProductParam.m_lSelectionTopRow );
					pclDestSheetDescription->GetSSheetPointer()->SetActiveCell( lActiveColumn, clLastProductParam.m_lSelectionBottomRow );
				}
			}
			
			break;			

		default:
			// Key is not managed here, thus we can dispatch message.
			bReturn = FALSE;
			break;
	}

	if( ScrollPosition::NoScroll != eScrollPosition && 0 != (int)m_vecProductSelected.size() )
	{
		switch( eScrollPosition )
		{
			case ScrollPosition::ScrollToCompute:
				{
					// 'NULL' to specify that 'rectPixel' coordinates are relative to the window and not the current sheet description.
					CRect rectPixel = _GetScrollingRange( clDestProductParam );

					// Add a verification. If the product height is higher that the client height (It can be the case for pressurisation
					// selection), we force 'bTopPreference' to true.
					CRect rectClient;
					GetClientRect( &rectClient );

					// Remove the frozen part if exist.
					long lRowsFreeze = pclSheet->GetRowsFreeze();

					if( lRowsFreeze > 0 )
					{
						CRect rectRowsFreezeSize = pclSheet->GetSelectionInPixels( 1, 1, 1, lRowsFreeze );
						rectClient.bottom -= rectRowsFreezeSize.Height();
					}

					if( rectPixel.Height() > rectClient.Height() )
					{
						bTopPreference = true;
						lRowReference = clDestProductParam.m_lScrollTopRow;
					}

					SetScrollPosition( eScrollPosition, NULL, rectPixel, bTopPreference, lRowReference );
				}
				break;

			case ScrollPosition::ScrollToTop:
			case ScrollPosition::ScrollToBottom:
				SetScrollPosition( eScrollPosition );
				break;
		}

		_InvalidateSSheetRect();
	}

	return bReturn;
}

bool CSelProdPageBase::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, TCHAR *pstrTipText, BOOL *pbShowTip )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return false;
	}

	*pwMultiLine = SS_TT_MULTILINE_AUTO;
	*pbShowTip = FALSE;
	CString str = _T("");
	CPoint pt( lColumn, lRow );
	bool bReturn = false;

	if( m_mapTooltipMessages.count( pt ) != 0 && m_mapTooltipMessages.at( pt ).size() != 0 )
	{
		str = m_mapTooltipMessages.at( pt ).at( 0 );

		if( m_mapTooltipMessages.at( pt ).size() > 1 )
		{
			str.Insert( 0, _T("- ") );

			for( int i = 1; i < (int)m_mapTooltipMessages.at( pt ).size(); i++ )
			{
				str += _T("\r\n- ") + m_mapTooltipMessages.at( pt ).at( i );
			}
		}
	}

	if( false == str.IsEmpty() )
	{
		*pnTipWidth = (SHORT)pclSheetDescription->GetSSheetPointer()->GetTipTextWidth( str );
		wcsncpy_s( pstrTipText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX );
		*pbShowTip = TRUE;
		bReturn = true;
	}

	return bReturn;
}

bool CSelProdPageBase::PrepareSheet( CSheetDescription *pclSheetDescription, int iColNum, bool bPrint, bool bInitRows )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return false;
	}

	m_bForPrint = bPrint;

	if( 0 == m_mapSheetInfos.count( pclSheetDescription ) )
	{
		SheetInfos rSheetInfos;
		rSheetInfos.m_iColNum = iColNum;
		m_mapSheetInfos[pclSheetDescription] = rSheetInfos;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	pclSheet->Init();
	pclSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSheet->SetBool( SSB_HORZSCROLLBAR, ( true == UseOnlyOneSpread() ) ? TRUE : FALSE );
	pclSheet->SetBool( SSB_VERTSCROLLBAR, ( true == UseOnlyOneSpread() ) ? TRUE : FALSE );
	pclSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSheet->SetOperationMode( SS_OPMODE_NORMAL );
	pclSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );
	
	CDC *pDC = pclSheet->GetDC();
	double dDeviceCaps = pDC->GetDeviceCaps( LOGPIXELSY );

	// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
	pclSheet->ReleaseDC( pDC );

	m_dRowHeight = 12.75 * dDeviceCaps / 96.0;		//96 Dpi as reference

	pclSheet->SetMaxCols( m_mapSheetInfos[pclSheetDescription].m_iColNum );
	
	if( true == bInitRows )
	{
		pclSheet->SetMaxRows( SelProdHeaderRow::HR_FirstRow );
		pclSheet->SetRowHeight( SelProdHeaderRow::HR_FirstEmptyLine, m_dRowHeight * 0.5 );
		pclSheet->SetRowHeight( SelProdHeaderRow::HR_PageTitle, m_dRowHeight * 2 );
		pclSheet->SetRowHeight( SelProdHeaderRow::HR_EmptyLine, m_dRowHeight * 0.5 );
		pclSheet->SetRowHeight( SelProdHeaderRow::HR_RowHeader, m_dRowHeight * 1.6 );

		// Cell span for the main title.
		pclSheet->AddCellSpanW( 2, SelProdHeaderRow::HR_PageTitle, m_mapSheetInfos[pclSheetDescription].m_iColNum - 2, 1 );
	}
	
	// Write empty static text to fix cell format
	pclSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	m_bInitialized = true;
	return true;
}

void CSelProdPageBase::SortTable( void )
{
	if( NULL == m_pTADSPageSetup )
	{
		m_pTADSPageSetup = m_pTADS->GetpPageSetup();
		ASSERT( NULL != m_pTADSPageSetup );
	}

	if( NULL == m_pTADSPageSetup )
	{
		return;
	}
	
	if( m_vecHMList.size() > 0 )
	{
		sort( m_vecHMList.begin(), m_vecHMList.end(), _ComparePairs );
	}
}

void CSelProdPageBase::SetColWidth( CSheetDescription* pclSheetDescription, int iColumn, double dSize )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || 0 == m_mapSheetInfos.count( pclSheetDescription ) )
		return;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	ASSERT( iColumn <= m_mapSheetInfos[pclSheetDescription].m_iColNum );
	pclSheet->SetColWidth( iColumn, dSize );
	m_mapSheetInfos[pclSheetDescription].m_mapColWidth[iColumn] = pclSheet->GetColWidthInPixelsW( iColumn );
}

void CSelProdPageBase::SetLastRow( CSheetDescription *pclSheetDescription, long lLastRow )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	ASSERT( lLastRow );

	if( lLastRow <= 0 )
	{
		return;
	}

	// Extent number of rows if needed.
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	long lMaxRows = pclSheet->GetMaxRows();

	if( lLastRow > lMaxRows ) 
	{
		pclSheet->SetMaxRows( lLastRow );
	}
}

long CSelProdPageBase::AddStaticText( CSheetDescription* pclSheetDescription, long lColumn, long lRow, CString strText, bool fAdaptRowHeightIfNeeded )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return lRow;
	}

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();

	if( lRow > pclSheet->GetMaxRows() )
	{
		SetLastRow( pclSheetDescription, lRow );
		pclSheet->SetRowHeight( lRow, m_dRowHeight );
	}
	
	if( false == fAdaptRowHeightIfNeeded )
	{
		pclSheet->SetStaticText( lColumn, lRow++, strText );
	}
	else
	{
		// 'false' to change row height if needed.
		pclSheet->SetStaticTextEx( lColumn, lRow++, strText, false );
	}

	return lRow;
}

void CSelProdPageBase::AddCellSpanW( CSheetDescription* pclSheetDescription, long lColumn, long lRow, long lNumCol, long lNumRow )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
		return;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	SetLastRow( pclSheetDescription, lRow + lNumRow - 1 );
	pclSheet->AddCellSpanW( lColumn, lRow, lNumCol, lNumRow );
}

void CSelProdPageBase::SetPageTitle( CSheetDescription *pclSheetDescription, int iIDstr, bool bMain, long lRowOffset, double dRowHeight, 
		COLORREF clTextColor, COLORREF clBackgroundColor, int iFontSize )
{
	CString strTitle = TASApp.LoadLocalizedString( iIDstr );
	SetPageTitle( pclSheetDescription, strTitle, bMain, lRowOffset, dRowHeight, clTextColor, clBackgroundColor, iFontSize );
}

void CSelProdPageBase::SetPageTitle( CSheetDescription *pclSheetDescription, CString strTitle, bool bMain, long lRowOffset, double dRowHeight, 
		COLORREF clTextColor, COLORREF clBackgroundColor, int iFontSize )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || 0 == m_mapSheetInfos.count( pclSheetDescription ) )
	{
		return;
	}

	long lFirstCol = 2;
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::TitlePage );

	if( false == bMain )
	{
		pclSheet->SetTextPattern( CSSheet::_SSTextPattern::SubTitle );
	}

	// Write title on all columns, title is not hidden when columns are hidden.
	// We set the text from the column after header column (1) and before the footer column (m_iColNum-1).
	for( int iLoopColumn = lFirstCol; iLoopColumn <= m_mapSheetInfos[pclSheetDescription].m_iColNum - 2; iLoopColumn++ )
	{
		pclSheet->SetStaticText( iLoopColumn, ( lRowOffset > 0 ) ? lRowOffset : SelProdHeaderRow::HR_PageTitle, strTitle );
	}
	
	if( -1 != clTextColor )
	{
		pclSheet->SetForeColor( 2, ( lRowOffset > 0 ) ? lRowOffset : SelProdHeaderRow::HR_PageTitle, m_mapSheetInfos[pclSheetDescription].m_iColNum - 2, 
				( lRowOffset > 0 ) ? lRowOffset : SelProdHeaderRow::HR_PageTitle, clTextColor );
	}

	if( -1 != clBackgroundColor )
	{
		pclSheet->SetBackColor( 2, ( lRowOffset > 0 ) ? lRowOffset : SelProdHeaderRow::HR_PageTitle, m_mapSheetInfos[pclSheetDescription].m_iColNum - 2, 
				( lRowOffset > 0 ) ? lRowOffset : SelProdHeaderRow::HR_PageTitle, clBackgroundColor );
	}

	if( -1 != iFontSize )
	{
		pclSheet->SetFontSize( 2, ( lRowOffset > 0 ) ? lRowOffset : SelProdHeaderRow::HR_PageTitle, m_mapSheetInfos[pclSheetDescription].m_iColNum - 2, 
				( lRowOffset > 0 ) ? lRowOffset : SelProdHeaderRow::HR_PageTitle, iFontSize );
	}

	pclSheet->AddCellSpanW( lFirstCol, ( lRowOffset > 0 ) ? lRowOffset : SelProdHeaderRow::HR_PageTitle, m_mapSheetInfos[pclSheetDescription].m_iColNum - 1 - lFirstCol, 1 );

	// Set row height if defined.
	if( 0.0 != dRowHeight )
	{
		pclSheet->SetRowHeight( ( lRowOffset > 0 ) ? lRowOffset : SelProdHeaderRow::HR_PageTitle, dRowHeight );
	}
}

long CSelProdPageBase::FillRowGen( CSheetDescription *pclSheetDescription, long lRow, CDS_SelProd *pSelProd )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pSelProd )
	{
		return lRow;
	}

	CString str1, str2;
	long lFirstRow = lRow;

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column Reference 1.
	pclSheet->SetRowHeight( lRow, m_dRowHeight );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	AddStaticText( pclSheetDescription, ColumnDescription::Reference1, lRow, pSelProd->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef1 ) );

	// Column Reference 2.
	AddStaticText( pclSheetDescription, ColumnDescription::Reference2, lRow, pSelProd->GetpSelectedInfos()->GetReference( CSelectedInfos::eRef2 ) );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

	// If it's directly selected don't fill Water, pipes.
	if( false == pSelProd->IsFromDirSel() && NULL != pSelProd->GetpSelectedInfos() && NULL != pSelProd->GetpSelectedInfos()->GetpWCData() )
	{
		// Column Water char.
		pSelProd->GetpSelectedInfos()->GetpWCData()->BuildWaterStrings( str1, str2 );
	
		long lPrev = pclSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
		CString strFinal = str1 + _T("\n") + str2;

		// Check if error with temperature.
		std::vector<CString> vecSmartErrorMessages;

		if( NULL != dynamic_cast<CDB_Product *>( pSelProd->GetProductIDPtr().MP ) )
		{
			CDB_Product *pclProduct = (CDB_Product *)( pSelProd->GetProductIDPtr().MP );
			double dTemp = pSelProd->GetpSelectedInfos()->GetpWCData()->GetTemp();
			CString strErrorMessage;

			if( dTemp < pclProduct->GetTmin() )
			{
				// The fluid temperature is lower that the minimum allowed for %1 (%2 < %3).
				CString strProductName = pclProduct->GetName();
				CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTemp );
				CString strMinTemp = WriteCUDouble( _U_TEMPERATURE, pclProduct->GetTmin(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_TEMPTOOLOWFORPRODUCT, strProductName, strTemp, strMinTemp );
				vecSmartErrorMessages.push_back( strErrorMessage );
			}
			else if( dTemp > pclProduct->GetTmax() )
			{
				// The fluid temperature is higher that the maximum allowed for %1 (%2 > %3).
				CString strProductName = pclProduct->GetName();
				CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTemp );
				CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, pclProduct->GetTmax(), true );
				FormatString( strErrorMessage, IDS_HYDROMOD_TEMPTOOHIGHFORPRODUCT, strProductName, strTemp, strMaxTemp );
				vecSmartErrorMessages.push_back( strErrorMessage );
			}
		}

		if( 0 == (int)vecSmartErrorMessages.size() )
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Water, lRow, strFinal );
		}
		else
		{
			// Add tooltip.
			AddTooltipMessage( CPoint( ColumnDescription::Water, lRow ), vecSmartErrorMessages );

			// Add remark to put below.
			WriteTextWithFlags( pclSheetDescription, strFinal, ColumnDescription::Water, lRow, CSheetDescription::RemarkFlags::ErrorMessage, vecSmartErrorMessages, _RED );
			lRow++;
		}

		lRow += 3;
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)lPrev );

		//Build the pipe string according to page setup 
		//Quid of LinDpflag and Uflag ??? Mark in red and italic???
		//SetTextRC(row,7,...);
		CDB_Pipe *pPipe = pSelProd->GetpSelPipe()->GetpPipe();
		lRow = lFirstRow;

		if( NULL != pPipe )
		{
			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, ( LPARAM )TRUE );

			CTable *pPipeTab = dynamic_cast<CTable *> (pPipe->GetIDPtr().PP);
			ASSERT( NULL != pPipeTab );

			if( NULL != pPipeTab && true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPENAME ) )
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, pPipeTab->GetName(), true );
			}

			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPESIZE ) )
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, pPipe->GetName() );
			}
			
			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEWATERU ) )
			{
				if( pSelProd->GetpSelPipe()->GetU() != 0.0 )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, WriteCUDouble( _U_VELOCITY, pSelProd->GetpSelPipe()->GetU(), true ) );
				}
			}

			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEDP ) )
			{
				if( pSelProd->GetpSelPipe()->GetLinDp() != 0.0 )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, WriteCUDouble( _U_LINPRESSDROP, pSelProd->GetpSelPipe()->GetLinDp(), true ) );
				}
			}
		}
		else
		{
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, _T("-") );
		}
	}

	// Column Remark.
	lRow = lFirstRow;

	if( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) )	
	{
		if( 0 != pSelProd->GetpSelectedInfos()->GetRemarkIndex() )		// remark exist
		{
			str1.Format( _T("%d"), pSelProd->GetpSelectedInfos()->GetRemarkIndex() );
			lRow = AddStaticText( pclSheetDescription, ColumnDescription::Remark, lRow, str1 );
		}
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageBase::FillRowGen( CSheetDescription *pclSheetDescription, long lRow, CDS_HydroMod *pHM, CDS_HydroMod::CBase *pBase )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pHM  )
	{
		return lRow;
	}

	CString str1, str2;
	long lFirstRow = lRow;

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Column Reference 1.
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	CString str = pHM->GetHMName();

	// HYS-1734: HMName and Bv localization in the same line because of the cell merge done in init functions.

	if( NULL != pBase )
	{
		str += _T("\n");

		if( NULL != pHM->GetpBypBv() || NULL != pHM->GetpSecBv() )
		{
			// Add Bv localization.
			if( pBase == pHM->GetpBv() )
			{
				str += CString( L"( ") + TASApp.LoadLocalizedString( IDS_PRIMARY ) + CString( L" )" );
			}
			else if( pBase == pHM->GetpBypBv() )
			{
				str += CString( L"( ") + TASApp.LoadLocalizedString( IDS_BYPASS ) + CString( L" )" );
			}
			else if( pBase == pHM->GetpSecBv() )
			{
				str += CString( L"( ") + TASApp.LoadLocalizedString( IDS_SECONDARY ) + CString( L" )" );
			}
		}
	}

	AddStaticText( pclSheetDescription, ColumnDescription::Reference1, lRow, str );

	// Column Reference 2.
	AddStaticText( pclSheetDescription, ColumnDescription::Reference2, lRow, pHM->GetDescription() );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
	
	// HYS-1734: Temperature and pipe for valves are in each location. If the valve is on the supply side
	// we should get supply pipe and supply temperature. Same for return side, secondary, primary, bypass, Distribution, circuit.
	if( NULL != pBase )
	{
		// Column water characteristic.
		// HYS-721: For HM calc add word wrap possibility for column water char.
		// HYS-1716: For the moment we show only one water characteristic. For the future, we will differentiante for example the water characteristic
		// at the supply and return of the circuit.
		// HYS-1734: We get the Water characteristic of the current displayed product at its location.
		CWaterChar* pclWaterChar = pHM->GetpWaterChar( pBase->GetHMObjectType() );
		ASSERT( NULL != pclWaterChar );

		long lPrev = pclSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap );

		if( NULL != pclWaterChar )
		{
			pclWaterChar->BuildWaterStrings( str1, str2 );

			pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
			CString strFinal = str1 + _T("\n") + str2;
			AddStaticText(pclSheetDescription, ColumnDescription::Water, lRow, strFinal );
		}

		lRow += 3;
		pclSheet->SetTextPatternProperty(CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)lPrev);

		// Build the pipe string according to page setup.
		lRow = lFirstRow;

		// Verify that pipes must be displayed.
		CDB_Pipe* pPipe = NULL;
		CPipes* pHMPipes = NULL;

		pHMPipes = pHM->GetpPipe( pBase->GetHMObjectType() );
		// for now no distribution pipe displayed! See HYS-1791

		if( NULL != pHMPipes && false == pHM->IsaModule() )
		{
			pPipe = dynamic_cast<CDB_Pipe*>(pHMPipes->GetIDPtr().MP);
		}

		if( NULL != pPipe )
		{
			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPENAME ) )
			{
				CTable *pTabPipeSeries = dynamic_cast<CTable *>( pPipe->GetIDPtr().PP );

				if( NULL != pTabPipeSeries )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, pTabPipeSeries->GetName() );
				}
			}

			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPESIZE ) )
			{
				lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, pPipe->GetName() );
			}

			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEWATERU ) )
			{
				if( pHMPipes->GetVelocity() != 0.0 )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, WriteCUDouble( _U_VELOCITY, pHMPipes->GetVelocity(), true ) );
				}
			}

			if( true == m_pTADSPageSetup->GetField( PageField_enum::epfPIPEDP ) )
			{
				if( pHMPipes->GetLinDp() != 0.0 )
				{
					lRow = AddStaticText( pclSheetDescription, ColumnDescription::Pipes, lRow, WriteCUDouble( _U_LINPRESSDROP, pHMPipes->GetLinDp(), true ) );
				}
			}
		}
	}

	// Column Remark.
	lRow = lFirstRow;

	if( true == m_pTADSPageSetup->GetField( PageField_enum::epfREMARK ) )	
	{
		if( false == pHM->GetRemL1().IsEmpty() || false == pHM->GetComment().IsEmpty() )
		{
			// CString str;
			// str.Format( _T("%d"), pHM->GetRemarkIndex() );
			// AddStaticText( pclSheetDescription, ColumnDescription::Remark, lRow, str );

			CString strInfoMessage = pHM->GetRemL1();

			if( false == pHM->GetComment().IsEmpty() )
			{
				if( true == strInfoMessage.IsEmpty() )
				{
					strInfoMessage = pHM->GetComment();
				}
				else
				{
					strInfoMessage += _T("\r\n") + pHM->GetComment();
				}
			}

			pclSheetDescription->WriteTextWithFlags( _T(""), ColumnDescription::Remark, lRow, CSheetDescription::RemarkFlags::InfoMessage, strInfoMessage );
		}
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageBase::FillRowGenOthers( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod* pHM, CDS_HydroMod::CBase* pBase )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pHM )
	{
		return lRow;
	}

	CString str1, str2;
	long lFirstRow = lRow;

	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	if( NULL != pBase )
	{
		// Column water characteristic.
		CWaterChar* pclWaterChar = pHM->GetpWaterChar( pBase->GetHMObjectType() );
		ASSERT( NULL != pclWaterChar );

		if( NULL != pclWaterChar )
		{
			pclWaterChar->BuildWaterStrings( str1, str2 );

			AddStaticText( pclSheetDescription, ColumnDescription::Water, lRow, str2 );
		}

		lRow ++;
	}

	return pclSheet->GetMaxRows();
}

void CSelProdPageBase::FillArtNumberCol( CSheetDescription* pclSheetDescription, long lColumn, long lRow, CDB_Product* pclProduct, CString strArtNumberStr )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclProduct )
		return;

	CString str1, str2;

	// Set font color to red when article is not available or deleted.
	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	if( true == pclProduct->IsDeleted() || false == pclProduct->IsAvailable() )
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );

	if( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) )
	{
		// Check first if we have a local article number available.
		CString strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclProduct->GetArtNum() );

		if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() ) )
		{
			str2 = strArtNumberStr;
			// Replace '&' by an row jump one article number by line.
			int iPos = -1;
			do
			{
				str1 = str2;
				iPos = str2.Find( _T("&") );
				if( iPos != -1 )
				{
					str1 = str2.Left( iPos );
					str2.Delete( 0, iPos + 1 );
				}
				lRow = AddStaticText( pclSheetDescription, lColumn, lRow, str1 );
			}while( iPos != -1 );
		}

		// Set local article number if allowed and exist.
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			// Prefix.
			if( false == TASApp.GetLocalArtNumberName().IsEmpty() )
				lRow = AddStaticText( pclSheetDescription, lColumn, lRow, TASApp.GetLocalArtNumberName() );
			
			// Replace '&' by an row jump one article number by line.
			str2 = strLocalArticleNumber;
			int iPos = -1;
			do
			{
				str1 = str2;
				iPos = str2.Find( _T("&") );
				if( iPos != -1 )
				{
					str1 = str2.Left( iPos );
					str2.Delete( 0, iPos + 1 );
				}
				lRow = AddStaticText( pclSheetDescription, lColumn, lRow, str1 );
			}while( iPos != -1 );
		}
		
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		if( true == pclProduct->IsDeleted() )
			lRow = AddStaticText( pclSheetDescription, lColumn, lRow, TASApp.LoadLocalizedString( IDS_TAPDELETED ) );
		else if( false == pclProduct->IsAvailable() )
			lRow = AddStaticText( pclSheetDescription, lColumn, lRow, TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE ) );
	}
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
}

void CSelProdPageBase::FillArtNumberColForPackage( CSheetDescription *pclSheetDescription, long lColumn, long lRow, CDB_Set *pclSet, CString strArtNumberStr )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclSet )
	{
		return;
	}

	CString str1, str2;

	// Set font color to red when article is not available or deleted.
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();

	if( true == pclSet->IsDeleted() || false == pclSet->IsAvailable() )
	{
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}

	if( true == m_pTADSPageSetup->GetField( PageField_enum::epfARTICLE ) )
	{
		// Check first if we have a local article number available.
		CString strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclSet->GetReference() );

		// If we can show article number OR we can't show but there is no local article number...
		if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() ) )
		{
			str2 = strArtNumberStr;
			// Replace '&' by an row jump one article number by line.
			int iPos = -1;
			
			do
			{
				str1 = str2;
				iPos = str2.Find( _T("&") );
				
				if( iPos != -1 )
				{
					str1 = str2.Left( iPos );
					str2.Delete( 0, iPos + 1 );
				}
				
				lRow = AddStaticText( pclSheetDescription, lColumn, lRow, str1 );
			
			}while( iPos != -1 );
		}

		// Set local article number if allowed and exist.
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			// If prefix exist...
			if( false == TASApp.GetLocalArtNumberName().IsEmpty() )
			{
				lRow = AddStaticText( pclSheetDescription, lColumn, lRow, TASApp.GetLocalArtNumberName() );
			}

			lRow = AddStaticText( pclSheetDescription, lColumn, lRow, strLocalArticleNumber );
		}

		lRow = AddStaticText( pclSheetDescription, lColumn, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SET ) );
		
		// Remarks: if product is flagged as 'not available' and 'deleted', we show only 'deleted'!
		if( true == pclSet->IsDeleted() )
		{
			lRow = AddStaticText( pclSheetDescription, lColumn, lRow, TASApp.LoadLocalizedString( IDS_TAPDELETED ) );
		}
		else if( false == pclSet->IsAvailable() )
		{
			lRow = AddStaticText( pclSheetDescription, lColumn, lRow, TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE ) );
		}
	}

	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
}

void CSelProdPageBase::FillQtyPriceCol( CSheetDescription* pclSheetDescription, long lFirstRow, CData* pclProduct, int iQuantity, bool fShowPrice )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclProduct )
		return;

	CString str1;
	
	// Column Quantity.
	CSSheet* pclSheet = pclSheetDescription->GetSSheetPointer();
	long lRow = lFirstRow;
	
	if( true == m_pTADSPageSetup->GetField( PageField_enum::epfQUANTITY ) )	
	{
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		str1.Format( _T("%d"), iQuantity );
		AddStaticText( pclSheetDescription, ColumnDescription::Quantity, lRow++, str1 );
	}

	if( true == TASApp.IsPriceUsed() )
	{
		// Column 'Price'.
		lRow = lFirstRow;
		AddStaticText( pclSheetDescription, ColumnDescription::UnitPrice, lRow, _T("-") );

		double dPrice = TASApp.GetpTADB()->GetPrice( pclProduct->GetArtNum( true ) );

		if( true == fShowPrice && true == m_pTADSPageSetup->GetField( PageField_enum::epfSALEUNITPRICE ) && dPrice > 0.0 )
		{
			str1 = (CString)WriteDouble( dPrice, 2, 2, 0 );
			AddStaticText( pclSheetDescription, ColumnDescription::UnitPrice, lRow++, str1 );
		}

		// Column 'Total Price'.
		lRow = lFirstRow;
		AddStaticText( pclSheetDescription, ColumnDescription::TotalPrice, lRow, _T("-") );			// Default string

		if( true == fShowPrice && true == m_pTADSPageSetup->GetField( PageField_enum::epfSALETOTALPRICE ) && dPrice > 0.0 )
		{
			double dTotal = dPrice * iQuantity;
			str1 = (CString)WriteDouble( dTotal, 2, 2, 0 );
			AddStaticText( pclSheetDescription, ColumnDescription::TotalPrice, lRow++, str1 );
		}
	}
}

long CSelProdPageBase::FillRemarks( CSheetDescription *pclSheetDescription, long lFirstRow, bool *pfRemarkExist )
{
	if( NULL != pfRemarkExist )
	{
		*pfRemarkExist = false;
	}
	
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || 0 == m_mapSheetInfos.count( pclSheetDescription ) || NULL == m_pclSelectionTable )
	{
		return lFirstRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	long lRow = lFirstRow;
	CArray<CString> aRemarks;

	// Pos 0 not used.
	aRemarks.SetSize( m_pclSelectionTable->GetItemCount() + 1 );
	
	if( 0 == CString( _T("SELHUB_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID )
			|| 0 == CString( _T("CTRLVALVE_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID )
			|| 0 == CString( _T("REGVALV_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID )
			|| 0 == CString( _T("DPCONTR_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID )
			|| 0 == CString( _T("DPCBCVALVE_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID )
			|| 0 == CString( _T("RADSET_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID )
			|| 0 == CString( _T("AIRVENTSEP_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID )
			|| 0 == CString( _T("PRESSMAINT_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID )
			|| 0 == CString( _T("SHUTOFF_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID )
			|| 0 == CString( _T("SAFETYVALVE_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID )
			|| 0 == CString( _T("6WAYCTRLVALV_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID ) 
			|| 0 == CString( _T("TAPWATERCTRL_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID ) 
			|| 0 == CString( _T("SMARTCONTROLVALVE_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID )
			|| 0 == CString( _T("FLOORHCTRL_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID )
			|| 0 == CString( _T("SMARTDPC_TAB") ).Compare( m_pclSelectionTable->GetIDPtr().ID ) )
	{
		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{		
			CSelectedInfos *pSelInfo = dynamic_cast<CSelectedInfos *>( IDPtr.MP->GetpSelectedInfos() );
			
			if( NULL == pSelInfo )
			{
				continue;
			}
			
			int iIndex = pSelInfo->GetRemarkIndex();	
			
			if( 0 == iIndex )
			{
				continue;
			}
			
			aRemarks[iIndex] = pSelInfo->GetRemark();
			
			// Remove Index when is saved.
			pSelInfo->SetRemarkIndex( 0 );
		}
	}
	else
	{
		// Other table.
		for( IDPTR IDPtr = m_pclSelectionTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_pclSelectionTable->GetNext() )
		{
			int iIndex = ( (CDS_SelThing *)( IDPtr.MP ) )->GetRemarkIndex();

			if( 0 == iIndex )
			{
				continue;
			}
			
			aRemarks[iIndex] = ( (CDS_SelThing *)( IDPtr.MP ) )->GetRemark();
		}
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	bool bOneRemarkWritten = false;

	for( int i = 1; i < aRemarks.GetCount(); i++ )
	{
		if( true == aRemarks[ i ].IsEmpty() )
		{
			continue;
		}
		
		if( true == bOneRemarkWritten )
		{
			lRow++;
		}

		pclSheet->SetMaxRows( lRow );

		// Add first the index in the first column.
		CString strIndex;
		strIndex.Format( _T("[%d] "), i );

		bool bFirst = true;
		int iCurPos = 0;
		CString strText;
		CString strRemark = aRemarks[i];
		CString strToken = strRemark.Tokenize( _T("\r\n"), iCurPos );
		
		while( _T("") != strToken )
		{
			strText = strToken;

			if( true == bFirst )
			{
				strText = strIndex + strToken;
			}
			else
			{
				// Go to the next line.
				lRow++;
				pclSheet->SetMaxRows( lRow );
			}

			// Same text for each column, prevent border effect of hiding column.
			// Remark: set the text from the column after the header column (1) to the column before the footer (m_iColNum - 1).
			for( int j = 2; j <= m_mapSheetInfos[pclSheetDescription].m_iColNum - 2; j++ )
			{
				pclSheet->SetStaticText( j, lRow, strText );
			}
			
			AddCellSpanW( pclSheetDescription, 2, lRow, m_mapSheetInfos[pclSheetDescription].m_iColNum - 2, 1 );
		
			// Remark: pointer is the last column (m_iColNum).
			pclSheet->SetCellParam( m_mapSheetInfos[pclSheetDescription].m_iColNum, lRow, 0 );

			strToken = strRemark.Tokenize( _T("\r\n"), iCurPos );
			bFirst = false;
		};

		if( NULL != pfRemarkExist )
		{
			*pfRemarkExist = true;
		}
		
		bOneRemarkWritten = true;
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageBase::FillHMRemarks( HMvector *pvecHMList, CSheetDescription *pclSheetDescription, long lFirstRow, bool *pfRemarkExist )
{
	if( NULL != pfRemarkExist )
	{
		*pfRemarkExist = false;
	}
	
	if( NULL == pvecHMList || NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() 
		|| 0 == m_mapSheetInfos.count( pclSheetDescription ) || 0 == (int)m_vecHMList.size() )
	{
		return lFirstRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	long lRow = lFirstRow;
	
	// Pos 0 not used.
	CArray<CString> aRemarks;
	aRemarks.SetSize( (int)pvecHMList->size() + 1 );
	
	for( int i = 0; i < (int)pvecHMList->size(); i++ )
	{
		CDS_HydroMod *pHM = NULL;

		if( CSelProdPageBase::eptHM == pvecHMList->at( i ).first )
		{
			pHM = dynamic_cast<CDS_HydroMod *>( (CData*)( pvecHMList->at( i ).second ) );
		}
		else
		{
			CDS_HydroMod::CBase *pBase = static_cast<CDS_HydroMod::CBase *>( pvecHMList->at( i ).second );

			if( NULL != pBase )
			{
				pHM = pBase->GetpParentHM();
			}
		}

		int iRemarkIndex = pHM->GetRemarkIndex();

		if( NULL == pHM || 0 == iRemarkIndex )
		{
			continue;
		}

		aRemarks[iRemarkIndex] = pHM->GetRemL1();

		if( false == pHM->GetComment().IsEmpty() )
		{
			if( true == aRemarks[iRemarkIndex].IsEmpty() )
			{
				aRemarks[iRemarkIndex] = pHM->GetComment();
			}
			else
			{
				aRemarks[iRemarkIndex] += _T("\r\n") + pHM->GetComment();
			}
		}
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	bool bOneRemarkWritten = false;

	for( int i = 1; i < aRemarks.GetCount(); i++ )
	{
		if( true == aRemarks[ i ].IsEmpty() )
		{
			continue;
		}

		if( true == bOneRemarkWritten )
		{
			lRow++;
		}

		pclSheet->SetMaxRows( lRow );
		
		// Add first the index in the first column.
		CString strIndex;
		strIndex.Format( _T("[%d] "), i );

		bool bFirst = true;
		int iCurPos = 0;
		CString strText;
		CString strRemark = aRemarks[i];
		CString strToken = strRemark.Tokenize( _T("\r\n"), iCurPos );
		
		while( _T("") != strToken )
		{
			strText = strToken;

			if( true == bFirst )
			{
				strText = strIndex + strToken;
			}
			else
			{
			   lRow++;
			   pclSheet->SetMaxRows( lRow );
			}

			// Same text for each column, prevent border effect of hiding column.
			// Remark: set the text from the column after the header column (1) to the column before the footer (m_iColNum - 1).
			for( int j = 2; j <= m_mapSheetInfos[pclSheetDescription].m_iColNum - 2; j++ )
			{
				pclSheet->SetStaticText( j, lRow, strText );
			}
			
			AddCellSpanW( pclSheetDescription, 2, lRow, m_mapSheetInfos[pclSheetDescription].m_iColNum - 2, 1 );
		
			// Remark: pointer is the last column (m_iColNum).
			pclSheet->SetCellParam( m_mapSheetInfos[pclSheetDescription].m_iColNum, lRow, 0 );

			strToken = strRemark.Tokenize( _T("\r\n"), iCurPos );
			bFirst = false;
		};

		if( NULL != pfRemarkExist )
		{
			*pfRemarkExist = true;
		}

		bOneRemarkWritten = true;
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageBase::FillAccessories( CSheetDescription *pclSheetDescription, long lRow, CDS_SSel *pSSel, int iGroupQuantity, int iDistributedQty, CProductParam *pclProductParam )
{
	if( NULL == pclSheetDescription || NULL == pSSel )
	{
		return 0;
	}

	return FillAccessories( pclSheetDescription, lRow, pSSel->GetAccessoryList(), iGroupQuantity, iDistributedQty, pclProductParam );
}

long CSelProdPageBase::FillAccessories( CSheetDescription *pclSheetDescription, long lRow, CAccessoryList *pclAccessoryList, 
		int iGroupQuantity, int iDistributedQty, CProductParam *pclProductParam, bool bWithArticleNumber )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclAccessoryList || 0 == pclAccessoryList->GetCount() )
	{
		return lRow;
	}

	CRank rkList;
	CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();

	while( rAccessoryItem.IDPtr.MP != NULL )
	{
		CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( rAccessoryItem.IDPtr.MP );
		VERIFY( NULL != pclAccessory );

		if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
		{
			break;
		}

		CAccessoryList::AccessoryItem *pclAccessoryItem = new CAccessoryList::AccessoryItem();

		if( NULL == pclAccessoryItem )
		{
			break;
		}

		*pclAccessoryItem = rAccessoryItem;
		rkList.AddStrSort( pclAccessory->GetName(), 0, (LPARAM)pclAccessoryItem, false, false );
		rAccessoryItem = pclAccessoryList->GetNext();
	}

	CString str;
	LPARAM lParam;
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();

	for( BOOL bContinue = rkList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = rkList.GetNext( str, lParam ) )
	{
		CAccessoryList::AccessoryItem *pclAccessoryItem = (CAccessoryList::AccessoryItem *)lParam;
		CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( (CData *)( pclAccessoryItem->IDPtr.MP ) );
		ASSERT( NULL != pclAccessory );

		bool bByPair = pclAccessoryItem->fByPair;
		bool bDistributed = pclAccessoryItem->fDistributed;
			
		if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
		{
			continue;
		}

		int iTotalQuantity = 1;

		if( true == bByPair )
		{
			iTotalQuantity = 2;
		}
		else if( iDistributedQty > 1 && true == bDistributed )
		{
			iTotalQuantity = iDistributedQty;
		}
		
		// HYS-987: if the accessory quantity has changed take this value
		if( -1 != pclAccessoryItem->lEditedQty )
		{
			iTotalQuantity = pclAccessoryItem->lEditedQty;
		}
		else
		{
			iTotalQuantity *= iGroupQuantity;
		}
		
		delete pclAccessoryItem;

		lRow = FillAccessory( pclSheetDescription, lRow, pclAccessory, bWithArticleNumber, iTotalQuantity, pclProductParam);
		lRow++;
	}

	return pclSheet->GetMaxRows();
}

long CSelProdPageBase::FillAndAddBuiltInHMAccessories(CSSheet *pclSheet, CSheetDescription *pclSheetDescription, CDB_TAProduct *pTAprod, long lRow)
{
	// Add built-in accessories.
	if (NULL != pTAprod)
	{
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>(pTAprod->GetAccessoriesGroupIDPtr().MP);

		if (NULL != pclRuledTable)
		{
			std::vector<CData *> vecBuiltInAccessories;
			int nCount = pclRuledTable->GetBuiltIn(&vecBuiltInAccessories);

			if (nCount > 0)
			{
				// Draw dash line.
				pclSheet->SetCellBorder(ColumnDescription::Reference1, lRow, ColumnDescription::Footer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_FINE_DASH);
			}

			// Run all built-in accessories.
			for (int i = 0; i < nCount; i++)
			{
				CDB_Product *pclBuiltInAccessory = dynamic_cast<CDB_Product *>(vecBuiltInAccessories.at(i));

				if ( NULL != pclBuiltInAccessory && pclBuiltInAccessory->IsAnAccessory() )
				{
					lRow++;
					lRow = FillAccessory(pclSheetDescription, lRow, pclBuiltInAccessory, true, 1);
					AddAccessoryInArticleContainer(pclBuiltInAccessory, NULL, 1, false);
				}
			}
		}
	}
	return lRow;
}

long CSelProdPageBase::FillAccessory( CSheetDescription *pclSheetDescription, long lRow, CDB_Product *pclAccessory, bool bWithArticleNumber, int iQuantity, CProductParam *pclProductParam, LPARAM lpParam )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return lRow;
	}
	
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	VERIFY( NULL != pclAccessory );

	if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
	{
		return pclSheet->GetMaxRows();
	}

	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)TRUE );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	SetLastRow( pclSheetDescription, lRow );

	// Write name / PN / Tmax.
	CString strName = pclAccessory->GetName();
	CString strDescription = _T("");
	FillAccessoryHelper_GetNameDescription( pclAccessory, strName, strDescription, lpParam );

	// Check if there is an error for the accessory in the 'name' column.
	std::vector<CString> vecNameErrorMsgList;
	FillAccessoryHelper_GetErrorMessage( pclAccessory, ColumnDescription::Reference1, &vecNameErrorMsgList, lpParam );

	pclSheet->AddCellSpanW( ColumnDescription::Reference1, lRow, 3, 1 );

	if( 0 == (int)vecNameErrorMsgList.size() )
	{
		pclSheet->SetStaticText( ColumnDescription::Reference1, lRow, strName );
	}
	else
	{
		// Add tooltip.
		AddTooltipMessage( CPoint( ColumnDescription::Reference1, lRow ), vecNameErrorMsgList );

		// Add remark to put below.
		WriteTextWithFlags( pclSheetDescription, strName, ColumnDescription::Reference1, lRow, CSheetDescription::RemarkFlags::ErrorMessage, vecNameErrorMsgList, _RED );
	}

	// Check current height necessary to display all info.
	double dMaxWidth, dMaxHeight;
	pclSheet->GetMaxTextCellSize( ColumnDescription::Reference1, lRow, &dMaxWidth, &dMaxHeight );

	// Write description.
	pclSheet->AddCellSpanW( ColumnDescription::Product, lRow, 2, 1 );

	// Check if there is an error for the accessory in the 'description' column.
	std::vector<CString> vecDescriptionErrorMsgList;
	FillAccessoryHelper_GetErrorMessage( pclAccessory, ColumnDescription::Product, &vecDescriptionErrorMsgList, lpParam );

	if( 0 == (int)vecDescriptionErrorMsgList.size() )
	{
		pclSheet->SetStaticText( ColumnDescription::Product, lRow, strDescription );
	}
	else
	{
		// Add tooltip.
		AddTooltipMessage( CPoint( ColumnDescription::Product, lRow ), vecDescriptionErrorMsgList );

		// Add remark to put below.
		WriteTextWithFlags( pclSheetDescription, strDescription, ColumnDescription::Product, lRow, CSheetDescription::RemarkFlags::ErrorMessage, vecDescriptionErrorMsgList, _RED );
	}

	double dHeight;
	pclSheet->GetMaxTextCellSize( ColumnDescription::Product, lRow, &dMaxWidth, &dHeight );

	if( dHeight > dMaxHeight )
	{
		dMaxHeight = dHeight;
	}

	// Set font color to red when accessory is not deleted or not available.
	if( true == pclAccessory->IsDeleted() || false == pclAccessory->IsAvailable() )
	{
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}

	// If accessory is included into a set, mask his article number.
	if( false == bWithArticleNumber )
	{
		CString strArticleNumber = _T("-");

		if( true == pclAccessory->IsDeleted() )
		{
			strArticleNumber += CString( _T("\r\n") ) + TASApp.LoadLocalizedString( IDS_TAPDELETED );
		}
		else if( false == pclAccessory->IsAvailable() )
		{
			strArticleNumber += CString( _T("\r\n") ) + TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE );
		}

		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		pclSheet->SetStaticText( ColumnDescription::ArticleNumber, lRow, strArticleNumber );
		pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	}
	else
	{
		// Check first if we have a local article number available.
		CString strLocalArticleNumber = _T("");

		if( true == TASApp.IsLocalArtNumberUsed() )
		{
			strLocalArticleNumber = LocArtNumTab.GetLocalArticleNumber( pclAccessory->GetArtNum() );
			strLocalArticleNumber.Trim();
		}

		// Fill article number.
		CString strArticleNumber = _T("");
		
		// If we can show article number OR we can't show but there is no local article number...
		if( false == TASApp.IsTAArtNumberHidden() || ( true == TASApp.IsTAArtNumberHidden() && true == strLocalArticleNumber.IsEmpty() )  )
		{
			strArticleNumber = pclAccessory->GetArtNum();
		}

		// Set local article number if allowed and exist.
		CString strFullArticleNumber = strArticleNumber;
		
		if( true == TASApp.IsLocalArtNumberUsed() && false == strLocalArticleNumber.IsEmpty() )
		{
			strLocalArticleNumber = (CString)TASApp.GetLocalArtNumberName() + LocArtNumTab.GetLocalArticleNumber( pclAccessory->GetArtNum() );
			strLocalArticleNumber.Trim();

			if( false == strArticleNumber.IsEmpty() && false == strLocalArticleNumber.IsEmpty() )
			{
				strFullArticleNumber = strArticleNumber + _T("\r\n") + strLocalArticleNumber;
			}
			else
			{
				strFullArticleNumber = ( true == strArticleNumber.IsEmpty() ) ? strLocalArticleNumber : strArticleNumber;
			}

			strFullArticleNumber.Trim();
		}

		if( true == pclAccessory->IsDeleted() )
		{
			strFullArticleNumber += CString( _T("\r\n") ) + TASApp.LoadLocalizedString( IDS_TAPDELETED );
		}
		else if( false == pclAccessory->IsAvailable() )
		{
			strFullArticleNumber += CString( _T("\r\n") ) + TASApp.LoadLocalizedString( IDS_TAPNOTAVAILABLE );
		}

		pclSheet->SetTextPatternProperty(CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER);
		pclSheet->SetStaticText( ColumnDescription::ArticleNumber, lRow, strFullArticleNumber );
		pclSheet->SetTextPatternProperty(CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT);
	}

	pclSheet->GetMaxTextCellSize( ColumnDescription::ArticleNumber, lRow, &dMaxWidth, &dHeight );

	if( dHeight > dMaxHeight )
	{
		dMaxHeight = dHeight;
	}

	// Adapt row height.
	pclSheet->SetRowHeight( lRow, ( dMaxHeight > m_dRowHeight ) ? dMaxHeight : m_dRowHeight );

 	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// If article number must be hidden, then price also.
	FillQtyPriceCol( pclSheetDescription, lRow, pclAccessory, iQuantity, bWithArticleNumber );

 	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
 	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold,(LPARAM) FALSE );

	if( NULL == pclProductParam )
	{
		m_rProductParam.AddRange( lRow, lRow, pclAccessory );
	}
	else
	{
		pclProductParam->AddRange( lRow, lRow, pclAccessory );
	}

	return pclSheet->GetMaxRows();
}

void CSelProdPageBase::FillAccessoryHelper_GetNameDescription( CDB_Product *pclAccessory, CString &strName, CString &strDescription, LPARAM lpParam )
{
	if( pclAccessory->GetPmaxmax() >= 0.0 && pclAccessory->GetPmaxmax() < DBL_MAX )
	{
		CTable *pPNTab = static_cast< CTable *>( TASApp.GetpTADB()->Get( _T("PN_TAB") ).MP );
		ASSERT( NULL != pPNTab );
		
		double dPN = 0.0;
		double dDP = DBL_MAX;
		double dPmax = pclAccessory->GetPmaxmax();
		CString strPN;

		for( IDPTR PNIDPtr = pPNTab->GetFirst( CLASS( CDB_StringID ) ); _T('\0') != *PNIDPtr.ID; PNIDPtr = pPNTab->GetNext() )
		{
			ReadDoubleFromStr( (TCHAR *)( (CDB_StringID *)PNIDPtr.MP )->GetIDstr(), &dPN );

			if( ( dPmax - dPN ) >= 0.0 && fabs( dPmax - dPN ) < dDP )
			{
				dDP = fabs( dPmax - dPN );
				strPN = CString( _T("; PN ") ) + ( (CDB_String *)PNIDPtr.MP )->GetString();
			}
		}

		if( strPN.IsEmpty() == false )
		{
			strName += strPN;
		}
	}

	if( pclAccessory->GetTmin() > -273.15 || pclAccessory->GetTmax() < DBL_MAX )
	{
		strName += CString( _T("; ") );

		if( pclAccessory->GetTmin() > -273.15 )
		{
			strName += WriteCUDouble( _U_TEMPERATURE, pclAccessory->GetTmin(), false == ( pclAccessory->GetTmax() < DBL_MAX ) );
		}
		else
		{
			strName += CString( _T("- ") );
		}

		strName += CString( _T("/") );

		if( pclAccessory->GetTmax() < DBL_MAX )
		{
			strName += WriteCUDouble( _U_TEMPERATURE, pclAccessory->GetTmax(), true );
		}
		else
		{
			strName += CString( _T(" -") );
		}
	}

	strDescription = pclAccessory->GetComment();

	// HYS-2007
	if( NULL != dynamic_cast<CDB_DpSensor*>(pclAccessory) )
	{
		// Name with Dp range.
		CDB_DpSensor* pclDpSensor = (CDB_DpSensor*)(pclAccessory);
		strName = pclDpSensor->GetFullName();

		// Add description.
		if( 0 < pclDpSensor->GetBurstPressure() )
		{
			CString strBurstPressure = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetBurstPressure(), true );
			// HYS-2059: Display like in DpSensorSet with TA-Smart Dp.
			if( _T( "" ) != pclDpSensor->GetComment() )
			{
				// Differential pressure sensor. Burst pressure = %1.
				FormatString( strDescription, pclDpSensor->GetComment(), strBurstPressure );
			}
			else
			{
				// Burst pressure: %1.
				FormatString( strDescription, IDS_TALINK_BURSTPRESSURE, strBurstPressure );
			}
		}
	}
}

void CSelProdPageBase::FillAccessoryHelper_GetErrorMessage( CDB_Product *pclAccessory, int iCol, std::vector<CString> *pvecErrorMsgList, LPARAM lpParam )
{
	if( NULL != pvecErrorMsgList )
	{
		pvecErrorMsgList->clear();
	}
}

long CSelProdPageBase::FillPNTminTmax( CSheetDescription *pclSheetDescription, long lColumn, long lRow, CDB_Product *product )
{
	CString str2 = TASApp.LoadLocalizedString( IDS_PN );
	CString str1 = str2 + _T(" ");
	str2 = product->GetPN().c_str();
	str1 += str2;
	str1 += CString( _T("; ") );

	if( product->GetTmin() > -273.15 )
	{
		str1 += WriteCUDouble( _U_TEMPERATURE, product->GetTmin(), false == ( product->GetTmax() < DBL_MAX ) );
	}
	else
	{
		str1 += CString( _T("- ") );
	}

	str1 += CString( _T("/") );

	if( product->GetTmax() < DBL_MAX )
	{
		str1 += WriteCUDouble( _U_TEMPERATURE, product->GetTmax(), true );
	}
	else
	{
		str1 += CString( _T(" -") );
	}

	return AddStaticText( pclSheetDescription, lColumn, lRow, str1 );
}

void CSelProdPageBase::SaveProduct( CProductParam& clProductParam )
{
	if( NULL == clProductParam.m_pclSheetDescription || 0 == m_mapSheetInfos.count( clProductParam.m_pclSheetDescription ) )
	{
		return;
	}

	m_mapSheetInfos[clProductParam.m_pclSheetDescription].m_mapProductRegistered[clProductParam.m_lSelectionTopRow] = clProductParam;
}

void CSelProdPageBase::AddCompArtList( CDB_TAProduct *pTAP, int iQuantity, bool fInlet )
{
	if( NULL == pTAP || NULL == m_pclArticleGroupList )
	{
		return;
	}

	CDB_Component *pComponent = NULL;
	
	if( true == fInlet )
	{
		pComponent = (CDB_Component *)( TASApp.GetpTADB()->Get( pTAP->GetConnectID() ).MP );
	}
	else
	{
		pComponent = (CDB_Component *)( TASApp.GetpTADB()->Get( pTAP->GetConn2ID() ).MP );
	}
	
	if( NULL == pComponent )
	{
		return;
	}

	CArticleGroup *pclArticleGroup = new CArticleGroup();

	if( NULL == pclArticleGroup )
	{
		return;
	}

	CArticleContainer *pclArticleContainer = pclArticleGroup->AddArticle( pComponent, iQuantity );

	CString strDescription = TASApp.LoadLocalizedString( IDS_CONN_PIECE );
	strDescription += CString( _T(": ") ) + CString( ( ( CDB_TAProduct *)pComponent )->GetConnect() ) + CString( _T(", ") );
	strDescription += CString( ( ( CDB_TAProduct *)pComponent )->GetSize() );
	
	if( NULL != pclArticleContainer )
	{
		pclArticleContainer->GetArticleItem()->SetDescription( strDescription );
	}

	AddArticleInGroupList( pclArticleGroup );
	delete pclArticleGroup;
}

void CSelProdPageBase::AddArticleInGroupList( CArticleGroup *pclArticleGroup )
{
	if( NULL == pclArticleGroup || NULL == m_pclArticleGroupList )
	{
		return;
	}

	bool fTest = false;

	// Don't test if 'm_fMergeSameArticleFlag' is set to 'false' (for pressure maintenance selection)
	if( true == pclArticleGroup->GetMergeSameArticleFlag() )
	{
		// Test if the article (with all sub articles) exist.
		fTest = _TestForSameArticle( pclArticleGroup, false );
	}

	if( true == fTest )
	{
		// We don't add a new article but we increment the number of article present in this article group.
		_TestForSameArticle( pclArticleGroup, true );
	}
	else
	{
		CArticleGroup *pSav = new CArticleGroup( pclArticleGroup );
		m_pclArticleGroupList->AddArticleGroup( pSav );
	}
}

void CSelProdPageBase::AddAccessoryInArticleContainer( CDB_Product *pclAccessory, CArticleContainer *pclArticleContainer, int iQuantity, bool bSelectionBySet )
{
	if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() || NULL == m_pclArticleGroupList )
	{
		return;
	}

	CArticleItem *pclArticleItem = new CArticleItem();

	if( NULL == pclArticleItem )
	{
		return;
	}

	pclArticleItem->SetID( pclAccessory->GetIDPtr().ID );
	pclArticleItem->SetDescription( pclAccessory->GetName() + CString( _T("; ") ) );
	pclArticleItem->AddDescription( pclAccessory->GetComment() );
	
	if( NULL != pclArticleContainer && true == bSelectionBySet )
	{
		// If current article is a sub article (attached or belonging to a set), we set the price to 0.0. When 'SelProdPageArtList' fill price column,
		// it set "-" if price is 0.0.
		pclArticleItem->SetPrice( 0.0 );
	}
	else
	{
		pclArticleItem->SetPrice( TASApp.GetpTADB()->GetPrice( pclAccessory->GetArtNum( true ) ) );
	}

	pclArticleItem->SetQuantity( iQuantity );
	pclArticleItem->SetIsAvailable( pclAccessory->IsAvailable() );
	pclArticleItem->SetIsDeleted( pclAccessory->IsDeleted() );
	pclArticleItem->SetIsAttached( pclAccessory->IsAttached() );
	pclArticleItem->SetIsSelectedInSet( bSelectionBySet );

	CString strArticleNumber;
	CString strLocArtNumber;

	if( NULL != pclArticleContainer && true == bSelectionBySet )
	{
		strArticleNumber = _T("-");
		strLocArtNumber = _T("-");
	}
	else
	{
		strArticleNumber = pclAccessory->GetArtNum();
		strLocArtNumber = LocArtNumTab.GetLocalArticleNumber( strArticleNumber );
	}

	pclArticleItem->CheckThingAvailability( pclAccessory, strArticleNumber, strLocArtNumber );

	if( true == strLocArtNumber.IsEmpty() )
	{
		strLocArtNumber = _T("-");
	}

	pclArticleItem->SetArticle( strArticleNumber );
	pclArticleItem->SetLocArtNum( strLocArtNumber );

	if( NULL != pclArticleContainer )
	{
		// Add accessory linked to the article.
		pclArticleContainer->AddAccessory( pclArticleItem );
	}
	else
	{
		// Add accessory as an article alone.
		CArticleGroup *pclArticleGroup = new CArticleGroup();

		if( NULL == pclArticleGroup )
		{
			return;
		}

		pclArticleGroup->AddArticle( pclArticleItem );
		
		// Remark: 'AddArticle' will call new CArticleContainer( pclArticleItem ) that copies content of 'pclArticleItem' and not saves the pointer.
		delete pclArticleItem;

		AddArticleInGroupList( pclArticleGroup );

		// Remark: pay attention that 'CPage::AddArticleInGroupList' will increment quantity if article already exist or
		//         will create a new 'CArticleGroup' and will copy to it data passed as argument. This is the reason
		//         why we need to delete 'pclArticleGroup' after the call.
		delete pclArticleGroup;
	}
}

void CSelProdPageBase::AddAccessoriesInArticleContainer( CAccessoryList *pclAccessoryList, CArticleContainer *pclArticleContainer, int iQuantity, 
		bool bSelectionBySet, int iDistributedQty, bool bForceToInsertInArticleContainer )
{
	if( NULL == pclAccessoryList || 0 == pclAccessoryList->GetCount() || iQuantity <= 0 || NULL == m_pclArticleGroupList )
	{
		return;
	}
	
	std::vector<CAccessoryList::AccessoryItem> vecAccessoryList;
	CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();

	while( rAccessoryItem.IDPtr.MP != NULL )
	{
		vecAccessoryList.push_back( rAccessoryItem );
		rAccessoryItem = pclAccessoryList->GetNext();
	}

	_VerifyAndAddAccessoriesInArticleContainer( vecAccessoryList, pclArticleContainer, iQuantity, bSelectionBySet, iDistributedQty, bForceToInsertInArticleContainer );
}

void CSelProdPageBase::AddAccessoriesInArticleContainer( std::vector<CDS_Actuator::AccessoryItem> *pvecAccessoryList, CArticleContainer *pclArticleContainer, 
		int iQuantity, bool bSelectionBySet, int iDistributedQty, bool bForceToInsertInArticleContainer )
{
	if( NULL == pvecAccessoryList || 0 == (int)pvecAccessoryList->size() || iQuantity <= 0 || NULL == m_pclArticleGroupList )
	{
		return;
	}

	std::vector<CAccessoryList::AccessoryItem> vecAccessoryList;

	for( int iLoopAccessory = 0; iLoopAccessory < (int)pvecAccessoryList->size(); iLoopAccessory++ )
	{
		CAccessoryList::AccessoryItem rAccessoryItem;
		rAccessoryItem.IDPtr = pvecAccessoryList->at( iLoopAccessory ).IDPtr;
		rAccessoryItem.fByPair = pvecAccessoryList->at( iLoopAccessory ).fByPair;
		// HYS-987: Handle actuator accessory quantity in article list when actuator is selected alone
		rAccessoryItem.lEditedQty = pvecAccessoryList->at( iLoopAccessory ).lEditedQty;
		rAccessoryItem.eAccessoryType = CAccessoryList::AccessoryType::_AT_Accessory;
		vecAccessoryList.push_back( rAccessoryItem );
	}

	_VerifyAndAddAccessoriesInArticleContainer( vecAccessoryList, pclArticleContainer, iQuantity, bSelectionBySet, iDistributedQty,	bForceToInsertInArticleContainer );
}

void CSelProdPageBase::AddAccessoriesInArticleContainer( CArray<IDPTR> *parAccessoryList, CDB_RuledTableBase *pclRuledTable, CArticleContainer *pclArticleContainer, 
			int iQuantity, bool bSelectionBySet, int iDistributedQty, bool bForceToInsertInArticleContainer )
{
	if( NULL == parAccessoryList || 0 == parAccessoryList->GetCount() || NULL == pclRuledTable || iQuantity <= 0 || NULL == m_pclArticleGroupList )
	{
		return;
	}

	CAccessoryList rAccessoryList;
	std::vector<CAccessoryList::AccessoryItem> vecAccessoryList;

	for( int i = 0; i < parAccessoryList->GetCount(); i++ )
	{
		rAccessoryList.Add( parAccessoryList->GetAt( i ), CAccessoryList::AccessoryType::_AT_Accessory, pclRuledTable );
		vecAccessoryList.push_back(	rAccessoryList.GetFirst() );
		rAccessoryList.Clear();
	}

	_VerifyAndAddAccessoriesInArticleContainer( vecAccessoryList, pclArticleContainer, iQuantity, bSelectionBySet, iDistributedQty,	bForceToInsertInArticleContainer );
}

void CSelProdPageBase::WriteTextWithFlags( CSheetDescription *pclSheetDescription, CString strText, long lColumn, long lRow, CSheetDescription::RemarkFlags eRemarkFlag, 
			CString strMessage, COLORREF cTextColor, COLORREF cBackColor )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();

	if( lRow > pclSheet->GetMaxRows() )
	{
		SetLastRow( pclSheetDescription, lRow );
		pclSheet->SetRowHeight( lRow, m_dRowHeight );
	}

	pclSheetDescription->WriteTextWithFlags( strText, lColumn, lRow, eRemarkFlag, strMessage, cTextColor, cBackColor );
}

void CSelProdPageBase::WriteTextWithFlags( CSheetDescription *pclSheetDescription, CString strText, long lColumn, long lRow, CSheetDescription::RemarkFlags eRemarkFlag, 
			std::vector<CString> vecErrorMessages, COLORREF cTextColor, COLORREF cBackColor )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();

	if( lRow > pclSheet->GetMaxRows() )
	{
		SetLastRow( pclSheetDescription, lRow );
		pclSheet->SetRowHeight( lRow, m_dRowHeight );
	}

	pclSheetDescription->WriteTextWithFlags( strText, lColumn, lRow, eRemarkFlag, vecErrorMessages, cTextColor, cBackColor );
}

void CSelProdPageBase::AddTooltipMessage( CPoint pt, CString strTooltipMessage )
{
	if( 0 == m_mapTooltipMessages.count( pt ) )
	{
		std::vector<CString> vecString = { strTooltipMessage };
		m_mapTooltipMessages.insert( std::pair<CPoint, std::vector<CString>>( pt, vecString ) );
	}
	else
	{
		m_mapTooltipMessages.at( pt ).push_back( strTooltipMessage );
	}
}

void CSelProdPageBase::AddTooltipMessage( CPoint pt, std::vector<CString> vecTooltipMessages )
{
	if( 0 == m_mapTooltipMessages.count( pt ) )
	{
		m_mapTooltipMessages.insert( std::pair<CPoint, std::vector<CString>>( pt, vecTooltipMessages ) );
	}
	else
	{
		for( auto &iter : vecTooltipMessages )
		{
			m_mapTooltipMessages.at( pt ).push_back( iter );
		}
	}
}

void CSelProdPageBase::_GetFirstProduct( CProductParam &clFirstProductParam, CSheetDescription *pclSheetDescription )
{
	clFirstProductParam.Clear();
	
	CSheetDescription *pclSDSearch = ( NULL == pclSheetDescription ) ? m_ViewDescription.GetTopSheetDescription() : pclSheetDescription;

	if( NULL == pclSDSearch || 0 == m_mapSheetInfos.count( pclSDSearch ) )
	{
		return;
	}

	if( 0 != m_mapSheetInfos[pclSDSearch].m_mapProductRegistered.size() )
	{
		clFirstProductParam = m_mapSheetInfos[pclSDSearch].m_mapProductRegistered.begin()->second;
	}
	else
	{
		CSheetDescription *pclNextSheetDescription = m_ViewDescription.GetNextSheetDescription( pclSDSearch );
		
		while( NULL != pclNextSheetDescription && (LPARAM)0 == clFirstProductParam.m_lpSelectionContainer )
		{
			if( 0 != m_mapSheetInfos.count( pclNextSheetDescription ) )
			{
				if( 0 != m_mapSheetInfos[pclNextSheetDescription].m_mapProductRegistered.size() )
				{
					clFirstProductParam = m_mapSheetInfos[pclNextSheetDescription].m_mapProductRegistered.begin()->second;
				}
			}
			
			pclNextSheetDescription = m_ViewDescription.GetNextSheetDescription( pclNextSheetDescription );
		}
	}
}

void CSelProdPageBase::_GetLastProduct( CProductParam &clLastProductParam, CSheetDescription *pclSheetDescription )
{
	clLastProductParam.Clear();
	
	CSheetDescription *pclSDSearch = ( NULL == pclSheetDescription ) ? m_ViewDescription.GetBottomSheetDescription() : pclSheetDescription;

	if( NULL == pclSDSearch || 0 == m_mapSheetInfos.count( pclSDSearch ) )
	{
		return;
	}

	if( 0 != m_mapSheetInfos[pclSDSearch].m_mapProductRegistered.size() )
	{
		clLastProductParam = m_mapSheetInfos[pclSDSearch].m_mapProductRegistered.rbegin()->second;
	}
	else
	{
		CSheetDescription *pclPrevSheetDescription = m_ViewDescription.GetPrevSheetDescription( pclSDSearch );
		
		while( NULL != pclPrevSheetDescription && (LPARAM)0 == clLastProductParam.m_lpSelectionContainer )
		{
			if( 0 != m_mapSheetInfos.count( pclPrevSheetDescription ) )
			{
				if( 0 != m_mapSheetInfos[pclPrevSheetDescription].m_mapProductRegistered.size() )
				{
					clLastProductParam = m_mapSheetInfos[pclPrevSheetDescription].m_mapProductRegistered.rbegin()->second;
				}
			}
			
			pclPrevSheetDescription = m_ViewDescription.GetPrevSheetDescription( pclPrevSheetDescription );
		}
	}
}

void CSelProdPageBase::_GetNextProduct( CProductParam &clFromProductParam, CProductParam &clDstProductParam )
{
	clDstProductParam.Clear();

	CSheetDescription *pclFromSD = clFromProductParam.m_pclSheetDescription;

	if( NULL == pclFromSD )
	{
		return;
	}

	if( 0 == m_mapSheetInfos.count( pclFromSD ) || 0 == m_mapSheetInfos[pclFromSD].m_mapProductRegistered.size() )
	{
		return;
	}

	// Retrieve the current product from the 'm_mapProductRegistered'.
	// Remark: the key of the 'm_mapProductRegistered' is the lower row where is saved the product.
	mapProductIter iterCurrent = m_mapSheetInfos[pclFromSD].m_mapProductRegistered.find( clFromProductParam.m_lSelectionTopRow );
	
	if( m_mapSheetInfos[pclFromSD].m_mapProductRegistered.end() == iterCurrent )
	{
		return;
	}

	iterCurrent++;
	
	if( m_mapSheetInfos[pclFromSD].m_mapProductRegistered.end() != iterCurrent )
	{
		clDstProductParam = iterCurrent->second;
	}
	else
	{
		// If there is no more product in the map, we have to go to the next sheet definition.
		CSheetDescription *pclNextSheetDescription = m_ViewDescription.GetNextSheetDescription( pclFromSD );
		
		while( NULL != pclNextSheetDescription && (LPARAM)0 == clDstProductParam.m_lpSelectionContainer )
		{
			if( 0 != m_mapSheetInfos.count( pclNextSheetDescription ) )
			{
				// Pay attention. It's absolutely possible to have two (or more) consecutive entries in the 'm_mapSheetInfos' with the same
				// registered product. It's typically the case for the 'SelProdPagePressMaint' where we have different sheet descriptions
				// (one for main title, one for input and calculated data, one for product and one for the direct selection).
				if( 0 != m_mapSheetInfos[pclNextSheetDescription].m_mapProductRegistered.size() )
				{
					iterCurrent = m_mapSheetInfos[pclNextSheetDescription].m_mapProductRegistered.begin();
					
					while( m_mapSheetInfos[pclNextSheetDescription].m_mapProductRegistered.end() != iterCurrent &&
							clFromProductParam.m_lpSelectionContainer == iterCurrent->second.m_lpSelectionContainer )
					{
						iterCurrent++;
					}
					
					if( m_mapSheetInfos[pclNextSheetDescription].m_mapProductRegistered.end() != iterCurrent )
					{
						clDstProductParam = iterCurrent->second;
					}
				}
			}
			
			pclNextSheetDescription = m_ViewDescription.GetNextSheetDescription( pclNextSheetDescription );
		}
	}
}

void CSelProdPageBase::_GetPrevProduct( CProductParam &clFromProductParam, CProductParam &clDstProductParam )
{
	clDstProductParam.Clear();

	CSheetDescription *pclFromSD = clFromProductParam.m_pclSheetDescription;

	if( NULL == pclFromSD )
	{
		return;
	}

	if( 0 == m_mapSheetInfos.count( pclFromSD ) || 0 == m_mapSheetInfos[pclFromSD].m_mapProductRegistered.size() )
	{
		return;
	}

	// Retrieve the current product from the 'm_mapProductRegistered'.
	mapProductIter iterCurrent = m_mapSheetInfos[pclFromSD].m_mapProductRegistered.find( clFromProductParam.m_lSelectionTopRow );
	
	if( m_mapSheetInfos[pclFromSD].m_mapProductRegistered.end() == iterCurrent )
	{
		return;
	}

	if( m_mapSheetInfos[pclFromSD].m_mapProductRegistered.begin() != iterCurrent )
	{
		iterCurrent--;
		clDstProductParam = iterCurrent->second;
	}
	else
	{
		// If there is no more product in the map, we have to go to the previous sheet definition.
		CSheetDescription *pclPreviousSheetDescription = m_ViewDescription.GetPrevSheetDescription( pclFromSD );
		
		while( NULL != pclPreviousSheetDescription && (LPARAM)0 == clDstProductParam.m_lpSelectionContainer )
		{
			if( 0 != m_mapSheetInfos.count( pclPreviousSheetDescription ) )
			{
				// Pay attention. It's absolutely possible to have two (or more) consecutive entries in the 'm_mapSheetInfos' with the same
				// registered product. It's typically the case for the 'SelProdPagePressMaint' where we have different sheet descriptions
				// (one for main title, one for input and calculated data, one for product and one for the direct selection).
				if( 0 != m_mapSheetInfos[pclPreviousSheetDescription].m_mapProductRegistered.size() )
				{
					mapProduct::reverse_iterator riterCurrent = m_mapSheetInfos[pclPreviousSheetDescription].m_mapProductRegistered.rbegin();
					
					while( m_mapSheetInfos[pclPreviousSheetDescription].m_mapProductRegistered.rend() != riterCurrent &&
							clFromProductParam.m_lpSelectionContainer == riterCurrent->second.m_lpSelectionContainer )
					{
						riterCurrent++;
					}
					
					if( m_mapSheetInfos[pclPreviousSheetDescription].m_mapProductRegistered.rend() != riterCurrent )
					{
						clDstProductParam = riterCurrent->second;
					}
				}
			}
			
			pclPreviousSheetDescription = m_ViewDescription.GetPrevSheetDescription( pclPreviousSheetDescription );
		}

		// Try now to take the first sheetdescription of a group (It's the case for pressurisation selection).
		if( (LPARAM)0 != clDstProductParam.m_lpSelectionContainer )
		{
			bool bCanStop = false;

			while( NULL != pclPreviousSheetDescription && false == bCanStop )
			{
				if( 0 != m_mapSheetInfos.count( pclPreviousSheetDescription ) )
				{
					if( 0 != m_mapSheetInfos[pclPreviousSheetDescription].m_mapProductRegistered.size() )
					{
						mapProduct::reverse_iterator riterCurrent = m_mapSheetInfos[pclPreviousSheetDescription].m_mapProductRegistered.rbegin();

						if( m_mapSheetInfos[pclPreviousSheetDescription].m_mapProductRegistered.rend() != riterCurrent )
						{
							if( clDstProductParam.m_lpSelectionContainer == riterCurrent->second.m_lpSelectionContainer )
							{
								clDstProductParam = riterCurrent->second;
							}
							else
							{
								bCanStop = true;
							}
						}
					}
				}

				pclPreviousSheetDescription = m_ViewDescription.GetPrevSheetDescription( pclPreviousSheetDescription );
			}
		}
	}
}

bool CSelProdPageBase::_GetProduct( CSheetDescription *pclSheetDescription, long lRow, CProductParam &clProductParam )
{
	clProductParam.Clear();

	if( NULL == pclSheetDescription || 0 == m_mapSheetInfos.count( pclSheetDescription ) ||	0 == m_mapSheetInfos[pclSheetDescription].m_mapProductRegistered.size() )
	{
		return false;
	}

	bool bReturn = false;

	for( mapProductIter iter = m_mapSheetInfos[pclSheetDescription].m_mapProductRegistered.begin();
			iter != m_mapSheetInfos[pclSheetDescription].m_mapProductRegistered.end() && false == bReturn ; iter++ )
	{
		if( lRow >= iter->second.m_lSelectionTopRow && lRow <= iter->second.m_lSelectionBottomRow )
		{
			clProductParam = iter->second;
			bReturn = true;
		}
	}

	// Try now to take the first sheetdescription of a group (It's the case for pressurisation selection).
	if( (LPARAM)0 != clProductParam.m_lpSelectionContainer )
	{
		CSheetDescription *pclPreviousSheetDescription = m_ViewDescription.GetPrevSheetDescription( pclSheetDescription );
		bool bCanStop = false;

		while( NULL != pclPreviousSheetDescription && false == bCanStop )
		{
			if( 0 != m_mapSheetInfos.count( pclPreviousSheetDescription ) )
			{
				if( 0 != m_mapSheetInfos[pclPreviousSheetDescription].m_mapProductRegistered.size() )
				{
					mapProduct::reverse_iterator riterCurrent = m_mapSheetInfos[pclPreviousSheetDescription].m_mapProductRegistered.rbegin();

					if( m_mapSheetInfos[pclPreviousSheetDescription].m_mapProductRegistered.rend() != riterCurrent ) 
					{
						if( clProductParam.m_lpSelectionContainer == riterCurrent->second.m_lpSelectionContainer )
						{
							clProductParam = riterCurrent->second;
						}
						else
						{
							bCanStop = true;
						}
					}
				}
			}

			pclPreviousSheetDescription = m_ViewDescription.GetPrevSheetDescription( pclPreviousSheetDescription );
		}
	}
	
	return bReturn;
}

CRect CSelProdPageBase::_GetSelectionSize( CProductParam &clProductSelected )
{
	CRect rect( 0, 0, 0, 0 );

	if( 0 == (int)m_vecProductSelected.size() )
	{
		return rect;
	}
	
	std::map<CProductParam *, bool> mapAlreadyDone;

	int iIndex = -1;
	CProductParam *pclProduct = NULL;
	
	if( (LPARAM)0 == clProductSelected.m_lpSelectionContainer )
	{
		// If no 'CProductParam' defined in the argument, we take the first one product selected in the list.
		iIndex = 0;
		pclProduct = &m_vecProductSelected[iIndex];
	}
	else
	{
		// Otherwise we take the 'clProductSelected'.
		pclProduct = &clProductSelected;
	}

	// Loop on all selected product on the list or check only the 'clProductSelected'.
	while( NULL != pclProduct )
	{
		CProductParam *pclMainProduct = pclProduct;
		int iIndexLinked = -1;

		while( NULL != pclProduct )
		{
			if( 0 == mapAlreadyDone.count( pclProduct ) && NULL != pclProduct->m_pclSheetDescription && NULL != pclProduct->m_pclSheetDescription->GetSSheetPointer() )
			{
				CSSheet *pclSheet = pclProduct->m_pclSheetDescription->GetSSheetPointer();
				CRect rectLogical = CRect( 1, pclProduct->m_lSelectionTopRow, pclSheet->GetMaxCols(), pclProduct->m_lSelectionBottomRow );
				CRect rectPixel = pclSheet->GetSelectionCoordInPixels( rectLogical );
				pclSheet->ClientToScreen( &rectPixel );
				ScreenToClient( &rectPixel );
				rectPixel.NormalizeRect();
				rect.UnionRect( &rect, &rectPixel );
				mapAlreadyDone[pclProduct] = true;
			}

			pclProduct = NULL;

			if( -1 == iIndexLinked )
			{
				// Check if there are linked products to the main one.
				if( pclMainProduct->m_vecLinkedProductParam.size() > 0 )
				{
					// If it's the case, we do a loop on all linked products.
					iIndexLinked = 0;
					pclProduct = &pclMainProduct->m_vecLinkedProductParam[0];
				}
			}
			else
			{
				if( iIndexLinked < (int)pclMainProduct->m_vecLinkedProductParam.size() - 1 )
				{
					// Go to the next linked product.
					iIndexLinked++;
					pclProduct = &pclMainProduct->m_vecLinkedProductParam[iIndexLinked];
				}
			}
		}
		
		pclProduct = NULL;

		if( -1 != iIndex )
		{
			// Go to the next selected product.
			if( iIndex < (int)m_vecProductSelected.size() - 1 )
			{
				iIndex++;
				pclProduct = &m_vecProductSelected[iIndex];
			}
		}
	}
	return rect;
}

CRect CSelProdPageBase::_GetScrollingRange( CProductParam &clProductSelected )
{
	CRect rect( 0, 0, 0, 0 );

	if( 0 == (int)m_vecProductSelected.size() )
	{
		return rect;
	}
	
	std::map<CProductParam *, bool> mapAlreadyDone;

	int iIndex = -1;
	CProductParam *pclProduct = NULL;

	if( (LPARAM)0 == clProductSelected.m_lpSelectionContainer )
	{
		// If no 'CProductParam' defined in the argument, we take the first one product selected in the list.
		iIndex = 0;
		pclProduct = &m_vecProductSelected[iIndex];
	}
	else
	{
		// Otherwise we take the 'clProductSelected'.
		pclProduct = &clProductSelected;
	}

	// Loop on all selected product on the list or check only the 'clProductSelected'.
	long lScrollTopRow, lScrollBottomRow;

	while( NULL != pclProduct )
	{
		CProductParam *pclMainProduct = pclProduct;
		int iIndexLinked = -1;

		while( NULL != pclProduct )
		{
			if( 0 == mapAlreadyDone.count( pclProduct ) && NULL != pclProduct->m_pclSheetDescription && NULL != pclProduct->m_pclSheetDescription->GetSSheetPointer() )
			{
				CSSheet *pclSheet = pclProduct->m_pclSheetDescription->GetSSheetPointer();
				pclProduct->GetScrollRange( lScrollTopRow, lScrollBottomRow );
				CRect rectLogical = CRect( 1, lScrollTopRow, pclSheet->GetMaxCols(), lScrollBottomRow );
				CRect rectPixel = pclSheet->GetSelectionCoordInPixels( rectLogical );
				pclSheet->ClientToScreen( &rectPixel );
				ScreenToClient( &rectPixel );
				rectPixel.NormalizeRect();
				rect.UnionRect( &rect, &rectPixel );
				mapAlreadyDone[pclProduct] = true;
			}

			pclProduct = NULL;

			if( -1 == iIndexLinked )
			{
				// Check if there are linked products to the main one.
				if( pclMainProduct->m_vecLinkedProductParam.size() > 0 )
				{
					// If it's the case, we do a loop on all linked products.
					iIndexLinked = 0;
					pclProduct = &pclMainProduct->m_vecLinkedProductParam[0];
				}
			}
			else
			{
				if( iIndexLinked < (int)pclMainProduct->m_vecLinkedProductParam.size() - 1 )
				{
					// Go to the next linked product.
					iIndexLinked++;
					pclProduct = &pclMainProduct->m_vecLinkedProductParam[iIndexLinked];
				}
			}
		}
		
		pclProduct = NULL;

		if( -1 != iIndex )
		{
			// Go to the next selected product.
			if( iIndex < (int)m_vecProductSelected.size() - 1 )
			{
				iIndex++;
				pclProduct = &m_vecProductSelected[iIndex];
			}
		}
	}
	
	return rect;
}

int CSelProdPageBase::_CompareTwoProductPosition( CProductParam &clFirstProductParam, CProductParam &clSecondProductParam )
{
	if( (LPARAM)0 == clFirstProductParam.m_lpSelectionContainer || (LPARAM)0 == clSecondProductParam.m_lpSelectionContainer )
	{
		return CompareProduct_Error;
	}
	
	int iReturn;

	switch( m_ViewDescription.CompareTwoSDPosition( clFirstProductParam.m_pclSheetDescription, clSecondProductParam.m_pclSheetDescription ) )
	{
		case CViewDescription::CompareSD_Error:
			iReturn = CompareProduct_Error;
			break;

		case CViewDescription::CompareSD_FirstAboveSecond:
			iReturn = CompareProduct_FirstAboveSecond;
			break;

		case CViewDescription::CompareSD_FirstBelowSecond:
			iReturn = CompareProduct_FirstBelowSecond;
			break;

		case CViewDescription::CompareSD_FirstEqualSecond:

			if( clFirstProductParam.m_lSelectionTopRow < clSecondProductParam.m_lSelectionTopRow )
			{
				iReturn = CompareProduct_FirstAboveSecond;
			}
			else if( clFirstProductParam.m_lSelectionTopRow > clSecondProductParam.m_lSelectionTopRow )
			{
				iReturn = CompareProduct_FirstBelowSecond;
			}
			else
			{
				iReturn = CompareProduct_FirstEqualSecond;
			}
			break;
	}
	
	return iReturn;
}

bool CSelProdPageBase::_IsProductRegistered( CSheetDescription *pclSheetDescription, LPARAM lpParam, CProductParam &clProductParam )
{
	clProductParam.Clear();

	if( NULL == pclSheetDescription || 0 == m_mapSheetInfos.count( pclSheetDescription )
			|| 0 == m_mapSheetInfos[pclSheetDescription].m_mapProductRegistered.size() || NULL == lpParam )
	{
		return false;
	}

	bool bReturn = false;

	for( mapProductIter iter = m_mapSheetInfos[pclSheetDescription].m_mapProductRegistered.begin();
			iter != m_mapSheetInfos[pclSheetDescription].m_mapProductRegistered.end() && false == bReturn; iter++ )
	{
		if( lpParam == iter->second.m_lpSelectionContainer )
		{
			clProductParam = iter->second;
			bReturn = true;
		}
	}
	
	return bReturn;
}

bool CSelProdPageBase::_IsProductExist( LPARAM lpObj, CProductParam &clProductParam, CSheetDescription *pclSheetDescription )
{
	clProductParam.Clear();
	bool bFound = false;

	for( mapSheetInfoIter mapSheetIter = m_mapSheetInfos.begin(); mapSheetIter != m_mapSheetInfos.end() && false == bFound; mapSheetIter++ )
	{
		for( mapProductIter mapProductIter = mapSheetIter->second.m_mapProductRegistered.begin(); mapProductIter != mapSheetIter->second.m_mapProductRegistered.end() && false == bFound; mapProductIter++ )
		{
			if( lpObj == mapProductIter->second.m_lpSelectionContainer )
			{
				clProductParam = mapProductIter->second;
				bFound = true;
			}
		}
	}

	return bFound;
}

void CSelProdPageBase::_SelectCell( CProductParam &clProductToSelect, CSSheet::_SSKeyBoardPressed eKeyPressed )
{
	if( NULL == clProductToSelect.m_pclSheetDescription || NULL == clProductToSelect.m_pclSheetDescription->GetSSheetPointer()
			|| 0 == m_mapSheetInfos.count( clProductToSelect.m_pclSheetDescription ) )
	{
		return;
	}

	CSheetDescription *pclSheetDescription = clProductToSelect.m_pclSheetDescription;
	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();

	pclSheet->SetFocus();

	_InvalidateSSheetRect();

	int iCurrentSelectedIndex = m_iProductSelectedIndex;
	m_iProductSelectedIndex = -1;
	
	if( CSSheet::_SSKeyBoardPressed::NoKey == eKeyPressed )
	{
		// Warn the inherited class with all the previous products selected.
		for( auto &iter : m_vecProductSelected )
		{
			OnUnselectCell( iter );
		}

		m_vecProductSelected.clear();
		m_vecSelectionBorderInfo.clear();

		// Remark: only if we are on a selectable product.
		if( (LPARAM)0 != clProductToSelect.m_lpSelectionContainer )
		{
			// Select all range defined.
			m_eSelectionKey = Selection_NoKey;
			_SelectCHelper( clProductToSelect );

			// Warn the inherited class with the current product selected.
			OnSelectCell( clProductToSelect );
			
			m_vecProductSelected.push_back( clProductToSelect );
			m_iProductSelectedIndex = m_vecProductSelected.size() - 1;
		}
	}
	else if( CSSheet::_SSKeyBoardPressed::CtrlKey == eKeyPressed )
	{
		// If the ctrl key were pressed, add it to the selection.

		// Verify the new selection is part of the same table.
		// If not do not accept to add it.
		bool bSkip = false;

		if( m_vecProductSelected.size() > 0 )
		{
			// Retrieve in which table belongs the current object under selection (at 'lRow' position).
			CData *pObjectUnderSelection = (CData *)clProductToSelect.m_lpSelectionContainer;
			CTable *pTab = NULL;

			if( NULL != pObjectUnderSelection )
			{
				pTab = (CTable *)( pObjectUnderSelection->GetIDPtr().PP );
			}

			// Retrieve in which table belongs the objects already selected.
			LPARAM lpObj = m_vecProductSelected[0].m_lpSelectionContainer;
			CData *pFirstObjectSelected = (CData *)lpObj;
			CTable *pFirstTab = NULL;

			if( NULL != pFirstObjectSelected )
			{
				pFirstTab = (CTable *)( pFirstObjectSelected->GetIDPtr().PP );
			}

			// Verify that these two tables are the same. Because we do not accept multi selection on object from
			// different type of selection.
			if( pTab != pFirstTab )
			{
				bSkip = true;
			}
		}

		if( false == bSkip )
		{
			// Verify if the object is already selected.
			// In that case, remove the selection.
			if( m_vecProductSelected.size() > 1 )
			{
				for( vecProductSelectedIter iter = m_vecProductSelected.begin(); iter != m_vecProductSelected.end(); iter++ )
				{
					if( clProductToSelect.m_lpSelectionContainer == iter->m_lpSelectionContainer )
					{
						for( std::vector<SelectionBorderInfo>::iterator iterBorder = m_vecSelectionBorderInfo.begin(); iterBorder != m_vecSelectionBorderInfo.end(); iterBorder++ )
						{
							if( iterBorder->m_ptTopLeftColRow.y == iter->m_lSelectionTopRow && iterBorder->m_ptBottomLeftColRow.y == iter->m_lSelectionBottomRow )
							{
								m_vecSelectionBorderInfo.erase( iterBorder );
								break;
							}
						}

						m_vecProductSelected.erase( iter );

						// Warn the inherited class with the current product selected.
						OnUnselectCell( *iter );

						bSkip = true;
						break;
					}
				}
			}
		}

		if( false == bSkip )
		{
			// Do the selection only if we are on a selectable product.
			if( (LPARAM)0 != clProductToSelect.m_lpSelectionContainer )
			{
				// Select all range defined.
				m_eSelectionKey = Selection_Ctrl;
				_SelectCHelper( clProductToSelect );
				
				m_vecProductSelected.push_back( clProductToSelect );
				m_iProductSelectedIndex = m_vecProductSelected.size() - 1;

				// Warn the inherited class with the current product selected.
				OnSelectCell( clProductToSelect );
			}
		}
	}
	else if( CSSheet::_SSKeyBoardPressed::ShiftKey == eKeyPressed )
	{
		// If the shift key were pressed, keep the first select object, all the others will be added at the end of the function depending
		// on the new user selection.

		// If there is at least one object already selected...
		if( m_vecProductSelected.size() > 0 )
		{
			// Retrieve in which table belongs the object to select.
			CData *pObjectToSelect = (CData *)clProductToSelect.m_lpSelectionContainer;
			CTable *pTableOfTheObjectToSelect = NULL;

			if( NULL != pObjectToSelect )
			{
				pTableOfTheObjectToSelect = (CTable *)( pObjectToSelect->GetIDPtr().PP );
			}

			// Retrieve in which table belongs the first object already selected.
			LPARAM lpObj = m_vecProductSelected[0].m_lpSelectionContainer;
			CData *pFirstObjectSelected = (CData *)lpObj;
			CTable *pTableOfTheFirstObjectSelected = NULL;

			if( NULL != pFirstObjectSelected )
			{
				pTableOfTheFirstObjectSelected = (CTable *)( pFirstObjectSelected->GetIDPtr().PP );
			}

			// Verify that these two tables are the same. Because we do not accept multi selection on object from
			// different type of selection.
			bool bSkip = false;

			if( pTableOfTheObjectToSelect != pTableOfTheFirstObjectSelected )
			{
				bSkip = true;
			}

			if( false == bSkip )
			{
				// Keep only the first selected object (because range selection will be done just after).
				CProductParam clCurrentSelected = m_vecProductSelected[iCurrentSelectedIndex];

				// Warn the inherited class with all the previous products selected.
				for( auto &iter : m_vecProductSelected )
				{
					OnUnselectCell( iter );
				}

				// Select only if user has not clicked on the same product...
				if( clCurrentSelected.m_lpSelectionContainer != clProductToSelect.m_lpSelectionContainer )
				{
					CProductParam clNextProduct;
					CProductParam clLastProduct;
					clNextProduct.Clear();
					int iResult = _CompareTwoProductPosition( clCurrentSelected, clProductToSelect );
					
					switch( iResult )
					{
						case CompareProduct_Error:
						case CompareProduct_FirstEqualSecond:
							break;

						case CompareProduct_FirstBelowSecond:
							clNextProduct = clProductToSelect;
							clLastProduct = m_vecProductSelected.back();
							break;

						case CompareProduct_FirstAboveSecond:
							clNextProduct = m_vecProductSelected.front();
							clLastProduct = clProductToSelect;
							break;
					}

					m_vecProductSelected.clear();

					m_vecSelectionBorderInfo.clear();
					m_eSelectionKey = Selection_Shift;

					// Select now between.
					while( (LPARAM)0 != clNextProduct.m_lpSelectionContainer )
					{
						// Select it.
						_SelectCHelper( clNextProduct );
						m_vecProductSelected.push_back( clNextProduct );

						// Warn the inherited class with the current product selected.
						OnSelectCell( clNextProduct );

						if( clNextProduct.m_lpSelectionContainer != clLastProduct.m_lpSelectionContainer )
						{
							// Try to get the next one.
							CProductParam clTempProduct;
							_GetNextProduct( clNextProduct, clTempProduct );
							clNextProduct = clTempProduct;
						}
						else
						{
							clNextProduct.Clear();
						}
					}

					if( m_vecProductSelected.size() > 0 )
					{
						if( CompareProduct_FirstBelowSecond == iResult )
						{
							m_iProductSelectedIndex = m_vecProductSelected.size() - 1;
						}
						else if( CompareProduct_FirstAboveSecond == iResult )
						{
							m_iProductSelectedIndex = 0;
						}
					}
				}
				else
				{
					// In this case, we keep the previous selected product.
					m_vecProductSelected.clear();
					m_vecProductSelected.push_back( clProductToSelect );

					m_vecSelectionBorderInfo.clear();
					m_eSelectionKey = Selection_Shift;
					
					_SelectCHelper( clProductToSelect );
					m_iProductSelectedIndex = 0;

					// Warn the inherited class with all the current product selected.
					OnSelectCell( clProductToSelect );
				}
			}
		}
	}
	else
	{
		ASSERT( 0 );
	}

	_InvalidateSSheetRect();
}

void CSelProdPageBase::_SelectCHelper( CProductParam &clProductToSelect, bool bFirst )
{
	if( NULL == clProductToSelect.m_pclSheetDescription || NULL == clProductToSelect.m_pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	// Select all range defined.
	long lFirstRow, lLastRow;
	SelectionBorderInfo rSelectionBorderInfo;

	if( true == clProductToSelect.GetFirstSelectioRange( lFirstRow, lLastRow ) )
	{
		long lLastColumn = m_mapSheetInfos[clProductToSelect.m_pclSheetDescription].m_iColNum - 2;

		rSelectionBorderInfo.m_ptTopLeftColRow = CPoint( 2, lFirstRow );
		rSelectionBorderInfo.m_ptTopRightColRow = CPoint( lLastColumn, lFirstRow );
		rSelectionBorderInfo.m_pclSheetDescription = clProductToSelect.m_pclSheetDescription;

		// To get the last selectable range.
		while( true == clProductToSelect.GetNextSelectionRange( lFirstRow, lLastRow ) );

		rSelectionBorderInfo.m_ptBottomLeftColRow = CPoint( 2, lLastRow );
		rSelectionBorderInfo.m_ptBottomRightColRow = CPoint( m_mapSheetInfos[clProductToSelect.m_pclSheetDescription].m_iColNum - 2, lLastRow );
	}

	// And select all linked sheet. For example for pressure maintenance, we have three sheets for the same product. One sheet for the
	// 'Input data / Calculated data', one sheet for 'Product' and the last for the graphs.
	if( (short)clProductToSelect.m_vecLinkedProductParam.size() > 0 )
	{
		for( auto &iter : clProductToSelect.m_vecLinkedProductParam )
		{
			SelectionBorderInfo rLinkedSelectionBorderInfo;

			if( true == iter.GetFirstSelectioRange( lFirstRow, lLastRow ) )
			{
				long lLastColumn = m_mapSheetInfos[iter.m_pclSheetDescription].m_iColNum - 2;

				rLinkedSelectionBorderInfo.m_ptTopLeftColRow = CPoint( 2, lFirstRow );
				rLinkedSelectionBorderInfo.m_ptTopRightColRow = CPoint( lLastColumn, lFirstRow );
				rLinkedSelectionBorderInfo.m_pclSheetDescription = iter.m_pclSheetDescription;

				// To get the last selectable range.
				while( true == iter.GetNextSelectionRange( lFirstRow, lLastRow ) );

				rLinkedSelectionBorderInfo.m_ptBottomLeftColRow = CPoint( 2, lLastRow );
				rLinkedSelectionBorderInfo.m_ptBottomRightColRow = CPoint( m_mapSheetInfos[iter.m_pclSheetDescription].m_iColNum - 2, lLastRow );

				rSelectionBorderInfo.m_vecLinkedSelectionBorderInfo.push_back( rLinkedSelectionBorderInfo );
			}
		}
	}

	m_vecSelectionBorderInfo.push_back( rSelectionBorderInfo );
}

void CSelProdPageBase::_VerifyAndAddAccessoriesInArticleContainer( std::vector<CAccessoryList::AccessoryItem> vecAccessoryList, 
		CArticleContainer *pclArticleContainer, int iQuantity, bool bSelectionBySet, int iDistributedQty, bool bForceToInsertInArticleContainer )
{
	std::vector<CAccessoryList::AccessoryItem>::iterator iter;

	for( iter = vecAccessoryList.begin(); iter != vecAccessoryList.end(); ++iter )
	{
		CDB_Product *pclAccessory = dynamic_cast<CDB_Product*>( (CData*)( *iter ).IDPtr.MP );

		if( NULL != pclAccessory && pclAccessory->IsAnAccessory() )
		{
			int iTotalQuantity = 1;
			// HYS-987: Handle editable quantity in article list
			if( -1 != ( *iter ).lEditedQty )
			{
				iTotalQuantity = ( *iter ).lEditedQty;
			}
			else if( true == ( *iter ).fByPair )
			{
				iTotalQuantity = 2;
				iTotalQuantity *= iQuantity;
			}
			else if( true == ( *iter ).fDistributed && iDistributedQty > 1 )
			{
				iTotalQuantity = iDistributedQty;
				iTotalQuantity *= iQuantity;
			}
			else
			{
				iTotalQuantity *= iQuantity;
			}

			if( true == bForceToInsertInArticleContainer || true == pclAccessory->IsAttached() 
					|| CAccessoryList::AccessoryType::_AT_SetAccessory == ( *iter ).eAccessoryType )
			{
				// If accessory is attached (cannot be sold alone) or if it's an accessory belonging to a set, we add this accessory as a 
				// sub article.
				AddAccessoryInArticleContainer( pclAccessory, pclArticleContainer, iTotalQuantity, bSelectionBySet );
			}
			else
			{
				// If second argument is NULL, accessory will be added as a article.
				AddAccessoryInArticleContainer( pclAccessory, NULL, iTotalQuantity, bSelectionBySet );				
			}
		}
	}
}

bool CSelProdPageBase::_TestForSameArticle( CArticleGroup *pclArticleGroupToTest, bool bIncrementation )
{
	if( NULL == pclArticleGroupToTest || NULL == pclArticleGroupToTest->GetFirstArticleContainer() 
			|| NULL == pclArticleGroupToTest->GetFirstArticleContainer()->GetArticleItem() || NULL == m_pclArticleGroupList )
	{
		return false;
	}

	bool bExist = false;
	bool bAccExactlySame = false;
	bool bAccDiff = true;

	CArticleContainer *pclArticleContainerToTest = pclArticleGroupToTest->GetFirstArticleContainer();
	CArticleGroup *pclLoopArticleGroup = m_pclArticleGroupList->GetFirstArticleGroup();
	
	// Loop on the article container list; if this item exists increment quantity, else add this article to the list.
	while( NULL != pclLoopArticleGroup && false == bExist && NULL != pclLoopArticleGroup->GetFirstArticleContainer() 
			&& NULL != pclLoopArticleGroup->GetFirstArticleContainer()->GetArticleItem() )
	{
		CArticleContainer *pclLoopArticleContainer = pclLoopArticleGroup->GetFirstArticleContainer();

		if( pclLoopArticleContainer->GetArticleItem()->GetID() == pclArticleContainerToTest->GetArticleItem()->GetID() )
		{
			// Test if the the articles have the same number of sub articles.
			if( pclLoopArticleContainer->GetAccessoryList()->size() != pclArticleContainerToTest->GetAccessoryList()->size() )
			{
				return false;
			}
			
			// If it isn't the same article -> it returns false because of the !
			bAccDiff = false;
			CArticleItem *pclLoopAccessory = pclLoopArticleContainer->GetFirstAccessory();
			
			while( NULL != pclLoopAccessory )
			{
				CArticleItem *pclLoopAccessoryToTest = pclArticleContainerToTest->GetFirstAccessory();
				
				while( NULL != pclLoopAccessoryToTest )
				{
					// Test if the sub articles match.
					if( pclLoopAccessory->GetID() == pclLoopAccessoryToTest->GetID() )
					{
						if( true == bIncrementation )
						{
							pclLoopAccessory->AddQuantity( pclLoopAccessoryToTest->GetQuantity() );
						}

						bAccExactlySame = true;
					}

					pclLoopAccessoryToTest = pclArticleContainerToTest->GetNextAccessory();
				}
				
				if( true == bAccExactlySame )
				{
					bAccExactlySame = false;
				}
				else
				{
					bAccDiff = true;
				}

				pclLoopAccessory = pclLoopArticleContainer->GetNextAccessory();
			}
			
			if( false == bAccDiff )
			{
				bExist = true;

				if( true == bIncrementation )
				{
					pclLoopArticleContainer->GetArticleItem()->AddQuantity( pclArticleContainerToTest->GetArticleItem()->GetQuantity() );
				}
			}
		}

		pclLoopArticleGroup = m_pclArticleGroupList->GetNextArticleGroup();
	}

	if( true == bAccDiff )
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool CSelProdPageBase::_ComparePairs( PairPtr pp1, PairPtr pp2 )
{
	if( NULL == TASApp.GetpTADS() )
	{
		ASSERTA_RETURN( false );
	}

	// We can't take here the 'm_pTADSPageSetup' protected variables. Because static method has no access to it.
	CDB_PageSetup *pclPageSetup = TASApp.GetpTADS()->GetpPageSetup();

	if( NULL == pclPageSetup )
	{
		ASSERTA_RETURN( false );
	}

	// TO COMPLETE: the 'CDB_Vessel', 'CDB_Compresso' and 'CDB_Transfero' objects are not inherited from a 'CDB_TAProduct' object.

	CDS_HydroMod *pHM1 = NULL;
	CDS_HydroMod *pHM2 = NULL;
	CDB_Product *pclProduct1 = NULL;
	CDB_Product *pclProduct2 = NULL;

	for( int i = 0; i < 2; i++ )
	{
		CDS_HydroMod **parHM = ( 0 == i ) ? &pHM2 : &pHM1;
		CDB_Product ** parProduct = ( 0 == i ) ? &pclProduct2 : &pclProduct1;
		PairPtr *pp = ( 0 == i ) ? &pp2 : &pp1;

		if( eptHM == pp->first )
		{
			*parHM = dynamic_cast<CDS_HydroMod *>( (CData *)pp->second );
			*parProduct = dynamic_cast<CDB_Product *>( (*parHM )->GetCBIValveIDPtr().MP );
		}
		else
		{
			CDS_HydroMod::CBase *pBase = static_cast<CDS_HydroMod::CBase *>( pp->second );
			*parHM = dynamic_cast<CDS_HydroMod *>( pBase->GetpParentHM() );
			*parProduct = dynamic_cast<CDB_Product *>( pBase->GetTAPIDPtr().MP );
		}
	}

	if( NULL == pHM1 || NULL == pHM2 )
	{
		ASSERTA_RETURN( false );
	}
	
	PageField_enum eKey = PageField_enum::epfNONE;
	bool bSortExist = false;

	for( int i = 0; i < 3; i++ )
	{
		eKey = pclPageSetup->GetKey( i );
		
		if( PageField_enum::epfNONE != eKey )
		{
			CString str1, str2;
			bSortExist = true;
			
			switch( eKey )								// Sorting Key
			{
				case epfFIRSTREF:
					str1 = pHM1->GetHMName();
					str2 = pHM2->GetHMName();
					break;

				case epfSECONDREF:
					str1 = pHM1->GetDescription();
					str2 = pHM2->GetDescription();
					break;

				case epfSEPARATORAIRVENTINFOPRODUCTNAME:
				case epfSTATICOINFOPRODUCTNAME:
				case epfTECHBOXINFOPRODUCTNAME:
				case epfBVINFOPRODUCTNAME:
				case epfDPCINFOPRODUCTNAME:
				case epfDPCBCVINFOPRODUCTNAME:
				case epfRD7INFOPRODUCTNAME:
				case epfPDINFOPRODUCTNAME:
				case epfCVINFOPRODUCTNAME:
				case epfSMARTCONTROLVALVEINFOPRODUCTNAME:
				case epf6WAYVALVEINFOPRODUCTNAME:
			
					if( NULL != pclProduct1 && NULL != pclProduct2 )
					{
						str1 = pclProduct1->GetName();
						str2 = pclProduct2->GetName();
					}

					break;

				case epfSEPARATORAIRVENTINFOPRODUCTSIZE:
				case epfBVINFOPRODUCTSIZE:
				case epfDPCINFOPRODUCTSIZE:
				case epfDPCBCVINFOPRODUCTSIZE:
				case epfRD7INFOPRODUCTSIZE:
				case epfPDINFOPRODUCTSIZE:
					
					//	Sorting on SizeID
					if( NULL != pclProduct1 && NULL != pclProduct2 && NULL != dynamic_cast<CDB_TAProduct *>( pclProduct1 ) 
							&& NULL != dynamic_cast<CDB_TAProduct *>( pclProduct2 ) )
					{
						str1 = dynamic_cast<CDB_TAProduct *>( pclProduct1 )->GetSizeID();
						str2 = dynamic_cast<CDB_TAProduct *>( pclProduct2 )->GetSizeID();
					}

					break;

				case epfARTICLE:

					if( NULL != pclProduct1 && NULL != pclProduct2 )
					{
						if( false == TASApp.IsTAArtNumberHidden() )
						{
							// Remark: 'true' to exclude any connection reference in the article number.
							str1 = pclProduct1->GetArtNum( true );
							str2 = pclProduct2->GetArtNum( true );
						}
						else
						{
							// Remark: 'true' to exclude any connection reference in the article number.
							str1 = LocArtNumTab.GetLocalArticleNumber( pclProduct1->GetArtNum( true ) );
							str2 = LocArtNumTab.GetLocalArticleNumber( pclProduct2->GetArtNum( true ) );
						}
					}

					break;

				default:
					break;
			}

			int iCompare = str1.CompareNoCase( str2 );

			if( 0 == iCompare )
			{
				continue;
			}
			else 
			{
				if( iCompare < 0 ) 
				{
					return true;
				}
				else 
				{
					return false;	
				}
			}
		}
	}

	if( false == bSortExist )
	{
		//Sorting is based on circuit position
		// Sorting key level depth 1000, circuits depth 1000
		int iKey1 = -1000000 + ( pHM1->GetLevel() *1000 ) + pHM1->GetPos();
		int iKey2 = -1000000 + ( pHM2->GetLevel() *1000 ) + pHM2->GetPos();
		return ( iKey1 < iKey2 );
	}

	return false;
}

void CSelProdPageBase::_InvalidateSSheetRect()
{
	if( 0 == m_vecSelectionBorderInfo.size() )
	{
		return;
	}

	for( auto &iterSelection : m_vecSelectionBorderInfo )
	{
		CSSheet *pclSSheet = iterSelection.m_pclSheetDescription->GetSSheetPointer();

		long lTopRow = pclSSheet->GetTopRow();
		long lRowsFrozen = pclSSheet->GetRowsFreeze();
		CRect rectTopRow = pclSSheet->GetCellCoordInPixel( iterSelection.m_ptTopLeftColRow.x, lTopRow );
		CRect rectLastRowFrozen = pclSSheet->GetCellCoordInPixel( iterSelection.m_ptTopLeftColRow.x, lRowsFrozen );
		long lYSheetOffset = rectLastRowFrozen.bottom - rectTopRow.top;

		CRect rectRange = CRect( iterSelection.m_ptTopLeftColRow, iterSelection.m_ptBottomRightColRow );
		CRect rectInvalidate = pclSSheet->GetSelectionCoordInPixels( rectRange );
		rectInvalidate.OffsetRect( 0, lYSheetOffset );
		pclSSheet->InvalidateRect( &rectInvalidate, TRUE );
		pclSSheet->UpdateWindow();

		for( auto &iterLinked : iterSelection.m_vecLinkedSelectionBorderInfo )
		{
			pclSSheet = iterLinked.m_pclSheetDescription->GetSSheetPointer();
			rectRange = CRect( iterLinked.m_ptTopLeftColRow, iterLinked.m_ptBottomRightColRow );
			rectInvalidate = pclSSheet->GetSelectionCoordInPixels( rectRange );
			rectInvalidate.OffsetRect( 0, lYSheetOffset );
			pclSSheet->InvalidateRect( &rectInvalidate, TRUE );
			pclSSheet->UpdateWindow();
		}
	}
}

void CSelProdPageBase::_DrawSelectionBorders()
{
	if( 0 == m_vecSelectionBorderInfo.size() )
	{
		return;
	}

	CDC *pDC = GetDC();

	CPen clPen( PS_SOLID, SELPRODPAGE_SELECTIONBORDERWIDTH, SELPRODPAGE_SELECTIONBORDERCOLOR );
	CPen *pOldPen = pDC->SelectObject( &clPen );
	
	long lXWindowOffset = 0;
	long lYWindowOffset = 0;

	if( false == UseOnlyOneSpread() )
	{
		// In the case of we use several spread in on sheet (Pressurisation for example), the scrolling is managed
		// by the base class and not by the TSpread.
		int iVertPos = 0;
		int iHorzPos = 0;
		GetScrollingPos( iVertPos, iHorzPos );
		lXWindowOffset = -iHorzPos;
		lYWindowOffset = -iVertPos;
	}

	if( Selection_NoKey == m_eSelectionKey )
	{
		_DrawOneSelectionBorder( m_vecSelectionBorderInfo.front(), pDC, lXWindowOffset, lYWindowOffset );
	}
	else if( Selection_Ctrl == m_eSelectionKey )
	{
		for( auto &iter : m_vecSelectionBorderInfo )
		{
			_DrawOneSelectionBorder( iter, pDC, lXWindowOffset, lYWindowOffset );
		}
	}
	else if( Selection_Shift == m_eSelectionKey )
	{
		SelectionBorderInfo rSelectionBorderInfo;

		if( true == UseOnlyOneSpread() )
		{
			rSelectionBorderInfo.m_ptTopLeftColRow = m_vecSelectionBorderInfo.front().m_ptTopLeftColRow;
			rSelectionBorderInfo.m_ptTopRightColRow = m_vecSelectionBorderInfo.front().m_ptTopRightColRow;
			rSelectionBorderInfo.m_ptBottomLeftColRow = m_vecSelectionBorderInfo.back().m_ptBottomLeftColRow;
			rSelectionBorderInfo.m_ptBottomRightColRow = m_vecSelectionBorderInfo.back().m_ptBottomRightColRow;
			rSelectionBorderInfo.m_pclSheetDescription = m_vecSelectionBorderInfo.front().m_pclSheetDescription;
			_DrawOneSelectionBorder( rSelectionBorderInfo, pDC, lXWindowOffset, lYWindowOffset );
		}
		else
		{
			rSelectionBorderInfo = m_vecSelectionBorderInfo.at( 0 );

			for( int i = 1; i < (int)m_vecSelectionBorderInfo.size(); i++ )
			{
				if( rSelectionBorderInfo.m_pclSheetDescription != m_vecSelectionBorderInfo.at( i ).m_pclSheetDescription )
				{
					_DrawOneSelectionBorder( rSelectionBorderInfo, pDC, lXWindowOffset, lYWindowOffset );
					rSelectionBorderInfo = m_vecSelectionBorderInfo.at( i );
				}
				else
				{
					rSelectionBorderInfo.m_ptBottomLeftColRow = m_vecSelectionBorderInfo.at( i ).m_ptBottomLeftColRow;
					rSelectionBorderInfo.m_ptBottomRightColRow = m_vecSelectionBorderInfo.at( i ).m_ptBottomRightColRow;
				}
			}

			_DrawOneSelectionBorder( rSelectionBorderInfo, pDC, lXWindowOffset, lYWindowOffset );
		}
	}

	pDC->SelectObject( pOldPen );
	ReleaseDC( pDC );
}

void CSelProdPageBase::_DrawOneSelectionBorder( SelectionBorderInfo rSelectionBorderInfo, CDC *pDC, long lXWindowOffset, long lYWindowOffset )
{
	if( true == UseOnlyOneSpread() )
	{
		// Draw the top border.
		_DrawTopBorder( rSelectionBorderInfo, pDC, lXWindowOffset, lYWindowOffset );

		// Now draw all vertical borders.
		_DrawVerticalBorders( rSelectionBorderInfo, pDC, lXWindowOffset, lYWindowOffset );

		// Last row - bottom line.
		_DrawBottomBorder( rSelectionBorderInfo, pDC, lXWindowOffset, lYWindowOffset );
	}
	else
	{
		// Draw the top border.
		_DrawTopBorder( rSelectionBorderInfo, pDC, lXWindowOffset, lYWindowOffset );

		// Now draw all vertical borders.
		_DrawVerticalBorders( rSelectionBorderInfo, pDC, lXWindowOffset, lYWindowOffset );

		SelectionBorderInfo *prSelectionBorderInfo = &rSelectionBorderInfo;

		if( rSelectionBorderInfo.m_vecLinkedSelectionBorderInfo.size() > 0 )
		{
			for( auto &iter : rSelectionBorderInfo.m_vecLinkedSelectionBorderInfo )
			{
				_DrawVerticalBorders( iter, pDC, lXWindowOffset, lYWindowOffset );
			}

			prSelectionBorderInfo = &rSelectionBorderInfo.m_vecLinkedSelectionBorderInfo.back();
		}

		// Last row - bottom line.
		_DrawBottomBorder( *prSelectionBorderInfo, pDC, lXWindowOffset, lYWindowOffset );
	}
}

void CSelProdPageBase::_DrawTopBorder( SelectionBorderInfo rSelectionBorderInfo, CDC *pDC, long lXWindowOffset, long lYWindowOffset )
{
	CSSheet *pclSSheet = rSelectionBorderInfo.m_pclSheetDescription->GetSSheetPointer();

	long lTopRow = pclSSheet->GetTopRow();
	long lRowsFrozen = pclSSheet->GetRowsFreeze();
	CRect rectTopRow = pclSSheet->GetCellCoordInPixel( rSelectionBorderInfo.m_ptTopLeftColRow.x, lTopRow );
	CRect rectLastRowFrozen = pclSSheet->GetCellCoordInPixel( rSelectionBorderInfo.m_ptTopLeftColRow.x, lRowsFrozen );
	long lYSheetOffset = rectLastRowFrozen.bottom - rectTopRow.top;
	lYSheetOffset += rSelectionBorderInfo.m_pclSheetDescription->GetSSheetOffset().y;
	long lYGlobalOffset = lYSheetOffset + lYWindowOffset;

	// First row - top line.
	if( TRUE == pclSSheet->GetVisible( rSelectionBorderInfo.m_ptTopLeftColRow.x, rSelectionBorderInfo.m_ptTopLeftColRow.y, SS_VISIBLE_PARTIAL ) )
	{
		CRect rectLeft = pclSSheet->GetCellCoordInPixel( rSelectionBorderInfo.m_ptTopLeftColRow.x, rSelectionBorderInfo.m_ptTopLeftColRow.y );

		// To avoid to write on the top frozen rows that are not refreshed when TSpread scrolls.
		int iYFirstRow = 0;
		
		if( lRowsFrozen > 0 && ( rectLeft.top + lYSheetOffset ) == rectLastRowFrozen.bottom )
		{
			iYFirstRow = 1;
		}

		// Move to the top left position.
		pDC->MoveTo( rectLeft.left - 1 + lXWindowOffset, rectLeft.top + lYGlobalOffset + iYFirstRow );

		CRect rectRight = pclSSheet->GetCellCoordInPixel( rSelectionBorderInfo.m_ptTopRightColRow.x, rSelectionBorderInfo.m_ptTopRightColRow.y );

		// Go to the top right position.
		pDC->LineTo( rectRight.right - 2 + lXWindowOffset, rectRight.top + lYGlobalOffset + iYFirstRow );
	}
}

void CSelProdPageBase::_DrawVerticalBorders( SelectionBorderInfo rSelectionBorderInfo, CDC *pDC, long lXWindowOffset, long lYWindowOffset )
{
	CSSheet *pclSSheet = rSelectionBorderInfo.m_pclSheetDescription->GetSSheetPointer();

	long lTopRow = pclSSheet->GetTopRow();
	long lRowsFrozen = pclSSheet->GetRowsFreeze();
	CRect rectTopRow = pclSSheet->GetCellCoordInPixel( rSelectionBorderInfo.m_ptTopLeftColRow.x, lTopRow );
	CRect rectLastRowFrozen = pclSSheet->GetCellCoordInPixel( rSelectionBorderInfo.m_ptTopLeftColRow.x, lRowsFrozen );
	long lYSheetOffset = rectLastRowFrozen.bottom - rectTopRow.top;
	lYSheetOffset += rSelectionBorderInfo.m_pclSheetDescription->GetSSheetOffset().y;
	long lYGlobalOffset = lYSheetOffset + lYWindowOffset;

	// Now draw all vertical borders.
	for( long lLoopRow = rSelectionBorderInfo.m_ptTopLeftColRow.y; lLoopRow <= rSelectionBorderInfo.m_ptBottomLeftColRow.y; lLoopRow++ )
	{
		int iYFirstRow = 0;

		if( lRowsFrozen > 0 && lLoopRow == rSelectionBorderInfo.m_ptTopLeftColRow.y && rSelectionBorderInfo.m_ptTopLeftColRow.y == lRowsFrozen + 1 )
		{
			// To avoid to write on the top frozen rows that are not refreshed when TSpread scrolls.
			iYFirstRow = 1;
		}

		// Left part.
		if( TRUE == pclSSheet->GetVisible( rSelectionBorderInfo.m_ptTopLeftColRow.x, lLoopRow, SS_VISIBLE_PARTIAL ) )
		{
			CRect rectLeft = pclSSheet->GetCellCoordInPixel( rSelectionBorderInfo.m_ptTopLeftColRow.x, lLoopRow );

			// Move to the left position.
			pDC->MoveTo( rectLeft.left - 1 + lXWindowOffset, rectLeft.top + lYGlobalOffset + iYFirstRow );

			// Go down.
			pDC->LineTo( rectLeft.left - 1 + lXWindowOffset, rectLeft.bottom + lYGlobalOffset );
		}

		// Right part.
		if( TRUE == pclSSheet->GetVisible( rSelectionBorderInfo.m_ptTopRightColRow.x, lLoopRow, SS_VISIBLE_PARTIAL ) )
		{
			CRect rectRight = pclSSheet->GetCellCoordInPixel( rSelectionBorderInfo.m_ptTopRightColRow.x, lLoopRow );

			// Move to the right position.
			pDC->MoveTo( rectRight.right - 2 + lXWindowOffset, rectRight.top + lYGlobalOffset + iYFirstRow );

			// Go down.
			pDC->LineTo( rectRight.right - 2 + lXWindowOffset, rectRight.bottom + lYGlobalOffset );
		}
	}
}

void CSelProdPageBase::_DrawBottomBorder( SelectionBorderInfo rSelectionBorderInfo, CDC *pDC, long lXWindowOffset, long lYWindowOffset )
{
	CSSheet *pclSSheet = rSelectionBorderInfo.m_pclSheetDescription->GetSSheetPointer();

	long lTopRow = pclSSheet->GetTopRow();
	long lRowsFrozen = pclSSheet->GetRowsFreeze();
	CRect rectTopRow = pclSSheet->GetCellCoordInPixel( rSelectionBorderInfo.m_ptTopLeftColRow.x, lTopRow );
	CRect rectLastRowFrozen = pclSSheet->GetCellCoordInPixel( rSelectionBorderInfo.m_ptTopLeftColRow.x, lRowsFrozen );
	long lYSheetOffset = rectLastRowFrozen.bottom - rectTopRow.top;
	lYSheetOffset += rSelectionBorderInfo.m_pclSheetDescription->GetSSheetOffset().y;
	long lYGlobalOffset = lYSheetOffset + lYWindowOffset;

	// Last row - bottom line.
	if( TRUE == pclSSheet->GetVisible( rSelectionBorderInfo.m_ptBottomLeftColRow.x, rSelectionBorderInfo.m_ptBottomLeftColRow.y, SS_VISIBLE_PARTIAL ) )
	{
		CRect rectLeft = pclSSheet->GetCellCoordInPixel( rSelectionBorderInfo.m_ptBottomLeftColRow.x, rSelectionBorderInfo.m_ptBottomLeftColRow.y );

		// Move to the bottom left position.
		pDC->MoveTo( rectLeft.left - 1 + lXWindowOffset, rectLeft.bottom + lYGlobalOffset );

		CRect rectRight = pclSSheet->GetCellCoordInPixel( rSelectionBorderInfo.m_ptBottomRightColRow.x, rSelectionBorderInfo.m_ptBottomRightColRow.y );

		// Go to the bottom right position.
		pDC->LineTo( rectRight.right - 2 + lXWindowOffset, rectRight.bottom + lYGlobalOffset );
	}
}
