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

#include "widelands_map_buildingdata_data_packet.h"

#include "constructionsite.h"
#include "editor.h"
#include "editorinteractive.h"
#include "editor_game_base.h"
#include "error.h"
#include "fileread.h"
#include "filewrite.h"
#include "game.h"
#include "map.h"
#include "militarysite.h"
#include "player.h"
#include "productionsite.h"
#include "production_program.h"
#include "soldier.h"
#include "trainingsite.h"
#include "transport.h"
#include "tribe.h"
#include "warehouse.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "worker.h"

#include <map>


// Versions
#define CURRENT_PACKET_VERSION 1

// Subversions
#define CURRENT_CONSTRUCTIONSITE_PACKET_VERSION 1
#define CURRENT_WAREHOUSE_PACKET_VERSION        1
#define CURRENT_MILITARYSITE_PACKET_VERSION     2
#define CURRENT_PRODUCTIONSITE_PACKET_VERSION   1
#define CURRENT_TRAININGSITE_PACKET_VERSION     1

/*
 * Destructor
 */
Widelands_Map_Buildingdata_Data_Packet::~Widelands_Map_Buildingdata_Data_Packet() {
}

/*
 * Read Function
 */
void Widelands_Map_Buildingdata_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (not skip) {

		FileRead fr;
		try {fr.Open(fs, "binary/building_data");} catch (...) {return;}

		const Uint16 packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			for (;;) {
         uint ser=fr.Unsigned32();
				if (ser == 0xffffffff) break; // Last building

         assert(ol->is_object_known(ser));
         assert(ol->get_object_by_file_index(ser)->get_type()==Map_Object::BUILDING);
         Building* building=static_cast<Building*>(ol->get_object_by_file_index(ser));

         // Animation
				building->m_anim = fr.Unsigned8() ?
					building->descr().get_animation(fr.CString()) : 0;
         building->m_animstart=fr.Unsigned32();

         building->m_leave_queue.resize(fr.Unsigned16());
         for (uint i=0; i<building->m_leave_queue.size(); i++) {
            ser=fr.Unsigned32();
					if (ser) {
               assert(ol->is_object_known(ser));
               building->m_leave_queue[i]=ol->get_object_by_file_index(ser);
				} else
               building->m_leave_queue[i]=0;
			}
         building->m_leave_time=fr.Unsigned32();
         ser=fr.Unsigned32();
				if (ser) {
            assert(ol->is_object_known(ser));
            building->m_leave_allow=ol->get_object_by_file_index(ser);
			} else
            building->m_leave_allow=0;
         building->m_stop=fr.Unsigned8();

         // Set economy now, some stuff below will count on this
         building->set_economy(building->m_flag->get_economy());

			if
				(ConstructionSite * const constructionsite =
				 dynamic_cast<ConstructionSite *>(building))
				read_constructionsite(*constructionsite, fr, egbase, ol);
			else if
				(Warehouse * const warehouse =
				 dynamic_cast<Warehouse *>(building))
				read_warehouse(*warehouse, fr, egbase, ol);
			else if
				(ProductionSite * const productionsite =
				 dynamic_cast<ProductionSite *>(building))
			{
				if
					(MilitarySite * const militarysite =
					 dynamic_cast<MilitarySite *>(productionsite))
					read_militarysite(*militarysite, fr, egbase, ol);
				else if
					(TrainingSite * const trainingsite =
					 dynamic_cast<TrainingSite *>(productionsite))
					read_trainingsite(*trainingsite, fr, egbase, ol);
				else read_productionsite(*productionsite, fr, egbase, ol);
			} else {
				assert(false);
				//  type of building is not one of (or derived from)
				//  {ConstructionSite, Warehouse, ProductionSite}
			}


         ol->mark_object_as_loaded(building);
		}
		} else
			throw wexception
				("Unknown version %u in Widelands_Map_Buildingdata_Data_Packet!",
				 packet_version);
	}
}

