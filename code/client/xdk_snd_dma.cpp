/*****************************************************************************
 * name:		snd_dma.c
 *
 * desc:		main control for any streaming sound output device
 *
 *
 *****************************************************************************/
// leave this as first line for PCH reasons...
//
#include "../server/exe_headers.h"

#include "snd_local.h"
#include "cl_mp3.h"
#include "snd_music.h"


portable_samplepair_t	s_rawsamples[MAX_RAW_SAMPLES];

int			s_rawend;
int			s_soundStarted;
qboolean	s_soundMuted;

dma_t		dma;

int			listener_number;
vec3_t		listener_origin;
vec3_t		listener_axis[3];

int			s_soundtime;		// sample PAIRS
int   		s_paintedtime; 		// sample PAIRS

cvar_t		*s_volume;
cvar_t		*s_volumeVoice;
cvar_t		*s_testsound;
cvar_t		*s_khz;
cvar_t		*s_allowDynamicMusic;
cvar_t		*s_show;
cvar_t		*s_mixahead;
cvar_t		*s_mixPreStep;
cvar_t		*s_musicVolume;
cvar_t		*s_separation;
cvar_t		*s_lip_threshold_1;
cvar_t		*s_lip_threshold_2;
cvar_t		*s_lip_threshold_3;
cvar_t		*s_lip_threshold_4;
cvar_t		*s_CPUType;
cvar_t		*s_language;	// note that this is distinct from "g_language"
cvar_t		*s_dynamix;
cvar_t		*s_debugdynamic;

int			s_entityWavVol[MAX_GENTITIES];

channel_t   s_channels[MAX_CHANNELS];

// MAX_SFX may be larger than MAX_SOUNDS because
// of custom player sounds
#define		MAX_SFX			10000	//512 * 2
sfx_t		s_knownSfx[MAX_SFX];
int			s_numSfx;

#define		LOOP_HASH		128
static	sfx_t		*sfxHash[LOOP_HASH];


void S_SoundInfo_f(void) {	
	Com_Printf("----- Sound Info -----\n" );

	S_DisplayFreeMemory();
	Com_Printf("----------------------\n" );
}


/*
===============================================================================

console functions

===============================================================================
*/

static void S_Play_f( void ) {
	int 	i;
	sfxHandle_t	h;
	char name[256];
	
	i = 1;
	while ( i<Cmd_Argc() ) {
		if ( !strrchr(Cmd_Argv(i), '.') ) {
			Com_sprintf( name, sizeof(name), "%s.wav", Cmd_Argv(1) );
		} else {
			Q_strncpyz( name, Cmd_Argv(i), sizeof(name) );
		}
		h = S_RegisterSound( name );
		if( h ) {
			S_StartLocalSound( h, CHAN_LOCAL_SOUND );
		}
		i++;
	}
}

static void S_Music_f( void ) {
	int		c;

	c = Cmd_Argc();

	if ( c == 2 ) {
		S_StartBackgroundTrack( Cmd_Argv(1), Cmd_Argv(1), qfalse );
	} else if ( c == 3 ) {
		S_StartBackgroundTrack( Cmd_Argv(1), Cmd_Argv(2), qfalse );		
	} else {
		Com_Printf ("music <musicfile> [loopfile]\n");
		return;
	}
}

// a debug function, but no harm to leave in...
//
static void S_SetDynamicMusic_f(void)
{
	
}

