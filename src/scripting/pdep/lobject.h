/*
** $Id: lobject.h,v 2.20.1.1 2007/12/27 13:02:25 roberto Exp $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#ifndef LOBJECT_H
#define LOBJECT_H

#include "llimits.h"
#include <lua.hpp>

#include <cstdarg>

/* tags for values visible from Lua */
#define LAST_TAG LUA_TTHREAD

#define NUM_TAGS (LAST_TAG + 1)


/*
** Extra tags for non-values
*/
#define LUA_TPROTO (LAST_TAG + 1)
#define LUA_TUPVAL (LAST_TAG + 2)


/*
** Union of all collectable objects
*/
union GCObject;


/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#define CommonHeader GCObject * next; lu_byte tt; lu_byte marked


/*
** Common header in struct form
*/
struct GCheader {
	CommonHeader;
};



/*
** Union of all Lua values
*/
union Value {
	GCObject * gc;
	void     * p;
	lua_Number n;
	int        b;
};


/*
** Tagged Values
*/

#define TValuefields Value value; int tt

struct TValue {
	TValuefields;
};


/* Macros to access values */
#define rawtsvalue(o) check_exp(ttisstring  (o), &(o)->value.gc->ts)
#define clvalue(o)    check_exp(ttisfunction(o), &(o)->value.gc->cl)
#define hvalue(o)     check_exp(ttistable   (o), &(o)->value.gc->h)

#define checkliveness(g, obj)                                                 \
  lua_assert                                                                  \
     (!iscollectable(obj) ||                                                  \
      ((ttype(obj) == (obj)->value.gc->gch.tt) && !isdead(g, (obj)->value.gc)))


#define setobj(L, obj1, obj2)                                                 \
  {                                                                           \
     TValue const * const o2 = (obj2);                                        \
     TValue       * const o1 = (obj1);                                        \
     o1->value = o2->value;                                                   \
     o1->tt=o2->tt;                                                           \
     checkliveness(G(L), o1);                                                 \
  }                                                                           \


/*
** different types of sets, according to destination
*/

/* to stack (not from same stack) */
#define setobj2s setobj

#define setttype(obj, tt) (ttype(obj) = (tt))


#define iscollectable(o) (ttype(o) >= LUA_TSTRING)


typedef TValue * StkId; //  index to stack elements */


/*
** String headers for string table
*/
union TString {
	L_Umaxalign dummy; //  ensures maximum alignment for strings
	struct {
		CommonHeader;
		lu_byte reserved;
		unsigned int hash;
		size_t len;
	} tsv;
};


union Udata {
	L_Umaxalign dummy; //  ensures maximum alignment for `local' udata
	struct {
		CommonHeader;
		struct Table * metatable;
		struct Table * env;
		size_t  len;
	} uv;
};


struct LocVar {
	TString * varname;
	int       startpc; //  first point where variable is active
	int       endpc;   //  first point where variable is dead
};


/*
** Function Prototypes
*/
struct Proto {
	CommonHeader;
	TValue      * k; //  constants used by the function
	Instruction * code;
	Proto     * * p; //  functions defined inside the function
	int         * lineinfo; //  map from opcodes to source lines
	LocVar      * locvars; //  information about local variables
	TString   * * upvalues; //  upvalue names
	TString     * source;
	int           sizeupvalues;
	int           sizek; //  size of `k'
	int           sizecode;
	int           sizelineinfo;
	int           sizep; //  size of `p'
	int           sizelocvars;
	int           linedefined;
	int           lastlinedefined;
	GCObject    * gclist;
	lu_byte       nups; //  number of upvalues
	lu_byte       numparams;
	lu_byte       is_vararg;
	lu_byte       maxstacksize;
};


/* masks for new-style vararg */
#define VARARG_HASARG   1
#define VARARG_ISVARARG 2
#define VARARG_NEEDSARG 4


/*
** Upvalues
*/

struct UpVal {
	CommonHeader;
	TValue * v; //  points to stack or to its own value
	union {
		TValue value; //  the value (when closed)
		struct { //  double linked list (when open)
			struct UpVal * prev;
			struct UpVal * next;
		} l;
	} u;
};


/*
** Closures
*/

#define ClosureHeader                                                         \
   CommonHeader;                                                              \
   lu_byte    isC;                                                            \
   lu_byte    nupvalues;                                                      \
   GCObject * gclist;                                                         \
   Table    * env                                                             \

struct CClosure {
	ClosureHeader;
	lua_CFunction f;
	TValue        upvalue[1];
};


struct LClosure {
	ClosureHeader;
	struct Proto * p;
	UpVal        * upvals[1];
};


union Closure {
	CClosure c;
	LClosure l;
};


#define iscfunction(o) (ttype(o) == LUA_TFUNCTION && clvalue(o)->c.isC)


/*
** Tables
*/

union TKey {
	struct {
		TValuefields;
		struct Node * next; //  for chaining
	} nk;
	TValue tvk;
};


struct Node {
	TValue i_val;
	TKey   i_key;
};


struct Table {
	CommonHeader;
	lu_byte    flags;     //  1 << p means tagmethod(p) is not present
	lu_byte    lsizenode; //  log2 of size of `node' array
	Table    * metatable;
	TValue   * array;     //  array part
	Node     * node;
	Node     * lastfree;  //  any free position is before this position
	GCObject * gclist;
	int        sizearray; //  size of `array' array
};

#endif
