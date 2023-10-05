/////////////////////////////////////////////////////////////////////////////
// CBlobProperty

class CBlobProperty : public IPersistStream
{
public:
	CBlobProperty(HGLOBAL pBlob = NULL);
	HGLOBAL GetBlob();
    void    SetPropSetFlag(BOOL bFlag);

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);

	STDMETHOD(GetClassID)(LPCLSID);
	STDMETHOD(IsDirty)();
	STDMETHOD(Load)(LPSTREAM);
	STDMETHOD(Save)(LPSTREAM, BOOL);
	STDMETHOD(GetSizeMax)(ULARGE_INTEGER*);

protected:
	BOOL m_bPropSet;
	long m_dwRef;
	HGLOBAL m_hBlob;
};

/////////////////////////////////////////////////////////////////////////////
// CfpPropbagPropExchange

class CfpPropbagPropExchange : public CPropExchange
{
public:
	CfpPropbagPropExchange(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog,
		BOOL bLoading, BOOL bSaveAllProperties=FALSE);
	~CfpPropbagPropExchange();

// Operations
	virtual BOOL ExchangeProp(LPCTSTR pszPropName, VARTYPE vtProp,
				void* pvProp, const void* pvDefault = NULL);
	virtual BOOL ExchangeBlobProp(LPCTSTR pszPropName, HGLOBAL* phBlob,
				HGLOBAL hBlobDefault = NULL);
	virtual BOOL ExchangeFontProp(LPCTSTR pszPropName, CFontHolder& font,
				const FONTDESC* pFontDesc, LPFONTDISP pFontDispAmbient);
	virtual BOOL ExchangePersistentProp(LPCTSTR pszPropName,
				LPUNKNOWN* ppUnk, REFIID iid, LPUNKNOWN pUnkDefault);

// Implementation
	LPPROPERTYBAG m_pPropBag;
	LPERRORLOG m_pErrorLog;
	BOOL m_bSaveAllProperties;
};
