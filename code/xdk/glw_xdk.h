#ifndef _WIN32
#  error You should not be including this file on this platform
#endif

#ifndef __GLW_WIN_H__
#define __GLW_WIN_H__

typedef struct
{
	qboolean allowdisplaydepthchange;
	qboolean pixelFormatSet;

	int		 desktopBitsPixel;
	int		 desktopWidth, desktopHeight;

	qboolean	cdsFullscreen;

	FILE *log_fp;
} glwstate_t;

extern glwstate_t glw_state;

#endif
