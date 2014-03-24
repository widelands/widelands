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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "game_io/game_map_data_packet.h"

#include "io/filesystem/filesystem.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "map_io/widelands_map_loader.h"
#include "map_io/widelands_map_saver.h"

namespace Widelands {

Game_Map_Data_Packet::~Game_Map_Data_Packet() {
	delete m_wms;
	delete m_wml;
}

void Game_Map_Data_Packet::Read
	(FileSystem & fs, Game & game, Map_Map_Object_Loader * const)
{
	if (not fs.FileExists("map") or not fs.IsDirectory("map"))
		throw game_data_error("no map");

	//  Now Load the map as it would be a normal map saving.
	delete m_wml;

	m_wml = new WL_Map_Loader(fs.MakeSubFileSystem("map"), &game.map());

	m_wml->preload_map(true);
	m_wml->load_world();

	//  DONE, mapfs gets deleted by WidelandsMapLoader.

	return;
}


void Game_Map_Data_Packet::Read_Complete(Game & game) {
	m_wml->load_map_complete(game, true);
	m_mol = m_wml->get_map_object_loader();
}


void Game_Map_Data_Packet::Write
	(FileSystem & fs, Game & game, Map_Map_Object_Saver * const)
{

	std::unique_ptr<FileSystem> mapfs
		(fs.CreateSubFileSystem("map", FileSystem::DIR));

	//  Now Write the map as it would be a normal map saving.
	delete m_wms;
	m_wms = new Map_Saver(*mapfs, game);
	m_wms->save();
	m_mos = m_wms->get_map_object_saver();
}

}
