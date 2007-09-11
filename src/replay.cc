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
#include "game_loader.h"
#include "layered_filesystem.h"
#include "md5.h"
#include "playercommand.h"
#include "random.h"
#include "replay.h"
#include "save_handler.h"
#include "streamread.h"
#include "streamwrite.h"
#include "wexception.h"

#include "log.h"

// File format definitions
#define REPLAY_MAGIC 0x2E21A101
#define REPLAY_VERSION 2

enum {
	pkt_end = 2,
	pkt_playercommand = 3,
	pkt_syncreport = 4
};

#define SYNC_INTERVAL 200


class Cmd_ReplaySyncRead : public Command {
public:
	Cmd_ReplaySyncRead(uint time, const md5_checksum& hash)
		: Command(time), m_hash(hash)
	{
	}

	int get_id() {return QUEUE_CMD_REPLAYSYNCREAD;}

	void execute(Game* g)
	{
		md5_checksum myhash = g->get_sync_hash();

		if (m_hash != myhash) {
			log("REPLAY: Lost synchronization at time %u\n"
			    "I have:     %s\n"
			    "Replay has: %s\n",
			    get_duetime(), myhash.str().c_str(), m_hash.str().c_str());

			g->set_speed(0);
		} else {
			log("REPLAY: Sync checked successfully.\n");
		}
	}

private:
	md5_checksum m_hash;
};


/**
 * Load the savegame part of the given replay and open the command log.
 */
ReplayReader::ReplayReader(Game* game, const std::string filename)
	: m_game(game)
{
	m_replaytime = 0;

	FileSystem* const fs = g_fs->MakeSubFileSystem(filename + WLGF_SUFFIX);
	Game_Loader gl(*fs, game);
	gl.load_game();
	delete fs;

	m_cmdlog = g_fs->OpenStreamRead(filename);

	try {
		Uint32 magic = m_cmdlog->Unsigned32();
		if (magic == 0x2E21A100) // Note: This was never released as part of a build
			throw wexception
					("%s is a replay from a version that is known to have desync problems",
					 filename.c_str());
		if (magic != REPLAY_MAGIC)
			throw wexception("%s apparently not a valid replay file", filename.c_str());

		Uint8 version = m_cmdlog->Unsigned8();
		if (version < REPLAY_VERSION)
			throw wexception("Replay of version %u is known to have desync problems", version);
		if (version != REPLAY_VERSION)
			throw wexception("Unknown version %u", version);

		game->get_rng()->ReadState(*m_cmdlog);
	}
	catch (...) {
		delete m_cmdlog;
		throw;
	}
}


/**
 * Cleanup after replays
 */
ReplayReader::~ReplayReader()
{
	delete m_cmdlog;
	m_cmdlog = 0;
}


/**
 * Retrieve the next command, until no more commands before the given
 * timestamp are available.
 *
 * \return a \ref Command that should be enqueued in the command queue
 * or 0 if there are no remaining commands before the given time.
 */
Command* ReplayReader::GetNextCommand(uint time)
{
	if (!m_cmdlog)
		return 0;

	if (static_cast<int>(m_replaytime - time) > 0)
		return 0;

	try {
		unsigned char pkt = m_cmdlog->Unsigned8();

		switch (pkt) {
		case pkt_playercommand: {
			m_replaytime = m_cmdlog->Unsigned32();

			uint duetime = m_cmdlog->Unsigned32();
			uint cmdserial = m_cmdlog->Unsigned32();
			PlayerCommand* cmd = PlayerCommand::deserialize(*m_cmdlog);
			cmd->set_duetime(duetime);
			cmd->set_cmdserial(cmdserial);

			return cmd;
		}

		case pkt_syncreport: {
			uint duetime = m_cmdlog->Unsigned32();
			md5_checksum hash;
			m_cmdlog->Data(hash.data, sizeof(hash.data));

			return new Cmd_ReplaySyncRead(duetime, hash);
		}

		case pkt_end: {
			uint endtime = m_cmdlog->Unsigned32();
			log("REPLAY: End of replay (gametime: %u)\n", endtime);
			delete m_cmdlog;
			m_cmdlog = 0;
			return 0;
			break;
		}

		default:
			throw wexception("Unknown packet %u", pkt);
		}
	}
	catch (_wexception& e) {
		log("REPLAY: Caught exception %s\n", e.what());
		delete m_cmdlog;
		m_cmdlog = 0;
		return 0;
	}
}


/**
 * \return \c true if the end of the replay was reached
 */
bool ReplayReader::EndOfReplay()
{
	return m_cmdlog == 0;
}


/**
 * Command / timer that regularly inserts synchronization hashes into
 * the replay.
 */
class Cmd_ReplaySyncWrite : public Command {
public:
	Cmd_ReplaySyncWrite(uint t) : Command(t) {}

	int get_id() {return QUEUE_CMD_REPLAYSYNCWRITE;}

	void execute(Game* g) {
		if (ReplayWriter* rw = g->get_replaywriter()) {
			rw->SendSync (g->get_sync_hash());

			g->enqueue_command (new Cmd_ReplaySyncWrite(get_duetime()+SYNC_INTERVAL));
		}
	}
};

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

	log("Reloading the game from replay\n");
	FileSystem* fs = g_fs->MakeSubFileSystem(filename + WLGF_SUFFIX);
	try {
		Game_Loader gl(*fs, game);
		game->cleanup_for_load(true, true);
		gl.load_game();
		game->postload();
	} catch (...) {
		delete fs;
		throw;
	}
	delete fs;
	log("Done reloading the game from replay\n");

	game->enqueue_command(new Cmd_ReplaySyncWrite(game->get_gametime() + SYNC_INTERVAL));

	m_cmdlog = g_fs->OpenStreamWrite(filename);
	m_cmdlog->Unsigned32(REPLAY_MAGIC);
	m_cmdlog->Unsigned8(REPLAY_VERSION);

	game->get_rng()->WriteState(*m_cmdlog);
}


/**
 * Close the command log
 */
ReplayWriter::~ReplayWriter()
{
	m_cmdlog->Unsigned8(pkt_end);
	m_cmdlog->Unsigned32(m_game->get_gametime());

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
	m_cmdlog->Unsigned32(cmd->get_duetime());
	m_cmdlog->Unsigned32(cmd->get_cmdserial());
	cmd->serialize(*m_cmdlog);

	m_cmdlog->Flush();
}


/**
 * Store a synchronization hash for the current game time in the replay.
 */
void ReplayWriter::SendSync(const md5_checksum& hash)
{
	m_cmdlog->Unsigned8(pkt_syncreport);
	m_cmdlog->Unsigned32(m_game->get_gametime());
	m_cmdlog->Data(hash.data, sizeof(hash.data));
	m_cmdlog->Flush();
}