/*
================
S_Init
================
*/
void S_Init( void ) {

	cvar_t	*cv;
	qboolean	r;
	int i, j;
	channel_t *ch;

	Com_Printf("\n------- sound initialization -------\n");

	s_volume = Cvar_Get ("s_volume", "0.5", CVAR_ARCHIVE);
	s_volumeVoice= Cvar_Get ("s_volumeVoice", "1.0", CVAR_ARCHIVE);
	s_musicVolume = Cvar_Get ("s_musicvolume", "0.25", CVAR_ARCHIVE);
	s_separation = Cvar_Get ("s_separation", "0.5", CVAR_ARCHIVE);
	s_khz = Cvar_Get ("s_khz", "22", CVAR_ARCHIVE|CVAR_LATCH);
	s_allowDynamicMusic = Cvar_Get ("s_allowDynamicMusic", "1", CVAR_ARCHIVE);
	s_mixahead = Cvar_Get ("s_mixahead", "0.2", CVAR_ARCHIVE);

	s_mixPreStep = Cvar_Get ("s_mixPreStep", "0.05", CVAR_ARCHIVE);
	s_show = Cvar_Get ("s_show", "0", CVAR_CHEAT);
	s_testsound = Cvar_Get ("s_testsound", "0", CVAR_CHEAT);
	s_debugdynamic = Cvar_Get("s_debugdynamic","0", CVAR_CHEAT);
	s_lip_threshold_1 = Cvar_Get("s_threshold1" , "0.5",0);
	s_lip_threshold_2 = Cvar_Get("s_threshold2" , "4.0",0);
	s_lip_threshold_3 = Cvar_Get("s_threshold3" , "7.0",0);
	s_lip_threshold_4 = Cvar_Get("s_threshold4" , "8.0",0);

	s_language = Cvar_Get("s_language","english",CVAR_ARCHIVE | CVAR_NORESTART);

	MP3_InitCvars();

	// cv = Cvar_Get ("s_initsound", "1", CVAR_ROM);
	cv = Cvar_Get ("s_initsound", "1", CVAR_ROM);
	if ( !cv->integer ) {
		s_soundStarted = 0;	// needed in case you set s_initsound to 0 midgame then snd_restart (div0 err otherwise later)
		Com_Printf ("not initializing.\n");
		Com_Printf("------------------------------------\n");
		return;
	}

	Cmd_AddCommand("play", S_Play_f);
	Cmd_AddCommand("music", S_Music_f);
	// Cmd_AddCommand("soundlist", S_SoundList_f);
	Cmd_AddCommand("soundinfo", S_SoundInfo_f);
	Cmd_AddCommand("soundstop", S_StopAllSounds);
	//Cmd_AddCommand("mp3_calcvols", S_MP3_CalcVols_f);
	Cmd_AddCommand("s_dynamic", S_SetDynamicMusic_f);

	r = SNDDMA_Init();

	if ( r ) {
		s_soundStarted = 1;
		s_soundMuted = 1;
//		s_numSfx = 0;	// do NOT do this here now!!!

		s_soundtime = 0;
		s_paintedtime = 0;

		S_StopAllSounds ();

		S_SoundInfo_f();
	}

	Com_Printf("\n--- ambient sound initialization ---\n");

	AS_Init();
}

// only called from snd_restart. QA request...
//
void S_ReloadAllUsedSounds(void)
{
	
}

// =======================================================================
// Shutdown sound engine
// =======================================================================

void S_Shutdown( void )
{
	Cmd_RemoveCommand("play");
	Cmd_RemoveCommand("music");
	Cmd_RemoveCommand("stopsound");
	Cmd_RemoveCommand("soundlist");
	Cmd_RemoveCommand("soundinfo");
	Cmd_RemoveCommand("mp3_calcvols");
	Cmd_RemoveCommand("s_dynamic");
	AS_Free();
}



/*
	Mutes / Unmutes all OpenAL sound
*/
void S_AL_MuteAllSounds(qboolean bMute)
{
     
}


// =======================================================================
// Load a sound
// =======================================================================
/*
================
return a hash value for the sfx name
================
*/
static long S_HashSFXName(const char *name) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (name[i] != '\0') {
		letter = tolower(name[i]);
		if (letter =='.') break;				// don't include extension
		if (letter =='\\') letter = '/';		// damn path names
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash &= (LOOP_HASH-1);
	return hash;
}

