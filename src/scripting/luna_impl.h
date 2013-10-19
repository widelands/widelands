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
 * This file is only included by luna.h. It hides the gory details
 * of implementation. Since most stuff is templated, we have to keep
 * it in a header file, not an implementation file
 */

#ifndef LUNA_IMPL_H
#define LUNA_IMPL_H

#include "scripting/c_utils.h"
#include "scripting/eris/lua.hpp"

int luna_unpersisting_closure(lua_State * L);

/**
 * Descriptions for the Properties/Methods that should be available to Lua
 */
template <class T>
struct PropertyType {
	char const * name;
	int (T::*getter)(lua_State *);
	int (T::*setter)(lua_State *);
};
template <class T>
struct MethodType {
	char const * name;
	int (T::*method)(lua_State *);
};


// Forward declaration of public function, because we need it below
template <class T> int to_lua(lua_State * L, T * obj);
template <class T> T * * get_user_class(lua_State * const L, int narg);

template <class T>
int m_dispatch_property_in_metatable(lua_State * const L, bool setter) {
	// stack for getter: table name
	// stack for setter: table name value
	int ret = 0;
	// Look up the key in the metatable
	lua_getmetatable(L,  1); // table name <value> mt
	lua_pushvalue   (L,  2); // table name <value> mt name
	lua_rawget      (L, -2); // table name <value> mt mt_val
	lua_remove(L, -2); // table name <value> mt_val
	if (lua_istable(L, -1)) {
		// dispatcher

		lua_pushstring(L, "dispatcher"); // table name <value> mt_val "dispatcher"
		lua_gettable(L, -2); // table name <value> mt_val dispatcher_val
		if (!lua_iscfunction(L, -1))
		{
			lua_pop(L, 2); //  table name <value>
			return report_error(L, "invalid property without dispatcher function");
		}
		lua_CFunction dispatcher = lua_tocfunction(L, -1);
		lua_pop(L, 1); // table name <value> mt_val

		// get property method to stack
		lua_pushstring(L, setter ? "setter" : "getter");
		lua_gettable(L, -2); // table name <value> mt_val getter_val/setter_val
		lua_remove(L, -2); // table name <value> getter_val/setter_val
		// dispatcher pops off getter/setter and returns whatever
		// get/set property functions returns
		ret = dispatcher(L); // table name value
	} else {
		if (setter) {
			lua_rawset(L, 1);
		} else {
			// leave the value to stack
			ret = 1;
		}
	}

	return ret;
}

/**
 * Handler for properties. We first try to return the object if it is in
 * our table. If not, we check if this is a registered property in the
 * metatable. If so, we call our getter method and return the value.
 * Otherwise, returns nil.
 *
 * Stack at call:
 *   idx 2: string - name of variable requested
 *   idx 1: table  - current objects table
 */
template <class T>
int m_property_getter(lua_State * const L) {
	// Try a normal get on the table
	lua_pushvalue(L, 2); // table name name
	lua_rawget   (L, 1); // table name val?

	if (!lua_isnil(L, -1)) {
		// Found in the table, we return it
		return 1;
	}
	lua_pop(L, 1); // table name

	return m_dispatch_property_in_metatable<T>(L, false);
}

template <class T>
int m_property_setter(lua_State * const L) {
	// stack: table name value
	return  m_dispatch_property_in_metatable<T>(L, true);
}

/**
 * This function calls a lua method in our given object.  we can already be
 * sure that the called method is registered in our metatable, but we have to
 * make sure that the method is called correctly: obj.method(obj) or
 * obj:method(). We also check that we are not called with another object
 */
template <class T, class PT>
int m_property_dispatch(lua_State * const L) {
	// Check for invalid: obj.method()
	int const n = lua_gettop(L);
	if (!n)
		return report_error(L, "Property needs at least the object as argument!");

	// Check for invalid: obj.method(plainOldDatatype)
	luaL_checktype(L, 1, LUA_TTABLE);

	typedef int (PT::* const * ConstMethodPtr)(lua_State *);
	ConstMethodPtr pfunc = reinterpret_cast<ConstMethodPtr>
		(lua_touserdata(L, -1));
	lua_pop(L, 1);

	T ** const obj = get_user_class<T>(L, 1);

	if (!*pfunc)
	{
		return report_error(L, "The property is read-only!\n");
	}
	// Call it on our instance
	return ((*obj)->*(*pfunc))(L);
}

/**
 * This function calls a lua method in our given object.  we can already be
 * sure that the called method is registered in our metatable, but we have to
 * make sure that the method is called correctly: obj.method(obj) or
 * obj:method(). We also check that we are not called with another object
 */
template <class T, class PT>
int m_method_dispatch(lua_State * const L) {
	// Check for invalid: obj.method()
	int const n = lua_gettop(L);
	if (!n)
		return report_error(L, "Method needs at least the object as argument!");

	// Check for invalid: obj.method(plainOldDatatype)
	luaL_checktype(L, 1, LUA_TTABLE);

	// Get the method pointer from the closure
	typedef int (PT::* const * ConstMethod)(lua_State *);
	ConstMethod func = reinterpret_cast<ConstMethod>
		(lua_touserdata(L, lua_upvalueindex(1)));

	T * * const obj = get_user_class<T>(L, 1);

	// Call it on our instance
	return ((*obj)->*(*func))(L);
}

