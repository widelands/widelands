/* $Id$ */

/* Pluto - Heavy-duty persistence for Lua
 * Copyright (C) 2004 by Ben Sunshine-Hill, and released into the public
 * domain. People making use of this software as part of an application
 * are politely requested to email the author at sneftel@gmail.com
 * with a brief description of the application, primarily to satisfy his
 * curiosity.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <lua.hpp>

#include "wexception.h"
#include "log.h"
#include "logic/widelands_filewrite.h"
#include "logic/widelands_fileread.h"

// TODO: either throw exception or lua_error

#include "pluto.h"

#include "pdep/pdep.h"
#define LIF(prefix, name) pdep ## _ ## name

#include <cstring>
#include <stdint.h>	/*for intptr_t*/


// #define PLUTO_DEBUG

// Forward declarated from lua_impl.h. So we do not need to include it
int luna_restore_object(lua_State * L);



#ifdef PLUTO_DEBUG
#include <stdio.h>
#endif

#define PLUTO_TPERMANENT 101

#define verify(x) { int v = (int)((x)); v=v; lua_assert(v); }

typedef struct PersistInfo_t {
	lua_State *L;
	int counter;
	lua_Chunkwriter writer;
	void *ud;
	Widelands::FileWrite * fw;
#ifdef PLUTO_DEBUG
	int level;
#endif
} PersistInfo;

typedef struct UnpersistInfo_t {
	lua_State *L;
	Widelands::FileRead * fr;
#ifdef PLUTO_DEBUG
	int level;
#endif
} UnpersistInfo;

static void unpersist(UnpersistInfo *upi);


#ifdef PLUTO_DEBUG
void printindent(int indent)
{
	int il;
	for(il=0; il<indent; il++) {
		printf("  ");
	}
}
#endif


/*
 * ========================================================================
 *                         HELPER FUNCTIONS
 * ========================================================================
 */

/* The object is left on the stack. This is primarily used by unpersist, but
 * may be used by GCed objects that may incur cycles in order to preregister
 * the object. */
static void registerobject(int ref, UnpersistInfo *upi)
{
					/* perms reftbl ... obj */
	lua_checkstack(upi->L, 2);
	lua_pushlightuserdata(upi->L, (void*)(intptr_t)ref);
					/* perms reftbl ... obj ref */
	lua_pushvalue(upi->L, -2);
					/* perms reftbl ... obj ref obj */
	lua_settable(upi->L, 2);
					/* perms reftbl ... obj */
}

/* Mutual recursion requires prototype */
static void persist(PersistInfo *pi);

/* A simple reimplementation of the unfortunately static function luaA_index.
 * Does not support the global table, registry, or upvalues. */
static StkId getobject(lua_State *L, int stackpos)
{
	if(stackpos > 0) {
		lua_assert(L->base+stackpos-1 < L->top);
		return L->base+stackpos-1;
	} else {
		lua_assert(L->top-stackpos >= L->base);
		return L->top+stackpos;
	}
}

/* Choose whether to do a regular or special persistence based on an object's
 * metatable. "default" is whether the object, if it doesn't have a __persist
 * entry, is literally persistable or not.
 * Returns true if special persistence is used. */
static int persistspecialobject(PersistInfo *pi, int defaction)
{
					/* perms reftbl ... obj */
	lua_checkstack(pi->L, 4);
	/* Check whether we should persist literally, or via the __persist
	 * metafunction */
	if(!lua_getmetatable(pi->L, -1)) {
		if(defaction) {
			pi->fw->Unsigned8(0);
			return 0;
		} else {
			lua_pushstring(pi->L, "Type not literally persistable by default");
			lua_error(pi->L);
		}
	}
					/* perms reftbl sptbl ... obj mt */
	lua_pushstring(pi->L, "__persist");
					/* perms reftbl sptbl ... obj mt "__persist" */
	lua_rawget(pi->L, -2);
					/* perms reftbl sptbl ... obj mt __persist? */
	if(lua_isnil(pi->L, -1)) {
					/* perms reftbl sptbl ... obj mt nil */
		lua_pop(pi->L, 2);
					/* perms reftbl sptbl ... obj */
		if(defaction) {
			pi->fw->Unsigned8(0);
			return 0;
		} else {
			lua_pushstring(pi->L, "Type not literally persistable by default");
			lua_error(pi->L);
			return 0; /* not reached */
		}
	} else if(lua_isboolean(pi->L, -1)) {
					/* perms reftbl sptbl ... obj mt bool */
		if(lua_toboolean(pi->L, -1)) {
					/* perms reftbl sptbl ... obj mt true */
			lua_pop(pi->L, 2);
					/* perms reftbl sptbl ... obj */
			pi->fw->Unsigned8(0);
			return 0;
		} else {
			lua_pushstring(pi->L, "Metatable forbade persistence");
			lua_error(pi->L);
			return 0; /* not reached */
		}
	} else if(!lua_isfunction(pi->L, -1)) {
		lua_pushstring(pi->L, "__persist not nil, boolean, or function");
		lua_error(pi->L);
	}
					/* perms reftbl ... obj mt __persist */
	lua_pushvalue(pi->L, -3);
					/* perms reftbl ... obj mt __persist obj */
	lua_call(pi->L, 1, 1);
					/* perms reftbl ... obj mt table? */
	// The __persist function returned a table which contains the data
	// the __unpersist function needs to recreate the state
	if(!lua_istable(pi->L, -1)) {
		lua_pushstring(pi->L, "__persist function did not return a table");
		lua_error(pi->L);
	}
					/* perms reftbl ... obj mt table */

	// It is indeed a special table
	pi->fw->Unsigned8(1);
	persist(pi);

					/* perms reftbl ... obj mt table */
	lua_pop(pi->L, 2);
					/* perms reftbl ... obj */
	return 1;
}

