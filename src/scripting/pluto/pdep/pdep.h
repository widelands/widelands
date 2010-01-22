#ifndef PDEP_H
#define PDEP_H

#include <lua.hpp>

#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "llimits.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstate.h"
#include "lstring.h"
#include "lauxlib.h"


#define pdep_reallocv(L,b,on,n,e) \
	pdep_realloc_(L, (b), (on)*(e), (n)*(e))
#define pdep_reallocvector(L, v,oldn,n,t) \
	((v)=cast(t *, pdep_reallocv(L, v, oldn, n, sizeof(t))))
#define pdep_freearray(L, b, n, t)   pdep_reallocv(L, (b), n, 0, sizeof(t))
#define pdep_newvector(L,n,t) \
	cast(t *, pdep_reallocv(L, NULL, 0, n, sizeof(t)))
#define pdep_new(L,t)		cast(t *, pdep_malloc(L, sizeof(t)))
#define pdep_malloc(L,t)	pdep_realloc_(L, NULL, 0, (t))
#define pdep_checkstack(L,n)	\
  if ((char *)L->stack_last - (char *)L->top <= (n)*(int)sizeof(TValue)) \
    pdep_growstack(L, n); \
  else pdep_reallocstack(L, L->stacksize - EXTRA_STACK - 1);


void pdep_pushobject (lua_State *L, const TValue *o);
void *pdep_realloc_ (lua_State *L, void *block, size_t osize, size_t nsize);
void pdep_link (lua_State *L, GCObject *o, lu_byte tt);
Proto *pdep_newproto (lua_State *L);
Closure *pdep_newLclosure (lua_State *L, int nelems, Table *e);
void pdep_reallocstack (lua_State *L, int newsize);
void pdep_growstack (lua_State *L, int n);
void pdep_reallocCI (lua_State *L, int newsize);
TString *pdep_newlstr (lua_State *L, const char *str, size_t l);

#endif