void Widelands_Map_Buildingdata_Data_Packet::read_constructionsite
(ConstructionSite & constructionsite,
 FileRead & fr,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Loader * const ol)
{
	const Uint16 packet_version = fr.Unsigned16();
	if (packet_version == CURRENT_CONSTRUCTIONSITE_PACKET_VERSION) {
      constructionsite.m_building=constructionsite.get_owner()->tribe().get_building_descr(constructionsite.get_owner()->tribe().get_safe_building_index(fr.CString()));
      bool prevb=fr.Unsigned8();
      if (prevb) {
         constructionsite.m_prev_building=constructionsite.get_owner()->tribe().get_building_descr(constructionsite.get_owner()->tribe().get_safe_building_index(fr.CString()));
		} else
         constructionsite.m_prev_building=0;

      // Builder request
         delete constructionsite.m_builder_request;
      bool request=fr.Unsigned8();
		if (request) {
         constructionsite.m_builder_request = new Request
				(&constructionsite,
				 0,
				 ConstructionSite::request_builder_callback,
				 &constructionsite,
				 Request::WORKER);
			constructionsite.m_builder_request->Read(&fr, egbase, ol);
		} else
         constructionsite.m_builder_request=0;

      // Builder
      uint reg=fr.Unsigned32();
		if (reg) {
         assert(ol->is_object_known(reg));
         constructionsite.m_builder=static_cast<Worker*>(ol->get_object_by_file_index(reg));
		} else
         constructionsite.m_builder=0;

      // Wares queues
      uint size=fr.Unsigned16();
      assert(constructionsite.m_wares.size()>=size);
      for (uint i=size; i<constructionsite.m_wares.size(); i++) {
			if (dynamic_cast<Game *>(egbase))
				constructionsite.m_wares[i]->cleanup();
         delete constructionsite.m_wares[i];
		}
      constructionsite.m_wares.resize(size);
      for (uint i=0; i<constructionsite.m_wares.size(); i++) {
         constructionsite.m_wares[i]->Read(&fr, egbase, ol);
		}

      constructionsite.m_fetchfromflag=fr.Signed32();

      constructionsite.m_working=fr.Unsigned8();
      constructionsite.m_work_steptime=fr.Unsigned32();
      constructionsite.m_work_completed=fr.Unsigned32();
      constructionsite.m_work_steps=fr.Unsigned32();

	} else
		throw wexception
			("Unknown Constructionsite-Version %u in "
			 "Widelands_Map_Buildingdata_Data_Packet!",
			 packet_version);
}

void Widelands_Map_Buildingdata_Data_Packet::read_warehouse
(Warehouse & warehouse,
 FileRead & fr,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Loader * const ol)
{
	const Uint16 packet_version = fr.Unsigned16();
	if (packet_version == CURRENT_WAREHOUSE_PACKET_VERSION) {
		log("Reading warehouse stuff for %p\n", &warehouse);
      // Supply
		const Tribe_Descr & tribe = warehouse.get_owner()->tribe();
		while (fr.Unsigned8()) {
			const int id = tribe.get_safe_ware_index(fr.CString());
         warehouse.remove_wares(id, warehouse.m_supply->stock_wares(id));
			warehouse.insert_wares(id, fr.Unsigned16());
		}
		while (fr.Unsigned8()) {
			const int id = tribe.get_safe_worker_index(fr.CString());
         warehouse.remove_workers(id, warehouse.m_supply->stock_workers(id));
			warehouse.insert_workers(id, fr.Unsigned16());
		}

      // Request
      for (uint i=0; i<warehouse.m_requests.size(); i++)
         delete warehouse.m_requests[i];
      warehouse.m_requests.resize(fr.Unsigned16());
      for (uint i=0; i<warehouse.m_requests.size(); i++) {
			Request & req = *new Request
				(&warehouse,
				 0,
				 Warehouse::idle_request_cb,
				 &warehouse,
				 Request::WORKER);
			req.Read(&fr, egbase, ol);
			warehouse.m_requests[i] = &req;
		}

      // Incorporated Workers
		while (warehouse.m_incorporated_workers.size()) {assert(false); //  FIXME why this loop?
         std::vector<Object_Ptr>::iterator i=warehouse.m_incorporated_workers.begin();
         static_cast<Worker*>(i->get(egbase))->remove(egbase);
         warehouse.m_incorporated_workers.erase(i);
		}
      warehouse.m_incorporated_workers.resize(0);
		Player & player = warehouse.owner();
      int nrworkers=fr.Unsigned16();
      for (int i=0; i<nrworkers; i++) {
         uint id=fr.Unsigned32();
         std::string name=fr.CString();
         assert(ol->is_object_known(id));
         // Worker might not yet be loaded so that get ware won't work
         // but make sure that such a worker exists in tribe
         if (warehouse.get_owner()->tribe().get_worker_index(name.c_str())==-1)
            throw wexception("Unknown worker %s in incorporated workers in Widelands_Map_Buildingdata_Data_Packet!\n", name.c_str());
         Worker* w=static_cast<Worker*>(ol->get_object_by_file_index(id));
         warehouse.sort_worker_in(egbase, name, w);
		}
		if (nrworkers) {
			Map & map = egbase->map();
			player.see_area
				(Area<FCoords>
				 (map.get_fcoords(warehouse.get_position()),
				  warehouse.vision_range()));
		}

      // Carrier spawn
      warehouse.m_next_carrier_spawn=fr.Unsigned32();

      log("Read warehouse stuff for %p\n", &warehouse);
	} else
		throw wexception
			("Unknown Warehouse-Version %u "
			 "in Widelands_Map_Buildingdata_Data_Packet!",
			 packet_version);
}

