/*
 * Copyright (C) 2002-2004, 2006-2009, 2015 by the Widelands Development Team
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

#ifndef WL_LOGIC_CMD_QUEUE_H
#define WL_LOGIC_CMD_QUEUE_H

#include <memory>
#include <queue>
#include <stdint.h>

#include <stdint.h>

#include "logic/queue_cmd_ids.h"
#include <stdint.h>

class FileRead;
class FileWrite;

namespace Widelands {

class EditorGameBase;
struct MapObjectSaver;
class MapObjectLoader;

// Define here all the possible users
#define SENDER_MAPOBJECT 0
#define SENDER_PLAYER1 1 // those are just place holder, a player can send
#define SENDER_PLAYER2 2 // commands with it's player number
#define SENDER_PLAYER3 3
#define SENDER_PLAYER4 4
#define SENDER_PLAYER5 5
#define SENDER_PLAYER6 6
#define SENDER_PLAYER7 7
#define SENDER_PLAYER8 8
#define SENDER_CMDQUEUE 100   // The Cmdqueue sends itself some action request

#define CMD_QUEUE_BUCKET_SIZE 65536 // Make this a power of two, so that % is fast

// ---------------------- END    OF CMDS ----------------------------------

//
// This is finally the command queue. It is fully widelands specific,
// it needs to know nearly all modules.
//
// It used to be implemented as a priority_queue sorted by execution_time,
// serial and type of commands. This proved to be a performance bottleneck on
// big games. I then changed this to use a constant size hash_map[gametime] of
// priority_queues. This allows for ~O(1) access by time and in my analyses,
// practically all buckets were used, so there is not much memory overhead.
// This removed the bottleneck for big games.
//
// I first tried with boost::unordered_map, but as expected, it grew beyond all
// limits when accessed with gametime. Therefore I reverted back to a simple
// vector.
//
// The price we pay is that when saving, we also have to traverse till we no
// longer find any new command to write. This could theoretically take forever
// but in my tests it was not noticeable.
class Game;

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
	Command (const uint32_t _duetime) : m_duetime(_duetime) {}
	virtual ~Command ();

	virtual void execute (Game &) = 0;
	virtual uint8_t id() const = 0;

	uint32_t duetime() const {return m_duetime;}
	void set_duetime(uint32_t const t) {m_duetime = t;}

private:
	uint32_t m_duetime;
};


/**
 * All commands that affect the game simulation (e.g. acting of \ref Bob
 * objects), players' commands, etc. must be derived from this class.
 *
 * GameLogicCommands are stored in savegames, and they must run in parallel
 * for all instances of a game to ensure parallel simulation.
 */
struct GameLogicCommand : public Command {
	GameLogicCommand (uint32_t const _duetime) : Command(_duetime) {}

	// Write these commands to a file (for savegames)
	virtual void write
		(FileWrite &, EditorGameBase &, MapObjectSaver  &);
	virtual void read
		(FileRead  &, EditorGameBase &, MapObjectLoader &);
};

class CmdQueue {
	friend struct GameCmdQueuePacket;

	enum {
		cat_nongamelogic = 0,
		cat_gamelogic,
		cat_playercommand
	};

	struct CmdItem {
		Command * cmd;

		/**
		 * category and serial are used to sort commands such that
		 * commands will be executed in the same order on all systems
		 * independent of details of the priority_queue implementation.
		 */
		int32_t category;
		uint32_t serial;

		bool operator< (const CmdItem & c) const
		{
			if (cmd->duetime() != c.cmd->duetime())
				return cmd->duetime() > c.cmd->duetime();
			else if (category != c.category)
				return category > c.category;
			else
				return serial > c.serial;
		}
	};

public:
	CmdQueue(Game &);
	~CmdQueue();

	/// Add a command to the queue. Takes ownership.
	void enqueue (Command *);

	// Run all commands scheduled for the next interval milliseconds, and update
	// the internal time as well. the game_time_var represents the current game
	// time, which we update and with which we must mess around (to run all
	// queued cmd.s) and which we update (add the interval)
	int32_t run_queue(int32_t interval, uint32_t & game_time_var);

	void flush(); // delete all commands in the queue now

private:
	Game                       & m_game;
	uint32_t                     nextserial;
	uint32_t m_ncmds;
	using CommandsContainer = std::vector<std::priority_queue<CmdItem>>;
	CommandsContainer m_cmds;
};

}

#endif  // end of include guard: WL_LOGIC_CMD_QUEUE_H
