/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef __S__EDITOR_GAME_BASE_H
#define __S__EDITOR_GAME_BASE_H

#include "bob.h"
#include "building.h"
#include "constants.h"
#include "map.h"
#include "player_area.h"

#include <cassert>
#include <map>
#include <string>
#include <vector>

struct AreaWatcher;
class Battle;
class Bob;
class Building_Descr;
class Immovable;
class Interactive_Base;
class Map;
class Object_Manager;
class Player;
class PlayerImmovable;
class Tribe_Descr;
class Flag;
class AttackController;
namespace UI {struct ProgressWindow;};

struct Editor_Game_Base {
	friend class Interactive_Base;
	friend class Game_Game_Class_Data_Packet;
	friend struct Fullscreen_Menu_LaunchGame;

	Editor_Game_Base();
	virtual ~Editor_Game_Base();

	void set_map(Map* map);
	Map & map() const throw () {return *m_map;}
	Map *get_map() {return m_map;}
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
		(const Player_Number plnum,
		 const int32_t type,
		 const std::string & tribe,
		 const std::string & name);
	Player * get_player(const int32_t n) const {
		assert(n >= 1);
		assert(n <= MAX_PLAYERS);
		return m_players[n - 1];
	}
	Player & player(const int32_t n) const {
		assert(1 <= n);
		assert     (n <= MAX_PLAYERS);
		return *m_players[n - 1];
	}
	virtual Player * get_safe_player(const int32_t n);

	// loading stuff
	void allocate_player_maps();
	virtual void postload();
	void load_graphics(UI::ProgressWindow & loader_ui);
	virtual void cleanup_for_load
		(const bool flush_graphics = true, const bool flush_animations = true);

	void set_road(const FCoords, const Uint8 direction, const Uint8 roadtype);

	// warping stuff. instantly creating map_objects
	Building * warp_building
		(const Coords,
		 const Player_Number,
		 const Building_Descr::Index);
		Building* warp_constructionsite(Coords c, int8_t owner, int32_t idx, int32_t oldid=-1);
	Bob * create_bob(const Coords, const Bob::Descr::Index, const Tribe_Descr * const = 0);
	Immovable* create_immovable(const Coords c, int32_t idx, const Tribe_Descr*);
	Immovable* create_immovable
		(const Coords c, const std::string & name, const Tribe_Descr*);
	Battle* create_battle ();
	AttackController* create_attack_controller(Flag* flag, int32_t attacker, int32_t defender, uint32_t num);
	AttackController* create_attack_controller();
	void remove_attack_controller(uint32_t serial);
	void register_attack_controller(AttackController* ctrl);
	const std::vector<uint32_t>& get_attack_controller_serials() const {return m_attack_serials;}

	std::vector<int32_t> get_battle_serials() const {return m_battle_serials;}
	typedef int32_t Time;
	static Time     Never  () throw () {return 0xffffffff;}
	typedef Uint32 Duration;
	static Duration Forever() throw ()
	{return std::numeric_limits<Duration>::max();}

	int32_t get_gametime() const {return m_gametime;}
	Interactive_Base * get_iabase() const {return m_iabase;}

	// safe system for storing pointers to non-Map_Object C++ objects
	// unlike objects in the Object_Manager, these pointers need not be
	// synchronized across the network, and they are not saved in savegames
	uint32_t add_trackpointer(void* ptr);
	void* get_trackpointer(uint32_t serial);
	void remove_trackpointer(uint32_t serial);

	// Manually load a tribe into memory. Used by the editor
	const Tribe_Descr & manually_load_tribe(const std::string & tribe);
	// Get a tribe from the loaded list, when available
	const Tribe_Descr * get_tribe(const char * const tribe) const;

	void inform_players_about_ownership(const Map::Index, const Player_Number);
	void inform_players_about_immovable
		(const Map::Index, const Map_Object_Descr * const);
	void inform_players_about_road
		(const FCoords, const Map_Object_Descr * const);

	enum losegain_t {LOSE=0, GAIN};
	virtual void player_immovable_notification (PlayerImmovable*, losegain_t);
	virtual void player_field_notification (const FCoords&, losegain_t);

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
		 const bool conquer,
		 const Player_Number preferred_player = 0,
		 const bool neutral_when_no_influence = false,
		 const bool neutral_when_competing_influence = false,
		 const bool conquer_guarded_location_by_superior_influence = false);

private:
	void cleanup_playerimmovables_area(const Area<FCoords>);

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
public:
	std::vector<int32_t>           m_battle_serials;    // The serials of the battles only used to load/save
	std::vector<uint32_t>          m_attack_serials;

private:
	Editor_Game_Base & operator=(const Editor_Game_Base &);
	Editor_Game_Base            (const Editor_Game_Base &);
};

#define iterate_players_existing(p, nr_players, egbase, player)                \
	iterate_player_numbers(p, nr_players)                                       \
		if (Player * const player = (egbase).get_player(p))                      \

#define iterate_players_existing_const(p, nr_players, egbase, player)          \
	iterate_player_numbers(p, nr_players)                                       \
		if (const Player * const player = (egbase).get_player(p))                \

extern const uint8_t g_playercolors[MAX_PLAYERS][12];

#endif // __S__EDITOR_GAME_BASE_H
