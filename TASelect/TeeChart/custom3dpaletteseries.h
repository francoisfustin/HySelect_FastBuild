// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CValueList;
class CBrush1;
class CPen1;

/////////////////////////////////////////////////////////////////////////////
// CCustom3DPaletteSeries wrapper class

class CCustom3DPaletteSeries : public COleDispatchDriver
{
public:
	CCustom3DPaletteSeries() {}		// Calls COleDispatchDriver default constructor
	CCustom3DPaletteSeries(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CCustom3DPaletteSeries(const CCustom3DPaletteSeries& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	long AddXYZ(double AX, double AY, double AZ, LPCTSTR AXLabel, unsigned long Value);
	double MaxZValue();
	double MinZValue();
	long GetTimesZOrder();
	void SetTimesZOrder(long nNewValue);
	CValueList GetZValues();
	double GetZValue(long Index);
	void SetZValue(long Index, double newValue);
	CBrush1 GetBrush();
	CPen1 GetPen();
	void AddArrayXYZ(const VARIANT& XArray, const VARIANT& YArray, const VARIANT& ZArray);
	void AddArrayGrid(long NumX, long NumZ, const VARIANT& XZArray);
	long CalcZPos(long ValueIndex);
	void AddArrayXYZColor(const VARIANT& XArray, const VARIANT& YArray, const VARIANT& ZArray, const VARIANT& Colors);
	long AddPalette(double Value, unsigned long Color);
	unsigned long GetStartColor();
	void SetStartColor(unsigned long newValue);
	unsigned long GetEndColor();
	void SetEndColor(unsigned long newValue);
	long GetPaletteSteps();
	void SetPaletteSteps(long nNewValue);
	BOOL GetUsePalette();
	void SetUsePalette(BOOL bNewValue);
	BOOL GetUseColorRange();
	void SetUseColorRange(BOOL bNewValue);
	void ClearPalette();
	void CreateDefaultPalette(long NumSteps);
	unsigned long GetSurfacePaletteColor(double Y);
	unsigned long GetMidColor();
	void SetMidColor(unsigned long newValue);
	void CreateRangePalette();
	long GetPaletteStyle();
	void SetPaletteStyle(long nNewValue);
	BOOL GetUsePaletteMin();
	void SetUsePaletteMin(BOOL bNewValue);
	double GetPaletteMin();
	void SetPaletteMin(double newValue);
	double GetPaletteStep();
	void SetPaletteStep(double newValue);
	void InvertPalette();
	void AddCustomPalette(const VARIANT& colorArray);
	long GetLegendEvery();
	void SetLegendEvery(long nNewValue);
	long CountLegendItems();
	void LoadPaletteFromFile(LPCTSTR FileName);
	void SavePaletteToFile(LPCTSTR FileName);
};
