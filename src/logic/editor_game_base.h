/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include <memory>

#include "base/macros.h"
#include "logic/addons.h"
#include "logic/map.h"
#include "logic/map_objects/bob.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/player_area.h"
#include "notifications/notifications.h"
#include "scripting/lua_interface.h"
#include "ui_basic/note_loading_message.h"
#include "wui/game_tips.h"

namespace UI {
struct ProgressWindow;
}
namespace FsMenu {
class LaunchGame;
}
class InteractiveBase;
class InteractiveGameBase;  // TODO(GunChleoc): Get rid

namespace Widelands {

class PlayersManager;
struct ObjectManager;
class Player;
struct BuildingSettings;

struct NoteFieldPossession {
	CAN_BE_SENT_AS_NOTE(NoteId::FieldPossession)

	// Has this been lost or gained?
	enum class Ownership { LOST, GAINED };
	Ownership ownership;

	// The field that has been lost/gained.
	FCoords fc;

	// The player that has lost or gained this field.
	Player* player;

	NoteFieldPossession(const FCoords& init_fc, Ownership const init_ownership, Player* init_player)
	   : ownership(init_ownership), fc(init_fc), player(init_player) {
	}
};

class EditorGameBase {
public:
	friend class InteractiveBase;
	friend class LaunchGame;
	friend struct GameClassPacket;

	explicit EditorGameBase(LuaInterface* lua);
	virtual ~EditorGameBase();

	const Map& map() const {
		return map_;
	}
	Map* mutable_map() {
		return &map_;
	}
	const ObjectManager& objects() const {
		return objects_;
	}
	ObjectManager& objects() {
		return objects_;
	}

	virtual bool is_game() const {
		return false;
	}

	// logic handler func
	virtual void think();

	// Player commands
	void remove_player(PlayerNumber);
	Player* add_player(PlayerNumber,
	                   uint8_t initialization_index,
	                   const RGBColor&,
	                   const std::string& tribe,
	                   const std::string& name,
	                   TeamNumber team = 0);
	Player* get_player(int32_t n) const;
	const Player& player(int32_t n) const;
	virtual Player* get_safe_player(PlayerNumber);

	// loading stuff
	void load_all_tribes();
	void allocate_player_maps();
	virtual void postload();
	void postload_addons();
	virtual void cleanup_for_load();
	virtual void full_cleanup();
	void delete_world_and_tribes();

	void init_addons(bool world_only);

	/// Create a new loader UI and register which type of gametips to select from.
	/// If 'show_game_tips' is true, game tips will be shown immediately.
	/// Optionally sets a background image.
	UI::ProgressWindow& create_loader_ui(const std::vector<std::string>& tipstexts,
	                                     bool show_game_tips,
	                                     const std::string& theme,
	                                     const std::string& background,
	                                     UI::Panel* parent = nullptr);

	/// Set step text for the current loader UI if it's not nullptr.
	void step_loader_ui(const std::string& text) const;

	/// Check whether we currently have a loader_ui.
	bool has_loader_ui() const {
		return loader_ui_ != nullptr;
	}

	// Destroy the loader UI and game tips
	void remove_loader_ui();
	UI::ProgressWindow* release_loader_ui();

	void set_road(const FCoords&, uint8_t direction, RoadSegment roadtype);

