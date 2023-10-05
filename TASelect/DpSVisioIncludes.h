#pragma once

#include <dbt.h>
#include <Hidsdi.h>
#include "afxwin.h"
#include "afxcmn.h"
#include "hidapi.h"
#include "USB100.h"
#include "DFUFile.h"
#include "STDFU.H"
#include "STDFUPrtInc.h"
#include "STDFUFilesinc.h"
#include "libusb.h"
#include "dfu_util.h" 
#include "dfu_file.h"
#include "dfuse.h"
#include "dfu.h"
#include "quirks.h"
#include "TASelect.h"
#include "DpSVisioUSBcmdDef.h"
#include "DpSVisioUtils.h"
#include "DpSVisioBaseCommand.h"
#include "DpSVisioDFUCommand.h"
#include "DpSVisioHIDCommand.h"
#include "DpSVisioBaseCommunication.h"
#include "DpSVisioDFUCommunication.h"
#include "DpSVisioNewDFUCommunication.h"
#include "DpSVisioHIDCommunication.h"

#ifdef DEBUG
#include "DpSVisioFakeHIDCommunication.h"
#include "DpSVisioFakeDFUCommunication.h"
#endif

#include "DlgDpSVisioUnplugPlug.h"
#include "DlgDpSVisioDFUInstallation.h"
#include "DlgDpSVisio.h"
#include "DlgDpSVisioBase.h"
#include "DlgDpSVisioStatus.h"
#include "DlgDpSVisioHIDUpgradeRunning.h"
#include "DlgDpSVisioDFUUpgradeRunning.h"
#include "DlgDpSVisioUpgradeFinished.h"

#define DFUCMD_STARTERASE		0x0001
#define DFUCMD_CHECKERASE		0x0002
#define DFUCMD_STARTUPGRADE		0x0003
#define DFUCMD_CHECKUPGRADE		0x0004
#define DFUCMD_DFUDETACH		0x0005