/*
==================
S_FindName

Will allocate a new sfx if it isn't found
==================
*/
sfx_t *S_FindName( const char *name ) {
	int		i;
	int		hash;

	sfx_t	*sfx;

	if (!name) {
		Com_Error (ERR_FATAL, "S_FindName: NULL\n");
	}
	if (!name[0]) {
		Com_Error (ERR_FATAL, "S_FindName: empty name\n");
	}

	if (strlen(name) >= MAX_QPATH) {
		Com_Error (ERR_FATAL, "Sound name too long: %s", name);
	}

	char sSoundNameNoExt[MAX_QPATH];
	COM_StripExtension(name,sSoundNameNoExt);

	hash = S_HashSFXName(sSoundNameNoExt);

	sfx = sfxHash[hash];
	// see if already loaded
	while (sfx) {
		if (!Q_stricmp(sfx->sSoundName, sSoundNameNoExt) ) {
			return sfx;
		}
		sfx = sfx->next;
	}
/*
	// find a free sfx
	for (i=0 ; i < s_numSfx ; i++) {
		if (!s_knownSfx[i].soundName[0]) {
			break;
		}
	}
*/
	i = s_numSfx;	//we don't clear the soundName after failed loads any more, so it'll always be the last entry
		
	if (s_numSfx == MAX_SFX) 
	{
		// ok, no sfx's free, but are there any with defaultSound set? (which the registering ent will never
		//	see because he gets zero returned if it's default...)
		//
		for (i=0 ; i < s_numSfx ; i++) {
			if (s_knownSfx[i].bDefaultSound) {
				break;
			}
		}

		if (i==s_numSfx)
		{	  
			// genuinely out of handles...

			// if we ever reach this, let me know and I'll either boost the array or put in a map-used-on
			//	reference to enable sfx_t recycling. TA codebase relies on being able to have structs for every sound
			//	used anywhere, ever, all at once (though audio bit-buffer gets recycled). SOF1 used about 1900 distinct
			//	events, so current MAX_SFX limit should do, or only need a small boost...	-ste
			//

			Com_Error (ERR_FATAL, "S_FindName: out of sfx_t");
		}
	}
	else
	{
		s_numSfx++;
	}
	
	sfx = &s_knownSfx[i];
	memset (sfx, 0, sizeof(*sfx));
	Q_strncpyz(sfx->sSoundName, sSoundNameNoExt, sizeof(sfx->sSoundName));

	sfx->next = sfxHash[hash];
	sfxHash[hash] = sfx;

	return sfx;
}

/*
=================
S_DefaultSound
=================
*/
void S_DefaultSound( sfx_t *sfx ) {
	
	int		i;

	sfx->iSoundLengthInSamples	= 512;								// #samples, ie shorts
	sfx->pSoundData				= (short *)	SND_malloc(512*2, sfx);	// ... so *2 for alloc bytes	
	sfx->bInMemory				= qtrue;
	
	for ( i=0 ; i < sfx->iSoundLengthInSamples ; i++ ) 
	{
		sfx->pSoundData[i] = i;
	}
}


/*
===================
S_DisableSounds

Disables sounds until the next S_BeginRegistration.
This is called when the hunk is cleared and the sounds
are no longer valid.
===================
*/
void S_DisableSounds( void ) {
	S_StopAllSounds();
}

/*
=====================
S_BeginRegistration

=====================
*/
void S_BeginRegistration( void )
{
	if (s_numSfx == 0) {
		SND_setup();

		s_numSfx = 0;
		memset( s_knownSfx, 0, sizeof( s_knownSfx ) );
		memset(sfxHash, 0, sizeof(sfx_t *)*LOOP_HASH);

#ifdef _DEBUG
		sfx_t *sfx = S_FindName( "***DEFAULT***" );
		S_DefaultSound( sfx );
#else
		S_RegisterSound("sound/null.wav");
#endif
	}
}


void EALFileInit(char *level)
{
	
}



/*
==================
S_RegisterSound

Creates a default buzz sound if the file can't be loaded
==================
*/
sfxHandle_t	S_RegisterSound( const char *name) 
{
	sfx_t	*sfx;

	if (!s_soundStarted) {
		return 0;
	}

	if ( strlen( name ) >= MAX_QPATH ) {		
		Com_Printf( S_COLOR_RED"Sound name exceeds MAX_QPATH - %s\n", name );
		return 0;
	}

	sfx = S_FindName( name );

	SND_TouchSFX(sfx);

	if ( sfx->bDefaultSound )
		return 0;

	
	if ( sfx->pSoundData )
	{
		return sfx - s_knownSfx;
	}

	sfx->bInMemory = qfalse;

	S_memoryLoad(sfx);

	if ( sfx->bDefaultSound ) {
#ifndef FINAL_BUILD
		Com_Printf( S_COLOR_YELLOW "WARNING: could not find %s - using default\n", sfx->sSoundName );
#endif


		return 0;
	}

	return sfx - s_knownSfx;
}

void S_memoryLoad(sfx_t	*sfx) 
{
	// load the sound file...
	//
	if ( !S_LoadSound( sfx ) ) 
	{
		//		Com_Printf( S_COLOR_YELLOW "WARNING: couldn't load sound: %s\n", sfx->sSoundName );
		sfx->bDefaultSound = qtrue;
	}
	sfx->bInMemory = qtrue;
}

// instead of clearing a whole channel_t struct, we're going to skip the MP3SlidingDecodeBuffer[] buffer in the middle...
//
static inline void Channel_Clear(channel_t *ch)
{
	// memset (ch, 0, sizeof(*ch));

	memset(ch,0,offsetof(channel_t,MP3SlidingDecodeBuffer));

	byte *const p = (byte *)ch + offsetof(channel_t,MP3SlidingDecodeBuffer) + sizeof(ch->MP3SlidingDecodeBuffer);

	memset(p,0,(sizeof(*ch) - offsetof(channel_t,MP3SlidingDecodeBuffer)) - sizeof(ch->MP3SlidingDecodeBuffer));
}

