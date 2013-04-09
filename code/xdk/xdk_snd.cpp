// leave this as first line for PCH reasons...
//
#include "../server/exe_headers.h"



#include <float.h>

#include "../client/snd_local.h"
#include "xdk_local.h"

/*
==================
SNDDMA_Shutdown
==================
*/
void SNDDMA_Shutdown( void ) {
	Com_DPrintf( "Shutting down sound system\n" );

	memset ((void *)&dma, 0, sizeof (dma));
}

/*
==================
SNDDMA_Init

Initialize direct sound
Returns false if failed
==================
*/
qboolean SNDDMA_Init(void) {

	memset ((void *)&dma, 0, sizeof (dma));

	// create the secondary buffer we'll actually work with
	dma.channels = 2;
	dma.samplebits = 16;

	if (s_khz->integer == 44)
		dma.speed = 44100;
	else if (s_khz->integer == 22)
		dma.speed = 22050;
	else
		dma.speed = 11025;
	
	SNDDMA_BeginPainting ();
	if (dma.buffer)
		memset(dma.buffer, 0, dma.samples * dma.samplebits/8);
	SNDDMA_Submit ();

	// Hack !!
	dma.samples = 32768;
	dma.submission_chunk = 1;

	dma.buffer = new byte[dma.samples * 2];

	Com_DPrintf("Completed successfully\n" );

    return qtrue;
}
/*
==============
SNDDMA_GetDMAPos

return the current sample position (in mono samples read)
inside the recirculating dma buffer, so the mixing code will know
how many sample are required to fill it up.
===============
*/
int SNDDMA_GetDMAPos( void ) {
	return 0x2000;
}

/*
==============
SNDDMA_BeginPainting

Makes sure dma.buffer is valid
===============
*/
void SNDDMA_BeginPainting( void ) {
	
}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
Also unlocks the dsound buffer
===============
*/
void SNDDMA_Submit( void ) {
    
}


/*
=================
SNDDMA_Activate

When we change windows we need to do this
=================
*/
void SNDDMA_Activate( qboolean bAppActive )
{
	
}



// I know this is a bit horrible, but I need to pass our LPDIRECTSOUND ptr to Bink for video playback,
//	and I don't want other modules to have to know about LPDIRECTSOUND handles, hence the int casting
//
// (I'd prefer to use DWORD, but not all modules understand those)
//
unsigned int SNDDMA_GetDSHandle(void)
{
#ifdef _DEBUG
	// Uch !!!
	DebugBreak();
#endif
	return NULL;
}


