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

#include "game_map_data_packet.h"

#include "filesystem.h"
#include "game.h"
#include "widelands_map_loader.h"
#include "widelands_map_saver.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

Game_Map_Data_Packet::~Game_Map_Data_Packet() {
	delete m_wms;
	delete m_wml;
}


void Game_Map_Data_Packet::Read
(FileSystem & fs, Game * game, Map_Map_Object_Loader * const)
throw (_wexception)
{
	if (not fs.FileExists("map") or not fs.IsDirectory("map"))
		throw wexception("No map in this save game!");

	FileSystem * const mapfs = fs.MakeSubFileSystem("map");

	//  Now Load the map as it would be a normal map saving.
	delete m_wml;

	m_wml = new WL_Map_Loader(*mapfs, game->get_map());

	m_wml->preload_map(true);
	m_wml->load_world();

	//  DONE, mapfs gets deleted by WidelandsMapLoader.

	return;
}


void Game_Map_Data_Packet::Read_Complete(Game & game) {
	m_wml->load_map_complete(&game, true);
	m_mol = m_wml->get_map_object_loader();
}


void Game_Map_Data_Packet::Write
(FileSystem & fs, Game * game, Map_Map_Object_Saver * const)
throw (_wexception)
{

	FileSystem * const mapfs = fs.CreateSubFileSystem("map", FileSystem::DIR);

	//  Now Write the map as it would be a normal map saving.
	delete m_wms;
	m_wms = new Map_Saver(*mapfs, game);
	m_wms->save();
	m_mos = m_wms->get_map_object_saver();

	delete mapfs;
}

};