static void persisttable(PersistInfo *pi)
{
					/* perms reftbl ... tbl */
	lua_checkstack(pi->L, 3);
	if(persistspecialobject(pi, 1)) {
					/* perms reftbl ... tbl */
		return;
	}
					/* perms reftbl ... tbl */
	/* First, persist the metatable (if any) */
	if(!lua_getmetatable(pi->L, -1)) {
		lua_pushnil(pi->L);
	}
					/* perms reftbl ... tbl mt/nil */
	persist(pi);
	lua_pop(pi->L, 1);
					/* perms reftbl ... tbl */

	/* Now, persist all k/v pairs */
	lua_pushnil(pi->L);
					/* perms reftbl ... tbl nil */
	while(lua_next(pi->L, -2)) {
					/* perms reftbl ... tbl k v */
		lua_pushvalue(pi->L, -2);
					/* perms reftbl ... tbl k v k */
		persist(pi);
		lua_pop(pi->L, 1);
					/* perms reftbl ... tbl k v */
		persist(pi);
		lua_pop(pi->L, 1);
					/* perms reftbl ... tbl k */
	}
					/* perms reftbl ... tbl */
	/* Terminate list */
	lua_pushnil(pi->L);
					/* perms reftbl ... tbl nil */
	persist(pi);
	lua_pop(pi->L, 1);
					/* perms reftbl ... tbl */
}

static void unpersistspecialtable(int, UnpersistInfo *upi)
{
					/* perms reftbl ... */
	lua_checkstack(upi->L, 1);
	unpersist(upi);
					/* perms reftbl ... spfunc? */
	lua_assert(lua_istable(upi->L, -1));
					/* perms reftbl ... spfunc */

   luna_restore_object(upi->L);

					/* perms reftbl ... tbl? */
	lua_assert(lua_istable(upi->L, -1));
					/* perms reftbl ... tbl */
}

static void unpersistliteraltable(int ref, UnpersistInfo *upi)
{
					/* perms reftbl ... */
	lua_checkstack(upi->L, 3);
	/* Preregister table for handling of cycles */
	lua_newtable(upi->L);
					/* perms reftbl ... tbl */
	registerobject(ref, upi);
					/* perms reftbl ... tbl */
	/* Unpersist metatable */
	{
		unpersist(upi);
					/* perms reftbl ... tbl mt/nil? */
		if(lua_istable(upi->L, -1)) {
					/* perms reftbl ... tbl mt */
			lua_setmetatable(upi->L, -2);
					/* perms reftbl ... tbl */
		} else {
					/* perms reftbl ... tbl nil? */
			lua_assert(lua_isnil(upi->L, -1));
					/* perms reftbl ... tbl nil */
			lua_pop(upi->L, 1);
					/* perms reftbl ... tbl */
		}
					/* perms reftbl ... tbl */
	}

	while(1)
	{
					/* perms reftbl ... tbl */
		unpersist(upi);
					/* perms reftbl ... tbl key/nil */
		if(lua_isnil(upi->L, -1)) {
					/* perms reftbl ... tbl nil */
			lua_pop(upi->L, 1);
					/* perms reftbl ... tbl */
			break;
		}
					/* perms reftbl ... tbl key */
		unpersist(upi);
					/* perms reftbl ... tbl key value? */
		lua_assert(!lua_isnil(upi->L, -1));
					/* perms reftbl ... tbl key value */
		lua_rawset(upi->L, -3);
					/* perms reftbl ... tbl */
	}
}


static void unpersisttable(int ref, UnpersistInfo *upi)
{
					/* perms reftbl ... */
	lua_checkstack(upi->L, 1);
	{
		if(upi->fr->Unsigned8()) {
			unpersistspecialtable(ref, upi);
					/* perms reftbl ... tbl */
		} else {
			unpersistliteraltable(ref, upi);
					/* perms reftbl ... tbl */
		}
					/* perms reftbl ... tbl */
	}
}


static void persistuserdata(PersistInfo *pi) {
					/* perms reftbl ... udata */
	lua_checkstack(pi->L, 2);
	if(persistspecialobject(pi, 0)) {
					/* perms reftbl ... udata */
		return;
	} else {
		lua_pushstring(pi->L, "Attempt to persist a userdata without a "
				"__persist function. This is not possible!");
		lua_error(pi->L);
	}
}


static Proto *toproto(lua_State *L, int stackpos)
{
	return gco2p(getobject(L, stackpos)->value.gc);
}

static UpVal *toupval(lua_State *L, int stackpos)
{
	lua_assert(ttype(getobject(L, stackpos)) == LUA_TUPVAL);
	return gco2uv(getobject(L, stackpos)->value.gc);
}

static void pushproto(lua_State *L, Proto *proto)
{
	TValue o;
	setptvalue(L, &o, proto);
	pdep_pushobject(L, &o);
}

#define setuvvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TUPVAL; \
    checkliveness(G(L),i_o); }

static void pushupval(lua_State *L, UpVal *upval)
{
	TValue o;
	setuvvalue(L, &o, upval);
	pdep_pushobject(L, &o);
}

static void pushclosure(lua_State *L, Closure *closure)
{
	TValue o;
	setclvalue(L, &o, closure);
	pdep_pushobject(L, &o);
}

static void pushstring(lua_State *L, TString *s)
{
	TValue o;
	setsvalue(L, &o, s);
	pdep_pushobject(L, &o);
}

