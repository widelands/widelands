/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include <iostream>
#include "editor_game_base.h"
#include "error.h"
#include "filesystem.h"
#include "game.h"
#include "profile.h"
#include "world.h"
#include "soldier.h"
#include "tribe.h"
#include "util.h"
#include "warehouse.h"
#include "wexception.h"
#include "worker.h"

using namespace std;

//
// Tribe_Descr class
//
Tribe_Descr::Tribe_Descr(const char* name)
{
	snprintf(m_name, sizeof(m_name), "%s", name);

	try
	{
		char directory[256];

		snprintf(directory, sizeof(directory), "tribes/%s", name);

		m_default_encdata.clear();
      parse_wares(directory);
		parse_buildings(directory);
		parse_workers(directory);
      parse_bobs(directory);
      parse_root_conf(directory);
	}
	catch(std::exception &e)
	{
		throw wexception("Error loading tribe %s: %s", name, e.what());
	}
}

Tribe_Descr::~Tribe_Descr(void)
{
}


/*
===============
Tribe_Descr::postload

Load all logic data
===============
*/
void Tribe_Descr::postload(Editor_Game_Base* g)
{
	// TODO: move more loads to postload
}

/*
===============
Tribe_Descr::load_graphics

Load tribe graphics
===============
*/
void Tribe_Descr::load_graphics()
{
	int i;

	for(i = 0; i < m_workers.get_nitems(); i++)
		m_workers.get(i)->load_graphics();
	
   for(i = 0; i < m_wares.get_nitems(); i++)
		m_wares.get(i)->load_graphics();

	for(i = 0; i < m_buildings.get_nitems(); i++)
		m_buildings.get(i)->load_graphics();
}


//
// down here: private read functions for loading
//

/*
===============
Tribe_Descr::parse_root_conf

Read and process the main conf file
===============
*/
void Tribe_Descr::parse_root_conf(const char *directory)
{
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/conf", directory);

	try
	{
		Profile prof(fname);
		Section *s;

		// Section [tribe]
		s = prof.get_safe_section("tribe");

		s->get_string("author");
		s->get_string("name"); // descriptive name
		s->get_string("descr"); // long description

		// Section [defaults]
		s = prof.get_section("defaults");

		if (s)
			m_default_encdata.parse(s);

		// Section [frontier]
		s = prof.get_section("frontier");
		if (!s)
			throw wexception("Missing section [frontier]");

		m_anim_frontier = g_anim.get(directory, s, 0, &m_default_encdata);

		// Section [flag]
		s = prof.get_section("flag");
		if (!s)
			throw wexception("Missing section [flag]");

		m_anim_flag = g_anim.get(directory, s, 0, &m_default_encdata);

      // default wares
      s = prof.get_safe_section("startwares");
	   Section::Value* value;

      while((value=s->get_next_val(0))) {
         int idx = m_wares.get_index(value->get_name());
         if(idx == -1)
            throw wexception("In section [startwares], ware %s is not know!", value->get_name());

         std::string name=value->get_name();
         m_startwares[name]=value->get_int();
      }

      // default workers
      s = prof.get_safe_section("startworkers");
      while((value=s->get_next_val(0))) {
         if(!strcmp(value->get_name(),"soldier")) continue; // Ignore soldiers here
         int idx = m_workers.get_index(value->get_name());
         if(idx == -1)
            throw wexception("In section [startworkers], worker %s is not know!", value->get_name());

         std::string name=value->get_name();
         m_startworkers[name]=value->get_int();
      }
   
      // default soldiers
      s = prof.get_safe_section("startsoldiers");
      while((value=s->get_next_val(0))) {
         // NOTE: no check here, since we do not know about max levels and so on
         std::string soldier=value->get_name();
         m_startsoldiers[soldier]=value->get_int();
      }
   }
   catch(std::exception &e) {
      throw wexception("%s: %s", fname, e.what());
   }
}


/*
===============
Tribe_Descr::parse_buildings

Read all the building descriptions
===============
*/
void Tribe_Descr::parse_buildings(const char *rootdir)
{
	char subdir[256];
	filenameset_t dirs;

	snprintf(subdir, sizeof(subdir), "%s/buildings", rootdir);

	g_fs->FindFiles(subdir, "*", &dirs);

	for(filenameset_t::iterator it = dirs.begin(); it != dirs.end(); it++) {
		Building_Descr *descr = 0;

		try {
			descr = Building_Descr::create_from_dir(this, it->c_str(), &m_default_encdata);
		} catch(std::exception &e) {
			log("Building %s failed: %s (garbage directory?)\n", it->c_str(), e.what());
		} catch(...) {
			log("Building %s failed: unknown exception (garbage directory?)\n", it->c_str());
		}

		if (descr)
			m_buildings.add(descr);
	}
}


