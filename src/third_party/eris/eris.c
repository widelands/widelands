/*
Eris - Heavy-duty persistence for Lua 5.3.0 - Based on Pluto
Copyright (c) 2013-2015 by Florian Nuecke.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/* Standard library headers. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Not using stdbool because Visual Studio lives in the past... */
typedef int bool;
#define false 0
#define true 1

/* Mark us as part of the Lua core to get access to what we need. */
#define LUA_CORE

/* Public Lua headers. */
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/* Internal Lua headers. */
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "lzio.h"

/* Eris header. */
#include "eris.h"

/*
** {===========================================================================
** Default settings.
** ============================================================================
*/

/* Note that these are the default settings. They can be changed either from C
 * by calling eris_g|set_setting() or from Lua using eris.settings(). */

/* The metatable key we use to allow customized persistence for tables and
 * userdata. */
static const char *const kPersistKey = "__persist";

/* Whether to pass the IO object (reader/writer) to the special function
 * defined in the metafield or not. This is disabled per default because it
 * mey allow Lua scripts to do more than they should. Enable this as needed. */
static const bool kPassIOToPersist = false;

/* Whether to persist debug information such as line numbers and upvalue and
 * local variable names. */
static const bool kWriteDebugInformation = true;

/* Generate a human readable "path" that is shown together with error messages
 * to indicate where in the object the error occurred. For example:
 * eris.persist({false, bad = setmetatable({}, {__persist = false})})
 * Will produce: main:1: attempt to persist forbidden table (root.bad)
 * This can be used for debugging, but is disabled per default due to the
 * processing and memory overhead this introduces. */
static const bool kGeneratePath = false;

/* The maximum object complexity. This is the number of allowed recursions when
 * persisting or unpersisting an object, for example for nested tables. This is
 * used to avoid segfaults when writing or reading user data. */
static const lua_Unsigned kMaxComplexity = 10000;

/*
** ============================================================================
** Lua internals interfacing.
** ============================================================================
*/

/* Lua internals we use. We define these as macros to make it easier to swap
 * them out, should the need ever arise. For example, the later Pluto versions
 * copied these function to own files (presumably to allow building it as an
 * extra shared library). These should be all functions we use that are not
 * declared in lua.h or lauxlib.h. If there are some still directly in the code
 * they were missed and should be replaced with a macro added here instead. */
/* I'm quite sure we won't ever want to do this, because Eris needs a slightly
 * patched Lua version to be able to persist some of the library functions,
 * anyway: it needs to put the continuation C functions in the perms table. */
/* ldebug.h */
#define eris_ci_func ci_func
/* ldo.h */
#define eris_incr_top incr_top
#define eris_savestack savestack
#define eris_restorestack restorestack
#define eris_reallocstack luaD_reallocstack
/* lfunc.h */
#define eris_newproto luaF_newproto
#define eris_newLclosure luaF_newLclosure
#define eris_initupvals luaF_initupvals
#define eris_findupval luaF_findupval
/* lmem.h */
#define eris_reallocvector luaM_reallocvector
/* lobject.h */
#define eris_ttnov ttnov
#define eris_clLvalue clLvalue
#define eris_setnilvalue setnilvalue
#define eris_setclLvalue setclLvalue
#define eris_setobj setobj
#define eris_setsvalue2n setsvalue2n
/* lstate.h */
#define eris_isLua isLua
#define eris_gch gch
#define eris_gco2uv gco2uv
#define eris_obj2gco obj2gco
#define eris_extendCI luaE_extendCI
/* lstring. h */
#define eris_newlstr luaS_newlstr
/* lzio.h */
#define eris_initbuffer luaZ_initbuffer
#define eris_buffer luaZ_buffer
#define eris_sizebuffer luaZ_sizebuffer
#define eris_bufflen luaZ_bufflen
#define eris_init luaZ_init
#define eris_read luaZ_read

/* These are required for cross-platform support, since the size of TValue may
 * differ, so the byte offset used by savestack/restorestack in Lua it is not a
 * valid measure. */
#define eris_savestackidx(L, p) ((p) - (L)->stack)
#define eris_restorestackidx(L, n) ((L)->stack + (n))

/* Enabled if we have a patched version of Lua (for accessing internals). */
#if 1

/* Functions in Lua libraries used to access C functions we need to add to the
 * permanents table to fully support yielded coroutines. */
extern void eris_permbaselib(lua_State *L, bool forUnpersist);
extern void eris_permcorolib(lua_State *L, bool forUnpersist);
extern void eris_permloadlib(lua_State *L, bool forUnpersist);
extern void eris_permiolib(lua_State *L, bool forUnpersist);
extern void eris_permstrlib(lua_State *L, bool forUnpersist);

/* Utility macro for populating the perms table with internal C functions. */
#define populateperms(L, forUnpersist) {\
  eris_permbaselib(L, forUnpersist);\
  eris_permcorolib(L, forUnpersist);\
  eris_permloadlib(L, forUnpersist);\
  eris_permiolib(L, forUnpersist);\
  eris_permstrlib(L, forUnpersist);\
}

#else

/* Does nothing if we don't have a patched version of Lua. */
#define populateperms(L, forUnpersist) ((void)0)

#endif

/*
** ============================================================================
** Language strings for errors.
** ============================================================================
*/

#define ERIS_ERR_CFUNC "attempt to persist a light C function (%p)"
#define ERIS_ERR_COMPLEXITY "object too complex"
#define ERIS_ERR_HOOK "cannot persist yielded hooks"
#define ERIS_ERR_METATABLE "bad metatable, not nil or table"
#define ERIS_ERR_NOFUNC "attempt to persist unknown function type"
#define ERIS_ERR_READ "could not read data"
#define ERIS_ERR_SPER_FUNC "%s did not return a function"
#define ERIS_ERR_SPER_LOAD "bad unpersist function (%s expected, returned %s)"
#define ERIS_ERR_SPER_PROT "attempt to persist forbidden table"
#define ERIS_ERR_SPER_TYPE "%d not nil, boolean, or function"
#define ERIS_ERR_SPER_UFUNC "invalid restore function"
#define ERIS_ERR_SPER_UPERM "bad permanent value (%s expected, got %s)"
#define ERIS_ERR_SPER_UPERMNIL "bad permanent value (no value)"
#define ERIS_ERR_STACKBOUNDS "stack index out of bounds"
#define ERIS_ERR_TABLE "bad table value, got a nil value"
#define ERIS_ERR_THREAD "cannot persist currently running thread"
#define ERIS_ERR_THREADCI "invalid callinfo"
#define ERIS_ERR_THREADCTX "bad C continuation function"
#define ERIS_ERR_THREADERRF "invalid errfunc"
#define ERIS_ERR_THREADPC "saved program counter out of bounds"
#define ERIS_ERR_TRUNC_INT "int value would get truncated"
#define ERIS_ERR_TRUNC_SIZE "size_t value would get truncated"
#define ERIS_ERR_TYPE_FLOAT "unsupported lua_Number type"
#define ERIS_ERR_TYPE_INT "unsupported int type"
#define ERIS_ERR_TYPE_SIZE "unsupported size_t type"
#define ERIS_ERR_TYPEP "trying to persist unknown type %d"
#define ERIS_ERR_TYPEU "trying to unpersist unknown type %d"
#define ERIS_ERR_UCFUNC "bad C closure (C function expected, got %s)"
#define ERIS_ERR_UCFUNCNULL "bad C closure (C function expected, got null)"
#define ERIS_ERR_USERDATA "attempt to literally persist userdata"
#define ERIS_ERR_WRITE "could not write data"
#define ERIS_ERR_REF "invalid reference #%d. this usually means a special "\
                      "persistence callback of a table referenced said table "\
                      "(directly or indirectly via an upvalue)."

/*
** ============================================================================
** Constants, settings, types and forward declarations.
** ============================================================================
*/

/* The upvalue tag type was removed in Lua 5.3, but we still need it for
 * special handling of upvalues, so we redeclare it for internal use. */
#define LUA_TUPVAL (LUA_TOTALTAGS + 1)

/* The "type" we write when we persist a value via a replacement from the
 * permanents table. This is just an arbitrary number, but it must we lower
 * than the reference offset (below) and outside the range Lua uses for its
 * types (> LUA_TOTALTAGS). */
#define ERIS_PERMANENT (LUA_TOTALTAGS + 2)

/* This is essentially the first reference we'll use. We do this to save one
 * field in our persisted data: if the value is smaller than this, the object
 * itself follows, otherwise we have a reference to an already unpersisted
 * object. Note that in the reftable the actual entries are still stored
 * starting at the first array index to have a sequence (when unpersisting). */
#define ERIS_REFERENCE_OFFSET (ERIS_PERMANENT + 1)

/* Avoids having to write the NULL all the time, plus makes it easier adding
 * a custom error message should you ever decide you want one. */
#define eris_checkstack(L, n) luaL_checkstack(L, n, NULL)

/* Used for internal consistency checks, for debugging. These are true asserts
 * in the sense that they should never fire, even for bad inputs. */
#if 0
#define eris_assert(c) assert(c)
#define eris_ifassert(e) e
#else
#define eris_assert(c) ((void)0)
#define eris_ifassert(e) ((void)0)
#endif

/* State information when persisting an object. */
typedef struct PersistInfo {
  lua_Writer writer;
  void *ud;
  const char *metafield;
  bool writeDebugInfo;
} PersistInfo;

/* State information when unpersisting an object. */
typedef struct UnpersistInfo {
  ZIO zio;
  size_t sizeof_int;
  size_t sizeof_size_t;
} UnpersistInfo;

/* Info shared in persist and unpersist. */
typedef struct Info {
  lua_State *L;
  lua_Unsigned level;
  int refcount; /* int because rawseti/rawgeti takes an int. */
  lua_Unsigned maxComplexity;
  bool generatePath;
  bool passIOToPersist;
  /* Which one it really is will always be clear from the context. */
  union {
    PersistInfo pi;
    UnpersistInfo upi;
  } u;
} Info;

/* Type names, used for error messages. */
static const char *const kTypenames[] = {
  "nil", "boolean", "lightuserdata", "number", "string",
  "table", "function", "userdata", "thread", "proto", "upval",
  "deadkey", "permanent"
};

/* Setting names as used in eris.settings / eris_g|set_setting. Also, the
 * addresses of these static variables are used as keys in the registry of Lua
 * states to save the current values of the settings (as light userdata). */
static const char *const kSettingMetafield = "spkey";
static const char *const kSettingPassIOToPersist = "spio";
static const char *const kSettingGeneratePath = "path";
static const char *const kSettingWriteDebugInfo = "debug";
static const char *const kSettingMaxComplexity = "maxrec";

/* Header we prefix to persisted data for a quick check when unpersisting. */
static char const kHeader[] = { 'E', 'R', 'I', 'S' };
#define HEADER_LENGTH sizeof(kHeader)

/* Floating point number used to check compatibility of loaded data. */
static const lua_Number kHeaderNumber = (lua_Number)-1.234567890;

/* Stack indices of some internal values/tables, to avoid magic numbers. */
#define PERMIDX 1
#define REFTIDX 2
#define BUFFIDX 3
#define PATHIDX 4

/* Table indices for upvalue tables, keeping track of upvals to open. */
#define UVTOCL 1
#define UVTONU 2
#define UVTVAL 3
#define UVTREF 4

/* }======================================================================== */

/*
** {===========================================================================
** Utility functions.
** ============================================================================
*/

/* Pushes an object into the reference table when unpersisting. This creates an
 * entry pointing from the id the object is referenced by to the object. */
static int
registerobject(Info *info) {                          /* perms reftbl ... obj */
  const int reference = ++(info->refcount);
  eris_checkstack(info->L, 1);
  lua_pushvalue(info->L, -1);                     /* perms reftbl ... obj obj */
  lua_rawseti(info->L, REFTIDX, reference);           /* perms reftbl ... obj */
  return reference;
}

/** ======================================================================== */

/* Pushes a TString* onto the stack if it holds a value, nil if it is NULL. */
static void
pushtstring(lua_State* L, TString *ts) {                               /* ... */
  if (ts) {
    eris_setsvalue2n(L, L->top, ts);
    eris_incr_top(L);                                              /* ... str */
  }
  else {
    lua_pushnil(L);                                                /* ... nil */
  }
}

/* Creates a copy of the string on top of the stack and sets it as the value
 * of the specified TString**. */
static void
copytstring(lua_State* L, TString **ts) {
  size_t length;
  const char *value = lua_tolstring(L, -1, &length);
  *ts = eris_newlstr(L, value, length);
}

/** ======================================================================== */

/* Pushes the specified segment to the current path, if we're generating one.
 * This supports formatting strings using Lua's formatting capabilities. */
static void
pushpath(Info *info, const char* fmt, ...) {     /* perms reftbl var path ... */
  if (!info->generatePath) {
    return;
  }
  else {
    va_list argp;
    eris_checkstack(info->L, 1);
    va_start(argp, fmt);
    lua_pushvfstring(info->L, fmt, argp);    /* perms reftbl var path ... str */
    va_end(argp);
    lua_rawseti(info->L, PATHIDX, luaL_len(info->L, PATHIDX) + 1);
  }                                              /* perms reftbl var path ... */
}  