/**
 * Deletes a given object, as soon as Lua wants to get rid of it
 */
template <class T>
int m_garbage_collect(lua_State* const L) {
	// This method is called in two cases - either the userdata that we store at
	// key 0 in all of our objects is deleted or the table that represents our
	// classes itself is deleted. If it is the table, the following check will
	// return nullptr and we have nothing else today. In other cases, we have to
	// delete our object.
	T** const obj = static_cast<T**>(luaL_testudata(L, -1, T::className));
	if (!obj)
		return 0;

	delete *obj;
	return 0;
}


/**
 * Object creation from lua. The object needs a constructor that only
 * takes a lua_State. If direct creation of this object is not desired,
 * use report_error in the constructor to tell this the user
 */
template <class T>
int m_constructor(lua_State * const L) {
	return to_lua<T>(L, new T(L));
}

template <class T>
void m_add_constructor_to_lua(lua_State * const L) {
	lua_pushcfunction (L, &m_constructor<T>);
	lua_setfield(L, -2, T::className);
}

/*
 * The instantiator creates an empty class. It is only used to immediately load
 * objects from a saved game. The instantiator has __ in front of its name.
 * So: ClassName == Constructor
 *   __ClassName == Instantiator
 */
template <class T>
int m_instantiator(lua_State * const L) {
	return to_lua<T>(L, new T());
}

template <class T>
void m_add_instantiator_to_lua(lua_State * const L) {
	std::string s = std::string("__") + T::className;
	lua_pushcfunction (L, &m_instantiator<T>);
	lua_setfield(L, -2, s.c_str());
}

template <class T>
int m_persist(lua_State * const L) {
	assert(lua_gettop(L) == 1); // S: lightuserdata
	T * * const obj = get_user_class<T>(L, 1);

	lua_newtable(L);  // S: user_obj table

	lua_pushstring(L, (*obj)->get_modulename());
	lua_setfield(L, -2, "module");

	lua_pushstring(L, T::className);
	lua_setfield(L, -2, "class");

	assert(lua_gettop(L) == 2); // S: user_obj table
	(*obj)->__persist(L);
	assert(lua_gettop(L) == 2); // S: user_obj table

	lua_pushcclosure(L, &luna_unpersisting_closure, 1);
	assert(lua_gettop(L) == 2); // S: user_obj closure

	return 1;
}


template <class T>
int m_create_metatable_for_class(lua_State * const L) {
	luaL_newmetatable(L, T::className);
	int const metatable = lua_gettop(L);

	// OVERLOAD LUA TABLE FUNCTIONS
	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, &m_garbage_collect<T>);
	lua_settable  (L, metatable);

	lua_pushstring(L, "__index");
	lua_pushcfunction(L, &m_property_getter<T>);
	lua_settable  (L, metatable);

	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, &m_property_setter<T>);
	lua_settable  (L, metatable);

	lua_pushstring(L, "__persist");
	lua_pushcfunction(L, &m_persist<T>);
	lua_settable  (L, metatable);

	return metatable;
}

template <class T, class PT>
void m_register_properties_in_metatable
	(lua_State * const L)
{
	for (int i = 0; PT::Properties[i].name; ++i) {
		// metatable[prop_name] = Pointer to getter setter
		lua_pushstring(L, PT::Properties[i].name);
		lua_newtable(L);
		lua_pushstring(L, "getter");
		lua_pushlightuserdata
			(L,
			 const_cast<void *>
			 	(reinterpret_cast<const void *>
					(&(PT::Properties[i].getter))));
		lua_settable(L, -3);
		lua_pushstring(L, "setter");
		lua_pushlightuserdata
			(L,
			 const_cast<void *>
			 	(reinterpret_cast<const void *>
					(&(PT::Properties[i].setter))));
		lua_settable(L, -3);

		lua_pushstring(L, "dispatcher");
		lua_pushcfunction(L, &(m_property_dispatch<T, PT>));
		lua_settable(L, -3);

		lua_settable(L, -3); // Metatable is directly before our pushed stuff
	}
}

template <class T, class PT>
void m_register_methods_in_metatable(lua_State * const L)
{
	// We add a lua C closure around the call, the closure gets the pointer to
	// the c method to call as its only argument. We can then use
	// method_dispatch as a one-fits-all caller
	for (int i = 0; PT::Methods[i].name; ++i) {
		lua_pushstring(L, PT::Methods[i].name);
		lua_pushlightuserdata
			(L,
			 const_cast<void *>
			 	(reinterpret_cast<void const *>(&PT::Methods[i].method)));
		lua_pushcclosure(L, &(m_method_dispatch<T, PT>), 1);
		lua_settable(L, -3); // Metatable is directly before our pushed stuff
	}
}

/*
 * Get the userdata in a given stack object
 */
template <class T>
void m_extract_userdata_from_user_class(lua_State * const L, int narg) {
	luaL_checktype(L, narg, LUA_TTABLE);

	//  GET table[0]
	lua_pushnumber(L, 0);
	if (narg > 0)
		lua_rawget(L, narg);
	else
		lua_rawget(L, narg - 1);

	if (!lua_isuserdata(L, -1)) {
		report_error(L, "Expected a userdata, but got something else.");
	}
}

#endif
