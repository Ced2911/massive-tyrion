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
#include <xinput2.h>

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

//
// Joystick definitions
//
#define	JOY_MAX_AXES		6				// X, Y, Z, R, U, V

typedef struct {
	qboolean	avail;
	int			id;			// joystick number
	// JOYCAPS		jc;

	int			oldbuttonstate;
	int			oldpovstate;

	// JOYINFOEX	ji;
} joystickInfo_t;

static	joystickInfo_t	joy;

/* 
=============== 
IN_StartupJoystick 
=============== 
*/  
void IN_StartupJoystick (void) { 
	// Detect available controller
	DWORD dwResult; 
	DWORD nbJoy;
	for (DWORD i=0; i< XUSER_MAX_COUNT; i++ )
	{
		XINPUT_STATE state;
		ZeroMemory( &state, sizeof(XINPUT_STATE) );

		// Simply get the state of the controller from XInput.
		dwResult = XInputGetState( i, &state );

		if( dwResult == ERROR_SUCCESS )
		{ 
			// Controller is connected 
			nbJoy++;
		}
		else
		{
			// Controller is not connected 
		}
	}


	// Do something ...

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
	A_CURSOR_UP, A_CURSOR_DOWN,
	A_CURSOR_LEFT, A_CURSOR_RIGHT,
	//A_JOY16, A_JOY17,
	A_MOUSE1, A_MOUSE2,
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
static short filter_axis(short axis, int deadzone) {
	if(axis < deadzone && axis > -deadzone)
		axis = 0;

	return axis;
}

static DWORD oldbuttonstate;

void IN_JoyMove( void ) {
	float	fAxisValue;
	int		i;
	XINPUT_STATE state;
	DWORD	buttonstate, povstate;
	int		x, y;

	if (SUCCEEDED(XInputGetState(0,  &state))) {
		buttonstate = state.Gamepad.wButtons;

		for( i = 0; i < 16; i++ ) {				
			// 0 && 1 // pushed
			if ( (buttonstate & (1<<i)) && !(oldbuttonstate & (1<<i)) ) {
				// Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, A_JOY0 + i, qtrue, 0, NULL );
				Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, joyDirectionKeys[i], qtrue, 0, NULL );
				
			} 
			// 1 && 0 // released
			if ( !(buttonstate & (1<<i)) && (oldbuttonstate & (1<<i)) ) {
				// Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, A_JOY0 + i, qfalse, 0, NULL );
				Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, joyDirectionKeys[i], qfalse, 0, NULL );
			}
		}

		// Filters axis
		state.Gamepad.sThumbLX = filter_axis(state.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		state.Gamepad.sThumbLY = filter_axis(state.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		state.Gamepad.sThumbRX = filter_axis(state.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		state.Gamepad.sThumbRY = filter_axis(state.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

		// Mouse emulation like
		int mx, my;

		mx = state.Gamepad.sThumbLX>>13;
		my = -state.Gamepad.sThumbLY>>13;

		Sys_QueEvent( 0, SE_MOUSE, mx, my, 0, NULL );
		

		// Save button for next loop
		oldbuttonstate = buttonstate;
	}

#if 0
	// loop through the joystick buttons
	// key a joystick event or auxillary event for higher number buttons for each state change
	buttonstate = joy.ji.dwButtons;
	for ( i=0 ; i < joy.jc.wNumButtons ; i++ ) {
		if ( (buttonstate & (1<<i)) && !(joy.oldbuttonstate & (1<<i)) ) {
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, A_JOY0 + i, qtrue, 0, NULL );
		}
		if ( !(buttonstate & (1<<i)) && (joy.oldbuttonstate & (1<<i)) ) {
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, A_JOY0 + i, qfalse, 0, NULL );
		}
	}
	joy.oldbuttonstate = buttonstate;

	povstate = 0;

	// convert main joystick motion into 6 direction button bits
	for (i = 0; i < joy.jc.wNumAxes && i < 4 ; i++) {
		// get the floating point zero-centered, potentially-inverted data for the current axis
		fAxisValue = JoyToF( (&joy.ji.dwXpos)[i] );
		
		if (i == 0 && !joy_xbutton->integer) {
			if ( fAxisValue < -joy_threshold->value || fAxisValue > joy_threshold->value){
				Sys_QueEvent( g_wv.sysMsgTime, SE_JOYSTICK_AXIS, AXIS_SIDE, (int) -(fAxisValue*127.0), 0, NULL );
			}else{
				Sys_QueEvent( g_wv.sysMsgTime, SE_JOYSTICK_AXIS, AXIS_SIDE, 0, 0, NULL );
			}
			continue;
		}
		
		if (i == 1 && !joy_ybutton->integer) {
			if ( fAxisValue < -joy_threshold->value || fAxisValue > joy_threshold->value){
				Sys_QueEvent( g_wv.sysMsgTime, SE_JOYSTICK_AXIS, AXIS_FORWARD, (int) -(fAxisValue*127.0), 0, NULL );
			}else{
				Sys_QueEvent( g_wv.sysMsgTime, SE_JOYSTICK_AXIS, AXIS_FORWARD, 0, 0, NULL );
			}
			continue;
		}
		
		if ( fAxisValue < -joy_threshold->value ) {
			povstate |= (1<<(i*2));
		} else if ( fAxisValue > joy_threshold->value ) {
			povstate |= (1<<(i*2+1));
		}
	}		

	// convert POV information from a direction into 4 button bits
	if ( joy.jc.wCaps & JOYCAPS_HASPOV ) {
		if ( joy.ji.dwPOV != JOY_POVCENTERED ) {
			if (joy.ji.dwPOV == JOY_POVFORWARD)
				povstate |= 1<<12;
			if (joy.ji.dwPOV == JOY_POVBACKWARD)
				povstate |= 1<<13;
			if (joy.ji.dwPOV == JOY_POVRIGHT)
				povstate |= 1<<14;
			if (joy.ji.dwPOV == JOY_POVLEFT)
				povstate |= 1<<15;
		}
	}

	// determine which bits have changed and key an auxillary event for each change
	for (i=0 ; i < 16 ; i++) {
		if ( (povstate & (1<<i)) && !(joy.oldpovstate & (1<<i)) ) {
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, joyDirectionKeys[i], qtrue, 0, NULL );
		}

		if ( !(povstate & (1<<i)) && (joy.oldpovstate & (1<<i)) ) {
			Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, joyDirectionKeys[i], qfalse, 0, NULL );
		}
	}
	joy.oldpovstate = povstate;

	// if there is a trackball like interface, simulate mouse moves
	if ( joy.jc.wNumAxes >= 6 ) {
		x = JoyToI( joy.ji.dwUpos ) * in_joyBallScale->value;
		y = JoyToI( joy.ji.dwVpos ) * in_joyBallScale->value;
		if ( x || y ) {
			Sys_QueEvent( g_wv.sysMsgTime, SE_MOUSE, x, y, 0, NULL );
		}
	}

#endif
}