/* Pops the last added segment from the current path if we're generating one. */
static void
poppath(Info *info) {                            /* perms reftbl var path ... */
  if (!info->generatePath) {
    return;
  }
  eris_checkstack(info->L, 1);
  lua_pushnil(info->L);                      /* perms reftbl var path ... nil */
  lua_rawseti(info->L, PATHIDX, luaL_len(info->L, PATHIDX));
}                                                /* perms reftbl var path ... */

/* Concatenates all current path segments into one string, pushes it and
 * returns it. This is relatively inefficient, but it's for errors only and
 * keeps the stack small, so it's better this way. */
static const char*
path(Info *info) {                               /* perms reftbl var path ... */
  if (!info->generatePath) {
    return "";
  }
  eris_checkstack(info->L, 3);
  lua_pushstring(info->L, "");               /* perms reftbl var path ... str */
  lua_pushnil(info->L);                  /* perms reftbl var path ... str nil */
  while (lua_next(info->L, PATHIDX)) {   /* perms reftbl var path ... str k v */
    lua_insert(info->L, -2);             /* perms reftbl var path ... str v k */
    lua_insert(info->L, -3);             /* perms reftbl var path ... k str v */
    lua_concat(info->L, 2);                /* perms reftbl var path ... k str */
    lua_insert(info->L, -2);               /* perms reftbl var path ... str k */
  }                                          /* perms reftbl var path ... str */
  return lua_tostring(info->L, -1);
}

/* Generates an error message with the appended path, if available. */
static int
eris_error(Info *info, const char *fmt, ...) {                         /* ... */
    va_list argp;
    eris_checkstack(info->L, 5);

    luaL_where(info->L, 1);                                     /* ... where */
    va_start(argp, fmt);
    lua_pushvfstring(info->L, fmt, argp);                    /* ... where str */
    va_end(argp);
    if (info->generatePath) {
      lua_pushstring(info->L, " (");                    /* ... where str " (" */
      path(info);                                 /* ...  where str " (" path */
      lua_pushstring(info->L, ")");            /* ... where str " (" path ")" */
      lua_concat(info->L, 5);                                      /* ... msg */
    }
    else {
      lua_concat(info->L, 2);                                      /* ... msg */
    }
    return lua_error(info->L);
}

/** ======================================================================== */

/* Tries to get a setting from the registry. */
static bool
get_setting(lua_State *L, void *key) {                                 /* ... */
  eris_checkstack(L, 1);
  lua_pushlightuserdata(L, key);                                   /* ... key */
  lua_gettable(L, LUA_REGISTRYINDEX);                        /* ... value/nil */
  if (lua_isnil(L, -1)) {                                          /* ... nil */
    lua_pop(L, 1);                                                     /* ... */
    return false;
  }                                                              /* ... value */
  return true;
}

/* Stores a setting in the registry (or removes it if the value is nil). */
static void
set_setting(lua_State *L, void *key) {                           /* ... value */
  eris_checkstack(L, 2);
  lua_pushlightuserdata(L, key);                             /* ... value key */
  lua_insert(L, -2);                                         /* ... key value */
  lua_settable(L, LUA_REGISTRYINDEX);                                  /* ... */
}

/* Used as a callback for luaL_opt to check boolean setting values. */
static bool
checkboolean(lua_State *L, int narg) {                       /* ... bool? ... */
  if (!lua_isboolean(L, narg)) {                                /* ... :( ... */
    return luaL_argerror(L, narg, lua_pushfstring(L,
      "boolean expected, got %s", lua_typename(L, lua_type(L, narg))));
  }                                                           /* ... bool ... */
  return lua_toboolean(L, narg);
}

/* }======================================================================== */

/*
** {===========================================================================
** Persist and unpersist.
** ============================================================================
*/

/* I have macros and I'm not afraid to use them! These are highly situational
 * and assume an Info* named 'info' is available. */

/* Writes a raw memory block with the specified size. */
#define WRITE_RAW(value, size) {\
  if (info->u.pi.writer(info->L, (value), (size), info->u.pi.ud)) \
    eris_error(info, ERIS_ERR_WRITE); }

/* Writes a single value with the specified type. */
#define WRITE_VALUE(value, type) write_##type(info, value)

/* Writes a typed array with the specified length. */
#define WRITE(value, length, type) { \
    int i; for (i = 0; i < length; ++i) WRITE_VALUE((value)[i], type); }

/** ======================================================================== */

/* Reads a raw block of memory with the specified size. */
#define READ_RAW(value, size) {\
  if (eris_read(&info->u.upi.zio, (value), (size))) \
    eris_error(info, ERIS_ERR_READ); }

/* Reads a single value with the specified type. */
#define READ_VALUE(type) read_##type(info)

/* Reads a typed array with the specified length. */
#define READ(value, length, type) { \
    int i; for (i = 0; i < length; ++i) (value)[i] = READ_VALUE(type); }

/** ======================================================================== */

static void
write_uint8_t(Info *info, uint8_t value) {
  WRITE_RAW(&value, sizeof(uint8_t));
}

static void
write_uint16_t(Info *info, uint16_t value) {
  write_uint8_t(info, value);
  write_uint8_t(info, value >> 8);
}

static void
write_uint32_t(Info *info, uint32_t value) {
  write_uint8_t(info, value);
  write_uint8_t(info, value >> 8);
  write_uint8_t(info, value >> 16);
  write_uint8_t(info, value >> 24);
}

static void
write_uint64_t(Info *info, uint64_t value) {
  write_uint8_t(info, value);
  write_uint8_t(info, value >> 8);
  write_uint8_t(info, value >> 16);
  write_uint8_t(info, value >> 24);
  write_uint8_t(info, value >> 32);
  write_uint8_t(info, value >> 40);
  write_uint8_t(info, value >> 48);
  write_uint8_t(info, value >> 56);
}

static void
write_int16_t(Info *info, int16_t value) {
  write_uint16_t(info, (uint16_t)value);
}

static void
write_int32_t(Info *info, int32_t value) {
  write_uint32_t(info, (uint32_t)value);
}

static void
write_int64_t(Info *info, int64_t value) {
  write_uint64_t(info, (uint64_t)value);
}

static void
write_float32(Info *info, float value) {
  uint32_t rep;
  memcpy(&rep, &value, sizeof(float));
  write_uint32_t(info, rep);
}

static void
write_float64(Info *info, double value) {
  uint64_t rep;
  memcpy(&rep, &value, sizeof(double));
  write_uint64_t(info, rep);
}

/* Note regarding the following: any decent compiler should be able
 * to reduce these to just the write call, since sizeof is constant. */

static void
write_int(Info *info, int value) {
  if (sizeof(int) == sizeof(int16_t)) {
    write_int16_t(info, value);
  }
  else if (sizeof(int) == sizeof(int32_t)) {
    write_int32_t(info, value);
  }
  else if (sizeof(int) == sizeof(int64_t)) {
    write_int64_t(info, value);
  }
  else {
    eris_error(info, ERIS_ERR_TYPE_INT);
  }
}

static void
write_size_t(Info *info, size_t value) {
  if (sizeof(size_t) == sizeof(uint16_t)) {
    write_uint16_t(info, value);
  }
  else if (sizeof(size_t) == sizeof(uint32_t)) {
    write_uint32_t(info, value);
  }
  else if (sizeof(size_t) == sizeof(uint64_t)) {
    write_uint64_t(info, value);
  }
  else {
    eris_error(info, ERIS_ERR_TYPE_SIZE);
  }
}

static void
write_lua_Number(Info *info, lua_Number value) {
  if (sizeof(lua_Number) == sizeof(uint32_t)) {
    write_float32(info, value);
  }
  else if (sizeof(lua_Number) == sizeof(uint64_t)) {
    write_float64(info, value);
  }
  else {
    eris_error(info, ERIS_ERR_TYPE_FLOAT);
  }
}

static void
write_lua_Integer(Info *info, lua_Integer value) {
  if (sizeof(lua_Integer) == sizeof(uint32_t)) {
    write_uint32_t(info, value);
  }
  else if (sizeof(lua_Integer) == sizeof(uint64_t)) {
    write_uint64_t(info, value);
  }
  else {
    eris_error(info, ERIS_ERR_TYPE_INT);
  }
}

/* Note that Lua only ever uses 32 bits of the Instruction type, so we can
 * assert that there will be no truncation, even if the underlying type has
 * more bits (might be the case on some 64 bit systems). */

static void
write_Instruction(Info *info, Instruction value) {
  if (sizeof(Instruction) == sizeof(uint32_t)) {
    write_uint32_t(info, value);
  }
  else {
    uint32_t pvalue = (uint32_t)value;
    /* Lua only uses 32 bits for its instructions. */
    eris_assert((Instruction)pvalue == value);
    write_uint32_t(info, pvalue);
  }
}

/** ======================================================================== */

static uint8_t
read_uint8_t(Info *info) {
  uint8_t value;
  READ_RAW(&value, sizeof(uint8_t));
  return value;
}

static uint16_t
read_uint16_t(Info *info) {
  return  (uint16_t)read_uint8_t(info) |
         ((uint16_t)read_uint8_t(info) << 8);
}

static uint32_t
read_uint32_t(Info *info) {
  return  (uint32_t)read_uint8_t(info) |
         ((uint32_t)read_uint8_t(info) << 8) |
         ((uint32_t)read_uint8_t(info) << 16) |
         ((uint32_t)read_uint8_t(info) << 24);
}

static uint64_t
read_uint64_t(Info *info) {
  return  (uint64_t)read_uint8_t(info) |
         ((uint64_t)read_uint8_t(info) << 8) |
         ((uint64_t)read_uint8_t(info) << 16) |
         ((uint64_t)read_uint8_t(info) << 24) |
         ((uint64_t)read_uint8_t(info) << 32) |
         ((uint64_t)read_uint8_t(info) << 40) |
         ((uint64_t)read_uint8_t(info) << 48) |
         ((uint64_t)read_uint8_t(info) << 56);
}

static int16_t
read_int16_t(Info *info) {
  return (int16_t)read_uint16_t(info);
}

static int32_t
read_int32_t(Info *info) {
  return (int32_t)read_uint32_t(info);
}

static int64_t
read_int64_t(Info *info) {
  return (int64_t)read_uint64_t(info);
}

static float
read_float32(Info *info) {
  float value;
  uint32_t rep = read_uint32_t(info);
  memcpy(&value, &rep, sizeof(float));
  return value;
}

static double
read_float64(Info *info) {
  double value;
  uint64_t rep = read_uint64_t(info);
  memcpy(&value, &rep, sizeof(double));
  return value;
}

/* Note regarding the following: unlike with writing the sizeof check will be
 * impossible to optimize away, since it depends on the input; however, the
 * truncation check may be optimized away in the case where the read data size
 * equals the native one, so reading data written on the same machine should be
 * reasonably quick. Doing a (rather rudimentary) benchmark this did not have
 * any measurable impact on performance. */

static int
read_int(Info *info) {
  int value;
  if (info->u.upi.sizeof_int == sizeof(int16_t)) {
    int16_t pvalue = read_int16_t(info);
    value = (int)pvalue;
    if ((int32_t)value != pvalue) {
      eris_error(info, ERIS_ERR_TRUNC_INT);
    }
  }
  else if (info->u.upi.sizeof_int == sizeof(int32_t)) {
    int32_t pvalue = read_int32_t(info);
    value = (int)pvalue;
    if ((int32_t)value != pvalue) {
      eris_error(info, ERIS_ERR_TRUNC_INT);
    }
  }
  else if (info->u.upi.sizeof_int == sizeof(int64_t)) {
    int64_t pvalue = read_int64_t(info);
    value = (int)pvalue;
    if ((int64_t)value != pvalue) {
      eris_error(info, ERIS_ERR_TRUNC_INT);
    }
  }
  else {
    eris_error(info, ERIS_ERR_TYPE_INT);
    value = 0; /* not reached */
  }
  return value;
}

static size_t
read_size_t(Info *info) {
  size_t value;
  if (info->u.upi.sizeof_size_t == sizeof(uint16_t)) {
    uint16_t pvalue = read_uint16_t(info);
    value = (size_t)pvalue;
    if ((uint32_t)value != pvalue) {
      eris_error(info, ERIS_ERR_TRUNC_SIZE);
    }
  }
  else if (info->u.upi.sizeof_size_t == sizeof(uint32_t)) {
    uint32_t pvalue = read_uint32_t(info);
    value = (size_t)pvalue;
    if ((uint32_t)value != pvalue) {
      eris_error(info, ERIS_ERR_TRUNC_SIZE);
    }
  }
  else if (info->u.upi.sizeof_size_t == sizeof(uint64_t)) {
    uint64_t pvalue = read_uint64_t(info);
    value = (size_t)pvalue;
    if ((uint64_t)value != pvalue) {
      eris_error(info, ERIS_ERR_TRUNC_SIZE);
    }
  }
  else {
    eris_error(info, ERIS_ERR_TYPE_SIZE);
    value = 0; /* not reached */
  }
  return value;
}

