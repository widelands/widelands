/*
 * Copyright (C) 2002-2004, 2006-2012, 2015 by the Widelands Development Team
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

#include "logic/game.h"

#include <cstring>
#include <limits>
#include <memory>
#include <string>

#include <boost/format.hpp>
#ifndef _WIN32
#include <SDL.h> // for a dirty hack.
#include <unistd.h> // for usleep
#else
#include <windows.h>
#endif

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/time_string.h"
#include "base/warning.h"
#include "economy/economy.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_packet.h"
#include "graphic/graphic.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "logic/cmd_calculate_statistics.h"
#include "logic/cmd_luacoroutine.h"
#include "logic/cmd_luascript.h"
#include "logic/game_settings.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/replay.h"
#include "logic/single_player_game_controller.h"
#include "map_io/widelands_map_loader.h"
#include "network/network.h"
#include "scripting/logic.h"
#include "scripting/lua_table.h"
#include "sound/sound_handler.h"
#include "ui_basic/progresswindow.h"
#include "wlapplication.h"
#include "wui/game_tips.h"
#include "wui/interactive_player.h"

namespace Widelands {

/// Define this to get lots of debugging output concerned with syncs
// #define SYNC_DEBUG

Game::SyncWrapper::~SyncWrapper() {
	if (dump_ != nullptr) {
		if (!syncstreamsave_)
			g_fs->fs_unlink(dumpfname_);
	}
}

void Game::SyncWrapper::start_dump(const std::string & fname) {
	dumpfname_ = fname + ".wss";
	dump_.reset(g_fs->open_stream_write(dumpfname_));
}

static const unsigned long long MINIMUM_DISK_SPACE = 256 * 1024 * 1024;

void Game::SyncWrapper::data(void const * const sync_data, size_t const size) {
#ifdef SYNC_DEBUG
	uint32_t time = game_.get_gametime();
	log("[sync:%08u t=%6u]", counter_, time);
	for (size_t i = 0; i < size; ++i)
		log(" %02x", (static_cast<uint8_t const *>(sync_data))[i]);
	log("\n");
#endif

	if (dump_ != nullptr && static_cast<int32_t>(counter_ - next_diskspacecheck_) >= 0) {
		next_diskspacecheck_ = counter_ + 16 * 1024 * 1024;

		if (g_fs->disk_space() < MINIMUM_DISK_SPACE) {
			log("Stop writing to syncstream file: disk is getting full.\n");
			dump_.reset();
		}
	}

	if (dump_ != nullptr) {
		try {
			dump_->data(sync_data, size);
		} catch (const WException &) {
			log("Writing to syncstream file %s failed. Stop synctream dump.\n", dumpfname_.c_str());
			dump_.reset();
		}
	}

	target_.data(sync_data, size);
	counter_ += size;
}


Game::Game() :
	EditorGameBase(new LuaGameInterface(this)),
	syncwrapper_         (*this, synchash_),
	ctrl_                (nullptr),
	writereplay_         (true),
	writesyncstream_     (false),
	state_               (gs_notrunning),
	cmdqueue_            (*this),
	/** TRANSLATORS: Win condition for this game has not been set. */
	win_condition_displayname_(_("Not set"))
{
}

Game::~Game()
{
}


void Game::sync_reset() {
	syncwrapper_.counter_ = 0;

	synchash_.Reset();
	log("[sync] Reset\n");
}


/**
 * Returns true if cheat codes have been activated (single-player only)
 */
bool Game::get_allow_cheats()
{
	return true;
}


/**
 * \return a pointer to the \ref InteractivePlayer if any.
 * \note This function may return 0 (in particular, it will return 0 during
 * playback or if player is spectator)
 */
InteractivePlayer * Game::get_ipl()
{
	return dynamic_cast<InteractivePlayer *>(get_ibase());
}

void Game::set_game_controller(GameController * const ctrl)
{
	ctrl_ = ctrl;
}

GameController * Game::game_controller()
{
	return ctrl_;
}

void Game::set_write_replay(bool const wr)
{
	//  we want to allow for the possibility to stop writing our replay
	//  this is to ensure we do not crash because of diskspace
	//  still this is only possibe to go from true->false
	//  still probally should not do this with an assert but with better checks
	assert(state_ == gs_notrunning || !wr);

	writereplay_ = wr;
}

void Game::set_write_syncstream(bool const wr)
{
	assert(state_ == gs_notrunning);

	writesyncstream_ = wr;
}


