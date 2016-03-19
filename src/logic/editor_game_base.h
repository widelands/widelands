/*
 * Copyright (C) 2002-2004, 2006-2008, 2010-2011 by the Widelands Development Team
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

#ifndef WL_LOGIC_EDITOR_GAME_BASE_H
#define WL_LOGIC_EDITOR_GAME_BASE_H

#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "logic/map_objects/bob.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map.h"
#include "logic/player_area.h"
#include "notifications/notifications.h"
#include "scripting/lua_interface.h"

namespace UI {struct ProgressWindow;}
struct FullscreenMenuLaunchGame;
class InteractiveBase;

namespace Widelands {

class PlayersManager;

class Battle;
class Bob;
class BuildingDescr;
class Immovable;
class Map;
struct ObjectManager;
class Player;
struct PlayerImmovable;
class Tribes;
class TribeDescr;
struct Flag;
struct AttackController;

struct NoteFieldPossession {
	CAN_BE_SENT_AS_NOTE(NoteId::FieldPossession)

	// Has this been lost or gained?
	enum class Ownership {LOST, GAINED};
	Ownership ownership;

	// The field that has been lost/gained.
	FCoords fc;

	// The player that has lost or gained this field.
	Player * player;

	NoteFieldPossession(const FCoords& init_fc, Ownership const init_ownership, Player* init_player)
	   : ownership(init_ownership), fc(init_fc), player(init_player) {
	}
};

class EditorGameBase {
public:
	friend class InteractiveBase;
	friend struct FullscreenMenuLaunchGame;
	friend struct GameClassPacket;

	EditorGameBase(LuaInterface* lua);
	virtual ~EditorGameBase();

	void set_map(Map*);
	// TODO(sirver): this should just be const Map& map() and Map* mutable_map().
	Map& map() const {
		return *map_;
	}
	Map* get_map() {
		return map_;
	}
	Map& get_map() const {
		return *map_;
	}
	const ObjectManager& objects() const {
		return objects_;
	}
	ObjectManager& objects() {
		return objects_;
	}

	// logic handler func
	virtual void think();

	// Player commands
	void remove_player(PlayerNumber);
	Player* add_player(PlayerNumber,
	                   uint8_t initialization_index,
	                   const std::string& tribe,
	                   const std::string& name,
	                   TeamNumber team = 0);
	Player* get_player(int32_t n) const;
	Player& player(int32_t n) const;
	virtual Player* get_safe_player(PlayerNumber);

	// loading stuff
	void allocate_player_maps();
	virtual void postload();
	void load_graphics(UI::ProgressWindow& loader_ui);
	virtual void cleanup_for_load();

	void set_road(FCoords, uint8_t direction, uint8_t roadtype);

	// warping stuff. instantly creating map_objects
	Building&
	warp_building(Coords,
					  PlayerNumber,
					  DescriptionIndex,
	              Building::FormerBuildings former_buildings = Building::FormerBuildings());
	Building&
	warp_constructionsite(Coords,
								 PlayerNumber,
								 DescriptionIndex,
	                      bool loading = false,
	                      Building::FormerBuildings former_buildings = Building::FormerBuildings());
	Building&
	warp_dismantlesite(Coords,
							 PlayerNumber,
	                   bool loading = false,
	                   Building::FormerBuildings former_buildings = Building::FormerBuildings());
	Bob& create_critter(Coords, DescriptionIndex bob_type_idx, Player* owner = nullptr);
	Bob& create_critter(Coords, const std::string& name, Player* owner = nullptr);
	Immovable& create_immovable(Coords,
										 uint32_t idx,
										 MapObjectDescr::OwnerType = MapObjectDescr::OwnerType::kWorld);
	Immovable& create_immovable(Coords,
										 const std::string& name,
										 MapObjectDescr::OwnerType = MapObjectDescr::OwnerType::kWorld);
	Bob& create_ship(Coords, int ship_type_idx, Player* owner = nullptr);
	Bob& create_ship(Coords, const std::string& name, Player* owner = nullptr);

	uint32_t get_gametime() const {
		return gametime_;
	}
	InteractiveBase* get_ibase() const {
		return ibase_.get();
	}

	// safe system for storing pointers to non-MapObject C++ objects
	// unlike objects in the ObjectManager, these pointers need not be
	// synchronized across the network, and they are not saved in savegames
	uint32_t add_trackpointer(void*);
	void* get_trackpointer(uint32_t serial);
	void remove_trackpointer(uint32_t serial);

	void inform_players_about_ownership(MapIndex, PlayerNumber);
	void inform_players_about_immovable(MapIndex, MapObjectDescr const*);
	void inform_players_about_road(FCoords, MapObjectDescr const*);

	void unconquer_area(PlayerArea<Area<FCoords>>, PlayerNumber destroying_player = 0);
	void conquer_area(PlayerArea<Area<FCoords>>);
	void conquer_area_no_building(PlayerArea<Area<FCoords>> const);

	void cleanup_objects() {
		objects().cleanup(*this);
	}

	// next function is used to update the current gametime,
	// for queue runs e.g.
	uint32_t& get_gametime_pointer() {
		return gametime_;
	}
	void set_ibase(InteractiveBase* const b);

	/// Lua frontend, used to run Lua scripts
	virtual LuaInterface& lua() {
		return *lua_;
	}

	PlayersManager* player_manager() {
		return player_manager_.get();
	}

	InteractiveGameBase* get_igbase();

	// Returns the world.
	const World& world() const;

	// Returns the world that can be modified. Prefer world() whenever possible.
	World* mutable_world();

	// Returns the tribes.
	const Tribes& tribes() const;

	// Returns the mutable tribes. Prefer tribes() whenever possible.
	Tribes* mutable_tribes();

private:
	/// Common function for create_critter and create_ship.
	Bob& create_bob(Coords, const BobDescr&, Player* owner = nullptr);

	/// \param preferred_player
	///  When conquer is false, this can be used to prefer a player over other
	///  players, when lost land is reassigned. This can for example be used to
	///  reward the player who actually destroyed a MilitarySite by giving an
	///  unconquered location that he has influence over to him, instead of some
	///  other player who has higher influence over that location. If 0, land is
	///  simply assigned by influence.
	///
	/// \param neutral_when_no_influence
	///  If true and the player completely loses influence over a location, it
	///  becomes neutral unless some other player claims it by having positive
	///  influence.
	///
	/// \param neutral_when_competing_influence
	///  If true and the player completely loses influence over a location and
	///  several players have positive and equal influence, the location becomes
	///  becomes neutral unless some other player claims it by having higher
	///  influence.
	///
	/// \param conquer_guarded_location_by_superior_influence
	///  If true, the conquering player will (automatically, without actually
	///  attacking) conquer a location even if another player already owns and
	///  covers the location with a militarysite, if the conquering player's
	///  influence becomes greater than the owner's influence.
	virtual void do_conquer_area(PlayerArea<Area<FCoords>> player_area,
	                             bool conquer,
										  PlayerNumber preferred_player = 0,
	                             bool neutral_when_no_influence = false,
	                             bool neutral_when_competing_influence = false,
	                             bool conquer_guarded_location_by_superior_influence = false);
	void cleanup_playerimmovables_area(PlayerArea<Area<FCoords>>);

	// Changes the owner of 'fc' from the current player to the new player and
	// sends notifications about this.
	void change_field_owner(const FCoords& fc, PlayerNumber new_owner);

	uint32_t gametime_;
	ObjectManager objects_;

	std::unique_ptr<LuaInterface> lua_;
	std::unique_ptr<PlayersManager> player_manager_;

	std::unique_ptr<World> world_;
	std::unique_ptr<Tribes> tribes_;
	std::unique_ptr<InteractiveBase> ibase_;
	Map* map_;

	uint32_t lasttrackserial_;
	std::map<uint32_t, void*> trackpointers_;

	DISALLOW_COPY_AND_ASSIGN(EditorGameBase);
};

#define iterate_players_existing(p, nr_players, egbase, player)                                    \
	iterate_player_numbers(                                                                         \
	   p, nr_players) if (Widelands::Player* const player = (egbase).get_player(p))

#define iterate_players_existing_novar(p, nr_players, egbase)                                      \
	iterate_player_numbers(p, nr_players) if ((egbase).get_player(p))

#define iterate_players_existing_const(p, nr_players, egbase, player)                              \
	iterate_player_numbers(                                                                         \
	   p, nr_players) if (Widelands::Player const* const player = (egbase).get_player(p))
}

#endif  // end of include guard: WL_LOGIC_EDITOR_GAME_BASE_H