static void persistfunction(PersistInfo *pi)
{
					/* perms reftbl ... func */
	Closure *cl = clvalue(getobject(pi->L, -1));
	lua_checkstack(pi->L, 2);
	if(cl->c.isC) {
		/* It's a C function. For now, we aren't going to allow
		 * persistence of C closures, even if the "C proto" is
		 * already in the permanents table. */
		lua_pushstring(pi->L, "Attempt to persist a C function");
		lua_error(pi->L);
	} else {
		/* It's a Lua closure. */
		/* We don't really _NEED_ the number of upvals,
		 * but it'll simplify things a bit */
		pi->fw->Unsigned8(cl->l.p->nups);

		/* Persist prototype */
		{
			pushproto(pi->L, cl->l.p);
					/* perms reftbl ... func proto */
			persist(pi);
			lua_pop(pi->L, 1);
					/* perms reftbl ... func */
		}
		/* Persist upvalue values (not the upvalue objects
		 * themselves) */
		{
			int i;
			for(i=0; i<cl->l.p->nups; i++) {
					/* perms reftbl ... func */
				pushupval(pi->L, cl->l.upvals[i]);
					/* perms reftbl ... func upval */
				persist(pi);
				lua_pop(pi->L, 1);
					/* perms reftbl ... func */
			}
					/* perms reftbl ... func */
		}
		/* Persist function environment */
		{
			lua_getfenv(pi->L, -1);
					/* perms reftbl ... func fenv */
			if(lua_equal(pi->L, -1, LUA_GLOBALSINDEX)) {
				/* Function has the default fenv */
					/* perms reftbl ... func _G */
				lua_pop(pi->L, 1);
					/* perms reftbl ... func */
				lua_pushnil(pi->L);
					/* perms reftbl ... func nil */
			}
					/* perms reftbl ... func fenv/nil */
			persist(pi);
			lua_pop(pi->L, 1);
					/* perms reftbl ... func */
		}
	}
}

static UpVal *makeupval(lua_State *L, int stackpos)
{
	UpVal *uv = pdep_new(L, UpVal);
	pdep_link(L, (GCObject*)uv, LUA_TUPVAL);
	uv->tt = LUA_TUPVAL;
	uv->v = &uv->u.value;
	uv->u.l.prev = NULL;
	uv->u.l.next = NULL;
	setobj(L, uv->v, getobject(L, stackpos));
	return uv;
}

static Proto *makefakeproto(lua_State *L, lu_byte nups)
{
	Proto *p = pdep_newproto(L);
	p->sizelineinfo = 1;
	p->lineinfo = pdep_newvector(L, 1, int);
	p->lineinfo[0] = 1;
	p->sizecode = 1;
	p->code = pdep_newvector(L, 1, Instruction);
	p->code[0] = CREATE_ABC(OP_RETURN, 0, 1, 0);
	p->source = pdep_newlstr(L, "", 0);
	p->maxstacksize = 2;
	p->nups = nups;
	p->sizek = 0;
	p->sizep = 0;

	return p;
}

static void unboxupval(lua_State *L)
{
					/* ... func */
	LClosure *lcl;
	UpVal *uv;

	lcl = (LClosure*)clvalue(getobject(L, -1));
	uv = lcl->upvals[0];
	lua_pop(L, 1);
					/* ... */
	pushupval(L, uv);
					/* ... upval */
}

static void unpersistfunction(int ref, UnpersistInfo *upi)
{
					/* perms reftbl ... */
	LClosure *lcl;
	int i;
	lua_checkstack(upi->L, 2);

	lu_byte nupvalues = upi->fr->Unsigned8();

	lcl = (LClosure*)pdep_newLclosure(upi->L, nupvalues, hvalue(&upi->L->l_gt));
	pushclosure(upi->L, (Closure*)lcl);

					/* perms reftbl ... func */
	/* Put *some* proto in the closure, before the GC can find it */
	lcl->p = makefakeproto(upi->L, nupvalues);

	/* Also, we need to temporarily fill the upvalues */
	lua_pushnil(upi->L);
					/* perms reftbl ... func nil */
	for(i=0; i<nupvalues; i++) {
		lcl->upvals[i] = makeupval(upi->L, -1);
	}
	lua_pop(upi->L, 1);
					/* perms reftbl ... func */

	/* I can't see offhand how a function would ever get to be self-
	 * referential, but just in case let's register it early */
	registerobject(ref, upi);

	/* Now that it's safe, we can get the real proto */
	unpersist(upi);
					/* perms reftbl ... func proto? */
	lua_assert(lua_type(upi->L, -1) == LUA_TPROTO);
					/* perms reftbl ... func proto */
	lcl->p = toproto(upi->L, -1);
	lua_pop(upi->L, 1);
					/* perms reftbl ... func */

	for(i=0; i<nupvalues; i++) {
					/* perms reftbl ... func */
		unpersist(upi);
					/* perms reftbl ... func func2 */
		unboxupval(upi->L);
					/* perms reftbl ... func upval */
		lcl->upvals[i] = toupval(upi->L, -1);
		lua_pop(upi->L, 1);
					/* perms reftbl ... func */
	}
					/* perms reftbl ... func */

	/* Finally, the fenv */
	unpersist(upi);
					/* perms reftbl ... func fenv/nil? */
	lua_assert(lua_type(upi->L, -1) == LUA_TNIL ||
		lua_type(upi->L, -1) == LUA_TTABLE);
					/* perms reftbl ... func fenv/nil */
	if(!lua_isnil(upi->L, -1)) {
					/* perms reftbl ... func fenv */
		lua_setfenv(upi->L, -2);
					/* perms reftbl ... func */
	} else {
					/* perms reftbl ... func nil */
		lua_pop(upi->L, 1);
					/* perms reftbl ... func */
	}
					/* perms reftbl ... func */
}



/* Upvalues are tricky. Here's why.
 *
 * A particular upvalue may be either "open", in which case its member v
 * points into a thread's stack, or "closed" in which case it points to the
 * upvalue itself. An upvalue is closed under any of the following conditions:
 * -- The function that initially declared the variable "local" returns
 * -- The thread in which the closure was created is garbage collected
 *
 * To make things wackier, just because a thread is reachable by Lua doesn't
 * mean it's in our root set. We need to be able to treat an open upvalue
 * from an unreachable thread as a closed upvalue.
 *
 * The solution:
 * (a) For the purposes of persisting, don't indicate whether an upvalue is
 * closed or not.
 * (b) When unpersisting, pretend that all upvalues are closed.
 * (c) When persisting, persist all open upvalues referenced by a thread
 * that is persisted, and tag each one with the corresponding stack position
 * (d) When unpersisting, "reopen" each of these upvalues as the thread is
 * unpersisted
 */
static void persistupval(PersistInfo *pi)
{
					/* perms reftbl ... upval */
	UpVal *uv = toupval(pi->L, -1);
	lua_checkstack(pi->L, 1);

	/* We can't permit the upval to linger around on the stack, as Lua
	* will bail if its GC finds it. */

	lua_pop(pi->L, 1);
					/* perms reftbl ... */
	pdep_pushobject(pi->L, uv->v);
					/* perms reftbl ... obj */
	persist(pi);
					/* perms reftbl ... obj */
}

