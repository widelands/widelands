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

#include "widelands_map_bobdata_data_packet.h"

#include "bob.h"
#include "carrier.h"
#include "critter_bob.h"
#include "critter_bob_program.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "soldier.h"
#include "transport.h"
#include "tribe.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "worker_program.h"

#include "upcast.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

// Bob subtype versions
#define CRITTER_BOB_PACKET_VERSION 1
#define WORKER_BOB_PACKET_VERSION 1

// Worker subtype versions
#define SOLDIER_WORKER_BOB_PACKET_VERSION 3
#define CARRIER_WORKER_BOB_PACKET_VERSION 1


void Map_Bobdata_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Map_Map_Object_Loader * const ol)
throw
(_wexception)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/bob_data");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			Map   const &       map        = egbase->map();
			Extent        const extent     = map.extent();
			Player_Number const nr_players = map.get_nrplayers();

			for (;;) {
				uint32_t const serial = fr.Unsigned32();
				if (serial == 0xffffffff) //  FIXME test EndOfFile instead in the
					break;                 //  FIXME next packet version

				try {
					if (not ol->is_object_known(serial))
						throw wexception("not known");
					upcast(Bob, bob, ol->get_object_by_file_index(serial));
					if (not bob)
						throw wexception("not a bob");
					Bob::Descr const & bob_descr = bob->descr();

					if (Player_Number const read_owner = fr.Player_Number8()) {
						if (nr_players < read_owner)
							throw wexception
								("owner number is %u but there are only %u players",
								 read_owner, nr_players);
						if (Player * const owner = egbase->get_player(read_owner))
							bob->set_owner(owner);
						else
							throw wexception
								("owning player %u does not exist", read_owner);
					}

         // Basic initialisation
         bob->set_position(egbase, fr.Coords32(extent));

         // Look if we had an transfer
         bool have_transfer=fr.Unsigned8();

         Transfer* trans=0;
					if (have_transfer) {
            trans=bob->m_stack[0].transfer;
            assert(trans);
					}

         //         if (!have_transfer)
         //           bob->reset_tasks(static_cast<Game*>(egbase));
         //
         bob->m_actid=fr.Unsigned32();

         // Animation
					bob->m_anim =
						fr.Unsigned8() ? bob_descr.get_animation(fr.CString()) : 0;
         bob->m_animstart=fr.Signed32();

         // walking
					{
						Map_Object::WalkingDir const walking_dir =
							static_cast<Map_Object::WalkingDir>(fr.Signed32());
						if (6 < walking_dir)
							throw wexception
								("walking dir is %u but must be one of {0 (idle), 1 "
								 "(northeast), 2 (east), 3 (southeast), 4 "
								 "(southwest), 5 (west), 6 (northwest)}",
								 walking_dir);
						bob->m_walking = walking_dir;
					}
         bob->m_walkstart=fr.Signed32();
         bob->m_walkend=fr.Signed32();

         uint32_t oldstacksize=bob->m_stack.size();
         bob->m_stack.resize(fr.Unsigned16());
					for (uint32_t i = 0; i < bob->m_stack.size(); ++i) {
						Bob::State & state = bob->m_stack[i];
						try {

							{ //  Task
            Bob::Task* task;
								char const * const taskname = fr.CString();
								if      (not strcmp(taskname, "idle"))
									task = &Bob::taskIdle;
								else if (not strcmp(taskname, "movepath"))
									task = &Bob::taskMovepath;
								else if (not strcmp(taskname, "forcemove"))
									task = &Bob::taskForcemove;
								else if (not strcmp(taskname, "roam"))
									task = &Critter_Bob::taskRoam;
								else if (not strcmp(taskname, "program")) {
									if (dynamic_cast<Worker const *>(bob))
										task = &Worker::taskProgram;
									else if (dynamic_cast<Critter_Bob const *>(bob))
										task = &Critter_Bob::taskProgram;
									else
										throw;
								}
								else if (not strcmp(taskname, "transfer"))
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
								else if (not strcmp(taskname, "road"))
									task = &Carrier::taskRoad;
								else if (not strcmp(taskname, "transport"))
									task = &Carrier::taskTransport;
								else if (not strcmp(taskname, "moveToBattle"))
									task = &Soldier::taskMoveToBattle;
								else if (not strcmp(taskname, "moveHome"))
									task = &Soldier::taskMoveHome;
								else if (*taskname == '\0')
									continue; // Skip task
								else
									throw wexception("unknown task %s", taskname);

								state.task = task;
							}

							state.ivar1 = fr.Signed32();
							state.ivar2 = fr.Signed32();
							state.ivar3 = fr.Signed32();

							state.transfer = 0;

							if (int32_t const obj = fr.Unsigned32()) {
								assert(ol->is_object_known(obj));
								state.objvar1 = ol->get_object_by_file_index(obj);
							} else
								state.objvar1 = 0;
							state.svar1 = fr.CString();
							state.coords.x = fr.Signed32(); //  FIXME fix format to use
							state.coords.y = fr.Signed32(); //  FIXME FileRead::Coords32

							if (fr.Unsigned8()) {
								const uint32_t ans[6] = {
                  bob_descr.get_animation(fr.CString()),
                  bob_descr.get_animation(fr.CString()),
                  bob_descr.get_animation(fr.CString()),
                  bob_descr.get_animation(fr.CString()),
                  bob_descr.get_animation(fr.CString()),
                  bob_descr.get_animation(fr.CString())
								};
								state.diranims = new DirAnimations
									(ans[0], ans[1], ans[2], ans[3], ans[4], ans[5]);
							} else
								state.diranims = 0;

							uint32_t const pathsteps = fr.Unsigned16();
							if (i < oldstacksize)
								delete state.path;
							if (pathsteps) {
								try {
									Path * const path = new Path(fr.Coords32(extent));
									for (uint32_t step = 0; step < pathsteps; ++step) {
										Direction const direction = fr.Unsigned8();
										if (direction == 0 or 6 < direction)
											throw wexception
												("step %u: direction is %u but must be "
												 "one of {0 (idle), 1 (northeast), 2 "
												 "(east), 3 (southeast), 4 (southwest), 5 "
												 "(west), 6 (northwest)}",
												 step, direction);
										path->append(map, direction);
									}
									state.path = path;
								} catch (_wexception const & e) {
									throw wexception("reading path: %s", e.what());
								}
							} else
								state.path = 0;

							if (i < oldstacksize && !trans)
								delete state.transfer;

							state.transfer =
								state.task == &Worker::taskGowarehouse
								||
								state.task == &Worker::taskTransfer
								?
								trans : 0;

							{
								bool has_route = fr.Unsigned8();
								if (i < oldstacksize && state.route)
									if (!has_route) {
										delete state.route;
										state.route = 0; //  paranoia
									} else
										state.route->clear();

								if (has_route) {
									Route * const route =
										state.route ? state.route : new Route();
									route->load_pointers(*route->load(fr), *ol);
									state.route = route;
								} else
									state.route = 0;
							}

            // Now programm
							if (fr.Unsigned8()) {
               std::string progname=fr.CString();
								if      (upcast(Worker      const, wor,  bob))
									state.program = wor->descr().get_program(progname);
								else if (upcast(Critter_Bob const, cri, bob))
									state.program = cri->descr().get_program(progname);
								else
									throw;
							} else
								state.program = 0;

						} catch (_wexception const & e) {
							throw wexception("reading state %u: %s", i, e.what());
						}
					}

         // Rest of bob stuff
         bob->m_stack_dirty=fr.Unsigned8();
         bob->m_sched_init_task=fr.Unsigned8();
         bob->m_signal=fr.CString();

			if      (upcast(Critter_Bob, critter_bob, bob))
				read_critter_bob(&fr, egbase, ol, critter_bob);
			else if (upcast(Worker,      worker,      bob))
				read_worker_bob(&fr, egbase, ol, worker);
			else
				assert(false);

         ol->mark_object_as_loaded(bob);
				} catch (_wexception const & e) {
					throw wexception("reading object %u: %s", serial, e.what());
				}
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("reading bobdata: %s", e.what());
	}
}

