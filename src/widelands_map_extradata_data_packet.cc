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

#include "widelands_map_extradata_data_packet.h"

#include "editor_game_base.h"
#include "filewrite.h"
#include "graphic.h" // Since we are laying about the path of the pictures
#include "map.h"
#include "profile.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_data_packet_ids.h"

#include <SDL_image.h>

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Extradata_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Map_Map_Object_Loader * const)
throw (_wexception)
{
	if (skip) return;

   Profile prof;
	try {prof.read("extra_data", 0, fs);} catch (...) {return;}
   Section* s = prof.get_section("global");

   // read packet version
	const int32_t packet_version = s->get_int("packet_version");
	if (packet_version == CURRENT_PACKET_VERSION) {
      // Nothing more. But read all pics
		if (fs.FileExists("pics") and fs.IsDirectory("pics")) {
         filenameset_t pictures;
			fs.FindFiles("pics", "*", &pictures);
			for
				(filenameset_t::iterator pname = pictures.begin();
				 pname != pictures.end();
				 ++pname)
			{
				if (fs.IsDirectory((*pname).c_str())) // Might be some dir, maybe CVS
               continue;

            FileRead fr;

				fr.Open(fs, pname->c_str());
            SDL_Surface* surf = IMG_Load_RW(SDL_RWFromMem(fr.Data(0), fr.GetSize()), 1);
				if (!surf)
               continue; // Illegal pic. Skip it
				Surface & picsurf = *new Surface();
				picsurf.set_sdl_surface(*surf);

	    std::string picname = FileSystem::FS_Filename((*pname).c_str());
            picname = "map:" + picname;

				const uint32_t data =
					g_gr->get_picture(PicMod_Game, picsurf, picname.c_str());

            // ok, the pic is now known to the game. But when the game is saved, this data has to be
            // regenerated.
            Map::Extradata_Info info;
            info.type = Map::Extradata_Info::PIC;
            info.filename = *pname;
            info.data = data;
            egbase->get_map()->m_extradatainfos.push_back(info);
			}
		}
	} else
		throw wexception
			("Map Terrain Extradata Data Packet with unknown version %u in map!",
			 packet_version);
}


void Map_Extradata_Data_Packet::Write
(FileSystem & fs, Editor_Game_Base * egbase, Map_Map_Object_Saver * const)
throw (_wexception)
{
   Profile prof;
   Section* s = prof.create_section("global");

   // packet version
   s->set_int("packet_version", CURRENT_PACKET_VERSION);

   // Nothing more. All pics in the dir pic are loaded as pictures
	const Map::Extradata_Infos & extradatainfos =
		egbase->map().m_extradatainfos;
	for (uint32_t i = 0; i < extradatainfos.size(); ++i) {
		const Map::Extradata_Info & edi = extradatainfos[i];
      assert(edi.type == Map::Extradata_Info::PIC);

		fs.EnsureDirectoryExists("pics");
      FileWrite fw;

      g_gr->save_png(edi.data, &fw);

      fw.Write(fs, edi.filename.c_str());
	}

   // Write out
   prof.write("extra_data", false, fs);
}

};
