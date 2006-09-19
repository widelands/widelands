/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include <map>
#include "constructionsite.h"
#include "editor.h"
#include "editorinteractive.h"
#include "editor_game_base.h"
#include "fileread.h"
#include "filewrite.h"
#include "game.h"
#include "map.h"
#include "militarysite.h"
#include "player.h"
#include "productionsite.h"
#include "production_program.h"
#include "soldier.h"
#include "transport.h"
#include "tribe.h"
#include "trainingsite.h"
#include "transport.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_buildingdata_data_packet.h"
#include "warehouse.h"
#include "worker.h"
#include "error.h"

// Versions
#define CURRENT_PACKET_VERSION 1

// Subversions
#define CURRENT_CONSTRUCTIONSITE_PACKET_VERSION 1
#define CURRENT_WAREHOUSE_PACKET_VERSION        1
#define CURRENT_MILITARYSITE_PACKET_VERSION     2
#define CURRENT_PRODUCTIONSITE_PACKET_VERSION   1
#define CURRENT_TRAININGSITE_PACKET_VERSION		1

/*
 * Destructor
 */
Widelands_Map_Buildingdata_Data_Packet::~Widelands_Map_Buildingdata_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Buildingdata_Data_Packet::Read(FileSystem* fs, Editor_Game_Base* egbase, bool skip, Widelands_Map_Map_Object_Loader* ol) throw(_wexception) {
   if( skip )
      return;

   FileRead fr;
   try {
      fr.Open( fs, "binary/building_data" );
   } catch ( ... ) {
      // not there, so skip
      return ;
   }

   // First packet version
   int packet_version=fr.Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      while(1) {
         uint ser=fr.Unsigned32();
         if(ser==0xffffffff) break; // Last building

         log("Loading building with the serial: %i\n", ser);

         assert(ol->is_object_known(ser));
         assert(ol->get_object_by_file_index(ser)->get_type()==Map_Object::BUILDING);
         Building* building=static_cast<Building*>(ol->get_object_by_file_index(ser));

         log("Reading building stuff for %p .. at location ", building);
         log("(%i,%i)\n", building->get_position().x, building->get_position().y);

         // Animation
         if(fr.Unsigned8())
            building->m_anim=building->get_descr()->get_animation(fr.CString());
         else
            building->m_anim=0;
         building->m_animstart=fr.Unsigned32();

         building->m_leave_queue.resize(fr.Unsigned16());
         for(uint i=0; i<building->m_leave_queue.size(); i++) {
            ser=fr.Unsigned32();
            if(ser) {
               assert(ol->is_object_known(ser));
               building->m_leave_queue[i]=ol->get_object_by_file_index(ser);
            } else
               building->m_leave_queue[i]=0;
         }
         building->m_leave_time=fr.Unsigned32();
         ser=fr.Unsigned32();
         if(ser) {
            assert(ol->is_object_known(ser));
            building->m_leave_allow=ol->get_object_by_file_index(ser);
         } else
            building->m_leave_allow=0;
         building->m_stop=fr.Unsigned8();

         // Set economy now, some stuff below will count on this
         building->set_economy(building->m_flag->get_economy());

         log("Read building stuff for %p\n", building);
         switch(building->get_building_type()) {
            case Building::CONSTRUCTIONSITE: read_constructionsite(building, &fr,egbase,ol); break;
            case Building::WAREHOUSE: read_warehouse(building, &fr, egbase, ol); break;
            case Building::PRODUCTIONSITE: read_productionsite(building, &fr, egbase, ol); break;
            case Building::MILITARYSITE: read_militarysite(building, &fr, egbase, ol); break;
            case Building::TRAININGSITE: read_trainingsite(building, &fr, egbase, ol); break;
            default: throw wexception("Widelands_Map_Buildingdata_Data_Packet::Read: Unknown building type %i!\n", building->get_building_type());
         }


         ol->mark_object_as_loaded(building);
      }

      // DONE
      return;
   }
   throw wexception("Unknown version %i in Widelands_Map_Buildingdata_Data_Packet!\n", packet_version);

   assert(0);
}

