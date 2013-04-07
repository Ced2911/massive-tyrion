#pragma once
#if !defined(G_HEADERS_H_INC)
#define G_HEADERS_H_INC

#if !defined(G_LOCAL_H_INC)
	#include "../game/g_local.h"
#endif

//#if !defined(G_WRAITH_H_INC)
//	#include "../game/g_Wraith.h"
//#endif

#if !defined(TEAMS_H_INC)
	#include "../game/Teams.h"
#endif

//#if !defined(IGINTERFACE_H_INC)
//	#include "../game/IGInterface.h"
//#endif

#include "../cgame/cg_local.h"	// yeah I know this is naughty, but we're shipping soon...
#include "../game/g_nav.h"
#include "../game/g_functions.h"
#include "../game/g_shared.h"
#include "../game/b_local.h"

#endif // G_HEADERS_H_INC