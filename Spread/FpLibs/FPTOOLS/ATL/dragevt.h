void Fire_OLECompleteDrag(
	long* Effect)
{
	VARIANTARG* pvars = new VARIANTARG;
	
	VariantInit(pvars);
	T* pT = (T*)this;
	pT->Lock();
	IUnknown** pp = m_vec.begin();
	IUnknown** ppEnd = m_vec.end();
	while (pp < ppEnd)
	{
		if (*pp != NULL)
		{
			pvars->vt = VT_BYREF|VT_I4;
			pvars->plVal= Effect;
			DISPPARAMS disp = { pvars, NULL, 1, 0 };
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
			pDispatch->Invoke(DISPID_OLECOMPLETEDRAG, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
		}
		pp++;
	}
	pT->Unlock();
	delete pvars;
}

void Fire_OLEDragDrop(
	CfpDataObject** DataObject,
	long* Effect,
	short* Button,
	short* Shift,
	float* X,
	float* Y)
{
	VARIANTARG* pvars = new VARIANTARG[6];
	for (int i = 0; i < 6; i++)
		VariantInit(&pvars[i]);
	T* pT = (T*)this;
	pT->Lock();
	IUnknown** pp = m_vec.begin();
	IUnknown** ppEnd = m_vec.end();
	while (pp < ppEnd)
	{
		if (*pp != NULL)
		{
			pvars[5].vt = VT_BYREF|VT_DISPATCH;
			pvars[5].ppdispVal= (LPDISPATCH*)DataObject;
			pvars[4].vt = VT_BYREF|VT_I4;
			pvars[4].plVal= Effect;
			pvars[3].vt = VT_BYREF|VT_I2;
			pvars[3].piVal= Button;
			pvars[2].vt = VT_BYREF|VT_I2;
			pvars[2].piVal= Shift;
			pvars[1].vt = VT_BYREF|VT_R4;
			pvars[1].pfltVal= X;
			pvars[0].vt = VT_BYREF|VT_R4;
			pvars[0].pfltVal= Y;
			DISPPARAMS disp = { pvars, NULL, 6, 0 };
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
			pDispatch->Invoke(DISPID_OLEDRAGDROP, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
		}
		pp++;
	}
	pT->Unlock();
	delete[] pvars;
}

void Fire_OLEDragOver(
	CfpDataObject** DataObject,
	long* Effect, 
	short* Button, 
	short* Shift, 
	float* X, 
	float* Y, 
	short* State)
{
	VARIANTARG* pvars = new VARIANTARG[7];
	for (int i = 0; i < 7; i++)
		VariantInit(&pvars[i]);
	T* pT = (T*)this;
	pT->Lock();
	IUnknown** pp = m_vec.begin();
	IUnknown** ppEnd = m_vec.end();
	while (pp < ppEnd)
	{
		if (*pp != NULL)
		{
			pvars[6].vt = VT_BYREF|VT_DISPATCH;
			pvars[6].ppdispVal= (LPDISPATCH*)DataObject;
			pvars[5].vt = VT_BYREF|VT_I4;
			pvars[5].plVal= Effect;
			pvars[4].vt = VT_BYREF|VT_I2;
			pvars[4].piVal= Button;
			pvars[3].vt = VT_BYREF|VT_I2;
			pvars[3].piVal= Shift;
			pvars[2].vt = VT_BYREF|VT_R4;
			pvars[2].pfltVal= X;
			pvars[1].vt = VT_BYREF|VT_R4;
			pvars[1].pfltVal= Y;
			pvars[0].vt = VT_BYREF|VT_I2;
			pvars[0].piVal= State;
			DISPPARAMS disp = { pvars, NULL, 7, 0 };
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
			pDispatch->Invoke(DISPID_OLEDRAGOVER, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
		}
		pp++;
	}
	pT->Unlock();
	delete[] pvars;
}

void Fire_OLEGiveFeedback(
	long* Effect, 
	BOOL* DefaultCursors)
{
// fix for bug 11064 -scl
  VARIANT_BOOL defCursors = *DefaultCursors ? VARIANT_TRUE : VARIANT_FALSE;
	VARIANTARG* pvars = new VARIANTARG[2];
	for (int i = 0; i < 2; i++)
		VariantInit(&pvars[i]);
	T* pT = (T*)this;
	pT->Lock();
	IUnknown** pp = m_vec.begin();
	IUnknown** ppEnd = m_vec.end();
	while (pp < ppEnd)
	{
		if (*pp != NULL)
		{
			pvars[1].vt = VT_BYREF|VT_I4;
			pvars[1].plVal= Effect;
			pvars[0].vt = VT_BYREF|VT_BOOL;
//			pvars[0].pboolVal= (VARIANT_BOOL*)DefaultCursors;
			pvars[0].pboolVal= (VARIANT_BOOL*)&defCursors;
			DISPPARAMS disp = { pvars, NULL, 2, 0 };
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
			pDispatch->Invoke(DISPID_OLEGIVEFEEDBACK, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
		}
		pp++;
	}
	pT->Unlock();
	delete[] pvars;
  *DefaultCursors = (defCursors == VARIANT_FALSE ? FALSE : TRUE);
}

void Fire_OLESetData(
	CfpDataObject** DataObject, 
	short* Format)
{
	VARIANTARG* pvars = new VARIANTARG[2];
	for (int i = 0; i < 2; i++)
		VariantInit(&pvars[i]);
	T* pT = (T*)this;
	pT->Lock();
	IUnknown** pp = m_vec.begin();
	IUnknown** ppEnd = m_vec.end();
	while (pp < ppEnd)
	{
		if (*pp != NULL)
		{
			pvars[1].vt = VT_BYREF|VT_DISPATCH;
			pvars[1].ppdispVal= (LPDISPATCH*)DataObject;
			pvars[0].vt = VT_BYREF|VT_I2;
			pvars[0].piVal= Format;
			DISPPARAMS disp = { pvars, NULL, 2, 0 };
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
			pDispatch->Invoke(DISPID_OLESETDATA, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
		}
		pp++;
	}
	pT->Unlock();
	delete[] pvars;
}

void Fire_OLEStartDrag(
	CfpDataObject** DataObject, 
	long* AllowedEffects)
{
	VARIANTARG* pvars = new VARIANTARG[2];
	for (int i = 0; i < 2; i++)
		VariantInit(&pvars[i]);
	T* pT = (T*)this;
	pT->Lock();
	IUnknown** pp = m_vec.begin();
	IUnknown** ppEnd = m_vec.end();
	while (pp < ppEnd)
	{
		if (*pp != NULL)
		{
			pvars[1].vt = VT_BYREF|VT_DISPATCH;
			pvars[1].ppdispVal= (LPDISPATCH*)DataObject;
			pvars[0].vt = VT_BYREF|VT_I4;
			pvars[0].plVal= AllowedEffects;
			DISPPARAMS disp = { pvars, NULL, 2, 0 };
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
			pDispatch->Invoke(DISPID_OLESTARTDRAG, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
		}
		pp++;
	}
	pT->Unlock();
	delete[] pvars;
}

