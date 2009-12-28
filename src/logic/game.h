/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef GAME_H
#define GAME_H

#include "cmd_queue.h"
#include "editor_game_base.h"
#include "md5.h"
#include "random.h"
#include "save_handler.h"

namespace UI {struct ProgressWindow;}
struct Computer_Player;
struct Interactive_Player;
struct Game_Main_Menu_Load_Game;
struct WLApplication;
struct GameSettings;
struct GameController;

namespace Widelands {

struct Flag;
struct Path;
struct PlayerImmovable;
struct TrainingSite;

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
	gs_running      // game was fully prepared at some point and is now in-game
};

struct Player;
struct Map_Loader;
struct PlayerCommand;
struct ReplayReader;
struct ReplayWriter;

struct Game : public Editor_Game_Base {
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
	};
	typedef std::vector<General_Stats> General_Stats_vector;

	friend class Cmd_Queue; // this class handles the commands
	friend class Game_Game_Class_Data_Packet;
	friend class Game_Player_Info_Data_Packet;
	friend class Game_Loader;
	friend struct ::Game_Main_Menu_Load_Game;
	friend struct ::WLApplication;

	// This friend is for legacy reasons and should probably be removed
	// at least after summer 2008, maybe even earlier.
	friend class Game_Interactive_Player_Data_Packet;

	Game();
	~Game();

	// life cycle
	void set_game_controller(GameController *);
	GameController * gameController();
	void set_write_replay(bool wr);
	void set_write_syncstream(bool wr);
	void save_syncstream(bool save);
	void init_newgame (UI::ProgressWindow &, GameSettings const &);
	void init_savegame(UI::ProgressWindow &, GameSettings const &);
	bool run_splayer_scenario_direct(char const * mapname);
	bool run_load_game (std::string filename);
	enum Start_Game_Type {NewScenario, NewNonScenario, Loaded};
	bool run(UI::ProgressWindow & loader_ui, Start_Game_Type);

	virtual void postload();

	void unconquer_area
		(Player_Area<Area<FCoords> >, Player_Number destroying_player = 0);
	void   conquer_area            (Player_Area<Area<FCoords> >);
	void   conquer_area_no_building(Player_Area<Area<FCoords> > const);

	void think();

	ReplayWriter * get_replaywriter() {return m_replaywriter;}

	/**
	 * \return \c true if the game is completely loaded and running (or paused)
	 * or \c false otherwise.
	 */
	bool is_loaded() {return m_state == gs_running;}

	void cleanup_for_load
		(const bool flush_graphics = true, const bool flush_animations = true);

	// in-game logic
	Cmd_Queue const & cmdqueue() const {return m_cmdqueue;}
	Cmd_Queue       & cmdqueue()       {return m_cmdqueue;}
	RNG       const & rng     () const {return m_rng;}
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
	void send_player_build      (int32_t, Coords, Building_Index);
	void send_player_build_flag (int32_t, Coords);
	void send_player_build_road (int32_t, Path &);
	void send_player_flagaction (Flag &);
	void send_player_start_stop_building (Building &);
	void send_player_enhance_building (Building &, Building_Index);
	void send_player_set_ware_priority
		(PlayerImmovable &, int32_t type, Ware_Index index, int32_t prio);
	void send_player_change_training_options(TrainingSite &, int32_t, int32_t);
	void send_player_drop_soldier(Building &, int32_t);
	void send_player_change_soldier_capacity(Building &, int32_t);
	void send_player_enemyflagaction
		(Flag const &, Player_Number, uint32_t count, uint32_t retreat);

	Interactive_Player * get_ipl();

	SaveHandler & save_handler() {return m_savehandler;}

	// Statistics
	const General_Stats_vector & get_general_statistics() const {
		return m_general_stats;
	}

	void ReadStatistics(FileRead &, uint32_t version);
	void WriteStatistics(FileWrite &);

private:
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
	virtual void do_conquer_area
		(Player_Area<Area<FCoords> > player_area,
		 bool          conquer,
		 Player_Number preferred_player                               = 0,
		 bool          neutral_when_no_influence                      = false,
		 bool          neutral_when_competing_influence               = false,
		 bool          conquer_guarded_location_by_superior_influence = false);

	void cleanup_playerimmovables_area(Player_Area<Area<FCoords> >);
	void sample_statistics();

private:
	void SyncReset();

	MD5Checksum<StreamWrite> m_synchash;

	struct SyncWrapper : public StreamWrite {
		SyncWrapper(Game & game, StreamWrite & target) :
			m_game          (game),
			m_target        (target),
			m_counter       (0),
			m_dump          (0),
			m_syncstreamsave(false)
		{}

		~SyncWrapper();

		/// Start dumping the entire syncstream into a file.
		///
		/// Note that this file is deleted at the end of the game, unless
		/// \ref m_syncstreamsave has been set.
		void StartDump(std::string const & fname);

		void Data(void const * data, size_t size);

		void Flush() {m_target.Flush();}

	public:
		Game        &   m_game;
		StreamWrite &   m_target;
		uint32_t        m_counter;
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

	ReplayReader * m_replayreader;
	ReplayWriter * m_replaywriter;

	uint32_t m_last_stats_update;
	General_Stats_vector m_general_stats;

};

inline Coords Game::random_location(Coords location, uint8_t radius) {
	const uint16_t s = radius * 2 + 1;
	location.x += logic_rand() % s - radius;
	location.y += logic_rand() % s - radius;
	return location;
}

}

#endif