void Widelands_Map_Buildingdata_Data_Packet::read_militarysite
(MilitarySite & militarysite,
 FileRead & fr,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Loader * const ol)
{
      // read the version
      uint version=fr.Unsigned16();

	if (version == CURRENT_MILITARYSITE_PACKET_VERSION) {
         // Read productionsite
		read_productionsite(militarysite, fr, egbase, ol);

         // Request
			uint nr_requests=fr.Unsigned16();

			for (uint i=0; i<militarysite.m_soldier_requests.size(); i++)
	         delete militarysite.m_soldier_requests[i];
		militarysite.m_soldier_requests.resize(nr_requests);

			for (uint i=0; i<nr_requests; i++) {
				Request & req = *new Request
					(&militarysite,
					 0,
					 MilitarySite::request_soldier_callback,
					 &militarysite,
					 Request::SOLDIER);
				req.Read(&fr, egbase, ol);
				militarysite.m_soldier_requests[i] = &req;
			}

         // Soldier
         uint nr_soldiers = fr.Unsigned16();
         assert(!militarysite.m_soldiers.size());
         militarysite.m_soldiers.resize(nr_soldiers);
         for (uint i=0; i<nr_soldiers; i++) {
            uint reg = fr.Unsigned32();
            assert(ol->is_object_known(reg));
            militarysite.m_soldiers[i] = static_cast<Soldier*>(ol->get_object_by_file_index(reg));
			}

         // did conquer
         militarysite.m_didconquer = fr.Unsigned8();

			// capacity (modified by user)
			militarysite.m_capacity = fr.Unsigned8();
         // DONE
		} else
         throw wexception("Unknown MilitarySite-Version %i in Widelands_Map_Buildingdata_Data_Packet!\n", version);
}