/*
===============
Tribe_Descr::parse_workers

Read all worker descriptions
===============
*/
void Tribe_Descr::parse_workers(const char *directory)
{
	char subdir[256];
	filenameset_t dirs;

	snprintf(subdir, sizeof(subdir), "%s/workers", directory);

	g_fs->FindFiles(subdir, "*", &dirs);

	for(filenameset_t::iterator it = dirs.begin(); it != dirs.end(); it++) {
		Worker_Descr *descr = 0;

		try {
			descr = Worker_Descr::create_from_dir(this, it->c_str(), &m_default_encdata);
		} catch(std::exception &e) {
			log("Worker %s failed: %s (garbage directory?)\n", it->c_str(), e.what());
		} catch(...) {
			log("Worker %s failed: unknown exception (garbage directory?)\n", it->c_str());
		}

		if (descr)
			m_workers.add(descr);
	}
}

/*
===============
Tribe_Descr::parse_wares

Parse the wares belonging to this tribe, adding it to the games warelist. This is delayed until the game starts,
and is called by the Game class
===============
*/
void Tribe_Descr::parse_wares(const char* directory)
{
   Descr_Maintainer<Item_Ware_Descr>* wares=&m_wares;
   char subdir[256];
	filenameset_t dirs;

	snprintf(subdir, sizeof(subdir), "%s/wares", directory);

	g_fs->FindFiles(subdir, "*", &dirs);

	for(filenameset_t::iterator it = dirs.begin(); it != dirs.end(); it++) {
		char fname[256];

		snprintf(fname, sizeof(fname), "%s/conf", it->c_str());

		if (!g_fs->FileExists(fname))
			continue;

		const char *name;
		const char *slash = strrchr(it->c_str(), '/');
		const char *backslash = strrchr(it->c_str(), '\\');

		if (backslash && (!slash || backslash > slash))
			slash = backslash;

		if (slash)
			name = slash+1;
		else
			name = it->c_str();

		if (wares->get_index(name) >= 0)
			log("Ware %s is already known in world init\n", it->c_str());

		Item_Ware_Descr* descr = 0;

		try
		{
			descr = Item_Ware_Descr::create_from_dir(name, it->c_str());
		}
		catch(std::exception& e)
		{
			cerr << it->c_str() << ": " << e.what() << " (garbage directory?)" << endl;
		}
		catch(...)
		{
			cerr << it->c_str() << ": Unknown exception" << endl;
		}

		if (descr)
			wares->add(descr);
	}
}

/*
 * Parse the player bobs (animations, immovables, critters)
 */
void Tribe_Descr::parse_bobs(const char* directory) {
	char subdir[256];
	filenameset_t dirs;

	snprintf(subdir, sizeof(subdir), "%s/bobs", directory);

	g_fs->FindFiles(subdir, "*", &dirs);

	for(filenameset_t::iterator it = dirs.begin(); it != dirs.end(); it++) {
		char fname[256];

		snprintf(fname, sizeof(fname), "%s/conf", it->c_str());

		if (!g_fs->FileExists(fname))
			continue;

		const char *name;
		const char *slash = strrchr(it->c_str(), '/');
		const char *backslash = strrchr(it->c_str(), '\\');

		if (backslash && (!slash || backslash > slash))
			slash = backslash;

		if (slash)
			name = slash+1;
		else
			name = it->c_str();

		try
		{
			Profile prof(fname, "global"); // section-less file
			Section *s = prof.get_safe_section("global");
			const char *type = s->get_safe_string("type");

			if (!strcasecmp(type, "critter")) {
				Bob_Descr *descr;
				descr = Bob_Descr::create_from_dir(name, it->c_str(), &prof, this);
				m_bobs.add(descr);
			} else {
				Immovable_Descr *descr = new Immovable_Descr(name, this);
				descr->parse(it->c_str(), &prof);
				m_immovables.add(descr);
			}
		} catch(std::exception &e) {
			cerr << it->c_str() << ": " << e.what() << " (garbage directory?)" << endl;
		} catch(...) {
			cerr << it->c_str() << ": unknown exception (garbage directory?)" << endl;
		}
	}
}

