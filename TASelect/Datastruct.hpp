
// File used to declare template function used into database

///////////////////////////////////////////////////////////////////////////////////
//
//								CDatastruct
// Base class for supporting CTADatastruct, CTADatastructX and CTAMetaData classes.
// Creation 09/10/27	Alen
///////////////////////////////////////////////////////////////////////////////////

class CDatastruct : public CDataBase
{
public:
	CDatastruct( CDataBase::ChildID eChildID ) : CDataBase( eChildID ) {}
	virtual ~CDatastruct() {}

	enum FindMode
	{
		OnlyModules = 0,
		OnlyValves = 1,
		Both = 2
	};

	////////////////////////////////////////////////////////////////////////////////
	// Find a Module or a valve with name matching str 
	template<typename T>
	T *FindHydroMod(const wchar_t * str, CTable *pTab, T *pHMToSkip, CDatastruct::FindMode eFindMode )
	{
		if( NULL == pTab )
			return NULL;
		
		for( IDPTR IDPtr = pTab->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext( IDPtr.MP ) )
		{
			T* p = dynamic_cast<T*>( IDPtr.MP );
			if( NULL != p && p != pHMToSkip )
			{
				switch( eFindMode )
				{
					case OnlyModules:
						if( true == p->IsaModule() )
						{
							if( 0 == _tcscmp( p->GetHMName()->c_str(), str ) )
								return ( (T *)IDPtr.MP );
						}
						break;
					
					case OnlyValves:
						if( false == ( (T *)IDPtr.MP)->IsaModule() )
						{
							if( 0 == _tcscmp( p->GetHMName()->c_str(), str ) )
								return p;
						}
						break;

					default:
						if( 0 == _tcscmp( p->GetHMName()->c_str(), str ) )
							return p;
				}
			}
			
			if( true == p->IsaModule() )
			{
				T * pHM = FindHydroMod( str, (CTable *)IDPtr.MP, pHMToSkip, eFindMode );
				if( NULL != pHM )
					return pHM;
			}
		}			
		return NULL;
	};
};			//End of class
