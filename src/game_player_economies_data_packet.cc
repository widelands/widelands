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

#include "game_player_economies_data_packet.h"

#include "fileread.h"
#include "filewrite.h"
#include "game.h"
#include "player.h"
#include <stdint.h>
#include "transport.h"


#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Game_Player_Economies_Data_Packet::~Game_Player_Economies_Data_Packet() {
}

/*
 * Read Function
 */
void Game_Player_Economies_Data_Packet::Read
(FileSystem & fs, Game* game, Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
   FileRead fr;

   fr.Open(fs, "binary/player_economies");

   // read packet version
	const uint16_t packet_version = fr.Unsigned16();
	if (packet_version == CURRENT_PACKET_VERSION) {
      // DONE
      Map* map=game->get_map();
      for (uint32_t i=1; i<=game->get_map()->get_nrplayers(); i++) {
         Player* plr=game->get_safe_player(i);
         if (!plr) continue;

         uint32_t nr_economies=fr.Unsigned16();
         assert(nr_economies == plr->m_economies.size());

         std::vector<Economy*> ecos;
         ecos.resize(nr_economies);

         for (uint32_t j=0; j<plr->m_economies.size(); j++) {
            int x=fr.Unsigned16();
            int y=fr.Unsigned16();
            Flag* flag=static_cast<Flag*>(map->get_field(Coords(x, y))->get_immovable());
            assert(flag);
            ecos[j]=flag->get_economy();
			}
         for (uint32_t j=0; j<ecos.size(); j++) {
            plr->m_economies[j]=ecos[j];
            ecos[j]->balance_requestsupply(); // Issue first balance
			}
		}
	} else
		throw wexception
			("Unknown version in Game_Player_Economies_Data_Packet: %u",
			 packet_version);
}

/*
 * Write Function
 */
void Game_Player_Economies_Data_Packet::Write
(FileSystem & fs, Game* game, Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
   FileWrite fw;

   // Now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   bool done=false;
	const Player_Number nr_players = game->map().get_nrplayers();
	iterate_players_existing_const(p, nr_players, *game, plr) {
      fw.Unsigned16(plr->m_economies.size());
      for (uint32_t j=0; j<plr->m_economies.size(); j++) {
         done=false;
         // Walk the map so that we find a representant
         Map* map=game->get_map();
         for (uint16_t y=0; y<map->get_height(); y++) {
            for (uint16_t x=0; x<map->get_width(); x++) {
               BaseImmovable* imm=map->get_field(Coords(x, y))->get_immovable();
               if (!imm) continue;

               if (imm->get_type()==Map_Object::FLAG) {
                  Flag* flag=static_cast<Flag*>(imm);
                  if (flag->get_economy() == plr->m_economies[j]) {
                     fw.Unsigned16(x);
                     fw.Unsigned16(y);
                     done=true;
						}
					}
               if (done) break;
				}
            if (done) break;
			}
         if (done) continue;
		}
	}

   fw.Write(fs, "binary/player_economies");
}
