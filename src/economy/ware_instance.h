/*
 * Copyright (C) 2004, 2006-2013 by the Widelands Development Team
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

#ifndef S__WARE_INSTANCE_H
#define S__WARE_INSTANCE_H

#include "economy/transfer.h"
#include "logic/instances.h"
#include "logic/ware_descr.h"
#include "logic/widelands.h"

namespace Widelands {

class Building;
class Economy;
class Editor_Game_Base;
class Game;
struct IdleWareSupply;
class Map_Object;
struct PlayerImmovable;
struct Transfer;

/**
 * WareInstance represents one item while it is being carried around.
 *
 * The WareInstance never draws itself; the carrying worker or the current flag
 * location are responsible for that.
 *
 * For robustness reasons, a WareInstance can only exist in a location that
 * assumes responsible for updating the instance's economy via \ref set_economy,
 * and that destroys the WareInstance when the location is destroyed.
 *
 * Currently, the location of a ware can be one of the following:
 * \li a \ref Flag
 * \li a \ref Worker that is currently carrying the ware
 * \li a \ref PortDock or \ref Ship where the ware is encapsulated in a \ref ShippingItem
 *     for seafaring
 */
class WareInstance : public Map_Object {
	friend struct Map_Waredata_Data_Packet;

	MO_DESCR(WareDescr);

public:
	WareInstance(Ware_Index, const WareDescr* const);
	~WareInstance();

	virtual int32_t get_type() const override;
	char const* type_name() const override {
		return "ware";
	}

	Map_Object* get_location(Editor_Game_Base& egbase) {
		return m_location.get(egbase);
	}
	Economy* get_economy() const {
		return m_economy;
	}
	Ware_Index descr_index() const {
		return m_descr_index;
	}

	void init(Editor_Game_Base&) override;
	void cleanup(Editor_Game_Base&) override;
	void act(Game&, uint32_t data) override;
	void update(Game&);

	void set_location(Editor_Game_Base&, Map_Object* loc);
	void set_economy(Economy*);

	void enter_building(Game&, Building& building);

	bool is_moving() const;
	void cancel_moving();

	PlayerImmovable* get_next_move_step(Game&);

	void set_transfer(Game&, Transfer&);
	void cancel_transfer(Game&);
	Transfer* get_transfer() const {
		return m_transfer;
	}

	virtual void log_general_info(const Editor_Game_Base& egbase) override;

private:
	Object_Ptr m_location;
	Economy* m_economy;
	Ware_Index m_descr_index;

	IdleWareSupply* m_supply;
	Transfer* m_transfer;
	Object_Ptr m_transfer_nextstep;  ///< cached PlayerImmovable, can be 0

	// loading and saving stuff
protected:
	struct Loader : Map_Object::Loader {
		Loader();

		void load(FileRead&);
		virtual void load_pointers() override;
		virtual void load_finish() override;

	private:
		uint32_t m_location;
		uint32_t m_transfer_nextstep;
		Transfer::ReadData m_transfer;
	};

public:
	virtual bool has_new_save_support() override {
		return true;
	}

	virtual void save(Editor_Game_Base&, Map_Map_Object_Saver&, FileWrite&) override;
	static Map_Object::Loader* load(Editor_Game_Base&, Map_Map_Object_Loader&, FileRead&);
};
}

#endif
