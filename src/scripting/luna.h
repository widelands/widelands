/*
 * Copyright (C) 2006-2010 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*
 * This code was inspired from the lua_users wiki:
 *     http://lua-users.org/wiki/SimplerCppBinding
 * and by some code written by TheBunny:
 *     http://plaidworld.com/
 *
 * A lot of new features (e.g. inheritance) were added for Widelands.
 */

#ifndef LUNA_H
#define LUNA_H

#define PROP_RO(klass, name) {#name, &klass::get_##name, 0}
#define PROP_RW(klass, name) {#name, &klass::get_##name, &klass::set_##name}
#define METHOD(klass, name) {#name, &klass::name}

#define LUNA_CLASS_HEAD(klass) \
	static const char className[]; \
	static const char parentName[]; \
	static const MethodType<klass> Methods[]; \
	static const PropertyType<klass> Properties[]; \

#include <lua.hpp>
#include "luna_impl.h"

/**
 * Register the class as a Lua class (that is a metatable and a function to
 * create an object out of this table).
 *
 * The class will always be created in the namespace wl, sub_namespace can
 * be used to create the class in one depth deeper (e.g. wl.map)
 */
template <class T>
int register_class(lua_State * const L, char const * const sub_namespace = "")
{
	// Get wl table which MUST already exist
	lua_getglobal(L, "wl");

	// push the wl sub namespace table onto the stack, if desired
	if (strlen(sub_namespace) != 0)
		lua_getfield(L, -1, sub_namespace);

	m_add_constructor_to_lua<T>(L);
	int metatable_idx = m_create_metatable_for_class<T>(L);

	m_register_properties_in_metatable<T>(L, metatable_idx);
	m_register_methods_in_metatable<T, T>(L, metatable_idx);

	return metatable_idx;
}
/**
 * Makes the first class a children of the second. Make sure that T is really a
 * child class of PT before calling this. This must also be called directly
 * after register_class, so that the Metatable index is still valid
 */
template <class T, class PT>
void add_parent(lua_State * L, int metatable_idx)
{
	m_register_properties_in_metatable<PT>(L, metatable_idx);
	m_register_methods_in_metatable<T, PT>(L, metatable_idx);
}

/*
 * Get the instance of this C object to lua. This is usually used
 * as last call in a function that should create a new Lua object
 */
template <class T>
int to_lua(lua_State * const L, T * const obj) {
	// Create a new table with some slots preallocated
	lua_createtable(L, 0, 30);

	// get the index of the new table on the stack
	int const newtable = lua_gettop(L);

	// push index of position of user data in our array
	lua_pushnumber(L, 0);

	// Make a new userdata. A lightuserdata won't do since we want to assign
	// a metatable to it
	T * * const a = static_cast<T * * >(lua_newuserdata(L, sizeof(T *)));
	*a = obj;

	int const userdata = lua_gettop(L);

	// Assign this metatable to this userdata. We only do this to add
	// garbage collection to this userdata, so that the destructor gets
	// called. As a (unwanted, but not critical) side effect we also add all
	// other methods to this object
	luaL_getmetatable(L, T::className);
	lua_setmetatable (L, userdata);

	// table[ 0 ] = USERDATA;
	lua_settable(L, newtable);

	// Assign this metatable to the newly created table
	luaL_getmetatable(L, T::className);
	lua_setmetatable (L, newtable);

	return 1;
}


#endif /* end of include guard: __S_LUNA_H */


