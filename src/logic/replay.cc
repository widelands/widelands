/*
 * Copyright (C) 2007-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/replay.h"

#include <memory>

#include "base/log.h"
#include "base/md5.h"
#include "base/random.h"
#include "base/time_string.h"
#include "base/wexception.h"
#include "build_info.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_packet.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamread.h"
#include "io/streamwrite.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_data_error.h"
#include "logic/playercommand.h"
#include "logic/save_handler.h"
#include "ui_basic/messagebox.h"
#include "wui/interactive_base.h"

namespace Widelands {

static inline void delete_temp_file(const std::string& temp_file) {
	try {
		g_fs->fs_unlink(temp_file);
	} catch (const std::exception& e) {
		log_err("Unable to delete temporary file '%s': %s", temp_file.c_str(), e.what());
	}
}

// File format definitions
constexpr uint32_t kReplayMagic = 0x2E21A102;
constexpr uint8_t kCurrentPacketVersion = 4;
constexpr Duration kSyncInterval(200);

enum { pkt_end = 2, pkt_playercommand = 3, pkt_syncreport = 4 };

class CmdReplaySyncRead : public Command {
public:
	CmdReplaySyncRead(const Time& init_duetime, const Md5Checksum& hash)
	   : Command(init_duetime), hash_(hash) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kReplaySyncRead;
	}

	void execute(Game& game) override {
		if (reported_desync_for_ == &game) {
			// We already know there was a desync
			return;
		}

		const Md5Checksum myhash = game.get_sync_hash();

		if (hash_ != myhash) {
			reported_desync_for_ = &game;
			log_err_time(game.get_gametime(),
			             "REPLAY: Lost synchronization at time %u\n"
			             "I have:     %s\n"
			             "Replay has: %s\n",
			             duetime().get(), myhash.str().c_str(), hash_.str().c_str());

			// In case syncstream logging is on, save it for analysis
			game.save_syncstream(true);

			// There has to be a better way to do this.
			game.game_controller()->set_desired_speed(0);

			UI::WLMessageBox m(
			   game.get_ibase(), UI::WindowStyle::kWui, _("Desync"),
			   format(_("The replay has desynced and the game was paused.\n"
			            "You are probably watching a replay created with another version of "
			            "Widelands, which is not supported.\n\n"
			            "If you are certain that the replay was created with the same version "
			            "of Widelands, %1$s, please report this problem as a bug.\n"
			            "You will find related messages in the standard output (stdout.txt on "
			            "Windows). Please add this information to your report."),
			          build_ver_details()),
			   UI::WLMessageBox::MBoxType::kOk);
			m.run<UI::Panel::Returncodes>();
		}
	}

private:
	Md5Checksum hash_;

	static const Game* reported_desync_for_;
};
const Game* CmdReplaySyncRead::reported_desync_for_(nullptr);

/**
 * Load the savegame part of the given replay and open the command log.
 */
ReplayReader::ReplayReader(Game& game, const std::string& filename) : replaytime_(Time(0)) {
	if (!g_fs->file_exists(filename)) {
		// Try locating file in a distinct fs
		std::unique_ptr<FileSystem> fs(g_fs->make_sub_file_system(FileSystem::fs_dirname(filename)));
		cmdlog_ = fs->open_stream_read(FileSystem::fs_filename(filename.c_str()));
	} else {
		cmdlog_ = g_fs->open_stream_read(filename);
	}

	try {
		const uint32_t magic = cmdlog_->unsigned_32();
		if (magic != kReplayMagic) {
			throw wexception("%s not a valid replay file", filename.c_str());
		}

		const uint8_t packet_version = cmdlog_->unsigned_8();
		if (packet_version != kCurrentPacketVersion) {
			throw UnhandledVersionError("ReplayReader", packet_version, kCurrentPacketVersion);
		}

		const std::string temp_file =
		   kTempFileDir + FileSystem::file_separator() + timestring() + kSavegameExtension;
		{
			const uint32_t bytes = cmdlog_->unsigned_32();
			std::unique_ptr<char[]> buffer(new char[bytes]);
			cmdlog_->data_complete(buffer.get(), bytes);
			FileWrite fw;
			fw.data(buffer.get(), bytes);
			fw.write(*g_fs, temp_file);
		}

		game.enabled_addons().clear();
		GameLoader gl(temp_file, game);
		Widelands::GamePreloadPacket gpdp;
		gl.preload_game(gpdp);
		game.set_win_condition_displayname(gpdp.get_win_condition());
		gl.load_game();
		game.postload_addons();

		delete_temp_file(temp_file);

		game.rng().read_state(*cmdlog_);
	} catch (...) {
		delete cmdlog_;
		throw;
	}
}

/**
 * Cleanup after replays
 */
ReplayReader::~ReplayReader() {
	delete cmdlog_;
}

/**
 * Retrieve the next command, until no more commands before the given
 * timestamp are available.
 *
 * \return a \ref Command that should be enqueued in the command queue
 * or 0 if there are no remaining commands before the given time.
 */
