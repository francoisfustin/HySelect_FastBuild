#include "stdafx.h"
#include "TASelect.h"
#include "HMInclude.h"
#include "DlgDefUnits.h"

CDlgDefUnits::CDlgDefUnits( CWnd *pParent )
	: CDialogEx( CDlgDefUnits::IDD, pParent )
{
}

int CDlgDefUnits::Display( CUnitDatabase *pUdb )
{
	m_UnitDB = *pUdb;
	int iRetVal = DoModal();

	if( IDOK == iRetVal )
	{
		*pUdb = m_UnitDB;
	}

	return iRetVal;
}

BEGIN_MESSAGE_MAP( CDlgDefUnits, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBODIFFPRESS, OnCbnSelChangeDifferentialPressure )
	ON_CBN_SELCHANGE( IDC_COMBOPRESSURE, OnCbnSelChangeStaticPressure )
	ON_CBN_SELCHANGE( IDC_COMBOLINDP, OnCbnSelChangeLinearPressureDrop )
	ON_CBN_SELCHANGE( IDC_COMBOFLOW, OnCbnSelChangeFlow )
	ON_CBN_SELCHANGE( IDC_COMBOVELOCITY, OnCbnSelChangeVelocity )
	ON_CBN_SELCHANGE( IDC_COMBOTEMP, OnCbnSelChangeTemperature )
	ON_CBN_SELCHANGE( IDC_COMBODIFFTEMP, OnCbnSelChangeTemperatureDifference )
	ON_CBN_SELCHANGE( IDC_COMBOPOWER, OnCbnSelChangePower )
	ON_CBN_SELCHANGE( IDC_COMBOELECPOWER, OnCbnSelChangeElecPower )
	ON_CBN_SELCHANGE( IDC_COMBOWATERHARD, OnCbnSelChangeWaterHardness )
	ON_CBN_SELCHANGE( IDC_COMBODENSITY, OnCbnSelChangeDensity )
	ON_CBN_SELCHANGE( IDC_COMBOKINVISC, OnCbnSelChangeKinematicViscosity )
	ON_CBN_SELCHANGE( IDC_COMBOSPECIFHEAT, OnCbnSelChangeSpecificHeat )
	ON_BN_CLICKED( IDC_RADIOKVCV1, OnRadioValveCoefficientKv )
	ON_BN_CLICKED( IDC_RADIOKVCV2, OnRadioValveCoefficientCv )
	ON_CBN_SELCHANGE( IDC_COMBOLENGTH, OnCbnSelChangeLength )
	ON_CBN_SELCHANGE( IDC_COMBODIAMETER, OnCbnSelChangeDiameter )
	ON_CBN_SELCHANGE( IDC_COMBOROUGHNESS, OnCbnSelChangeRoughness )
	ON_CBN_SELCHANGE( IDC_COMBOTHICKNESS, OnCbnSelChangeThickness )
	ON_CBN_SELCHANGE( IDC_COMBOAREA, OnCbnSelChangeArea )
	ON_CBN_SELCHANGE( IDC_COMBOVOLUME, OnCbnSelChangeVolume )
	ON_CBN_SELCHANGE( IDC_COMBOMASS, OnCbnSelChangeMass )
	ON_CBN_SELCHANGE( IDC_COMBOTHERMCOND, OnCbnSelChangeThermalConductivity )
	ON_CBN_SELCHANGE( IDC_COMBOHEATTRANSF, OnCbnSelChangeHeatTransfer )
	ON_CBN_SELCHANGE( IDC_COMBOFORCE, OnCbnSelChangeForce )
	ON_CBN_SELCHANGE( IDC_COMBOTORQUE, OnCbnSelChangeTorque )
	ON_CBN_SELCHANGE( IDC_COMBOENERGY, OnCbnSelChangeEnergy )
	ON_BN_CLICKED( IDC_BUTTONDEFAULT, OnBnClickedResetToDefault )
