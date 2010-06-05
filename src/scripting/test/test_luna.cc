#include <exception>
#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <lua.hpp>
#include "scripting/luna.h"

#include "scripting/luna_impl.h"

using namespace boost;

class L_Class : public LunaClass
{
	int x;
public:
	LUNA_CLASS_HEAD(L_Class);
	const char * get_modulename() {return "test";}
	L_Class() :x(123) {}
	L_Class(lua_State *L) :x(124) {}
	int test(lua_State *L) 
	{
		lua_pushuint32(L, x);
		return 1;
	}
	virtual void __persist(lua_State * L) {}
	virtual void __unpersist(lua_State * L) {}
};
const char L_Class::className[] = "Class";
const MethodType<L_Class> L_Class::Methods[] = {
	METHOD(L_Class, test),
	{0, 0},
};
const PropertyType<L_Class> L_Class::Properties[] = {
	{0, 0, 0},
};

const static struct luaL_reg wltest [] = {
	{0, 0}
};
const static struct luaL_reg wl [] = {
	{0, 0}
};

BOOST_AUTO_TEST_CASE(test_luna)
{
	char * script =
		"print( 'test_luna' )\n"
		"t = wl.test.Class()\n"
		"x = t:test()\n"
		"print( x )\n";
	shared_ptr<lua_State> L_ptr(lua_open(),&lua_close);
	lua_State*L = L_ptr.get();
	luaL_openlibs(L);

	luaL_register(L, "wl", wl);
	lua_pop(L, 1); // pop the table from the stack
	luaL_register(L, "wl.test", wltest);
	lua_pop(L, 1); // pop the table from the stack
	register_class<L_Class>(L, "test");

	BOOST_CHECK_EQUAL(0, luaL_loadbuffer(L, script, strlen(script), "testscript"));

	BOOST_CHECK_EQUAL(0, lua_pcall(L, 0, 1, 0));
	

}