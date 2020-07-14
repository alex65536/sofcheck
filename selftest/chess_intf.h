#ifndef CHESS_INTF_INCLUDED
#define CHESS_INTF_INCLUDED

#ifdef INTF_DODECAHEDRON
#ifdef INTF_USED
#error Interface is already included!
#endif
#define INTF_USED
#include "dodecahedron/intf.h"
#endif

#ifdef INTF_SEAGULL
#ifdef INTF_USED
#error Interface is already included!
#endif
#define INTF_USED
#include "seagull/intf.h"
#endif

#ifdef INTF_SOFCHECK
#ifdef INTF_USED
#error Interface is already included!
#endif
#define INTF_USED
#include "sofcheck/intf.h"
#endif

#ifndef INTF_USED
#error No interface included!
#endif

#endif