END_MESSAGE_MAP()

void CDlgDefUnits::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBODIFFPRESS, m_ComboDiffPress );
	DDX_Control( pDX, IDC_COMBOPRESSURE, m_ComboPressure );
	DDX_Control( pDX, IDC_COMBOLINDP, m_ComboLinDp );
	DDX_Control( pDX, IDC_COMBOFLOW, m_ComboFlow );
	DDX_Control( pDX, IDC_COMBOVELOCITY, m_ComboVelocity );
	DDX_Control( pDX, IDC_COMBOTEMP, m_ComboTemp );
	DDX_Control( pDX, IDC_COMBODIFFTEMP, m_ComboDiffTemp );
	DDX_Control( pDX, IDC_COMBOPOWER, m_ComboPower );
	DDX_Control( pDX, IDC_COMBOELECPOWER, m_ComboElecPower );
	DDX_Control( pDX, IDC_COMBOWATERHARD, m_ComboWaterHardness );
	DDX_Control( pDX, IDC_COMBODENSITY, m_ComboDensity );
	DDX_Control( pDX, IDC_COMBOKINVISC, m_ComboKinVisc );
	DDX_Control( pDX, IDC_COMBOSPECIFHEAT, m_ComboSpecifHeat );
	DDX_Control( pDX, IDC_STATICKVCV, m_GroupKvCv );
	DDX_Control( pDX, IDC_RADIOKVCV1, m_RadioWorkForControlValvesKvsValues );
	DDX_Control( pDX, IDC_RADIOKVCV2, m_RadioCv );
	DDX_Control( pDX, IDC_COMBOLENGTH, m_ComboLength );
	DDX_Control( pDX, IDC_COMBODIAMETER, m_ComboDiameter );
	DDX_Control( pDX, IDC_COMBOROUGHNESS, m_ComboRoughness );
	DDX_Control( pDX, IDC_COMBOTHICKNESS, m_ComboThickness );
	DDX_Control( pDX, IDC_COMBOAREA, m_ComboArea );
	DDX_Control( pDX, IDC_COMBOVOLUME, m_ComboVolume );
	DDX_Control( pDX, IDC_COMBOMASS, m_ComboMass );
	DDX_Control( pDX, IDC_COMBOTHERMCOND, m_ComboThermCond );
	DDX_Control( pDX, IDC_COMBOHEATTRANSF, m_ComboHeatTransf );
	DDX_Control( pDX, IDC_COMBOFORCE, m_ComboForce );
	DDX_Control( pDX, IDC_COMBOTORQUE, m_ComboTorque );
	DDX_Control( pDX, IDC_COMBOENERGY, m_ComboEnergy );

	DDX_Control( pDX, IDC_BUTTONDEFAULT, m_ButtonDefault );
}

void CDlgDefUnits::OnOK()
{
	PREVENT_ENTER_KEY

	CDialogEx::OnOK();
}