void Widelands_Map_Buildingdata_Data_Packet::read_constructionsite(Building* building, FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* ol) {
   ConstructionSite* cs=static_cast<ConstructionSite*>(building);

   log("Reading cs stuff for %p\n", building);

   int version=fr->Unsigned16();
   if(version==CURRENT_CONSTRUCTIONSITE_PACKET_VERSION) {
      cs->m_building=cs->get_owner()->get_tribe()->get_building_descr(cs->get_owner()->get_tribe()->get_safe_building_index(fr->CString()));
      bool prevb=fr->Unsigned8();
      if(prevb) {
         cs->m_prev_building=cs->get_owner()->get_tribe()->get_building_descr(cs->get_owner()->get_tribe()->get_safe_building_index(fr->CString()));
      } else
         cs->m_prev_building=0;

      // Builder request
      if(cs->m_builder_request)
         delete cs->m_builder_request;
      bool request=fr->Unsigned8();
      if(request) {
         cs->m_builder_request = new Request(cs, 0,
               &ConstructionSite::request_builder_callback, cs, Request::WORKER);
         cs->m_builder_request->Read(fr, egbase, ol);
      } else
         cs->m_builder_request=0;

      // Builder
      uint reg=fr->Unsigned32();
      if(reg) {
         assert(ol->is_object_known(reg));
         cs->m_builder=static_cast<Worker*>(ol->get_object_by_file_index(reg));
      } else
         cs->m_builder=0;

      // Wares queues
      uint size=fr->Unsigned16();
      assert(cs->m_wares.size()>=size);
      for(uint i=size; i<cs->m_wares.size(); i++) {
         Game * const game = dynamic_cast<Game * const>(egbase);
         if (game) cs->m_wares[i]->cleanup(game);
         delete cs->m_wares[i];
      }
      cs->m_wares.resize(size);
      for(uint i=0; i<cs->m_wares.size(); i++) {
         log("Reading waresqueue %i,%i\n", i+1, (int)cs->m_wares.size());
         cs->m_wares[i]->Read(fr,egbase,ol);
      }

      cs->m_fetchfromflag=fr->Signed32();

      cs->m_working=fr->Unsigned8();
      cs->m_work_steptime=fr->Unsigned32();
      cs->m_work_completed=fr->Unsigned32();
      cs->m_work_steps=fr->Unsigned32();

      log("Read  cs stuff for %p\n", building);
   } else
      throw wexception("Unknown Constructionsite-Version %i in Widelands_Map_Buildingdata_Data_Packet!\n", version);
}