/**
 * Set whether the syncstream dump should be copied to a permanent location
 * at the end of the game.
 */
void Game::save_syncstream(bool const save)
{
	syncwrapper_.syncstreamsave_ = save;
}


bool Game::run_splayer_scenario_direct(const std::string& mapname, const std::string& script_to_run) {
	assert(!get_map());

	// Replays can't handle scenarios
	set_write_replay(false);

	set_map(new Map);

	std::unique_ptr<MapLoader> maploader(map().get_correct_loader(mapname));
	if (!maploader)
		throw wexception("could not load \"%s\"", mapname.c_str());
	UI::ProgressWindow loader_ui;

	loader_ui.step (_("Preloading map"));
	maploader->preload_map(true);
	std::string const background = map().get_background();
	if (!background.empty()) {
		loader_ui.set_background(background);
	}
	loader_ui.step(_("Loading world"));
	world();
	loader_ui.step(_("Loading tribes"));
	tribes();

	// We have to create the players here.
	loader_ui.step(_("Creating players"));
	PlayerNumber const nr_players = map().get_nrplayers();
	iterate_player_numbers(p, nr_players) {
		add_player
			(p,
			 0,
			 map().get_scenario_player_tribe(p),
			 map().get_scenario_player_name (p));
		get_player(p)->set_ai(map().get_scenario_player_ai(p));
	}
	win_condition_displayname_ = "Scenario";

	set_ibase
		(new InteractivePlayer
		 	(*this, g_options.pull_section("global"), 1, false));

	loader_ui.step(_("Loading map…"));
	maploader->load_map_complete(*this, Widelands::MapLoader::LoadType::kScenario);
	maploader.reset();

	set_game_controller(new SinglePlayerGameController(*this, true, 1));
	try {
		bool const result = run(&loader_ui, NewSPScenario, script_to_run, false, "single_player");
		delete ctrl_;
		ctrl_ = nullptr;
		return result;
	} catch (...) {
		delete ctrl_;
		ctrl_ = nullptr;
		throw;
	}
}


/**
 * Initialize the game based on the given settings.
 *
 */
void Game::init_newgame
	(UI::ProgressWindow* loader_ui, const GameSettings& settings)
{
	assert(loader_ui != nullptr);

	loader_ui->step(_("Preloading map"));

	assert(!get_map());
	set_map(new Map);

	std::unique_ptr<MapLoader> maploader
		(map().get_correct_loader(settings.mapfilename));
	assert(maploader != nullptr);
	maploader->preload_map(settings.scenario);

	loader_ui->step(_("Loading world"));
	world();

	loader_ui->step(_("Loading tribes"));
	tribes();

	std::string const background = map().get_background();
	if (!background.empty()) {
		loader_ui->set_background(background);
	}
	loader_ui->step(_("Creating players"));

	std::vector<PlayerSettings> shared;
	std::vector<uint8_t>        shared_num;
	for (uint32_t i = 0; i < settings.players.size(); ++i) {
		const PlayerSettings & playersettings = settings.players[i];

		if
			(playersettings.state == PlayerSettings::stateClosed ||
			 playersettings.state == PlayerSettings::stateOpen)
			continue;
		else if (playersettings.state == PlayerSettings::stateShared) {
			shared.push_back(playersettings);
			shared_num.push_back(i + 1);
			continue;
		}

		add_player
			(i + 1,
			 playersettings.initialization_index,
			 playersettings.tribe,
			 playersettings.name,
			 playersettings.team);
		get_player(i + 1)->set_ai(playersettings.ai);
	}

	// Add shared in starting positions
	for (uint8_t n = 0; n < shared.size(); ++n) {
		// This player's starting position is used in another (shared) kingdom
		get_player(shared.at(n).shared_in)
			->add_further_starting_position(shared_num.at(n), shared.at(n).initialization_index);
	}

	loader_ui->step(_("Loading map…"));
	maploader->load_map_complete(*this,
										  settings.scenario ?
											  Widelands::MapLoader::LoadType::kScenario :
											  Widelands::MapLoader::LoadType::kGame);

	// Check for win_conditions
	if (!settings.scenario) {
		std::unique_ptr<LuaTable> table(lua().run_script(settings.win_condition_script));
		table->do_not_warn_about_unaccessed_keys();
		win_condition_displayname_ = table->get_string("name");
		std::unique_ptr<LuaCoroutine> cr = table->get_coroutine("func");
		enqueue_command(new CmdLuaCoroutine(get_gametime() + 100, cr.release()));
	} else {
		win_condition_displayname_ = "Scenario";
	}
}



