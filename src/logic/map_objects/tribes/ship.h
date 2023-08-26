/*
 * Copyright (C) 2010-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_SHIP_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_SHIP_H

#include <deque>
#include <memory>

#include "base/macros.h"
#include "economy/shippingitem.h"
#include "graphic/animation/diranimations.h"
#include "logic/map_objects/bob.h"
#include "logic/map_objects/tribes/shipstates.h"

namespace Widelands {

class PinnedNote;
struct ShipFleet;

// This can't be part of the Ship class because of forward declaration in game.h
// Keep the order of entries for savegame compatibility.
enum class IslandExploreDirection {
	kNotSet,
	kCounterClockwise,
	kClockwise,
};

struct NoteShip {
	CAN_BE_SENT_AS_NOTE(NoteId::Ship)

	Ship* ship;

	enum class Action { kDestinationChanged, kWaitingForCommand, kNoPortLeft, kLost, kGained };
	Action action;

	NoteShip(Ship* const init_ship, const Action& init_action)
	   : ship(init_ship), action(init_action) {
	}
};

class ShipDescr : public BobDescr {
public:
	ShipDescr(const std::string& init_descname, const LuaTable& t);
	~ShipDescr() override = default;

	[[nodiscard]] Bob& create_object() const override;

	[[nodiscard]] uint32_t movecaps() const override;
	[[nodiscard]] const DirAnimations& get_sail_anims() const {
		return sail_anims_;
	}

	[[nodiscard]] Quantity get_default_capacity() const {
		return default_capacity_;
	}

	[[nodiscard]] const std::vector<std::string>& get_ship_names() const {
		return ship_names_;
	}

	const uint32_t max_hitpoints_;
	const uint32_t min_attack_;
	const uint32_t max_attack_;
	const uint32_t defense_;
	const uint32_t attack_accuracy_;
	const uint32_t heal_per_second_;

private:
	DirAnimations sail_anims_;
	Quantity default_capacity_;
	std::vector<std::string> ship_names_;

	DISALLOW_COPY_AND_ASSIGN(ShipDescr);
};

constexpr int32_t kShipInterval = 1500;

/**
 * Ships belong to a player and to an economy. The usually are in a (unique)
 * fleet for a player, but only if they are on standard duty. Exploration ships
 * are an economy of their own and are not part of a Fleet.
 */
struct Ship : Bob {
	MO_DESCR(ShipDescr)

	explicit Ship(const ShipDescr& descr);
	~Ship() override = default;

	// Returns the fleet the ship is a part of.
	[[nodiscard]] ShipFleet* get_fleet() const;

	/* A ship's destination can be a port, or another ship, or a pinned note (or nullptr). */
	void set_destination(EditorGameBase& egbase, MapObject* dest, bool is_playercommand = false);
	void set_destination(EditorGameBase& egbase,
	                     const DetectedPortSpace& dest,
	                     bool is_playercommand = false);
	[[nodiscard]] PortDock* get_destination_port(EditorGameBase& e) const;
	[[nodiscard]] Ship* get_destination_ship(EditorGameBase& e) const;
	[[nodiscard]] PinnedNote* get_destination_note(EditorGameBase& e) const;
	[[nodiscard]] const PortDock* get_destination_port(const EditorGameBase& e) const;
	[[nodiscard]] const Ship* get_destination_ship(const EditorGameBase& e) const;
	[[nodiscard]] const PinnedNote* get_destination_note(const EditorGameBase& e) const;
	[[nodiscard]] bool is_on_destination_dock() const;
	[[nodiscard]] bool has_destination() const;
	[[nodiscard]] const DetectedPortSpace* get_destination_detected_port_space() const {
		return destination_coords_;
	}

	[[nodiscard]] bool get_send_message_at_destination() const {
		return send_message_at_destination_;
	}

	// Returns the last visited portdock of this ship or nullptr if there is none or
	// the last visited was removed.
	[[nodiscard]] PortDock* get_lastdock(EditorGameBase& egbase) const;

	[[nodiscard]] Economy* get_economy(WareWorker type) const {
		return type == wwWARE ? ware_economy_ : worker_economy_;
	}
	void set_economy(const Game&, Economy* e, WareWorker);

