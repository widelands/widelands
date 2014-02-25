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

#ifndef ECONOMY_PORTDOCK_H
#define ECONOMY_PORTDOCK_H

#include "logic/immovable.h"
#include "logic/wareworker.h"
#include "economy/shippingitem.h"

namespace Widelands {

struct Fleet;
struct RoutingNodeNeighbour;
struct Ship;
class Warehouse;
class ExpeditionBootstrap;

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
class PortDock : public PlayerImmovable {
public:
	PortDock(Warehouse* warehouse);
	virtual ~PortDock();

	void add_position(Widelands::Coords where);
	Warehouse * get_warehouse() const;

	Fleet * get_fleet() const {return m_fleet;}
	PortDock * get_dock(Flag & flag) const;
	bool get_need_ship() const {return m_need_ship || m_expedition_ready;}

	virtual void set_economy(Economy *) override;

	virtual int32_t get_size() const override;
	virtual bool get_passable() const override;
	virtual int32_t get_type() const override;
	virtual char const * type_name() const override;

	virtual Flag & base_flag() override;
	virtual PositionList get_positions
		(const Editor_Game_Base &) const override;
	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords&, const Point&) override;
	virtual const std::string & name() const override;

	virtual void init(Editor_Game_Base &) override;
	virtual void cleanup(Editor_Game_Base &) override;

	void add_neighbours(std::vector<RoutingNodeNeighbour> & neighbours);

	void add_shippingitem(Game &, WareInstance &);
	void update_shippingitem(Game &, WareInstance &);

	void add_shippingitem(Game &, Worker &);
	void update_shippingitem(Game &, Worker &);

	void ship_arrived(Game &, Ship &);

	virtual void log_general_info(const Editor_Game_Base &) override;

	uint32_t count_waiting(WareWorker waretype, Ware_Index wareindex);

	// Returns true if a expedition is started or ready to be send out.
	bool expedition_started();

	// Called when the button in the warehouse window is pressed.
	void start_expedition();
	void cancel_expedition(Game &);

	// May return nullptr when there is no expedition ongoing or if the
	// expedition ship is already underway.
	ExpeditionBootstrap* expedition_bootstrap();

	// Gets called by the ExpeditionBootstrap as soon as all wares and workers are available.
	void expedition_bootstrap_complete(Game& game);

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
	bool m_expedition_ready;

	std::unique_ptr<ExpeditionBootstrap> m_expedition_bootstrap;

	// saving and loading
protected:
	class Loader : public PlayerImmovable::Loader {
	public:
		Loader();

		void load(FileRead &, uint8_t version);
		virtual void load_pointers() override;
		virtual void load_finish() override;

	private:
		uint32_t m_warehouse;
		std::vector<ShippingItem::Loader> m_waiting;
	};

public:
	virtual bool has_new_save_support() override {return true;}
	virtual void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &) override;

	static Map_Object::Loader * load
		(Editor_Game_Base &, Map_Map_Object_Loader &, FileRead &);
};

} // namespace Widelands

#endif // ECONOMY_PORTDOCK_H