/**
 * Initialize the savegame based on the given settings.
 * At return the game is at the same state like a map loaded with Game::init()
 * Only difference is, that players are already initialized.
 * run<Returncode>() takes care about this difference.
 */
void Game::init_savegame
	(UI::ProgressWindow* loader_ui, const GameSettings& settings)
{
	assert(loader_ui != nullptr);

	loader_ui->step(_("Preloading map"));

	assert(!get_map());
	set_map(new Map);
	try {
		GameLoader gl(settings.mapfilename, *this);
		Widelands::GamePreloadPacket gpdp;
		gl.preload_game(gpdp);
		win_condition_displayname_ = gpdp.get_win_condition();
		if (win_condition_displayname_ == "Scenario") {
			// Replays can't handle scenarios
			set_write_replay(false);
		}
		std::string background(gpdp.get_background());
		loader_ui->set_background(background);
		loader_ui->step(_("Loading…"));
		gl.load_game(settings.multiplayer);
	} catch (...) {
		throw;
	}
}

bool Game::run_load_game(const std::string& filename, const std::string& script_to_run) {
	UI::ProgressWindow loader_ui;
	std::vector<std::string> tipstext;
	tipstext.push_back("general_game");
	tipstext.push_back("singleplayer");
	GameTips tips (loader_ui, tipstext);
	int8_t player_nr;

	loader_ui.step(_("Preloading map"));

	// We have to create an empty map, otherwise nothing will load properly
	set_map(new Map);

	{
		GameLoader gl(filename, *this);

		Widelands::GamePreloadPacket gpdp;
		gl.preload_game(gpdp);
		std::string background(gpdp.get_background());
		win_condition_displayname_ = gpdp.get_win_condition();
		if (win_condition_displayname_ == "Scenario") {
			// Replays can't handle scenarios
			set_write_replay(false);
		}
		loader_ui.set_background(background);
		player_nr = gpdp.get_player_nr();
		set_ibase
			(new InteractivePlayer
			 	(*this, g_options.pull_section("global"), player_nr, false));

		loader_ui.step(_("Loading…"));
		gl.load_game();
	}

	// Store the filename for further saves
	save_handler().set_current_filename(filename);

	set_game_controller(new SinglePlayerGameController(*this, true, player_nr));
	try {
		bool const result = run(&loader_ui, Loaded, script_to_run, false, "single_player");
		delete ctrl_;
		ctrl_ = nullptr;
		return result;
	} catch (...) {
		delete ctrl_;
		ctrl_ = nullptr;
		throw;
	}
}

/**
 * Called for every game after loading (from a savegame or just from a map
 * during single/multiplayer/scenario).
 *
 * Ensure that players and player controllers are setup properly (in particular
 * AI and the \ref InteractivePlayer if any).
 */
void Game::postload()
{
	EditorGameBase::postload();
	get_ibase()->postload();
}


/**
 * This runs a game, including game creation phase.
 *
 * The setup and loading of a game happens (or rather: will happen) in three
 * stages.
 * 1.  First of all, the host (or single player) configures the game. During
 *     this time, only short descriptions of the game data (such as map
 *     headers)are loaded to minimize loading times.
 * 2a. Once the game is about to start and the configuration screen is finished,
 *     all logic data (map, tribe information, building information) is loaded
 *     during postload.
 * 2b. If a game is created, initial player positions are set. This step is
 *     skipped when a game is loaded.
 * 3.  After this has happened, the game graphics are loaded.
 *
 * \return true if a game actually took place, false otherwise
 */
