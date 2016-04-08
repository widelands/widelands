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

/* lua.h must be included before this file */

#ifndef ERIS_H
#define ERIS_H

#define ERIS_VERSION_MAJOR  "1"
#define ERIS_VERSION_MINOR  "1"
#define ERIS_VERSION_NUM    101
#define ERIS_VERSION_RELEASE  "0"

/*
** ==================================================================
** API
** ==================================================================
*/

/**
 * This provides an interface to Eris' persist functionality for writing in
 * an arbitrary way, using a writer.
 *
 * When called, the stack in 'L' must look like this:
 * 1: perms:table
 * 2: value:any
 *
 * 'writer' is the writer function used to store all data, 'ud' is passed to
 * the writer function whenever it is called.
 *
 * [-0, +0, e]
 */
LUA_API void eris_dump(lua_State* L, lua_Writer writer, void* ud);

/**
 * This provides an interface to Eris' unpersist functionality for reading
 * in an arbitrary way, using a reader.
 *
 * When called, the stack in 'L' must look like this:
 * 1: perms:table
 *
 * 'reader' is the reader function used to read all data, 'ud' is passed to
 * the reader function whenever it is called.
 *
 * The result of the operation will be pushed onto the stack.
 *
 * [-0, +1, e]
 */
LUA_API void eris_undump(lua_State* L, lua_Reader reader, void* ud);

/**
 * This is a stack-based alternative to eris_dump.
 *
 * It expects the perms table at the specified index 'perms' and the value to
 * persist at the specified index 'value'. It will push the resulting string
 * onto the stack on success.
 *
 * [-0, +1, e]
 */
LUA_API void eris_persist(lua_State* L, int perms, int value);

/**
 * This is a stack-based alternative to eris_undump.
 *
 * It expects the perms table at the specified index 'perms' and the string
 * containing persisted data at the specified index 'value'. It will push the
 * resulting value onto the stack on success.
 *
 * [-0, +1, e]
 */
LUA_API void eris_unpersist(lua_State* L, int perms, int value);

/**
 * Pushes the current value of a setting onto the stack.
 *
 * The name is the name of the setting to get the value for:
 * - 'debug'  whether to write debug information when persisting function
 *            prototypes (line numbers, local variable names, upvalue names).
 * - 'maxrec' the maximum complexity of objects we support (the nesting level
 *            of tables, for example). This can be useful to avoid segmentation
 *            faults due to too deep recursion when working with user-provided
 *            data.
 * - 'path'   whether to generate a "path" used to indicate where in an object
 *            an error occurred. This adds considerable overhead and should
 *            only be used to debug errors as they appear.
 * - 'spio'   whether to pass IO objects along as light userdata to special
 *            persistence functions. When persisting this will pass along the
 *            lua_Writer and its void* in addition to the original object, when
 *            unpersisting it will pass along a ZIO*.
 * - 'spkey'  the name of the field in the metatable of tables and userdata
 *            used to control persistence (on/off or special persistence).
 *
 * If an unknown name is specified this will throw an error.
 *
 * [-0, +1, e]
 */
LUA_API void eris_get_setting(lua_State *L, const char *name);

/**
 * Changes the value of a setting to a value on the stack.
 *
 * For the available settings see eris_set_setting(). This will get the new
 * value from the specified stack index 'value'. If the type does not match
 * this will throw an error. Specify a nil value to reset the setting to its
 * default value.
 *
 * [-0, +0, e]
 */
LUA_API void eris_set_setting(lua_State *L, const char *name, int value);

/*
** ==================================================================
** Library installer
** ==================================================================
*/

/**
 * This pushes a table with the two functions 'persist' and 'unpersist':
 *   persist([perms,] value)
 *     Where 'perms' is a table with "permanent" objects and 'value' is the
 *     value that should be persisted. Returns the string with persisted data.
 *     If only one value is given, the perms table is assumed to be empty.
 *
 *   unpersist([perms,] value)
 *     Where 'perms' is a table with the inverse mapping as used when
 *     persisting the data via persist() and 'value' is the string with the
 *     persisted data returned by persist(). Returns the unpersisted value.
 *     If only one value is given, the perms table is assumed to be empty.
 */
LUA_API int luaopen_eris(lua_State* L);

#endif