void Widelands_Map_Buildingdata_Data_Packet::read_warehouse(Building* building, FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* ol) {
   Warehouse* wh=static_cast<Warehouse*>(building);

   int version=fr->Unsigned16();
   if(version==CURRENT_WAREHOUSE_PACKET_VERSION) {
      log("Reading warehouse stuff for %p\n", building);
      // Supply
      while(1) {
         if(!fr->Unsigned8()) break;
         std::string waren=fr->CString();
         uint stock=fr->Unsigned16();
         int id=wh->get_owner()->get_tribe()->get_safe_ware_index(waren.c_str());
         wh->remove_wares(id, wh->m_supply->stock_wares(id));
         wh->insert_wares(id, stock);
      }
      while(1) {
         if(!fr->Unsigned8()) break;
         std::string workern=fr->CString();
         uint stock=fr->Unsigned16();
         int id=wh->get_owner()->get_tribe()->get_safe_worker_index(workern.c_str());
         wh->remove_workers(id, wh->m_supply->stock_workers(id));
         wh->insert_workers(id, stock);
      }

      // Request
      for(uint i=0; i<wh->m_requests.size(); i++)
         delete wh->m_requests[i];
      wh->m_requests.resize(fr->Unsigned16());
      for(uint i=0; i<wh->m_requests.size(); i++) {
         Request* req = new Request(wh, 0, &Warehouse::idle_request_cb, wh, Request::WORKER);
         req->Read(fr,egbase,ol);
         wh->m_requests[i]=req;
      }

      // Incorporated Workers
      while(wh->m_incorporated_workers.size()) {
         std::vector<Object_Ptr>::iterator i=wh->m_incorporated_workers.begin();
         static_cast<Worker*>(i->get(egbase))->remove(egbase);
         wh->m_incorporated_workers.erase(i);
      }
      wh->m_incorporated_workers.resize(0);
      int nrworkers=fr->Unsigned16();
      for(int i=0; i<nrworkers; i++) {
         uint id=fr->Unsigned32();
         std::string name=fr->CString();
         assert(ol->is_object_known(id));
         // Worker might not yet be loaded so that get ware won't work
         // but make sure that such a worker exists in tribe
	      try {wh->get_owner()->get_tribe()->get_worker_index(name.c_str());}
	      catch (Descr_Maintainer<Worker_Descr>::Nonexistent) {
            throw wexception
			      ("Unknown worker %s in incorporated workers in "
			       "Widelands_Map_Buildingdata_Data_Packet!\n",
			       name.c_str());
	      }
         Worker* w=static_cast<Worker*>(ol->get_object_by_file_index(id));
         wh->sort_worker_in(egbase, name, w);
      }

      // Carrier spawn
      wh->m_next_carrier_spawn=fr->Unsigned32();

      log("Read warehouse stuff for %p\n", building);
   } else
      throw wexception("Unknown Warehouse-Version %i in Widelands_Map_Buildingdata_Data_Packet!\n", version);
}

void Widelands_Map_Buildingdata_Data_Packet::read_militarysite(Building* building, FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* ol) {
   MilitarySite* ms=static_cast<MilitarySite*>(building);

      // read the version
      uint version=fr->Unsigned16();

      if(version==CURRENT_MILITARYSITE_PACKET_VERSION) {
         // Read productionsite
         read_productionsite(building, fr, egbase, ol);

         // Request
			uint nr_requests=fr->Unsigned16();

			for(uint i=0; i<ms->m_soldier_requests.size(); i++)
	         delete ms->m_soldier_requests[i];
      	ms->m_soldier_requests.resize(nr_requests);

			for(uint i=0; i<nr_requests; i++) {
	         Request* req = new Request(ms, 0, &MilitarySite::request_soldier_callback, ms, Request::SOLDIER);
         	req->Read(fr,egbase,ol);
         	ms->m_soldier_requests[i]=req;
      	}

         // Soldier
         uint nr_soldiers = fr->Unsigned16();
         assert(!ms->m_soldiers.size());
         ms->m_soldiers.resize(nr_soldiers);
         for(uint i=0; i<nr_soldiers; i++) {
            uint reg = fr->Unsigned32();
            assert(ol->is_object_known(reg));
            ms->m_soldiers[i] = static_cast<Soldier*>(ol->get_object_by_file_index(reg));
         }

         // did conquer
         ms->m_didconquer = fr->Unsigned8();

			// capacity (modified by user)
			ms->m_capacity = fr->Unsigned8();
         // DONE
      } else
         throw wexception("Unknown MilitarySite-Version %i in Widelands_Map_Buildingdata_Data_Packet!\n", version);
}