bool Game::run
	(UI::ProgressWindow * loader_ui, StartGameType const start_game_type,
	 const std::string& script_to_run, bool replay, const std::string& prefix_for_replays)
{
	assert(loader_ui != nullptr);

	replay_ = replay;
	postload();

	if (start_game_type != Loaded) {
		PlayerNumber const nr_players = map().get_nrplayers();
		if (start_game_type == NewNonScenario) {
			if (loader_ui) {
				loader_ui->step(_("Creating player infrastructure"));
			}
			iterate_players_existing(p, nr_players, *this, plr) {
				plr->create_default_infrastructure();
			}
		} else {
			// Is a scenario!
			// Replays can't handle scenarios
			set_write_replay(false);
			iterate_players_existing_novar(p, nr_players, *this) {
				if (!map().get_starting_pos(p))
				throw WLWarning
					(_("Missing starting position"),
					 _
					 	("Widelands could not start the game, because player %u has "
					 	 "no starting position.\n"
					 	 "You can manually add a starting position with the Widelands "
					 	 "Editor to fix this problem."),
					 static_cast<unsigned int>(p));
			}
		}

		if (get_ipl())
			get_ipl()->move_view_to
				(map().get_starting_pos(get_ipl()->player_number()));

		// Prepare the map, set default textures
		map().recalc_default_resources(world());

		// Finally, set the scenario names and tribes to represent
		// the correct names of the players
		iterate_player_numbers(p, nr_players) {
			const Player * const plr = get_player(p);
			const std::string                                              no_name;
			const std::string & player_tribe = plr ? plr->tribe().name() : no_name;
			const std::string & player_name  = plr ? plr->    get_name() : no_name;
			const std::string & player_ai    = plr ? plr->    get_ai()    : no_name;
			map().set_scenario_player_tribe    (p, player_tribe);
			map().set_scenario_player_name     (p, player_name);
			map().set_scenario_player_ai       (p, player_ai);
			map().set_scenario_player_closeable(p, false); // player is already initialized.
		}

		// Run the init script, if the map provides one.
		if (start_game_type == NewSPScenario)
			enqueue_command(new CmdLuaScript(get_gametime(), "map:scripting/init.lua"));
		else if (start_game_type == NewMPScenario)
			enqueue_command
				(new CmdLuaScript(get_gametime(), "map:scripting/multiplayer_init.lua"));

		// Queue first statistics calculation
		enqueue_command(new CmdCalculateStatistics(get_gametime() + 1));
	}

	if (!script_to_run.empty() && (start_game_type == NewSPScenario || start_game_type == Loaded)) {
		enqueue_command(new CmdLuaScript(get_gametime() + 1, script_to_run));
	}

	if (writereplay_ || writesyncstream_) {
		// Derive a replay filename from the current time
		const std::string fname = (boost::format("%s/%s_%s%s") % REPLAY_DIR % timestring() %
		                           prefix_for_replays % REPLAY_SUFFIX).str();
		if (writereplay_) {
			log("Starting replay writer\n");

			assert(!replaywriter_);
			replaywriter_.reset(new ReplayWriter(*this, fname));

			log("Replay writer has started\n");
		}

		if (writesyncstream_)
			syncwrapper_.start_dump(fname);
	}

	sync_reset();

	load_graphics(*loader_ui);

#ifdef _WIN32
	//  Clear the event queue before starting game because we don't want
	//  to handle events at game start that happened during loading procedure.
	SDL_Event event;
	while (SDL_PollEvent(&event));
#endif

	g_sound_handler.change_music("ingame", 1000, 0);

	state_ = gs_running;

	get_ibase()->run<UI::Panel::Returncodes>();

	state_ = gs_ending;

	g_sound_handler.change_music("menu", 1000, 0);

	cleanup_objects();
	set_ibase(nullptr);

	state_ = gs_notrunning;

	return true;
}


/**
 * think() is called by the UI objects initiated during Game::run()
 * during their modal loop.
 * Depending on the current state we advance game logic and stuff,
 * running the cmd queue etc.
 */
void Game::think()
{
	assert(ctrl_);

	ctrl_->think();

	if (state_ == gs_running) {
		// TODO(sirver): This is not good. Here, it depends on the speed of the
		// computer and the fps if and when the game is saved - this is very bad
		// for scenarios and even worse for the regression suite (which relies on
		// the timings of savings.
		cmdqueue().run_queue(ctrl_->get_frametime(), get_gametime_pointer());

		// check if autosave is needed
		savehandler_.think(*this);
	}
}

/**
 * Cleanup for load
 * \deprecated
 */
// TODO(unknown): Get rid of this. Prefer to delete and recreate Game-style objects
// Note that this needs fixes in the editor.
void Game::cleanup_for_load()
{
	state_ = gs_notrunning;

	EditorGameBase::cleanup_for_load();

	cmdqueue().flush();

	// Statistics
	general_stats_.clear();
}


/**
 * Game logic code may write to the synchronization
 * token stream. All written data will be hashed and can be used to
 * check for network or replay desyncs.
 *
 * \return the synchronization token stream
 *
 * \note This is returned as a \ref StreamWrite object to prevent
 * the caller from messing with the checksumming process.
 */
StreamWrite & Game::syncstream()
{
	return syncwrapper_;
}


/**
 * Calculate the current synchronization checksum and copy
 * it into the given array, without affecting the subsequent
 * checksumming process.
 *
 * \return the checksum
 */
