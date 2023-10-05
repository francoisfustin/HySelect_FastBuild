#include "stdafx.h"
#include "MainFrm.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "Global.h"
#include "Utilities.h"
#include "RCImageManager.h"

CRCImageManager::ImageDefinition CRCImageManager::m_arImageList[] =
{
	{
		ImageListName::ILN_6WayValveTabCltr,
		IDB_IMGLIST_6WAYVALVETABCTRL,
		ImageSize::IS_Size16,
		{
			16, 16, ILC_COLOR32, ImgList6WayValveTabCtrl::IL6WV_Last, 1, CLR_NONE
		}
	},
	{
		ImageListName::ILN_Button,
		IDB_IMGLIST_BUTTON,
		ImageSize::IS_Size16,
		{
			16, 16, ILC_COLOR32, ImgListButton::ILB_Last, 1, CLR_NONE
		}
	},
	{
		ImageListName::ILN_CheckBox,
		IDB_IMGLST_CHECKBOX1616,
		ImageSize::IS_Size16,
		{
			16, 16, ILC_COLOR32, ImgListCheckBox::ILCB_Last, 1, CLR_NONE
		}
	},
	{
		ImageListName::ILN_CustomPipe,
		IDB_IMGLIST_CUSTPIPE,
		ImageSize::IS_Size16, 
		{
			16, 16, ILC_MASK, ImgListCustPipe::ILCP_Last, 1, _BLACK
		}
	},
	{
		ImageListName::ILN_GroupBox,
		IDB_IMGLIST_GROUPBOX,
		ImageSize::IS_Size16,
		{
			16, 16, ILC_COLOR32, ImgListGroupBox::ILGB_Last, 1, CLR_NONE
		}
	},
	{
		ImageListName::ILN_GroupBoxGrayed,
		IDB_IMGLIST_GROUPBOX_GRAYED,
		ImageSize::IS_Size16,
		{
			16, 16, ILC_COLOR32, ImgListGroupBoxGrayed::ILGBG_Last, 1, CLR_NONE
		}
	},
	{
		ImageListName::ILN_HMTree,
		IDB_IMGLST_HMTREE,
		ImageSize::IS_Size16,
		{
			16, 16, ILC_COLOR32, ImgListHMTree::ILHM_Last, 1, CLR_NONE
		}
	},
	{
		ImageListName::ILN_InfoTree,
		IDB_LVINFOTREE,
		ImageSize::IS_Size16,
		{
			16, 16, ILC_COLOR32, ImgListInfoTree::ILIT_Last, 1, CLR_NONE
		}
	},
	{
		ImageListName::ILN_PMWQPrefs,
		IDB_IMGLIST_PMWQPREFS,
		ImageSize::IS_Size16, 
		{
			16, 16, ILC_COLOR32, ImgListPMWQPrefs::ILPMWQP_Last, 1, CLR_NONE
		}
	},
	{
		ImageListName::ILN_Print,
		IDB_IMGLIST_PRINT,
		ImageSize::IS_Size16,
		{
			16, 16, ILC_MASK, ImgListPrint::ILP_Last, 1, _BLACK
		}
	},
	{
		ImageListName::ILN_ProjectTree,
		IDB_LVPROJTREE,
		ImageSize::IS_Size16,
		{
			16, 16, ILC_MASK, ImgListProjectTree::ILPT_Last, 1, _BLACK
		}
	},
	{
		ImageListName::ILN_StripHome,
		IDB_STRIP_HOME_16,
		ImageSize::IS_Size16, 
		{
			16, 16, ILC_COLOR32, ImgListStripHome::ILSH_Last, 1, CLR_NONE
		}
	},
	{
		ImageListName::ILN_StripProject,
		IDB_STRIP_PROJECT_16,
		ImageSize::IS_Size16, 
		{
			16, 16, ILC_COLOR32, ImgListStripProject::ILSP_Last, 1, CLR_NONE
		}
	},
	{
		ImageListName::ILN_StripSelectionCategory,
		IDB_STRIP_SINGLESELCATEG,
		ImageSize::IS_Size48, 
		{
			48, 48, ILC_COLOR32, ImgListStripSelCategory::ILSSC_Last, 1, CLR_NONE
		}
	},
	{ ImageListName::ILN_Undefined, -1, ImageSize::IS_Undefined, { -1 } }
};

