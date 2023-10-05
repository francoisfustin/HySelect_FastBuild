========================================================================
    DYNAMIC LINK LIBRARY : TAProdPics Project Overview
========================================================================

Create a GetpImgTAProd in Phoenix.h that do exaclty the same thing as the one("GetImgTAProd ") in this Dll

{
	pGetImgTAProd pdllfunction;
	pdllfunction =(pGetImgTAProd)GetProcAddress(pPhoenixApp->m_hmProdPics,"GetImgTAProd");
	CImgTAProd *pPointer = pdllfunction(_T("STAD25Image"),LocSymb);
	return pPointer
}

You will have a pointer to an objet in a multimap.

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
All ressources will be implemented in the
multimap by this way:
	
	ADDIMAGE(PX_DA50_200A,CProdPic(GIF_DA50_200A_PIC,Pic,...)
	ADDIMAGE(PX_DA50_200A,CProdPic(GIF_DA50_PIC,CProdPic::PicTASymb, epVertical,...)
		...
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