static void persistproto(PersistInfo *pi)
{
					/* perms reftbl ... proto */
	Proto *p = toproto(pi->L, -1);
	lua_checkstack(pi->L, 2);

	/* Persist constant refs */
	{
		int i;
		pi->fw->Signed32(p->sizek);
		for(i=0; i<p->sizek; i++) {
			pdep_pushobject(pi->L, &p->k[i]);
					/* perms reftbl ... proto const */
			persist(pi);
			lua_pop(pi->L, 1);
					/* perms reftbl ... proto */
		}
	}
					/* perms reftbl ... proto */

	/* serialize inner Proto refs */
	{
		pi->fw->Signed32(p->sizep);
		for(int i=0; i<p->sizep; i++)
		{
			pushproto(pi->L, p->p[i]);
					/* perms reftbl ... proto subproto */
			persist(pi);
			lua_pop(pi->L, 1);
					/* perms reftbl ... proto */
		}
	}
					/* perms reftbl ... proto */

	/* Serialize code */
	{
		compile_assert(sizeof(Instruction) == 4);
		pi->fw->Signed32(p->sizecode);
		for (int32_t i = 0; i < p->sizecode; i++)
			pi->fw->Unsigned32(p->code[i]);
	}

	/* Serialize upvalue names */
	{
		int i;
		pi->fw->Signed32(p->sizeupvalues);
		for(i=0; i<p->sizeupvalues; i++)
		{
			pushstring(pi->L, p->upvalues[i]);
			persist(pi);
			lua_pop(pi->L, 1);
		}
	}
	/* Serialize local variable infos */
	{
		int i;
		pi->fw->Signed32(p->sizelocvars);
		for(i=0; i<p->sizelocvars; i++)
		{
			pushstring(pi->L, p->locvars[i].varname);
			persist(pi);
			lua_pop(pi->L, 1);

			pi->fw->Signed32(p->locvars[i].startpc);
			pi->fw->Signed32(p->locvars[i].endpc);
		}
	}

	/* Serialize source string */
	pushstring(pi->L, p->source);
	persist(pi);
	lua_pop(pi->L, 1);

	/* Serialize line numbers */
	{
		pi->fw->Signed32(p->sizelineinfo);
		for(int32_t i = 0; i < p->sizelineinfo; i++)
			pi->fw->Signed32(p->lineinfo[i]);
	}

	/* Serialize linedefined and lastlinedefined */
	pi->fw->Signed32(p->linedefined);
	pi->fw->Signed32(p->lastlinedefined);

	/* Serialize misc values */
	{
		pi->fw->Unsigned8(p->nups);
		pi->fw->Unsigned8(p->numparams);
		pi->fw->Unsigned8(p->is_vararg);
		pi->fw->Unsigned8(p->maxstacksize);
	}
	/* We do not currently persist upvalue names, local variable names,
	 * variable lifetimes, line info, or source code. */
}

/* Copies a stack, but the stack is reversed in the process
 */
static size_t revappendstack(lua_State *from, lua_State *to)
{
	StkId o;
	for(o=from->top-1; o>=from->stack; o--) {
		setobj2s(to, to->top, o);
		to->top++;
	}
	return from->top - from->stack;
}

/* Persist all stack members
 */
static void persistthread(PersistInfo *pi)
{
	size_t posremaining;
	lua_State *L2;
					/* perms reftbl ... thr */
	L2 = lua_tothread(pi->L, -1);
	lua_checkstack(pi->L, L2->top - L2->stack + 1);
	if(pi->L == L2) {
		lua_pushstring(pi->L, "Can't persist currently running thread");
		lua_error(pi->L);
		return; /* not reached */
	}

	/* Persist the stack */
	posremaining = revappendstack(L2, pi->L);
					/* perms reftbl ... thr (rev'ed contents of L2) */
	pi->fw->Unsigned32(posremaining);
	for(; posremaining > 0; posremaining--) {
		persist(pi);
		lua_pop(pi->L, 1);
	}
					/* perms reftbl ... thr */
	/* Now, persist the CallInfo stack. */
	{
		size_t i, numframes = (L2->ci - L2->base_ci) + 1;
		pi->fw->Unsigned32(numframes);
		for(i=0; i<numframes; i++) {
			CallInfo *ci = L2->base_ci + i;
			size_t stackbase = ci->base - L2->stack;
			size_t stackfunc = ci->func - L2->stack;
			size_t stacktop = ci->top - L2->stack;
			size_t savedpc = (ci != L2->base_ci) ?
				ci->savedpc - ci_func(ci)->l.p->code :
				0;
			pi->fw->Unsigned32(stackbase);
			pi->fw->Unsigned32(stackfunc);
			pi->fw->Unsigned32(stacktop);
			pi->fw->Signed32(ci->nresults);
			pi->fw->Unsigned32(savedpc);
		}
	}

	/* Serialize the state's other parameters, with the exception of upval stuff */
	{
		size_t stackbase = L2->base - L2->stack;
		size_t stacktop = L2->top - L2->stack;
		lua_assert(L2->nCcalls <= 1);
		pi->fw->Unsigned8(L2->status);
		pi->fw->Unsigned32(stackbase);
		pi->fw->Unsigned32(stacktop);
		pi->fw->Signed32(L2->errfunc);
	}

	/* Finally, record upvalues which need to be reopened */
	/* See the comment above persistupval() for why we do this */
	{
		GCObject *gco;
		UpVal *uv;
					/* perms reftbl ... thr */
		for(gco = L2->openupval; gco != NULL; gco = uv->next) {
			uv = gco2uv(gco);

			/* Make sure upvalue is really open */
			lua_assert(uv->v != &uv->u.value);
			pushupval(pi->L, uv);
					/* perms reftbl ... thr uv */
			persist(pi);
			lua_pop(pi->L, 1);
					/* perms reftbl ... thr */
			pi->fw->Unsigned32(uv->v - L2->stack);
		}
					/* perms reftbl ... thr */
		lua_pushnil(pi->L);
					/* perms reftbl ... thr nil */
		persist(pi);
		lua_pop(pi->L, 1);
					/* perms reftbl ... thr */
	}
					/* perms reftbl ... thr */
}

