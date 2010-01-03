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

#include "widelands_map_bobdata_data_packet.h"

#include "logic/battle.h"
#include "logic/bob.h"
#include "logic/carrier.h"
#include "logic/critter_bob.h"
#include "logic/critter_bob_program.h"
#include "economy/route.h"
#include "economy/transfer.h"
#include "economy/ware_instance.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/militarysite.h"
#include "logic/path.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "upcast.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "logic/worker_program.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 4

// Bob subtype versions
#define CRITTER_BOB_PACKET_VERSION 1
#define WORKER_BOB_PACKET_VERSION 1

// Worker subtype versions
#define SOLDIER_WORKER_BOB_PACKET_VERSION 6
#define CARRIER_WORKER_BOB_PACKET_VERSION 1


void Map_Bobdata_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const ol)
	throw (_wexception)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/bob_data");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (1 <= packet_version and packet_version <= CURRENT_PACKET_VERSION) {
			Map   const &       map        = egbase.map();
			Extent        const extent     = map.extent();
			Player_Number const nr_players = map.get_nrplayers();
			for (;;) {
				if (3 <= packet_version and fr.EndOfFile())
					break;
				Serial const serial = fr.Unsigned32();
				if (packet_version < 3 and serial == 0xffffffff) {
					if (not fr.EndOfFile())
						throw game_data_error
							("expected end of file after serial 0xffffffff");
					break;
				}
				try {
					Bob & bob = ol->get<Bob>(serial);
					Bob::Descr const & bob_descr = bob.descr();

					if (Player_Number const read_owner = fr.Player_Number8()) {
						if (nr_players < read_owner)
							throw game_data_error
								("owner number is %u but there are only %u players",
								 read_owner, nr_players);
						if (Player * const owner = egbase.get_player(read_owner))
							bob.set_owner(owner);
						else
							throw game_data_error
								("owning player %u does not exist", read_owner);
					}

					//  basic initialization
					bob.set_position(egbase, fr.Coords32(extent));

					Transfer * trans = 0;
					if (fr.Unsigned8()) { //  look if we had an transfer
						// stack should be non-empty at this point,
						// but savegames can always be broken/corrupted...
						if (bob.m_stack.size()) {
							trans = bob.m_stack[0].transfer;
							assert(trans);
						}
					}

					bob.m_actid = fr.Unsigned32();

					if (packet_version < 3) {
						bob.m_anim =
							fr.Unsigned8() ? bob_descr.get_animation(fr.CString()) :
							0;
					} else {
						char const * const anim_name = fr.CString();
						bob.m_anim =
							*anim_name ? bob_descr.get_animation(anim_name) : 0;
					}
					bob.m_animstart = fr.Signed32();

					if (packet_version < 3) {
						Map_Object::WalkingDir const walking_dir =
							static_cast<Map_Object::WalkingDir>(fr.Signed32());
						if (6 < walking_dir)
							throw game_data_error
								("walking dir is %u but must be one of {0 (idle), 1 "
								 "(northeast), 2 (east), 3 (southeast), 4 "
								 "(southwest), 5 (west), 6 (northwest)}",
								 walking_dir);
						bob.m_walking = walking_dir;
					} else
						try {
							bob.m_walking =
								static_cast<Map_Object::WalkingDir>
									(fr.Direction8_allow_null());
						} catch (StreamRead::direction_invalid const & e) {
							throw game_data_error
								("walking dir is %u but must be one of {0 (idle), 1 "
								 "(northeast), 2 (east), 3 (southeast), 4 "
								 "(southwest), 5 (west), 6 (northwest)}",
								 e.direction);
						}
					bob.m_walkstart = fr.Signed32();
					bob.m_walkend   = fr.Signed32();
					if (bob.m_walkend < bob.m_walkstart)
						throw game_data_error
							("walkend (%i) < walkstart (%i)",
							 bob.m_walkend, bob.m_walkstart);

					uint16_t const old_stacksize = bob.m_stack.size();
					uint16_t const new_stacksize = fr.Unsigned16();
					bob.m_stack.resize(new_stacksize);
					for (uint32_t i = 0; i < new_stacksize; ++i) {
						Bob::State & state = bob.m_stack[i];
						try {
							{ //  Task
								const Bob::Task * task;
								char const * const taskname = fr.CString();
								if      (not strcmp(taskname, "idle"))
									task = &Bob::taskIdle;
								else if (not strcmp(taskname, "movepath"))
									task = &Bob::taskMovepath;
								else if
									(packet_version == 1 &&
									 not strcmp(taskname, "forcemove"))
								{
									task = &Bob::taskMove;
								} else if (not strcmp(taskname, "move"))
									task = &Bob::taskMove;
								else if (not strcmp(taskname, "roam"))
									task = &Critter_Bob::taskRoam;
								else if (not strcmp(taskname, "program")) {
									if      (dynamic_cast<Worker      const *>(&bob))
										task = &Worker::taskProgram;
									else if (dynamic_cast<Critter_Bob const *>(&bob))
										task = &Critter_Bob::taskProgram;
									else
										throw;
								} else if (not strcmp(taskname, "transfer"))
									task = &Worker::taskTransfer;
								else if (not strcmp(taskname, "buildingwork"))
									task = &Worker::taskBuildingwork;
								else if (not strcmp(taskname, "return"))
									task = &Worker::taskReturn;
								else if (not strcmp(taskname, "gowarehouse"))
									task = &Worker::taskGowarehouse;
								else if (not strcmp(taskname, "dropoff"))
									task = &Worker::taskDropoff;
								else if (not strcmp(taskname, "fetchfromflag"))
									task = &Worker::taskFetchfromflag;
								else if (not strcmp(taskname, "waitforcapacity"))
									task = &Worker::taskWaitforcapacity;
								else if (not strcmp(taskname, "leavebuilding"))
									task = &Worker::taskLeavebuilding;
								else if (not strcmp(taskname, "fugitive"))
									task = &Worker::taskFugitive;
								else if (not strcmp(taskname, "geologist"))
									task = &Worker::taskGeologist;
								else if (not strcmp(taskname, "scout"))
									task = &Worker::taskScout;
								else if (not strcmp(taskname, "road"))
									task = &Carrier::taskRoad;
								else if (not strcmp(taskname, "transport"))
									task = &Carrier::taskTransport;
								else if (not strcmp(taskname, "attack"))
									task = &Soldier::taskAttack;
								else if (not strcmp(taskname, "defense"))
									task = &Soldier::taskDefense;
								else if (not strcmp(taskname, "battle"))
									task = &Soldier::taskBattle;
								else if (not strcmp(taskname, "die"))
									task = &Soldier::taskDie;
								else if (not strcmp(taskname, "moveInBattle"))
									task = &Soldier::taskMoveInBattle;
								else if
									(not strcmp(taskname, "moveToBattle") ||
									 not strcmp(taskname, "moveHome"))
									//  Weird hack to support legacy games.
									task = &Worker::taskBuildingwork;
								else if (*taskname == '\0')
									continue; // Skip task
								else
									throw game_data_error
										(_("unknown task type \"%s\""), taskname);

								state.task = task;
							}

							state.ivar1 = fr.Signed32();
							state.ivar2 = fr.Signed32();
							state.ivar3 = fr.Signed32();

							state.transfer = 0;

							if (Serial const objvar1_serial = fr.Unsigned32()) {
								try {
									state.objvar1 =
										&ol->get<Map_Object>(objvar1_serial);
								} catch (_wexception const & e) {
									throw game_data_error
										("objvar1 (%u): %s", objvar1_serial, e.what());
								}
							} else
								state.objvar1 = 0;
							state.svar1 = fr.CString();
							if (packet_version < 3) {
								int32_t const x = fr.Signed32();
								int32_t const y = fr.Signed32();
								state.coords = Coords(x, y);
								if (state.coords and (extent.w <= x or extent.h <= y))
									throw game_data_error
										("invalid coordinates (%i, %i)", x, y);
							} else
								state.coords = fr.Coords32_allow_null(extent);

							if (fr.Unsigned8()) {
								const uint32_t ans[6] = {
									bob_descr.get_animation(fr.CString()),
									bob_descr.get_animation(fr.CString()),
									bob_descr.get_animation(fr.CString()),
									bob_descr.get_animation(fr.CString()),
									bob_descr.get_animation(fr.CString()),
									bob_descr.get_animation(fr.CString())
								};
								state.diranims =
									new DirAnimations
										(ans[0], ans[1], ans[2], ans[3], ans[4], ans[5]);

								if
									(state.task == &Bob::taskMove and
									 packet_version < 4)
									throw game_data_error
										("savegame created with old broken game engine "
										 "version that pushed task move without "
										 "starting walk first; not fixed; try another "
										 "savegame from a slightly different point in "
										 "time if available; the erroneous state that "
										 "this bob is in only lasts 10ms");
							} else
								state.diranims = 0;

							uint32_t const pathsteps = fr.Unsigned16();
							if (i < old_stacksize) {
								delete state.path;
								state.path = 0;
							}
							if (pathsteps) {
								try {
									assert(not state.path);
									state.path = new Path(fr.Coords32(extent));
									for (uint32_t step = pathsteps; step; --step)
										try {
											state.path->append(map, fr.Direction8());
										} catch (_wexception const & e) {
											throw game_data_error
												("step #%u: %s",
												 pathsteps - step, e.what());
										}
								} catch (_wexception const & e) {
									throw game_data_error
										(_("reading path: %s"), e.what());
								}
							}

							if (i < old_stacksize && !trans)
								delete state.transfer;

							state.transfer =
								state.task == &Worker::taskGowarehouse
								||
								state.task == &Worker::taskTransfer
								?
								trans : 0;

							{
								bool const has_route = fr.Unsigned8();
								if (i < old_stacksize && state.route) {
									if (!has_route) {
										delete state.route;
										// in case we get an exception further down
										state.route = 0;
									} else
										state.route->init(0);
								}

								if (has_route) {
									Route * const route =
										state.route ? state.route : new Route();
									Route::LoadData d;
									route->load(d, fr);
									route->load_pointers(d, *ol);
									state.route = route;
								} else
									state.route = 0;
							}

							if (fr.Unsigned8()) {
								std::string progname = fr.CString();
								std::transform
									(progname.begin(), progname.end(), progname.begin(),
									 tolower);
								if      (upcast(Worker      const, wor, &bob))
									state.program = wor->descr().get_program(progname);
								else if (upcast(Critter_Bob const, cri, &bob))
									state.program = cri->descr().get_program(progname);
								else
									throw;
							} else
								state.program = 0;

						} catch (_wexception const & e) {
							throw game_data_error
								("reading state %u: %s", i, e.what());
						}
					}

					//  rest of bob stuff
					if (packet_version == 1) {
						fr.Unsigned8(); // used to be m_stack_dirty
						fr.Unsigned8(); // used to be m_sched_init_task
					}
					bob.m_signal          = fr.CString  ();

					if      (upcast(Critter_Bob, critter_bob, &bob))
						read_critter_bob(fr, egbase, ol, *critter_bob);
					else if (upcast(Worker,      worker,      &bob))
						read_worker_bob (fr, egbase, ol, *worker);
					else
						assert(false);

					ol->mark_object_as_loaded(&bob);
				} catch (_wexception const & e) {
					throw game_data_error(_("bob %u: %s"), serial, e.what());
				}
			}
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("bobdata: %s"), e.what());
	}
}

