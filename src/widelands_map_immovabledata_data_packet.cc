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

#include "widelands_map_immovabledata_data_packet.h"

#include "fileread.h"
#include "filewrite.h"
#include "editor.h"
#include "editorinteractive.h"
#include "editor_game_base.h"
#include "immovable.h"
#include "immovable_program.h"
#include "map.h"
#include "player.h"
#include "tribe.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "error.h"

#include <map>


#define CURRENT_PACKET_VERSION 1


Widelands_Map_Immovabledata_Data_Packet::
~Widelands_Map_Immovabledata_Data_Packet
()
{}


void Widelands_Map_Immovabledata_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base *,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const ol)
throw (_wexception)
{
   if( skip )
      return;

   FileRead fr;
   try {
      fr.Open( fs, "binary/immovable_data" );
   } catch ( ... ) {
      // not there, so skip
      return ;
   }

   // First packet version
   int packet_version=fr.Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      while(1) {
         uint reg=fr.Unsigned32();
         if(reg==0xffffffff) break; // Last immovable

         assert(ol->is_object_known(reg));
         Immovable* imm=static_cast<Immovable*>(ol->get_object_by_file_index(reg));

         // Animation
         imm->m_anim=imm->descr().get_animation(fr.CString());
         imm->m_animstart=fr.Signed32();

         // Programm
         if(fr.Unsigned8())
            imm->m_program=imm->descr().get_program(fr.CString());
         else
            imm->m_program=0;
         imm->m_program_ptr=fr.Unsigned32();
         imm->m_program_step=fr.Signed32();

         ol->mark_object_as_loaded(imm);
      }
      // DONE
      return;
   }
   throw wexception("Unknown version %i in Widelands_Map_Immovabledata_Data_Packet!\n", packet_version);

   assert( 0 );
}


/*
 * Write Function
 */
void Widelands_Map_Immovabledata_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
throw (_wexception)
{
   FileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   Map* map=egbase->get_map();
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         BaseImmovable* immovable=map->get_field(Coords(x,y))->get_immovable();

         // We do not write player immovables
         if(immovable && immovable->get_type()==Map_Object::IMMOVABLE) {
            assert(os->is_object_known(immovable));
            Immovable* imm=static_cast<Immovable*>(immovable);

            fw.Unsigned32(os->get_object_file_index(imm));

            // My position is not needed, set on creation

            // Animations
            fw.CString(imm->descr().get_animation_name(imm->m_anim).c_str());
            fw.Signed32(imm->m_animstart);

            // Program Stuff
            if(imm->m_program) {
               fw.Unsigned8(1);
               fw.CString(imm->m_program->get_name().c_str());
            } else
               fw.Unsigned8(0);
            fw.Unsigned32(imm->m_program_ptr);
            fw.Signed32(imm->m_program_step);

            os->mark_object_as_saved(imm);
         }
      }
   }

   fw.Unsigned32(0xffffffff);

   fw.Write( fs, "binary/immovable_data" );
   // DONE
}