Md5Checksum Game::get_sync_hash() const
{
	MD5Checksum<StreamWrite> copy(synchash_);

	copy.finish_checksum();
	return copy.get_checksum();
}


/**
 * Return a random value that can be used in parallel game logic
 * simulation.
 *
 * \note Do NOT use for random events in the UI or other display code.
 */
uint32_t Game::logic_rand()
{
	uint32_t const result = rng().rand();
	syncstream().unsigned_32(result);
	return result;
}


/**
 * All player-issued commands must enter the queue through this function.
 * It takes the appropriate action, i.e. either add to the cmd_queue or send
 * across the network.
 */
void Game::send_player_command (PlayerCommand & pc)
{
	ctrl_->send_player_command(pc);
}


/**
 * Actually enqueue a command.
 *
 * \note In a network game, player commands are only allowed to enter the
 * command queue after being accepted by the networking logic via
 * \ref send_player_command, so you must never enqueue a player command
 * directly.
 */
void Game::enqueue_command (Command * const cmd)
{
	if (writereplay_ && replaywriter_) {
		if (upcast(PlayerCommand, plcmd, cmd)) {
			replaywriter_->send_player_command(plcmd);
		}
	}
	cmdqueue().enqueue(cmd);
}

// we might want to make these inlines:
void Game::send_player_bulldoze (PlayerImmovable & pi, bool const recurse)
{
	send_player_command
		(*new CmdBulldoze
		 	(get_gametime(), pi.owner().player_number(), pi, recurse));
}

void Game::send_player_dismantle (PlayerImmovable & pi)
{
	send_player_command
		(*new CmdDismantleBuilding
		 	(get_gametime(), pi.owner().player_number(), pi));
}


void Game::send_player_build
	(int32_t const pid, Coords const coords, DescriptionIndex const id)
{
	assert(tribes().building_exists(id));
	send_player_command (*new CmdBuild(get_gametime(), pid, coords, id));
}

void Game::send_player_build_flag (int32_t const pid, Coords const coords)
{
	send_player_command (*new CmdBuildFlag(get_gametime(), pid, coords));
}

void Game::send_player_build_road (int32_t pid, Path & path)
{
	send_player_command (*new CmdBuildRoad(get_gametime(), pid, path));
}

void Game::send_player_flagaction (Flag & flag)
{
	send_player_command
		(*new CmdFlagAction
		 	(get_gametime(), flag.owner().player_number(), flag));
}

void Game::send_player_start_stop_building (Building & building)
{
	send_player_command
		(*new CmdStartStopBuilding
		 	(get_gametime(), building.owner().player_number(), building));
}

void Game::send_player_militarysite_set_soldier_preference (Building & building, uint8_t my_preference)
{
	send_player_command
		(*new CmdMilitarySiteSetSoldierPreference
		 (get_gametime(), building.owner().player_number(), building, my_preference));
}

void Game::send_player_start_or_cancel_expedition (Building & building)
{
	send_player_command
		(*new CmdStartOrCancelExpedition
		 	(get_gametime(), building.owner().player_number(), building));
}

void Game::send_player_enhance_building
	(Building & building, DescriptionIndex const id)
{
	assert(building.owner().tribe().has_building(id));

	send_player_command
		(*new CmdEnhanceBuilding
		 	(get_gametime(), building.owner().player_number(), building, id));
}

void Game::send_player_evict_worker(Worker & worker)
{
	send_player_command
		(*new CmdEvictWorker
			(get_gametime(), worker.owner().player_number(), worker));
}

void Game::send_player_set_ware_priority
	(PlayerImmovable &       imm,
	 int32_t           const type,
	 DescriptionIndex        const index,
	 int32_t           const prio)
{
	send_player_command
		(*new CmdSetWarePriority
		 	(get_gametime(),
		 	 imm.owner().player_number(),
		 	 imm,
		 	 type,
		 	 index,
		 	 prio));
}

void Game::send_player_set_ware_max_fill
	(PlayerImmovable &       imm,
	 DescriptionIndex        const index,
	 uint32_t          const max_fill)
{
	send_player_command
		(*new CmdSetWareMaxFill
		 	(get_gametime(),
		 	 imm.owner().player_number(),
		 	 imm,
		 	 index,
		 	 max_fill));
}


void Game::send_player_change_training_options
	(TrainingSite & ts, TrainingAttribute attr, int32_t const val)
{
	send_player_command
		(*new CmdChangeTrainingOptions
			(get_gametime(), ts.owner().player_number(), ts, attr, val));
}

