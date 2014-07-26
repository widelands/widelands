/*
 * Copyright (C) 2011-2012 by the Widelands Development Team
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

#ifndef WL_ECONOMY_FLEET_H
#define WL_ECONOMY_FLEET_H

#include <boost/shared_ptr.hpp>

#include "base/macros.h"
#include "logic/instances.h"

namespace Widelands {

class Economy;
struct Flag;
class PortDock;
struct RoutingNodeNeighbour;
struct Ship;

class Fleet_Descr : public Map_Object_Descr {
public:
	Fleet_Descr(char const* const _name, char const* const _descname)
	   : Map_Object_Descr(Map_Object_Type::FLEET, _name, _descname) {
	}
	~Fleet_Descr() override {
	}

private:
	DISALLOW_COPY_AND_ASSIGN(Fleet_Descr);
};

/**
 * Manage all ships and ports of a player that are connected
 * by ocean.
 *
 * That is, two ports belong to the same fleet if - and only if - ships can
 * travel between them, and so on. Players may have several fleets, if they
 * build ports that cannot communicate (e.g. one port on the ocean, and another
 * in a lake).
 *
 * @paragraph Lifetime
 *
 * Fleet objects are created on-the-fly by @ref Ship and @ref PortDock,
 * and destroy themselves when they become empty.
 *
 * The intention is for fleet objects to merge automatically and separate
 * again in reaction to changes in the map. However, this may not work
 * properly at the moment.
 */
struct Fleet : Map_Object {
	struct PortPath {
		int32_t cost;
		boost::shared_ptr<Path> path;

		PortPath() : cost(-1) {}
	};

	const Fleet_Descr& descr() const;

	Fleet(Player & player);

	Player * get_owner() const {return &m_owner;}
	Player & owner() const {return m_owner;}

	PortDock * get_dock(Flag & flag) const;
	PortDock * get_arbitrary_dock() const;
	void set_economy(Economy * e);

	bool active() const;

	void init(Editor_Game_Base &) override;
	void cleanup(Editor_Game_Base &) override;
	void update(Editor_Game_Base &);

	void add_ship(Ship * ship);
	void remove_ship(Editor_Game_Base & egbase, Ship * ship);
	void add_port(Editor_Game_Base & egbase, PortDock * port);
	void remove_port(Editor_Game_Base & egbase, PortDock * port);

	void log_general_info(const Editor_Game_Base &) override;

	bool get_path(PortDock & start, PortDock & end, Path & path);
	void add_neighbours(PortDock & pd, std::vector<RoutingNodeNeighbour> & neighbours);

protected:
	void act(Game &, uint32_t data) override;

private:
	void find_other_fleet(Editor_Game_Base & egbase);
	void merge(Editor_Game_Base & egbase, Fleet * other);
	void check_merge_economy();
	void connect_port(Editor_Game_Base & egbase, uint32_t idx);

	PortPath & portpath(uint32_t i, uint32_t j);
	const PortPath & portpath(uint32_t i, uint32_t j) const;
	PortPath & portpath_bidir(uint32_t i, uint32_t j, bool & reverse);
	const PortPath & portpath_bidir(uint32_t i, uint32_t j, bool & reverse) const;

	Player & m_owner;
	std::vector<Ship *> m_ships;
	std::vector<PortDock *> m_ports;

	bool m_act_pending;

	/**
	 * Store all pairs shortest paths between port docks
	 *
	 * Let i < j, then the path from m_ports[i] to m_ports[j] is stored in
	 * m_portpaths[binom(j,2) + i]
	 */
	std::vector<PortPath> m_portpaths;

	// saving and loading
protected:
	struct Loader : Map_Object::Loader {
		Loader();

		void load(FileRead &, uint8_t version);
		void load_pointers() override;
		void load_finish() override;

	private:
		std::vector<uint32_t> m_ships;
		std::vector<uint32_t> m_ports;
	};

public:
	bool has_new_save_support() override {return true;}
	void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &) override;

	static Map_Object::Loader * load
		(Editor_Game_Base &, Map_Map_Object_Loader &, FileRead &);
};

} // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_FLEET_H