static lua_Number
read_lua_Number(Info *info) {
  if (sizeof(lua_Number) == sizeof(uint32_t)) {
    return read_float32(info);
  }
  else if (sizeof(lua_Number) == sizeof(uint64_t)) {
    return read_float64(info);
  }
  else {
    eris_error(info, ERIS_ERR_TYPE_FLOAT);
    return 0; /* not reached */
  }
}

static lua_Integer
read_lua_Integer(Info *info) {
  if (sizeof(lua_Integer) == sizeof(uint32_t)) {
    return (lua_Integer) read_uint32_t(info);
  }
  else if (sizeof(lua_Integer) == sizeof(uint64_t)) {
    return (lua_Integer) read_uint64_t(info);
  }
  else {
    eris_error(info, ERIS_ERR_TYPE_INT);
    return 0; /* not reached */
  }
}

static Instruction
read_Instruction(Info *info) {
  return (Instruction)read_uint32_t(info);
}

/** ======================================================================== */

/* Forward declarations for recursively called top-level functions. */
static void persist_keyed(Info*, int type);
static void persist(Info*);
static void unpersist(Info*);

/*
** ============================================================================
** Simple types.
** ============================================================================
*/

static void
p_boolean(Info *info) {                                           /* ... bool */
  WRITE_VALUE(lua_toboolean(info->L, -1), uint8_t);
}

static void
u_boolean(Info *info) {                                                /* ... */
  eris_checkstack(info->L, 1);
  lua_pushboolean(info->L, READ_VALUE(uint8_t));                  /* ... bool */

  eris_assert(lua_type(info->L, -1) == LUA_TBOOLEAN);
}

/** ======================================================================== */

static void
p_pointer(Info *info) {                                         /* ... ludata */
  WRITE_VALUE((size_t)lua_touserdata(info->L, -1), size_t);
}

static void
u_pointer(Info *info) {                                                /* ... */
  eris_checkstack(info->L, 1);
  lua_pushlightuserdata(info->L, (void*)READ_VALUE(size_t));    /* ... ludata */

  eris_assert(lua_type(info->L, -1) == LUA_TLIGHTUSERDATA);
}

/** ======================================================================== */

static void
p_number(Info *info) {                                             /* ... num */
  if (lua_isinteger(info->L, -1)) {
    WRITE_VALUE(true, uint8_t);
    WRITE_VALUE(lua_tointeger(info->L, -1), lua_Integer);
  }
  else {
    WRITE_VALUE(false, uint8_t);
    WRITE_VALUE(lua_tonumber(info->L, -1), lua_Number);
  }
}

static void
u_number(Info *info) {                                                 /* ... */
  eris_checkstack(info->L, 1);
  if (READ_VALUE(uint8_t)) {
    lua_pushinteger(info->L, READ_VALUE(lua_Integer));
  }
  else {
    lua_pushnumber(info->L, READ_VALUE(lua_Number));               /* ... num */
  }

  eris_assert(lua_type(info->L, -1) == LUA_TNUMBER);
}

/** ======================================================================== */

static void
p_string(Info *info) {                                             /* ... str */
  size_t length;
  const char *value = lua_tolstring(info->L, -1, &length);
  WRITE_VALUE(length, size_t);
  WRITE_RAW(value, length);
}

static void
u_string(Info *info) {                                                 /* ... */
  eris_checkstack(info->L, 2);
  {
    /* TODO Can we avoid this copy somehow? (Without it getting too nasty) */
    const size_t length = READ_VALUE(size_t);
    char *value = lua_newuserdata(info->L, length * sizeof(char)); /* ... tmp */
    READ_RAW(value, length);
    lua_pushlstring(info->L, value, length);                   /* ... tmp str */
    lua_replace(info->L, -2);                                      /* ... str */
  }
  registerobject(info);

  eris_assert(lua_type(info->L, -1) == LUA_TSTRING);
}

/*
** ============================================================================
** Tables and userdata.
** ============================================================================
*/

static void
p_metatable(Info *info) {                                          /* ... obj */
  eris_checkstack(info->L, 1);
  pushpath(info, "@metatable");
  if (!lua_getmetatable(info->L, -1)) {                        /* ... obj mt? */
    lua_pushnil(info->L);                                      /* ... obj nil */
  }                                                         /* ... obj mt/nil */
  persist(info);                                            /* ... obj mt/nil */
  lua_pop(info->L, 1);                                             /* ... obj */
  poppath(info);
}

static void
u_metatable(Info *info) {                                          /* ... tbl */
  eris_checkstack(info->L, 1);
  pushpath(info, "@metatable");
  unpersist(info);                                         /* ... tbl mt/nil? */
  if (lua_istable(info->L, -1)) {                               /* ... tbl mt */
    lua_setmetatable(info->L, -2);                                 /* ... tbl */
  }
  else if (lua_isnil(info->L, -1)) {                           /* ... tbl nil */
    lua_pop(info->L, 1);                                           /* ... tbl */
  }
  else {                                                            /* tbl :( */
    eris_error(info, ERIS_ERR_METATABLE);
  }
  poppath(info);
}

/** ======================================================================== */

static void
p_literaltable(Info *info) {                                       /* ... tbl */
  eris_checkstack(info->L, 3);

  /* Persist all key / value pairs. */
  lua_pushnil(info->L);                                        /* ... tbl nil */
  while (lua_next(info->L, -2)) {                              /* ... tbl k v */
    lua_pushvalue(info->L, -2);                              /* ... tbl k v k */

    if (info->generatePath) {
      if (lua_type(info->L, -1) == LUA_TSTRING) {
        const char *key = lua_tostring(info->L, -1);
        pushpath(info, ".%s", key);
      }
      else {
        const char *key = luaL_tolstring(info->L, -1, NULL);
        pushpath(info, "[%s]", key);
        lua_pop(info->L, 1);
      }
    }

    persist(info);                                           /* ... tbl k v k */
    lua_pop(info->L, 1);                                       /* ... tbl k v */
    persist(info);                                             /* ... tbl k v */
    lua_pop(info->L, 1);                                         /* ... tbl k */

    poppath(info);
  }                                                                /* ... tbl */

  /* Terminate list. */
  lua_pushnil(info->L);                                        /* ... tbl nil */
  persist(info);                                               /* ... tbl nil */
  lua_pop(info->L, 1);                                             /* ... tbl */

  p_metatable(info);
}

static void
u_literaltable(Info *info) {                                           /* ... */
  eris_checkstack(info->L, 3);

  lua_newtable(info->L);                                           /* ... tbl */

  /* Preregister table for handling of cycles (keys, values or metatable). */
  registerobject(info);

  /* Unpersist all key / value pairs. */
  for (;;) {
    pushpath(info, "@key");
    unpersist(info);                                       /* ... tbl key/nil */
    poppath(info);
    if (lua_isnil(info->L, -1)) {                              /* ... tbl nil */
      lua_pop(info->L, 1);                                         /* ... tbl */
      break;
    }                                                          /* ... tbl key */

    if (info->generatePath) {
      if (lua_type(info->L, -1) == LUA_TSTRING) {
        const char *key = lua_tostring(info->L, -1);
        pushpath(info, ".%s", key);
      }
      else {
        const char *key = luaL_tolstring(info->L, -1, NULL);
        pushpath(info, "[%s]", key);
        lua_pop(info->L, 1);
      }
    }

    unpersist(info);                                    /* ... tbl key value? */
    if (!lua_isnil(info->L, -1)) {                       /* ... tbl key value */
      lua_rawset(info->L, -3);                                     /* ... tbl */
    }
    else {
      eris_error(info, ERIS_ERR_TABLE);
    }

    poppath(info);
  }

  u_metatable(info);                                               /* ... tbl */
}

/** ======================================================================== */

static void
p_literaluserdata(Info *info) {                                  /* ... udata */
  const size_t size = lua_rawlen(info->L, -1);
  const void *value = lua_touserdata(info->L, -1);
  WRITE_VALUE(size, size_t);
  WRITE_RAW(value, size);
  p_metatable(info);                                             /* ... udata */
}

static void
u_literaluserdata(Info *info) {                                        /* ... */
  eris_checkstack(info->L, 1);
  {
    size_t size = READ_VALUE(size_t);
    void *value = lua_newuserdata(info->L, size);                /* ... udata */
    READ_RAW(value, size);                                       /* ... udata */
  }
  registerobject(info);
  u_metatable(info);
}

/** ======================================================================== */

typedef void (*Callback) (Info*);

static void
p_special(Info *info, Callback literal) {                          /* ... obj */
  int allow = (lua_type(info->L, -1) == LUA_TTABLE);
  eris_checkstack(info->L, 4);

  /* Check whether we should persist literally, or via the metafunction. */
  if (lua_getmetatable(info->L, -1)) {                          /* ... obj mt */
    lua_pushstring(info->L, info->u.pi.metafield);         /* ... obj mt pkey */
    lua_rawget(info->L, -2);                           /* ... obj mt persist? */
    switch (lua_type(info->L, -1)) {
      /* No entry, act according to default. */
      case LUA_TNIL:                                        /* ... obj mt nil */
        lua_pop(info->L, 2);                                       /* ... obj */
        break;

      /* Boolean value, tells us whether allowed or not. */
      case LUA_TBOOLEAN:                                   /* ... obj mt bool */
        allow = lua_toboolean(info->L, -1);
        lua_pop(info->L, 2);                                       /* ... obj */
        break;

      /* Function value, call it and don't persist literally. */
      case LUA_TFUNCTION:                                  /* ... obj mt func */
        lua_replace(info->L, -2);                             /* ... obj func */
        lua_pushvalue(info->L, -2);                       /* ... obj func obj */

        if (info->passIOToPersist) {
          lua_pushlightuserdata(info->L, info->u.pi.writer);
                                                   /* ... obj func obj writer */
          lua_pushlightuserdata(info->L, info->u.pi.ud);
                                                /* ... obj func obj writer ud */
          lua_call(info->L, 3, 1);                           /* ... obj func? */
        }
        else {
          lua_call(info->L, 1, 1);                           /* ... obj func? */
        }
        if (!lua_isfunction(info->L, -1)) {                     /* ... obj :( */
          eris_error(info, ERIS_ERR_SPER_FUNC, info->u.pi.metafield);
        }                                                     /* ... obj func */

        /* Special persistence, call this function when unpersisting. */
        WRITE_VALUE(true, uint8_t);
        persist(info);                                        /* ... obj func */
        lua_pop(info->L, 1);                                       /* ... obj */
        return;
      default:                                               /* ... obj mt :( */
        eris_error(info, ERIS_ERR_SPER_TYPE, info->u.pi.metafield);
        return; /* not reached */
    }
  }

  if (allow) {
    /* Not special but literally persisted object. */
    WRITE_VALUE(false, uint8_t);
    literal(info);                                                 /* ... obj */
  }
  else if (lua_type(info->L, -1) == LUA_TTABLE) {
    eris_error(info, ERIS_ERR_SPER_PROT);
  }
  else {
    eris_error(info, ERIS_ERR_USERDATA);
  }
}

static void
u_special(Info *info, int type, Callback literal) {                    /* ... */
  eris_checkstack(info->L, 2);
  if (READ_VALUE(uint8_t)) {
    int reference;
    /* Reserve entry in the reftable before unpersisting the function to keep
     * the reference order intact. We can set this to nil at first, because
     * there's no way the special function would access this. */
    lua_pushnil(info->L);                                          /* ... nil */
    reference = registerobject(info);
    lua_pop(info->L, 1);                                               /* ... */
    /* Increment reference counter by one to compensate for the increment when
     * persisting a special object. */
    unpersist(info);                                           /* ... spfunc? */
    if (!lua_isfunction(info->L, -1)) {                             /* ... :( */
      eris_error(info, ERIS_ERR_SPER_UFUNC);
    }                                                           /* ... spfunc */

    if (info->passIOToPersist) {
      lua_pushlightuserdata(info->L, &info->u.upi.zio);     /* ... spfunc zio */
      lua_call(info->L, 1, 1);                                    /* ... obj? */
    } else {
      lua_call(info->L, 0, 1);                                    /* ... obj? */
    }

    if (lua_type(info->L, -1) != type) {                            /* ... :( */
      const char *want = kTypenames[type];
      const char *have = kTypenames[lua_type(info->L, -1)];
      eris_error(info, ERIS_ERR_SPER_LOAD, want, have);
    }                                                              /* ... obj */

    /* Update the reftable entry. */
    lua_pushvalue(info->L, -1);                                /* ... obj obj */
    lua_rawseti(info->L, 2, reference);                            /* ... obj */
  }
  else {
    literal(info);                                                 /* ... obj */
  }
}

/** ======================================================================== */

static void
p_table(Info *info) {                                              /* ... tbl */
  p_special(info, p_literaltable);                                 /* ... tbl */
}

