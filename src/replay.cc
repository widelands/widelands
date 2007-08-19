/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#include "game.h"
#include "layered_filesystem.h"
#include "playercommand.h"
#include "replay.h"
#include "save_handler.h"
#include "streamread.h"
#include "streamwrite.h"
#include "wexception.h"


// File format definitions
#define REPLAY_MAGIC 0x2E21A100

enum {
	pkt_playercommand = 1,
	pkt_end = 2
};


ReplayReader::ReplayReader(Game* game, const std::string filename)
	: m_game(game)
{
}

ReplayReader::~ReplayReader()
{
}


/**
 * Start a replay at the given filename (the caller must add the suffix).
 *
 * This will immediately save the given game.
 * This is expected to be called just after game load has completed
 * and the game has changed into running state.
 */
ReplayWriter::ReplayWriter(Game* game, const std::string filename)
	: m_game(game)
{
	g_fs->EnsureDirectoryExists(REPLAY_DIR);

	SaveHandler* savehandler = m_game->get_save_handler();

	std::string error;
	if (!savehandler->save_game(m_game, filename + WLGF_SUFFIX, &error))
		throw wexception("Failed to save game for replay: %s", error.c_str());

	m_cmdlog = g_fs->OpenStreamWrite(filename);
	m_cmdlog->Unsigned32(REPLAY_MAGIC);
}


/**
 * Close the command log
 */
ReplayWriter::~ReplayWriter()
{
	m_cmdlog->Unsigned8(pkt_end);

	delete m_cmdlog;
	m_cmdlog = 0;
}


/**
 * Call this whenever a new player command has entered the command queue.
 */
void ReplayWriter::SendPlayerCommand(PlayerCommand* cmd)
{
	m_cmdlog->Unsigned8(pkt_playercommand);
	// The semantics of the timestamp is
	// "There will be no more player commands that are due *before* the
	// given time".
	m_cmdlog->Unsigned32(m_game->get_gametime());
	cmd->serialize(m_cmdlog);
}
