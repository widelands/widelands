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

class FileRead;
class FileWrite;
class Editor_Game_Base;
class Widelands_Map_Map_Object_Saver;
class Widelands_Map_Map_Object_Loader;

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

class BaseCommand {
private:
	int duetime;

public:
	BaseCommand (int);
	virtual ~BaseCommand ();

	virtual void execute (Game*)=0;

	int get_duetime() const {return duetime;}
	void set_duetime(int t) {duetime=t;}

	// Write these commands to a file (for savegames)
	virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*)=0;
	virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*)=0;

	virtual int get_id() = 0; // Get this command id

	// Write commands for BaseCommand. Must be called from upper classes
	void BaseCmdWrite(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
	void BaseCmdRead(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);
};

class Cmd_Queue {
	friend class Game_Cmd_Queue_Data_Packet;

	struct cmditem {
		BaseCommand * cmd;
		unsigned long serial;

		bool operator< (const cmditem& c) const
		{
			if (cmd->get_duetime()==c.cmd->get_duetime())
				return serial > c.serial;
			else
				return cmd->get_duetime() > c.cmd->get_duetime();
		}
	};

public:
	Cmd_Queue(Game *g);
	~Cmd_Queue();

	void enqueue (BaseCommand*);
	int run_queue (int interval, int* game_time_var);

	void flush(); // delete all commands in the queue now

private:
	Game *                       m_game;
	std::priority_queue<cmditem> m_cmds;
	unsigned long                nextserial;
};

#endif // __S__CMD_QUEUE_H