void Widelands_Map_Buildingdata_Data_Packet::read_productionsite(Building* building, FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* ol) {
   ProductionSite* ps=static_cast<ProductionSite*>(building);

   ushort version = fr->Unsigned16();

   if(version==CURRENT_PACKET_VERSION) {
      // Requests
      uint nr_requests=fr->Unsigned16();
      for(uint i=0; i<ps->m_worker_requests.size(); i++)
         delete ps->m_worker_requests[i];
      ps->m_worker_requests.resize(nr_requests);
      for(uint i=0; i<nr_requests; i++) {
         Request* req = new Request(ps, 0, &ProductionSite::request_worker_callback, ps, Request::WORKER);
         req->Read(fr,egbase,ol);
         ps->m_worker_requests[i]=req;
      }

      // Workers
      uint nr_workers = fr->Unsigned16();
      assert(!ps->m_workers.size());
      ps->m_workers.resize(nr_workers);
      for(uint i=0; i<nr_workers; i++) {
         uint reg = fr->Unsigned32();
         assert(ol->is_object_known(reg));
         ps->m_workers[i] = static_cast<Worker*>(ol->get_object_by_file_index(reg));
      }

      // Items from flags
      ps->m_fetchfromflag = fr->Signed32();

      // State
      uint nr_progs = fr->Unsigned16();
      ps->m_program.resize(nr_progs);
      for(uint i=0; i<nr_progs; i++) {
         std::string prog = fr->CString();
         ps->m_program[i].program = ps->get_descr()->get_program(prog.c_str());
         ps->m_program[i].ip = fr->Signed32();
         ps->m_program[i].phase = fr->Signed32();
         ps->m_program[i].flags = fr->Unsigned32();
      }
      ps->m_program_timer = fr->Unsigned8();
      ps->m_program_time = fr->Signed32();

      // Wares
      uint nr_queues = fr->Unsigned16();
      if( nr_queues != ps->m_input_queues.size() )
         throw ("Productionsite has wrong number of input queues!\n");
      for(uint i=0; i<ps->m_input_queues.size(); i++)
         ps->m_input_queues[i]->Read(fr,egbase,ol);

      // Statistics
      uint stats_size = fr->Unsigned16();
      ps->m_statistics.resize(stats_size);
      for(uint i=0; i<ps->m_statistics.size(); i++)
         ps->m_statistics[i] = fr->Unsigned8();
      ps->m_statistics_changed = fr->Unsigned8();
      memcpy(ps->m_statistics_buf, fr->Data(sizeof(ps->m_statistics_buf)), sizeof(ps->m_statistics_buf));
   } else
      throw wexception("Unknown ProductionSite-Version %i in Widelands_Map_Buildingdata_Data_Packet!\n", version);
}

void Widelands_Map_Buildingdata_Data_Packet::read_trainingsite(Building* building, FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* ol) {
   TrainingSite* ts=static_cast<TrainingSite*>(building);

      // read the version
	uint version=fr->Unsigned16();

	if(version==CURRENT_TRAININGSITE_PACKET_VERSION) {
         // Read productionsite
		read_productionsite(building, fr, egbase, ol);

         // Requests
		uint nr_requests=fr->Unsigned16();

		for(uint i=0; i<ts->m_soldier_requests.size(); i++)
			delete ts->m_soldier_requests[i];
		ts->m_soldier_requests.resize(nr_requests);

		for(uint i=0; i<nr_requests; i++) {
			Request* req = new Request(ts, 0, &TrainingSite::request_soldier_callback, ts, Request::SOLDIER);
			req->Read(fr,egbase,ol);
			ts->m_soldier_requests[i]=req;
		}

         // Soldiers
		uint nr_soldiers = fr->Unsigned16();
		assert(!ts->m_soldiers.size());
		ts->m_soldiers.resize(nr_soldiers);
		for(uint i=0; i<nr_soldiers; i++) {
			uint reg = fr->Unsigned32();
			assert(ol->is_object_known(reg));
			ts->m_soldiers[i] = static_cast<Soldier*>(ol->get_object_by_file_index(reg));
		}

		// Don't save m_list_upgrades (remake at load)

			//Building heros ?
		ts->m_build_heros = fr->Unsigned8();

			// Priority upgrades
		ts->m_pri_hp = fr->Unsigned16();
		ts->m_pri_attack = fr->Unsigned16();
		ts->m_pri_defense = fr->Unsigned16();
		ts->m_pri_evade = fr->Unsigned16();

			// Priority modificators
		ts->m_pri_hp_mod = fr->Unsigned16();
		ts->m_pri_attack_mod = fr->Unsigned16();
		ts->m_pri_defense_mod = fr->Unsigned16();
		ts->m_pri_evade_mod = fr->Unsigned16();

		// capacity (modified by user)
		ts->m_capacity = fr->Unsigned8();

		// Need to read the m_prog_name as string !!
		std::string prog = fr->CString();
		ts->m_prog_name = prog;

		//m_total_soldiers is just a convenience variable and not saved, recalculate it
		ts->m_total_soldiers=ts->m_soldiers.size()+ts->m_soldier_requests.size();

		// DONE
	} else
		throw wexception("Unknown TrainingSite-Version %i in Widelands_Map_Buildingdata_Data_Packet!\n", version);
}





