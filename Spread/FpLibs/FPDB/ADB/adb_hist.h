/*-------------------------------------------------------------------------

DB_HIST.H  -  Strictly comments listing the changes to FPTOOLS library.         
                                                                       
HISTORY OF CHANGES:
-----------------------                                                
                                                                       
$00   04/20/99  SCL: Creation of this history log file.
$01	  04/20/99  SCL: Fixed some memory leaks in CfpOleBinding.  Added code to
					 destructor to free data member string, and to 
					 CfpOleBinding::fpVerifySetDataAccessor() to free the 
					 data bindings that were allocated.
					 Files: Fpoledba.cpp
$02   05/11/99  SCP: Implemented CfpDbStream in fpdbsupp.h/.cpp because 
                     of a reported bug of Memo fields not writing with
                     ADO 2.1 (ok w/ADO 2.0).
$03   05/17/99  SCP: Add support for retrieving the list of DataMember's.
                     Used by VB's browser to give user a list of choices.
$04   06/14/99  SCL: Changed default value for CfpOleBinding::m_fFreezeEvents
					 from TRUE to FALSE (makes controls bind correctly in VC++)
$05   06/17/99  SCP: Added support for reading data of type, DBTYPE_VARIANT.

--------------------------------------------------------------------------*/

