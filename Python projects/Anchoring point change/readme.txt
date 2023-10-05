This script allowed to transform all the old:

	NEWPRODPIC TAMULTI20_CV(GIF_TAMULTI20_CV, CProdPic::Pic, epHorizontal, _Pt(20, 0), _Pt(20, 97), _Pt(62, 6), 45, _Pt(62, 91), 135);

into:

	NEWPRODPIC TAMULTI20_CV(GIF_TAMULTI20_CV, CProdPic::Pic, epHorizontal, { APDf( _I, _Pt( 20, 0 ) ), APDf( _O, _Pt( 20, 97 ) ), APDf( _CP, _Pt( 62, 6 ), 45.0 ), APDf( _CS, _Pt( 62, 91 ), 135.0 ) } );


