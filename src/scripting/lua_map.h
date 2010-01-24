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

#ifndef LUA_MAP_H
#define LUA_MAP_H

#include <lua.hpp>

#include "economy/flag.h"
#include "logic/game.h"
#include "logic/warehouse.h"

#include "luna.h"

void luaopen_wlmap(lua_State *);


/*
 * Base class for all classes in wl.map
 */
class L_MapModuleClass : public LunaClass {
	public:
		const char * get_modulename() {return "map";}
};


class L_MapObject : public L_MapModuleClass {
	Widelands::Object_Ptr * m_ptr;

public:
	LUNA_CLASS_HEAD(L_MapObject);

	L_MapObject() : m_ptr(0) {}
	L_MapObject(Widelands::Map_Object & mo) {
		m_ptr = new Widelands::Object_Ptr(&mo);
	}
	L_MapObject(lua_State * L) : m_ptr(0) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}
	virtual ~L_MapObject() {
		if (m_ptr) {
			delete m_ptr;
			m_ptr = 0;
		}
	}

	virtual void __persist(lua_State * L);
	virtual void __unpersist(lua_State * L);

	/*
	 * attributes
	 */
	int get_serial(lua_State * L);

	/*
	 * Lua Methods
	 */
	int __eq(lua_State * L);
	int remove(lua_State * L);

	/*
	 * C Methods
	 */
private:
	Widelands::Map_Object * m_get(Widelands::Game & game, lua_State * L);
};


class L_BaseImmovable : public L_MapObject {
protected:
	Widelands::OPtr<Widelands::BaseImmovable>* m_biptr;

public:
	LUNA_CLASS_HEAD(L_BaseImmovable);

	L_BaseImmovable() : m_biptr(0) {}
	L_BaseImmovable(Widelands::BaseImmovable & mo) : L_MapObject(mo) {
		m_biptr = new Widelands::OPtr<Widelands::BaseImmovable>(&mo);
	}
	L_BaseImmovable(lua_State * L) : L_MapObject(L), m_biptr(0) {}
	virtual ~L_BaseImmovable() {
		if (m_biptr) {
			delete m_biptr;
			m_biptr = 0;
		}
	}

	virtual void __persist(lua_State * L);
	virtual void __unpersist(lua_State * L);

	/*
	 * Properties
	 */
	int get_size(lua_State * L);
	int get_name(lua_State * L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
private:
	Widelands::BaseImmovable * m_get(Widelands::Game & game, lua_State * L);
};

class L_PlayerImmovable : public L_BaseImmovable {
public:
	LUNA_CLASS_HEAD(L_PlayerImmovable);

	L_PlayerImmovable() {}
	L_PlayerImmovable(Widelands::PlayerImmovable & mo) : L_BaseImmovable(mo) {
	}
	L_PlayerImmovable(lua_State * L) : L_BaseImmovable(L) {}
	virtual ~L_PlayerImmovable() {}

	virtual void __persist(lua_State * L);
	virtual void __unpersist(lua_State * L);

	/*
	 * Properties
	 */
	int get_player(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
private:
	Widelands::PlayerImmovable * m_get(Widelands::Game & game, lua_State * L);

protected:
	Widelands::Ware_Index m_get_ware_index
		(lua_State * L, Widelands::PlayerImmovable*, const std::string& s);
	Widelands::Ware_Index m_get_worker_index
		(lua_State * L, Widelands::PlayerImmovable*, const std::string& s);
};

class L_Building : public L_PlayerImmovable {
public:
	LUNA_CLASS_HEAD(L_Building);

	L_Building() {}
	L_Building(Widelands::Building & mo) : L_PlayerImmovable(mo) {
	}
	L_Building(lua_State * L) : L_PlayerImmovable(L) {}
	virtual ~L_Building() {}

	virtual void __persist(lua_State * L);
	virtual void __unpersist(lua_State * L);

	/*
	 * Properties
	 */
	int get_building_type(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
private:
	Widelands::Building * m_get(Widelands::Game & game, lua_State * L);
};


class L_Flag : public L_PlayerImmovable {
public:
	LUNA_CLASS_HEAD(L_Flag);

	L_Flag() {}
	L_Flag(Widelands::Flag & mo) : L_PlayerImmovable(mo) {
	}
	L_Flag(lua_State * L) : L_PlayerImmovable(L) {}
	virtual ~L_Flag() {}

	/*
	 * Properties
	 */

	/*
	 * Lua Methods
	 */
	int add_ware(lua_State *);

	/*
	 * C Methods
	 */
private:
	Widelands::Flag * m_get(Widelands::Game & game, lua_State * L);
};


class L_Warehouse : public L_Building {
public:
	LUNA_CLASS_HEAD(L_Warehouse);

	L_Warehouse() {}
	L_Warehouse(Widelands::Warehouse & mo) : L_Building(mo) {
	}
	L_Warehouse(lua_State * L) : L_Building(L) {}
	virtual ~L_Warehouse() {}

	/*
	 * Properties
	 */

	/*
	 * Lua Methods
	 */
	int set_wares(lua_State*);
	int get_wares(lua_State*);
	int set_workers(lua_State*);
	int get_workers(lua_State*);

	/*
	 * C Methods
	 */
private:
	Widelands::Warehouse * m_get(Widelands::Game & game, lua_State * L);
};


class L_Field : public L_MapModuleClass {
	Widelands::FCoords m_c;
public:
	LUNA_CLASS_HEAD(L_Field);

	L_Field() {}
	L_Field
		(Widelands::Coordinate x, Widelands::Coordinate y,
		 Widelands::Field * f) : m_c(Widelands::Coords(x, y), f) {}
	L_Field(lua_State * L);
	~L_Field() {}

	virtual void __persist(lua_State * L);
	virtual void __unpersist(lua_State * L);

	/*
	 * Properties
	 */
	int get_x(lua_State * L);
	int get_y(lua_State * L);
	int get_height(lua_State * L);
	int set_height(lua_State * L);
	int get_immovable(lua_State * L);
	int get_terr(lua_State * L);
	int set_terr(lua_State * L);
	int get_terd(lua_State * L);
	int set_terd(lua_State * L);
	int get_rn(lua_State*);
	int get_ln(lua_State*);
	int get_trn(lua_State*);
	int get_tln(lua_State*);
	int get_bln(lua_State*);
	int get_brn(lua_State*);

	/*
	 * Lua methods
	 */
	int __eq(lua_State * L);
	int region(lua_State * L);

	/*
	 * C methods
	 */
	inline Widelands::FCoords & coords() {return m_c;}

private:
	int m_region(lua_State * L, uint32_t radius);
	int m_hollow_region(lua_State * L, uint32_t radius, uint32_t inner_radius);
};

int upcasted_immovable_to_lua(lua_State * L, Widelands::BaseImmovable * bi);

#endif