static void persistboolean(PersistInfo *pi)
{
	pi->fw->Unsigned8(lua_toboolean(pi->L, -1));
}
static void unpersistboolean(UnpersistInfo *upi)
{
					/* perms reftbl ... */
	lua_checkstack(upi->L, 1);
	lua_pushboolean(upi->L, upi->fr->Unsigned8());
					/* perms reftbl ... bool */
}


/* Problem: lua_Numbers are doubles. Doubles can't be saved portably, so
 * we do what all scientists to since the dawn of matlab: we convert it
 * to a string that Lua can interpret as the same number again
 */
static void persistnumber(PersistInfo *pi)
{
	const char * n = lua_tostring(pi->L, -1);
	pi->fw->CString(n);
}
static void unpersistnumber(UnpersistInfo *upi)
{
					/* perms reftbl ... */
	lua_checkstack(upi->L, 1);
	const char * s = upi->fr->CString();

	lua_pushstring(upi->L, s);
	lua_Number n = lua_tonumber(upi->L, -1);
	lua_pop(upi->L, 1);
	lua_pushnumber(upi->L, n);
					/* perms reftbl ... num */
}


static void persiststring(PersistInfo *pi)
{
	size_t length = lua_strlen(pi->L, -1);
	pi->fw->Unsigned32(length);
	pi->fw->Data
		(lua_tostring(pi->L, -1), length, Widelands::FileWrite::Pos::Null());
}
static void unpersiststring(UnpersistInfo *upi)
{
					/* perms reftbl sptbl ref */
	lua_checkstack(upi->L, 1);
	size_t length = upi->fr->Unsigned32();
	lua_pushlstring(upi->L, upi->fr->Data(length), length);
}


/* Top-level delegating persist function
 */
static void persist(PersistInfo *pi)
{
					/* perms reftbl ... obj */
	lua_checkstack(pi->L, 2);
	/* If the object has already been written, write a reference to it */
	lua_pushvalue(pi->L, -1);
					/* perms reftbl ... obj obj */
	lua_rawget(pi->L, 2);
					/* perms reftbl ... obj ref? */
	if(!lua_isnil(pi->L, -1)) {
					/* perms reftbl ... obj ref */
		int ref = (intptr_t)lua_touserdata(pi->L, -1);
		pi->fw->Unsigned8(0);
		pi->fw->Signed32(ref);
		lua_pop(pi->L, 1);
					/* perms reftbl ... obj ref */
#ifdef PLUTO_DEBUG
		printindent(pi->level);
		printf("0 %d\n", ref);
#endif
		return;
	}
					/* perms reftbl ... obj nil */
	lua_pop(pi->L, 1);
					/* perms reftbl ... obj */
	/* If the object is nil, write the pseudoreference 0 */
	if(lua_isnil(pi->L, -1)) {
		/* firsttime, ref */
		pi->fw->Unsigned8(0);
		pi->fw->Unsigned32(0);
#ifdef PLUTO_DEBUG
		printindent(pi->level);
		printf("0 0\n");
#endif
		return;
	}
	/* indicate that it's the first time */
	pi->fw->Unsigned8(1);
	lua_pushvalue(pi->L, -1);
					/* perms reftbl ... obj obj */
	lua_pushlightuserdata(pi->L, (void*)((intptr_t) ++(pi->counter)));
					/* perms reftbl ... obj obj ref */
	lua_rawset(pi->L, 2);
					/* perms reftbl ... obj */

	pi->fw->Signed32(pi->counter);


	/* At this point, we'll give the permanents table a chance to play. */
	{
		lua_pushvalue(pi->L, -1);
					/* perms reftbl ... obj obj */
		lua_gettable(pi->L, 1);
					/* perms reftbl ... obj permkey? */
		if(!lua_isnil(pi->L, -1)) {
					/* perms reftbl ... obj permkey */
			int type = PLUTO_TPERMANENT;
#ifdef PLUTO_DEBUG
			printindent(pi->level);
			printf("1 %d PERM\n", pi->counter);
			pi->level++;
#endif
			pi->fw->Signed32(type);
			persist(pi);
			lua_pop(pi->L, 1);
					/* perms reftbl ... obj */
#ifdef PLUTO_DEBUG
			pi->level--;
#endif
			return;
		} else {
					/* perms reftbl ... obj nil */
			lua_pop(pi->L, 1);
					/* perms reftbl ... obj */
		}
					/* perms reftbl ... obj */
	}
	{
		int type = lua_type(pi->L, -1);
		pi->fw->Signed32(type);

#ifdef PLUTO_DEBUG
		printindent(pi->level);
		printf("1 %d %d\n", pi->counter, type);
		pi->level++;
#endif
	}

	switch(lua_type(pi->L, -1)) {
		case LUA_TBOOLEAN:
			persistboolean(pi);
			break;
		case LUA_TNUMBER:
			persistnumber(pi);
			break;
		case LUA_TSTRING:
			persiststring(pi);
			break;
		case LUA_TTABLE:
			persisttable(pi);
			break;
		case LUA_TFUNCTION:
			persistfunction(pi);
			break;
		case LUA_TTHREAD:
			persistthread(pi);
			break;
		case LUA_TPROTO:
			persistproto(pi);
			break;
		case LUA_TUPVAL:
			persistupval(pi);
			break;
		case LUA_TUSERDATA:
			persistuserdata(pi);
			break;
		case LUA_TLIGHTUSERDATA:
			throw wexception("Can't persist a lightuserdata object!");
			break;
		default:
			lua_assert(0);
	}
#ifdef PLUTO_DEBUG
	pi->level--;
#endif
}