CRCImageManager::CRCImageManager()
{
	_Init();
}

CRCImageManager::~CRCImageManager()
{
	for( mapEnumImageListIter iter = m_mapImageContainer.begin(); iter != m_mapImageContainer.end(); iter++ )
	{
		if( NULL != iter->second )
		{
			delete iter->second;
		}
	}

	m_mapImageContainer.clear();
}

CImageList *CRCImageManager::GetImageList( ImageListName eImageListName )
{
	CImageList *pclImageList = NULL;

	if( m_mapImageContainer.count( eImageListName ) > 0 )
	{
		pclImageList = m_mapImageContainer[eImageListName];
	}

	return pclImageList;
}

CImageList* CRCImageManager::GetImageListCopy( ImageListName eImageListName, bool bShiftForStatus )
{
	if( 0 == m_mapImageContainer.count( eImageListName ) || 0 == m_mapImageIndex.count( eImageListName ) )
	{
		return NULL;
	}

	CImageList *pclImageList = _CreateImageList( m_mapImageIndex[eImageListName], bShiftForStatus );
	return pclImageList;
}

void CRCImageManager::_Init( void )
{
	int iLoopImageList = 0;

	while( m_arImageList[iLoopImageList].m_eImageListName != -1 )
	{
		bool bInsert = false;
		CImageList *pclImageList = _CreateImageList( iLoopImageList );

		if( NULL != pclImageList )
		{
			m_mapImageContainer[m_arImageList[iLoopImageList].m_eImageListName] = pclImageList;
			m_mapImageIndex[m_arImageList[iLoopImageList].m_eImageListName] = iLoopImageList;
		}

		iLoopImageList++;
	}
}

CImageList *CRCImageManager::_CreateImageList( short nIndex, bool bShiftForStatus )
{
	CImageList *pclImageList = new CImageList();

	if( NULL == pclImageList )
	{
		return NULL;
	}

	bool bOK = false;

	if( TRUE == pclImageList->Create(	m_arImageList[nIndex].m_rImageInfos.m_cx,
										m_arImageList[nIndex].m_rImageInfos.m_cy,
										m_arImageList[nIndex].m_rImageInfos.m_nFlags,
										( false == bShiftForStatus ) ? m_arImageList[nIndex].m_rImageInfos.m_nInitial : m_arImageList[nIndex].m_rImageInfos.m_nInitial + 1,
										m_arImageList[nIndex].m_rImageInfos.m_nGrow ) )
	{
		if( true == bShiftForStatus )
		{
			// Add a fictive bitmap at the first position.
			CBitmap clBitmap;

			if( TRUE == clBitmap.CreateBitmap(	m_arImageList[nIndex].m_rImageInfos.m_cx, 
												m_arImageList[nIndex].m_rImageInfos.m_cy, 
												1, 
												m_arImageList[nIndex].m_rImageInfos.m_nFlags,
												NULL ) )
			{
				if( -1 != pclImageList->Add( &clBitmap, m_arImageList[nIndex].m_rImageInfos.m_crMask ) )
				{
					bOK = true;
				}
			}
		}
		
		if( false == bShiftForStatus || true == bOK )
		{
			bOK = false;
			CBitmap clBitmap;

			if( TRUE == clBitmap.LoadBitmap( m_arImageList[nIndex].m_iBitmapID ) )
			{
				if( -1 != pclImageList->Add( &clBitmap, m_arImageList[nIndex].m_rImageInfos.m_crMask ) )
				{
					bOK = true;
				}
			}
		}
	}

	if( false == bOK && NULL != pclImageList )
	{
		delete pclImageList;
		pclImageList = NULL;
	}

	return pclImageList;
}