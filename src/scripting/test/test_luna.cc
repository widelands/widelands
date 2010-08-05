#include <exception>
#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <lua.hpp>
#include "scripting/luna.h"

#include "scripting/luna_impl.h"

using namespace boost;

#ifndef BEGIN_LUNA_PROPERTIES
#define BEGIN_LUNA_PROPERTIES(klass) \
	const PropertyType<klass> klass::Properties[] = {

#define END_LUNA_PROPERTIES() {0, 0, 0}};
#endif

class L_Class : public LunaClass
{
	int x;
	int prop;
public:
	LUNA_CLASS_HEAD(L_Class);
	const char * get_modulename() {return "test";}
	L_Class() :x(123), prop(246) {}
	virtual ~L_Class() {}
	L_Class(lua_State *L) :x(124), prop(248) {}
	virtual int test(lua_State *L) 
	{
		lua_pushuint32(L, x);
		return 1;
	}
	virtual int get_prop1(lua_State*L)
	{
		lua_pushint32(L, prop);
		return 1;
	}
	virtual int get_propr(lua_State*L)
	{
		lua_pushint32(L, 0);
		return 1;
	}
	virtual int set_prop1(lua_State*L)
	{
		prop = lua_tointeger(L, -1);
		return 0;
	}
	virtual void __persist(lua_State * L) {}
	virtual void __unpersist(lua_State * L) {}
};
const char L_Class::className[] = "Class";
const MethodType<L_Class> L_Class::Methods[] = {
	METHOD(L_Class, test),
	{0, 0},
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
	L_SubClass() :y(1230) {}
	L_SubClass(lua_State *L) : L_Class(L), y(1240) {}
	virtual int subtest(lua_State *L) 
	{
		lua_pushuint32(L, y);
		return 1;
	}
	virtual void __persist(lua_State * L) {}
	virtual void __unpersist(lua_State * L) {}
};
const char L_SubClass::className[] = "SubClass";
const MethodType<L_SubClass> L_SubClass::Methods[] = {
	METHOD(L_SubClass, subtest),
	{0, 0},
};
BEGIN_LUNA_PROPERTIES(L_SubClass)
END_LUNA_PROPERTIES()

class L_VirtualClass : public virtual L_Class
{
	int z;
public:
	LUNA_CLASS_HEAD(L_VirtualClass);
	L_VirtualClass() :z(12300) {}
	L_VirtualClass(lua_State *L) : L_Class(L), z(12400) {}
	virtual int virtualtest(lua_State *L) 
	{
		lua_pushuint32(L, z);
		return 1;
	}
	virtual void __persist(lua_State * L) {}
	virtual void __unpersist(lua_State * L) {}
};
const char L_VirtualClass::className[] = "VirtualClass";
const MethodType<L_VirtualClass> L_VirtualClass::Methods[] = {
	METHOD(L_VirtualClass, virtualtest),
	{0, 0},
};
BEGIN_LUNA_PROPERTIES(L_VirtualClass)
END_LUNA_PROPERTIES()

class L_Second
{
public:
	int get_second(lua_State *L) {lua_pushstring(L, "second"); return 1;}
	virtual int multitest(lua_State *L) 
	{
		lua_pushstring(L, "secondfunc");
		return 1;
	}
};

class L_MultiClass : public L_Class, public L_Second
{
	int z;
public:
	LUNA_CLASS_HEAD(L_MultiClass );
	L_MultiClass () :z(12300) {}
	L_MultiClass (lua_State *L) : L_Class(L), z(12400) {}
	virtual int virtualtest(lua_State *L) 
	{
		lua_pushuint32(L, z);
		return 1;
	}
	virtual void __persist(lua_State * L) {}
	virtual void __unpersist(lua_State * L) {}
};
const char L_MultiClass::className[] = "MultiClass";
const MethodType<L_MultiClass> L_MultiClass::Methods[] = {
	METHOD(L_MultiClass, virtualtest),
	METHOD(L_Second, multitest),
	{0, 0},
};
BEGIN_LUNA_PROPERTIES(L_MultiClass)
	PROP_RO(L_MultiClass, second),
END_LUNA_PROPERTIES()

const static struct luaL_reg wltest [] = {
	{0, 0}
};
const static struct luaL_reg wl [] = {
	{0, 0}
};

BOOST_AUTO_TEST_CASE(test_luna_simple)
{
	const char * script1 =
		"print( 'test_luna simple class' )\n"
		"t = wl.test.Class()\n"
		"x = t:test()\n"
		"t.prop1 = 999\n"
		"p = t.prop1\n"
		"print( 'test(): ' .. x .. ' prop1: ' .. p )\n";

	shared_ptr<lua_State> L_ptr(lua_open(),&lua_close);
	lua_State*L = L_ptr.get();
	luaL_openlibs(L);

	luaL_register(L, "wl", wl);
	lua_pop(L, 1); // pop the table from the stack
	luaL_register(L, "wl.test", wltest);
	lua_pop(L, 1); // pop the table from the stack
	register_class<L_Class>(L, "test");

	BOOST_CHECK_EQUAL(0, luaL_loadbuffer(L, script1, strlen(script1), "testscript1"));
	BOOST_CHECK_EQUAL(0, lua_pcall(L, 0, 1, 0));
	
}
BOOST_AUTO_TEST_CASE(test_luna_property_ro)
{
	const char * script1 =
		"print( 'test_luna readonly property' )\n"
		"t = wl.test.Class()\n"
		"p = t.propr\n"
		"t.propr = 1\n";

	shared_ptr<lua_State> L_ptr(lua_open(),&lua_close);
	lua_State*L = L_ptr.get();
	luaL_openlibs(L);

	luaL_register(L, "wl", wl);
	lua_pop(L, 1); // pop the table from the stack
	luaL_register(L, "wl.test", wltest);
	lua_pop(L, 1); // pop the table from the stack
	register_class<L_Class>(L, "test");

	BOOST_CHECK_EQUAL(0, luaL_loadbuffer(L, script1, strlen(script1), "testscript1"));
	// Should get LUA runtime error instead of for example crashing
	BOOST_CHECK_EQUAL(LUA_ERRRUN, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_inheritance)
{
	const char * script2 =
		"print( 'test_luna simple inheritance' )\n"
		"t = wl.test.SubClass()\n"
		"x = t:test()\n"
		"p = t.prop1\n"
		"print( 'test(): ' .. x .. ' prop1: ' .. p )\n";

	shared_ptr<lua_State> L_ptr(lua_open(),&lua_close);
	lua_State*L = L_ptr.get();
	luaL_openlibs(L);

	luaL_register(L, "wl", wl);
	lua_pop(L, 1); // pop the table from the stack
	luaL_register(L, "wl.test", wltest);
	lua_pop(L, 1); // pop the table from the stack

	// single inheritance
	register_class<L_SubClass>(L, "test", true);
	add_parent<L_SubClass, L_Class>(L);
	lua_pop(L, 1); // Pop the meta table
	
	BOOST_CHECK_EQUAL(0, luaL_loadbuffer(L, script2, strlen(script2), "testscript2"));
	BOOST_CHECK_EQUAL(0, lua_pcall(L, 0, 1, 0));

}

BOOST_AUTO_TEST_CASE(test_luna_virtualbase_method)
{
	const char * script3 =
		"print( 'test_luna virtual inheritance method call' )\n"
		"t = wl.test.VirtualClass()\n"
		"x = t:test()\n"
		"print( 'test(): ' .. x )\n";

	shared_ptr<lua_State> L_ptr(lua_open(),&lua_close);
	lua_State*L = L_ptr.get();
	luaL_openlibs(L);

	luaL_register(L, "wl", wl);
	lua_pop(L, 1); // pop the table from the stack
	luaL_register(L, "wl.test", wltest);
	lua_pop(L, 1); // pop the table from the stack

	// virtual base class 
	register_class<L_VirtualClass>(L, "test", true);
	add_parent<L_VirtualClass, L_Class>(L);
	lua_pop(L, 1); // Pop the meta table

	BOOST_CHECK_EQUAL(0, luaL_loadbuffer(L, script3, strlen(script3), "testscript3"));
	BOOST_CHECK_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_virtualbase_property)
{
	const char * script4 =
		"print( 'test_luna virtual inheritance properties' )\n"
		"t = wl.test.VirtualClass()\n"
		"p = t.prop1\n"
		"print( 'prop1: ' .. p )\n";

	shared_ptr<lua_State> L_ptr(lua_open(),&lua_close);
	lua_State*L = L_ptr.get();
	luaL_openlibs(L);

	luaL_register(L, "wl", wl);
	lua_pop(L, 1); // pop the table from the stack
	luaL_register(L, "wl.test", wltest);
	lua_pop(L, 1); // pop the table from the stack

	// virtual base class 
	register_class<L_VirtualClass>(L, "test", true);
	add_parent<L_VirtualClass, L_Class>(L);
	lua_pop(L, 1); // Pop the meta table

	BOOST_CHECK_EQUAL(0, luaL_loadbuffer(L, script4, strlen(script4), "testscript4"));
	BOOST_CHECK_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_multibase_method)
{
	const char * script5 =
		"print( 'test_luna virtual inheritance method call' )\n"
		"t = wl.test.MultiClass()\n"
		"x = t:test()\n"
		"y = t:multitest()\n"
		"print( 'test(): ' .. x .. ' multitest(): ' .. y)\n";

	shared_ptr<lua_State> L_ptr(lua_open(),&lua_close);
	lua_State*L = L_ptr.get();
	luaL_openlibs(L);

	luaL_register(L, "wl", wl);
	lua_pop(L, 1); // pop the table from the stack
	luaL_register(L, "wl.test", wltest);
	lua_pop(L, 1); // pop the table from the stack

	// virtual base class 
	register_class<L_MultiClass>(L, "test", true);
	add_parent<L_MultiClass, L_Class>(L);
	lua_pop(L, 1); // Pop the meta table

	BOOST_CHECK_EQUAL(0, luaL_loadbuffer(L, script5, strlen(script5), "testscript5"));
	BOOST_CHECK_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_multibase_property_get)
{
	const char * script6 =
		"print( 'test_luna multiple inheritance properties' )\n"
		"t = wl.test.MultiClass()\n"
		"p1 = t.prop1\n"
		"p2 = t.second\n"
		"print( 'prop1: ' .. p1 .. ' second: ' .. p2 )\n";

	shared_ptr<lua_State> L_ptr(lua_open(),&lua_close);
	lua_State*L = L_ptr.get();
	luaL_openlibs(L);

	luaL_register(L, "wl", wl);
	lua_pop(L, 1); // pop the table from the stack
	luaL_register(L, "wl.test", wltest);
	lua_pop(L, 1); // pop the table from the stack

	register_class<L_MultiClass>(L, "test", true);
	add_parent<L_MultiClass, L_Class>(L);
	lua_pop(L, 1); // Pop the meta table

	BOOST_CHECK_EQUAL(0, luaL_loadbuffer(L, script6, strlen(script6), "testscript6"));
	BOOST_CHECK_EQUAL(0, lua_pcall(L, 0, 1, 0));
}

BOOST_AUTO_TEST_CASE(test_luna_multibase_property_set)
{
	const char * script6 =
		"print( 'test_luna multiple inheritance properties' )\n"
		"t = wl.test.MultiClass()\n"
		"t.prop1 = 123\n"
		"p1 = t.prop1\n"
		"print( 'prop1: ' .. p1 )\n";

	shared_ptr<lua_State> L_ptr(lua_open(),&lua_close);
	lua_State*L = L_ptr.get();
	luaL_openlibs(L);

	luaL_register(L, "wl", wl);
	lua_pop(L, 1); // pop the table from the stack
	luaL_register(L, "wl.test", wltest);
	lua_pop(L, 1); // pop the table from the stack

	register_class<L_MultiClass>(L, "test", true);
	add_parent<L_MultiClass, L_Class>(L);
	lua_pop(L, 1); // Pop the meta table

	BOOST_CHECK_EQUAL(0, luaL_loadbuffer(L, script6, strlen(script6), "testscript6"));
	BOOST_CHECK_EQUAL(0, lua_pcall(L, 0, 1, 0));
}
