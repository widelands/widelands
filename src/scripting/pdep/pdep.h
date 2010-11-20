#ifndef PDEP_H
#define PDEP_H

#include "llimits.h"

#include <cstddef>

#define pdep_reallocv(L, b, on, n, e) pdep_realloc_(L, (b), (on)*(e), (n)*(e))
#define pdep_reallocvector(L, v, oldn, n, t)                                  \
   ((v) = cast(t *, pdep_reallocv(L, v, oldn, n, sizeof(t))))
#define pdep_newvector(L,n,t)                                                 \
   cast(t *, pdep_reallocv(L, 0, 0, n, sizeof(t)))
#define pdep_new(L, t) cast(t *, pdep_malloc(L, sizeof(t)))
#define pdep_malloc(L, t) pdep_realloc_(L, 0, 0, (t))

union Closure;
union GCObject;
struct Proto;
struct Table;
union TString;
struct TValue;

void pdep_pushobject (lua_State *, TValue const *);
void * pdep_realloc_ (lua_State *, void * block, size_t osize, size_t nsize);
void pdep_link (lua_State *, GCObject *, lu_byte tt);
Proto * pdep_newproto (lua_State *);
Closure * pdep_newLclosure (lua_State *, int nelems, Table *);
void pdep_reallocstack (lua_State *, int newsize);
void pdep_growstack (lua_State *, int);
void pdep_reallocCI (lua_State *, int newsize);
TString * pdep_newlstr (lua_State *, char const *, size_t);

#endif