	// warping stuff. instantly creating map_objects
	Building& warp_building(const Coords&,
	                        PlayerNumber,
	                        DescriptionIndex,
	                        const FormerBuildings& former_buildings = FormerBuildings());
	Building& warp_constructionsite(const Coords&,
	                                PlayerNumber,
	                                DescriptionIndex,
	                                bool loading = false,
	                                const FormerBuildings& former_buildings = FormerBuildings(),
	                                const BuildingSettings* settings = nullptr,
	                                const std::map<DescriptionIndex, Quantity>& preserved_wares =
	                                   std::map<DescriptionIndex, Quantity>());
	Building& warp_dismantlesite(const Coords&,
	                             PlayerNumber,
	                             bool loading = false,
	                             const FormerBuildings& former_buildings = FormerBuildings(),
	                             const std::map<DescriptionIndex, Quantity>& preserved_wares =
	                                std::map<DescriptionIndex, Quantity>());
	Bob& create_critter(const Coords&, DescriptionIndex bob_type_idx, Player* owner = nullptr);
	Bob& create_critter(const Coords&, const std::string& name, Player* owner = nullptr);
	Immovable& create_immovable(const Coords&, DescriptionIndex idx, Player* owner);
	Immovable& create_immovable_with_name(const Coords&,
	                                      const std::string& name,
	                                      Player* owner,
	                                      const BuildingDescr* former_building);
	Bob& create_ship(const Coords&, const DescriptionIndex ship_type_idx, Player* owner = nullptr);
	Bob& create_ship(const Coords&, const std::string& name, Player* owner = nullptr);
	Bob& create_worker(const Coords&, DescriptionIndex worker, Player* owner);

	const Time& get_gametime() const {
		return gametime_;
	}
	// TODO(GunChleoc): Get rid.
	InteractiveBase* get_ibase() const {
		return ibase_.get();
	}

	void inform_players_about_ownership(MapIndex, PlayerNumber);
	void inform_players_about_immovable(MapIndex, MapObjectDescr const*);
	void inform_players_about_road(FCoords, MapObjectDescr const*);
	void inform_players_about_waterway(FCoords, MapObjectDescr const*);

	void unconquer_area(PlayerArea<Area<FCoords>>, PlayerNumber destroying_player = 0);
	void conquer_area(PlayerArea<Area<FCoords>>, bool conquer_guarded_location = false);
	void conquer_area_no_building(PlayerArea<Area<FCoords>> const);

	void cleanup_objects() {
		objects().cleanup(*this);
	}

	// next function is used to update the current gametime,
	// for queue runs e.g.
	Time& get_gametime_pointer() {
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

	// Returns the tribe and world descriptions.
	const Descriptions& descriptions() const;

	// Returns the mutable descriptions. Prefer descriptions() whenever possible.
	Descriptions* mutable_descriptions();

	void create_tempfile_and_save_mapdata(FileSystem::Type type);

	AddOns::AddOnsList& enabled_addons() {
		return enabled_addons_;
	}
	const AddOns::AddOnsList& enabled_addons() const {
		return enabled_addons_;
	}
	const AllTribes& all_tribes() const;

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
	                             bool conquer_guarded_location_by_superior_influence = false,
	                             bool neutral_when_no_influence = false,
	                             bool neutral_when_competing_influence = false);
	void cleanup_playerimmovables_area(PlayerArea<Area<FCoords>>);

	// Changes the owner of 'fc' from the current player to the new player and
	// sends notifications about this.
	void change_field_owner(const FCoords& fc, PlayerNumber new_owner);

	Immovable& do_create_immovable(const Coords& c,
	                               DescriptionIndex const idx,
	                               Player* owner,
	                               const BuildingDescr* former_building_descr);

	bool did_postload_addons_;
	Time gametime_;
	ObjectManager objects_;

	std::unique_ptr<LuaInterface> lua_;
	std::unique_ptr<PlayersManager> player_manager_;
	std::unique_ptr<Descriptions> descriptions_;
	std::unique_ptr<InteractiveBase> ibase_;
	Map map_;

	// Shown while loading or saving a game/map
	std::unique_ptr<UI::ProgressWindow> loader_ui_;
	std::unique_ptr<GameTips> game_tips_;
	std::vector<std::string> registered_game_tips_;
	std::unique_ptr<Notifications::Subscriber<UI::NoteLoadingMessage>> loading_message_subscriber_;

	/// Even after a map is fully loaded, some static data (images, scripts)
	/// will still be read from a filesystem whenever a map/game is saved.
	/// To avoid potential filesystem conflicts when (pre)loading/saving/deleting
	/// map/game files (and to avoid having to deal with this in many different places)
	/// a temporary file (in a special dir) is created for such data.
	std::unique_ptr<FileSystem> tmp_fs_;
	void delete_tempfile();

	AddOns::AddOnsList enabled_addons_;

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
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_EDITOR_GAME_BASE_H
