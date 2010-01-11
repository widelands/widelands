/*
 * This code was taken from the lua_users wiki:
 *     http://lua-users.org/wiki/SimplerCppBinding
 *
 * It is written by Lenny Palozzi. Our implementation was taken from
 *
 *	 http://plaidworld.com/
 *
 *	and is written by TheBunny. It was adapted here and there to suite the needs
 *	of widelands.
 *
 *	Another implementation that is rather new is LunaFour:
 * http://lua-users.org/wiki/LunaFourCode
 *
 * But it is not working right now.
 *
 * It was slightly adapted here and there to fit the needs of Widelands
 */

#ifndef __S_LUNA_H
#define __S_LUNA_H

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

// THIS IS MY CUSTOM VERSION OF LUNA

// C++
#include <string>
#include <vector>
#include <list>

using namespace std;

extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

// MAKES BINDINGS
template <class T> class Luna {

public:

	enum{
		NUMBER,
		STRING
	};

	struct PropertyType {
		const char *name;
		int (T::*getter)(lua_State*);
		int (T::*setter)(lua_State*);
    };

    struct FunctionType {
		const char *name;
		int (T::*function)(lua_State*);
    };

	// IF YOU NEED THE CALLING CONTEXT FROM IN YOUR C++ DESTRUCTOR
	//static lua_State *delete_state;

	// REGISTER CLASS AS A GLOBAL TABLE
	static void register_class( lua_State *L , const char* sub_namespace = "" ) {
		//printf("Luna:register_class:\n");


		// Get wl table which MUST already exist
		lua_getglobal(L, "wl" );

		if( strcmp( sub_namespace, "" ) != 0 ){
			// ADD OPTIONAL NAMESPACE
			lua_getfield(L, -1, sub_namespace);
		}
		// ADD CONSTRUCTOR FUNCTION WITH CLASS NAME
		lua_pushcfunction (L, &Luna < T >::constructor);
		lua_setfield(L, -2, T::className);

		lua_pop(L,1);

		// ADD METETABLE WITH CLASS NAME
		// STORE IT IN LUA_REGISTRYINDEX
		// OVERLOAD LUA TABLE FUNCTIONS
		luaL_newmetatable( L, T::className );
		int metatable = lua_gettop(L);

		// ADD FUNCTION OVERRIDES TO METATABLE
		lua_pushstring (L, "__gc" );
		lua_pushcfunction(L,  &Luna<T>::garbage_collect );
		lua_settable(L, metatable );

		lua_pushstring( L,"__index" );
		lua_pushcfunction(L, &Luna<T>::property_getter);
		lua_settable(L, metatable );

		lua_pushstring( L,"__newindex" );
		lua_pushcfunction(L, &Luna<T>::property_setter);
		lua_settable(L, metatable );


		// REGISTER PROPERTIES on METATABLE

		// ADDS PROPERTY NAMES AND DEFUALT VALUES TO THE METATABLE
		for ( int i = 0; T::Properties[i].name; i++ ) {
			// ADD NAME KEY
			lua_pushstring( L, T::Properties[i].name );
			lua_pushnumber( L, i );
			lua_settable( L, metatable );
		}


		// REGISTER FUNCTIONS
		// THIS MAKES A WRAPPER FUNCTION THAT WHEN CALLED TRIGGERS function_dispatch()
		// THE WRAPPER HOLDS A INDEX TO THE ARRAY OF FUNCTIONS TO CALL

		// MAKES A CLOSER WITH THE NAME? AND 1 VALUE OF i?
		// A CLOSURE IS A FUNCTION WITH BOUND VARS
		// WHEN CALLED THE FUNCTION CAN ACCESS THESE VARS
		for ( int i = 0; T::Functions[ i ].name; i++ ) {
			lua_pushstring( L, T::Functions[ i ].name);
			lua_pushnumber( L, i);
			lua_pushcclosure( L, &Luna<T>::function_dispatch, 1 );
			lua_settable( L, metatable );
		}


		// ADD GLOBAL REFERENCE TO THE METATABLE
		// SO WE CAN EASILY COPY FUNCTIONS INTO IT FROM LUA IF WE WANT
		//string metatable_name = T::className;
		//metatable_name += "_metatable";
		//lua_setglobal( L, metatable_name.c_str() );

    }

    static int constructor( lua_State *L ) {
		//printf("Luna:constructor:\n");

		// MAKE A NEW TABLE TO RETURN
		// AKA OUR NEW LUA OBJECT
		//lua_newtable( L );

		// PRE ALLOC SOME MEMORY FOR 30 HASH SLOT
		// OBJECTS USE 1
		lua_createtable( L, 0, 30 );

		// GET ADDRESS OF newtable ON STACK
		int newtable = lua_gettop( L );

		// PUSH INDEX OF USER DATA
		lua_pushnumber( L, 0 );

		// MAKE USER DATA ON THE STACK AND POINT IT TO LUNA OBJECT
		T** a = static_cast<T**>(lua_newuserdata(L, sizeof(T*) ));
		T* obj = new T( L );
		*a = obj;

		// FOR SEEING PROPERTIES IN DEBUGGER
		//const PropertyType *p;
		//p =  T::Properties;
		//p = (*a)->T::Properties;

		int userdata = lua_gettop( L );

		// GET CLASS META TABLE FROM GLOBAL LUA_REGISTRYINDEX
		luaL_getmetatable( L, T::className );

		// SET THE METATABLE ON USER DATA
		// WE DO THIS JUST TO ADD GC TO USER DATA
		// EVEN THO WE HAVE __index and __newindex OVERLOADED ALSO
		lua_setmetatable( L, userdata );


		// SET table[ 0 ] = USERDATA;
		lua_settable( L, newtable );

		// GET CLASS META TABLE FROM GLOBAL LUA_REGISTRYINDEX
		luaL_getmetatable( L, T::className );

		// SET THE METATABLE ON newtable
		// SET THE OBJECTS META TABLE
		lua_setmetatable( L, newtable );


		// *** MAYBE THESE CAN ALL BE ADDED WHEN WE REGISTER SINCE THEY ARE ALL
		// GOING IN THE META TABLE... which really only need to be done once
		// and not for each instance.
		// AH! only the properties get placed in the MT...
		// FUNCTIONS ARE NATIVE TO THE OBJECT TABLE

		// MOVE TO REGISTER
		/*
		// REGISTER PROPERTIES on METATABLE
		luaL_getmetatable( L, T::className );

		// ADDS PROPERTY NAMES AND DEFUALT VALUES TO THE TABLE
		for ( int i = 0; T::Properties[i].name; i++ ) {
			// ADD NAME KEY
			lua_pushstring( L, T::Properties[i].name );
			lua_pushnumber( L, i );
			lua_settable( L, -3 );
		}

		// POP OFF META
		lua_pop(L, 1);
		*/

		// REGISTER FUNCTIONS
		// THIS MAKES A WRAPPER FUNCTION THAT WHEN CALLED TRIGGERS function_dispatch()
		// THE WRAPPER HOLDS A INDEX TO THE ARRAY OF FUNCTIONS TO CALL

		// MAKES A CLOSER WITH THE NAME? AND 1 VALUE OF i?
		// A CLOUSER IS A FUNCTION WITH BOUND VARS
		// WHEN CALLED THE FUNCTION CAN ACCESS THESE VARS

		/*
		for ( int i = 0; T::Functions[ i ].name; i++ ) {
			lua_pushstring( L, T::Functions[ i ].name);
			lua_pushnumber( L, i);
			lua_pushcclosure( L, &Luna<T>::function_dispatch, 1 );
			lua_settable( L, newtable );
		}
		*/

		return 1;
    }

	static int property_getter(lua_State *L) {
		//printf("Luna:property_getter:\n");

		// GET VAR NAME OFF STACK
		//string _name = lua_tostring( L, 2 ); // UNCOMMENT THIS IS YOU NEED TO SEE THE NAME IN THE DEBUGGER
		//printf( _name.c_str() );
		//printf("\n");

		lua_pushvalue( L, 2 );

		// DO A NORMAL GET ON TABLE
		//lua_gettable( L, 1 );
		lua_rawget( L, 1 );

		// CHECK THAT WE FOUND THE VAR
		if( !lua_isnil( L, -1 ) ){
			// WE FOUND IT RETURN IT
			return 1;
		}

		// GET METABLE
		lua_getmetatable( L, 1 );

		// LOOK UP KEY IN METATABLE
		lua_pushvalue( L, 2 );
		lua_rawget( L, -2 );

		// CHECK IF ITS A INDEX TO A PROPERTY WE STORED
		// BACK IN REGISTER
		// NUMVERS == REGISTER
		if( lua_isnumber( L, -1 ) ){

			// GET ARRAY INDEX OF PROPERTY STORED IN METABLE
			int _index = lua_tonumber( L, -1 );

			// GET table[ 0 ]
			lua_pushnumber( L, 0 );
			//lua_gettable( L, 1 );
			lua_rawget( L, 1 );

			// GETS THE USER DATA FROM table[ 0 ]
			T** obj = static_cast<T**>( lua_touserdata( L, -1 ) );

			// POP OFF STACK
			//lua_pop( L, 1 );
			//lua_remove( L, -1 );

			// PUSH VALUE TO TOP OF STACK
			// FOR FUNCTION CALL
			lua_pushvalue( L, 3 );

			// CACHE ARRAY
			// const PropertyType* _properties = (*obj)->T::Properties;

			// CALL FUNCTION
			return ( (*obj)->*( T::Properties[ _index ].getter ) )(L);

		}


		// IT WAS NOT AN INDEX
		// RETURN WHATEVER IT IS
		return 1;

	}

	static int property_setter(lua_State *L) {
		// GET METABLE
		lua_getmetatable( L, 1 );

		// LOOK UP KEY IN META TABLE
		lua_pushvalue( L, 2 );
		//lua_gettable( L, -2 );
		lua_rawget( L, -2 );

		/*
		if( lua_isnil( L, -1 ) ){

			// POP KEY
			// POP METATABLE
			lua_pop( L, 2 );

			// PROPERTY NOT FOUND
			// DO A NORMAL SET
			lua_rawset( L, 1 );

			return 0;

		}
		*/

		// NUMBER IS AN INDEX TO THE PROPERTY ARRAY IN THE META TABLE
		if( lua_isnumber( L, -1 ) ){

			// GET ARRAY INDEX OF PROPERTY STORED IN METABLE
			int _index = lua_tonumber( L, -1 );
			//lua_pop( L, 1 );

			// POP OFF METATABLE
			//lua_pop( L, 1 );

			// GET table[ 0 ]
			lua_pushnumber( L, 0 );
			//lua_gettable( L, 1 );
			lua_rawget( L, 1 );

			// GETS THE USER DATA FROM table[ 0 ]
			T** obj = static_cast<T**>( lua_touserdata( L, -1 ) );

			// POP OFF USER DATA
			// THIS SHOULD LEAVE THE VALUE AT THE TOP OF THE STACK FOR THE FUNCTION CALL
			//lua_pop( L, 1 );

			// PUSH VALUE TO TOP OF STACK
			// FOR FUNCTION CALL
			lua_pushvalue( L, 3 );

			// CACHE ARRAY
			// const PropertyType* _properties = (*obj)->T::Properties;

			return ( (*obj)->*( T::Properties[ _index ].setter ) )(L);

		}
		else{
			// PROPERTY NOT FOUND

			// POP RESULT
			// POP METATABLE
			lua_pop( L, 2 );

			// DO A NORMAL SET ON TABLE
			lua_rawset( L, 1 );

			return 0;
		}

		return 0;

	}

	// FUNCTION DISPATCH
	// GETS CALLED ON THE OBJECTS METATABLE
    static int function_dispatch( lua_State *L ) {
		//printf("Luna:function_dispatch\n");

		// GET FUNCTION INDEX FROM CLOSURE
		int i = static_cast<int>(lua_tonumber( L, lua_upvalueindex( 1 ) ));

		// GET table[ 0 ]
		lua_pushnumber( L, 0 );
		//lua_gettable( L, 1 );
		lua_rawget( L, 1 );

		// GETS THE USER DATA FROM? / FOR? table[ 0 ]
		T** obj = static_cast<T**>( lua_touserdata(L, -1 ) );

		// POP OFF STACK
		//lua_remove(L, -1 );
		lua_pop( L, 1 );

		// CALL FUNCTION
		return ( (*obj)->*( T::Functions[ i ].function ) )( L );
    }

	// GC FOR USER DATA ONLY ???
    static int garbage_collect( lua_State *L ) {
		//printf( "Luna:garbage_collect\n" );

		T** obj = static_cast<T**>( luaL_checkudata( L, -1, T::className ) );

		// ADDED CHECK SINCE WE ARE HOOKING THE SAME METATABLE TO OUR LUA OBJECT
		// AS WE ARE HOOKING TO THE USERDATA
		if( obj ){
			//printf( "Luna Deleting the C++ Object \n" );
			delete ( *obj );
		}

		return 0;
    }

};

#endif /* end of include guard: __S_LUNA_H */


