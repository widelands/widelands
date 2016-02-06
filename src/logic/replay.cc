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

#include "logic/replay.h"

#include "base/log.h"
#include "base/md5.h"
#include "base/wexception.h"
#include "game_io/game_loader.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamread.h"
#include "io/streamwrite.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_data_error.h"
#include "logic/playercommand.h"
#include "logic/save_handler.h"
#include "random/random.h"

namespace Widelands {

// File format definitions
constexpr uint32_t kReplayMagic = 0x2E21A101;
constexpr uint8_t kCurrentPacketVersion = 2;
constexpr uint32_t kSyncInterval = 200;

enum {
	pkt_end = 2,
	pkt_playercommand = 3,
	pkt_syncreport = 4
};

class CmdReplaySyncRead : public Command {
public:
	CmdReplaySyncRead(const uint32_t _duetime, const Md5Checksum & hash)
		: Command(_duetime), m_hash(hash)
	{}

	QueueCommandTypes id() const override {return QueueCommandTypes::kReplaySyncRead;}

	void execute(Game & game) override
	{
		const Md5Checksum myhash = game.get_sync_hash();

		if (m_hash != myhash) {
			log
				("REPLAY: Lost synchronization at time %u\n"
				 "I have:     %s\n"
				 "Replay has: %s\n",
				 duetime(), myhash.str().c_str(), m_hash.str().c_str());

			// In case syncstream logging is on, save it for analysis
			game.save_syncstream(true);

			// There has to be a better way to do this.
			game.game_controller()->set_desired_speed(0);
		}
	}

private:
	Md5Checksum m_hash;
};


/**
 * Load the savegame part of the given replay and open the command log.
 */
ReplayReader::ReplayReader(Game & game, const std::string & filename)
{
	m_replaytime = 0;

	{
		GameLoader gl(filename + WLGF_SUFFIX, game);
		gl.load_game();
	}

	m_cmdlog = g_fs->open_stream_read(filename);

	try {
		const uint32_t magic = m_cmdlog->unsigned_32();
		if (magic == 0x2E21A100)
			// Note: This was never released as part of a build
			throw wexception
				("%s is a replay from a version that is known to have desync "
				 "problems",
				 filename.c_str());
		if (magic != kReplayMagic)
			throw wexception
				("%s apparently not a valid replay file", filename.c_str());

		const uint8_t packet_version = m_cmdlog->unsigned_8();
		if (packet_version != kCurrentPacketVersion) {
			throw UnhandledVersionError("ReplayReader", packet_version, kCurrentPacketVersion);
		}
		game.rng().read_state(*m_cmdlog);
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
}


/**
 * Retrieve the next command, until no more commands before the given
 * timestamp are available.
 *
 * \return a \ref Command that should be enqueued in the command queue
 * or 0 if there are no remaining commands before the given time.
 */
Command * ReplayReader::get_next_command(const uint32_t time)
{
	if (!m_cmdlog)
		return nullptr;

	if (static_cast<int32_t>(m_replaytime - time) > 0)
		return nullptr;

	try {
		uint8_t pkt = m_cmdlog->unsigned_8();

		switch (pkt) {
		case pkt_playercommand: {
			m_replaytime = m_cmdlog->unsigned_32();

			uint32_t duetime = m_cmdlog->unsigned_32();
			uint32_t cmdserial = m_cmdlog->unsigned_32();
			PlayerCommand & cmd = *PlayerCommand::deserialize(*m_cmdlog);
			cmd.set_duetime  (duetime);
			cmd.set_cmdserial(cmdserial);

			return &cmd;
		}

		case pkt_syncreport: {
			uint32_t duetime = m_cmdlog->unsigned_32();
			Md5Checksum hash;
			m_cmdlog->data(hash.data, sizeof(hash.data));

			return new CmdReplaySyncRead(duetime, hash);
		}

		case pkt_end: {
			uint32_t endtime = m_cmdlog->unsigned_32();
			log("REPLAY: End of replay (gametime: %u)\n", endtime);
			delete m_cmdlog;
			m_cmdlog = nullptr;
			return nullptr;
		}

		default:
			throw wexception("Unknown packet %u", pkt);
		}
	} catch (const WException & e) {
		log("REPLAY: Caught exception %s\n", e.what());
		delete m_cmdlog;
		m_cmdlog = nullptr;
	}

	return nullptr;
}


/**
 * \return \c true if the end of the replay was reached
 */
bool ReplayReader::end_of_replay()
{
	return m_cmdlog == nullptr;
}


/**
 * Command / timer that regularly inserts synchronization hashes into
 * the replay.
 */
class CmdReplaySyncWrite : public Command {
public:
	CmdReplaySyncWrite(const uint32_t _duetime) : Command(_duetime) {}

	QueueCommandTypes id() const override {return QueueCommandTypes::kReplaySyncWrite;}

	void execute(Game & game) override {
		if (ReplayWriter * const rw = game.get_replaywriter()) {
			rw->send_sync (game.get_sync_hash());

			game.enqueue_command
				(new CmdReplaySyncWrite(duetime() + kSyncInterval));
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
ReplayWriter::ReplayWriter(Game & game, const std::string & filename)
	: m_game(game), m_filename(filename)
{
	log("#sirver filename: %s\n", filename.c_str());
	g_fs->ensure_directory_exists(REPLAY_DIR);

	SaveHandler & save_handler = m_game.save_handler();

	std::string error;
	if (!save_handler.save_game(m_game, m_filename + WLGF_SUFFIX, &error))
		throw wexception("Failed to save game for replay: %s", error.c_str());

	log("Reloading the game from replay\n");
	game.cleanup_for_load();
	{
		GameLoader gl(m_filename + WLGF_SUFFIX, game);
		gl.load_game();
	}
	game.postload();
	log("Done reloading the game from replay\n");

	game.enqueue_command
		(new CmdReplaySyncWrite(game.get_gametime() + kSyncInterval));

	m_cmdlog = g_fs->open_stream_write(filename);
	m_cmdlog->unsigned_32(kReplayMagic);
	m_cmdlog->unsigned_8(kCurrentPacketVersion);

	game.rng().write_state(*m_cmdlog);
}


/**
 * Close the command log
 */
ReplayWriter::~ReplayWriter()
{
	m_cmdlog->unsigned_8(pkt_end);
	m_cmdlog->unsigned_32(m_game.get_gametime());

	delete m_cmdlog;
}


/**
 * Call this whenever a new player command has entered the command queue.
 */
void ReplayWriter::send_player_command(PlayerCommand * cmd)
{
	m_cmdlog->unsigned_8(pkt_playercommand);
	// The semantics of the timestamp is
	// "There will be no more player commands that are due *before* the
	// given time".
	m_cmdlog->unsigned_32(m_game.get_gametime());
	m_cmdlog->unsigned_32(cmd->duetime());
	m_cmdlog->unsigned_32(cmd->cmdserial());
	cmd->serialize(*m_cmdlog);

	m_cmdlog->flush();
}


/**
 * Store a synchronization hash for the current game time in the replay.
 */
void ReplayWriter::send_sync(const Md5Checksum & hash)
{
	m_cmdlog->unsigned_8(pkt_syncreport);
	m_cmdlog->unsigned_32(m_game.get_gametime());
	m_cmdlog->data(hash.data, sizeof(hash.data));
	m_cmdlog->flush();
}

}
