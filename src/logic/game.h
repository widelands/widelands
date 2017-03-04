/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_LOGIC_GAME_H
#define WL_LOGIC_GAME_H

#include <memory>

#include "base/md5.h"
#include "io/streamwrite.h"
#include "logic/cmd_queue.h"
#include "logic/editor_game_base.h"
#include "logic/save_handler.h"
#include "random/random.h"
#include "scripting/logic.h"

namespace UI {
struct ProgressWindow;
}
struct ComputerPlayer;
class InteractivePlayer;
struct GameMainMenuLoadGame;
struct WLApplication;
struct GameSettings;
class GameController;

namespace Widelands {

struct Flag;
struct Path;
struct PlayerImmovable;
enum class IslandExploreDirection;
enum class ScoutingDirection;
struct Ship;
struct PlayerEndStatus;
class TrainingSite;
class MilitarySite;

#define WLGF_SUFFIX ".wgf"
#define WLGF_MAGIC "WLgf"

/** class Game
 *
 * This class manages the entire lifetime of a game session, from creating the
 * game and setting options, selecting maps to the actual playing phase and the
 * final statistics screen(s).
 */
enum {
	gs_notrunning = 0,  // game is being prepared
	gs_running,         // game was fully prepared at some point and is now in-game
	gs_ending
};

class Player;
class MapLoader;
class PlayerCommand;
class ReplayReader;
class ReplayWriter;

class Game : public EditorGameBase {
public:
	struct GeneralStats {
		std::vector<uint32_t> land_size;
		std::vector<uint32_t> nr_workers;
		std::vector<uint32_t> nr_buildings;
		std::vector<uint32_t> nr_wares;
		std::vector<uint32_t> productivity;
		std::vector<uint32_t> nr_casualties;
		std::vector<uint32_t> nr_kills;
		std::vector<uint32_t> nr_msites_lost;
		std::vector<uint32_t> nr_msites_defeated;
		std::vector<uint32_t> nr_civil_blds_lost;
		std::vector<uint32_t> nr_civil_blds_defeated;
		std::vector<uint32_t> miltary_strength;

		std::vector<uint32_t> custom_statistic;
	};
	using GeneralStatsVector = std::vector<GeneralStats>;

	friend class CmdQueue;  // this class handles the commands
	friend struct GameClassPacket;
	friend struct GamePlayerInfoPacket;
	friend struct GameLoader;
	friend struct ::GameMainMenuLoadGame;
	friend struct ::WLApplication;

	Game();
	~Game();

	// life cycle
	void set_game_controller(GameController*);
	GameController* game_controller();
	void set_write_replay(bool wr);
	void set_write_syncstream(bool wr);
	void save_syncstream(bool save);
	void init_newgame(UI::ProgressWindow* loader_ui, const GameSettings&);
	void init_savegame(UI::ProgressWindow* loader_ui, const GameSettings&);
	enum StartGameType { NewSPScenario, NewNonScenario, Loaded, NewMPScenario };

	bool run(UI::ProgressWindow* loader_ui,
	         StartGameType,
	         const std::string& script_to_run,
	         bool replay,
	         const std::string& prefix_for_replays);

	// Returns the upcasted lua interface.
	LuaGameInterface& lua() override;

	// Run a single player scenario directly via --scenario on the cmdline. Will
	// run the 'script_to_run' after any init scripts of the map.
	// Returns the result of run().
	bool run_splayer_scenario_direct(const std::string& mapname, const std::string& script_to_run);

	// Run a single player loaded game directly via --loadgame on the cmdline. Will
	// run the 'script_to_run' directly after the game was loaded.
	// Returns the result of run().
	bool run_load_game(const std::string& filename, const std::string& script_to_run);

	void postload() override;

	void think() override;

	ReplayWriter* get_replaywriter() {
		return replaywriter_.get();
	}

	/**
	 * \return \c true if the game is completely loaded and running (or paused)
	 * or \c false otherwise.
	 */
	bool is_loaded() {
		return state_ == gs_running;
	}

	void cleanup_for_load() override;

	// in-game logic
	const CmdQueue& cmdqueue() const {
		return cmdqueue_;
	}
	CmdQueue& cmdqueue() {
		return cmdqueue_;
	}
	const RNG& rng() const {
		return rng_;
	}
	RNG& rng() {
		return rng_;
	}

	uint32_t logic_rand();

	/// Generate a random location within radius from location.
	Coords random_location(Coords location, uint8_t radius);