void Game::send_player_drop_worker (Building & b, int32_t const ser)
{
	assert(ser != -1);
	send_player_command
		(*new CmdDropWorker
		 	(get_gametime(), b.owner().player_number(), b, ser));
}

void Game::send_player_change_worker_capacity
	(Building & b, DescriptionIndex worker_type, int16_t const val)
{
	send_player_command
		(*new CmdChangeWorkerCapacity
		 	(get_gametime(), b.owner().player_number(), b, worker_type, val));
}

void Game::send_player_drop_soldier (Building & b, int32_t const ser)
{
	assert(ser != -1);
	send_player_command
		(*new CmdDropSoldier
		 	(get_gametime(), b.owner().player_number(), b, ser));
}

void Game::send_player_change_soldier_capacity
	(Building & b, int32_t const val)
{
	send_player_command
		(*new CmdChangeSoldierCapacity
		 	(get_gametime(), b.owner().player_number(), b, val));
}

/////////////////////// TESTING STUFF
void Game::send_player_enemyflagaction
	(const Flag  &       flag,
	 PlayerNumber const who_attacks,
	 uint32_t      const num_soldiers)
{
	if
		(1
		 <
		 player(who_attacks).vision
		 	(Map::get_index
		 	 	(flag.get_building()->get_position(), map().get_width())))
		send_player_command
			(*new CmdEnemyFlagAction
			 	(get_gametime(), who_attacks, flag, num_soldiers));
}


void Game::send_player_ship_scouting_direction(Ship & ship, WalkingDir direction)
{
	send_player_command
		(*new CmdShipScoutDirection
			(get_gametime(), ship.get_owner()->player_number(), ship.serial(), direction));
}

void Game::send_player_ship_construct_port(Ship & ship, Coords coords)
{
	send_player_command
		(*new CmdShipConstructPort
			(get_gametime(), ship.get_owner()->player_number(), ship.serial(), coords));
}

void Game::send_player_ship_explore_island(Ship & ship, IslandExploreDirection direction)
{
	send_player_command
		(*new CmdShipExploreIsland
			(get_gametime(), ship.get_owner()->player_number(), ship.serial(), direction));
}

void Game::send_player_sink_ship(Ship & ship) {
	send_player_command
		(*new CmdShipSink
			(get_gametime(), ship.get_owner()->player_number(), ship.serial()));
}

void Game::send_player_cancel_expedition_ship(Ship & ship) {
	send_player_command
		(*new CmdShipCancelExpedition
			(get_gametime(), ship.get_owner()->player_number(), ship.serial()));
}

LuaGameInterface& Game::lua() {
	return static_cast<LuaGameInterface&>(EditorGameBase::lua());
}

/**
 * Sample global statistics for the game.
 */
