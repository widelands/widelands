/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#ifndef GAME_H
#define GAME_H

#include "logic/cmd_queue.h"
#include "logic/editor_game_base.h"
#include "md5.h"
#include "random.h"
#include "save_handler.h"

namespace UI {struct ProgressWindow;}
struct Computer_Player;
struct Interactive_Player;
struct Game_Main_Menu_Load_Game;
struct WLApplication;
struct GameSettings;
class GameController;

namespace Widelands {

struct Flag;
struct Path;
struct PlayerImmovable;
struct Ship;
struct PlayerEndStatus;
class TrainingSite;
class MilitarySite;

#define WLGF_SUFFIX ".wgf"
#define WLGF_MAGIC      "WLgf"

/** class Game
 *
 * This class manages the entire lifetime of a game session, from creating the
 * game and setting options, selecting maps to the actual playing phase and the
 * final statistics screen(s).
 */
enum {
	gs_notrunning = 0, // game is being prepared
	gs_running,        // game was fully prepared at some point and is now in-game
	gs_ending
};

class Player;
class Map_Loader;
class PlayerCommand;
class ReplayReader;
class ReplayWriter;

class Game : public Editor_Game_Base {
public:
	struct General_Stats {
		std::vector< uint32_t > land_size;
		std::vector< uint32_t > nr_workers;
		std::vector< uint32_t > nr_buildings;
		std::vector< uint32_t > nr_wares;
		std::vector< uint32_t > productivity;
		std::vector< uint32_t > nr_casualties;
		std::vector< uint32_t > nr_kills;
		std::vector< uint32_t > nr_msites_lost;
		std::vector< uint32_t > nr_msites_defeated;
		std::vector< uint32_t > nr_civil_blds_lost;
		std::vector< uint32_t > nr_civil_blds_defeated;
		std::vector< uint32_t > miltary_strength;

		std::vector< uint32_t > custom_statistic;
	};
	typedef std::vector<General_Stats> General_Stats_vector;

	friend class Cmd_Queue; // this class handles the commands
	friend struct Game_Game_Class_Data_Packet;
	friend struct Game_Player_Info_Data_Packet;
	friend struct Game_Loader;
	friend struct ::Game_Main_Menu_Load_Game;
	friend struct ::WLApplication;

	Game();
	~Game();

	// life cycle
	void set_game_controller(GameController *);
	GameController * gameController();
	void set_write_replay(bool wr);
	void set_write_syncstream(bool wr);
	void save_syncstream(bool save);
	void init_newgame (UI::ProgressWindow *, const GameSettings &);
	void init_savegame(UI::ProgressWindow *, const GameSettings &);
	enum Start_Game_Type {NewSPScenario, NewNonScenario, Loaded, NewMPScenario};
	bool run(UI::ProgressWindow * loader_ui, Start_Game_Type, const std::string& script_to_run, bool replay);

	// Run a single player scenario directly via --scenario on the cmdline. Will
	// run the 'script_to_run' after any init scripts of the map.
	// Returns the result of run().
	bool run_splayer_scenario_direct(char const * mapname, const std::string& script_to_run);

	// Run a single player loaded game directly via --loadgame on the cmdline. Will
	// run the 'script_to_run' directly after the game was loaded.
	// Returns the result of run().
	bool run_load_game (std::string filename, const std::string& script_to_run);

	virtual void postload() override;

	void think() override;

	ReplayWriter * get_replaywriter() {return m_replaywriter;}

	/**
	 * \return \c true if the game is completely loaded and running (or paused)
	 * or \c false otherwise.
	 */
	bool is_loaded() {return m_state == gs_running;}
	void end_dedicated_game();

	void cleanup_for_load();

	// in-game logic
	const Cmd_Queue & cmdqueue() const {return m_cmdqueue;}
	Cmd_Queue       & cmdqueue()       {return m_cmdqueue;}
	const RNG       & rng     () const {return m_rng;}
	RNG             & rng     ()       {return m_rng;}

	uint32_t logic_rand();

	/// Generate a random location within radius from location.
	Coords random_location(Coords location, uint8_t radius);

	void logic_rand_seed (uint32_t const seed) {rng().seed (seed);}

