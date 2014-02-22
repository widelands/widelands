/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#include <exception>
#include <memory>

#include <boost/test/unit_test.hpp>

#include "scripting/eris/lua.hpp"
#include "scripting/luna.h"
#include "scripting/luna_impl.h"

#ifndef BEGIN_LUNA_PROPERTIES
#define BEGIN_LUNA_PROPERTIES(klass) \
	const PropertyType<klass> klass::Properties[] = {

#define END_LUNA_PROPERTIES() {0, 0, 0}};
#endif

struct LuaCloser {
	void operator()(lua_State* L) {
		lua_close(L);
	}
};

class L_Class : public LunaClass
{
	int x;
	int prop;
public:
	LUNA_CLASS_HEAD(L_Class);
	const char * get_modulename() override {return "test";}
	L_Class() :x(123), prop(246) {}
	virtual ~L_Class() {}
	L_Class(lua_State * /* L */) :x(124), prop(248) {}
	virtual int test(lua_State* L)
	{
		lua_pushuint32(L, x);
		return 1;
	}
	virtual int get_prop1(lua_State* L)
	{
		lua_pushint32(L, prop);
		return 1;
	}
	virtual int get_propr(lua_State* L)
	{
		lua_pushint32(L, 1);
		return 1;
	}
	virtual int set_prop1(lua_State* L)
	{
		prop = lua_tointeger(L, -1);
		return 0;
	}
	virtual void __persist(lua_State * /* L */) override {}
	virtual void __unpersist(lua_State * /* L */) override {}
};
const char L_Class::className[] = "Class";
const MethodType<L_Class> L_Class::Methods[] = {
	METHOD(L_Class, test),
	{nullptr, nullptr},
};
BEGIN_LUNA_PROPERTIES(L_Class)
	PROP_RO(L_Class, propr),
	PROP_RW(L_Class, prop1),
END_LUNA_PROPERTIES()

class L_SubClass : public L_Class
{
	int y;
public:
	LUNA_CLASS_HEAD(L_SubClass);
	L_SubClass() : y(1230) {}
	L_SubClass(lua_State* L) : L_Class(L), y(1240) {}
	virtual int subtest(lua_State* L)
	{
		lua_pushuint32(L, y);
		return 1;
	}
	virtual void __persist(lua_State * /* L */) override {}
	virtual void __unpersist(lua_State * /* L */) override {}
};
const char L_SubClass::className[] = "SubClass";
const MethodType<L_SubClass> L_SubClass::Methods[] = {
	METHOD(L_SubClass, subtest),
	{nullptr, nullptr},
};
BEGIN_LUNA_PROPERTIES(L_SubClass)
END_LUNA_PROPERTIES()

class L_VirtualClass : public L_Class
{
	int z;
public:
	LUNA_CLASS_HEAD(L_VirtualClass);
	L_VirtualClass() :z(12300) {}
	L_VirtualClass(lua_State* L) : L_Class(L), z(12400) {}
	virtual int virtualtest(lua_State* L)
	{
		lua_pushuint32(L, z);
		return 1;
	}
	virtual void __persist(lua_State * /* L */) override {}
	virtual void __unpersist(lua_State * /* L */) override {}
};
const char L_VirtualClass::className[] = "VirtualClass";
const MethodType<L_VirtualClass> L_VirtualClass::Methods[] = {
	METHOD(L_VirtualClass, virtualtest),
	{nullptr, nullptr},
};
BEGIN_LUNA_PROPERTIES(L_VirtualClass)
END_LUNA_PROPERTIES()

class L_Second
{
public:
	int get_second(lua_State* L) {lua_pushint32(L, 2001); return 1;}
	virtual int multitest(lua_State* L)
	{
		lua_pushint32(L, 2002);
		return 1;
	}
};

class L_MultiClass : public L_Class, public L_Second
{
	int z;
public:
	LUNA_CLASS_HEAD(L_MultiClass);
	L_MultiClass () :z(12300) {}
	L_MultiClass (lua_State* L) : L_Class(L), z(12400) {}
	virtual int virtualtest(lua_State* L)
	{
		lua_pushuint32(L, z);
		return 1;
	}
	virtual void __persist(lua_State * /* L */) override {}
	virtual void __unpersist(lua_State * /* L */) override {}
};
const char L_MultiClass::className[] = "MultiClass";
const MethodType<L_MultiClass> L_MultiClass::Methods[] = {
	METHOD(L_MultiClass, virtualtest),
	METHOD(L_Second, multitest),
	{nullptr, nullptr},
};
BEGIN_LUNA_PROPERTIES(L_MultiClass)
	PROP_RO(L_MultiClass, second),
END_LUNA_PROPERTIES()

const static struct luaL_Reg wltest [] = {
	{nullptr, nullptr}
};
const static struct luaL_Reg wl [] = {
	{nullptr, nullptr}
};

int test_check_int(lua_State* L) {
	int a = lua_tointeger(L, -2);
	int b = lua_tointeger(L, -1);

	BOOST_CHECK_EQUAL(a, b);
	return 0;
}

void InitLuaTests(lua_State* L)
{
	luaL_openlibs(L);

	luaL_newlib(L, wl);  // S: wl_table
	lua_setglobal(L, "wl"); // S:

	lua_getglobal(L, "wl");  // S: wl_table
	lua_pushstring(L, "test"); // S: wl_table "test"
	luaL_newlib(L, wltest);  // S: wl_table "test" wl.test_table
	lua_settable(L, -3); // S: wl_table
	lua_getfield(L, -1, "test"); // S: wl_table wl.test_table

	lua_pushcfunction (L, &test_check_int);  // S: wl_table wl.test_table func
	lua_setfield(L, -2, "CheckInt");  // S: wl_table wl.test_table
	lua_pop(L, 2); // S:
}

BOOST_AUTO_TEST_CASE(test_luna_simple)
{
	const char * script1 =
		"t = wl.test.Class()\n"
		"wl.test.CheckInt(248,t.prop1)\n"
		"wl.test.CheckInt(124,t:test())\n"
		"t.prop1 = 999\n"
		"wl.test.CheckInt(999,t.prop1)\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	InitLuaTests(L);
	register_class<L_Class>(L, "test");

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script1, strlen(script1), "testscript1"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_property_ro)
{
	const char * script1 =
		"t = wl.test.Class()\n"
		"wl.test.CheckInt(1, t.propr)"
		"t.propr = 1\n"; // This final line should generate an arror

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	InitLuaTests(L);
	register_class<L_Class>(L, "test");

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script1, strlen(script1), "testscript1"));
	// Should get LUA runtime error instead of for example crashing
	BOOST_REQUIRE_EQUAL(LUA_ERRRUN, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_inheritance)
{
	const char * script2 =
		"t = wl.test.SubClass()\n"
		"wl.test.CheckInt(124, t:test())\n"
		"wl.test.CheckInt(248, t.prop1)\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	InitLuaTests(L);

	// single inheritance
	register_class<L_SubClass>(L, "test", true);
	add_parent<L_SubClass, L_Class>(L);
	lua_pop(L, 1); // Pop the meta table

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script2, strlen(script2), "testscript2"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_virtualbase_method)
{
	const char * script3 =
		"t = wl.test.VirtualClass()\n"
		"wl.test.CheckInt(124, t:test())\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	InitLuaTests(L);

	// virtual base class
	register_class<L_VirtualClass>(L, "test", true);
	add_parent<L_VirtualClass, L_Class>(L);
	lua_pop(L, 1); // Pop the meta table

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script3, strlen(script3), "testscript3"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_virtualbase_property)
{
	const char * script4 =
		"t = wl.test.VirtualClass()\n"
		"wl.test.CheckInt(248, t.prop1)\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	InitLuaTests(L);

	// virtual base class
	register_class<L_VirtualClass>(L, "test", true);
	add_parent<L_VirtualClass, L_Class>(L);
	lua_pop(L, 1); // Pop the meta table

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script4, strlen(script4), "testscript4"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_multibase_method)
{
	const char * script5 =
		"t = wl.test.MultiClass()\n"
		"wl.test.CheckInt(124, t:test())\n"
		"wl.test.CheckInt(2002, t:multitest())\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	InitLuaTests(L);

	// virtual base class
	register_class<L_MultiClass>(L, "test", true);
	add_parent<L_MultiClass, L_Class>(L);
	lua_pop(L, 1); // Pop the meta table

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script5, strlen(script5), "testscript5"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_multibase_property_get)
{
	const char * script6 =
		"t = wl.test.MultiClass()\n"
		"wl.test.CheckInt(248, t.prop1)\n"
		"wl.test.CheckInt(2001, t.second)\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	InitLuaTests(L);

	register_class<L_MultiClass>(L, "test", true);
	add_parent<L_MultiClass, L_Class>(L);
	lua_pop(L, 1); // Pop the meta table

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script6, strlen(script6), "testscript6"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_multibase_property_set)
{
	const char * script6 =
		"t = wl.test.MultiClass()\n"
		"wl.test.CheckInt(248, t.prop1)\n"
		"t.prop1 = 111\n"
		"wl.test.CheckInt(111, t.prop1)\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	InitLuaTests(L);

	register_class<L_MultiClass>(L, "test", true);
	add_parent<L_MultiClass, L_Class>(L);
	lua_pop(L, 1); // Pop the meta table

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script6, strlen(script6), "testscript6"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}