static void
u_table(Info *info) {                                                  /* ... */
  u_special(info, LUA_TTABLE, u_literaltable);                     /* ... tbl */

  eris_assert(lua_type(info->L, -1) == LUA_TTABLE);
}

/** ======================================================================== */

static void
p_userdata(Info *info) {                            /* perms reftbl ... udata */
  p_special(info, p_literaluserdata);
}

static void
u_userdata(Info *info) {                                               /* ... */
  u_special(info, LUA_TUSERDATA, u_literaluserdata);             /* ... udata */

  eris_assert(lua_type(info->L, -1) == LUA_TUSERDATA);
}

/*
** ============================================================================
** Closures and threads.
** ============================================================================
*/

/* We track the actual upvalues themselves by pushing their "id" (meaning a
 * pointer to them) as lightuserdata to the reftable. This is safe because
 * lightuserdata will not normally end up in there, because simple value types
 * are always persisted directly (because that'll be just as large, memory-
 * wise as when pointing to the first instance). Same for protos. */

static void
p_proto(Info *info) {                                            /* ... proto */
  int i;
  const Proto *p = lua_touserdata(info->L, -1);
  eris_checkstack(info->L, 3);

  /* Write general information. */
  WRITE_VALUE(p->linedefined, int);
  WRITE_VALUE(p->lastlinedefined, int);
  WRITE_VALUE(p->numparams, uint8_t);
  WRITE_VALUE(p->is_vararg, uint8_t);
  WRITE_VALUE(p->maxstacksize, uint8_t);

  /* Write byte code. */
  WRITE_VALUE(p->sizecode, int);
  WRITE(p->code, p->sizecode, Instruction);

  /* Write constants. */
  WRITE_VALUE(p->sizek, int);
  pushpath(info, ".constants");
  for (i = 0; i < p->sizek; ++i) {
    pushpath(info, "[%d]", i);
    eris_setobj(info->L, info->L->top++, &p->k[i]);      /* ... lcl proto obj */
    persist(info);                                       /* ... lcl proto obj */
    lua_pop(info->L, 1);                                     /* ... lcl proto */
    poppath(info);
  }
  poppath(info);

  /* Write child protos. */
  WRITE_VALUE(p->sizep, int);
  pushpath(info, ".protos");
  for (i = 0; i < p->sizep; ++i) {
    pushpath(info, "[%d]", i);
    lua_pushlightuserdata(info->L, p->p[i]);           /* ... lcl proto proto */
    lua_pushvalue(info->L, -1);                  /* ... lcl proto proto proto */
    persist_keyed(info, LUA_TPROTO);                   /* ... lcl proto proto */
    lua_pop(info->L, 1);                                     /* ... lcl proto */
    poppath(info);
  }
  poppath(info);

  /* Write upvalues. */
  WRITE_VALUE(p->sizeupvalues, int);
  for (i = 0; i < p->sizeupvalues; ++i) {
    WRITE_VALUE(p->upvalues[i].instack, uint8_t);
    WRITE_VALUE(p->upvalues[i].idx, uint8_t);
  }

  /* If we don't have to persist debug information skip the rest. */
  WRITE_VALUE(info->u.pi.writeDebugInfo, uint8_t);
  if (!info->u.pi.writeDebugInfo) {
    return;
  }

  /* Write function source code. */
  pushtstring(info->L, p->source);                    /* ... lcl proto source */
  persist(info);                                      /* ... lcl proto source */
  lua_pop(info->L, 1);                                       /* ... lcl proto */

  /* Write line information. */
  WRITE_VALUE(p->sizelineinfo, int);
  WRITE(p->lineinfo, p->sizelineinfo, int);

  /* Write locals info. */
  WRITE_VALUE(p->sizelocvars, int);
  pushpath(info, ".locvars");
  for (i = 0; i < p->sizelocvars; ++i) {
    pushpath(info, "[%d]", i);
    WRITE_VALUE(p->locvars[i].startpc, int);
    WRITE_VALUE(p->locvars[i].endpc, int);
    pushtstring(info->L, p->locvars[i].varname);     /* ... lcl proto varname */
    persist(info);                                   /* ... lcl proto varname */
    lua_pop(info->L, 1);                                     /* ... lcl proto */
    poppath(info);
  }
  poppath(info);

  /* Write upvalue names. */
  pushpath(info, ".upvalnames");
  for (i = 0; i < p->sizeupvalues; ++i) {
    pushpath(info, "[%d]", i);
    pushtstring(info->L, p->upvalues[i].name);          /* ... lcl proto name */
    persist(info);                                      /* ... lcl proto name */
    lua_pop(info->L, 1);                                     /* ... lcl proto */
    poppath(info);
  }
  poppath(info);
}

static void
u_proto(Info *info) {                                            /* ... proto */
  int i, n;
  Proto *p = lua_touserdata(info->L, -1);
  eris_assert(p);

  eris_checkstack(info->L, 2);

  /* Preregister proto for handling of cycles (probably impossible, but
   * maybe via the constants of the proto... not worth taking the risk). */
  registerobject(info);

  /* Read general information. */
  p->linedefined = READ_VALUE(int);
  p->lastlinedefined = READ_VALUE(int);
  p->numparams = READ_VALUE(uint8_t);
  p->is_vararg = READ_VALUE(uint8_t);
  p->maxstacksize = READ_VALUE(uint8_t);

  /* Read byte code. */
  p->sizecode = READ_VALUE(int);
  eris_reallocvector(info->L, p->code, 0, p->sizecode, Instruction);
  READ(p->code, p->sizecode, Instruction);

  /* Read constants. */
  p->sizek = READ_VALUE(int);
  eris_reallocvector(info->L, p->k, 0, p->sizek, TValue);
  /* Set all values to nil to avoid confusing the GC. */
  for (i = 0, n = p->sizek; i < n; ++i) {
    eris_setnilvalue(&p->k[i]);
  }
  pushpath(info, ".constants");
  for (i = 0, n = p->sizek; i < n; ++i) {
    pushpath(info, "[%d]", i);
    unpersist(info);                                         /* ... proto obj */
    eris_setobj(info->L, &p->k[i], info->L->top - 1);
    lua_pop(info->L, 1);                                         /* ... proto */
    poppath(info);
  }
  poppath(info);

  /* Read child protos. */
  p->sizep = READ_VALUE(int);
  eris_reallocvector(info->L, p->p, 0, p->sizep, Proto*);
  /* Null all entries to avoid confusing the GC. */
  memset(p->p, 0, p->sizep * sizeof(Proto*));
  pushpath(info, ".protos");
  for (i = 0, n = p->sizep; i < n; ++i) {
    Proto *cp;
    pushpath(info, "[%d]", i);
    p->p[i] = eris_newproto(info->L);
    lua_pushlightuserdata(info->L, p->p[i]);              /* ... proto nproto */
    unpersist(info);                        /* ... proto nproto nproto/oproto */
    cp = lua_touserdata(info->L, -1);
    if (cp != p->p[i]) {                           /* ... proto nproto oproto */
      /* Just overwrite it, GC will clean this up. */
      p->p[i] = cp;
    }
    lua_pop(info->L, 2);                                         /* ... proto */
    poppath(info);
  }
  poppath(info);

  /* Read upvalues. */
  p->sizeupvalues = READ_VALUE(int);
  eris_reallocvector(info->L, p->upvalues, 0, p->sizeupvalues, Upvaldesc);
  for (i = 0, n = p->sizeupvalues; i < n; ++i) {
    p->upvalues[i].name = NULL;
    p->upvalues[i].instack = READ_VALUE(uint8_t);
    p->upvalues[i].idx = READ_VALUE(uint8_t);
  }

  /* Read debug information if any is present. */
  if (!READ_VALUE(uint8_t)) {
    return;
  }

  /* Read function source code. */
  unpersist(info);                                           /* ... proto str */
  copytstring(info->L, &p->source);
  lua_pop(info->L, 1);                                           /* ... proto */

  /* Read line information. */
  p->sizelineinfo = READ_VALUE(int);
  eris_reallocvector(info->L, p->lineinfo, 0, p->sizelineinfo, int);
  READ(p->lineinfo, p->sizelineinfo, int);

  /* Read locals info. */
  p->sizelocvars = READ_VALUE(int);
  eris_reallocvector(info->L, p->locvars, 0, p->sizelocvars, LocVar);
  /* Null the variable names to avoid confusing the GC. */
  for (i = 0, n = p->sizelocvars; i < n; ++i) {
    p->locvars[i].varname = NULL;
  }
  pushpath(info, ".locvars");
  for (i = 0, n = p->sizelocvars; i < n; ++i) {
    pushpath(info, "[%d]", i);
    p->locvars[i].startpc = READ_VALUE(int);
    p->locvars[i].endpc = READ_VALUE(int);
    unpersist(info);                                         /* ... proto str */
    copytstring(info->L, &p->locvars[i].varname);
    lua_pop(info->L, 1);                                         /* ... proto */
    poppath(info);
  }
  poppath(info);

  /* Read upvalue names. */
  pushpath(info, ".upvalnames");
  for (i = 0, n = p->sizeupvalues; i < n; ++i) {
    pushpath(info, "[%d]", i);
    unpersist(info);                                         /* ... proto str */
    copytstring(info->L, &p->upvalues[i].name);
    lua_pop(info->L, 1);                                         /* ... proto */
    poppath(info);
  }
  poppath(info);
  lua_pushvalue(info->L, -1);                              /* ... proto proto */

  eris_assert(lua_type(info->L, -1) == LUA_TLIGHTUSERDATA);
}

/** ======================================================================== */

static void
p_upval(Info *info) {                                              /* ... obj */
  persist(info);                                                   /* ... obj */
}

static void
u_upval(Info *info) {                                                  /* ... */
  eris_checkstack(info->L, 2);

  /* Create the table we use to store the stack location to the upval (1+2),
   * the value of the upval (3) and any references to the upvalue's value (4+).
   * References are stored as two entries each, the actual closure holding the
   * upvalue, and the index of the upvalue in that closure. */
  lua_createtable(info->L, 5, 0);                                  /* ... tbl */
  registerobject(info);
  unpersist(info);                                             /* ... tbl obj */
  lua_rawseti(info->L, -2, UVTVAL);                                /* ... tbl */

  eris_assert(lua_type(info->L, -1) == LUA_TTABLE);
}

/** ======================================================================== */

/* For Lua closures we write the upvalue ID, which is usually the memory
 * address at which it is stored. This is used to tell which upvalues are
 * identical when unpersisting. */
/* In either case we store the upvale *values*, i.e. the actual objects they
 * point to. As in Pluto, we will restore any upvalues of Lua closures as
 * closed as first, i.e. the upvalue will store the TValue itself. When
 * loading a thread containing the upvalue (meaning it's the actual owner of
 * the upvalue) we open it, i.e. we point it to the thread's upvalue list.
 * For C closures, upvalues are always closed. */
static void
p_closure(Info *info) {                              /* perms reftbl ... func */
  int nup;
  eris_checkstack(info->L, 2);
  switch (ttype(info->L->top - 1)) {
    case LUA_TLCF: /* light C function */
      /* We cannot persist these, they have to be handled via the permtable. */
      eris_error(info, ERIS_ERR_CFUNC, lua_tocfunction(info->L, -1));
      return; /* not reached */
    case LUA_TCCL: /* C closure */ {                  /* perms reftbl ... ccl */
      CClosure *cl = clCvalue(info->L->top - 1);
      /* Mark it as a C closure. */
      WRITE_VALUE(true, uint8_t);
      /* Write the upvalue count first, since we have to know it when creating
       * a new closure when unpersisting. */
      WRITE_VALUE(cl->nupvalues, uint8_t);

      /* We can only persist these if the underlying C function is in the
       * permtable. So we try to persist it first as a light C function. If it
       * isn't in the permtable that'll cause an error (in the case above). */
      lua_pushcfunction(info->L, lua_tocfunction(info->L, -1));
                                                /* perms reftbl ... ccl cfunc */
      persist(info);                            /* perms reftbl ... ccl cfunc */
      lua_pop(info->L, 1);                            /* perms reftbl ... ccl */

      /* Persist the upvalues. Since for C closures all upvalues are always
       * closed we can just write the actual values. */
      pushpath(info, ".upvalues");
      for (nup = 1; nup <= cl->nupvalues; ++nup) {
        pushpath(info, "[%d]", nup);
        lua_getupvalue(info->L, -1, nup);         /* perms reftbl ... ccl obj */
        persist(info);                            /* perms reftbl ... ccl obj */
        lua_pop(info->L, 1);                          /* perms reftbl ... ccl */
        poppath(info);
      }
      poppath(info);
      break;
    }
    case LUA_TLCL: /* Lua function */ {               /* perms reftbl ... lcl */
      LClosure *cl = eris_clLvalue(info->L->top - 1);
      /* Mark it as a Lua closure. */
      WRITE_VALUE(false, uint8_t);
      /* Write the upvalue count first, since we have to know it when creating
       * a new closure when unpersisting. */
      WRITE_VALUE(cl->nupvalues, uint8_t);

      /* Persist the function's prototype. Pass the proto as a parameter to
       * p_proto so that it can access it and register it in the ref table. */
      pushpath(info, ".proto");
      lua_pushlightuserdata(info->L, cl->p);    /* perms reftbl ... lcl proto */
      lua_pushvalue(info->L, -1);         /* perms reftbl ... lcl proto proto */
      persist_keyed(info, LUA_TPROTO);          /* perms reftbl ... lcl proto */
      lua_pop(info->L, 1);                            /* perms reftbl ... lcl */
      poppath(info);

      /* Persist the upvalues. We pretend to write these as their own type,
       * to get proper identity preservation. We also pass them as a parameter
       * to p_upval so it can register the upvalue in the reference table. */
      pushpath(info, ".upvalues");
      for (nup = 1; nup <= cl->nupvalues; ++nup) {
        const char *name = lua_getupvalue(info->L, -1, nup);
                                                  /* perms reftbl ... lcl obj */
        pushpath(info, ".%s", name);
        lua_pushlightuserdata(info->L, lua_upvalueid(info->L, -2, nup));
                                               /* perms reftbl ... lcl obj id */
        persist_keyed(info, LUA_TUPVAL);          /* perms reftbl ... lcl obj */
        lua_pop(info->L, 1);                         /* perms reftble ... lcl */
        poppath(info);
      }
      poppath(info);
      break;
    }
    default:
      eris_error(info, ERIS_ERR_NOFUNC);
      return; /* not reached */
  }
}