BOOL CDlgDefUnits::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Initialize dialog strings.
	CString str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_CAPTION );
	SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICDIFFPRESS );
	GetDlgItem( IDC_STATICDIFFPRESS )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICPRESSURE );
	GetDlgItem( IDC_STATICPRESSURE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICLINDP );
	GetDlgItem( IDC_STATICLINDP )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICFLOW );
	GetDlgItem( IDC_STATICFLOW )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICVELOCITY );
	GetDlgItem( IDC_STATICVELOCITY )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICTEMP );
	GetDlgItem( IDC_STATICTEMP )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICDIFFTEMP );
	GetDlgItem( IDC_STATICDIFFTEMP )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICPOWER );
	GetDlgItem( IDC_STATICPOWER )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICELECPOWER );
	GetDlgItem( IDC_STATICELECPOWER )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICWATERHARD );
	GetDlgItem( IDC_STATICWATERHARD )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICDENSITY );
	GetDlgItem( IDC_STATICDENSITY )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICKINVISC );
	GetDlgItem( IDC_STATICKINVISC )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICSPECIFHEAT );
	GetDlgItem( IDC_STATICSPECIFHEAT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICKVCV );
	m_GroupKvCv.SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICLENGTH );
	GetDlgItem( IDC_STATICLENGTH )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICDIAMETER );
	GetDlgItem( IDC_STATICDIAMETER )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICROUGHNESS );
	GetDlgItem( IDC_STATICROUGHNESS )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICTHICKNESS );
	GetDlgItem( IDC_STATICTHICKNESS )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICAREA );
	GetDlgItem( IDC_STATICAREA )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICVOLUME );
	GetDlgItem( IDC_STATICVOLUME )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICMASS );
	GetDlgItem( IDC_STATICMASS )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICTHERMCOND );
	GetDlgItem( IDC_STATICTHERMCOND )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICHEATTRANSF );
	GetDlgItem( IDC_STATICHEATTRANSF )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICFORCE );
	GetDlgItem( IDC_STATICFORCE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICTORQUE );
	GetDlgItem( IDC_STATICTORQUE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_STATICENERGY );
	GetDlgItem( IDC_STATICENERGY )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDEFUNITS_BUTTONDEFAULT );
	GetDlgItem( IDC_BUTTONDEFAULT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );

	// Fill the combo boxes.
	for( int i = 1 ; i < _UNITS_NUMBER_OF ; i++ )
	{
		_FillCombo( i );
	}

	// Give check state to the right radio button.
	m_RadioWorkForControlValvesKvsValues.SetCheck( !m_UnitDB.GetDefaultUnitIndex( _C_KVCVCOEFF ) );
	m_RadioCv.SetCheck( m_UnitDB.GetDefaultUnitIndex( _C_KVCVCOEFF ) );

	return TRUE;
}

