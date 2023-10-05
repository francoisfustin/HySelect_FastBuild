/*************************************************************************\
* FPOLEDB.H                                                               *
*                                                                         *
* Copyright (C) 1991-1998 - FarPoint Technologies, Inc.                   *
* All Rights Reserved.                                                    *
*                                                                         *
* No part of this source code may be copied, modified or reproduced       *
* in any form without retaining the above copyright notice.  This         *
* source code, or source code derived from it, may not be redistributed   *
* without express written permission of FarPoint Technologies.            *
*                                                                         *
\*************************************************************************/

//
// fpoledb.h : Declaration of the CfpOleBinding class.
//
//             FarPoint's implementation of DB Binding for OLE controls.
//
//             This class is intented to be a member variable (public) of
//             an OCX control (subclassed from COleControl).
//

#ifndef FPOLEDB_H
#define FPOLEDB_H

#ifdef FP_ADB
#include "fpoledba.h"
#else
#include "fpoledbc.h"
#endif

#endif  // ifndef FPOLEDB_H