static void
u_closure(Info *info) {                                                /* ... */
  int nup;
  bool isCClosure = READ_VALUE(uint8_t);
  lu_byte nups = READ_VALUE(uint8_t);
  if (isCClosure) {
    lua_CFunction f;

    /* Reserve reference for the closure to avoid light C function or its
     * perm table key going first. */
    const int reference = ++(info->refcount);

    /* nups is guaranteed to be >= 1, otherwise it'd be a light C function. */
    eris_checkstack(info->L, nups < 2 ? 2 : nups);

    /* Read the C function from the permanents table. */
    unpersist(info);                                             /* ... cfunc */
    if (!lua_iscfunction(info->L, -1)) {
      eris_error(info, ERIS_ERR_UCFUNC, kTypenames[lua_type(info->L, -1)]);
    }
    f = lua_tocfunction(info->L, -1);
    if (!f) {
      eris_error(info, ERIS_ERR_UCFUNCNULL);
    }
    lua_pop(info->L, 1);                                               /* ... */

    /* Now this is a little roundabout, but we want to create the closure
     * before unpersisting the actual upvalues to avoid cycles. So we have to
     * create it with all nil first, then fill the upvalues in afterwards. */
    for (nup = 1; nup <= nups; ++nup) {
      lua_pushnil(info->L);                        /* ... nil[1] ... nil[nup] */
    }
    lua_pushcclosure(info->L, f, nups);                            /* ... ccl */

    /* Create the entry in the reftable. */
    lua_pushvalue(info->L, -1);                   /* perms reftbl ... ccl ccl */
    lua_rawseti(info->L, REFTIDX, reference);         /* perms reftbl ... ccl */

    /* Unpersist actual upvalues. */
    pushpath(info, ".upvalues");
    for (nup = 1; nup <= nups; ++nup) {
      pushpath(info, "[%d]", nup);
      unpersist(info);                                         /* ... ccl obj */
      lua_setupvalue(info->L, -2, nup);                            /* ... ccl */
      poppath(info);
    }
    poppath(info);
  }
  else {
    LClosure *cl;
    Proto *p;

    eris_checkstack(info->L, 4);

    /* Create closure and anchor it on the stack (avoid collection via GC). */
    cl = eris_newLclosure(info->L, nups);
    eris_setclLvalue(info->L, info->L->top, cl);                   /* ... lcl */
    eris_incr_top(info->L);

    /* Preregister closure for handling of cycles (upvalues). */
    registerobject(info);

    /* Read prototype. In general, we create protos (and upvalues) before
     * trying to read them and pass a pointer to the instance along to the
     * unpersist function. This way the instance is safely hooked up to an
     * object, so we don't have to worry about it getting GCed. */
    pushpath(info, ".proto");
    cl->p = eris_newproto(info->L);
    /* Push the proto into which to unpersist as a parameter to u_proto. */
    lua_pushlightuserdata(info->L, cl->p);                /* ... lcl nproto */
    unpersist(info);                          /* ... lcl nproto nproto/oproto */
    eris_assert(lua_type(info->L, -1) == LUA_TLIGHTUSERDATA);
    /* The proto we have now may differ, if we already unpersisted it before.
     * In that case we now have a reference to the originally unpersisted
     * proto so we'll use that. */
    p = lua_touserdata(info->L, -1);
    if (p != cl->p) {                              /* ... lcl nproto oproto */
      /* Just overwrite the old one, GC will clean this up. */
      cl->p = p;
    }
    lua_pop(info->L, 2);                                           /* ... lcl */
    eris_assert(cl->nupvalues == cl->p->sizeupvalues);
    eris_initupvals(info->L, cl); /* Init to all closed, fill in later. */
    poppath(info);

    /* Unpersist all upvalues. */
    pushpath(info, ".upvalues");
    for (nup = 1; nup <= nups; ++nup) {
      UpVal **uv = &cl->upvals[nup - 1];
      /* Get the actual name of the upvalue, if possible. */
      if (p->upvalues[nup - 1].name) {
        pushpath(info, "[%s]", getstr(p->upvalues[nup - 1].name));
      }
      else {
        pushpath(info, "[%d]", nup);
      }
      unpersist(info);                                         /* ... lcl tbl */
      eris_assert(lua_type(info->L, -1) == LUA_TTABLE);
      lua_rawgeti(info->L, -1, UVTOCL);               /* ... lcl tbl olcl/nil */
      if (lua_isnil(info->L, -1)) {                        /* ... lcl tbl nil */
        lua_pop(info->L, 1);                                   /* ... lcl tbl */
        lua_pushvalue(info->L, -2);                        /* ... lcl tbl lcl */
        lua_rawseti(info->L, -2, UVTOCL);                      /* ... lcl tbl */
        lua_pushinteger(info->L, nup);                     /* ... lcl tbl nup */
        lua_rawseti(info->L, -2, UVTONU);                      /* ... lcl tbl */
      }
      else {                                              /* ... lcl tbl olcl */
        int onup;
        eris_assert(lua_type(info->L, -1) == LUA_TFUNCTION);
        lua_rawgeti(info->L, -2, UVTONU);            /* ... lcl tbl olcl onup */
        eris_assert(lua_type(info->L, -1) == LUA_TNUMBER);
        onup = lua_tointeger(info->L, -1);
        lua_pop(info->L, 1);                              /* ... lcl tbl olcl */
        lua_upvaluejoin(info->L, -3, nup, -1, onup);
        lua_pop(info->L, 1);                                   /* ... lcl tbl */
      }

      /* Set the upvalue's actual value and add our reference to the upvalue to
       * the list, for reference patching if we have to open the upvalue in
       * u_thread. Either is only necessary if the upvalue is still closed. */
      if (!upisopen(*uv)) {
        int i;
        /* Always update the value of the upvalue's value for closed upvalues,
         * even if we re-used one - if we had a cycle, it might have been
         * incorrectly initialized to nil before (or rather, not yet set). */
        lua_rawgeti(info->L, -1, UVTVAL);                  /* ... lcl tbl obj */
        eris_setobj(info->L, &(*uv)->u.value, info->L->top - 1);
        lua_pop(info->L, 1);                                   /* ... lcl tbl */

        lua_pushinteger(info->L, nup);                     /* ... lcl tbl nup */
        lua_pushvalue(info->L, -3);                    /* ... lcl tbl nup lcl */
        if (luaL_len(info->L, -3) >= UVTVAL) {
          /* Got a valid sequence (value already set), insert at the end. */
          i = luaL_len(info->L, -3);
          lua_rawseti(info->L, -3, i + 1);                 /* ... lcl tbl nup */
          lua_rawseti(info->L, -2, i + 2);                     /* ... lcl tbl */
        }
        else {                                         /* ... lcl tbl nup lcl */
          /* Find where to insert. This can happen if we have cycles, in which
           * case the table is not fully initialized at this point, i.e. the
           * value is not in it, yet (we work around that by always setting it,
           * as seen above). */
          for (i = UVTREF;; i += 2) {
            lua_rawgeti(info->L, -3, i);       /* ... lcl tbl nup lcl lcl/nil */
            if (lua_isnil(info->L, -1)) {          /* ... lcl tbl nup lcl nil */
              lua_pop(info->L, 1);                     /* ... lcl tbl nup lcl */
              lua_rawseti(info->L, -3, i);                 /* ... lcl tbl nup */
              lua_rawseti(info->L, -2, i + 1);                 /* ... lcl tbl */
              break;
            }
            else {
              lua_pop(info->L, 1);                     /* ... lcl tbl nup lcl */
            }
          }                                                    /* ... lcl tbl */
        }
      }

      lua_pop(info->L, 1);                                         /* ... lcl */
      poppath(info);
    }
    poppath(info);

    /* save it in cache for reuse, see lvm.c:416 */
    if (!isblack(p))
      p->cache = cl;
  }

  eris_assert(lua_type(info->L, -1) == LUA_TFUNCTION);
}

/** ======================================================================== */

static void
p_thread(Info *info) {                                          /* ... thread */
  lua_State* thread = lua_tothread(info->L, -1);
  size_t level = 0, total = thread->top - thread->stack;
  CallInfo *ci;
  UpVal *uv;

  eris_checkstack(info->L, 2);

  /* We cannot persist any running threads, because by definition we *are* that
   * running thread. And we use the stack. So yeah, really not a good idea. */
  if (thread == info->L) {
    eris_error(info, ERIS_ERR_THREAD);
    return; /* not reached */
  }

  /* Persist the stack. Save the total size and used space first. */
  WRITE_VALUE(thread->stacksize, int);
  WRITE_VALUE(total, size_t);

  /* The Lua stack looks like this:
   * stack ... top ... stack_last
   * Where stack <= top <= stack_last, and "top" actually being the first free
   * element, i.e. there's nothing stored there. So we stop one below that. */
  pushpath(info, ".stack");
  lua_pushnil(info->L);                                     /* ... thread nil */
  /* Since the thread's stack may be re-allocated in the meantime, we cannot
   * use pointer arithmetic here (i.e. o = thread->stack; ...; ++o). Instead we
   * have to keep track of our position in the stack directly (which we do for
   * the path info anyway) and compute the actual address each time.
   */
  for (; level < total; ++level) {
    pushpath(info, "[%d]", level);
    eris_setobj(info->L, info->L->top - 1, thread->stack + level);
                                                            /* ... thread obj */
    persist(info);                                          /* ... thread obj */
    poppath(info);
  }
  lua_pop(info->L, 1);                                          /* ... thread */
  poppath(info);

  /* If the thread isn't running this must be the default value, which is 1. */
  eris_assert(thread->nny == 1);

  /* Error jump info should only be set while thread is running. */
  eris_assert(thread->errorJmp == NULL);

  /* thread->oldpc always seems to be uninitialized, at least gdb always shows
   * it as 0xbaadf00d when I set a breakpoint here. */

  /* Write general information. */
  WRITE_VALUE(thread->status, uint8_t);
  WRITE_VALUE(eris_savestackidx(thread,
    eris_restorestack(thread, thread->errfunc)), size_t);
  /* These are only used while a thread is being executed or can be deduced:
  WRITE_VALUE(thread->nCcalls, uint16_t);
  WRITE_VALUE(thread->allowhook, uint8_t); */

  /* Hooks are not supported, bloody can of worms, those.
  WRITE_VALUE(thread->hookmask, uint8_t);
  WRITE_VALUE(thread->basehookcount, int);
  WRITE_VALUE(thread->hookcount, int); */

  if (thread->hook) {
    /* TODO Warn that hooks are not persisted? */
  }

  /* Write call information (stack frames). In 5.2 CallInfo is stored in a
   * linked list that originates in thead.base_ci. Upon initialization the
   * thread.ci is set to thread.base_ci. During thread calls this is extended
   * and always represents the tail of the callstack, though not necessarily of
   * the linked list (which can be longer if the callstack was deeper earlier,
   * but shrunk due to returns). */
  pushpath(info, ".callinfo");
  level = 0;
  eris_assert(&thread->base_ci != thread->ci->next);
  for (ci = &thread->base_ci; ci != thread->ci->next; ci = ci->next) {
    pushpath(info, "[%d]", level++);
    WRITE_VALUE(eris_savestackidx(thread, ci->func), size_t);
    WRITE_VALUE(eris_savestackidx(thread, ci->top), size_t);
    WRITE_VALUE(ci->nresults, int16_t);
    WRITE_VALUE(ci->callstatus, uint8_t);
    if (ci->callstatus) {
      /* CallInfo.extra is used in two contexts: if L->status == LUA_YIELD and
       * CallInfo is the one stored as L->ci, in which case ci->extra refers to
       * the original value of L->ci->func, and when we have a yieldable pcall
       * (ci->callstatus & CIST_YPCALL) where ci->extra holds the stack level
       * of the function being called (see lua_pcallk). We save the ci->extra
       * for L->ci after the loop, because we won't know which one it is when
       * unpersisting. */
      if (ci->callstatus & CIST_YPCALL) {
        WRITE_VALUE(eris_savestackidx(thread,
          eris_restorestack(thread, ci->extra)), size_t);
      }

      eris_assert(eris_isLua(ci) || (ci->callstatus & CIST_TAIL) == 0);
      if (ci->callstatus & CIST_HOOKYIELD) {
        eris_error(info, ERIS_ERR_HOOK);
      }

      if (eris_isLua(ci)) {
        const LClosure *lcl = eris_ci_func(ci);
        WRITE_VALUE(eris_savestackidx(thread, ci->u.l.base), size_t);
        WRITE_VALUE(ci->u.l.savedpc - lcl->p->code, size_t);
      }
      else {
        /* These are only used while a thread is being executed:
        WRITE_VALUE(ci->u.c.old_errfunc, ptrdiff_t); */
        if (thread->status == LUA_YIELD && ci->u.c.k) {
          WRITE_VALUE(true, uint8_t);
          WRITE_VALUE(ci->u.c.ctx, int);
          /* NOTE Ugly hack. We have to push the continuation function as a C
           * function to properly track it in our ref table. It's never called,
           * so we can get away with this. */
          lua_pushcfunction(info->L, (lua_CFunction) ci->u.c.k);
                                                             /* ... thread func */
          persist(info);                                 /* ... thread func/nil */
          lua_pop(info->L, 1);                                    /* ... thread */
        }
        else {
          WRITE_VALUE(false, uint8_t);
        }
      }
    }

    /* Write whether there's more to come. */
    WRITE_VALUE(ci->next == thread->ci->next, uint8_t);

    poppath(info);
  }
  /** See comment on ci->extra in loop. */
  if (thread->status == LUA_YIELD) {
    WRITE_VALUE(eris_savestackidx(thread,
        eris_restorestack(thread, thread->ci->extra)), size_t);
  }
  poppath(info);

  pushpath(info, ".openupval");
  lua_pushnil(info->L);                                     /* ... thread nil */
  level = 0;
  for (uv = thread->openupval;
       uv != NULL;
       uv = uv->u.open.next)
  {
    pushpath(info, "[%d]", level++);
    WRITE_VALUE(eris_savestackidx(thread, uv->v) + 1, size_t);
    eris_setobj(info->L, info->L->top - 1, uv->v);          /* ... thread obj */
    lua_pushlightuserdata(info->L, uv);                  /* ... thread obj id */
    persist_keyed(info, LUA_TUPVAL);                        /* ... thread obj */
    poppath(info);
  }
  WRITE_VALUE(0, size_t);
  lua_pop(info->L, 1);                                          /* ... thread */
  poppath(info);
}

