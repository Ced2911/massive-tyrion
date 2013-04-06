// Filename:-	win_video.cpp
//
// leave this as first line for PCH reasons...
//
#include "../server/exe_headers.h"

#include "xdk_local.h"

#include "../client/client.h"
#include "../client/fffx.h"
#include "../ff/common_headers.h"

typedef int	ffHandle_t;

void CL_InitFF( void )
{
}

void CL_ShutdownFF( void )
{
}

qboolean IsLocalClient( int clientNum )
{
	return false;
}

void CL_FF_Start( ffHandle_t ff, int clientNum )
{
}

void CL_FF_Stop( ffHandle_t ff, int clientNum )
{
}


void CL_FF_AddLoopingForce( ffHandle_t ff, int clientNum )
{
}


void _FF_PlayFXSlot(int iSlotNum)
{

}



void FF_StopAll(void)
{
}


void FF_Stop(ffFX_e fffx)
{

}

void FF_EnsurePlaying(ffFX_e fffx)
{

}

qboolean FF_Play(ffHandle_t ff)
{
	return false;
}

void FF_Play(ffFX_e fffx)
{

}

void FF_Shutdown(void)
{

}

qboolean FF_Init( void )
{
	return false;
}