void pluto_persist(lua_State *L, lua_Chunkwriter writer, void *ud, Widelands::FileWrite & fw)
{
	PersistInfo pi;

	pi.counter = 0;
	pi.L = L;
	pi.writer = writer;
	pi.ud = ud;
	pi.fw = &fw;
#ifdef PLUTO_DEBUG
	pi.level = 0;
#endif

	lua_checkstack(L, 4);
					/* perms? rootobj? ...? */
	lua_assert(lua_gettop(L) == 2);
					/* perms rootobj */
	lua_assert(!lua_isnil(L, 2));
					/* perms rootobj */
	lua_newtable(L);
					/* perms rootobj reftbl */

	/* Now we're going to make the table weakly keyed. This prevents the
	 * GC from visiting it and trying to mark things it doesn't want to
	 * mark in tables, e.g. upvalues. All objects in the table are
	 * a priori reachable, so it doesn't matter that we do this. */
	lua_newtable(L);
					/* perms rootobj reftbl mt */
	lua_pushstring(L, "__mode");
					/* perms rootobj reftbl mt "__mode" */
	lua_pushstring(L, "k");
					/* perms rootobj reftbl mt "__mode" "k" */
	lua_settable(L, 4);
					/* perms rootobj reftbl mt */
	lua_setmetatable(L, 3);
					/* perms rootobj reftbl */
	lua_insert(L, 2);
					/* perms reftbl rootobj */
	persist(&pi);
					/* perms reftbl rootobj */
	lua_remove(L, 2);
					/* perms rootobj */
}


/* The GC is not fond of finding upvalues in tables. We get around this
 * during persistence using a weakly keyed table, so that the GC doesn't
 * bother to mark them. This won't work in unpersisting, however, since
 * if we make the values weak they'll be collected (since nothing else
 * references them). Our solution, during unpersisting, is to represent
 * upvalues as dummy functions, each with one upvalue. */
static void boxupval_start(lua_State *L)
{
	LClosure *lcl;
	lcl = (LClosure*)pdep_newLclosure(L, 1, hvalue(&L->l_gt));
	pushclosure(L, (Closure*)lcl);
					/* ... func */
	lcl->p = makefakeproto(L, 1);

	/* Temporarily initialize the upvalue to nil */

	lua_pushnil(L);
	lcl->upvals[0] = makeupval(L, -1);
	lua_pop(L, 1);
}

static void boxupval_finish(lua_State *L)
{
					/* ... func obj */
	LClosure *lcl = (LClosure *) clvalue(getobject(L, -2));

	lcl->upvals[0]->u.value = *getobject(L, -1);
	lua_pop(L, 1);
}



static void unpersistupval(int ref, UnpersistInfo *upi)
{
					/* perms reftbl ... */
	lua_checkstack(upi->L, 2);

	boxupval_start(upi->L);
					/* perms reftbl ... func */
	registerobject(ref, upi);

	unpersist(upi);
					/* perms reftbl ... func obj */
	boxupval_finish(upi->L);
					/* perms reftbl ... func */
}

static void unpersistproto(int ref, UnpersistInfo *upi)
{
					/* perms reftbl ... */
	Proto *p;
	int i;
	int sizep, sizek;

	/* We have to be careful. The GC expects a lot out of protos. In
	 * particular, we need to give the function a valid string for its
	 * source, and valid code, even before we actually read in the real
	 * code. */
	TString *source = pdep_newlstr(upi->L, "", 0);

	(void) ref;			/* unused */

	p = pdep_newproto(upi->L);
	p->source = source;
	p->sizecode=1;
	p->code = pdep_newvector(upi->L, 1, Instruction);
	p->code[0] = CREATE_ABC(OP_RETURN, 0, 1, 0);
	p->maxstacksize = 2;
	p->sizek = 0;
	p->sizep = 0;

	lua_checkstack(upi->L, 2);

	pushproto(upi->L, p);
					/* perms reftbl ... proto */
	/* We don't need to register early, since protos can never ever be
	 * involved in cyclic references */

	/* Read in constant references */
	{
		sizek = upi->fr->Signed32();
		pdep_reallocvector(upi->L, p->k, 0, sizek, TValue);
		for(i=0; i<sizek; i++) {
					/* perms reftbl ... proto */
			unpersist(upi);
					/* perms reftbl ... proto k */
			setobj2s(upi->L, &p->k[i], getobject(upi->L, -1));
			p->sizek++;
			lua_pop(upi->L, 1);
					/* perms reftbl ... proto */
		}
					/* perms reftbl ... proto */
	}
	/* Read in sub-proto references */
	{
		sizep = upi->fr->Signed32();
		pdep_reallocvector(upi->L, p->p, 0, sizep, Proto*);
		for(i=0; i<sizep; i++) {
					/* perms reftbl ... proto */
			unpersist(upi);
					/* perms reftbl ... proto subproto */
			p->p[i] = toproto(upi->L, -1);
			p->sizep++;
			lua_pop(upi->L, 1);
					/* perms reftbl ... proto */
		}
					/* perms reftbl ... proto */
	}

	/* Read in code */
	{
		p->sizecode = upi->fr->Signed32();
		pdep_reallocvector(upi->L, p->code, 1, p->sizecode, Instruction);
		for (int32_t i = 0; i < p->sizecode; i++)
			p->code[i] = upi->fr->Unsigned32();
	}

	/* Read in upvalue names */
	{
		p->sizeupvalues = upi->fr->Signed32();
		if (p->sizeupvalues)
		{
			pdep_reallocvector(upi->L, p->upvalues, 0, p->sizeupvalues, TString *);
			for(i=0; i<p->sizeupvalues; i++)
			{
				unpersist(upi);
				p->upvalues[i] = pdep_newlstr(upi->L, lua_tostring(upi->L, -1), strlen(lua_tostring(upi->L, -1)));
				lua_pop(upi->L, 1);
			}
		}
	}

	/* Read in local variable infos */
	{
		p->sizelocvars = upi->fr->Signed32();
		if (p->sizelocvars)
		{
			pdep_reallocvector(upi->L, p->locvars, 0, p->sizelocvars, LocVar);
			for(i=0; i<p->sizelocvars; i++)
			{
				unpersist(upi);
				p->locvars[i].varname = pdep_newlstr(upi->L, lua_tostring(upi->L, -1), strlen(lua_tostring(upi->L, -1)));
				lua_pop(upi->L, 1);

				p->locvars[i].startpc = upi->fr->Signed32();
				p->locvars[i].endpc = upi->fr->Signed32();
			}
		}
	}

	/* Read in source string*/
	unpersist(upi);
	p->source = pdep_newlstr(upi->L, lua_tostring(upi->L, -1), strlen(lua_tostring(upi->L, -1)));
	lua_pop(upi->L, 1);

	/* Read in line numbers */
	{
		p->sizelineinfo = upi->fr->Signed32();
		if (p->sizelineinfo)
		{
			pdep_reallocvector(upi->L, p->lineinfo, 0, p->sizelineinfo, int);
			for(int32_t i = 0; i < p->sizelineinfo; i++)
				p->lineinfo[i] = upi->fr->Signed32();
		}
	}

	/* Read in linedefined and lastlinedefined */
	p->linedefined = upi->fr->Signed32();
	p->lastlinedefined = upi->fr->Signed32();

	/* Read in misc values */
	{
		p->nups = upi->fr->Unsigned8();
		p->numparams = upi->fr->Unsigned8();
		p->is_vararg = upi->fr->Unsigned8();
		p->maxstacksize = upi->fr->Unsigned8();
	}
}