/* Used in u_thread to validate read stack positions. */
#define validate(stackpos, inclmax) \
  if ((stackpos) < thread->stack || stackpos > (inclmax)) { \
    (stackpos) = thread->stack; \
    eris_error(info, ERIS_ERR_STACKBOUNDS); }

/* I had so hoped to get by without any 'hacks', but I surrender. We mark the
 * thread as incomplete to avoid the GC messing with it while we're building
 * it. Otherwise it may try to shrink its stack. We do this by setting its
 * stack field to null for every call that may trigger a GC run, since that
 * field is what's used to determine whether threads should be shrunk. See
 * lgc.c:699. Some of the locks could probably be joined (since nothing
 * inbetween requires the stack field to be valid), but I prefer to keep the
 * "invalid" blocks as small as possible to make it clearer. Also, locking and
 * unlocking are really just variable assignments, so they're really cheap. */
#define LOCK(L) (L->stack = NULL)
#define UNLOCK(L) (L->stack = stack)

static void
u_thread(Info *info) {                                                 /* ... */
  lua_State* thread;
  size_t level;
  StkId stack, o;

  eris_checkstack(info->L, 3);

  thread = lua_newthread(info->L);                              /* ... thread */
  registerobject(info);

  /* Unpersist the stack. Read size first and adjust accordingly. */
  eris_reallocstack(thread, READ_VALUE(int));
  stack = thread->stack; /* After the realloc in case the address changes. */
  thread->top = thread->stack + READ_VALUE(size_t);
  validate(thread->top, thread->stack_last);

  /* Read the elements one by one. */
  LOCK(thread);
  pushpath(info, ".stack");
  UNLOCK(thread);
  level = 0;
  for (o = stack; o < thread->top; ++o) {
    LOCK(thread);
    pushpath(info, "[%d]", level++);
    unpersist(info);                                        /* ... thread obj */
    UNLOCK(thread);
    eris_setobj(thread, o, info->L->top - 1);
    lua_pop(info->L, 1);                                        /* ... thread */
    LOCK(thread);
    poppath(info);
    UNLOCK(thread);
  }
  LOCK(thread);
  poppath(info);
  UNLOCK(thread);

  /* As in p_thread, just to make sure. */
  eris_assert(thread->nny == 1);
  eris_assert(thread->errorJmp == NULL);
  eris_assert(thread->hook == NULL);

  /* See comment in persist. */
  thread->oldpc = NULL;

  /* Read general information. */
  thread->status = READ_VALUE(uint8_t);
  thread->errfunc = eris_savestack(thread,
    eris_restorestackidx(thread, READ_VALUE(size_t)));
  if (thread->errfunc) {
    o = eris_restorestack(thread, thread->errfunc);
    validate(o, thread->top);
    if (eris_ttnov(o) != LUA_TFUNCTION) {
      eris_error(info, ERIS_ERR_THREADERRF);
    }
  }
  /* These are only used while a thread is being executed or can be deduced:
  thread->nCcalls = READ_VALUE(uint16_t);
  thread->allowhook = READ_VALUE(uint8_t); */
  eris_assert(thread->allowhook == 1);

  /* Not supported.
  thread->hookmask = READ_VALUE(uint8_t);
  thread->basehookcount = READ_VALUE(int);
  thread->hookcount = READ_VALUE(int); */

  /* Read call information (stack frames). */
  LOCK(thread);
  pushpath(info, ".callinfo");
  UNLOCK(thread);
  thread->ci = &thread->base_ci;
  level = 0;
  for (;;) {
    LOCK(thread);
    pushpath(info, "[%d]", level++);
    UNLOCK(thread);
    thread->ci->func = eris_restorestackidx(thread, READ_VALUE(size_t));
    validate(thread->ci->func, thread->top - 1);
    thread->ci->top = eris_restorestackidx(thread, READ_VALUE(size_t));
    validate(thread->ci->top, thread->stack_last);
    thread->ci->nresults = READ_VALUE(int16_t);
    thread->ci->callstatus = READ_VALUE(uint8_t);
    if (thread->ci->callstatus) {
      /** See comment in p_thread. */
      if (thread->ci->callstatus & CIST_YPCALL) {
        thread->ci->extra = eris_savestack(thread,
          eris_restorestackidx(thread, READ_VALUE(size_t)));
        o = eris_restorestack(thread, thread->ci->extra);
        validate(o, thread->top);
        if (eris_ttnov(o) != LUA_TFUNCTION) {
          eris_error(info, ERIS_ERR_THREADCI);
        }
      }

      if (eris_isLua(thread->ci)) {
        LClosure *lcl = eris_ci_func(thread->ci);
        thread->ci->u.l.base = eris_restorestackidx(thread, READ_VALUE(size_t));
        validate(thread->ci->u.l.base, thread->top);
        thread->ci->u.l.savedpc = lcl->p->code + READ_VALUE(size_t);
        if (thread->ci->u.l.savedpc < lcl->p->code ||
            thread->ci->u.l.savedpc > lcl->p->code + lcl->p->sizecode)
        {
          thread->ci->u.l.savedpc = lcl->p->code; /* Just to be safe. */
          eris_error(info, ERIS_ERR_THREADPC);
        }
      }
      else {
        /* These are only used while a thread is being executed:
        thread->ci->u.c.old_errfunc = READ_VALUE(ptrdiff_t); */
        thread->ci->u.c.old_errfunc = 0;

        if (thread->status == LUA_YIELD && READ_VALUE(uint8_t)) {
          thread->ci->u.c.ctx = READ_VALUE(int);
          LOCK(thread);
          unpersist(info);                                  /* ... thread func? */
          UNLOCK(thread);
          if (lua_iscfunction(info->L, -1)) {                /* ... thread func */
            /* NOTE Ugly hack. See p_thread. */
            thread->ci->u.c.k = (lua_KFunction) lua_tocfunction(info->L, -1);
          }
          else {
            eris_error(info, ERIS_ERR_THREADCTX);
            return; /* not reached */
          }
          lua_pop(info->L, 1);                                    /* ... thread */
        }
        else {
          thread->ci->u.c.ctx = 0;
          thread->ci->u.c.k = NULL;
        }
      }
      LOCK(thread);
      poppath(info);
      UNLOCK(thread);
    }
    else {
      thread->ci->u.c.k = NULL;
      thread->ci->u.c.old_errfunc = 0;
      thread->ci->u.c.ctx = 0;
    }

    /* Read in value for check for next iteration. */
    if (READ_VALUE(uint8_t)) {
      break;
    }
    else {
      thread->ci = eris_extendCI(thread);
    }
  }
  if (thread->status == LUA_YIELD) {
    thread->ci->extra = eris_savestack(thread,
      eris_restorestackidx(thread, READ_VALUE(size_t)));
    o = eris_restorestack(thread, thread->ci->extra);
    validate(o, thread->top);
    if (eris_ttnov(o) != LUA_TFUNCTION) {
      eris_error(info, ERIS_ERR_THREADCI);
    }
  }
  LOCK(thread);
  poppath(info);
  UNLOCK(thread);

  /* Get from context: only zero for dead threads, otherwise one. */
  thread->nCcalls = thread->status != LUA_OK || lua_gettop(thread) != 0;

  /* Proceed to open upvalues. These upvalues will already exist due to the
   * functions using them having been unpersisted (they'll usually be in the
   * stack of the thread). For this reason we store all previous references to
   * the upvalue in a table that is returned when we try to unpersist an
   * upvalue, so that we can adjust these references in here. */
  LOCK(thread);
  pushpath(info, ".openupval");
  UNLOCK(thread);
  level = 0;
  for (;;) {
    UpVal *nuv;
    StkId stk;
    /* Get the position of the upvalue on the stack. As a special value we pass
     * zero to indicate there are no more upvalues. */
    const size_t offset = READ_VALUE(size_t);
    if (offset == 0) {
      break;
    }
    LOCK(thread);
    pushpath(info, "[%d]", level);
    UNLOCK(thread);
    stk = eris_restorestackidx(thread, offset - 1);
    validate(stk, thread->top - 1);
    LOCK(thread);
    unpersist(info);                                        /* ... thread tbl */
    UNLOCK(thread);
    eris_assert(lua_type(info->L, -1) == LUA_TTABLE);

    /* Create the open upvalue either way. */
    LOCK(thread);
    nuv = eris_findupval(thread, stk);
    UNLOCK(thread);

    /* Then check if we need to patch some references. */
    lua_rawgeti(info->L, -1, UVTREF);               /* ... thread tbl lcl/nil */
    if (!lua_isnil(info->L, -1)) {                      /* ... thread tbl lcl */
      int i, n;
      eris_assert(lua_type(info->L, -1) == LUA_TFUNCTION);
      /* Already exists, replace it. To do this we have to patch all the
       * references to the already existing one, which we added to the table in
       * u_closure. */
      lua_pop(info->L, 1);                                  /* ... thread tbl */
      for (i = UVTREF, n = luaL_len(info->L, -1); i <= n; i += 2) {
        LClosure *cl;
        int nup;
        lua_rawgeti(info->L, -1, i);                    /* ... thread tbl lcl */
        cl = eris_clLvalue(info->L->top - 1);
        lua_pop(info->L, 1);                                /* ... thread tbl */
        lua_rawgeti(info->L, -1, i + 1);                /* ... thread tbl nup */
        nup = lua_tointeger(info->L, -1);
        lua_pop(info->L, 1);                                /* ... thread tbl */
        /* Open the upvalue by pointing to the stack and register in GC. */
        cl->upvals[nup - 1] = nuv;
        cl->upvals[nup - 1]->refcount++;
      }
    }
    else {                                              /* ... thread tbl nil */
      eris_assert(lua_isnil(info->L, -1));
      lua_pop(info->L, 1);                                  /* ... thread tbl */
    }

    /* Store open upvalue in table for future references. */
    LOCK(thread);
    lua_pop(info->L, 1);                                        /* ... thread */
    poppath(info);
    UNLOCK(thread);
  }
  poppath(info);

  eris_assert(lua_type(info->L, -1) == LUA_TTHREAD);
}