/*
 * Write Function
 */
void Widelands_Map_Buildingdata_Data_Packet::Write(FileSystem* fs, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* os) throw(_wexception) {
   FileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   // Walk the map again
   Map* map=egbase->get_map();
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         Field* f=map->get_field(Coords(x,y));
         BaseImmovable* imm=f->get_immovable();

         if(!imm) continue;

         if(imm->get_type()==Map_Object::BUILDING) {
            Building* building=static_cast<Building*>(imm);

            assert(os->is_object_known(building));

            if(building->get_position()!=Coords(x,y)) {
               // This is not this buildings main position
               continue;
            }

            int ser=os->get_object_file_index(building);
            fw.Unsigned32(ser);

            // Player immovable owner is already in existance packet

            // Write the general stuff
            if(building->m_anim) {
               fw.Unsigned8(1);
               fw.CString(building->get_descr()->get_animation_name(building->m_anim).c_str());
            } else
               fw.Unsigned8(0);

            fw.Unsigned32(building->m_animstart);

            // Leave queue time
            fw.Unsigned16(building->m_leave_queue.size());
            for(uint i=0; i<building->m_leave_queue.size(); i++) {
               assert(os->is_object_known(building->m_leave_queue[i].get(egbase)));
               fw.Unsigned32(os->get_object_file_index(building->m_leave_queue[i].get(egbase)));
            }
            fw.Unsigned32(building->m_leave_time);
            if(building->m_leave_allow.get(egbase)) {
               assert(os->is_object_known(building->m_leave_allow.get(egbase)));
               fw.Unsigned32(os->get_object_file_index(building->m_leave_allow.get(egbase)));
            } else
               fw.Unsigned32(0);
            fw.Unsigned8(building->m_stop);

            switch(building->get_building_type()) {
               case Building::CONSTRUCTIONSITE: write_constructionsite(building, &fw,egbase,os); break;
               case Building::WAREHOUSE: write_warehouse(building, &fw, egbase, os); break;
               case Building::PRODUCTIONSITE: write_productionsite(building, &fw, egbase, os); break;
               case Building::MILITARYSITE: write_militarysite(building, &fw, egbase, os); break;
               case Building::TRAININGSITE: write_trainingsite(building, &fw, egbase, os); break;
               default: throw wexception("Widelands_Map_Buildingdata_Data_Packet::Write: Unknown building type %i!\n", building->get_building_type());
            }

            os->mark_object_as_saved(building);
         }

      }
   }

   fw.Unsigned32(0xffffffff); // End of buildings

   fw.Write( fs, "binary/building_data" );
   // DONE
}

/*
 * write for constructionsite
 */
