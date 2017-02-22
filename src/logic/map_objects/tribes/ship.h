/*
 * Copyright (C) 2010-2017 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_SHIP_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_SHIP_H

#include <list>
#include <memory>

#include "base/macros.h"
#include "economy/shippingitem.h"
#include "graphic/diranimations.h"
#include "logic/map_objects/bob.h"

namespace Widelands {

class Economy;
struct Fleet;
class PortDock;

// This can't be part of the Ship class because of forward declaration in game.h
enum class IslandExploreDirection {
	kCounterClockwise = 0,  // This comes first for savegame compatibility (used to be = 0)
	kClockwise = 1,
	kNotSet
};

struct NoteShip {
	CAN_BE_SENT_AS_NOTE(NoteId::Ship)

	Ship* ship;

	enum class Action { kStateChanged, kDestinationChanged, kWaitingForCommand, kLost, kGained };
	Action action;

	NoteShip(Ship* const init_ship, const Action& init_action)
		: ship(init_ship), action(init_action) {
	}
};


class ShipDescr : public BobDescr {
public:
	ShipDescr(const std::string& init_descname, const LuaTable& t);
	~ShipDescr() override {
	}

	Bob& create_object() const override;

	uint32_t movecaps() const override;
	const DirAnimations& get_sail_anims() const {
		return sail_anims_;
	}

	Quantity get_capacity() const {
		return capacity_;
	}

private:
	DirAnimations sail_anims_;
	Quantity capacity_;
	DISALLOW_COPY_AND_ASSIGN(ShipDescr);
};

/**
 * Ships belong to a player and to an economy. The usually are in a (unique)
 * fleet for a player, but only if they are on standard duty. Exploration ships
 * are an economy of their own and are not part of a Fleet.
 */
struct Ship : Bob {
	MO_DESCR(ShipDescr)

	Ship(const ShipDescr& descr);
	virtual ~Ship();

	// Returns the fleet the ship is a part of.
	Fleet* get_fleet() const;

	// Returns the current destination or nullptr if there is no current
	// destination.
	PortDock* get_destination(EditorGameBase& egbase) const;

	// Returns the last visited portdock of this ship or nullptr if there is none or
	// the last visited was removed.
	PortDock* get_lastdock(EditorGameBase& egbase) const;

	Economy* get_economy() const {
		return economy_;
	}
	void set_economy(Game&, Economy* e);
	void set_destination(Game&, PortDock&);

	void init_auto_task(Game&) override;

	void init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;

	void start_task_ship(Game&);
	void start_task_movetodock(Game&, PortDock&);
	void start_task_expedition(Game&);

	uint32_t calculate_sea_route(Game& game, PortDock& pd, Path* finalpath = nullptr) const;

	void log_general_info(const EditorGameBase&) override;

	uint32_t get_nritems() const {
		return items_.size();
	}
	const ShippingItem& get_item(uint32_t idx) const {
		return items_[idx];
	}

	void withdraw_items(Game& game, PortDock& pd, std::vector<ShippingItem>& items);
	void add_item(Game&, const ShippingItem& item);

	// A ship with task expedition can be in four states: kExpeditionWaiting, kExpeditionScouting,
	// kExpeditionPortspaceFound or kExpeditionColonizing in the first states, the owning player of
	// this ship
	// can give direction change commands to change the direction of the moving ship / send the ship
	// in a
	// direction. Once the ship is on its way, it is in kExpeditionScouting state. In the backend, a
	// click
	// on a direction begins to the movement into that direction until a coast is reached or the user
	// cancels the direction through a direction change.
	//
	// The kExpeditionWaiting state means, that an event happend and thus the ship stopped and waits
	// for a
	// new command by the owner. An event leading to a kExpeditionWaiting state can be:
	// * expedition is ready to start
	// * new island appeared in vision range (only outer ring of vision range has to be checked due
	// to the
	//   always ongoing movement).
	// * island was completely surrounded
	//
	// The kExpeditionPortspaceFound state means, that a port build space was found.
	//
	enum class ShipStates : uint8_t {
		kTransport = 0,
		kExpeditionWaiting = 1,
		kExpeditionScouting = 2,
		kExpeditionPortspaceFound = 3,
		kExpeditionColonizing = 4,
		kSinkRequest = 8,
		kSinkAnimation = 9
	};

	/// \returns the current state the ship is in
	ShipStates get_ship_state() const {
		return ship_state_;
	}

