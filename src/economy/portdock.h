/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#ifndef ECONOMY_PORTDOCK_H
#define ECONOMY_PORTDOCK_H

#include "logic/immovable.h"
#include "logic/wareworker.h"
#include "shippingitem.h"

namespace Widelands {

struct Fleet;
struct RoutingNodeNeighbour;
struct Ship;
class Warehouse;

/**
 * The PortDock occupies the fields in the water at which ships
 * dock at a port. As such, this class cooperates closely with
 * @ref Warehouse to implement the port functionality.
 *
 * @ref WareInstance and @ref Worker that are waiting to be
 * transported by ship are stored in the PortDock instead of
 * the associated @ref WareHouse.
 *
 * @paragraph Lifetime
 *
 * The PortDock is created and removed by its owning warehouse.
 * Throughout the life of the PortDock, the corresponding @ref Warehouse
 * instance exists.
 *
 * @paragraph Limitations
 *
 * Currently, there is a 1:1 relationship between @ref Warehouse
 * and PortDock. In principle, it would be conceivable to have a
 * port that is on a land bridge and therefore close to two
 * disconnected bodies of water. Such a port would have to have
 * two PortDock that belong to the same @ref Warehouse, but have
 * separate @ref Fleet instances.
 * However, we expect this to be such a rare case that it is not
 * implemented at the moment.
 */
struct PortDock : PlayerImmovable {
	PortDock();

	void add_position(Widelands::Coords where);
	void set_warehouse(Warehouse * wh);
	Warehouse * get_warehouse() const {return m_warehouse;}

	Fleet * get_fleet() const {return m_fleet;}
	PortDock * get_dock(Flag & flag) const;
	bool get_need_ship() const {return m_need_ship;}

	virtual void set_economy(Economy *);

	virtual int32_t get_size() const throw ();
	virtual bool get_passable() const throw ();
	virtual int32_t get_type() const throw ();
	virtual char const * type_name() const throw ();

	virtual Flag & base_flag();
	virtual PositionList get_positions
		(const Editor_Game_Base &) const throw ();
	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords, const Point);
	virtual std::string const & name() const throw ();

	virtual void init(Editor_Game_Base &);
	virtual void cleanup(Editor_Game_Base &);

	void add_neighbours(std::vector<RoutingNodeNeighbour> & neighbours);

	void add_shippingitem(Game &, WareInstance &);
	void update_shippingitem(Game &, WareInstance &);

	void add_shippingitem(Game &, Worker &);
	void update_shippingitem(Game &, Worker &);

	void ship_arrived(Game &, Ship &);

	virtual void log_general_info(Editor_Game_Base const &);

	uint32_t count_waiting(WareWorker waretype, Ware_Index wareindex);

private:
	friend struct Fleet;

	void init_fleet(Editor_Game_Base & egbase);
	void set_fleet(Fleet * fleet);
	void _update_shippingitem(Game &, std::vector<ShippingItem>::iterator);
	void set_need_ship(Game &, bool need);

	Fleet * m_fleet;
	Warehouse * m_warehouse;
	PositionList m_dockpoints;
	std::vector<ShippingItem> m_waiting;
	bool m_need_ship;

	// saving and loading
protected:
	struct Loader : PlayerImmovable::Loader {
		Loader();

		void load(FileRead &, uint8_t version);
		virtual void load_pointers();
		virtual void load_finish();

	private:
		uint32_t m_warehouse;
		std::vector<ShippingItem::Loader> m_waiting;
	};

public:
	virtual bool has_new_save_support() {return true;}
	virtual void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &);

	static Map_Object::Loader * load
		(Editor_Game_Base &, Map_Map_Object_Loader &, FileRead &);
};

} // namespace Widelands

#endif // ECONOMY_PORTDOCK_H