void Widelands_Map_Buildingdata_Data_Packet::write_constructionsite(Building* building, FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* os) {
   ConstructionSite* cs=static_cast<ConstructionSite*>(building);

   // First, write current version
   fw->Unsigned16(CURRENT_CONSTRUCTIONSITE_PACKET_VERSION);

   // Describtions
   fw->CString(cs->m_building->get_name());
   if(cs->m_prev_building) {
      fw->Unsigned8(1);
      fw->CString(cs->m_prev_building->get_name());
   } else
      fw->Unsigned8(0);

   // builder request
   if(cs->m_builder_request) {
      fw->Unsigned8(1);
      cs->m_builder_request->Write(fw, egbase, os);
   } else
      fw->Unsigned8(0);

   // builder
   if(cs->m_builder) {
      assert(os->is_object_known(cs->m_builder));
      fw->Unsigned32(os->get_object_file_index(cs->m_builder));
   } else
      fw->Unsigned32(0);

   // ware queues
   fw->Unsigned16(cs->m_wares.size());
   for(uint i=0; i<cs->m_wares.size(); i++) {
      cs->m_wares[i]->Write(fw,egbase,os);
   }

   fw->Signed32(cs->m_fetchfromflag);

   fw->Unsigned8(cs->m_working);
   fw->Unsigned32(cs->m_work_steptime);
   fw->Unsigned32(cs->m_work_completed);
   fw->Unsigned32(cs->m_work_steps);
}

/*
 * write for warehouse
 */
void Widelands_Map_Buildingdata_Data_Packet::write_warehouse(Building* building, FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* os) {
   Warehouse* wh=static_cast<Warehouse*>(building);

   fw->Unsigned16(CURRENT_WAREHOUSE_PACKET_VERSION);

   // Supply
   const WareList& wares=wh->m_supply->get_wares();
   for(int i=0; i<wares.get_nrwareids(); i++) {
      fw->Unsigned8(1);
      fw->CString(wh->get_owner()->get_tribe()->get_ware_descr(i)->get_name());
      fw->Unsigned16(wares.stock(i));
   }
   fw->Unsigned8(0);
   const WareList& workers=wh->m_supply->get_workers();
   for(int i=0; i<workers.get_nrwareids(); i++) {
      fw->Unsigned8(1);
      fw->CString(wh->get_owner()->get_tribe()->get_worker_descr(i)->get_name());
      fw->Unsigned16(workers.stock(i));
   }
   fw->Unsigned8(0);

   // Request
   fw->Unsigned16(wh->m_requests.size());
   for(uint i=0; i<wh->m_requests.size(); i++) {
      wh->m_requests[i]->Write(fw,egbase,os);
   }

   // Incorporated workers, write sorted after file-serial
   fw->Unsigned16(wh->m_incorporated_workers.size());
   std::map<uint,Worker*> workermap;
   std::vector<Object_Ptr>::iterator i=wh->m_incorporated_workers.begin();
   while(i!=wh->m_incorporated_workers.end()) {
      assert(os->is_object_known(i->get(egbase)));
      uint ser=os->get_object_file_index(i->get(egbase));
      workermap.insert(std::pair<uint,Worker*>(ser,static_cast<Worker*>(i->get(egbase))));
      ++i;
   }

   std::map<uint,Worker*>::iterator j=workermap.begin();
   while(j!=workermap.end()) {
      assert(os->is_object_known(j->second));
      fw->Unsigned32(os->get_object_file_index(j->second));
      fw->CString(j->second->get_name().c_str());
      ++j;
   }

   // Carrier spawn
   fw->Unsigned32(wh->m_next_carrier_spawn);
}

/*
 * write for militarysite
 */
void Widelands_Map_Buildingdata_Data_Packet::write_militarysite(Building* building, FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* os) {
   // Write the version
   fw->Unsigned16(CURRENT_MILITARYSITE_PACKET_VERSION);

   // Write for productionsite
   write_productionsite(building, fw, egbase, os);

   MilitarySite* ms=static_cast<MilitarySite*>(building);

   // Request
   fw->Unsigned16(ms->m_soldier_requests.size());
   for(uint i=0; i<ms->m_soldier_requests.size(); i++)
      ms->m_soldier_requests[i]->Write(fw,egbase,os);


   // Soldier
   fw->Unsigned16(ms->m_soldiers.size());
   for(uint i=0; i<ms->m_soldiers.size(); i++) {
      assert(os->is_object_known(ms->m_soldiers[i]));
      fw->Unsigned32(os->get_object_file_index(ms->m_soldiers[i]));
   }

   // did conquer
   fw->Unsigned8(ms->m_didconquer);

	// capacity
	fw->Unsigned8(ms->m_capacity);
}

