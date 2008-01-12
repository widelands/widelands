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

#ifndef __S__REPLAY_H
#define __S__REPLAY_H

/**
 * Allow players to watch previous game in a platform-independent way.
 * Also useful as a debugging aid.
 *
 * A game replay consists of a savegame plus a log-file of subsequent
 * playercommands.
 */

#include <string>

#include <stdint.h>

#define REPLAY_DIR "replays"
#define REPLAY_SUFFIX ".wrpl"

struct md5_checksum;

class StreamRead;
class StreamWrite;


namespace Widelands {

/**
 * Read game replays from disk.
 */
struct ReplayReader {
	ReplayReader(Game &, std::string const & filename);
	~ReplayReader();

	Command* GetNextCommand(uint32_t time);
	bool EndOfReplay();

private:
	Game       & m_game;
	StreamRead* m_cmdlog;

	uint32_t m_replaytime;
};

/**
 * Write game replays to disk.
 */
struct ReplayWriter {
	ReplayWriter(Game &, std::string const & filename);
	~ReplayWriter();

	void SendPlayerCommand(PlayerCommand* cmd);
	void SendSync(const md5_checksum& hash);

private:
	Game        & m_game;
	StreamWrite* m_cmdlog;
};

};

#endif // __S__REPLAYWRITER_H