void Map_Bobdata_Data_Packet::read_critter_bob
	(FileRead & fr, Editor_Game_Base &, Map_Map_Object_Loader *, Critter_Bob &)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CRITTER_BOB_PACKET_VERSION) {
			// No data for critter bob currently
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("critter bob: %s"), e.what());
	}
}

void Map_Bobdata_Data_Packet::read_worker_bob
	(FileRead              & fr,
	 Editor_Game_Base      & egbase,
	 Map_Map_Object_Loader * ol,
	 Worker                & worker)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == WORKER_BOB_PACKET_VERSION) {
			bool oldsoldier_fix = false;

			if (upcast(Soldier, soldier, &worker)) {
				try {
					uint16_t const soldier_worker_bob_packet_version =
						fr.Unsigned16();
					if
						(2 <= soldier_worker_bob_packet_version
						 and
						 soldier_worker_bob_packet_version
						 <=
						 SOLDIER_WORKER_BOB_PACKET_VERSION)
					{
						Soldier_Descr const & descr = soldier->descr();

						uint32_t min_hp = descr.get_min_hp();
						assert(min_hp);
						{
							//  Soldiers created by old versions of Widelands have
							//  wrong values for m_hp_max and m_hp_current; they were
							//  soldier->descr().get_min_hp() less than they should
							//  be, see bug #1687368.
							uint32_t const broken_hp_compensation =
								soldier_worker_bob_packet_version < 3 ? min_hp : 0;

							soldier->m_hp_current =
								broken_hp_compensation + fr.Unsigned32();
							soldier->m_hp_max =
								broken_hp_compensation + fr.Unsigned32();
						}
						// This has been commented because now exists a 'die' task,
						// so a soldier can have 0 hitpoints if it's dying.
						//if (not soldier->m_hp_current)
						// throw game_data_error("no hitpoints (should be dead)");
						if (soldier->m_hp_max < soldier->m_hp_current)
							throw game_data_error
								("hp_max (%u) < hp_current (%u)",
								 soldier->m_hp_max, soldier->m_hp_current);

						soldier->m_min_attack    = fr.Unsigned32();
						soldier->m_max_attack    = fr.Unsigned32();
						if (soldier->m_max_attack < soldier->m_min_attack)
							throw game_data_error
								("max_attack = %u but must be at least %u",
								 soldier->m_max_attack, soldier->m_min_attack);

						soldier->m_defense       = fr.Unsigned32();

						soldier->m_evade         = fr.Unsigned32();

#define READLEVEL(variable, pn, maxfunction)                                  \
   soldier->variable = fr.Unsigned32();                                       \
   {                                                                          \
      uint32_t const max = descr.maxfunction();                               \
      if (max < soldier->variable) {                                          \
         log                                                                  \
            ("WARNING: %s %s (%u) of player %u has "                          \
             pn                                                               \
             "_level = %u but it can be at most %u, decreasing it to that "   \
             "value\n",                                                       \
             descr.tribe().name().c_str(), descr.descname().c_str(),          \
             soldier->serial(), soldier->owner().player_number(),             \
             soldier->variable, max);                                         \
         soldier->variable = max;                                             \
       }                                                                      \
   }                                                                          \

						READLEVEL(m_hp_level,      "hp",      get_max_hp_level);
						READLEVEL(m_attack_level,  "attack",  get_max_attack_level);
						READLEVEL(m_defense_level, "defense", get_max_defense_level);
						READLEVEL(m_evade_level,   "evade",   get_max_evade_level);

						{ //  validate hp values
							uint32_t const level_increase =
								descr.get_hp_incr_per_level() * soldier->m_hp_level;
							min_hp += level_increase;
							uint32_t const max = descr.get_max_hp() + level_increase;
							if (soldier->m_hp_max < min_hp) {
								//  The soldier's type's definition may have changed,
								//  so that max_hp must be larger. Adjust it and scale
								//  up the current amount of hitpoints proportionally.
								uint32_t const new_current =
									soldier->m_hp_current * min_hp / soldier->m_hp_max;
								log
									("WARNING: %s %s (%u) of player %u has hp_max = %u "
									 "but it must be at least %u (= %u + %u * %u), "
									 "changing it to that value and increasing current "
									 "hp from %u to %u\n",
									 descr.tribe().name().c_str(),
									 descr.descname().c_str(), soldier->serial(),
									 soldier->owner().player_number(),
									 soldier->m_hp_max, min_hp,
									 descr.get_min_hp(),
									 descr.get_hp_incr_per_level(), soldier->m_hp_level,
									 soldier->m_hp_current, new_current);
								soldier->m_hp_current = new_current;
								soldier->m_hp_max = min_hp;
							} else if (max < soldier->m_hp_max) {
								//  The soldier's type's definition may have changed,
								//  so that max_hp must be smaller. Adjust it and scale
								//  down the current amount of hitpoints
								//  proportionally. Round to the soldier's favour and
								//  make sure that the scaling does not kill the
								//  soldier.
								uint32_t const new_current =
									1
									+
									(soldier->m_hp_current * max - 1)
									/
									soldier->m_hp_max;
								assert(new_current);
								assert(new_current <= soldier->m_hp_max);
								log
									("WARNING: %s %s (%u) of player %u has hp_max = %u "
									 "but it can be at most %u (= %u + %u * %u), "
									 "changing it to that value and decreasing current "
									 "hp from %u to %u\n",
									 descr.tribe().name().c_str(),
									 descr.descname().c_str(), soldier->serial(),
									 soldier->owner().player_number(),
									 soldier->m_hp_max, max,
									 descr.get_max_hp(),
									 descr.get_hp_incr_per_level(), soldier->m_hp_level,
									 soldier->m_hp_current, new_current);
								soldier->m_hp_current = new_current;
								soldier->m_hp_max = max;
							}
						}

						{ //  validate attack values
							uint32_t const level_increase =
								descr.get_attack_incr_per_level()
								*
								soldier->m_attack_level;
							{
								uint32_t const min =
									descr.get_min_attack() + level_increase;
								if (soldier->m_min_attack < min) {
									log
										("WARNING: %s %s (%u) of player %u has "
										 "min_attack = %u but it must be at least %u (= "
										 "%u + %u * %u), changing it to that value\n",
										 descr.tribe().name().c_str(),
										 descr.descname().c_str(), soldier->serial(),
										 soldier->owner().player_number(),
										 soldier->m_min_attack, min,
										 descr.get_min_attack(),
										 descr.get_attack_incr_per_level(),
										 soldier->m_attack_level);
									soldier->m_min_attack = min;
									if (soldier->m_max_attack < min) {
										log
											(" (and changing max_attack from %u to the "
											 "same value)",
											 soldier->m_max_attack);
										soldier->m_max_attack = min;
									}
									log("\n");
								}
							}
							{
								uint32_t const max =
									descr.get_max_attack() + level_increase;
								if (max < soldier->m_max_attack) {
									log
										("WARNING: %s %s (%u) of player %u has "
										 "max_attack = %u but it can be at most %u (= "
										 "%u + %u * %u), changing it to that value",
										 descr.tribe().name().c_str(),
										 descr.descname().c_str(), soldier->serial(),
										 soldier->owner().player_number(),
										 soldier->m_max_attack, max,
										 descr.get_max_attack(),
										 descr.get_attack_incr_per_level(),
										 soldier->m_attack_level);
									soldier->m_max_attack = max;
									if (max < soldier->m_min_attack) {
										log
											(" (and changing min_attack from %u to the "
											 "same value)",
											 soldier->m_min_attack);
										soldier->m_min_attack = max;
									}
									log("\n");
								}
							}
							assert(soldier->m_min_attack <= soldier->m_max_attack);
						}

#define VALIDATE_VALUE(pn, valuefunct, incrfunct, level_variable, variable)   \
   {                                                                          \
      uint32_t const value =                                                  \
         descr.valuefunct() + descr.incrfunct() * soldier->level_variable;    \
      if (value != soldier->variable) {                                       \
         log                                                                  \
            ("WARNING: %s %s (%u) of player %u has "                          \
             pn                                                               \
             " = %u but it must be %u (= %u + %u * %u), changing it to that " \
             "value\n",                                                       \
             descr.tribe().name().c_str(), descr.descname().c_str(),          \
             soldier->serial(), soldier->owner().player_number(),             \
             soldier->variable, value,                                        \
             descr.valuefunct(), descr.incrfunct(), soldier->level_variable); \
         soldier->variable = value;                                           \
      }                                                                       \
   }                                                                          \

						VALIDATE_VALUE
							("defense", get_defense, get_defense_incr_per_level,
							 m_defense_level, m_defense);

						VALIDATE_VALUE
							("evade",   get_evade,   get_evade_incr_per_level,
							 m_evade_level,   m_evade);

						if (soldier_worker_bob_packet_version <= 3) {
							fr.Unsigned8 (); // old soldier->m_marked
							oldsoldier_fix = true;
						}
						if (soldier_worker_bob_packet_version >= 5)
							if (Serial const battle = fr.Unsigned32())
								soldier->m_battle = &ol->get<Battle>(battle);

						if (soldier_worker_bob_packet_version >= 6)
						{
							try {
								soldier->m_combat_walking =
									static_cast<CombatWalkingDir>
										(fr.Direction8_allow_null());
							} catch (StreamRead::direction_invalid const & e) {
								throw game_data_error
									("combat walking dir is %u but must be one of {0 "
									 "(none), 1 (combat walk to west), 2 (combat walk"
									 " to east), 3 (fighting at west), 4 (fighting at"
									 " east), 5 (return from west), 6 (return from "
									 "east)}",
									 e.direction);
							}
							soldier->m_combat_walkstart = fr.Signed32();
							soldier->m_combat_walkend   = fr.Signed32();
							if
								(soldier->m_combat_walkend <
								 soldier->m_combat_walkstart)
								throw game_data_error
									("combat_walkend (%i) < combat_walkstart (%i)",
									 soldier->m_combat_walkend,
									 soldier->m_combat_walkstart);
						}
					} else
						throw game_data_error
							(_("unknown/unhandled version %u"),
							 soldier_worker_bob_packet_version);
				} catch (_wexception const & e) {
					throw game_data_error(_("soldier: %s"), e.what());
				}
			} else if (upcast(Carrier, carrier, &worker)) {
				try {
					uint16_t const carrier_worker_bob_packet_version =
						fr.Unsigned16();
					if
						(carrier_worker_bob_packet_version
						 ==
						 CARRIER_WORKER_BOB_PACKET_VERSION)
						carrier->m_acked_ware = fr.Signed32();
					else
						throw game_data_error
							(_("unknown/unhandled version %u"),
							 carrier_worker_bob_packet_version);
				} catch (_wexception const & e) {
					throw game_data_error(_("carrier: %s"), e.what());
				}
			}

			if (uint32_t const location_serial = fr.Unsigned32()) {
				try {
					worker.set_location(&ol->get<PlayerImmovable>(location_serial));
				} catch (_wexception const & e) {
					throw game_data_error
						("location (%u): %s", location_serial, e.what());
				}
			} else
				worker.m_location = 0;

			if (uint32_t const carried_item_serial = fr.Unsigned32()) {
				try {
					worker.m_carried_item =
						&ol->get<WareInstance>(carried_item_serial);
				} catch (_wexception const & e) {
					throw game_data_error
						("carried item (%u): %s", carried_item_serial, e.what());
				}
			} else
				worker.m_carried_item = 0;

			// Skip supply

			worker.m_needed_exp  = fr.Signed32();
			worker.m_current_exp = fr.Signed32();

			Economy * economy = 0;
			if (PlayerImmovable * const location = worker.m_location.get(egbase))
				economy = location->get_economy();
			worker.set_economy(economy);
			if
				(WareInstance * const carried_item =
				 	worker.m_carried_item.get(egbase))
				carried_item->set_economy(economy);

			if (oldsoldier_fix)
				if (upcast(Soldier, soldier, &worker))
					if (upcast(Game, game, &egbase))
						if (upcast(MilitarySite, ms, soldier->get_location(egbase)))
							if (soldier->get_position() == ms->get_position()) {
								// Fix behaviour of soldiers in buildings
								soldier->reset_tasks
									(ref_cast<Game, Editor_Game_Base>(egbase));
								soldier->start_task_buildingwork
									(ref_cast<Game, Editor_Game_Base>(egbase));
								ms->update_soldier_request();
							}
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error
			("worker %p (%u): %s", &worker, worker.serial(), e.what());
	}
}