	StreamWrite & syncstream();
	md5_checksum get_sync_hash() const;

	bool get_allow_cheats();

	void enqueue_command (Command * const);

	void send_player_command (Widelands::PlayerCommand &);

	void send_player_bulldoze   (PlayerImmovable &, bool recurse = false);
	void send_player_dismantle  (PlayerImmovable &);
	void send_player_build      (int32_t, Coords, Building_Index);
	void send_player_build_flag (int32_t, Coords);
	void send_player_build_road (int32_t, Path &);
	void send_player_flagaction (Flag &);
	void send_player_start_stop_building (Building &);
	void send_player_militarysite_set_soldier_preference (Building &, uint8_t preference);
	void send_player_start_or_cancel_expedition    (Building &);

	void send_player_enhance_building (Building &, Building_Index);
	void send_player_evict_worker (Worker &);
	void send_player_set_ware_priority
		(PlayerImmovable &, int32_t type, Ware_Index index, int32_t prio);
	void send_player_set_ware_max_fill
		(PlayerImmovable &, Ware_Index index, uint32_t);
	void send_player_change_training_options(TrainingSite &, int32_t, int32_t);
	void send_player_drop_soldier(Building &, int32_t);
	void send_player_change_soldier_capacity(Building &, int32_t);
	void send_player_enemyflagaction
		(const Flag &, Player_Number, uint32_t count, uint8_t retreat);
	void send_player_changemilitaryconfig(Player_Number, uint8_t);

	void send_player_ship_scout_direction(Ship &, uint8_t);
	void send_player_ship_construct_port(Ship &, Coords);
	void send_player_ship_explore_island(Ship &, bool);
	void send_player_sink_ship(Ship &);
	void send_player_cancel_expedition_ship(Ship &);

	Interactive_Player * get_ipl();

	SaveHandler & save_handler() {return m_savehandler;}

	// Statistics
	const General_Stats_vector & get_general_statistics() const {
		return m_general_stats;
	}

	void ReadStatistics(FileRead &, uint32_t version);
	void WriteStatistics(FileWrite &);

	void sample_statistics();

	const std::string & get_win_condition_displayname() {return m_win_condition_displayname;}

	bool is_replay() const {return m_replay;};

private:
	void SyncReset();

	MD5Checksum<StreamWrite> m_synchash;

	struct SyncWrapper : public StreamWrite {
		SyncWrapper(Game & game, StreamWrite & target) :
			m_game          (game),
			m_target        (target),
			m_counter       (0),
			m_next_diskspacecheck(0),
			m_dump          (nullptr),
			m_syncstreamsave(false)
		{}

		~SyncWrapper();

		/// Start dumping the entire syncstream into a file.
		///
		/// Note that this file is deleted at the end of the game, unless
		/// \ref m_syncstreamsave has been set.
		void StartDump(const std::string & fname);

		void Data(void const * data, size_t size) override;

		void Flush() override {m_target.Flush();}

	public:
		Game        &   m_game;
		StreamWrite &   m_target;
		uint32_t        m_counter;
		uint32_t        m_next_diskspacecheck;
		::StreamWrite * m_dump;
		std::string     m_dumpfname;
		bool            m_syncstreamsave;
	}                    m_syncwrapper;

	GameController     * m_ctrl;

	/// Whether a replay writer should be created.
	/// Defaults to \c true, and should only be set to \c false for playing back
	/// replays.
	bool                 m_writereplay;

	/// Whether a syncsteam file should be created.
	/// Defaults to \c false, and can be set to true for network games. The file
	/// is written only if \ref m_writereplay is true too.
	bool                 m_writesyncstream;

	int32_t              m_state;

	RNG                  m_rng;

	Cmd_Queue            m_cmdqueue;

	SaveHandler          m_savehandler;

	ReplayReader       * m_replayreader;
	ReplayWriter       * m_replaywriter;

	General_Stats_vector m_general_stats;

	/// For save games and statistics generation
	std::string          m_win_condition_displayname;
	bool                 m_replay;
};

inline Coords Game::random_location(Coords location, uint8_t radius) {
	const uint16_t s = radius * 2 + 1;
	location.x += logic_rand() % s - radius;
	location.y += logic_rand() % s - radius;
	return location;
}

}

#endif
