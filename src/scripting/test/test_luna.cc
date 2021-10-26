/*
 * Copyright (C) 2010-2021 by the Widelands Development Team
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

#include <memory>

#include <cstring>

#include <boost/test/unit_test.hpp>

#include "base/macros.h"
#include "scripting/lua.h"
#include "scripting/luna.h"
#include "scripting/luna_impl.h"

// Triggered by BOOST_AUTO_TEST_CASE
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
CLANG_DIAG_OFF("-Wused-but-marked-unused")

#ifndef BEGIN_LUNA_PROPERTIES
#define BEGIN_LUNA_PROPERTIES(klass) const PropertyType<klass> klass::Properties[] = {

#define END_LUNA_PROPERTIES()                                                                      \
	{ 0, 0, 0 }                                                                                     \
	}                                                                                               \
	;
#endif

struct LuaCloser {
	void operator()(lua_State* L) {
		lua_close(L);
	}
};

class LuaClass : public LunaClass {
	int x = 0;
	int prop = 0;

public:
	LUNA_CLASS_HEAD(LuaClass);
	const char* get_modulename() override {
		return "test";
	}
	LuaClass() : x(123), prop(246) {
	}
	~LuaClass() override = default;
	explicit LuaClass(lua_State* /* L */) : x(124), prop(248) {
	}
	virtual int test(lua_State* L) {
		lua_pushuint32(L, x);
		return 1;
	}
	virtual int get_prop1(lua_State* L) {
		lua_pushint32(L, prop);
		return 1;
	}
	virtual int get_propr(lua_State* L) {
		lua_pushint32(L, 1);
		return 1;
	}
	virtual int set_prop1(lua_State* L) {
		prop = lua_tointeger(L, -1);
		return 0;
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* /* L */) override {
	}
	void __unpersist(lua_State* /* L */) override {
	}
	CLANG_DIAG_RESERVED_IDENTIFIER_ON
};

const char LuaClass::className[] = "Class";
const MethodType<LuaClass> LuaClass::Methods[] = {
   METHOD(LuaClass, test),
   {nullptr, nullptr},
};
BEGIN_LUNA_PROPERTIES(LuaClass)
PROP_RO(LuaClass, propr)
, PROP_RW(LuaClass, prop1),
   END_LUNA_PROPERTIES()

      class LuaSubClass : public LuaClass {
	int y = 0;

public:
	LUNA_CLASS_HEAD(LuaSubClass);
	LuaSubClass() : y(1230) {
	}
	explicit LuaSubClass(lua_State* L) : LuaClass(L), y(1240) {
	}
	virtual int subtest(lua_State* L);

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* /* L */) override {
	}
	void __unpersist(lua_State* /* L */) override {
	}
	CLANG_DIAG_RESERVED_IDENTIFIER_ON
};
const char LuaSubClass::className[] = "SubClass";
const MethodType<LuaSubClass> LuaSubClass::Methods[] = {
   METHOD(LuaSubClass, subtest),
   {nullptr, nullptr},
};
BEGIN_LUNA_PROPERTIES(LuaSubClass)
END_LUNA_PROPERTIES()

int LuaSubClass::subtest(lua_State* L) {
	lua_pushuint32(L, y);
	return 1;
}

class LuaVirtualClass : public LuaClass {
	int z = 0;

public:
	LUNA_CLASS_HEAD(LuaVirtualClass);
	LuaVirtualClass() : z(12300) {
	}
	explicit LuaVirtualClass(lua_State* L) : LuaClass(L), z(12400) {
	}
	virtual int virtualtest(lua_State* L);

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* /* L */) override {
	}
	void __unpersist(lua_State* /* L */) override {
	}
	CLANG_DIAG_RESERVED_IDENTIFIER_ON
};
const char LuaVirtualClass::className[] = "VirtualClass";
const MethodType<LuaVirtualClass> LuaVirtualClass::Methods[] = {
   METHOD(LuaVirtualClass, virtualtest),
   {nullptr, nullptr},
};
BEGIN_LUNA_PROPERTIES(LuaVirtualClass)
END_LUNA_PROPERTIES()

int LuaVirtualClass::virtualtest(lua_State* L) {
	lua_pushuint32(L, z);
	return 1;
}

class LuaSecond {
public:
	int get_second(lua_State* L) {
		lua_pushint32(L, 2001);
		return 1;
	}
	virtual ~LuaSecond() = default;
	virtual int multitest(lua_State* L) {
		lua_pushint32(L, 2002);
		return 1;
	}
};

class LuaMultiClass : public LuaClass, public LuaSecond {
	int z = 0;

public:
	LUNA_CLASS_HEAD(LuaMultiClass);
	LuaMultiClass() : z(12300) {
	}
	explicit LuaMultiClass(lua_State* L) : LuaClass(L), z(12400) {
	}
	virtual int virtualtest(lua_State* L);

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* /* L */) override {
	}
	void __unpersist(lua_State* /* L */) override {
	}
	CLANG_DIAG_RESERVED_IDENTIFIER_ON
};
const char LuaMultiClass::className[] = "MultiClass";
const MethodType<LuaMultiClass> LuaMultiClass::Methods[] = {
   METHOD(LuaMultiClass, virtualtest),
   METHOD(LuaSecond, multitest),
   {nullptr, nullptr},
};
BEGIN_LUNA_PROPERTIES(LuaMultiClass)
PROP_RO(LuaMultiClass, second)
, END_LUNA_PROPERTIES()

     int LuaMultiClass::virtualtest(lua_State* L) {
	lua_pushuint32(L, z);
	return 1;
}