void Game::sample_statistics()
{
	// Update general stats
	PlayerNumber const nr_plrs = map().get_nrplayers();
	std::vector<uint32_t> land_size;
	std::vector<uint32_t> nr_buildings;
	std::vector<uint32_t> nr_casualties;
	std::vector<uint32_t> nr_kills;
	std::vector<uint32_t> nr_msites_lost;
	std::vector<uint32_t> nr_msites_defeated;
	std::vector<uint32_t> nr_civil_blds_lost;
	std::vector<uint32_t> nr_civil_blds_defeated;
	std::vector<uint32_t> miltary_strength;
	std::vector<uint32_t> nr_workers;
	std::vector<uint32_t> nr_wares;
	std::vector<uint32_t> productivity;
	std::vector<uint32_t> nr_production_sites;
	std::vector<uint32_t> custom_statistic;
	land_size             .resize(nr_plrs);
	nr_buildings          .resize(nr_plrs);
	nr_casualties         .resize(nr_plrs);
	nr_kills              .resize(nr_plrs);
	nr_msites_lost        .resize(nr_plrs);
	nr_msites_defeated    .resize(nr_plrs);
	nr_civil_blds_lost    .resize(nr_plrs);
	nr_civil_blds_defeated.resize(nr_plrs);
	miltary_strength      .resize(nr_plrs);
	nr_workers            .resize(nr_plrs);
	nr_wares              .resize(nr_plrs);
	productivity          .resize(nr_plrs);
	nr_production_sites   .resize(nr_plrs);
	custom_statistic      .resize(nr_plrs);

	//  We walk the map, to gain all needed information.
	const Map &  themap = map();
	Extent const extent = themap.extent();
	iterate_Map_FCoords(themap, extent, fc) {
		if (PlayerNumber const owner = fc.field->get_owned_by())
			++land_size[owner - 1];

			// Get the immovable
		if (upcast(Building, building, fc.field->get_immovable()))
			if (building->get_position() == fc) { // only count main location
				uint8_t const player_index = building->owner().player_number() - 1;
				++nr_buildings[player_index];

				//  If it is a productionsite, add its productivity.
				if (upcast(ProductionSite, productionsite, building)) {
					++nr_production_sites[player_index];
					productivity[player_index] +=
						productionsite->get_statistics_percent();
				}
			}

			// Now, walk the bobs
		for (Bob const * b = fc.field->get_first_bob(); b; b = b->get_next_bob())
			if (upcast(Soldier const, s, b))
				miltary_strength[s->owner().player_number() - 1] +=
					s->get_level(TrainingAttribute::kTotal) + 1; //  So that level 0 also counts.
	}

	//  Number of workers / wares / casualties / kills.
	iterate_players_existing(p, nr_plrs, *this, plr) {
		uint32_t wostock = 0;
		uint32_t wastock = 0;

		for (uint32_t j = 0; j < plr->get_nr_economies(); ++j) {
			Economy * const eco = plr->get_economy_by_number(j);
			const TribeDescr & tribe = plr->tribe();

			for (const DescriptionIndex& ware_index : tribe.wares()) {
				wastock += eco->stock_ware(ware_index);
			}

			for (const DescriptionIndex& worker_index : tribe.workers()) {
				if (tribe.get_worker_descr(worker_index)->type() != MapObjectType::CARRIER) {
					wostock += eco->stock_worker(worker_index);
				}
			}
		}
		nr_wares  [p - 1] = wastock;
		nr_workers[p - 1] = wostock;
		nr_casualties[p - 1] = plr->casualties();
		nr_kills     [p - 1] = plr->kills     ();
		nr_msites_lost        [p - 1] = plr->msites_lost        ();
		nr_msites_defeated    [p - 1] = plr->msites_defeated    ();
		nr_civil_blds_lost    [p - 1] = plr->civil_blds_lost    ();
		nr_civil_blds_defeated[p - 1] = plr->civil_blds_defeated();
	}

	// Now, divide the statistics
	for (uint32_t i = 0; i < map().get_nrplayers(); ++i) {
		if (productivity[i])
			productivity[i] /= nr_production_sites[i];
	}

	// If there is a hook function defined to sample special statistics in this
	// game, call the corresponding Lua function
	std::unique_ptr<LuaTable> hook = lua().get_hook("custom_statistic");
	if (hook) {
		hook->do_not_warn_about_unaccessed_keys();
		iterate_players_existing(p, nr_plrs, *this, plr) {
			std::unique_ptr<LuaCoroutine> cr(hook->get_coroutine("calculator"));
			cr->push_arg(plr);
			cr->resume();
			custom_statistic[p - 1] = cr->pop_uint32();
		}
	}

	// Now, push this on the general statistics
	general_stats_.resize(map().get_nrplayers());
	for (uint32_t i = 0; i < map().get_nrplayers(); ++i) {
		general_stats_[i].land_size       .push_back(land_size       [i]);
		general_stats_[i].nr_buildings    .push_back(nr_buildings    [i]);
		general_stats_[i].nr_casualties   .push_back(nr_casualties   [i]);
		general_stats_[i].nr_kills        .push_back(nr_kills        [i]);
		general_stats_[i].nr_msites_lost        .push_back
			(nr_msites_lost        [i]);
		general_stats_[i].nr_msites_defeated    .push_back
			(nr_msites_defeated    [i]);
		general_stats_[i].nr_civil_blds_lost    .push_back
			(nr_civil_blds_lost    [i]);
		general_stats_[i].nr_civil_blds_defeated.push_back
			(nr_civil_blds_defeated[i]);
		general_stats_[i].miltary_strength.push_back(miltary_strength[i]);
		general_stats_[i].nr_workers      .push_back(nr_workers      [i]);
		general_stats_[i].nr_wares        .push_back(nr_wares        [i]);
		general_stats_[i].productivity    .push_back(productivity    [i]);
		general_stats_[i].custom_statistic.push_back(custom_statistic[i]);
	}


	// Calculate statistics for the players
	const PlayerNumber nr_players = map().get_nrplayers();
	iterate_players_existing(p, nr_players, *this, plr)
		plr->sample_statistics();
}


/**
 * Read statistics data from a file.
 *
 * \param fr file to read from
 */
