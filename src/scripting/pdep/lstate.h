/*
** $Id: lstate.h,v 2.24.1.2 2008/01/03 15:20:39 roberto Exp $
** Global State
** See Copyright Notice in lua.h
*/

#ifndef lstate_h
#define lstate_h

#include "lobject.h"
#include "ltm.h"
#include "lzio.h"

struct lua_longjmp;  //  defined in ldo.c


/* extra stack space to handle TM calls and some other extras */
#define EXTRA_STACK   5


struct stringtable {
	GCObject * * hash;
	uint32_t     nuse; //  number of elements
	int          size;
};


//  information about a call
struct CallInfo {
	StkId base;      //  base for this function
	StkId func;      //  function index in the stack
	StkId top;       //  top for this function
	Instruction const * savedpc;
	int   nresults;  //  expected number of results from this function
	int   tailcalls; //  number of tail calls lost under this entry
};


#define ci_func(ci) (clvalue((ci)->func))


/*
** `global state', shared by all threads of this state
*/
struct global_State {
	stringtable   strt;       //  hash table for strings
	lua_Alloc     frealloc;   //  function to reallocate memory
	void        * ud;         //  auxiliary data to `frealloc'
	lu_byte       currentwhite;
	lu_byte       gcstate;    //  state of garbage collector
	int           sweepstrgc; //  position of sweep in `strt'
	GCObject    * rootgc;     //  list of all collectable objects
	GCObject  * * sweepgc;    //  position of sweep in `rootgc'
	GCObject    * gray;       //  list of gray objects
	GCObject    * grayagain;  //  list of objects to be traversed atomically
	GCObject    * weak;       //  list of weak tables (to be cleared)
	GCObject    * tmudata;    //  last element of list of userdata to be GC
	Mbuffer       buff;       //  temporary buffer for string concatentation
	lu_mem        GCthreshold;
	lu_mem        totalbytes; //  number of bytes currently allocated
	lu_mem        estimate;   //  an estimate of number of bytes actually in use
	lu_mem        gcdept;     //  how much GC is `behind schedule'
	int           gcpause;    //  size of pause between successive GCs
	int           gcstepmul;  //  GC `granularity'
	lua_CFunction panic;      //  to be called in unprotected errors
	TValue        l_registry;
	lua_State   * mainthread;
	UpVal         uvhead;  //  head of double-linked list of all open upvalues
	Table       * mt[NUM_TAGS]; //  metatables for basic types
	TString     * tmname[TM_N]; //  array with tag-method names
};


/*
** `per thread' state
*/
struct lua_State {
	CommonHeader;
	lu_byte status;
	StkId top;                      //  first free slot in the stack
	StkId               base;       //  base of current function
	global_State      * l_G;
	CallInfo          * ci;         //  call info for current function
	Instruction const * savedpc;    //  `savedpc' of current function
	StkId               stack_last; //  last free slot in the stack
	StkId               stack;      //  stack base
	CallInfo          * end_ci;     //  points after end of ci array
	CallInfo          * base_ci;    //  array of CallInfo's
	int                 stacksize;
	int                 size_ci;    //  size of array `base_ci'
	unsigned short      nCcalls;    //  number of nested C calls
	unsigned short      baseCcalls; //  nested C calls when resuming coroutine
	lu_byte             hookmask;
	lu_byte             allowhook;
	int                 basehookcount;
	int                 hookcount;
	lua_Hook            hook;
	TValue              l_gt;       //  table of globals
	TValue              env;        //  temporary place for environments
	GCObject          * openupval;  //  list of open upvalues in this stack
	GCObject          * gclist;
	lua_longjmp       * errorJmp;   //  current error recover point
	ptrdiff_t errfunc; //  current error handling function (stack index)
};


#define G(L) (L->l_G)


/*
** Union of all collectable objects
*/
union GCObject {
	GCheader         gch;
	union TString    ts;
	union Udata      u;
	union Closure    cl;
	struct Table     h;
	struct Proto     p;
	struct UpVal     uv;
	struct lua_State th;  //  thread
};


/* macros to convert a GCObject into a specific value */
#define gco2p(o)  check_exp((o)->gch.tt == LUA_TPROTO, &((o)->p))
#define gco2uv(o) check_exp((o)->gch.tt == LUA_TUPVAL, &((o)->uv))

/* macro to convert any Lua object into a GCObject */
#define obj2gco(v) (reinterpret_cast<GCObject *>(v))

#endif