	void init_auto_task(Game&) override;
	void set_position(EditorGameBase&, const Coords&) override;

	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;

	void start_task_ship(Game&);
	bool start_task_movetodock(Game&, PortDock&);
	void start_task_expedition(Game&);

	struct Battle {
		enum class Phase : uint8_t {
			kNotYetStarted = 0,
			kAttackerMovingTowardsOpponent = 1,
			kAttackersTurn = 2,
			kDefendersTurn = 3,
			kAttackerAttacking = 4,
			kDefenderAttacking = 5,
		};

		Battle(Ship* o, Coords ac, std::vector<uint32_t> a, bool f)
		   : opponent(o), attack_coords(ac), attack_soldier_serials(a), is_first(f) {
		}

		OPtr<Ship> opponent;
		Coords attack_coords;
		std::vector<uint32_t> attack_soldier_serials;
		Time time_of_last_action;
		uint32_t pending_damage{0U};
		Phase phase{Phase::kNotYetStarted};
		bool is_first;
	};
	void start_battle(Game&, Battle, bool immediately);

	uint32_t calculate_sea_route(EditorGameBase&, PortDock&, Path* = nullptr) const;

	void log_general_info(const EditorGameBase&) const override;

	[[nodiscard]] uint32_t get_nritems() const {
		return items_.size();
	}
	[[nodiscard]] const ShippingItem& get_item(uint32_t idx) const {
		return items_[idx];
	}

	void add_item(Game&, const ShippingItem&);
	bool withdraw_item(Game&, PortDock&);

	/// \returns the current state the ship is in
	[[nodiscard]] ShipStates get_ship_state() const {
		return ship_state_;
	}

	/// \returns the current name of ship
	[[nodiscard]] const std::string& get_shipname() const {
		return shipname_;
	}

	void set_shipname(const std::string& name);

	/// \returns whether the ship is currently on an expedition
	[[nodiscard]] bool state_is_expedition() const {
		return (ship_state_ == ShipStates::kExpeditionScouting ||
		        ship_state_ == ShipStates::kExpeditionWaiting ||
		        ship_state_ == ShipStates::kExpeditionPortspaceFound ||
		        ship_state_ == ShipStates::kExpeditionColonizing);
	}
	/// \returns whether the ship is in transport mode
	[[nodiscard]] bool state_is_transport() const {
		return (ship_state_ == ShipStates::kTransport);
	}
	/// \returns whether a sink request for the ship is currently valid
	[[nodiscard]] bool state_is_sinkable() const {
		return (ship_state_ != ShipStates::kSinkRequest &&
		        ship_state_ != ShipStates::kSinkAnimation &&
		        ship_state_ != ShipStates::kExpeditionColonizing);
	}
	/// \returns whether the ship is currently sinking
	[[nodiscard]] bool state_is_sinking() const {
		return (ship_state_ == ShipStates::kSinkRequest || ship_state_ == ShipStates::kSinkAnimation);
	}
	[[nodiscard]] bool is_expedition_or_warship() const {
		return expedition_ != nullptr;
	}

	/// \returns (in expedition mode only!) whether the next field in direction \arg dir is swimmable
	[[nodiscard]] bool exp_dir_swimmable(Direction dir) const {
		return expedition_ != nullptr && expedition_->swimmable[dir - 1];
	}

	// whether the ship's expedition is in state "island-exploration" (circular movement)
	[[nodiscard]] bool is_exploring_island() const {
		return expedition_->island_exploration;
	}

	/// \returns whether the expedition ship is close to the coast
	[[nodiscard]] bool exp_close_to_coast() const {
		if (expedition_ == nullptr) {
			return false;
		}
		for (uint8_t dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
			if (!expedition_->swimmable[dir - 1]) {
				return true;
			}
		}
		return false;
	}

	/// \returns (in expedition mode only!) the list of currently seen port build spaces
	[[nodiscard]] const std::vector<Coords>& exp_port_spaces() const {
		assert(expedition_ != nullptr);
		return expedition_->seen_port_buildspaces;
	}