void Map_Bobdata_Data_Packet::read_critter_bob
	(FileRead * fr, Editor_Game_Base *, Map_Map_Object_Loader *, Critter_Bob *)
{
	const uint16_t packet_version = fr->Unsigned16();
	if (packet_version == CRITTER_BOB_PACKET_VERSION) {
      // No data for critter bob currently
	} else
		throw wexception
			("Unknown version %u in Critter Bob Subpacket!", packet_version);
}

void Map_Bobdata_Data_Packet::read_worker_bob
	(FileRead              * fr,
	 Editor_Game_Base      * egbase,
	 Map_Map_Object_Loader * ol,
	 Worker                * worker)
{
	const uint16_t packet_version = fr->Unsigned16();
	if (packet_version == WORKER_BOB_PACKET_VERSION) {
		if (upcast(Soldier, soldier, worker)) {
			const uint16_t soldier_worker_bob_packet_version = fr->Unsigned16();
			if
				(2 <= soldier_worker_bob_packet_version
				 and
				 soldier_worker_bob_packet_version
				 <=
				 SOLDIER_WORKER_BOB_PACKET_VERSION)
			{
				const uint32_t min_hp = soldier->descr().get_min_hp();
				assert(min_hp);
				{
					//  Soldiers created by old versions of Widelands have wrong
					//  values for m_hp_max and m_hp_current; they
					//  were soldier->descr().get_min_hp() less than they should be,
					//  see bug #1687368.
					const uint32_t broken_hp_compensation =
						soldier_worker_bob_packet_version < 3 ? min_hp : 0;

					soldier->m_hp_current =
						broken_hp_compensation + fr->Unsigned32();
					soldier->m_hp_max = broken_hp_compensation + fr->Unsigned32();
				}
				if (soldier->m_hp_max < min_hp)
					throw wexception
						("Map_Bobdata_Data_Packet::read_worker_bob: "
						 "binary/bob_data:%u: soldier %p (serial %u): m_hp_max = %u "
						 "but must be at least %u",
					 fr->GetPos() - 4, worker, worker->get_serial(),
					 soldier->m_hp_max, min_hp);
               soldier->m_min_attack=fr->Unsigned32();
               soldier->m_max_attack=fr->Unsigned32();
               soldier->m_defense=fr->Unsigned32();
               soldier->m_evade=fr->Unsigned32();
               soldier->m_hp_level=fr->Unsigned32();
               soldier->m_attack_level=fr->Unsigned32();
               soldier->m_defense_level=fr->Unsigned32();
               soldier->m_evade_level=fr->Unsigned32();
               soldier->m_marked=fr->Unsigned8();
			} else
				throw wexception
					("Map_Bobdata_Data_Packet::read_worker_bob: "
					 "binary/bob_data:%u: soldier %p (serial %u): unknown soldier "
					 "worker bob packet version %u",
					 fr->GetPos() - 2,
					 worker, worker->get_serial(),
					 soldier_worker_bob_packet_version);
		} else if (upcast(Carrier, carrier, worker)) {
			const uint16_t carrier_worker_bob_packet_version = fr->Unsigned16();
			if
				(carrier_worker_bob_packet_version
				 ==
				 CARRIER_WORKER_BOB_PACKET_VERSION)
				carrier->m_acked_ware = fr->Signed32();
			else
				throw wexception
					("Map_Bobdata_Data_Packet::read_worker_bob:"
					 "binary/bob_data:%u: carrier %p (serial %u): unknown carrier "
					 "worker bob packet version %u",
					 fr->GetPos() - 2,
					 worker, worker->get_serial(),
					 carrier_worker_bob_packet_version);
		}

      // location
		if (uint32_t const reg = fr->Unsigned32()) {
         assert(ol->is_object_known(reg));
         worker->set_location(static_cast<PlayerImmovable*>(ol->get_object_by_file_index(reg)));
         assert(worker->m_location.get(egbase));
		} else
         worker->m_location=0;


      // Carried item
		if (uint32_t const reg = fr->Unsigned32()) {
         assert(ol->is_object_known(reg));
         worker->m_carried_item=ol->get_object_by_file_index(reg);
		} else
         worker->m_carried_item=0;

      // Skip supply

      worker->m_needed_exp=fr->Signed32();
      worker->m_current_exp=fr->Signed32();

      Economy* eco=0;
		if (worker->m_location.get(egbase))
         eco=static_cast<PlayerImmovable*>(worker->m_location.get(egbase))->get_economy();

      worker->set_economy(eco);
		if (worker->m_carried_item.get(egbase))
         static_cast<WareInstance*>(worker->m_carried_item.get(egbase))->set_economy(eco);


	} else
		throw wexception
			("Unknown version %i in Worker Bob Subpacket!", packet_version);
}