/*
 * write for productionsite
 */
void Widelands_Map_Buildingdata_Data_Packet::write_productionsite(Building* building, FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* os) {
   ProductionSite* ps=static_cast<ProductionSite*>(building);

   // Write the version
   fw->Unsigned16(CURRENT_PRODUCTIONSITE_PACKET_VERSION);


   // Requests
   fw->Unsigned16(ps->m_worker_requests.size());
   for(uint i=0; i<ps->m_worker_requests.size(); i++)
      ps->m_worker_requests[i]->Write(fw,egbase,os);

   // Workers
   fw->Unsigned16(ps->m_workers.size());
   for(uint i=0; i<ps->m_workers.size(); i++) {
      assert(os->is_object_known(ps->m_workers[i]));
      fw->Unsigned32(os->get_object_file_index(ps->m_workers[i]));
   }

   // Items from flag
   fw->Signed32(ps->m_fetchfromflag);

   // State
   fw->Unsigned16(ps->m_program.size());
   for(uint i=0; i<ps->m_program.size(); i++) {
      fw->CString(ps->m_program[i].program->get_name().c_str());
      fw->Signed32(ps->m_program[i].ip);
      fw->Signed32(ps->m_program[i].phase);
      fw->Unsigned32(ps->m_program[i].flags);
   }
   fw->Unsigned8(ps->m_program_timer);
   fw->Signed32(ps->m_program_time);

   // Wares Queues
   fw->Unsigned16(ps->m_input_queues.size());
   for(uint i=0; i<ps->m_input_queues.size(); i++)
      ps->m_input_queues[i]->Write(fw,egbase,os);

   // Statistics
   fw->Unsigned16(ps->m_statistics.size());
   for(uint i=0; i<ps->m_statistics.size(); i++)
      fw->Unsigned8(ps->m_statistics[i]);
   fw->Unsigned8(ps->m_statistics_changed);
   fw->Data(ps->m_statistics_buf, sizeof(ps->m_statistics_buf));
}

/*
 * write for trainingsite
 */
void Widelands_Map_Buildingdata_Data_Packet::write_trainingsite(Building* building, FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* os) {
   // Write the version
   fw->Unsigned16(CURRENT_TRAININGSITE_PACKET_VERSION);

   // Write for productionsite
   write_productionsite(building, fw, egbase, os);

   TrainingSite* ts=static_cast<TrainingSite*>(building);

   	// Requests
   fw->Unsigned16(ts->m_soldier_requests.size());
   for(uint i=0; i<ts->m_soldier_requests.size(); i++)
      ts->m_soldier_requests[i]->Write(fw,egbase,os);


   	// Soldiers
   fw->Unsigned16(ts->m_soldiers.size());
   for(uint i=0; i<ts->m_soldiers.size(); i++) {
      assert(os->is_object_known(ts->m_soldiers[i]));
      fw->Unsigned32(os->get_object_file_index(ts->m_soldiers[i]));
   }

	// Don't save m_list_upgrades (remake at load)

		//Building heros ?
	fw->Unsigned8(ts->m_build_heros);

		// Priority upgrades
	fw->Unsigned16(ts->m_pri_hp);
	fw->Unsigned16(ts->m_pri_attack);
	fw->Unsigned16(ts->m_pri_defense);
	fw->Unsigned16(ts->m_pri_evade);

		// Priority modificators
	fw->Unsigned16(ts->m_pri_hp_mod);
	fw->Unsigned16(ts->m_pri_attack_mod);
	fw->Unsigned16(ts->m_pri_defense_mod);
	fw->Unsigned16(ts->m_pri_evade_mod);

	// capacity (modified by user)
	fw->Unsigned8(ts->m_capacity);

	// Need to read the m_prog_name as string !!
	fw->CString(ts->m_prog_name.c_str());

	// DONE
}