const static struct luaL_Reg wltest[] = {{nullptr, nullptr}};
const static struct luaL_Reg wl[] = {{nullptr, nullptr}};

static int test_check_int(lua_State* L) {
	int a = lua_tointeger(L, -2);
	int b = lua_tointeger(L, -1);

	BOOST_CHECK_EQUAL(a, b);
	return 0;
}

static void init_lua_tests(lua_State* L) {
	luaL_openlibs(L);

	luaL_newlib(L, wl);      // S: wl_table
	lua_setglobal(L, "wl");  // S:

	lua_getglobal(L, "wl");       // S: wl_table
	lua_pushstring(L, "test");    // S: wl_table "test"
	luaL_newlib(L, wltest);       // S: wl_table "test" wl.test_table
	lua_settable(L, -3);          // S: wl_table
	lua_getfield(L, -1, "test");  // S: wl_table wl.test_table

	lua_pushcfunction(L, &test_check_int);  // S: wl_table wl.test_table func
	lua_setfield(L, -2, "CheckInt");        // S: wl_table wl.test_table
	lua_pop(L, 2);                          // S:
}

BOOST_AUTO_TEST_CASE(test_luna_simple) {
	const char* script1 = "t = wl.test.Class()\n"
	                      "wl.test.CheckInt(248,t.prop1)\n"
	                      "wl.test.CheckInt(124,t:test())\n"
	                      "t.prop1 = 999\n"
	                      "wl.test.CheckInt(999,t.prop1)\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	init_lua_tests(L);
	register_class<LuaClass>(L, "test");

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script1, strlen(script1), "testscript1"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_property_ro) {
	const char* script1 = "t = wl.test.Class()\n"
	                      "wl.test.CheckInt(1, t.propr)"
	                      "t.propr = 1\n";  // This final line should generate an arror

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	init_lua_tests(L);
	register_class<LuaClass>(L, "test");

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script1, strlen(script1), "testscript1"));
	// Should get LUA runtime error instead of for example crashing
	BOOST_REQUIRE_EQUAL(LUA_ERRRUN, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_inheritance) {
	const char* script2 = "t = wl.test.SubClass()\n"
	                      "wl.test.CheckInt(124, t:test())\n"
	                      "wl.test.CheckInt(248, t.prop1)\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	init_lua_tests(L);

	// single inheritance
	register_class<LuaSubClass>(L, "test", true);
	add_parent<LuaSubClass, LuaClass>(L);
	lua_pop(L, 1);  // Pop the meta table

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script2, strlen(script2), "testscript2"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_virtualbase_method) {
	const char* script3 = "t = wl.test.VirtualClass()\n"
	                      "wl.test.CheckInt(124, t:test())\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	init_lua_tests(L);

	// virtual base class
	register_class<LuaVirtualClass>(L, "test", true);
	add_parent<LuaVirtualClass, LuaClass>(L);
	lua_pop(L, 1);  // Pop the meta table

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script3, strlen(script3), "testscript3"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_virtualbase_property) {
	const char* script4 = "t = wl.test.VirtualClass()\n"
	                      "wl.test.CheckInt(248, t.prop1)\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	init_lua_tests(L);

	// virtual base class
	register_class<LuaVirtualClass>(L, "test", true);
	add_parent<LuaVirtualClass, LuaClass>(L);
	lua_pop(L, 1);  // Pop the meta table

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script4, strlen(script4), "testscript4"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_multibase_method) {
	const char* script5 = "t = wl.test.MultiClass()\n"
	                      "wl.test.CheckInt(124, t:test())\n"
	                      "wl.test.CheckInt(2002, t:multitest())\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	init_lua_tests(L);

	// virtual base class
	register_class<LuaMultiClass>(L, "test", true);
	add_parent<LuaMultiClass, LuaClass>(L);
	lua_pop(L, 1);  // Pop the meta table

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script5, strlen(script5), "testscript5"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_multibase_property_get) {
	const char* script6 = "t = wl.test.MultiClass()\n"
	                      "wl.test.CheckInt(248, t.prop1)\n"
	                      "wl.test.CheckInt(2001, t.second)\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	init_lua_tests(L);

	register_class<LuaMultiClass>(L, "test", true);
	add_parent<LuaMultiClass, LuaClass>(L);
	lua_pop(L, 1);  // Pop the meta table

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script6, strlen(script6), "testscript6"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_multibase_property_set) {
	const char* script6 = "t = wl.test.MultiClass()\n"
	                      "wl.test.CheckInt(248, t.prop1)\n"
	                      "t.prop1 = 111\n"
	                      "wl.test.CheckInt(111, t.prop1)\n";

	std::unique_ptr<lua_State, LuaCloser> L_ptr(luaL_newstate());
	lua_State* L = L_ptr.get();
	init_lua_tests(L);

	register_class<LuaMultiClass>(L, "test", true);
	add_parent<LuaMultiClass, LuaClass>(L);
	lua_pop(L, 1);  // Pop the meta table

	BOOST_REQUIRE_EQUAL(0, luaL_loadbuffer(L, script6, strlen(script6), "testscript6"));
	BOOST_REQUIRE_EQUAL(0, lua_pcall(L, 0, 1, 0));
}