void Widelands_Map_Buildingdata_Data_Packet::read_productionsite
(ProductionSite & productionsite,
 FileRead & fr,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Loader * const ol)
{
	const Uint16 packet_version = fr.Unsigned16();
	if (packet_version == CURRENT_PACKET_VERSION) {
      // Requests
      uint nr_requests=fr.Unsigned16();
      for (uint i=0; i<productionsite.m_worker_requests.size(); i++)
         delete productionsite.m_worker_requests[i];
      productionsite.m_worker_requests.resize(nr_requests);
      for (uint i=0; i<nr_requests; i++) {
			Request & req = *new Request
				(&productionsite,
				 0,
				 ProductionSite::request_worker_callback,
				 &productionsite,
				 Request::WORKER);
			req.Read(&fr, egbase, ol);
			productionsite.m_worker_requests[i] = &req;
		}

      // Workers
      uint nr_workers = fr.Unsigned16();
      assert(!productionsite.m_workers.size());
      productionsite.m_workers.resize(nr_workers);
      for (uint i=0; i<nr_workers; i++) {
         uint reg = fr.Unsigned32();
         assert(ol->is_object_known(reg));
         productionsite.m_workers[i] = static_cast<Worker*>(ol->get_object_by_file_index(reg));
		}

      // Items from flags
      productionsite.m_fetchfromflag = fr.Signed32();

      // State
      uint nr_progs = fr.Unsigned16();
      productionsite.m_program.resize(nr_progs);
      for (uint i=0; i<nr_progs; i++) {
         std::string prog = fr.CString();
         productionsite.m_program[i].program = productionsite.descr().get_program(prog.c_str());
         productionsite.m_program[i].ip = fr.Signed32();
         productionsite.m_program[i].phase = fr.Signed32();
         productionsite.m_program[i].flags = fr.Unsigned32();
		}
      productionsite.m_program_timer = fr.Unsigned8();
      productionsite.m_program_time = fr.Signed32();

      // Wares
      uint nr_queues = fr.Unsigned16();
		if (nr_queues != productionsite.m_input_queues.size())
         throw wexception("Productionsite has wrong number of input queues!\n");
      for (uint i=0; i<productionsite.m_input_queues.size(); i++)
         productionsite.m_input_queues[i]->Read(&fr, egbase, ol);

      // Statistics
      uint stats_size = fr.Unsigned16();
      productionsite.m_statistics.resize(stats_size);
      for (uint i=0; i<productionsite.m_statistics.size(); i++)
         productionsite.m_statistics[i] = fr.Unsigned8();
      productionsite.m_statistics_changed = fr.Unsigned8();
      memcpy(productionsite.m_statistics_buf, fr.Data(sizeof(productionsite.m_statistics_buf)), sizeof(productionsite.m_statistics_buf));
	} else
		throw wexception
			("Unknown ProductionSite-Version %u in "
			 "Widelands_Map_Buildingdata_Data_Packet!\n",
			 packet_version);
}

void Widelands_Map_Buildingdata_Data_Packet::read_trainingsite
(TrainingSite & trainingsite,
 FileRead & fr,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Loader * const ol)
{
	const Uint16 trainingsite_packet_version = fr.Unsigned16();
	if (trainingsite_packet_version == CURRENT_TRAININGSITE_PACKET_VERSION) {
         // Read productionsite
		read_productionsite(trainingsite, fr, egbase, ol);

         // Requests
		uint nr_requests=fr.Unsigned16();

		for (uint i=0; i<trainingsite.m_soldier_requests.size(); i++)
			delete trainingsite.m_soldier_requests[i];
		trainingsite.m_soldier_requests.resize(nr_requests);

		for (uint i=0; i<nr_requests; i++) {
			Request & req = *new Request
				(&trainingsite,
				 0,
				 TrainingSite::request_soldier_callback,
				 &trainingsite,
				 Request::SOLDIER);
			req.Read(&fr, egbase, ol);
			trainingsite.m_soldier_requests[i] = &req;
		}

         // Soldiers
		uint nr_soldiers = fr.Unsigned16();
		assert(!trainingsite.m_soldiers.size());
		trainingsite.m_soldiers.resize(nr_soldiers);
		for (uint i=0; i<nr_soldiers; i++) {
			uint reg = fr.Unsigned32();
			assert(ol->is_object_known(reg));
			trainingsite.m_soldiers[i] = static_cast<Soldier*>(ol->get_object_by_file_index(reg));
		}

		// Don't save m_list_upgrades (remake at load)

			//Building heros ?
		trainingsite.m_build_heros = fr.Unsigned8();

			// Priority upgrades
		trainingsite.m_pri_hp = fr.Unsigned16();
		trainingsite.m_pri_attack = fr.Unsigned16();
		trainingsite.m_pri_defense = fr.Unsigned16();
		trainingsite.m_pri_evade = fr.Unsigned16();

			// Priority modificators
		trainingsite.m_pri_hp_mod = fr.Unsigned16();
		trainingsite.m_pri_attack_mod = fr.Unsigned16();
		trainingsite.m_pri_defense_mod = fr.Unsigned16();
		trainingsite.m_pri_evade_mod = fr.Unsigned16();

		// capacity (modified by user)
		trainingsite.m_capacity = fr.Unsigned8();

		// Need to read the m_prog_name as string !!
		std::string prog = fr.CString();
		trainingsite.m_prog_name = prog;

		//m_total_soldiers is just a convenience variable and not saved, recalculate it
		trainingsite.m_total_soldiers=trainingsite.m_soldiers.size()+trainingsite.m_soldier_requests.size();
	} else
		throw wexception
			("Unknown TrainingSite-Version %u in "
			 "Widelands_Map_Buildingdata_Data_Packet!",
			 trainingsite_packet_version);
}