//=============================================================================
static qboolean S_CheckChannelStomp( int chan1, int chan2 )
{	
	if ( chan1 == chan2 )
	{
		return qtrue;
	}

	if ( ( chan1 == CHAN_VOICE || chan1 == CHAN_VOICE_ATTEN || chan1 == CHAN_VOICE_GLOBAL  ) && ( chan2 == CHAN_VOICE || chan2 == CHAN_VOICE_ATTEN || chan2 == CHAN_VOICE_GLOBAL ) )
	{
		return qtrue;
	}

	return qfalse;
}


/*
=================
S_PickChannel
=================
*/
// there were 2 versions of this, one for A3D and one normal, but the normal one wouldn't compile because
//	it hadn't been updated for some time, so rather than risk anything weird/out of date, I just removed the 
//	A3D lines from this version and deleted the other one. 
//
// If this really bothers you then feel free to play with it. -Ste.
//
channel_t *S_PickChannel(int entnum, int entchannel)
{
	int			ch_idx;
	channel_t	*ch, *firstToDie;
	qboolean	foundChan = qfalse;

	if ( entchannel<0 ) {
		Com_Error (ERR_DROP, "S_PickChannel: entchannel<0");
	}

	// Check for replacement sound, or find the best one to replace

    firstToDie = &s_channels[0];

	for ( int pass = 0; (pass < ((entchannel == CHAN_AUTO || entchannel == CHAN_LESS_ATTEN)?1:2)) && !foundChan; pass++ )
	{
		for (ch_idx = 0, ch = &s_channels[0]; ch_idx < MAX_CHANNELS ; ch_idx++, ch++ ) 
		{
			if ( entchannel == CHAN_AUTO || entchannel == CHAN_LESS_ATTEN || pass > 0 )
			{//if we're on the second pass, just find the first open chan
				if ( !ch->thesfx )
				{//grab the first open channel
					firstToDie = ch;
					break;
				}

			}
			else if ( ch->entnum == entnum && S_CheckChannelStomp( ch->entchannel, entchannel ) ) 
			{
				// always override sound from same entity
				if ( s_show->integer == 1 && ch->thesfx ) {
					Com_Printf( S_COLOR_YELLOW"...overrides %s\n", ch->thesfx->sSoundName );
					ch->thesfx = 0;	//just to clear the next error msg
				}
				firstToDie = ch;
				foundChan = qtrue;
				break;
			}

			// don't let anything else override local player sounds
			if ( ch->entnum == listener_number 	&& entnum != listener_number && ch->thesfx) {
				continue;
			}

			// don't override loop sounds
			if ( ch->loopSound ) {
				continue;
			}

			if ( ch->startSample < firstToDie->startSample ) {
				firstToDie = ch;
			}
		}
	}

	if ( s_show->integer == 1 && firstToDie->thesfx ) {
		Com_Printf( S_COLOR_RED"***kicking %s\n", firstToDie->thesfx->sSoundName );
	}

	Channel_Clear(firstToDie);	// memset(firstToDie, 0, sizeof(*firstToDie));
    
	return firstToDie;
}



/*
	For use with Open AL

	Allows more than one sound of the same type to emanate from the same entity - sounds much better
	on hardware this way esp. rapid fire modes of weapons!
*/
channel_t *S_OpenALPickChannel(int entnum, int entchannel)
{	
    return NULL;
}


/*
=================
S_SpatializeOrigin

Used for spatializing s_channels
=================
*/
void S_SpatializeOrigin (const vec3_t origin, float master_vol, int *left_vol, int *right_vol, int channel)
{
    
}


// =======================================================================
// Start a sound effect
// =======================================================================

/*
====================
S_StartAmbientSound

Starts an ambient, 'one-shot" sound.
====================
*/

void S_StartAmbientSound( const vec3_t origin, int entityNum, unsigned char volume, sfxHandle_t sfxHandle )
{
	
}

/*
====================
S_StartSound

Validates the parms and ques the sound up
if pos is NULL, the sound will be dynamically sourced from the entity
Entchannel 0 will never override a playing sound
====================
*/
void S_StartSound(const vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle ) 
{
	
}

