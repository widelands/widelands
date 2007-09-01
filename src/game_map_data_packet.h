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

#ifndef __S__GAME_MAP_DATA_PACKET_H
#define __S__GAME_MAP_DATA_PACKET_H

#include "game_data_packet.h"

class Widelands_Map_Saver;
class Widelands_Map_Loader;

/*
 * This is just a wrapper around Widelands_Map_Saver and Widelands_Map_Loader
 */
struct Game_Map_Data_Packet : public Game_Data_Packet {
      Game_Map_Data_Packet() {m_mos = 0; m_mol = 0; m_wms = 0; m_wml = 0;}
      virtual ~Game_Map_Data_Packet();


	/// Ensures that the world gets loaded but does not much more.
	virtual void Read
		(FileSystem &, Game*, Widelands_Map_Map_Object_Loader * const = 0)
		throw (_wexception);

	void Read_Complete(Game &); ///  Loads the rest of the map.

	virtual void Write
		(FileSystem &, Game*, Widelands_Map_Map_Object_Saver * const = 0)
		throw (_wexception);

      inline Widelands_Map_Map_Object_Saver* get_map_object_saver() {return m_mos;}
      inline Widelands_Map_Map_Object_Loader* get_map_object_loader() {return m_mol;}

private:
      Widelands_Map_Map_Object_Saver* m_mos;
      Widelands_Map_Map_Object_Loader* m_mol;
      Widelands_Map_Saver* m_wms;
      Widelands_Map_Loader* m_wml;
};

#endif