	void logic_rand_seed(uint32_t const seed) {
		rng().seed(seed);
	}

	StreamWrite& syncstream();
	Md5Checksum get_sync_hash() const;

	bool get_allow_cheats();

	void enqueue_command(Command* const);

	void send_player_command(Widelands::PlayerCommand&);

	void send_player_bulldoze(PlayerImmovable&, bool recurse = false);
	void send_player_dismantle(PlayerImmovable&);
	void send_player_build(int32_t, const Coords&, DescriptionIndex);
	void send_player_build_flag(int32_t, const Coords&);
	void send_player_build_road(int32_t, Path&);
	void send_player_flagaction(Flag&);
	void send_player_start_stop_building(Building&);
	void send_player_militarysite_set_soldier_preference(Building&, uint8_t preference);
	void send_player_start_or_cancel_expedition(Building&);

	void send_player_enhance_building(Building&, DescriptionIndex);
	void send_player_evict_worker(Worker&);
	void send_player_set_ware_priority(PlayerImmovable&,
	                                   int32_t type,
	                                   DescriptionIndex index,
	                                   int32_t prio);
	void send_player_set_input_max_fill(PlayerImmovable&,
	                                    DescriptionIndex index,
	                                    WareWorker type,
	                                    uint32_t);
	void send_player_change_training_options(TrainingSite&, TrainingAttribute, int32_t);
	void send_player_drop_soldier(Building&, int32_t);
	void send_player_change_soldier_capacity(Building&, int32_t);
	void send_player_enemyflagaction(const Flag&, PlayerNumber, uint32_t count);

	void send_player_ship_scouting_direction(Ship&, WalkingDir);
	void send_player_ship_construct_port(Ship&, Coords);
	void send_player_ship_explore_island(Ship&, IslandExploreDirection);
	void send_player_sink_ship(Ship&);
	void send_player_cancel_expedition_ship(Ship&);

	InteractivePlayer* get_ipl();

	SaveHandler& save_handler() {
		return savehandler_;
	}

	// Statistics
	const GeneralStatsVector& get_general_statistics() const {
		return general_stats_;
	}

	void read_statistics(FileRead&);
	void write_statistics(FileWrite&);

	void sample_statistics();

	const std::string& get_win_condition_displayname() {
		return win_condition_displayname_;
	}

	bool is_replay() const {
		return replay_;
	}

private:
	void sync_reset();

	MD5Checksum<StreamWrite> synchash_;

	struct SyncWrapper : public StreamWrite {
		SyncWrapper(Game& game, StreamWrite& target)
		   : game_(game),
		     target_(target),
		     counter_(0),
		     next_diskspacecheck_(0),
		     syncstreamsave_(false) {
		}

		~SyncWrapper();

		/// Start dumping the entire syncstream into a file.
		///
		/// Note that this file is deleted at the end of the game, unless
		/// \ref syncstreamsave_ has been set.
		void start_dump(const std::string& fname);

		void data(void const* data, size_t size) override;

		void flush() override {
			target_.flush();
		}

	public:
		Game& game_;
		StreamWrite& target_;
		uint32_t counter_;
		uint32_t next_diskspacecheck_;
		std::unique_ptr<StreamWrite> dump_;
		std::string dumpfname_;
		bool syncstreamsave_;
	} syncwrapper_;

	GameController* ctrl_;

	/// Whether a replay writer should be created.
	/// Defaults to \c true, and should only be set to \c false for playing back
	/// replays.
	bool writereplay_;

	/// Whether a syncsteam file should be created.
	/// Defaults to \c false, and can be set to true for network games. The file
	/// is written only if \ref writereplay_ is true too.
	bool writesyncstream_;

	int32_t state_;

	RNG rng_;

	CmdQueue cmdqueue_;

	SaveHandler savehandler_;

	std::unique_ptr<ReplayWriter> replaywriter_;

	GeneralStatsVector general_stats_;

	/// For save games and statistics generation
	std::string win_condition_displayname_;
	bool replay_;
};

inline Coords Game::random_location(Coords location, uint8_t radius) {
	const uint16_t s = radius * 2 + 1;
	location.x += logic_rand() % s - radius;
	location.y += logic_rand() % s - radius;
	return location;
}

// Returns a value between [0., 1].
double logic_rand_as_double(Game* game);
}

#endif  // end of include guard: WL_LOGIC_GAME_H