/*
==================
S_StartLocalSound
==================
*/
void S_StartLocalSound( sfxHandle_t sfxHandle, int channelNum ) {

}


/*
==================
S_StartLocalLoopingSound
==================
*/
void S_StartLocalLoopingSound( sfxHandle_t sfxHandle) {


}

// returns length in milliseconds of supplied sound effect...  (else 0 for bad handle now)
//
float S_GetSampleLengthInMilliSeconds( sfxHandle_t sfxHandle)
{
	return 512 * 1000;
}


/*
==================
S_ClearSoundBuffer

If we are about to perform file access, clear the buffer
so sound doesn't stutter.
==================
*/
void S_ClearSoundBuffer( void ) {
}


// kinda kludgy way to stop a special-use sfx_t playing...
//
void S_CIN_StopSound(sfxHandle_t sfxHandle)
{
}


/*
==================
S_StopAllSounds
==================
*/
void S_StopSounds(void)
{
}

/*
==================
S_StopAllSounds
 and music
==================
*/
void S_StopAllSounds(void) {

}

/*
==============================================================

continuous looping sounds are added each frame

==============================================================
*/

/*
==================
S_ClearLoopingSounds

==================
*/
void S_ClearLoopingSounds( void )
{
	

}

/*
==================
S_AddLoopingSound

Called during entity generation for a frame
Include velocity in case I get around to doing doppler...
==================
*/
void S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfxHandle ) {
	
}


/*
==================
S_AddAmbientLoopingSound
==================
*/
void S_AddAmbientLoopingSound( const vec3_t origin, unsigned char volume, sfxHandle_t sfxHandle ) 
{
	
}



/*
==================
S_AddLoopSounds

Spatialize all of the looping sounds.
All sounds are on the same cycle, so any duplicates can just
sum up the channel multipliers.
==================
*/
void S_AddLoopSounds (void) 
{
	
}

//=============================================================================

/*
=================
S_ByteSwapRawSamples

If raw data has been loaded in little endien binary form, this must be done.
If raw data was calculated, as with ADPCM, this should not be called.
=================
*/
void S_ByteSwapRawSamples( int samples, int width, int s_channels, const byte *data ) {
	int		i;

	if ( width != 2 ) {
		return;
	}
	if ( LittleShort( 256 ) == 256 ) {
		return;
	}

	if ( s_channels == 2 ) {
		samples <<= 1;
	}
	for ( i = 0 ; i < samples ; i++ ) {
		((short *)data)[i] = LittleShort( ((short *)data)[i] );
	}
}


portable_samplepair_t *S_GetRawSamplePointer() {
	return s_rawsamples;
}


/*
============
S_RawSamples

Music streaming
============
*/
void S_RawSamples( int samples, int rate, int width, int s_channels, const byte *data, float volume, qboolean bFirstOrOnlyUpdateThisFrame )
{
	
}

//=============================================================================

/*
=====================
S_UpdateEntityPosition

let the sound system know where an entity currently is
======================
*/
void S_UpdateEntityPosition( int entityNum, const vec3_t origin )
{
	
}

/*
============
S_Respatialize

Change the volumes of all the playing sounds for changes in their positions
============
*/
void S_Respatialize( int entityNum, const vec3_t head, vec3_t axis[3], qboolean inwater )
{
}


/*
========================
S_ScanChannelStarts

Returns qtrue if any new sounds were started since the last mix
========================
*/
qboolean S_ScanChannelStarts( void ) {
	channel_t		*ch;
	int				i;
	qboolean		newSamples;

	return newSamples;
}

// this is now called AFTER the DMA painting, since it's only the painter calls that cause the MP3s to be unpacked,
//	and therefore to have data readable by the lip-sync volume calc code.
//
void S_DoLipSynchs( const unsigned s_oldpaintedtime )
{
	
}

/*
============
S_Update

Called once each time through the main loop
============
*/
void S_Update( void ) {
	
}

void S_GetSoundtime(void)
{
}

void S_Update_(void) {
	
}


void UpdateSingleShotSounds()
{
	
}




void UpdateLoopingSounds()
{
	
}


void AL_UpdateRawSamples()
{
	
}


int S_MP3PreProcessLipSync(channel_t *ch, short *data)
{
	return 1;
}


void S_SetLipSyncs()
{
	
}


void S_SoundList_f( void ) {
	
}


/*
===============================================================================

background music functions

===============================================================================
*/

int	FGetLittleLong( fileHandle_t f ) {
	int		v;

	FS_Read( &v, sizeof(v), f );

	return LittleLong( v);
}