void CDlgDefUnits::OnCbnSelChangeDifferentialPressure()
{
	m_UnitDB.SetDefaultUnit( _U_DIFFPRESS, m_ComboDiffPress.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeStaticPressure()
{
	m_UnitDB.SetDefaultUnit( _U_PRESSURE, m_ComboPressure.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeLinearPressureDrop()
{
	m_UnitDB.SetDefaultUnit( _U_LINPRESSDROP, m_ComboLinDp.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeFlow()
{
	m_UnitDB.SetDefaultUnit( _U_FLOW, m_ComboFlow.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeVelocity()
{
	m_UnitDB.SetDefaultUnit( _U_VELOCITY, m_ComboVelocity.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeTemperature()
{
	m_UnitDB.SetDefaultUnit( _U_TEMPERATURE, m_ComboTemp.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeTemperatureDifference()
{
	m_UnitDB.SetDefaultUnit( _U_DIFFTEMP, m_ComboDiffTemp.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangePower()
{
	m_UnitDB.SetDefaultUnit( _U_TH_POWER, m_ComboPower.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeElecPower()
{
	m_UnitDB.SetDefaultUnit( _U_ELEC_POWER, m_ComboElecPower.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeWaterHardness()
{
	m_UnitDB.SetDefaultUnit( _U_WATERHARDNESS, m_ComboWaterHardness.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeDensity()
{
	m_UnitDB.SetDefaultUnit( _U_DENSITY, m_ComboDensity.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeKinematicViscosity()
{
	m_UnitDB.SetDefaultUnit( _U_KINVISCOSITY, m_ComboKinVisc.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeSpecificHeat()
{
	m_UnitDB.SetDefaultUnit( _U_SPECIFHEAT, m_ComboSpecifHeat.GetCurSel() );
}

void CDlgDefUnits::OnRadioValveCoefficientKv()
{
	m_RadioWorkForControlValvesKvsValues.SetCheck( BST_CHECKED );
	m_RadioCv.SetCheck( BST_UNCHECKED );
	m_UnitDB.SetDefaultUnit( _C_KVCVCOEFF, 0 );
}

void CDlgDefUnits::OnRadioValveCoefficientCv()
{
	m_RadioWorkForControlValvesKvsValues.SetCheck( BST_UNCHECKED );
	m_RadioCv.SetCheck( BST_CHECKED );
	m_UnitDB.SetDefaultUnit( _C_KVCVCOEFF, 1 );
}

void CDlgDefUnits::OnCbnSelChangeLength()
{
	m_UnitDB.SetDefaultUnit( _U_LENGTH, m_ComboLength.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeDiameter()
{
	m_UnitDB.SetDefaultUnit( _U_DIAMETER, m_ComboDiameter.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeRoughness()
{
	m_UnitDB.SetDefaultUnit( _U_ROUGHNESS, m_ComboRoughness.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeThickness()
{
	m_UnitDB.SetDefaultUnit( _U_THICKNESS, m_ComboThickness.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeArea()
{
	m_UnitDB.SetDefaultUnit( _U_AREA, m_ComboArea.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeVolume()
{
	m_UnitDB.SetDefaultUnit( _U_VOLUME, m_ComboVolume.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeMass()
{
	m_UnitDB.SetDefaultUnit( _U_MASS, m_ComboMass.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeThermalConductivity()
{
	m_UnitDB.SetDefaultUnit( _U_THERMCOND, m_ComboThermCond.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeHeatTransfer()
{
	m_UnitDB.SetDefaultUnit( _U_HEATTRANSF, m_ComboHeatTransf.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeForce()
{
	m_UnitDB.SetDefaultUnit( _U_FORCE, m_ComboForce.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeTorque()
{
	m_UnitDB.SetDefaultUnit( _U_TORQUE, m_ComboTorque.GetCurSel() );
}

void CDlgDefUnits::OnCbnSelChangeEnergy()
{
	m_UnitDB.SetDefaultUnit( _U_ENERGY, m_ComboEnergy.GetCurSel() );
}

void CDlgDefUnits::OnBnClickedResetToDefault()
{
	// Reset to localized defaults units.
	CDB_DefaultUnits *pLocalUnits = ( CDB_DefaultUnits * )( TASApp.GetpTADB()->Get( _T("DEFAULT_UNITS") ).MP );
	ASSERT( NULL != pLocalUnits );

	if( NULL == pLocalUnits )
	{
		return;
	}

	for( int i = 0; i < _UNITS_NUMBER_OF; i++ )
	{
		m_UnitDB.SetDefaultUnit( i, pLocalUnits->GetLocalizedDefaultUnits( i ) );
	}

	// Reset all combobox.
	m_ComboDiffPress.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_DIFFPRESS ) );
	m_ComboPressure.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_PRESSURE ) );
	m_ComboLinDp.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_LINPRESSDROP ) );
	m_ComboFlow.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_FLOW ) );
	m_ComboVelocity.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_VELOCITY ) );
	m_ComboTemp.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_TEMPERATURE ) );
	m_ComboDiffTemp.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_DIFFTEMP ) );
	m_ComboPower.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_TH_POWER ) );
	m_ComboElecPower.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_ELEC_POWER ) );
	m_ComboWaterHardness.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_WATERHARDNESS ) );
	m_ComboDensity.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_DENSITY ) );
	m_ComboKinVisc.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_KINVISCOSITY ) );
	m_ComboSpecifHeat.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_SPECIFHEAT ) );

	if( m_UnitDB.GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		m_RadioWorkForControlValvesKvsValues.SetCheck( BST_UNCHECKED );
		m_RadioCv.SetCheck( BST_CHECKED );
	}
	else
	{
		m_RadioWorkForControlValvesKvsValues.SetCheck( BST_CHECKED );
		m_RadioCv.SetCheck( BST_UNCHECKED );
	}

	m_ComboLength.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_LENGTH ) );
	m_ComboDiameter.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_DIAMETER ) );
	m_ComboRoughness.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_ROUGHNESS ) );
	m_ComboThickness.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_THICKNESS ) );
	m_ComboArea.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_AREA ) );
	m_ComboVolume.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_VOLUME ) );
	m_ComboMass.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_MASS ) );
	m_ComboThermCond.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_THERMCOND ) );
	m_ComboHeatTransf.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_HEATTRANSF ) );
	m_ComboForce.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_FORCE ) );
	m_ComboTorque.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_TORQUE ) );
	m_ComboEnergy.SetCurSel( m_UnitDB.GetDefaultUnitIndex( _U_ENERGY ) );
}

