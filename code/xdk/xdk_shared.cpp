// leave this as first line for PCH reasons...
//
#include "../server/exe_headers.h"




#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"
#include "xdk_local.h"

/*
================
Sys_Milliseconds
================
*/
int sys_timeBase;
int Sys_Milliseconds (void)
{
	int sys_curtime;
	static qboolean initialized = qfalse;

	if (!initialized) {		
		sys_timeBase = GetTickCount();
		initialized = qtrue;
	}
	sys_curtime = GetTickCount() - sys_timeBase;

	return sys_curtime;
}

int Sys_GetProcessorId( void )
{
	return CPUID_GENERIC;
}

//============================================

char *Sys_GetCurrentUser( void )
{
	static char s_userName[1024];
	unsigned long size = sizeof( s_userName );

	if ( !s_userName[0] )
	{
		strcpy( s_userName, "player" );
	}

	return s_userName;
}