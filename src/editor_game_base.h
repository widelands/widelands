/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef EDITOR_GAME_BASE_H
#define EDITOR_GAME_BASE_H

#include "bob.h"
#include "building.h"
#include "constants.h"
#include "map.h"
#include "notification.h"
#include "player_area.h"

#include <cassert>
#include <string>
#include <cstring>
#include <vector>

namespace UI {struct ProgressWindow;};
struct Fullscreen_Menu_LaunchGame;
struct Interactive_Base;

namespace Widelands {

struct AreaWatcher;
struct Battle;
class Bob;
struct Building_Descr;
class Immovable;
struct Map;
struct Object_Manager;
struct Player;
struct PlayerImmovable;
struct Tribe_Descr;
class Flag;
struct AttackController;

struct Editor_Game_Base : NoteReceiver<NoteImmovable>, NoteReceiver<NoteField>
{
	friend struct ::Fullscreen_Menu_LaunchGame;
	friend struct ::Interactive_Base;
	friend class Game_Game_Class_Data_Packet;

	Editor_Game_Base();
	virtual ~Editor_Game_Base();

	void set_map(Map *);
	Map & map() const throw () {return *m_map;}
	Map * get_map() {return m_map;}
	Map & get_map() const {return *m_map;}
	const Object_Manager & objects() const {return m_objects;}
	Object_Manager       & objects()       {return m_objects;}

	void unconquer_area
		(Player_Area<Area<FCoords> >, const Player_Number destroying_player = 0);
	void conquer_area                  (Player_Area<Area<FCoords> >);
	void conquer_area_no_building(const Player_Area<Area<FCoords> >);

	// logic handler func
	virtual void think();

	// Player commands
	void remove_player(Player_Number);
	Player * add_player
		(Player_Number,
		 uint8_t             initialization_index,
		 const std::string & tribe,
		 const std::string & name);
	Player * get_player(const int32_t n) const {
		assert(1 <= n);
		assert     (n <= MAX_PLAYERS);
		return m_players[n - 1];
	}
	Player & player(const int32_t n) const {
		assert(1 <= n);
		assert     (n <= MAX_PLAYERS);
		return *m_players[n - 1];
	}
	virtual Player * get_safe_player(Player_Number);

	// loading stuff
	void allocate_player_maps();
	virtual void postload();
	void load_graphics(UI::ProgressWindow & loader_ui);
	virtual void cleanup_for_load
		(bool flush_graphics = true, bool flush_animations = true);

	void set_road(FCoords, uint8_t direction, uint8_t roadtype);

	// warping stuff. instantly creating map_objects
	Building * warp_building(Coords, Player_Number, Building_Index);
	Building * warp_constructionsite
		(Coords, Player_Number, Building_Index,
		 Building_Index oldid = Building_Index::Null());
	Bob * create_bob(Coords, Bob::Descr::Index, const Tribe_Descr * const = 0);
	Immovable & create_immovable(Coords, int32_t idx, Tribe_Descr const *);
	Immovable & create_immovable
		(Coords, std::string const & name, Tribe_Descr const *);

	int32_t get_gametime() const {return m_gametime;}
	Interactive_Base * get_iabase() const {return m_iabase;}

	// safe system for storing pointers to non-Map_Object C++ objects
	// unlike objects in the Object_Manager, these pointers need not be
	// synchronized across the network, and they are not saved in savegames
	uint32_t add_trackpointer(void *);
	void * get_trackpointer(uint32_t serial);
	void remove_trackpointer(uint32_t serial);

	// Manually load a tribe into memory. Used by the editor
	const Tribe_Descr & manually_load_tribe(const std::string & tribe);
	Tribe_Descr const & manually_load_tribe(Player_Number const p) {
		return manually_load_tribe(map().get_scenario_player_tribe(p));
	}
	// Get a tribe from the loaded list, when available
	Tribe_Descr const * get_tribe(const char * tribe) const;

	void inform_players_about_ownership(Map_Index, Player_Number);
	void inform_players_about_immovable(Map_Index, Map_Object_Descr const *);
	void inform_players_about_road     (FCoords,   Map_Object_Descr const *);

	void receive(NoteImmovable const &);
	void receive(NoteField     const &);

	void cleanup_objects() throw () {
		objects().cleanup(this);
	}

	// next function is used to update the current gametime,
	// for queue runs e.g.
	int32_t* get_game_time_pointer() {return &m_gametime;}
	void set_iabase(Interactive_Base* b) {m_iabase=b;}

protected:
	/**
	 *
	 * \param preferred_player
	 *  When conquer is false, this can be used to prefer a player over other
	 *  players, when lost land is reassigned. This can for example be used
	 *  to reward the player who actually destroyed a MilitarySite by giving
	 *  an unconquered location that he has influence over to him, instead of
	 *  some other player who has higher influence over that location. If 0,
	 *  land is simply assigned by influence.
	 *
	 * \param neutral_when_no_influence
	 *  If true and the player completely loses influence over a location, it
	 *  becomes neutral unless some other player claims it by having
	 *  positive influence.
	 *
	 * \param neutral_when_competing_influence
	 *  If true and the player completely loses influence over a location and
	 *  several players have positive and equal influence, the location
	 *  becomes neutral unless some other player claims it by having higher
	 *  influence.
	 *
	 * \param conquer_guarded_location_by_superior_influence
	 *  If true, the conquering player will (automatically, without actually
	 *  attacking) conquer a location even if another player already owns and
	 *  covers the location with a militarysite, if the conquering player's
	 *  influence becomes greater than the owner's influence.
	 */
	virtual void do_conquer_area
		(Player_Area<Area<FCoords> > player_area,
		 bool          conquer,
		 Player_Number preferred_player                               = 0,
		 bool          neutral_when_no_influence                      = false,
		 bool          neutral_when_competing_influence               = false,
		 bool          conquer_guarded_location_by_superior_influence = false);

private:
	void cleanup_playerimmovables_area(Player_Area<Area<FCoords> >);

	int32_t m_gametime;
	Player                   * m_players[MAX_PLAYERS];
	Object_Manager             m_objects;
protected:
	typedef std::vector<Tribe_Descr *> Tribe_Vector;
	Tribe_Vector           m_tribes;
private:
	Interactive_Base         * m_iabase;
	Map                      * m_map;

	uint32_t                       m_lasttrackserial;
	std::map<uint32_t, void *>     m_trackpointers;

private:
	Editor_Game_Base & operator= (Editor_Game_Base const &);
	explicit Editor_Game_Base    (Editor_Game_Base const &);
};

#define iterate_players_existing(p, nr_players, egbase, player)               \
	iterate_player_numbers(p, nr_players)                                      \
		if (Widelands::Player * const player = (egbase).get_player(p))          \

#define iterate_players_existing_const(p, nr_players, egbase, player)         \
	iterate_player_numbers(p, nr_players)                                      \
		if (Widelands::Player const * const player = (egbase).get_player(p))    \

extern const uint8_t g_playercolors[MAX_PLAYERS][12];

};

#endif