void Map_Bobdata_Data_Packet::Write
(FileSystem & fs, Editor_Game_Base * egbase, Map_Map_Object_Saver * const os)
throw (_wexception)
{
   FileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   Map* map=egbase->get_map();
	for (uint16_t y = 0; y < map->get_height(); ++y) {
		for (uint16_t x = 0; x < map->get_width(); ++x) {

         std::vector<Bob*> bobarr;

		map->find_bobs(Area<FCoords>(map->get_fcoords(Coords(x, y)), 0), &bobarr);

			for (uint32_t i = 0; i < bobarr.size(); ++i) {
            Bob* bob=bobarr[i];
            assert(os->is_object_known(bob));
            uint32_t reg=os->get_object_file_index(bob);

            fw.Unsigned32(reg);
            // BOB STUFF

				fw.Unsigned8(bob->m_owner ? bob->m_owner->get_player_number() : 0);

            // m_position
				fw.Coords32(bob->m_position);
            // FIELD can't be saved

            // m_linknext, linkpprev are handled automatically

            // Are we currently transfering?
				fw.Unsigned8(bob->m_stack.size() && bob->m_stack[0].transfer);

            fw.Unsigned32(bob->m_actid);

            // Animation
				//  FIXME Just write the string without the 1 first:
				//  FIXME   fw.CString(bob->m_anim ? bob->descr().get_animation_name(bob->m_anim).c_str() : "");
				//  FIXME When reading, the empty string should mean no animation.
				if (bob->m_anim) {
               fw.Unsigned8(1);
               fw.CString(bob->descr().get_animation_name(bob->m_anim).c_str());
				} else
               fw.Unsigned8(0);
            fw.Signed32(bob->m_animstart);

            // WalkingDirection
            fw.Signed32(bob->m_walking);

            // WalkingStart
            fw.Signed32(bob->m_walkstart);

            // WalkEnd
            fw.Signed32(bob->m_walkend);

            // Nr of States
            fw.Unsigned16(bob->m_stack.size());
				for (uint32_t index = 0; index < bob->m_stack.size(); ++index) {
               Bob::State* s=&bob->m_stack[index];

               // Write name, enough to reconstruct the
               // Task structure
               fw.CString(s->task->name);

               fw.Signed32(s->ivar1);
               fw.Signed32(s->ivar2);
               fw.Signed32(s->ivar3);

					if (Map_Object * const obj = s->objvar1.get(egbase)) {
                  assert(os->is_object_known(obj));
                  fw.Unsigned32(os->get_object_file_index(obj));
					} else
                  fw.Unsigned32(0);

               fw.CString(s->svar1.c_str());

               // Coords
					fw.Signed32(s->coords.x); //  FIXME fix format to use
					fw.Signed32(s->coords.y); //  FIXME FileWrite::Coords32

					if (s->diranims) {
                  fw.Unsigned8(1);
                  fw.CString(bob->descr().get_animation_name(s->diranims->get_animation(1)).c_str());
                  fw.CString(bob->descr().get_animation_name(s->diranims->get_animation(2)).c_str());
                  fw.CString(bob->descr().get_animation_name(s->diranims->get_animation(3)).c_str());
                  fw.CString(bob->descr().get_animation_name(s->diranims->get_animation(4)).c_str());
                  fw.CString(bob->descr().get_animation_name(s->diranims->get_animation(5)).c_str());
                  fw.CString(bob->descr().get_animation_name(s->diranims->get_animation(6)).c_str());
					} else
                  fw.Unsigned8(0);

               // Path
					if (const Path * const path = s->path) {
						const Path::Step_Vector::size_type nr_steps =
							s->path->get_nsteps();
						fw.Unsigned16(nr_steps);
                  fw.Coords32(path->get_start());
						for
							(Path::Step_Vector::size_type idx = 0;
							 idx < nr_steps;
							 ++idx)
							fw.Unsigned8((*path)[idx]);
					} else
                  fw.Unsigned16(0);

               // Route
					if (s->route) {
                  fw.Unsigned8(1);
						s->route->save(fw, egbase, os);
					} else
                  fw.Unsigned8(0);

               // Programm
					if (s->program) {
                  fw.Unsigned8(1);
                  fw.CString(s->program->get_name().c_str());
					} else
                  fw.Unsigned8(0);


				}

            fw.Unsigned8(bob->m_stack_dirty);
            fw.Unsigned8(bob->m_sched_init_task);
            fw.CString(bob->m_signal.c_str());

				if      (upcast(Critter_Bob, critter_bob, bob))
					write_critter_bob(&fw, egbase, os, critter_bob);
				else if (upcast(Worker,      worker,      bob))
					write_worker_bob (&fw, egbase, os, worker);
				else
					assert(false);

            os->mark_object_as_saved(bob);
			}

		}
	}
   fw.Unsigned32(0xffffffff); // No more bobs

   fw.Write(fs, "binary/bob_data");
   // DONE
}

