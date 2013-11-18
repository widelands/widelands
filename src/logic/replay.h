/*
 * Copyright (C) 2007-2009,2013 by the Widelands Development Team
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

#ifndef REPLAY_H
#define REPLAY_H

/**
 * Allow players to watch previous game in a platform-independent way.
 * Also useful as a debugging aid.
 *
 * A game replay consists of a savegame plus a log-file of subsequent
 * playercommands.
 */

#include <cstring>
#include <string>

#include <stdint.h>

#define REPLAY_DIR "replays"
#define REPLAY_SUFFIX ".wrpl"

struct md5_checksum;

namespace Widelands {
struct Command;
class Game;
class PlayerCommand;
class StreamRead;
class StreamWrite;

/**
 * Read game replays from disk.
 */
class ReplayReader {
public:
	ReplayReader(Game & game, const std::string & filename);
	~ReplayReader();

	Command * GetNextCommand(uint32_t time);
	bool EndOfReplay();

private:
	StreamRead * m_cmdlog;

	uint32_t m_replaytime;
};

/**
 * Write game replays to disk.
 */
class ReplayWriter {
public:
	ReplayWriter(Game &, const std::string & filename);
	~ReplayWriter();

	void SendPlayerCommand(PlayerCommand *);
	void SendSync(const md5_checksum &);

private:
	Game        & m_game;
	StreamWrite * m_cmdlog;
	std::string m_filename;
};

}

#endif