void CDlgDefUnits::_FillCombo( int iPhysType )
{
	ASSERT( ( iPhysType > 0 ) && ( iPhysType < _UNITS_NUMBER_OF ) ) ;

	bool bComboExist = true;
	CComboBox *pCombo;

	switch( iPhysType )
	{
		case _U_PRESSURE:
			pCombo = &m_ComboPressure;
			break;

		case _U_DIFFPRESS:
			pCombo = &m_ComboDiffPress;
			break;

		case _U_LINPRESSDROP:
			pCombo = &m_ComboLinDp;
			break;

		case _U_FLOW:
			pCombo = &m_ComboFlow;
			break;

		case _U_VELOCITY:
			pCombo = &m_ComboVelocity;
			break;

		case _U_TEMPERATURE:
			pCombo = &m_ComboTemp;
			break;

		case _U_DIFFTEMP:
			pCombo = &m_ComboDiffTemp;
			break;

		case _U_TH_POWER:
			pCombo = &m_ComboPower;
			break;
			
		case _U_ELEC_POWER:
			pCombo = &m_ComboElecPower;
			break;

		case _U_WATERHARDNESS:
			pCombo = &m_ComboWaterHardness;
			break;

		case _U_LENGTH:
			pCombo = &m_ComboLength;
			break;

		case _U_DIAMETER:
			pCombo = &m_ComboDiameter;
			break;

		case _U_ROUGHNESS:
			pCombo = &m_ComboRoughness;
			break;

		case _U_THICKNESS:
			pCombo = &m_ComboThickness;
			break;

		case _U_AREA:
			pCombo = &m_ComboArea;
			break;

		case _U_VOLUME:
			pCombo = &m_ComboVolume;
			break;

		case _U_MASS:
			pCombo = &m_ComboMass;
			break;

		case _U_DENSITY:
			pCombo = &m_ComboDensity;
			break;

		case _U_KINVISCOSITY:
			pCombo = &m_ComboKinVisc;
			break;

		case _U_DYNVISCOSITY:
			bComboExist = false;
			break;

		case _U_SPECIFHEAT:
			pCombo = &m_ComboSpecifHeat;
			break;

		case _U_THERMCOND:
			pCombo = &m_ComboThermCond;
			break;

		case _U_HEATTRANSF:
			pCombo = &m_ComboHeatTransf;
			break;

		case _U_FORCE:
			pCombo = &m_ComboForce;
			break;

		case _U_TORQUE:
			pCombo = &m_ComboTorque;
			break;
			
		case _U_ENERGY:
			pCombo = &m_ComboEnergy;
			break;

		case _C_KVCVCOEFF:
			bComboExist = false;
			return;

		// Not yet used default unit
		default:
			bComboExist = false;
			break;
	}

	if( true == bComboExist )
	{
		TCHAR name[_MAXCHARS];

		if( pCombo->GetCount() > 0 )
		{
			pCombo->ResetContent();
		}

		for( int i = 0 ; i < m_UnitDB.GetLength( iPhysType ) ; i++ )
		{
			GetNameOf( m_UnitDB.GetUnit( iPhysType, i ), name );
			pCombo->AddString( name );
		}

		pCombo->SetCurSel( m_UnitDB.GetDefaultUnitIndex( iPhysType ) );
	}
}
