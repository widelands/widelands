/*
 * Copyright (C) 2004-2025 by the Widelands Development Team
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

#ifndef WL_COMMANDS_COMMAND_H
#define WL_COMMANDS_COMMAND_H

#include "base/times.h"
#include "commands/queue_cmd_ids.h"
#include "logic/message_id.h"
#include "logic/widelands.h"

class StreamRead;
class StreamWrite;

namespace Widelands {

class EditorGameBase;
class Game;
class MapObjectLoader;
struct MapObjectSaver;

/**
 * A command that is supposed to be executed at a certain gametime.
 *
 * Unlike \ref GameLogicCommand objects, base Command object may be used to
 * orchestrate network synchronization and other things that are outside the
 * game simulation itself.
 *
 * Therefore, base Commands are not saved in savegames and do not need to be
 * the same for all parallel simulation.
 */
struct Command {
	explicit Command(const Time& init_duetime) : duetime_(init_duetime) {
	}
	virtual ~Command() = default;

	virtual void execute(Game&) = 0;
	[[nodiscard]] virtual QueueCommandTypes id() const = 0;

	[[nodiscard]] const Time& duetime() const {
		return duetime_;
	}
	void set_duetime(const Time& t) {
		duetime_ = t;
	}

private:
	Time duetime_;
};

/**
 * All commands that affect the game simulation (e.g. acting of \ref Bob
 * objects), players' commands, etc. must be derived from this class.
 *
 * GameLogicCommands are stored in savegames, and they must run in parallel
 * for all instances of a game to ensure parallel simulation.
 */
struct GameLogicCommand : public Command {
	explicit GameLogicCommand(const Time& init_duetime) : Command(init_duetime) {
	}

	// Write these commands to a file (for savegames)
	virtual void write(FileWrite&, EditorGameBase&, MapObjectSaver&);
	virtual void read(FileRead&, EditorGameBase&, MapObjectLoader&);
};

/**
 * PlayerCommand is for commands issued by players. It has the additional
 * ability to send itself over the network
 *
 * PlayerCommands are given serial numbers once they become authoritative
 * (e.g. after being acked by the server). The serial numbers must then be
 * reasonably unique (to be precise, they must be unique per duetime) and
 * the same across all hosts, to ensure parallel simulation.
 */
class PlayerCommand : public GameLogicCommand {
public:
	PlayerCommand(const Time& time, PlayerNumber);

	/// For savegame loading
	PlayerCommand() : GameLogicCommand(Time(0)), sender_(0), cmdserial_(0) {
	}

	void write_id_and_sender(StreamWrite& ser);

	[[nodiscard]] PlayerNumber sender() const {
		return sender_;
	}
	[[nodiscard]] uint32_t cmdserial() const {
		return cmdserial_;
	}
	void set_cmdserial(const uint32_t s) {
		cmdserial_ = s;
	}

	// For networking and replays
	virtual void serialize(StreamWrite&) = 0;
	static Widelands::PlayerCommand* deserialize(StreamRead&);

	// Call these from child classes
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	PlayerNumber sender_;
	uint32_t cmdserial_;
};

/// Abstract base for commands about a message.
struct PlayerMessageCommand : public PlayerCommand {
	PlayerMessageCommand() = default;  //  for savegames
	PlayerMessageCommand(const Time& t, const PlayerNumber p, const MessageId& i)
	   : PlayerCommand(t, p), message_id_(i) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	explicit PlayerMessageCommand(StreamRead&);

	[[nodiscard]] MessageId message_id() const {
		return message_id_;
	}

private:
	MessageId message_id_;
};

/// Abstract base for commands about a target quantity.
struct CmdChangeTargetQuantity : public PlayerCommand {
	CmdChangeTargetQuantity() = default;  //  For savegame loading.
	CmdChangeTargetQuantity(const Time& duetime,
	                        PlayerNumber sender,
	                        uint32_t economy,
	                        DescriptionIndex init_ware_type);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	explicit CmdChangeTargetQuantity(StreamRead&);

	void serialize(StreamWrite&) override;

protected:
	[[nodiscard]] Serial economy() const {
		return economy_;
	}
	[[nodiscard]] DescriptionIndex ware_type() const {
		return ware_type_;
	}

private:
	Serial economy_{0U};
	DescriptionIndex ware_type_{INVALID_INDEX};
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_COMMAND_H
