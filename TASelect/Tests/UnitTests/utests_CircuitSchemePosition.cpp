#include "stdafx.h"

#include "utests_base.h"
#include "utests_CircuitSchemePosition.h"
#include "DataBObj.h"
#include "TASelect.h"
#include "HydroMod.h"
#include "SSheetPanelCirc2.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( utests_CircuitSchemePosition, CPPUNIT_TEST_CATEGORYNAME_DATABASE );

void utests_CircuitSchemePosition::TestCircuitSchemePosition()
{
	CSSheetPanelCirc2 *pSPC2 = new CSSheetPanelCirc2;
	IDPTR idptr;

	if( false == pSPC2->TestHydronicSchemesBoxPosition( idptr ) )
	{
		CStringA strID = CW2A( idptr.ID );

		stringstream str;
		str << "Fail on " << strID << "\n";
		str << "\tBox name: " << pSPC2->TestHydronicSchemesBoxPositionGetBoxName() << "\n";

		// Status.
		int iStatusCode = pSPC2->TestHydronicSchemesBoxPositionGetStatus();

		switch( iStatusCode )
		{
			case CSSheetPanelCirc2::Status_XOutOfRange:
				str << "\tError: X value (" << pSPC2->TestHydronicSchemesBoxPositionGetXValue() << ") is out of range.\n";
				str << "\tMax colum: " << pSPC2->TestHydronicSchemesBoxPositionGetTotalCols() << "\n";
				break;

			case CSSheetPanelCirc2::Status_YOutOfRange:
				str << "\tError: Y value (" << pSPC2->TestHydronicSchemesBoxPositionGetYValue() << ") is out of range.\n";
				str << "\tMax row: " << pSPC2->TestHydronicSchemesBoxPositionGetTotalRows() << "\n";
				break;

			case CSSheetPanelCirc2::Status_PositionBusy:
				str << "\tError: Position already busy.\n";
				str << "\tX value: " << pSPC2->TestHydronicSchemesBoxPositionGetXValue() << "\n";
				str << "\tY value: " << pSPC2->TestHydronicSchemesBoxPositionGetYValue() << "\n";
				break;
		}

		CPPUNIT_FAIL( str.str() );
	}
	
	delete pSPC2;
}
