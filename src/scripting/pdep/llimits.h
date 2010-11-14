/*
** $Id: llimits.h,v 1.69.1.1 2007/12/27 13:02:25 roberto Exp $
** Limits, basic types, and some other `installation-dependent' definitions
** See Copyright Notice in lua.h
*/

#ifndef LLIMITS_H
#define LLIMITS_H

#include <lua.hpp>

#include <stdint.h>

typedef LUAI_UMEM lu_mem;

typedef LUAI_MEM l_mem;



/* chars used as small naturals (so that `char' is reserved for characters) */
typedef unsigned char lu_byte;


/* type to ensure maximum alignment */
typedef LUAI_USER_ALIGNMENT_T L_Umaxalign;


/* internal assertions for in-house debugging */
#ifdef lua_assert

#define check_exp(c, e) (lua_assert(c), (e))
#define api_check(l, e)  lua_assert(e)

#else

#define lua_assert(c) (static_cast<void>(0))
#define check_exp(c, e) (e)

#endif

#ifndef cast
#define cast(t, exp) (static_cast<t>(exp))
#endif

#define cast_byte(i) cast(lu_byte, (i))

/*
** type for virtual-machine instructions
** must be an unsigned with (at least) 4 bytes (see details in lopcodes.h)
*/
typedef uint32_t Instruction;

#endif