/*
===========
void Tribe_Descr::load_warehouse_with_start_wares()

This loads a warehouse with the given start wares as defined in
the conf files
===========
*/
void Tribe_Descr::load_warehouse_with_start_wares(Editor_Game_Base* egbase, Warehouse* wh) {
   std::map<std::string, int>::iterator cur;
   
   for(cur=m_startwares.begin(); cur!=m_startwares.end(); cur++) {
      wh->insert_wares(get_safe_ware_index((*cur).first.c_str()), (*cur).second);
   }
   for(cur=m_startworkers.begin(); cur!=m_startworkers.end(); cur++) {
      wh->insert_workers(get_safe_worker_index((*cur).first.c_str()), (*cur).second);
   }
   for(cur=m_startsoldiers.begin(); cur!=m_startsoldiers.end(); cur++) {
      std::vector<std::string> list;
      split_string(cur->first, &list, "/");

      if(list.size()!=4) 
         throw wexception("Error in tribe (%s), startsoldier %s is not valid!", get_name(), cur->first.c_str());

      char* endp;
      int hplvl=strtol(list[0].c_str(),&endp, 0);
      if(endp && *endp)
         throw wexception("Bad hp level '%s'", list[0].c_str());
      int attacklvl=strtol(list[1].c_str(),&endp, 0);
      if(endp && *endp)
         throw wexception("Bad attack level '%s'", list[1].c_str());
      int defenselvl=strtol(list[2].c_str(),&endp, 0);
      if(endp && *endp)
         throw wexception("Bad defense level '%s'", list[2].c_str());
      int evadelvl=strtol(list[3].c_str(),&endp, 0);
      if(endp && *endp)
         throw wexception("Bad evade level '%s'", list[3].c_str());
  
      int i=0;
      for(i=0;i<cur->second; i++) {
         if(egbase->is_game()) { 
            Game* game=static_cast<Game*>(egbase);
            Soldier_Descr* soldierd=static_cast<Soldier_Descr*>(get_worker_descr(get_worker_index("soldier")));
            Soldier* soldier=static_cast<Soldier*>(soldierd->create(game, wh->get_owner(), wh, wh->get_position()));
            soldier->set_level(hplvl,attacklvl,defenselvl,evadelvl);
            wh->incorporate_worker(game, soldier);
         } 
      }
      //TODO: What to do in editor
   }
}


/*
 * does this tribe exist?
 */
bool Tribe_Descr::exists_tribe(std::string name) {
   std::string buf;
   buf="tribes/" + name + "/conf";;

   FileRead f;
   return f.TryOpen(g_fs, buf.c_str());
}

/*
 * Returns all tribes that exists
 */
void Tribe_Descr::get_all_tribes(std::vector<std::string>* retval) {
   retval->resize(0);

   // get all tribes
   filenameset_t m_tribes;
   g_fs->FindFiles("tribes", "*", &m_tribes);
   for(filenameset_t::iterator pname = m_tribes.begin(); pname != m_tribes.end(); pname++) {
      std::string tribe=*pname;
      tribe.erase(0,7); // remove 'tribes/'
      if(Tribe_Descr::exists_tribe(tribe.c_str()))
         retval->push_back(tribe);
   }
}

/*
==============
Resource_Descr::get_indicator

Find the best matching indicator for the given amount.
==============
*/
int Tribe_Descr::get_resource_indicator(Resource_Descr *res, uint amount)
{
   if(!res || !amount) {
      int idx=get_immovable_index("resi_none");
      if(idx==-1) 
         throw wexception("Tribe %s doesn't declare a resource indicator resi_none!\n", get_name());
      return idx;
   }
   
   char buffer[256];

   int i=1;
   int num_indicators=0;
   while(true) {
      sprintf(buffer, "resi_%s%i", res->get_name(), i);
      if(get_immovable_index(buffer)==-1) 
         break;
      ++i;
      ++num_indicators;
   }
  
   if(!num_indicators) {
      // Upsy, no indicators for this resource
      throw wexception("Tribe %s doesn't declar a resource indicator for resource %s!\n", get_name(), res->get_name());
   }
   
   uint bestmatch = (uint) (( static_cast<float>(amount)/res->get_max_amount() ) * num_indicators);
   if(((int)amount)<res->get_max_amount()) 
      bestmatch+=1; // Resi start with 1, not 0

   sprintf(buffer, "resi_%s%i", res->get_name(), bestmatch);

	// NoLog("Resource(%s): Indicator '%s' for amount = %u\n",
	//	res->get_name(), buffer, amount);


   
	return get_immovable_index(buffer);
}

/*
 * Return the given ware or die trying
 */
int Tribe_Descr::get_safe_ware_index(const char *name) {
   int retval=get_ware_index(name);

   if(retval==-1) 
      throw wexception("Tribe_Descr::get_safe_ware_index: Unknown ware %s!", name);
   return retval;
}

/*
 * Return the given worker or die trying
 */
int Tribe_Descr::get_safe_worker_index(const char *name) {
   int retval=get_worker_index(name);

   if(retval==-1) 
      throw wexception("Tribe_Descr::get_safe_worker_index: Unknown worker %s!", name);
   return retval;
}

/*
 * Return the given building or die trying
 */
int Tribe_Descr::get_safe_building_index(const char *name) {
   int retval=get_building_index(name);

   if(retval==-1) 
      throw wexception("Tribe_Descr::get_safe_building_index: Unknown building %s!", name);
   return retval;
}

