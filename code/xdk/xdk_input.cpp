// win_input.c -- win32 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.

// leave this as first line for PCH reasons...
//
#include "../server/exe_headers.h"




#include "../client/client.h"
#ifndef _IMMERSION
#include "../client/fffx.h"
#endif // _IMMERSION
#include "xdk_local.h"

cvar_t	*in_midi;
cvar_t	*in_midiport;
cvar_t	*in_midichannel;
cvar_t	*in_mididevice;

cvar_t	*in_mouse;
cvar_t	*in_joystick;
cvar_t	*in_joyBallScale;
cvar_t	*in_debugJoystick;
cvar_t	*joy_threshold;
cvar_t	*js_ffmult;
cvar_t	*joy_xbutton;
cvar_t	*joy_ybutton;

qboolean	in_appactive;

// forward-referenced functions
void IN_StartupJoystick (void);
void IN_JoyMove(void);

/*
============================================================

WIN32 MOUSE CONTROL

============================================================
*/

/*
================
IN_InitWin32Mouse
================
*/
void IN_InitWin32Mouse( void ) 
{
}

/*
================
IN_ShutdownWin32Mouse
================
*/
void IN_ShutdownWin32Mouse( void ) {
}

/*
================
IN_ActivateWin32Mouse
================
*/
void IN_ActivateWin32Mouse( void ) {
}

/*
================
IN_DeactivateWin32Mouse
================
*/
void IN_DeactivateWin32Mouse( void ) 
{
}

/*
================
IN_Win32Mouse
================
*/
void IN_Win32Mouse( int *mx, int *my ) {
	
}



void IN_DIMouse( int *mx, int *my );

/*
========================
IN_InitDIMouse
========================
*/
qboolean IN_InitDIMouse( void ) {
	return qtrue;
}

/*
==========================
IN_ShutdownDIMouse
==========================
*/
void IN_ShutdownDIMouse( void ) 
{
   
}

/*
==========================
IN_ActivateDIMouse
==========================
*/
void IN_ActivateDIMouse( void ) {
	
}

/*
==========================
IN_DeactivateDIMouse
==========================
*/
void IN_DeactivateDIMouse( void ) {
}


/*
===================
IN_DIMouse
===================
*/
void IN_DIMouse( int *mx, int *my ) {
}

/*
============================================================

  MOUSE CONTROL

============================================================
*/

/*
===========
IN_ActivateMouse

Called when the window gains focus or changes in some way
===========
*/
void IN_ActivateMouse( void ) 
{
	
}


/*
===========
IN_DeactivateMouse

Called when the window loses focus
===========
*/
void IN_DeactivateMouse( void ) {
	
}



/*
===========
IN_StartupMouse
===========
*/
void IN_StartupMouse( void ) 
{
	
}

void IN_MouseEvent (int mstate)
{

}



/*
===========
IN_MouseMove
===========
*/
void IN_MouseMove ( void ) {
	
}


/*
=========================================================================

=========================================================================
*/

/*
===========
IN_Startup
===========
*/
void IN_Startup( void ) {
	Com_Printf ("\n------- Input Initialization -------\n");
	IN_StartupMouse ();
	IN_StartupJoystick ();
//	IN_StartupMIDI();
	Com_Printf ("------------------------------------\n");

	in_mouse->modified = qfalse;
	in_joystick->modified = qfalse;
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown( void ) {
	IN_DeactivateMouse();
	IN_ShutdownDIMouse();
	Cmd_RemoveCommand("midiinfo" );
}


/*
===========
IN_Init
===========
*/
void IN_Init( void ) {
	// MIDI input controler variables
	in_midi					= Cvar_Get ("in_midi",					"0",		CVAR_ARCHIVE);
	in_midiport				= Cvar_Get ("in_midiport",				"1",		CVAR_ARCHIVE);
	in_midichannel			= Cvar_Get ("in_midichannel",			"1",		CVAR_ARCHIVE);
	in_mididevice			= Cvar_Get ("in_mididevice",			"0",		CVAR_ARCHIVE);

//	Cmd_AddCommand( "midiinfo", MidiInfo_f );

	// mouse variables
    in_mouse				= Cvar_Get ("in_mouse",					"-1",		CVAR_ARCHIVE|CVAR_LATCH);

	// joystick variables
	in_joystick				= Cvar_Get ("in_joystick",				"0",		CVAR_ARCHIVE|CVAR_LATCH);
	in_joyBallScale			= Cvar_Get ("in_joyBallScale",			"0.02",		CVAR_ARCHIVE);
	in_debugJoystick		= Cvar_Get ("in_debugjoystick",			"0",		CVAR_TEMP);

	joy_threshold			= Cvar_Get ("joy_threshold",			"0.15",		CVAR_ARCHIVE);

	js_ffmult				= Cvar_Get ("js_ffmult",				"3.0",		CVAR_ARCHIVE);	// force feedback

	joy_xbutton			= Cvar_Get ("joy_xbutton",			"1",		CVAR_ARCHIVE);	// treat axis as a button
	joy_ybutton			= Cvar_Get ("joy_ybutton",			"0",		CVAR_ARCHIVE);	// treat axis as a button

	IN_Startup();
}


/*
===========
IN_Activate

Called when the main window gains or loses focus.
The window may have been destroyed and recreated
between a deactivate and an activate.
===========
*/
void IN_Activate (qboolean active) {
	in_appactive = active;

	if ( !active )
	{
		IN_DeactivateMouse();
	}
}


/*
==================
IN_Frame

Called every frame, even if not generating commands
==================
*/
void IN_Frame (void) {
	// post joystick events
	IN_JoyMove();

	// post events to the system que
	IN_MouseMove();

}


/*
===================
IN_ClearStates
===================
*/
void IN_ClearStates (void) 
{
}


/*
=========================================================================

JOYSTICK

=========================================================================
*/

/* 
=============== 
IN_StartupJoystick 
=============== 
*/  
void IN_StartupJoystick (void) { 
	
}

/*
===========
JoyToF
===========
*/
float JoyToF( int value ) {
	float	fValue;

	// move centerpoint to zero
	value -= 32768;

	// convert range from -32768..32767 to -1..1 
	fValue = (float)value / 32768.0;

	if ( fValue < -1 ) {
		fValue = -1;
	}
	if ( fValue > 1 ) {
		fValue = 1;
	}
	return fValue;
}

int JoyToI( int value ) {
	// move centerpoint to zero
	value -= 32768;

	return value;
}

int	joyDirectionKeys[16] = {
	A_CURSOR_LEFT, A_CURSOR_RIGHT,
	A_CURSOR_UP, A_CURSOR_DOWN,
	A_JOY16, A_JOY17,
	A_JOY18, A_JOY19,
	A_JOY20, A_JOY21,
	A_JOY22, A_JOY23,

	A_JOY24, A_JOY25,
	A_JOY26, A_JOY27
};

/*
===========
IN_JoyMove
===========
*/
void IN_JoyMove( void ) {
	
}