/*
 * Write Function
 */
void Widelands_Map_Buildingdata_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
throw (_wexception)
{
   FileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   // Walk the map again
	Map & map = egbase->map();
	const uint mapwidth = map.get_width();
	const Map::Index max_index = map.max_index();
	for (Map::Index i = 0; i < max_index; ++i) {
		const Building * const building =
			dynamic_cast<const Building *>(map[i].get_immovable());

		if (building) {
            assert(os->is_object_known(building));

			if (Map::get_index(building->get_position(), mapwidth) != i) {
               // This is not this buildings main position
               continue;
				}

			fw.Unsigned32(os->get_object_file_index(building));

            // Player immovable owner is already in existance packet

            // Write the general stuff
			if (building->m_anim) {
               fw.Unsigned8(1);
               fw.CString(building->descr().get_animation_name(building->m_anim).c_str());
				} else
               fw.Unsigned8(0);

			fw.Unsigned32(building->m_animstart);

            // Leave queue time
            fw.Unsigned16(building->m_leave_queue.size());
            for (uint idx=0; idx<building->m_leave_queue.size(); idx++) {
               assert(os->is_object_known(building->m_leave_queue[idx].get(egbase)));
               fw.Unsigned32(os->get_object_file_index(building->m_leave_queue[idx].get(egbase)));
				}
            fw.Unsigned32(building->m_leave_time);
			if (building->m_leave_allow.get(egbase)) {
               assert(os->is_object_known(building->m_leave_allow.get(egbase)));
               fw.Unsigned32(os->get_object_file_index(building->m_leave_allow.get(egbase)));
				} else
               fw.Unsigned32(0);
            fw.Unsigned8(building->m_stop);

			if
				(const ConstructionSite * const constructionsite =
				 dynamic_cast<const ConstructionSite *>(building))
				write_constructionsite(*constructionsite, fw, egbase, os);
			else if
				(const Warehouse * const warehouse =
				 dynamic_cast<const Warehouse *>(building))
				write_warehouse(*warehouse, fw, egbase, os);
			else if
				(const ProductionSite * const productionsite =
				 dynamic_cast<const ProductionSite *>(building))
			{
				if
					(const MilitarySite * const militarysite =
					 dynamic_cast<const MilitarySite *>(productionsite))
						write_militarysite(*militarysite, fw, egbase, os);
				else if
					(const TrainingSite * const trainingsite =
					 dynamic_cast<const TrainingSite *>(productionsite))
					write_trainingsite(*trainingsite, fw, egbase, os);
				else write_productionsite(*productionsite, fw, egbase, os);
			} else {
				assert(false);
				//  type of building is not one of (or derived from)
				//  {ConstructionSite, Warehouse, ProductionSite}
				}

            os->mark_object_as_saved(building);
			}

	}

   fw.Unsigned32(0xffffffff); // End of buildings

   fw.Write(fs, "binary/building_data");
   // DONE
}

/*
 * write for constructionsite
 */
