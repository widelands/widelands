/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include "logic/editor_game_base.h"

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/scoped_timer.h"
#include "base/time_string.h"
#include "base/wexception.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "economy/waterway.h"
#include "graphic/color.h"
#include "graphic/road_segments.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/findimmovable.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/battle.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/mapregion.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "map_io/map_saver.h"
#include "scripting/logic.h"
#include "scripting/lua_table.h"
#include "sound/sound_handler.h"
#include "ui_basic/progresswindow.h"
#include "wui/interactive_base.h"
#include "wui/interactive_gamebase.h"

namespace Widelands {

/*
============
EditorGameBase::EditorGameBase()

initialization
============
*/
EditorGameBase::EditorGameBase(LuaInterface* lua_interface)
   : did_postload_addons_(false),
     gametime_(0),
     // TODO(SirVer): this is sooo ugly, I can't say
     lua_(lua_interface ? lua_interface : new LuaEditorInterface(this)),
     player_manager_(new PlayersManager(*this)),
     ibase_(nullptr),
     loader_ui_(nullptr),
     game_tips_(nullptr),
     loading_message_subscriber_(Notifications::subscribe<UI::NoteLoadingMessage>(
        [this](const UI::NoteLoadingMessage& note) { step_loader_ui(note.message); })),
     tmp_fs_(nullptr) {

	init_addons(false);

	// Ensure descriptions are registered
	descriptions();
}

EditorGameBase::~EditorGameBase() {
	delete_tempfile();
}

static inline bool addon_initially_enabled(AddOns::AddOnCategory c) {
	return c == AddOns::AddOnCategory::kTribes || c == AddOns::AddOnCategory::kWorld ||
	       c == AddOns::AddOnCategory::kScript;
}
void EditorGameBase::init_addons(bool world_only) {
	enabled_addons_.clear();
	for (const auto& pair : AddOns::g_addons) {
		if (pair.second && (world_only ? pair.first->category == AddOns::AddOnCategory::kWorld :
                                       addon_initially_enabled(pair.first->category))) {
			enabled_addons_.push_back(pair.first);
		}
	}
}

/**
 * deletes the temporary file/dir
 * also resets the map filesystem if it points to the temporary file
 */
void EditorGameBase::delete_tempfile() {
	if (!tmp_fs_) {
		return;
	}

	std::string fs_filename = tmp_fs_->get_basename();
	std::string mapfs_filename = map_.filesystem()->get_basename();
	if (mapfs_filename == fs_filename) {
		map_.reset_filesystem();
	}
	tmp_fs_.reset();
	try {
		g_fs->fs_unlink(fs_filename);
	} catch (const std::exception& e) {
		// if file deletion fails then we have an abandoned file lying around, but otherwise that's
		// unproblematic
		log_warn_time(get_gametime(),
		              "EditorGameBase::delete_tempfile: deleting temporary file/dir failed: %s\n",
		              e.what());
	}
}

/**
 * creates a new file/dir, saves the map data, and reassigns the map filesystem
 * does not delete the former temp file if one exists
 * throws an exception if something goes wrong
 */
void EditorGameBase::create_tempfile_and_save_mapdata(FileSystem::Type const type) {
	if (!map_.filesystem()) {
		return;
	}

	// save map data to temporary file and reassign map fs
	try {
		g_fs->ensure_directory_exists(kTempFileDir);

		std::string filename =
		   kTempFileDir + FileSystem::file_separator() + timestring() + "_mapdata";
		std::string complete_filename = filename + kTempFileExtension;

		// if a file with that name already exists, then try a few name modifications
		if (g_fs->file_exists(complete_filename)) {
			int suffix;
			for (suffix = 0; suffix <= 9; suffix++) {
				complete_filename =
				   filename.append("-").append(std::to_string(suffix)).append(kTempFileExtension);
				if (!g_fs->file_exists(complete_filename)) {
					break;
				}
			}
			if (suffix > 9) {
				throw wexception(
				   "EditorGameBase::create_tempfile_and_save_mapdata(): for all considered "
				   "filenames a file already existed");
			}
		}

		// create tmp_fs_
		tmp_fs_.reset(g_fs->create_sub_file_system(complete_filename, type));

		// save necessary map data (we actually save the whole map)
		std::unique_ptr<Widelands::MapSaver> wms(new Widelands::MapSaver(*tmp_fs_, *this));
		wms->save();

		// swap map fs
		std::unique_ptr<FileSystem> mapfs(tmp_fs_->make_sub_file_system("."));
		map_.swap_filesystem(mapfs);
		mapfs.reset();

		// This is just a convenience hack:
		// If tmp_fs_ is a zip filesystem then - because of the way zip filesystems are currently
		// implemented -
		// the file is still in zip mode right now, which means that the file isn't finalized yet,
		// i.e.,
		// not even a valid zip file until zip mode ends. To force ending the zip mode (thus
		// finalizing
		// the file)
		// we simply perform a (otherwise useless) filesystem request.
		// It's not strictly necessary, but this way we get a valid zip file immediately istead of
		// at some unkown later point (when an unzip operation happens or a filesystem object
		// destructs).
		tmp_fs_->file_exists("binary");
	} catch (const WException& e) {
		log_err_time(
		   get_gametime(), "EditorGameBase: saving map to temporary file failed: %s", e.what());
		throw;
	}
}

void EditorGameBase::think() {
	// TODO(unknown): Get rid of this; replace by a function that just advances gametime
	// by a given number of milliseconds
}

void EditorGameBase::delete_world_and_tribes() {
	descriptions_.reset(nullptr);
}

const Descriptions& EditorGameBase::descriptions() const {
	// Const casts are evil, but this is essentially lazy evaluation and the
	// caller should really not modify this.
	return *const_cast<EditorGameBase*>(this)->mutable_descriptions();
}

Descriptions* EditorGameBase::mutable_descriptions() {
	if (!descriptions_) {
		// Lazy initialization of Descriptions. We need to create the pointer to the
		// descriptions immediately though, because the lua scripts need to have access
		// to descriptions through this method already.
		ScopedTimer timer("Registering the descriptions took %ums", true);
		assert(lua_);
		descriptions_.reset(new Descriptions(lua_.get(), enabled_addons_));
		if (game_tips_) {
			game_tips_.reset(new GameTips(*loader_ui_, registered_game_tips_, all_tribes()));
		}
	}
	return descriptions_.get();
}

void EditorGameBase::set_ibase(InteractiveBase* const b) {
	ibase_.reset(b);
}

InteractiveGameBase* EditorGameBase::get_igbase() {
	return dynamic_cast<InteractiveGameBase*>(get_ibase());
}

/// @see PlayerManager class
void EditorGameBase::remove_player(PlayerNumber plnum) {
	player_manager_->remove_player(plnum);
}

/// @see PlayerManager class
Player* EditorGameBase::add_player(PlayerNumber const player_number,
                                   uint8_t const initialization_index,
                                   const RGBColor& pc,
                                   const std::string& tribe,
                                   const std::string& name,
                                   TeamNumber team) {
	Notifications::publish(UI::NoteLoadingMessage(
	   bformat(_("Creating player %d…"), static_cast<unsigned int>(player_number))));
	return player_manager_->add_player(player_number, initialization_index, pc, tribe, name, team);
}

Player* EditorGameBase::get_player(const int32_t n) const {
	return player_manager_->get_player(n);
}

const Player& EditorGameBase::player(const int32_t n) const {
	return player_manager_->player(n);
}

void EditorGameBase::inform_players_about_ownership(MapIndex const i,
                                                    PlayerNumber const new_owner) {
	iterate_players_existing_const(plnum, kMaxPlayers, *this, p) {
		Player::Field& player_field = p->fields_[i];
		if (VisibleState::kVisible == player_field.vision) {
			player_field.owner = new_owner;
		}
	}
}
void EditorGameBase::inform_players_about_immovable(MapIndex const i,
                                                    MapObjectDescr const* const descr) {
	if (!Road::is_road_descr(descr) && !Waterway::is_waterway_descr(descr)) {
		iterate_players_existing_const(plnum, kMaxPlayers, *this, p) {
			Player::Field& player_field = p->fields_[i];
			if (VisibleState::kVisible == player_field.vision) {
				player_field.map_object_descr = descr;
			}
		}
	}
}

const AllTribes& EditorGameBase::all_tribes() const {
	return descriptions().all_tribes();
}

// Loads map object descriptions for all tribes
void EditorGameBase::load_all_tribes() {
	// Load all tribes
	for (const auto& tribe_info : all_tribes()) {
		mutable_descriptions()->load_tribe(tribe_info.name);
	}
}

void EditorGameBase::allocate_player_maps() {
	iterate_players_existing(plnum, kMaxPlayers, *this, p) {
		p->allocate_map();
	}
}

/**
 * Load and prepare detailed game and map data.
 * This happens once just after the host has started the game / the editor has started and before
 * the graphics are loaded.
 */
void EditorGameBase::postload() {
	create_tempfile_and_save_mapdata(FileSystem::ZIP);
	assert(descriptions_);
}

void EditorGameBase::postload_addons() {
	if (did_postload_addons_) {
		return;
	}
	did_postload_addons_ = true;

	Notifications::publish(UI::NoteLoadingMessage(_("Postloading world and tribes…")));

	assert(lua_);
	assert(descriptions_);

	for (const auto& info : enabled_addons_) {
		if (info->category == AddOns::AddOnCategory::kWorld ||
		    info->category == AddOns::AddOnCategory::kTribes) {
			const std::string script(kAddOnDir + FileSystem::file_separator() + info->internal_name +
			                         FileSystem::file_separator() + "postload.lua");
			if (g_fs->file_exists(script)) {
				verb_log_info("Running postload script for add-on %s", info->internal_name.c_str());
				lua_->run_script(script);
			}
		}
	}

	// Postload all tribes. We can do this only now to ensure that any changes
	// made by add-ons are taken into account when computing dependency chains.
	for (DescriptionIndex i = 0; i < descriptions_->nr_tribes(); ++i) {
		descriptions_->get_mutable_tribe_descr(i)->finalize_loading(*descriptions_);
	}
}

UI::ProgressWindow& EditorGameBase::create_loader_ui(const std::vector<std::string>& tipstexts,
                                                     bool show_game_tips,
                                                     const std::string& theme,
                                                     const std::string& background,
                                                     UI::Panel* parent) {
	assert(!has_loader_ui());
	loader_ui_.reset(new UI::ProgressWindow(parent, theme, background));
	registered_game_tips_ = tipstexts;
	if (show_game_tips) {
		game_tips_.reset(registered_game_tips_.empty() ?
                          nullptr :
                          new GameTips(*loader_ui_, registered_game_tips_, all_tribes()));
	}
	return *loader_ui_;
}
void EditorGameBase::step_loader_ui(const std::string& text) const {
	if (loader_ui_ != nullptr) {
		loader_ui_->step(text);
	}
}
void EditorGameBase::remove_loader_ui() {
	assert(loader_ui_ != nullptr);
	loader_ui_.reset(nullptr);
	game_tips_.reset(nullptr);
	registered_game_tips_.clear();
}

UI::ProgressWindow* EditorGameBase::release_loader_ui() {
	assert(loader_ui_ != nullptr);
	game_tips_.reset(nullptr);
	registered_game_tips_.clear();
	return loader_ui_.release();
}

/**
 * Instantly create a building at the given x/y location. There is no build time.
 * \li owner  is the player number of the building's owner.
 * \li idx is the building type index.
 * \li former_buildings is the list of former buildings
 */
Building& EditorGameBase::warp_building(const Coords& c,
                                        PlayerNumber const owner,
                                        DescriptionIndex const idx,
                                        const FormerBuildings& former_buildings) {
	Player* plr = get_player(owner);
	const TribeDescr& tribe = plr->tribe();
	return tribe.get_building_descr(idx)->create(*this, plr, c, false, true, former_buildings);
}

/**
 * Create a building site at the given x/y location for the given building type.
 *
 * \li idx : the building index of the building in construction
 * \li former_buildings : the former buildings. If it is not empty, this is
 * an enhancement.
 */
Building&
EditorGameBase::warp_constructionsite(const Coords& c,
                                      PlayerNumber const owner,
                                      DescriptionIndex idx,
                                      bool loading,
                                      const FormerBuildings& former_buildings,
                                      const BuildingSettings* settings,
                                      const std::map<DescriptionIndex, Quantity>& preserved_wares) {
	Player* plr = get_player(owner);
	const TribeDescr& tribe = plr->tribe();
	ConstructionSite& b = dynamic_cast<ConstructionSite&>(
	   tribe.get_building_descr(idx)->create(*this, plr, c, true, loading, former_buildings));
	if (settings) {
		b.apply_settings(*settings);
	}
	b.add_dropout_wares(preserved_wares);
	return b;
}

/**
 * Create a dismantle site
 * \li former_buildings : the former buildings list. This should not be empty,
 * except during loading.
 */
Building&
EditorGameBase::warp_dismantlesite(const Coords& c,
                                   PlayerNumber const owner,
                                   bool loading,
                                   const FormerBuildings& former_buildings,
                                   const std::map<DescriptionIndex, Quantity>& preserved_wares) {
	Player* plr = get_player(owner);
	const TribeDescr& tribe = plr->tribe();

	BuildingDescr const* const descr =
	   tribe.get_building_descr(tribe.safe_building_index("dismantlesite"));

	upcast(const DismantleSiteDescr, ds_descr, descr);

	return *new DismantleSite(*ds_descr, *this, c, plr, loading, former_buildings, preserved_wares);
}

/**
 * Instantly create a bob at the given x/y location.
 */
Bob& EditorGameBase::create_bob(Coords c, const BobDescr& descr, Player* owner) {
	return descr.create(*this, owner, c);
}

/**
 * Instantly create a critter at the given x/y location.
 *
 */

Bob& EditorGameBase::create_critter(const Coords& c,
                                    DescriptionIndex const bob_type_idx,
                                    Player* owner) {
	const BobDescr* descr =
	   dynamic_cast<const BobDescr*>(descriptions().get_critter_descr(bob_type_idx));
	return create_bob(c, *descr, owner);
}

Bob& EditorGameBase::create_critter(const Coords& c, const std::string& name, Player* owner) {
	const BobDescr* descr = dynamic_cast<const BobDescr*>(descriptions().get_critter_descr(name));
	if (descr == nullptr) {
		throw GameDataError("create_critter(%i,%i,%s,%s): critter not found", c.x, c.y, name.c_str(),
		                    owner->get_name().c_str());
	}
	return create_bob(c, *descr, owner);
}

/*
===============
Create an immovable at the given location.
If tribe is not zero, create a immovable of a player (not a PlayerImmovable
but an immovable defined by the players tribe)
Does not perform any placeability checks.
If this immovable was created by a building, 'former_building' can be set in order to display
information about it.
===============
*/
Immovable&
EditorGameBase::create_immovable(const Coords& c, DescriptionIndex const idx, Player* owner) {
	return do_create_immovable(c, idx, owner, nullptr);
}

Immovable& EditorGameBase::create_immovable_with_name(const Coords& c,
                                                      const std::string& name,
                                                      Player* owner,
                                                      const BuildingDescr* former_building_descr) {
	const DescriptionIndex idx = descriptions().immovable_index(name);
	if (!descriptions().immovable_exists(idx)) {
		throw wexception("EditorGameBase::create_immovable_with_name(%i, %i): %s is not defined", c.x,
		                 c.y, name.c_str());
	}
	return do_create_immovable(c, idx, owner, former_building_descr);
}

Immovable& EditorGameBase::do_create_immovable(const Coords& c,
                                               DescriptionIndex const idx,
                                               Player* owner,
                                               const BuildingDescr* former_building_descr) {
	const ImmovableDescr* descr = descriptions().get_immovable_descr(idx);
	inform_players_about_immovable(Map::get_index(c, map().get_width()), descr);
	Immovable& immovable = descr->create(*this, c, former_building_descr);
	if (owner != nullptr) {
		immovable.set_owner(owner);
	}
	return immovable;
}

/**
 * Instantly create a ship at the given x/y location.
 *
 * idx is the bob type.
 */

Bob& EditorGameBase::create_ship(const Coords& c,
                                 DescriptionIndex const ship_type_idx,
                                 Player* owner) {
	const BobDescr* descr =
	   dynamic_cast<const BobDescr*>(descriptions().get_ship_descr(ship_type_idx));
	return create_bob(c, *descr, owner);
}

Bob& EditorGameBase::create_ship(const Coords& c, const std::string& name, Player* owner) {
	try {
		return create_ship(c, descriptions().safe_ship_index(name), owner);
	} catch (const GameDataError& e) {
		throw GameDataError("create_ship(%i,%i,%s,%s): ship not found: %s", c.x, c.y, name.c_str(),
		                    owner->get_name().c_str(), e.what());
	}
}

Bob& EditorGameBase::create_worker(const Coords& c, DescriptionIndex worker, Player* owner) {
	if (!owner->tribe().has_worker(worker)) {
		throw GameDataError(
		   "Tribe %s does not have worker with index %d", owner->tribe().name().c_str(), worker);
	}
	const BobDescr* descr = dynamic_cast<const BobDescr*>(descriptions().get_worker_descr(worker));
	return create_bob(c, *descr, owner);
}

/*
================
Returns the correct player, creates it
with the scenario data when he is not yet created
This should only happen in the editor.
In the game, this is the same as get_player(). If it returns
zero it means that this player is disabled in the game.
================
*/
Player* EditorGameBase::get_safe_player(PlayerNumber const n) {
	return get_player(n);
}

/**
 * Cleanup for load
 *
 * make this object ready to load new data
 */
void EditorGameBase::cleanup_for_load() {
	Notifications::publish(UI::NoteLoadingMessage(_("Cleaning up for loading: Map objects (1/3)")));
	if (InteractiveBase* i = get_ibase()) {
		i->cleanup_for_load();
	}
	cleanup_objects();  /// Clean all the stuff up, so we can load.

	Notifications::publish(UI::NoteLoadingMessage(_("Cleaning up for loading: Players (2/3)")));
	player_manager_->cleanup();

	Notifications::publish(UI::NoteLoadingMessage(_("Cleaning up for loading: Map (3/3)")));
	map_.cleanup();

	delete_tempfile();
}

/** Cleanup *everything* so we can load a completely new savegame. */
void EditorGameBase::full_cleanup() {
	cleanup_for_load();
	enabled_addons().clear();
	did_postload_addons_ = false;
	descriptions_.reset(nullptr);
	gametime_ = Time(0);
	// See the comment about `lua_` in the ctor
	if (is_game()) {
		lua_.reset(new LuaGameInterface(dynamic_cast<Game*>(this)));
	} else {
		lua_.reset(new LuaEditorInterface(this));
	}
}

void EditorGameBase::set_road(const FCoords& f,
                              uint8_t const direction,
                              RoadSegment const roadtype) {
	const Map& m = map();
	const Field& first_field = m[0];
	assert(0 <= f.x);
	assert(f.x < m.get_width());
	assert(0 <= f.y);
	assert(f.y < m.get_height());
	assert(&first_field <= f.field);
	assert(f.field < &first_field + m.max_index());
	assert(direction == WALK_SW || direction == WALK_SE || direction == WALK_E);

	if (f.field->get_road(direction) == roadtype) {
		return;
	}
	f.field->set_road(direction, roadtype);

	FCoords neighbour;
	switch (direction) {
	case WALK_SW:
		neighbour = m.bl_n(f);
		break;
	case WALK_SE:
		neighbour = m.br_n(f);
		break;
	case WALK_E:
		neighbour = m.r_n(f);
		break;
	default:
		NEVER_HERE();
	}
	MapIndex const i = f.field - &first_field;
	MapIndex const neighbour_i = neighbour.field - &first_field;
	iterate_players_existing_const(plnum, kMaxPlayers, *this, p) {
		Player::Field& player_field = p->fields_[i];
		if (VisibleState::kVisible == player_field.vision ||
		    VisibleState::kVisible == p->fields_[neighbour_i].vision) {
			switch (direction) {
			case WALK_SE:
				player_field.r_se = roadtype;
				break;
			case WALK_SW:
				player_field.r_sw = roadtype;
				break;
			case WALK_E:
				player_field.r_e = roadtype;
				break;
			default:
				NEVER_HERE();
			}
		}
	}
}

/// This unconquers an area. This is only possible, when there is a building
/// placed on this node.
void EditorGameBase::unconquer_area(PlayerArea<Area<FCoords>> player_area,
                                    PlayerNumber const destroying_player) {
	assert(0 <= player_area.x);
	assert(player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert(player_area.y < map().get_height());
	assert(&map()[0] <= player_area.field);
	assert(player_area.field < &map()[map().max_index()]);
	assert(0 < player_area.player_number);
	assert(player_area.player_number <= map().get_nrplayers());

	//  Here must be a building.
	assert(
	   dynamic_cast<const Building&>(*map().get_immovable(player_area)).owner().player_number() ==
	   player_area.player_number);

	//  step 1: unconquer area of this building
	do_conquer_area(player_area, false, destroying_player);
}

/// This conquers a given area because of a new (military) building that is set
/// there.
void EditorGameBase::conquer_area(PlayerArea<Area<FCoords>> player_area,
                                  bool conquer_guarded_location) {
	assert(0 <= player_area.x);
	assert(player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert(player_area.y < map().get_height());
	assert(&map()[0] <= player_area.field);
	assert(player_area.field < &map()[map().max_index()]);
	assert(0 < player_area.player_number);
	assert(player_area.player_number <= map().get_nrplayers());

	do_conquer_area(player_area, true, 0, conquer_guarded_location);
}

void EditorGameBase::change_field_owner(const FCoords& fc, PlayerNumber const new_owner) {
	const Field& first_field = map()[0];

	PlayerNumber const old_owner = fc.field->get_owned_by();
	if (old_owner == new_owner) {
		return;
	}

	if (old_owner) {
		Notifications::publish(
		   NoteFieldPossession(fc, NoteFieldPossession::Ownership::LOST, get_player(old_owner)));
	}

	fc.field->set_owned_by(new_owner);

	// TODO(unknown): the player should do this when it gets the NoteFieldPossession.
	// This means also sending a note when new_player = 0, i.e. the field is no
	// longer owned.
	inform_players_about_ownership(fc.field - &first_field, new_owner);

	if (new_owner) {
		Notifications::publish(
		   NoteFieldPossession(fc, NoteFieldPossession::Ownership::GAINED, get_player(new_owner)));
	}
}

void EditorGameBase::conquer_area_no_building(PlayerArea<Area<FCoords>> player_area) {
	assert(0 <= player_area.x);
	assert(player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert(player_area.y < map().get_height());
	assert(&map()[0] <= player_area.field);
	assert(player_area.field < &map()[0] + map().max_index());
	assert(0 < player_area.player_number);
	assert(player_area.player_number <= map().get_nrplayers());
	MapRegion<Area<FCoords>> mr(map(), player_area);
	do {
		change_field_owner(mr.location(), player_area.player_number);
	} while (mr.advance(map()));

	//  This must reach two steps beyond the conquered area to adjust the borders
	//  of neighbour players.
	player_area.radius += 2;
	map_.recalc_for_field_area(*this, player_area);
}

/// Conquers the given area for that player; does the actual work.
/// Additionally, it updates the visible area for that player.
// TODO(unknown): this needs a more fine grained refactoring
// for example scripts will want to (un)conquer area of non oval shape
// or give area back to the neutral player (this is very important for the Lua
// testsuite).
void EditorGameBase::do_conquer_area(PlayerArea<Area<FCoords>> player_area,
                                     bool const conquer,
                                     PlayerNumber const preferred_player,
                                     bool const conquer_guarded_location_by_superior_influence,
                                     bool const neutral_when_no_influence,
                                     bool const neutral_when_competing_influence) {
	assert(0 <= player_area.x);
	assert(player_area.x < map().get_width());
	assert(0 <= player_area.y);
	assert(player_area.y < map().get_height());
	const Field& first_field = map()[0];
	assert(&first_field <= player_area.field);
	assert(player_area.field < &first_field + map().max_index());
	assert(0 < player_area.player_number);
	assert(player_area.player_number <= map().get_nrplayers());
	assert(preferred_player <= map().get_nrplayers());
	assert(!conquer || !preferred_player);
	Player* conquering_player = get_player(player_area.player_number);
	MapRegion<Area<FCoords>> mr(map(), player_area);
	do {
		MapIndex const index = mr.location().field - &first_field;
		MilitaryInfluence const influence =
		   map().calc_influence(mr.location(), Area<>(player_area, player_area.radius));

		PlayerNumber const owner = mr.location().field->get_owned_by();
		if (conquer) {
			//  adds the influence
			MilitaryInfluence new_influence_modified = conquering_player->military_influence(index) +=
			   influence;
			if (owner && !conquer_guarded_location_by_superior_influence) {
				new_influence_modified = 1;
			}
			if (!owner || player(owner).military_influence(index) < new_influence_modified) {
				change_field_owner(mr.location(), player_area.player_number);
			}
		} else if (!(conquering_player->military_influence(index) -= influence) &&
		           owner == player_area.player_number) {
			//  The player completely lost influence over the location, which he
			//  owned. Now we must see if some other player has influence and if
			//  so, transfer the ownership to that player.
			PlayerNumber best_player;
			if (preferred_player && player(preferred_player).military_influence(index)) {
				best_player = preferred_player;
			} else {
				best_player = neutral_when_no_influence ? 0 : player_area.player_number;
				MilitaryInfluence highest_military_influence = 0;
				PlayerNumber const nr_players = map().get_nrplayers();
				iterate_players_existing_const(p, nr_players, *this, plr) {
					if (MilitaryInfluence const value = plr->military_influence(index)) {
						if (value > highest_military_influence) {
							highest_military_influence = value;
							best_player = p;
						} else if (value == highest_military_influence) {
							best_player = neutral_when_competing_influence ? 0 : player_area.player_number;
						}
					}
				}
			}
			if (best_player != player_area.player_number) {
				change_field_owner(mr.location(), best_player);
			}
		}
	} while (mr.advance(map()));

	// This must reach two steps beyond the conquered area to adjust the borders
	// of neighbour players.
	player_area.radius += 2;
	map_.recalc_for_field_area(*this, player_area);

	//  Deal with player immovables in the lost area
	//  Players are not allowed to have their immovables on their borders.
	//  Therefore the area must be enlarged before calling
	//  cleanup_playerimmovables_area, so that those new border locations are
	//  covered.
	// TODO(SirVer): In the editor, no buildings should burn down when a military
	// building is removed. Check this again though
	if (is_game()) {
		cleanup_playerimmovables_area(player_area);
	}
}

/// Makes sure that buildings cannot exist outside their owner's territory.
void EditorGameBase::cleanup_playerimmovables_area(PlayerArea<Area<FCoords>> const area) {
	std::vector<ImmovableFound> immovables;
	std::vector<PlayerImmovable*> burnlist;

	//  find all immovables that need fixing
	map_.find_immovables(*this, area, &immovables, FindImmovablePlayerImmovable());

	for (const ImmovableFound& temp_imm : immovables) {
		upcast(PlayerImmovable, imm, temp_imm.object);
		if (!map_[temp_imm.coords].is_interior(imm->owner().player_number())) {
			if (std::find(burnlist.begin(), burnlist.end(), imm) == burnlist.end()) {
				burnlist.push_back(imm);
			}
		}
	}

	//  fix all immovables
	upcast(Game, game, this);
	for (PlayerImmovable* temp_imm : burnlist) {
		if (upcast(Building, building, temp_imm)) {
			building->set_defeating_player(area.player_number);
		} else if (upcast(Flag, flag, temp_imm)) {
			if (Building* const flag_building = flag->get_building()) {
				flag_building->set_defeating_player(area.player_number);
			}
		}
		if (game) {
			temp_imm->schedule_destroy(*game);
		} else {
			temp_imm->remove(*this);
		}
	}
}
}  // namespace Widelands