#undef UNLOCK
#undef LOCK

#undef validate

/*
** ============================================================================
** Top-level delegator.
** ============================================================================
*/

static void
persist_typed(Info *info, int type) {                 /* perms reftbl ... obj */
  eris_ifassert(const int top = lua_gettop(info->L));
  if (info->level >= info->maxComplexity) {
    eris_error(info, ERIS_ERR_COMPLEXITY);
  }
  ++info->level;

  WRITE_VALUE(type, int);
  switch(type) {
    case LUA_TBOOLEAN:
      p_boolean(info);
      break;
    case LUA_TLIGHTUSERDATA:
      p_pointer(info);
      break;
    case LUA_TNUMBER:
      p_number(info);
      break;
    case LUA_TSTRING:
      p_string(info);
      break;
    case LUA_TTABLE:
      p_table(info);
      break;
    case LUA_TFUNCTION:
      p_closure(info);
      break;
    case LUA_TUSERDATA:
      p_userdata(info);
      break;
    case LUA_TTHREAD:
      p_thread(info);
      break;
    case LUA_TPROTO:
      p_proto(info);
      break;
    case LUA_TUPVAL:
      p_upval(info);
      break;
    default:
      eris_error(info, ERIS_ERR_TYPEP, type);
  }                                                   /* perms reftbl ... obj */

  --info->level;
  eris_assert(top == lua_gettop(info->L));
}

/* Second-level delegating persist function, used for cases when persisting
 * data that's stored in the reftable with a key that is not the data itself,
 * namely upvalues and protos. */
static void
persist_keyed(Info *info, int type) {          /* perms reftbl ... obj refkey */
  eris_checkstack(info->L, 2);

  /* Keep a copy of the key for pushing it to the reftable, if necessary. */
  lua_pushvalue(info->L, -1);           /* perms reftbl ... obj refkey refkey */

  /* If the object has already been written, write a reference to it. */
  lua_rawget(info->L, REFTIDX);           /* perms reftbl ... obj refkey ref? */
  if (!lua_isnil(info->L, -1)) {           /* perms reftbl ... obj refkey ref */
    const int reference = lua_tointeger(info->L, -1);
    WRITE_VALUE(reference + ERIS_REFERENCE_OFFSET, int);
    lua_pop(info->L, 2);                              /* perms reftbl ... obj */
    return;
  }                                        /* perms reftbl ... obj refkey nil */
  lua_pop(info->L, 1);                         /* perms reftbl ... obj refkey */

  /* Copy the refkey for the perms check below. */
  lua_pushvalue(info->L, -1);           /* perms reftbl ... obj refkey refkey */

  /* Put the value in the reference table. This creates an entry pointing from
   * the object (or its key) to the id the object is referenced by. */
  lua_pushinteger(info->L, ++(info->refcount));
                                    /* perms reftbl ... obj refkey refkey ref */
  lua_rawset(info->L, REFTIDX);                /* perms reftbl ... obj refkey */

  /* At this point, we'll give the permanents table a chance to play. */
  lua_gettable(info->L, PERMIDX);            /* perms reftbl ... obj permkey? */
  if (!lua_isnil(info->L, -1)) {              /* perms reftbl ... obj permkey */
    type = lua_type(info->L, -2);
    /* Prepend permanent "type" so that we know it's a permtable key. This will
     * trigger u_permanent when unpersisting. Also write the original type, so
     * that we can verify what we get in the permtable when unpersisting is of
     * the same kind we had when persisting. */
    WRITE_VALUE(ERIS_PERMANENT, int);
    WRITE_VALUE(type, uint8_t);
    persist(info);                            /* perms reftbl ... obj permkey */
    lua_pop(info->L, 1);                              /* perms reftbl ... obj */
  }
  else {                                          /* perms reftbl ... obj nil */
    /* No entry in the permtable for this object, persist it directly. */
    lua_pop(info->L, 1);                              /* perms reftbl ... obj */
    persist_typed(info, type);                        /* perms reftbl ... obj */
  }                                                   /* perms reftbl ... obj */
}

/* Top-level delegating persist function. */
static void
persist(Info *info) {                                 /* perms reftbl ... obj */
  /* Grab the object's type. */
  const int type = lua_type(info->L, -1);

  /* If the object is nil, only write its type. */
  if (type == LUA_TNIL) {
    WRITE_VALUE(type, int);
  }
  /* Write simple values directly, because writing a "reference" would take up
   * just as much space and we can save ourselves work this way. */
  else if (type == LUA_TBOOLEAN ||
           type == LUA_TLIGHTUSERDATA ||
           type == LUA_TNUMBER)
  {
    persist_typed(info, type);                        /* perms reftbl ... obj */
  }
  /* For all non-simple values we keep a record in the reftable, so that we
   * keep references alive across persisting and unpersisting an object. This
   * has the nice side-effect of saving some space. */
  else {
    eris_checkstack(info->L, 1);
    lua_pushvalue(info->L, -1);                   /* perms reftbl ... obj obj */
    persist_keyed(info, type);                        /* perms reftbl ... obj */
  }
}

/** ======================================================================== */

static void
u_permanent(Info *info) {                                 /* perms reftbl ... */
  const int type = READ_VALUE(uint8_t);
  /* Reserve reference to avoid the key going first. */
  const int reference = ++(info->refcount);
  eris_checkstack(info->L, 1);
  unpersist(info);                                /* perms reftbl ... permkey */
  lua_gettable(info->L, PERMIDX);                    /* perms reftbl ... obj? */
  if (lua_isnil(info->L, -1)) {                       /* perms reftbl ... nil */
    /* Since we may need permanent values to rebuild other structures, namely
     * closures and threads, we cannot allow perms to fail unpersisting. */
    eris_error(info, ERIS_ERR_SPER_UPERMNIL);
  }
  else if (lua_type(info->L, -1) != type) {            /* perms reftbl ... :( */
    /* For the same reason that we cannot allow nil we must also require the
     * unpersisted value to be of the correct type. */
    const char *want = kTypenames[type];
    const char *have = kTypenames[lua_type(info->L, -1)];
    eris_error(info, ERIS_ERR_SPER_UPERM, want, have);
  }                                                   /* perms reftbl ... obj */
  /* Create the entry in the reftable. */
  lua_pushvalue(info->L, -1);                     /* perms reftbl ... obj obj */
  lua_rawseti(info->L, REFTIDX, reference);           /* perms reftbl ... obj */
}

static void
unpersist(Info *info) {                                   /* perms reftbl ... */
  eris_ifassert(const int top = lua_gettop(info->L));
  if (info->level >= info->maxComplexity) {
    eris_error(info, ERIS_ERR_COMPLEXITY);
  }
  ++info->level;

  eris_checkstack(info->L, 1);
  {
    const int typeOrReference = READ_VALUE(int);
    if (typeOrReference > ERIS_REFERENCE_OFFSET) {
      const int reference = typeOrReference - ERIS_REFERENCE_OFFSET;
      lua_rawgeti(info->L, REFTIDX, reference);   /* perms reftbl ud ... obj? */
      if (lua_isnil(info->L, -1)) {                 /* perms reftbl ud ... :( */
        eris_error(info, ERIS_ERR_REF, reference);
      }                                            /* perms reftbl ud ... obj */
    }
    else {
      const int type = typeOrReference;
      switch (type) {
        case LUA_TNIL:
          lua_pushnil(info->L);
          break;
        case LUA_TBOOLEAN:
          u_boolean(info);
          break;
        case LUA_TLIGHTUSERDATA:
          u_pointer(info);
          break;
        case LUA_TNUMBER:
          u_number(info);
          break;
        case LUA_TSTRING:
          u_string(info);
          break;
        case LUA_TTABLE:
          u_table(info);
          break;
        case LUA_TFUNCTION:
          u_closure(info);
          break;
        case LUA_TUSERDATA:
          u_userdata(info);
          break;
        case LUA_TTHREAD:
          u_thread(info);
          break;
        case LUA_TPROTO:
          u_proto(info);
          break;
        case LUA_TUPVAL:
          u_upval(info);
          break;
        case ERIS_PERMANENT:
          u_permanent(info);
          break;
        default:
          eris_error(info, ERIS_ERR_TYPEU, type);
      }                                              /* perms reftbl ... obj? */
    }
  }

  --info->level;
  eris_assert(top + 1 == lua_gettop(info->L));
}

/* }======================================================================== */

/*
** {===========================================================================
** Writer and reader implementation for library calls.
** ============================================================================
*/

/* Implementation note: we use the MBuffer struct, but we don't use the built-
 * in reallocation functions since we'll keep our working copy on the stack, to
 * allow for proper collection if we have to throw an error. This is very much
 * what the auxlib does with its buffer functionality. Which we don't use since
 * we cannot guarantee stack balance inbetween calls to luaL_add*. */

static int
writer(lua_State *L, const void *p, size_t sz, void *ud) {
                                               /* perms reftbl buff path? ... */
  const char *value = (const char*)p;
  Mbuffer *buff = (Mbuffer*)ud;
  const size_t size = eris_bufflen(buff);
  const size_t capacity = eris_sizebuffer(buff);
  if (capacity - size < sz) {
    size_t newcapacity = capacity * 2; /* overflow checked below */
    if (newcapacity - size < sz) {
      newcapacity = capacity + sz; /* overflow checked below */
    }
    if (newcapacity <= capacity) {
      /* Overflow in capacity, buffer size limit reached. */
      return 1;
    } else {
      char *newbuff;
      eris_checkstack(L, 1);
      newbuff = (char*)lua_newuserdata(L, newcapacity * sizeof(char));
                                         /* perms reftbl buff path? ... nbuff */
      memcpy(newbuff, eris_buffer(buff), eris_bufflen(buff));
      lua_replace(L, BUFFIDX);                /* perms reftbl nbuff path? ... */
      eris_buffer(buff) = newbuff;
      eris_sizebuffer(buff) = newcapacity;
    }
  }
  memcpy(&eris_buffer(buff)[eris_bufflen(buff)], value, sz);
  eris_bufflen(buff) += sz;
  return 0;
}

/** ======================================================================== */

/* Readonly, interface compatible with MBuffer macros. */
typedef struct RBuffer {
  const char *buffer;
  size_t n;
  size_t buffsize;
} RBuffer;

static const char*
reader(lua_State *L, void *ud, size_t *sz) {
  RBuffer *buff = (RBuffer*)ud;
  (void) L; /* unused */
  if (eris_bufflen(buff) == 0) {
    return NULL;
  }
  *sz = eris_bufflen(buff);
  eris_bufflen(buff) = 0;
  return eris_buffer(buff);
}

/* }======================================================================== */

/*
** {===========================================================================
** Library functions.
** ============================================================================
*/

static void
p_header(Info *info) {
  WRITE_RAW(kHeader, HEADER_LENGTH);
  WRITE_VALUE(sizeof(lua_Number), uint8_t);
  WRITE_VALUE(kHeaderNumber, lua_Number);
  WRITE_VALUE(sizeof(lua_Integer), uint8_t);
  WRITE_VALUE(sizeof(int), uint8_t);
  WRITE_VALUE(sizeof(size_t), uint8_t);
}

static void
u_header(Info *info) {
  char header[HEADER_LENGTH];
  uint8_t number_size;
  READ_RAW(header, HEADER_LENGTH);
  if (strncmp(kHeader, header, HEADER_LENGTH)) {
    luaL_error(info->L, "invalid data");
  }
  number_size = READ_VALUE(uint8_t);
  if (number_size == 0) {
    /* Old 64-bit versions of eris wrote '\0' and then three random bytes. */
    /* We skip them here for backwards compatibility. */
    char throw_away[3];
    READ_RAW(throw_away, 3);

    number_size = READ_VALUE(uint8_t);
  }
  if (number_size != sizeof(lua_Number)) {
    luaL_error(info->L, "incompatible floating point type");
  }
  /* In this case we really do want floating point equality. */
  if (READ_VALUE(lua_Number) != kHeaderNumber) {
    luaL_error(info->L, "incompatible floating point representation");
  }
  if (READ_VALUE(uint8_t) != sizeof(lua_Integer)) {
    luaL_error(info->L, "incompatible integer type");
  }
  info->u.upi.sizeof_int = READ_VALUE(uint8_t);
  info->u.upi.sizeof_size_t = READ_VALUE(uint8_t);
}

