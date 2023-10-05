#ifndef CDELETEDPRODUCTHELPER_H__INCLUDED_
#define CDELETEDPRODUCTHELPER_H__INCLUDED_

#include "HydroMod.h"

class CDeletedProductHelper
{
public:
	CDeletedProductHelper();

	virtual ~CDeletedProductHelper();

	enum FromWhere
	{
		FromIndividualSelection,
		FromDirectSelection,
		FromHydraulicNetwork
	};

	bool VerifyAccessoryList( CAccessoryList *pclAccessoryList, FromWhere eWhere, int iQuantity );
	bool VerifyAccessory( IDPTR AccIDPtr, FromWhere eWhere, int iQuantity );
	void VerifyIndividualSelection( bool &bDisplay );
	void VerifyHydroMod( IDPTR IDPtr, bool &bDisplay );
	// HYS-1291 : Get the map with hydromod deleted product
	multimap< CDS_HydroMod *, CDS_HydroMod::eHMObj > *GetHMMapDeleted() { return &m_mapHMDeleted; }

// Protected members.
protected:
	// HYS-1868: Manage the case where actuator is no longer compatible or is not updated.
	// Determine the inputs "bIsCompatible" and "NameToCheck" the product name that must be compatible.
	bool _PrepareListCtrl( CDB_Thing *pThing, LPCTSTR lpName, LPCTSTR lpNameID, FromWhere eFromWhere, int iQuantity, 
						   bool bIsCompatible = true, LPCTSTR NameToCheck = '\0');
	void _VerifyIndividualSelection6WayValveHelper( IDPTR ProductIDPtr, IDPTR ActuatorIDPtr, CAccessoryList *pclAccessoryList, 
		CAccessoryList *pclActuatorAccessoryList, FromWhere eWhere, int iGlobalQuantity, bool &bDisplay );
	void _VerifyIndividualSelectionBvHelper( IDPTR ProductIDPtr, CAccessoryList *pclAccessoryList, FromWhere eWhere, int iGlobalQuantity, bool &bDisplay );

// Protected variables.
protected:

	enum eColumn
	{
		ecProblem,
		ecWhere,
		ecValve,
		ecValveID,
		ecNumber
	};

	CTADatastruct *m_pTADS;
	CTADatabase *m_pTADB;

	typedef struct _Info
	{
		CString m_strValve;
		int m_iNumber;
		CString m_strBottomMessage;
	}Info;

	typedef std::map<CString, Info> mapIDInfo;
	typedef mapIDInfo::iterator mapIDInfoIter;
	typedef std::map<FromWhere, mapIDInfo > mapWhereInfo;
	typedef mapWhereInfo::iterator mapWhereInfoIter;

	mapWhereInfo m_mapUndef;
	mapWhereInfo m_mapDeleted;
	mapWhereInfo m_mapNotAvailable;
	// HYS-1868
	mapWhereInfo m_mapNotCompatible;
	// HYS-1291 : To store hydromod deleted product
	multimap< CDS_HydroMod *, CDS_HydroMod::eHMObj > m_mapHMDeleted;
};
#endif