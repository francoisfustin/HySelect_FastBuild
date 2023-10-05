#pragma once

// File used to declare template function used into database
#include <map>

template<typename T, typename U>
class CTemplateMap
{
public:
	CTemplateMap() {}
	virtual ~CTemplateMap() {}

	void Clear()
	{ 
		m_mapList.clear();
	}

	int size()
	{
		return m_mapList.size();
	}
	
	void Add( T First, U Second )
	{ 
		m_mapList[First] = Second;
	}
	
	bool IsExist( T First )
	{ 
		return ( ( m_mapList.count( First ) > 0 ) ? true : false );
	}

	bool Get( T First, U &Second )
	{ 
		bool fReturn = false;
		if( true == IsExist( First ) ) 
		{
			Second = m_mapList[First];
			fReturn = true;
		}
		return fReturn;
	}

	// Return first Key that match second element, 
	bool GetSecond(T &First, U Second)
	{
		T FirstElm;
		U SecondElm;
		bool bCont = GetFirst(FirstElm, SecondElm);
		while (true == bCont)
		{
			if (Second == SecondElm)
			{
				First = FirstElm;
				return true;
			}
			bCont = GetNext(FirstElm, SecondElm);
		}
		return false;
	}


	bool GetFirst( T &First, U&Second )
	{
		bool fReturn = false;
		if( m_mapList.empty() == false )
		{
			m_mapIter = m_mapList.begin();
			if( m_mapIter != m_mapList.end() )
			{
				First = m_mapIter->first;
				Second = m_mapIter->second;
				fReturn = true;
				++m_mapIter;
			}
		}
		return fReturn;
	}

	bool GetNext( T &First, U&Second )
	{
		bool fReturn = false;
		if( m_mapIter != m_mapList.end() )
		{
			First = m_mapIter->first;
			Second = m_mapIter->second;
			fReturn = true;
			++m_mapIter;
		}
		return fReturn;
	}

private:
	std::map<T, U>						m_mapList;
	typename std::map<T, U>::iterator	m_mapIter;
};