void Map_Bobdata_Data_Packet::Write
	(FileSystem           &       fs,
	 Editor_Game_Base     &       egbase,
	 Map_Map_Object_Saver * const os)
throw (_wexception)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	Map & map = egbase.map();
	for (uint16_t y = 0; y < map.get_height(); ++y) {
		for (uint16_t x = 0; x < map.get_width(); ++x) {

			std::vector<Bob *> bobarr;

			map.find_bobs
				(Area<FCoords>(map.get_fcoords(Coords(x, y)), 0), &bobarr);

			for (uint32_t i = 0; i < bobarr.size(); ++i) {
				Bob        const & bob   = *bobarr[i];
				Bob::Descr const & descr = bob.descr();
				assert(os->is_object_known(bob));
				uint32_t const reg = os->get_object_file_index(bob);

				fw.Unsigned32(reg);
				//  BOB STUFF

				fw.Unsigned8(bob.m_owner ? bob.m_owner->player_number() : 0);

				fw.Coords32(bob.m_position);
				//  FIELD can't be saved

				//  m_linknext, linkpprev are handled automatically

				//  Are we currently transferring?
				fw.Unsigned8(bob.m_stack.size() && bob.m_stack[0].transfer);

				fw.Unsigned32(bob.m_actid);
				// Don't have to save m_actscheduled, as that's only used for
				// integrity checks

				// Animation
				fw.CString
					(bob.m_anim ?
					 bob.descr().get_animation_name(bob.m_anim).c_str() : "");
				fw.Signed32(bob.m_animstart);

				fw.Direction8_allow_null(bob.m_walking);

				fw.Signed32(bob.m_walkstart);

				fw.Signed32(bob.m_walkend);

				//  number of states
				fw.Unsigned16(bob.m_stack.size());
				for (uint32_t index = 0; index < bob.m_stack.size(); ++index) {
					Bob::State const & s = bob.m_stack[index];

					//  Write name, enough to reconstruct the task structure
					fw.CString(s.task->name);

					fw.Signed32(s.ivar1);
					fw.Signed32(s.ivar2);
					fw.Signed32(s.ivar3);

					if (Map_Object const * const obj = s.objvar1.get(egbase)) {
						assert(os->is_object_known(*obj));
						fw.Unsigned32(os->get_object_file_index(*obj));
					} else
						fw.Unsigned32(0);

					fw.CString(s.svar1.c_str());

					fw.Coords32(s.coords);

					if (DirAnimations const * const diranims = s.diranims) {
						fw.Unsigned8(1);
						fw.CString
							(descr.get_animation_name(diranims->get_animation(1))
							 .c_str());
						fw.CString
							(descr.get_animation_name(diranims->get_animation(2))
							 .c_str());
						fw.CString
							(descr.get_animation_name(diranims->get_animation(3))
							 .c_str());
						fw.CString
							(descr.get_animation_name(diranims->get_animation(4))
							 .c_str());
						fw.CString
							(descr.get_animation_name(diranims->get_animation(5))
							 .c_str());
						fw.CString
							(descr.get_animation_name(diranims->get_animation(6))
							 .c_str());
					} else
						fw.Unsigned8(0);

					if (const Path * const path = s.path) {
						const Path::Step_Vector::size_type nr_steps =
							s.path->get_nsteps();
						fw.Unsigned16(nr_steps);
						if (nr_steps) {
							fw.Coords32(path->get_start());
							for
								(Path::Step_Vector::size_type idx = 0;
								 idx < nr_steps;
								 ++idx)
								fw.Unsigned8((*path)[idx]);
						}
					} else
						fw.Unsigned16(0);

					if (s.route) {
						fw.Unsigned8(1);
						s.route->save(fw, egbase, os);
					} else
						fw.Unsigned8(0);

					if (s.program) {
						fw.Unsigned8(1);
						fw.CString(s.program->get_name().c_str());
					} else
						fw.Unsigned8(0);

				}

				fw.CString(bob.m_signal.c_str());

				if      (upcast(Critter_Bob const, critter_bob, &bob))
					write_critter_bob(fw, egbase, os, *critter_bob);
				else if (upcast(Worker      const, worker,      &bob))
					write_worker_bob (fw, egbase, os, *worker);
				else
					assert(false);

				os->mark_object_as_saved(bob);
			}

		}
	}

	fw.Write(fs, "binary/bob_data");
}