void Map_Bobdata_Data_Packet::write_critter_bob
(FileWrite * fw, Editor_Game_Base *, Map_Map_Object_Saver *, Critter_Bob *)
{fw->Unsigned16(CRITTER_BOB_PACKET_VERSION);}

void Map_Bobdata_Data_Packet::write_worker_bob
	(FileWrite            * fw,
	 Editor_Game_Base     * egbase,
	 Map_Map_Object_Saver * os,
	 Worker               * worker)
{
   fw->Unsigned16(WORKER_BOB_PACKET_VERSION);

	switch (worker->get_worker_type()) {
	case Worker_Descr::NORMAL:
		break;
	case Worker_Descr::SOLDIER: {
         fw->Unsigned16(SOLDIER_WORKER_BOB_PACKET_VERSION);
         Soldier* soldier=static_cast<Soldier*>(worker);

         fw->Unsigned32(soldier->m_hp_current);
         fw->Unsigned32(soldier->m_hp_max);
         fw->Unsigned32(soldier->m_min_attack);
         fw->Unsigned32(soldier->m_max_attack);
         fw->Unsigned32(soldier->m_defense);
         fw->Unsigned32(soldier->m_evade);
         fw->Unsigned32(soldier->m_hp_level);
         fw->Unsigned32(soldier->m_attack_level);
         fw->Unsigned32(soldier->m_defense_level);
         fw->Unsigned32(soldier->m_evade_level);
         fw->Unsigned8 (soldier->m_marked);
	}
      break;

	case Worker_Descr::CARRIER: {
         fw->Unsigned16(CARRIER_WORKER_BOB_PACKET_VERSION);
         Carrier* c=static_cast<Carrier*>(worker);

         fw->Signed32(c->m_acked_ware);
	}
      break;
	default:
		throw wexception
			("Unknown Worker in Map_Bobdata_Data_Packet::write_worker_bob()");
	}

   // location
	if (Map_Object * const loca = worker->m_location.get(egbase)) {
      assert(os->is_object_known(loca));
      fw->Unsigned32(os->get_object_file_index(loca));
	} else
      fw->Unsigned32(0);

   // Economy is not our beer

   // Carried item
	if (Map_Object * const carried_item = worker->m_carried_item.get(egbase)) {
      assert(os->is_object_known(carried_item));
      fw->Unsigned32(os->get_object_file_index(carried_item));
	} else
      fw->Unsigned32(0);

   //This is not needed
   // Write if a idle supply is to be created
   //if (worker->m_supply)
   //   fw->Unsigned8(1);
   //else
   //   fw->Unsigned8(0);
   //

   fw->Signed32(worker->m_needed_exp);
   fw->Signed32(worker->m_current_exp);
}

};
