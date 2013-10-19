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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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


#define LUNA_CLASS_HEAD(klass) \
	static const char className[]; \
	static const MethodType<klass> Methods[]; \
	static const PropertyType<klass> Properties[];

/*
 * Macros for filling the description tables
 */
#define PROP_RO(klass, name) {#name, &klass::get_##name, 0}
#define PROP_RW(klass, name) {#name, &klass::get_##name, &klass::set_##name}
#define METHOD(klass, name) {#name, &klass::name}

/*
 * Macros for helping with persistence and unpersistence
 */
#define _PERS_TYPE(name, value, type) \
   lua_push ##type(L, value); \
   lua_setfield(L, -2, name)
#define PERS_INT32(name, value) _PERS_TYPE(name, value, int32)
#define PERS_UINT32(name, value) _PERS_TYPE(name, value, uint32)
#define PERS_STRING(name, value) _PERS_TYPE(name, value.c_str(), string)

#define _UNPERS_TYPE(name, value, type) lua_getfield(L, lua_upvalueindex(1), name); \
   value = luaL_check ##type(L, -1); \
   lua_pop(L, 1);
#define UNPERS_INT32(name, value) _UNPERS_TYPE(name, value, int32)
#define UNPERS_UINT32(name, value) _UNPERS_TYPE(name, value, uint32)
#define UNPERS_STRING(name, value) _UNPERS_TYPE(name, value, string)

#include "scripting/eris/lua.hpp"
#include "scripting/luna_impl.h"

/**
 * Base Class. All Luna class must derive from this
 */
class LunaClass {
	public:
		virtual ~LunaClass() {}
		virtual void __persist(lua_State *) = 0;
		virtual void __unpersist(lua_State *) = 0;
		virtual const char * get_modulename() = 0;
};

/**
 * Register the class as a Lua class (that is a metatable and a function to
 * create an object out of this table).
 *
 * The class will always be created in the namespace wl, sub_namespace can
 * be used to create the class in one depth deeper (e.g. wl.map)
 */
template <class T>
void register_class
	(lua_State * const L, char const * const sub_namespace = "",
	 bool return_metatable = false)
{
	int to_pop = 0;

	// Get wl table which MUST already exist
	lua_getglobal(L, "wl");
	to_pop ++;

	// push the wl sub namespace table onto the stack, if desired
	if (strlen(sub_namespace) != 0) {
		lua_getfield(L, -1, sub_namespace);
		to_pop ++;
	}

	m_add_constructor_to_lua<T>(L);
	m_add_instantiator_to_lua<T>(L);
	lua_pop(L, to_pop); // Pop everything we used so far.

	m_create_metatable_for_class<T>(L);

	m_register_properties_in_metatable<T, T>(L);
	m_register_methods_in_metatable<T, T>(L);

	if (!return_metatable)
		lua_pop(L, 1); // remove the Metatable
}
/**
 * Makes the first class a children of the second. Make sure that T is really a
 * child class of PT before calling this. This must also be called directly
 * after register_class, so that the Metatable index is still valid
 */
template <class T, class PT>
void add_parent(lua_State * L)
{
	m_register_properties_in_metatable<T, PT>(L);
	m_register_methods_in_metatable<T, PT>(L);
}

/*
 * Get the instance of this C object to lua. This is usually used
 * as last call in a function that should create a new Lua object
 */
template <class T>
int to_lua(lua_State * const L, T * const obj) {
	// Create a new table with some slots preallocated
	lua_createtable(L, 0, 30); // table

	// get the index of the new table on the stack
	int const newtable = lua_gettop(L);

	// push index of position of user data in our array
	lua_pushnumber(L, 0);  // table 0

	// Make a new userdata. A lightuserdata won't do since we want to assign
	// a metatable to it
	T * * const a = static_cast<T * * >(lua_newuserdata(L, sizeof(T *)));
	*a = obj;

	int const userdata = lua_gettop(L); // table 0 ud

	// Assign this metatable to this userdata. We only do this to add
	// garbage collection to this userdata, so that the destructor gets
	// called. As a (unwanted, but not critical) side effect we also add all
	// other methods to this object
	luaL_getmetatable(L, T::className); // table 0 ud mt
	lua_setmetatable (L, userdata);  // table 0 ud

	// table[ 0 ] = USERDATA;
	lua_settable(L, newtable); // table

	// Assign this metatable to the newly created table
	luaL_getmetatable(L, T::className);
	lua_setmetatable (L, newtable);

	// table
	return 1;
}

/*
 * Our userdata is saved in table[0]. This function fetches it and makes sure
 * that it is the correct userdata.
 */
template <class T>
T ** get_user_class(lua_State * const L, int narg) {
	m_extract_userdata_from_user_class<T>(L, narg);

	T ** rv = static_cast<T **>(luaL_checkudata(L, -1, T::className));
	lua_pop(L, 1);

	return rv;
}
/*
 * This forces the pointer to be a base class. ONLY use this if you are sure
 * that indeed the object is a base class, like you can be in __eq
 */
template <class T>
T ** get_base_user_class(lua_State * const L, int narg) {
	m_extract_userdata_from_user_class<T>(L, narg);

	T ** rv = static_cast<T * *>(lua_touserdata(L, -1));
	lua_pop(L, 1);

	return rv;
}


#endif /* end of include guard: __S_LUNA_H */