void Map_Bobdata_Data_Packet::write_critter_bob
	(FileWrite            & fw,
	 Editor_Game_Base     &,
	 Map_Map_Object_Saver *,
	 Critter_Bob    const &)
{
	fw.Unsigned16(CRITTER_BOB_PACKET_VERSION);
}

void Map_Bobdata_Data_Packet::write_worker_bob
	(FileWrite            & fw,
	 Editor_Game_Base     & egbase,
	 Map_Map_Object_Saver * os,
	 Worker         const & worker)
{
	fw.Unsigned16(WORKER_BOB_PACKET_VERSION);

	if (upcast(Soldier const, soldier, &worker)) {
		fw.Unsigned16(SOLDIER_WORKER_BOB_PACKET_VERSION);
		fw.Unsigned32(soldier->m_hp_current);
		fw.Unsigned32(soldier->m_hp_max);
		fw.Unsigned32(soldier->m_min_attack);
		fw.Unsigned32(soldier->m_max_attack);
		fw.Unsigned32(soldier->m_defense);
		fw.Unsigned32(soldier->m_evade);
		fw.Unsigned32(soldier->m_hp_level);
		fw.Unsigned32(soldier->m_attack_level);
		fw.Unsigned32(soldier->m_defense_level);
		fw.Unsigned32(soldier->m_evade_level);
		if (soldier->m_battle)
			fw.Unsigned32(os->get_object_file_index(*soldier->m_battle));
		else
			fw.Unsigned32(0);
		// New at version 6
		fw.Direction8_allow_null(soldier->m_combat_walking);
		fw.Unsigned32(soldier->m_combat_walkstart);
		fw.Unsigned32(soldier->m_combat_walkend);
	} else if (upcast(Carrier const, carrier, &worker)) {
		fw.Unsigned16(CARRIER_WORKER_BOB_PACKET_VERSION);
		fw.Signed32(carrier->m_acked_ware);
	}

	if (Map_Object const * const loca = worker.m_location.get(egbase)) {
		assert(os->is_object_known(*loca));
		fw.Unsigned32(os->get_object_file_index(*loca));
	} else
		fw.Unsigned32(0);

	//  Economy is not our beer.

	if
		(Map_Object const * const carried_item =
		 	worker.m_carried_item.get(egbase))
	{
		assert(os->is_object_known(*carried_item));
		fw.Unsigned32(os->get_object_file_index(*carried_item));
	} else
		fw.Unsigned32(0);

	fw.Signed32(worker.m_needed_exp);
	fw.Signed32(worker.m_current_exp);
}

}