	/// \returns the current name of ship
	const std::string& get_shipname() const {
		return shipname_;
	}

	/// \returns whether the ship is currently on an expedition
	bool state_is_expedition() const {
		return (ship_state_ == ShipStates::kExpeditionScouting ||
		        ship_state_ == ShipStates::kExpeditionWaiting ||
		        ship_state_ == ShipStates::kExpeditionPortspaceFound ||
		        ship_state_ == ShipStates::kExpeditionColonizing);
	}
	/// \returns whether the ship is in transport mode
	bool state_is_transport() const {
		return (ship_state_ == ShipStates::kTransport);
	}
	/// \returns whether a sink request for the ship is currently valid
	bool state_is_sinkable() const {
		return (ship_state_ != ShipStates::kSinkRequest &&
		        ship_state_ != ShipStates::kSinkAnimation &&
		        ship_state_ != ShipStates::kExpeditionColonizing);
	}

	/// \returns (in expedition mode only!) whether the next field in direction \arg dir is swimmable
	bool exp_dir_swimmable(Direction dir) const {
		if (!expedition_)
			return false;
		return expedition_->swimmable[dir - 1];
	}

	// whether the ship's expedition is in state "island-exploration" (circular movement)
	bool is_exploring_island() {
		return expedition_->island_exploration;
	}

	/// \returns whether the expedition ship is close to the coast
	bool exp_close_to_coast() const {
		if (!expedition_)
			return false;
		for (uint8_t dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir)
			if (!expedition_->swimmable[dir - 1])
				return true;
		return false;
	}

	/// \returns (in expedition mode only!) the list of currently seen port build spaces
	const std::vector<Coords>& exp_port_spaces() const {
		return expedition_->seen_port_buildspaces;
	}

	void exp_scouting_direction(Game&, WalkingDir);
	void exp_construct_port(Game&, const Coords&);
	void exp_explore_island(Game&, IslandExploreDirection);

	// Returns integer of direction, or WalkingDir::IDLE if query invalid
	// Intended for LUA scripting
	WalkingDir get_scouting_direction() const;

	// Returns integer of direction, or IslandExploreDirection::kNotSet
	// if query invalid
	// Intended for LUA scripting
	IslandExploreDirection get_island_explore_direction() const;

	void exp_cancel(Game&);
	void sink_ship(Game&);

protected:
	void draw(const EditorGameBase&,
	          const TextToDraw& draw_text,
	          const Vector2f& field_on_dst,
	          float scale,
	          RenderTarget* dst) const override;

private:
	friend struct Fleet;

	void wakeup_neighbours(Game&);

	static const Task taskShip;

	void ship_update(Game&, State&);
	void ship_wakeup(Game&);

	bool ship_update_transport(Game&, State&);
	void ship_update_expedition(Game&, State&);
	void ship_update_idle(Game&, State&);
	/// Set the ship's state to 'state' and if the ship state has changed, publish a notification.
	void set_ship_state_and_notify(ShipStates state, NoteShip::Action action);

	void init_fleet(EditorGameBase&);
	void set_fleet(Fleet* fleet);

	void send_message(Game& game,
	                  const std::string& title,
	                  const std::string& heading,
	                  const std::string& description,
	                  const std::string& picture);

	Fleet* fleet_;
	Economy* economy_;
	OPtr<PortDock> lastdock_;
	OPtr<PortDock> destination_;
	std::vector<ShippingItem> items_;
	ShipStates ship_state_;
	std::string shipname_;

	struct Expedition {
		std::vector<Coords> seen_port_buildspaces;
		bool swimmable[LAST_DIRECTION];
		bool island_exploration;
		WalkingDir scouting_direction;
		Coords exploration_start;
		IslandExploreDirection island_explore_direction;
		std::unique_ptr<Economy> economy;
	};
	std::unique_ptr<Expedition> expedition_;

	// saving and loading
protected:
	struct Loader : Bob::Loader {
		Loader();

		const Task* get_task(const std::string& name) override;

		void load(FileRead& fr);
		void load_pointers() override;
		void load_finish() override;

	private:
		uint32_t lastdock_;
		uint32_t destination_;
		ShipStates ship_state_;
		std::string shipname_;
		std::unique_ptr<Expedition> expedition_;
		std::vector<ShippingItem::Loader> items_;
	};

public:
	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;

	static MapObject::Loader* load(EditorGameBase&, MapObjectLoader&, FileRead&);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_SHIP_H