int	FGetLittleShort( fileHandle_t f ) {
	short	v;

	FS_Read( &v, sizeof(v), f );

	return LittleShort( v);
}

// returns the length of the data in the chunk, or 0 if not found
int S_FindWavChunk( fileHandle_t f, char *chunk ) {
	char	name[5];
	int		len;
	int		r;

	name[4] = 0;
	len = 0;
	r = FS_Read( name, 4, f );
	if ( r != 4 ) {
		return 0;
	}
	len = FGetLittleLong( f );
	if ( len < 0 || len > 0xfffffff ) {
		len = 0;
		return 0;
	}
	len = (len + 1 ) & ~1;		// pad to word boundary
//	s_nextWavChunk += len + 8;

	if ( strcmp( name, chunk ) ) {
		return 0;
	}
	return len;
}

// fixme: need to move this into qcommon sometime?, but too much stuff altered by other people and I won't be able
//	to compile again for ages if I check that out...
//
// DO NOT replace this with a call to FS_FileExists, that's for checking about writing out, and doesn't work for this.
//
qboolean S_FileExists( const char *psFilename )
{
	fileHandle_t fhTemp;

	FS_FOpenFileRead (psFilename, &fhTemp, qtrue);	// qtrue so I can fclose the handle without closing a PAK
	if (!fhTemp) 
		return qfalse;
	
	FS_FCloseFile(fhTemp);
	return qtrue;
}

// called only by snd_shutdown (from snd_restart or app exit)
//
void S_UnCacheDynamicMusic( void )
{

}

static char gsIntroMusic[MAX_QPATH]={0};
static char gsLoopMusic [MAX_QPATH]={0};

void S_RestartMusic( void ) 
{

}

void S_StopBackgroundTrack( void )
{

	s_rawend = 0;
}



// used to be just for dynamic, but now even non-dynamic music has to know whether it should be silent or not...
//
static LPCSTR S_Music_GetRequestedState(void)
{
	return NULL;
}
cvar_t *s_soundpoolmegs = NULL;


// currently passing in sfx as a param in case I want to do something with it later.
//
byte *SND_malloc(int iSize, sfx_t *sfx) 
{
	byte *pData = (byte *) Z_Malloc(iSize, TAG_SND_RAWDATA, qfalse);	// don't bother asking for zeroed mem
	
	return pData;
}


// called once-only in EXE lifetime...
//
void SND_setup() 
{	
	Com_Printf("Sound memory manager started\n");
}


// ask how much mem an sfx has allocated...
//
static int SND_MemUsed(sfx_t *sfx)
{
	int iSize = 0;

	return iSize;
}

// free any allocated sfx mem...
//
// now returns # bytes freed to help with z_malloc()-fail recovery
//
static int SND_FreeSFXMem(sfx_t *sfx)
{
	int iBytesFreed = 0;
	return iBytesFreed;
}

void S_DisplayFreeMemory() 
{
	
}

void SND_TouchSFX(sfx_t *sfx)
{
	sfx->iLastTimeUsed		= Com_Milliseconds()+1;
	sfx->iLastLevelUsedOn	= RE_RegisterMedia_GetLevel();
}


// currently this is only called during snd_shutdown or snd_restart
//
void S_FreeAllSFXMem(void)
{

}

// returns number of bytes freed up...
//
// new param is so we can be usre of not freeing ourselves (without having to rely on possible uninitialised timers etc)
//
int SND_FreeOldestSound(sfx_t *pButNotThisOne /* = NULL */) 
{	
	int iBytesFreed = 0;

	return iBytesFreed;
}
int SND_FreeOldestSound(void)
{
	return SND_FreeOldestSound(NULL);	// I had to add a void-arg version of this because of link issues, sigh
}


// just before we drop into a level, ensure the audio pool is under whatever the maximum
//	pool size is (but not by dropping out sounds used by the current level)...
//
// returns qtrue if at least one sound was dropped out, so z_malloc-fail recovery code knows if anything changed
//
extern qboolean gbInsideLoadSound;
qboolean SND_RegisterAudio_LevelLoadEnd(qboolean bDeleteEverythingNotUsedThisLevel /* 99% qfalse */)
{
	qboolean bAtLeastOneSoundDropped = qfalse;

	Com_DPrintf( "SND_RegisterAudio_LevelLoadEnd():\n");
	
	return bAtLeastOneSoundDropped;
}


void S_StartBackgroundTrack( const char *intro, const char *loop, qboolean bCalledByCGameStart ) {

}