Command* ReplayReader::get_next_command(const Time& time) {
	if (cmdlog_ == nullptr) {
		return nullptr;
	}

	if (replaytime_ > time) {
		return nullptr;
	}

	try {
		uint8_t pkt = cmdlog_->unsigned_8();

		switch (pkt) {
		case pkt_playercommand: {
			replaytime_ = Time(cmdlog_->unsigned_32());

			Time duetime(cmdlog_->unsigned_32());
			uint32_t cmdserial = cmdlog_->unsigned_32();
			PlayerCommand& cmd = *PlayerCommand::deserialize(*cmdlog_);
			cmd.set_duetime(duetime);
			cmd.set_cmdserial(cmdserial);

			return &cmd;
		}

		case pkt_syncreport: {
			Time duetime(cmdlog_->unsigned_32());
			Md5Checksum hash;
			cmdlog_->data(hash.data, sizeof(hash.data));

			return new CmdReplaySyncRead(duetime, hash);
		}

		case pkt_end: {
			Time endtime(cmdlog_->unsigned_32());
			log_err_time(time, "REPLAY: End of replay (gametime: %u)\n", endtime.get());
			delete cmdlog_;
			cmdlog_ = nullptr;
			return nullptr;
		}

		default:
			throw wexception("Unknown packet %u", pkt);
		}
	} catch (const WException& e) {
		log_err_time(time, "REPLAY: Caught exception %s\n", e.what());
		delete cmdlog_;
		cmdlog_ = nullptr;
	}

	return nullptr;
}

/**
 * \return \c true if the end of the replay was reached
 */
bool ReplayReader::end_of_replay() {
	return cmdlog_ == nullptr;
}

/**
 * Command / timer that regularly inserts synchronization hashes into
 * the replay.
 */
class CmdReplaySyncWrite : public Command {
public:
	explicit CmdReplaySyncWrite(const Time& init_duetime) : Command(init_duetime) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kReplaySyncWrite;
	}

	void execute(Game& game) override {
		if (ReplayWriter* const rw = game.get_replaywriter()) {
			rw->send_sync(game.get_sync_hash());

			game.enqueue_command(new CmdReplaySyncWrite(duetime() + kSyncInterval));
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
ReplayWriter::ReplayWriter(Game& game, const std::string& filename)
   : game_(game), filename_(filename) {
	g_fs->ensure_directory_exists(kReplayDir);
	g_fs->ensure_directory_exists(kTempFileDir);

	SaveHandler& save_handler = game_.save_handler();

	const std::string temp_savegame =
	   g_fs->create_unique_temp_file_path(kTempFileDir, kSavegameExtension);
	std::string error;
	if (!save_handler.save_game(game_, temp_savegame, FileSystem::ZIP, &error)) {
		throw wexception("Failed to save game for replay: %s", error.c_str());
	}

	verb_log_info("Initializing replay stream");
	cmdlog_ = g_fs->open_stream_write(filename);
	cmdlog_->unsigned_32(kReplayMagic);
	cmdlog_->unsigned_8(kCurrentPacketVersion);

	{
		std::unique_ptr<FileSystem> sub_fs(
		   g_fs->make_sub_file_system(FileSystem::fs_dirname(temp_savegame)));
		FileRead fr;
		fr.open(*sub_fs, FileSystem::fs_filename(temp_savegame.c_str()));
		const size_t bytes = fr.get_size();
		cmdlog_->unsigned_32(bytes);
		cmdlog_->data(fr.data(bytes), bytes);
	}

	verb_log_info("Reloading the game from replay");
	game.cleanup_for_load();
	{
		GameLoader gl(temp_savegame, game);
		gl.load_game();
	}
	verb_log_info("Done reloading the game from replay");
	delete_temp_file(temp_savegame);

	game.enqueue_command(new CmdReplaySyncWrite(game.get_gametime() + kSyncInterval));

	game.rng().write_state(*cmdlog_);
}

/**
 * Close the command log
 */
ReplayWriter::~ReplayWriter() {
	cmdlog_->unsigned_8(pkt_end);
	cmdlog_->unsigned_32(game_.get_gametime().get());

	delete cmdlog_;
}

/**
 * Call this whenever a new player command has entered the command queue.
 */
void ReplayWriter::send_player_command(PlayerCommand* cmd) {
	cmdlog_->unsigned_8(pkt_playercommand);
	// The semantics of the timestamp is
	// "There will be no more player commands that are due *before* the
	// given time".
	cmdlog_->unsigned_32(game_.get_gametime().get());
	cmdlog_->unsigned_32(cmd->duetime().get());
	cmdlog_->unsigned_32(cmd->cmdserial());
	cmd->serialize(*cmdlog_);

	cmdlog_->flush();
}

/**
 * Store a synchronization hash for the current game time in the replay.
 */
void ReplayWriter::send_sync(const Md5Checksum& hash) {
	cmdlog_->unsigned_8(pkt_syncreport);
	cmdlog_->unsigned_32(game_.get_gametime().get());
	cmdlog_->data(hash.data, sizeof(hash.data));
	cmdlog_->flush();
}

ReplayfileSavegameExtractor::ReplayfileSavegameExtractor(const std::string& gamefilename)
   : source_file_(gamefilename) {
	if (!ends_with(source_file_, kReplayExtension)) {
		return;
	}

	FileRead fr;
	fr.open(*g_fs, source_file_);

	const uint32_t magic = fr.unsigned_32();
	if (magic != kReplayMagic) {
		throw wexception("%s not a valid replay file", source_file_.c_str());
	}

	const uint8_t packet_version = fr.unsigned_8();
	if (packet_version != kCurrentPacketVersion) {
		throw UnhandledVersionError(
		   "ReplayfileSavegameExtractor", packet_version, kCurrentPacketVersion);
	}

	const uint32_t bytes = fr.unsigned_32();
	std::unique_ptr<char[]> buffer(new char[bytes]);
	fr.data_complete(buffer.get(), bytes);
	FileWrite fw;
	fw.data(buffer.get(), bytes);
	temp_file_ = g_fs->create_unique_temp_file_path(kTempFileDir, kSavegameExtension);
	fw.write(*g_fs, temp_file_);
}

ReplayfileSavegameExtractor::~ReplayfileSavegameExtractor() {
	if (!temp_file_.empty()) {
		delete_temp_file(temp_file_);
	}
}
}  // namespace Widelands