	void exp_scouting_direction(Game&, WalkingDir);
	void exp_construct_port(Game&, const Coords&);
	void exp_explore_island(Game&, IslandExploreDirection);

	// Returns integer of direction, or WalkingDir::IDLE if query invalid
	// Intended for LUA scripting
	[[nodiscard]] WalkingDir get_scouting_direction() const;

	// Returns integer of direction, or IslandExploreDirection::kNotSet
	// if query invalid
	// Intended for LUA scripting
	[[nodiscard]] IslandExploreDirection get_island_explore_direction() const;

	void exp_cancel(Game&);
	void sink_ship(Game&);

	[[nodiscard]] Quantity get_capacity() const {
		return capacity_;
	}
	void set_capacity(Quantity c);

	[[nodiscard]] bool has_attack_target(Ship* s) const {
		return expedition_ != nullptr && expedition_->attack_targets.count(s) > 0;
	}
	[[nodiscard]] bool has_battle() const {
		return !battles_.empty();
	}
	[[nodiscard]] unsigned get_sea_attack_soldier_bonus(const EditorGameBase& egbase) const;

	[[nodiscard]] bool can_be_attacked() const;
	[[nodiscard]] bool can_attack() const;
	[[nodiscard]] bool is_attackable_enemy_warship(const Bob&) const;
	[[nodiscard]] uint32_t get_hitpoints() const {
		return hitpoints_;
	}
	[[nodiscard]] uint32_t get_warship_soldier_capacity() const {
		return warship_soldier_capacity_;
	}
	[[nodiscard]] uint32_t min_warship_soldier_capacity() const;

	[[nodiscard]] std::vector<Soldier*> onboard_soldiers() const;
	[[nodiscard]] std::vector<Soldier*> associated_soldiers() const;

	/**
	 * Execute a warship command.
	 * For a kAttack against a port, `parameters` contains first the port coordinates x,y and then
	 * the serials of the soldiers to send.
	 * For a kAttack against a ship, `parameters` contains the target ship's serial.
	 * For a kSetCapacity, `parameters` must contain exactly one value to specify the new capacity.
	 * For a kRetreat, `parameters` must be empty.
	 */
	void warship_command(Game& game, WarshipCommand cmd, const std::vector<uint32_t>& parameters);
	void drop_soldier(Game& game, Serial soldier);

	static void warship_soldier_callback(
	   Game& game, Request& req, DescriptionIndex di, Worker* worker, PlayerImmovable& immovable);

	[[nodiscard]] ShipType get_ship_type() const {
		return ship_type_;
	}
	[[nodiscard]] ShipType get_pending_refit() const {
		return pending_refit_;
	}
	[[nodiscard]] bool can_refit(ShipType) const;
	[[nodiscard]] inline bool is_refitting() const {
		return get_pending_refit() != get_ship_type();
	}
	void refit(Game&, ShipType);

	void set_soldier_preference(SoldierPreference pref);
	[[nodiscard]] SoldierPreference get_soldier_preference() const {
		return soldier_preference_;
	}

	// Editor only
	void set_ship_type(EditorGameBase& egbase, ShipType t);
	void set_warship_soldier_capacity(Quantity c);

	void draw_healthbar(const EditorGameBase& egbase,
	                    RenderTarget* dst,
	                    const Vector2f& point_on_dst,
	                    float scale) const;

protected:
	void draw(const EditorGameBase& egbase,
	          const InfoToDraw& info_to_draw,
	          const Vector2f& field_on_dst,
	          const Coords& coords,
	          float scale,
	          RenderTarget* dst) const override;

private:
	friend struct ShipFleet;
	friend struct ShippingSchedule;

	void recalc_expedition_swimmable(const EditorGameBase& egbase);
	void wakeup_neighbours(Game&);

	static const Task taskShip;

	void ship_update(Game&, State&);
	void ship_wakeup(Game&);

	bool ship_update_transport(Game&, State&);
	bool ship_update_expedition(Game&, State&);
	void ship_update_idle(Game&, State&);
	void battle_update(Game&);
	void update_warship_soldier_request(bool create);
	void kickout_superfluous_soldiers(Game& game);
	/// Set the ship's state to 'state' and if the ship state has changed, publish a notification.
	void set_ship_state_and_notify(ShipStates state, NoteShip::Action action);
	bool check_port_space_still_available(Game&);