static void
unchecked_persist(lua_State *L, lua_Writer writer, void *ud) {
  Info info;                                            /* perms buff rootobj */
  info.L = L;
  info.level = 0;
  info.refcount = 0;
  info.maxComplexity = kMaxComplexity;
  info.passIOToPersist = kPassIOToPersist;
  info.generatePath = kGeneratePath;
  info.u.pi.writer = writer;
  info.u.pi.ud = ud;
  info.u.pi.metafield = kPersistKey;
  info.u.pi.writeDebugInfo = kWriteDebugInformation;

  eris_checkstack(L, 3);

  if (get_setting(L, (void*)&kSettingMaxComplexity)) {
                                                  /* perms buff rootobj value */
    info.maxComplexity = lua_tointeger(L, -1);
    lua_pop(L, 1);                                      /* perms buff rootobj */
  }
  if (get_setting(L, (void*)&kSettingGeneratePath)) {
                                                  /* perms buff rootobj value */
    info.generatePath = lua_toboolean(L, -1);
    lua_pop(L, 1);                                      /* perms buff rootobj */
  }
  if (get_setting(L, (void*)&kSettingPassIOToPersist)) {
                                                  /* perms buff rootobj value */
    info.passIOToPersist = lua_toboolean(L, -1);
    lua_pop(L, 1);                                      /* perms buff rootobj */
  }
  if (get_setting(L, (void*)&kSettingMetafield)) {/* perms buff rootobj value */
    info.u.pi.metafield = lua_tostring(L, -1);
    lua_pop(L, 1);                                      /* perms buff rootobj */
  }
  if (get_setting(L, (void*)&kSettingWriteDebugInfo)) {
                                                  /* perms buff rootobj value */
    info.u.pi.writeDebugInfo = lua_toboolean(L, -1);
    lua_pop(L, 1);                                      /* perms buff rootobj */
  }

  lua_newtable(L);                               /* perms buff rootobj reftbl */
  lua_insert(L, REFTIDX);                        /* perms reftbl buff rootobj */
  if (info.generatePath) {
    lua_newtable(L);                        /* perms reftbl buff rootobj path */
    lua_insert(L, PATHIDX);                 /* perms reftbl buff path rootobj */
    pushpath(&info, "root");
  }

  /* Populate perms table with Lua internals. */
  lua_pushvalue(L, PERMIDX);         /* perms reftbl buff path? rootobj perms */
  populateperms(L, false);
  lua_pop(L, 1);                           /* perms reftbl buff path? rootobj */

  p_header(&info);
  persist(&info);                          /* perms reftbl buff path? rootobj */

  if (info.generatePath) {                  /* perms reftbl buff path rootobj */
    lua_remove(L, PATHIDX);                      /* perms reftbl buff rootobj */
  }                                              /* perms reftbl buff rootobj */
  lua_remove(L, REFTIDX);                               /* perms buff rootobj */
}

static void
unchecked_unpersist(lua_State *L, lua_Reader reader, void *ud) {/* perms str? */
  Info info;
  info.L = L;
  info.level = 0;
  info.refcount = 0;
  info.maxComplexity = kMaxComplexity;
  info.generatePath = kGeneratePath;
  info.passIOToPersist = kPassIOToPersist;
  eris_init(L, &info.u.upi.zio, reader, ud);

  eris_checkstack(L, 3);

  if (get_setting(L, (void*)&kSettingMaxComplexity)) {
                                                  /* perms buff rootobj value */
    info.maxComplexity = lua_tointeger(L, -1);
    lua_pop(L, 1);                                      /* perms buff rootobj */
  }
  if (get_setting(L, (void*)&kSettingGeneratePath)) {
                                                 /* perms buff? rootobj value */
    info.generatePath = lua_toboolean(L, -1);
    lua_pop(L, 1);                                     /* perms buff? rootobj */
  }
  if (get_setting(L, (void*)&kSettingPassIOToPersist)) {  /* perms str? value */
    info.passIOToPersist = lua_toboolean(L, -1);
    lua_pop(L, 1);                                              /* perms str? */
  }

  lua_newtable(L);                                       /* perms str? reftbl */
  lua_insert(L, REFTIDX);                                /* perms reftbl str? */
  if (info.generatePath) {
    /* Make sure the path is always at index 4, so that it's the same for
     * persist and unpersist. */
    lua_pushnil(L);                                  /* perms reftbl str? nil */
    lua_insert(L, BUFFIDX);                          /* perms reftbl nil str? */
    lua_newtable(L);                            /* perms reftbl nil str? path */
    lua_insert(L, PATHIDX);                     /* perms reftbl nil path str? */
    pushpath(&info, "root");
  }

  /* Populate perms table with Lua internals. */
  lua_pushvalue(L, PERMIDX);            /* perms reftbl nil? path? str? perms */
  populateperms(L, true);
  lua_pop(L, 1);                              /* perms reftbl nil? path? str? */

  u_header(&info);
  unpersist(&info);                   /* perms reftbl nil? path? str? rootobj */
  if (info.generatePath) {              /* perms reftbl nil path str? rootobj */
    lua_remove(L, PATHIDX);                  /* perms reftbl nil str? rootobj */
    lua_remove(L, BUFFIDX);                      /* perms reftbl str? rootobj */
  }                                              /* perms reftbl str? rootobj */
  lua_remove(L, REFTIDX);                               /* perms str? rootobj */
}

/** ======================================================================== */

static int
l_persist(lua_State *L) {                             /* perms? rootobj? ...? */
  Mbuffer buff;

  /* See if we have anything at all. */
  luaL_checkany(L, 1);

  /* If we only have one object we assume it is the root object and that there
   * is no perms table, so we create an empty one for internal use. */
  if (lua_gettop(L) == 1) {                                        /* rootobj */
    eris_checkstack(L, 1);
    lua_newtable(L);                                         /* rootobj perms */
    lua_insert(L, PERMIDX);                                  /* perms rootobj */
  }
  else {
    luaL_checktype(L, 1, LUA_TTABLE);                  /* perms rootobj? ...? */
    luaL_checkany(L, 2);                                /* perms rootobj ...? */
    lua_settop(L, 2);                                        /* perms rootobj */
  }
  eris_checkstack(L, 1);
  lua_pushnil(L);                                       /* perms rootobj buff */
  lua_insert(L, 2);                                     /* perms buff rootobj */

  eris_initbuffer(L, &buff);
  eris_bufflen(&buff) = 0; /* Not initialized by initbuffer... */

  unchecked_persist(L, writer, &buff);                  /* perms buff rootobj */

  /* Copy the buffer as the result string before removing it, to avoid the data
   * being garbage collected. */
  lua_pushlstring(L, eris_buffer(&buff), eris_bufflen(&buff));
                                                    /* perms buff rootobj str */

  return 1;
}

static int
l_unpersist(lua_State *L) {                               /* perms? str? ...? */
  RBuffer buff;

  /* See if we have anything at all. */
  luaL_checkany(L, 1);

  /* If we only have one object we assume it is the root object and that there
   * is no perms table, so we create an empty one for internal use. */
  if (lua_gettop(L) == 1) {                                           /* str? */
    eris_checkstack(L, 1);
    lua_newtable(L);                                            /* str? perms */
    lua_insert(L, PERMIDX);                                     /* perms str? */
  }
  else {
    luaL_checktype(L, 1, LUA_TTABLE);                      /* perms str? ...? */
  }
  eris_buffer(&buff) = luaL_checklstring(L, 2, &eris_bufflen(&buff));
  eris_sizebuffer(&buff) = eris_bufflen(&buff);             /* perms str ...? */
  lua_settop(L, 2);                                              /* perms str */

  unchecked_unpersist(L, reader, &buff);                 /* perms str rootobj */

  return 1;
}

#define IS(s) strncmp(s, name, length < sizeof(s) ? length : sizeof(s)) == 0

static int
l_settings(lua_State *L) {                                /* name value? ...? */
  size_t length;
  const char *name = luaL_checklstring(L, 1, &length);
  if (lua_isnone(L, 2)) {                                        /* name ...? */
    lua_settop(L, 1);                                                 /* name */
    /* Get the current setting value and return it. */
    if (IS(kSettingMetafield)) {
      if (!get_setting(L, (void*)&kSettingMetafield)) {
        lua_pushstring(L, kPersistKey);
      }
    }
    else if (IS(kSettingPassIOToPersist)) {
      if (!get_setting(L, (void*)&kSettingPassIOToPersist)) {
        lua_pushboolean(L, kPassIOToPersist);
      }
    }
    else if (IS(kSettingWriteDebugInfo)) {
      if (!get_setting(L, (void*)&kSettingWriteDebugInfo)) {
        lua_pushboolean(L, kWriteDebugInformation);
      }
    }
    else if (IS(kSettingGeneratePath)) {
      if (!get_setting(L, (void*)&kSettingGeneratePath)) {
        lua_pushboolean(L, kGeneratePath);
      }
    }
    else if (IS(kSettingMaxComplexity)) {
      if (!get_setting(L, (void*)&kSettingMaxComplexity)) {
        lua_pushinteger(L, kMaxComplexity);
      }
    }
    else {
      return luaL_argerror(L, 1, "no such setting");
    }                                                           /* name value */
    return 1;
  }
  else {                                                   /* name value ...? */
    lua_settop(L, 2);                                           /* name value */
    /* Set a new value for the setting. */
    if (IS(kSettingMetafield)) {
      luaL_optstring(L, 2, NULL);
      set_setting(L, (void*)&kSettingMetafield);
    }
    else if (IS(kSettingPassIOToPersist)) {
      luaL_opt(L, checkboolean, 2, false);
      set_setting(L, (void*)&kSettingPassIOToPersist);
    }
    else if (IS(kSettingWriteDebugInfo)) {
      luaL_opt(L, checkboolean, 2, false);
      set_setting(L, (void*)&kSettingWriteDebugInfo);
    }
    else if (IS(kSettingGeneratePath)) {
      luaL_opt(L, checkboolean, 2, false);
      set_setting(L, (void*)&kSettingGeneratePath);
    }
    else if (IS(kSettingMaxComplexity)) {
      luaL_optinteger(L, 2, 0);
      set_setting(L, (void*)&kSettingMaxComplexity);
    }
    else {
      return luaL_argerror(L, 1, "no such setting");
    }                                                                 /* name */
    return 0;
  }
}

#undef IS

/** ======================================================================== */

static luaL_Reg erislib[] = {
  { "persist", l_persist },
  { "unpersist", l_unpersist },
  { "settings", l_settings },
  { NULL, NULL }
};

LUA_API int luaopen_eris(lua_State *L) {
  luaL_newlib(L, erislib);
  return 1;
}

/* }======================================================================== */

/*
** {===========================================================================
** Public API functions.
** ============================================================================
*/

LUA_API void
eris_dump(lua_State *L, lua_Writer writer, void *ud) {     /* perms? rootobj? */
  if (lua_gettop(L) > 2) {
    luaL_error(L, "too many arguments");
  }
  luaL_checktype(L, 1, LUA_TTABLE);                         /* perms rootobj? */
  luaL_checkany(L, 2);                                       /* perms rootobj */
  lua_pushnil(L);                                        /* perms rootobj nil */
  lua_insert(L, -2);                                     /* perms nil rootobj */
  unchecked_persist(L, writer, ud);                      /* perms nil rootobj */
  lua_remove(L, -2);                                         /* perms rootobj */
}

LUA_API void
eris_undump(lua_State *L, lua_Reader reader, void *ud) {            /* perms? */
  if (lua_gettop(L) > 1) {
    luaL_error(L, "too many arguments");
  }
  luaL_checktype(L, 1, LUA_TTABLE);                                  /* perms */
  unchecked_unpersist(L, reader, ud);                        /* perms rootobj */
}

/** ======================================================================== */

LUA_API void
eris_persist(lua_State *L, int perms, int value) {                    /* ...? */
  perms = lua_absindex(L, perms);
  value = lua_absindex(L, value);
  eris_checkstack(L, 3);
  lua_pushcfunction(L, l_persist);                           /* ... l_persist */
  lua_pushvalue(L, perms);                             /* ... l_persist perms */
  lua_pushvalue(L, value);                     /* ... l_persist perms rootobj */
  lua_call(L, 2, 1);                                               /* ... str */
}

LUA_API void
eris_unpersist(lua_State *L, int perms, int value) {                   /* ... */
  perms = lua_absindex(L, perms);
  value = lua_absindex(L, value);
  eris_checkstack(L, 3);
  lua_pushcfunction(L, l_unpersist);                       /* ... l_unpersist */
  lua_pushvalue(L, perms);                           /* ... l_unpersist perms */
  lua_pushvalue(L, value);                       /* ... l_unpersist perms str */
  lua_call(L, 2, 1);                                           /* ... rootobj */
}

LUA_API void
eris_get_setting(lua_State *L, const char *name) {                     /* ... */
  eris_checkstack(L, 2);
  lua_pushcfunction(L, l_settings);                         /* ... l_settings */
  lua_pushstring(L, name);                             /* ... l_settings name */
  lua_call(L, 1, 1);                                             /* ... value */
}

LUA_API void
eris_set_setting(lua_State *L, const char *name, int value) {          /* ... */
  value = lua_absindex(L, value);
  eris_checkstack(L, 3);
  lua_pushcfunction(L, l_settings);                         /* ... l_settings */
  lua_pushstring(L, name);                             /* ... l_settings name */
  lua_pushvalue(L, value);                       /* ... l_settings name value */
  lua_call(L, 2, 0);                                                   /* ... */
}

/* }======================================================================== */

