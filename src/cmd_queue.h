/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#ifndef __S__CMD_QUEUE_H
#define __S__CMD_QUEUE_H

#include "queue_cmd_ids.h"

#include <queue>
#include <stdint.h>

class Editor_Game_Base;
class Widelands_Map_Map_Object_Saver;
class Widelands_Map_Map_Object_Loader;
class FileRead;
class FileWrite;

// Define here all the possible users
#define SENDER_MAPOBJECT 0
#define SENDER_PLAYER1 1 // those are just place holder, a player can send commands with
#define SENDER_PLAYER2 2 // it's player number
#define SENDER_PLAYER3 3
#define SENDER_PLAYER4 4
#define SENDER_PLAYER5 5
#define SENDER_PLAYER6 6
#define SENDER_PLAYER7 7
#define SENDER_PLAYER8 8
#define SENDER_CMDQUEUE 100   // The Cmdqueue sends itself some action request


enum {
	FLAGACTION_GEOLOGIST   =  0, //  call a geologist


	ENEMYFLAGACTION_ATTACK = 10,   // Performs an attack
};
// ---------------------- END    OF CMDS ----------------------------------

//
// This is finally the command queue. It is fully widelands specific,
// it needs to know nearly all modules.
//
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
class Command {
private:
	int32_t m_duetime;

public:
	Command (int32_t);
	virtual ~Command ();

	virtual void execute (Game*) = 0;
	virtual int32_t get_id() = 0;

	int32_t get_duetime() const {return m_duetime;}
	void set_duetime(int32_t t) {m_duetime = t;}
};


/**
 * All commands that affect the game simulation (e.g. acting of \ref Bob
 * objects), players' commands, etc. must be derived from this class.
 *
 * GameLogicCommands are stored in savegames, and they must run in parallel
 * for all instances of a game to ensure parallel simulation.
 */
class GameLogicCommand : public Command {
public:
	GameLogicCommand (int32_t duetime);

	// Write these commands to a file (for savegames)
	virtual void Write
		(FileWrite             &,
		 Editor_Game_Base               &,
		 Widelands_Map_Map_Object_Saver &);
	virtual void Read
		(FileRead               &,
		 Editor_Game_Base                &,
		 Widelands_Map_Map_Object_Loader &);
};

class Cmd_Queue {
	friend class Game_Cmd_Queue_Data_Packet;

	enum {
		cat_nongamelogic = 0,
		cat_gamelogic,
		cat_playercommand
	};

	struct cmditem {
		Command * cmd;

		/**
		 * category and serial are used to sort commands such that
		 * commands will be executed in the same order on all systems
		 * independent of details of the priority_queue implementation.
		 */
		int32_t category;
		uint32_t serial;

		bool operator< (const cmditem& c) const
		{
			if (cmd->get_duetime() != c.cmd->get_duetime())
				return cmd->get_duetime() > c.cmd->get_duetime();
			else if (category != c.category)
				return category > c.category;
			else
				return serial > c.serial;
		}
	};

public:
	Cmd_Queue(Game *g);
	~Cmd_Queue();

	void enqueue (Command*);
	int32_t run_queue (int32_t interval, int32_t* game_time_var);

	void flush(); // delete all commands in the queue now

private:
	Game *                       m_game;
	std::priority_queue<cmditem> m_cmds;
	uint32_t                     nextserial;
};

#endif // __S__CMD_QUEUE_H