	bool init_fleet(EditorGameBase&);
	void set_fleet(ShipFleet* fleet);

	PortDock* find_nearest_port(Game& game);

	// Checks and remembers port spaces within the ship's vision range.
	// If report_known is true, then sends message on all newly spotted port spaces, otherwise only
	// on newly discovered port spaces.
	// If stop_on_report is true, then also stops the ship when a port space is reported.
	// Returns whether the ship was stopped.
	bool update_seen_portspaces(Game& game, bool report_known = true, bool stop_on_report = true);

	// Stores coords as a DetectedPortSpace or updates owner if already known.
	// Returns true if the port space was previously not known.
	bool remember_detected_portspace(const Coords& coords);

	// Uses the applicable check of the below two functions according to ship type.
	[[nodiscard]] bool is_suitable_portspace(const Coords& coords) const;
	[[nodiscard]] bool can_build_port_here(const Coords& coords) const;
	[[nodiscard]] bool suited_as_invasion_portspace(const Coords& coords) const;

	void send_message(Game& game,
	                  const std::string& title,
	                  const std::string& heading,
	                  const std::string& description,
	                  const std::string& picture);
	void send_new_portspace_message(Game& g);
	void send_known_portspace_message(Game& g);

	ShipFleet* fleet_{nullptr};
	Economy* ware_economy_{nullptr};
	Economy* worker_economy_{nullptr};
	OPtr<PortDock> lastdock_;
	std::vector<ShippingItem> items_;
	ShipStates ship_state_{ShipStates::kTransport};
	ShipType ship_type_{ShipType::kTransport};
	ShipType pending_refit_{ShipType::kTransport};
	std::string shipname_;

	OPtr<MapObject> destination_object_{nullptr};
	const DetectedPortSpace* destination_coords_{nullptr};

	struct Expedition {
		~Expedition();

		std::vector<Coords> seen_port_buildspaces;
		bool swimmable[LAST_DIRECTION];
		bool island_exploration;
		WalkingDir scouting_direction;
		Coords exploration_start;
		IslandExploreDirection island_explore_direction;
		std::set<OPtr<Ship>> attack_targets;
		Economy* ware_economy;  // Owned by Player
		Economy* worker_economy;
	};
	std::unique_ptr<Expedition> expedition_;

	std::deque<Battle> battles_;
	uint32_t hitpoints_;
	Time last_heal_time_{0U};
	bool send_message_at_destination_{false};

	Quantity capacity_;
	Quantity warship_soldier_capacity_;
	SoldierPreference soldier_preference_{SoldierPreference::kHeroes};

	// saving and loading
protected:
	struct Loader : Bob::Loader {

		const Task* get_task(const std::string& name) override;

		void load(FileRead& fr, uint8_t);
		void load_pointers() override;
		void load_finish() override;

	private:
		uint32_t lastdock_{0U};
		Serial ware_economy_serial_{kInvalidSerial};
		Serial worker_economy_serial_{kInvalidSerial};
		uint32_t destination_object_{0U};
		uint32_t destination_coords_{0U};
		Quantity capacity_{0U};
		Quantity warship_soldier_capacity_{0U};
		SoldierPreference soldier_preference_{SoldierPreference::kHeroes};
		int32_t hitpoints_{0};
		Time last_heal_time_{0U};
		bool send_message_at_destination_{false};
		ShipStates ship_state_{ShipStates::kTransport};
		ShipType ship_type_{ShipType::kTransport};
		ShipType pending_refit_{ShipType::kTransport};
		std::string shipname_;
		std::unique_ptr<Expedition> expedition_;
		std::deque<Battle> battles_;
		std::vector<ShippingItem::Loader> items_;
		std::set<Serial> expedition_attack_target_serials_{0U};
		std::vector<Serial> battle_serials_;
	};

public:
	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;

	static MapObject::Loader* load(EditorGameBase&, MapObjectLoader&, FileRead&);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_SHIP_H