void Game::read_statistics(FileRead & fr)
{
	fr.unsigned_32(); // used to be last stats update time

	// Read general statistics
	uint32_t entries = fr.unsigned_16();
	const PlayerNumber nr_players = map().get_nrplayers();
	general_stats_.resize(nr_players);

	iterate_players_existing_novar(p, nr_players, *this) {
		general_stats_[p - 1].land_size       .resize(entries);
		general_stats_[p - 1].nr_workers      .resize(entries);
		general_stats_[p - 1].nr_buildings    .resize(entries);
		general_stats_[p - 1].nr_wares        .resize(entries);
		general_stats_[p - 1].productivity    .resize(entries);
		general_stats_[p - 1].nr_casualties   .resize(entries);
		general_stats_[p - 1].nr_kills        .resize(entries);
		general_stats_[p - 1].nr_msites_lost        .resize(entries);
		general_stats_[p - 1].nr_msites_defeated    .resize(entries);
		general_stats_[p - 1].nr_civil_blds_lost    .resize(entries);
		general_stats_[p - 1].nr_civil_blds_defeated.resize(entries);
		general_stats_[p - 1].miltary_strength.resize(entries);
		general_stats_[p - 1].custom_statistic.resize(entries);
	}

	iterate_players_existing_novar(p, nr_players, *this)
		for (uint32_t j = 0; j < general_stats_[p - 1].land_size.size(); ++j)
		{
			general_stats_[p - 1].land_size       [j] = fr.unsigned_32();
			general_stats_[p - 1].nr_workers      [j] = fr.unsigned_32();
			general_stats_[p - 1].nr_buildings    [j] = fr.unsigned_32();
			general_stats_[p - 1].nr_wares        [j] = fr.unsigned_32();
			general_stats_[p - 1].productivity    [j] = fr.unsigned_32();
			general_stats_[p - 1].nr_casualties   [j] = fr.unsigned_32();
			general_stats_[p - 1].nr_kills        [j] = fr.unsigned_32();
			general_stats_[p - 1].nr_msites_lost        [j] = fr.unsigned_32();
			general_stats_[p - 1].nr_msites_defeated    [j] = fr.unsigned_32();
			general_stats_[p - 1].nr_civil_blds_lost    [j] = fr.unsigned_32();
			general_stats_[p - 1].nr_civil_blds_defeated[j] = fr.unsigned_32();
			general_stats_[p - 1].miltary_strength[j] = fr.unsigned_32();
			general_stats_[p - 1].custom_statistic[j] = fr.unsigned_32();
		}
}


/**
 * Write general statistics to the given file.
 */
void Game::write_statistics(FileWrite & fw)
{
	fw.unsigned_32(0); // Used to be last stats update time. No longer needed

	// General statistics
	// First, we write the size of the statistics arrays
	uint32_t entries = 0;

	const PlayerNumber nr_players = map().get_nrplayers();
	iterate_players_existing_novar(p, nr_players, *this)
		if (!general_stats_.empty()) {
			entries = general_stats_[p - 1].land_size.size();
			break;
		}

	fw.unsigned_16(entries);

	iterate_players_existing_novar(p, nr_players, *this)
		for (uint32_t j = 0; j < entries; ++j) {
			fw.unsigned_32(general_stats_[p - 1].land_size       [j]);
			fw.unsigned_32(general_stats_[p - 1].nr_workers      [j]);
			fw.unsigned_32(general_stats_[p - 1].nr_buildings    [j]);
			fw.unsigned_32(general_stats_[p - 1].nr_wares        [j]);
			fw.unsigned_32(general_stats_[p - 1].productivity    [j]);
			fw.unsigned_32(general_stats_[p - 1].nr_casualties   [j]);
			fw.unsigned_32(general_stats_[p - 1].nr_kills        [j]);
			fw.unsigned_32(general_stats_[p - 1].nr_msites_lost        [j]);
			fw.unsigned_32(general_stats_[p - 1].nr_msites_defeated    [j]);
			fw.unsigned_32(general_stats_[p - 1].nr_civil_blds_lost    [j]);
			fw.unsigned_32(general_stats_[p - 1].nr_civil_blds_defeated[j]);
			fw.unsigned_32(general_stats_[p - 1].miltary_strength[j]);
			fw.unsigned_32(general_stats_[p - 1].custom_statistic[j]);
		}
}

double logic_rand_as_double(Game* game) {
	return static_cast<double>(game->logic_rand()) / std::numeric_limits<uint32_t>::max();
}

}
