#include "StdAfx.h"


#include "HydronicPic.h"

CProdPic::CProdPic()
{
	_Init();

	m_ePicType = Pic;
	m_iResID = 0;
}

CProdPic::CProdPic(	UINT iResID, eProdPicType etype, ePosition ePos, std::vector<AnchoringPointDef> vecAnchPointList )
{
	_Init();

	m_iResID = iResID;
	m_ePicType = etype;
	m_ePosition = ePos;
	m_vecAnchoringPointList = vecAnchPointList;
}

const CProdPic::AnchoringPointDef *CProdPic::GetOneAnchoringPoint( eConnectionPoints ConnPt )
{
	AnchoringPointDef *pAnchoringPointDef = NULL;

	for( auto &iter : m_vecAnchoringPointList )
	{
		if( iter.m_eType == ConnPt )
		{
			pAnchoringPointDef = &iter;
			break;
		}
	}

	return pAnchoringPointDef;
}

void CProdPic::_Init()
{
	m_ePosition = epHorizontal;
}

///////////////////////////////////////////////////////////////////
// CDynCircSch class

CDynCircSch::CDynCircSch( UINT uiResourceID )
{
	m_uiBckgndResID = uiResourceID;
}

void CDynCircSch::Reset( UINT iResID )
{
	m_vAnchorPt.clear();
	m_uiBckgndResID = iResID;
}

CDynCircSch &CDynCircSch::AddAnchorPt( CAnchorPt cAnchorPt )
{
	m_vAnchorPt.push_back( cAnchorPt );
	return *this;
}

CAnchorPt *CDynCircSch::GetAnchoringPt( unsigned uiIndex )
{
	if( uiIndex >= GetNbrAnchoringPt() )
	{
		return false;
	}

	return &( m_vAnchorPt[uiIndex] );
}

UINT CDynCircSch::GetNbrAnchoringPt()
{
	return m_vAnchorPt.size();
}