/* Does basically the opposite of luaC_link().
 * Right now this function is rather inefficient; it requires traversing the
 * entire root GC set in order to find one object. If the GC list were doubly
 * linked this would be much easier, but there's no reason for Lua to have
 * that. */
static void gcunlink(lua_State *L, GCObject *gco)
{
	GCObject *prevslot;
	if(G(L)->rootgc == gco) {
		G(L)->rootgc = G(L)->rootgc->gch.next;
		return;
	}

	prevslot = G(L)->rootgc;
	while(prevslot->gch.next != gco) {
		lua_assert(prevslot->gch.next != NULL);
		prevslot = prevslot->gch.next;
	}

	prevslot->gch.next = prevslot->gch.next->gch.next;
}

/* FIXME __ALL__ field ordering */
static void unpersistthread(int ref, UnpersistInfo *upi)
{
					/* perms reftbl ... */
	lua_State *L2;
	size_t stacklimit = 0;
	L2 = lua_newthread(upi->L);
	lua_checkstack(upi->L, 3);
					/* L1: perms reftbl ... thr */
					/* L2: (empty) */
	registerobject(ref, upi);

	/* First, deserialize the object stack. */
	{
		uint32_t stacksize = upi->fr->Unsigned32();
		pdep_growstack(L2, (int)stacksize);
		/* Make sure that the first stack element (a nil, representing
		 * the imaginary top-level C function) is written to the very,
		 * very bottom of the stack */
		L2->top--;
		for(uint32_t i = 0; i < stacksize; i++) {
			unpersist(upi);
					/* L1: perms reftbl ... thr obj* */
		}
		lua_xmove(upi->L, L2, stacksize);
					/* L1: perms reftbl ... thr */
					/* L2: obj* */
	}
					/* (hereafter, stacks refer to L1) */

	/* Now, deserialize the CallInfo stack. */
	{
		uint32_t numframes = upi->fr->Unsigned32();
		pdep_reallocCI(L2,numframes*2);
		for(uint32_t i=0; i<numframes; i++) {
			CallInfo *ci = L2->base_ci + i;
			size_t stackbase, stackfunc, stacktop, savedpc;
			stackbase = upi->fr->Unsigned32();
			stackfunc = upi->fr->Unsigned32();
			stacktop = upi->fr->Unsigned32();
			ci->nresults = upi->fr->Signed32();
			savedpc = upi->fr->Unsigned32();

			if(stacklimit < stacktop)
				stacklimit = stacktop;

			ci->base = L2->stack+stackbase;
			ci->func = L2->stack+stackfunc;
			ci->top = L2->stack+stacktop;
			ci->savedpc = (ci != L2->base_ci) ?
				ci_func(ci)->l.p->code+savedpc :
				0;
			ci->tailcalls = 0;
			/* Update the pointer each time, to keep the GC
			 * happy*/
			L2->ci = ci;
		}
	}
					/* perms reftbl ... thr */
	/* Deserialize the state's other parameters, with the exception of upval stuff */
	{
		size_t stackbase, stacktop;
		L2->savedpc = L2->ci->savedpc;
		L2->status = upi->fr->Unsigned8();
		stackbase = upi->fr->Unsigned32();
		stacktop = upi->fr->Unsigned32();
		L2->errfunc = upi->fr->Signed32();

		L2->base = L2->stack + stackbase;
		L2->top = L2->stack + stacktop;
	}
	/* Finally, "reopen" upvalues (see persistupval() for why) */
	{
		UpVal* uv;
		GCObject **nextslot = &L2->openupval;
		global_State *g = G(L2);
		while(1) {
			unpersist(upi);
					/* perms reftbl ... thr uv/nil */
			if(lua_isnil(upi->L, -1)) {
					/* perms reftbl ... thr nil */
				lua_pop(upi->L, 1);
					/* perms reftbl ... thr */
				break;
			}
					/* perms reftbl ... thr boxeduv */
			unboxupval(upi->L);
					/* perms reftbl ... thr uv */
			uv = toupval(upi->L, -1);
			lua_pop(upi->L, 1);
					/* perms reftbl ... thr */

			uv->v = L2->stack + upi->fr->Unsigned32();
			gcunlink(upi->L, (GCObject*)uv);
			uv->marked = luaC_white(g);
			*nextslot = (GCObject*)uv;
			nextslot = &uv->next;
			uv->u.l.prev = &G(L2)->uvhead;
			uv->u.l.next = G(L2)->uvhead.u.l.next;
			uv->u.l.next->u.l.prev = uv;
			G(L2)->uvhead.u.l.next = uv;
			lua_assert(uv->u.l.next->u.l.prev == uv && uv->u.l.prev->u.l.next == uv);
		}
		*nextslot = NULL;
	}

	/* The stack must be valid at least to the highest value among the CallInfos */
	/* 'top' and the values up to there must be filled with 'nil' */
	{
		StkId o;
		pdep_checkstack(L2, (int)stacklimit);
		for (o = L2->top; o <= L2->top + stacklimit; o++)
			setnilvalue(o);
	}
}