void Widelands_Map_Buildingdata_Data_Packet::write_constructionsite
(const ConstructionSite & constructionsite,
 FileWrite & fw,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
{

   // First, write current version
   fw.Unsigned16(CURRENT_CONSTRUCTIONSITE_PACKET_VERSION);

   // Describtions
   fw.CString(constructionsite.m_building->name().c_str());
	if (constructionsite.m_prev_building) {
      fw.Unsigned8(1);
      fw.CString(constructionsite.m_prev_building->name().c_str());
	} else
      fw.Unsigned8(0);

   // builder request
	if (constructionsite.m_builder_request) {
      fw.Unsigned8(1);
      constructionsite.m_builder_request->Write(&fw, egbase, os);
	} else
      fw.Unsigned8(0);

	// builder
	const Worker* builder = constructionsite.m_builder.get(egbase);
	if (builder) {
		assert(os->is_object_known(builder));
		fw.Unsigned32(os->get_object_file_index(builder));
	} else
		fw.Unsigned32(0);

   // ware queues
   fw.Unsigned16(constructionsite.m_wares.size());
   for (uint i=0; i<constructionsite.m_wares.size(); i++) {
      constructionsite.m_wares[i]->Write(&fw, egbase, os);
	}

   fw.Signed32(constructionsite.m_fetchfromflag);

   fw.Unsigned8(constructionsite.m_working);
   fw.Unsigned32(constructionsite.m_work_steptime);
   fw.Unsigned32(constructionsite.m_work_completed);
   fw.Unsigned32(constructionsite.m_work_steps);
}

/*
 * write for warehouse
 */
void Widelands_Map_Buildingdata_Data_Packet::write_warehouse
(const Warehouse & warehouse,
 FileWrite & fw,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
{
   fw.Unsigned16(CURRENT_WAREHOUSE_PACKET_VERSION);

   // Supply
   const WareList& wares=warehouse.m_supply->get_wares();
	for (size_t i = 0; i < wares.get_nrwareids(); ++i) {
      fw.Unsigned8(1);
      fw.CString(warehouse.get_owner()->tribe().get_ware_descr(i)->name().c_str());
      fw.Unsigned16(wares.stock(i));
	}
   fw.Unsigned8(0);
   const WareList& workers=warehouse.m_supply->get_workers();
	for (size_t i = 0; i < workers.get_nrwareids(); ++i) {
      fw.Unsigned8(1);
      fw.CString(warehouse.get_owner()->tribe().get_worker_descr(i)->name().c_str());
      fw.Unsigned16(workers.stock(i));
	}
   fw.Unsigned8(0);

   // Request
   fw.Unsigned16(warehouse.m_requests.size());
   for (uint i=0; i<warehouse.m_requests.size(); i++) {
      warehouse.m_requests[i]->Write(&fw, egbase, os);
	}

   // Incorporated workers, write sorted after file-serial
   fw.Unsigned16(warehouse.m_incorporated_workers.size());
	std::map<uint, const Worker *> workermap;
	const std::vector<Object_Ptr>::const_iterator iw_end =
		warehouse.m_incorporated_workers.end();
	for
		(std::vector<Object_Ptr>::const_iterator it =
		 warehouse.m_incorporated_workers.begin();
		 it != iw_end;
		 ++it)
	{
		assert(os->is_object_known(it->get(egbase)));
		workermap.insert
			(std::pair<uint, const Worker *>
			 (os->get_object_file_index(it->get(egbase)),
			  static_cast<const Worker *>(it->get(egbase))));
	}

	for
		(std::map<uint, const Worker *>::const_iterator it = workermap.begin();
		 it != workermap.end();
		++it)
	{
		assert(os->is_object_known(it->second));
		fw.Unsigned32(os->get_object_file_index(it->second));
		fw.CString(it->second->name().c_str());
	}

   // Carrier spawn
   fw.Unsigned32(warehouse.m_next_carrier_spawn);
}

/*
 * write for militarysite
 */
void Widelands_Map_Buildingdata_Data_Packet::write_militarysite
(const MilitarySite & militarysite,
 FileWrite & fw,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
{
   // Write the version
   fw.Unsigned16(CURRENT_MILITARYSITE_PACKET_VERSION);

   // Write for productionsite
   write_productionsite(militarysite, fw, egbase, os);

   // Request
   fw.Unsigned16(militarysite.m_soldier_requests.size());
   for (uint i=0; i<militarysite.m_soldier_requests.size(); i++)
      militarysite.m_soldier_requests[i]->Write(&fw, egbase, os);


   // Soldier
   fw.Unsigned16(militarysite.m_soldiers.size());
   for (uint i=0; i<militarysite.m_soldiers.size(); i++) {
      assert(os->is_object_known(militarysite.m_soldiers[i]));
      fw.Unsigned32(os->get_object_file_index(militarysite.m_soldiers[i]));
	}

   // did conquer
	fw.Unsigned8(militarysite.m_didconquer);

	// capacity
	fw.Unsigned8(militarysite.m_capacity);
}

/*
 * write for productionsite
 */
void Widelands_Map_Buildingdata_Data_Packet::write_productionsite
(const ProductionSite & productionsite,
 FileWrite & fw,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
{
   // Write the version
   fw.Unsigned16(CURRENT_PRODUCTIONSITE_PACKET_VERSION);


   // Requests
   fw.Unsigned16(productionsite.m_worker_requests.size());
   for (uint i=0; i<productionsite.m_worker_requests.size(); i++)
      productionsite.m_worker_requests[i]->Write(&fw, egbase, os);

   // Workers
   fw.Unsigned16(productionsite.m_workers.size());
   for (uint i=0; i<productionsite.m_workers.size(); i++) {
      assert(os->is_object_known(productionsite.m_workers[i]));
      fw.Unsigned32(os->get_object_file_index(productionsite.m_workers[i]));
	}

   // Items from flag
   fw.Signed32(productionsite.m_fetchfromflag);

   // State
   fw.Unsigned16(productionsite.m_program.size());
   for (uint i=0; i<productionsite.m_program.size(); i++) {
      fw.CString(productionsite.m_program[i].program->get_name().c_str());
      fw.Signed32(productionsite.m_program[i].ip);
      fw.Signed32(productionsite.m_program[i].phase);
      fw.Unsigned32(productionsite.m_program[i].flags);
	}
   fw.Unsigned8(productionsite.m_program_timer);
   fw.Signed32(productionsite.m_program_time);

   // Wares Queues
   fw.Unsigned16(productionsite.m_input_queues.size());
   for (uint i=0; i<productionsite.m_input_queues.size(); i++)
      productionsite.m_input_queues[i]->Write(&fw, egbase, os);

   // Statistics
   fw.Unsigned16(productionsite.m_statistics.size());
   for (uint i=0; i<productionsite.m_statistics.size(); i++)
      fw.Unsigned8(productionsite.m_statistics[i]);
   fw.Unsigned8(productionsite.m_statistics_changed);
   fw.Data(productionsite.m_statistics_buf, sizeof(productionsite.m_statistics_buf));
}

/*
 * write for trainingsite
 */
void Widelands_Map_Buildingdata_Data_Packet::write_trainingsite
(const TrainingSite & trainingsite,
 FileWrite & fw,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
{
   // Write the version
   fw.Unsigned16(CURRENT_TRAININGSITE_PACKET_VERSION);

   // Write for productionsite
   write_productionsite(trainingsite, fw, egbase, os);

	//  requests
   fw.Unsigned16(trainingsite.m_soldier_requests.size());
   for (uint i=0; i<trainingsite.m_soldier_requests.size(); i++)
      trainingsite.m_soldier_requests[i]->Write(&fw, egbase, os);


	//  soldiers
   fw.Unsigned16(trainingsite.m_soldiers.size());
   for (uint i=0; i<trainingsite.m_soldiers.size(); i++) {
      assert(os->is_object_known(trainingsite.m_soldiers[i]));
      fw.Unsigned32(os->get_object_file_index(trainingsite.m_soldiers[i]));
	}

	// Don't save m_list_upgrades (remake at load)

		//Building heros ?
	fw.Unsigned8(trainingsite.m_build_heros);

		// Priority upgrades
	fw.Unsigned16(trainingsite.m_pri_hp);
	fw.Unsigned16(trainingsite.m_pri_attack);
	fw.Unsigned16(trainingsite.m_pri_defense);
	fw.Unsigned16(trainingsite.m_pri_evade);

		// Priority modificators
	fw.Unsigned16(trainingsite.m_pri_hp_mod);
	fw.Unsigned16(trainingsite.m_pri_attack_mod);
	fw.Unsigned16(trainingsite.m_pri_defense_mod);
	fw.Unsigned16(trainingsite.m_pri_evade_mod);

	// capacity (modified by user)
	fw.Unsigned8(trainingsite.m_capacity);

	// Need to read the m_prog_name as string !!
	fw.CString(trainingsite.m_prog_name.c_str());

	// DONE
}
