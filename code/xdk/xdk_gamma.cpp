// leave this as first line for PCH reasons...
//
#include "../server/exe_headers.h"



#include <assert.h>
#include "../renderer/tr_local.h"
#include "../qcommon/qcommon.h"
#include "glw_xdk.h"
#include "xdk_local.h"


/*
** WG_CheckHardwareGamma
**
** Determines if the underlying hardware supports the Win32 gamma correction API.
*/
void WG_CheckHardwareGamma( void )
{
	
}

/*
** GLimp_SetGamma
**
** This routine should only be called if glConfig.deviceSupportsGamma is TRUE
*/
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] ) {
	
}

/*
** WG_RestoreGamma
*/
void WG_RestoreGamma( void )
{
	
}