static void unpersistuserdata(int ref, UnpersistInfo *upi)
{
					/* perms reftbl ... */
	lua_checkstack(upi->L, 2);
	if(upi->fr->Unsigned8()) {
		unpersist(upi);
					/* perms reftbl ... spfunc? */
		lua_assert(lua_isfunction(upi->L, -1));
					/* perms reftbl ... spfunc */
		lua_call(upi->L, 0, 1);
					/* perms reftbl ... udata? */
/* This assertion might not be necessary; it's conceivable, for
 * example, that the SP function might decide to return a table
 * with equivalent functionality. For the time being, we'll
 * ignore this possibility in favor of stricter and more testable
 * requirements. */
		lua_assert(lua_isuserdata(upi->L, -1));
					/* perms reftbl ... udata */
	} else {
		lua_pushstring(upi->L, "Attempt to unpersist a userdata without a "
				"__unpersist function. This is not possible!");
		lua_error(upi->L);
	}
					/* perms reftbl ... udata */
}

static void unpersistpermanent(int ref, UnpersistInfo *upi)
{
	(void) ref;			/* unused */
					/* perms reftbl ... */
	lua_checkstack(upi->L, 2);
	unpersist(upi);
					/* perms reftbl permkey */
	lua_gettable(upi->L, 1);
					/* perms reftbl perm? */
	/* We assume currently that the substituted permanent value
	 * shouldn't be nil. This may be a bad assumption. Real-life
	 * experience is needed to evaluate this. */
	lua_assert(!lua_isnil(upi->L, -1));
					/* perms reftbl perm */
}

/* For debugging only; not called when lua_assert is empty */
int inreftable(lua_State *L, int ref)
{
	int res;
	lua_checkstack(L, 1);
					/* perms reftbl ... */
	lua_pushlightuserdata(L, (void*)(intptr_t)ref);
					/* perms reftbl ... ref */
	lua_gettable(L, 2);
					/* perms reftbl ... obj? */
	res = !lua_isnil(L, -1);
	lua_pop(L, 1);
					/* perms reftbl ... */
	return res;
}

static void unpersist(UnpersistInfo *upi)
{
					/* perms reftbl ... */
	int stacksize = lua_gettop(upi->L); stacksize = stacksize; /* DEBUG */
	lua_checkstack(upi->L, 2);
	if(upi->fr->Unsigned8()) {
		int ref = upi->fr->Signed32();
		lua_assert(!inreftable(upi->L, ref));
		int type = upi->fr->Signed32();
#ifdef PLUTO_DEBUG
		printindent(upi->level);
		printf("1 %d %d\n", ref, type);
		upi->level++;
#endif
		switch(type) {
		case LUA_TBOOLEAN:
			unpersistboolean(upi);
			break;
		case LUA_TNUMBER:
			unpersistnumber(upi);
			break;
		case LUA_TSTRING:
			unpersiststring(upi);
			break;
		case LUA_TTABLE:
			unpersisttable(ref, upi);
			break;
		case LUA_TFUNCTION:
			unpersistfunction(ref, upi);
			break;
		case LUA_TTHREAD:
			unpersistthread(ref, upi);
			break;
		case LUA_TPROTO:
			unpersistproto(ref, upi);
			break;
		case LUA_TUPVAL:
			unpersistupval(ref, upi);
			break;
		case LUA_TUSERDATA:
			unpersistuserdata(ref, upi);
			break;
		case PLUTO_TPERMANENT:
			unpersistpermanent(ref, upi);
			break;
		case LUA_TLIGHTUSERDATA:
			throw wexception("Can't unpersist a lightuserdata object!");
			break;
		default:
			lua_assert(0);
		}
					/* perms reftbl ... obj */
		lua_assert(lua_type(upi->L, -1) == type ||
			type == PLUTO_TPERMANENT ||
			/* Remember, upvalues get a special dispensation, as
			 * described in boxupval */
			(lua_type(upi->L, -1) == LUA_TFUNCTION &&
				type == LUA_TUPVAL));
		registerobject(ref, upi);
					/* perms reftbl ... obj */
#ifdef PLUTO_DEBUG
		upi->level--;
#endif
	} else {
		int ref = upi->fr->Signed32();
#ifdef PLUTO_DEBUG
		printindent(upi->level);
		printf("0 %d\n", ref);
#endif
		if(ref == 0) {
			lua_pushnil(upi->L);
					/* perms reftbl ... nil */
		} else {
			lua_pushlightuserdata(upi->L, (void*)(intptr_t)ref);
					/* perms reftbl ... ref */
			lua_gettable(upi->L, 2);
					/* perms reftbl ... obj? */
			lua_assert(!lua_isnil(upi->L, -1));
		}
					/* perms reftbl ... obj/nil */
	}
					/* perms reftbl ... obj/nil */
	lua_assert(lua_gettop(upi->L) == stacksize + 1);
}

void pluto_unpersist
	(lua_State *L, lua_Chunkreader reader, void *ud, Widelands::FileRead & fr)
{
	/* We use the graciously provided ZIO (what the heck does the Z stand
	 * for?) library so that we don't have to deal with the reader directly.
	 * Letting the reader function decide how much data to return can be
	 * very unpleasant.
	 */
	UnpersistInfo upi;
	upi.L = L;
	upi.fr = &fr;
#ifdef PLUTO_DEBUG
	upi.level = 0;
#endif

	lua_checkstack(L, 3);

					/* perms */
	lua_newtable(L);
					/* perms reftbl */
	lua_gc(L, LUA_GCSTOP, 0);
	unpersist(&upi);
	lua_gc(L, LUA_GCRESTART, 0);
					/* perms reftbl rootobj */
	lua_replace(L, 2);
					/* perms rootobj  */
}

