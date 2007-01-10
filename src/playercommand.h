/*
 * Copyright (C) 2004, 2006-2007 by the Widelands Development Team
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

#ifndef __PLAYERCOMMAND_H__
#define __PLAYERCOMMAND_H__


#include "cmd_queue.h"
#include "building.h"
#include "transport.h"

class Serializer;
class Deserializer;


// PlayerCommand is for commands issued by players. It has the additional
// ability to send itself over the network
class PlayerCommand:public BaseCommand {
    private:
	char sender;

    public:
	PlayerCommand (int, char);
	PlayerCommand(void) : BaseCommand(0) { } // For savegame loading
	virtual ~PlayerCommand ();

	char get_sender() const { return sender; }

	virtual void serialize (Serializer*)=0;
	static PlayerCommand* deserialize (Deserializer*);

   protected:
   // Call these from child classes
   void PlayerCmdWrite(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
   void PlayerCmdRead(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);
};

class Cmd_Bulldoze:public PlayerCommand {
    private:
	int serial;

    public:
	Cmd_Bulldoze(void) : PlayerCommand() { } // For savegame loading
	Cmd_Bulldoze (int t, int p, PlayerImmovable* pi):PlayerCommand(t,p)
	{ serial=pi->get_serial(); }

	Cmd_Bulldoze (Deserializer*);

   // Write these commands to a file (for savegames)
   virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
   virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

   virtual int get_id(void) { return QUEUE_CMD_BULLDOZE; } // Get this command id

	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};

class Cmd_Build:public PlayerCommand {
    private:
	Coords coords;
	int id;

    public:
	Cmd_Build(void) : PlayerCommand() { } // For savegame loading
   Cmd_Build (int t, int p, const Coords& c, int i):PlayerCommand(t,p)
	{ coords=c; id=i; }

	Cmd_Build (Deserializer*);

   // Write these commands to a file (for savegames)
   virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
   virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

   virtual int get_id(void) { return QUEUE_CMD_BUILD; } // Get this command id

	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};

class Cmd_BuildFlag:public PlayerCommand {
    private:
	Coords coords;

    public:
	Cmd_BuildFlag(void) : PlayerCommand() { } // For savegame loading
	Cmd_BuildFlag (int t, int p, const Coords& c):PlayerCommand(t,p)
	{ coords=c; }

	Cmd_BuildFlag (Deserializer*);

   // Write these commands to a file (for savegames)
   virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
   virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

   virtual int get_id(void) { return QUEUE_CMD_FLAG; } // Get this command id

	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};

class Cmd_BuildRoad:public PlayerCommand {
    private:
	Path* path;

	Coords start;
	Path::Step_Vector::size_type nsteps;
	char* steps;

    public:
	Cmd_BuildRoad(void) : PlayerCommand() { } // For savegame loading
	Cmd_BuildRoad (int, int, Path &);
	Cmd_BuildRoad (Deserializer*);

	virtual ~Cmd_BuildRoad ();

   // Write these commands to a file (for savegames)
   virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
   virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

   virtual int get_id(void) { return QUEUE_CMD_BUILDROAD; } // Get this command id


	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};

class Cmd_FlagAction:public PlayerCommand {
    private:
	int serial;
	int action;

    public:
	Cmd_FlagAction(void) : PlayerCommand() { } // For savegame loading
	Cmd_FlagAction (int t, int p, Flag* f, int a):PlayerCommand(t,p)
	{ serial=f->get_serial(); action=a; }

   // Write these commands to a file (for savegames)
   virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
   virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

   virtual int get_id(void) { return QUEUE_CMD_FLAGACTION; } // Get this command id


	Cmd_FlagAction (Deserializer*);

	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};

class Cmd_StartStopBuilding:public PlayerCommand {
    private:
	int serial;

    public:
	Cmd_StartStopBuilding(void) : PlayerCommand() { } // For savegame loading
	Cmd_StartStopBuilding (int t, int p, Building* b):PlayerCommand(t,p)
	{ serial=b->get_serial(); }

   // Write these commands to a file (for savegames)
   virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
   virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

   virtual int get_id(void) { return QUEUE_CMD_STOPBUILDING; } // Get this command id

   Cmd_StartStopBuilding (Deserializer*);

	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};

class Cmd_EnhanceBuilding:public PlayerCommand {
	private:
		int serial;
		int id;

	public:
		Cmd_EnhanceBuilding(void) : PlayerCommand() { } // For savegame loading
		Cmd_EnhanceBuilding (int t, int p, Building* b, int i):PlayerCommand(t,p)
		{ serial=b->get_serial(); id=i; }

   // Write these commands to a file (for savegames)
		virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
		virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

		virtual int get_id(void) { return QUEUE_CMD_ENHANCEBUILDING; } // Get this command id

		Cmd_EnhanceBuilding (Deserializer*);

		virtual void execute (Game* g);
		virtual void serialize (Serializer*);
};


class Cmd_ChangeTrainingOptions:public PlayerCommand {
	private:
		int serial;
		int attribute;
		int value;

	public:
		Cmd_ChangeTrainingOptions(void) : PlayerCommand() { } // For savegame loading
		Cmd_ChangeTrainingOptions(int t, int p, Building* b, int at, int val):PlayerCommand(t,p)
		{ serial=b->get_serial(); attribute=at; value=val; }

   // Write these commands to a file (for savegames)
		virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
		virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

		virtual int get_id(void) { return QUEUE_CMD_CHANGETRAININGOPTIONS; } // Get this command id

		Cmd_ChangeTrainingOptions (Deserializer*);

		virtual void execute (Game* g);
		virtual void serialize (Serializer*);
};

class Cmd_DropSoldier:public PlayerCommand {
	private:
		int serial;
		int soldier;

	public:
		Cmd_DropSoldier (void):PlayerCommand() { } ; // For savegames
		Cmd_DropSoldier(int t, int p, Building* b, int _soldier):PlayerCommand(t,p)
		{ serial=b->get_serial(); soldier=_soldier; }

   // Write these commands to a file (for savegames)
		virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
		virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

		virtual int get_id(void) { return QUEUE_CMD_DROPSOLDIER; } // Get this command id

		Cmd_DropSoldier(Deserializer*);

		virtual void execute (Game* g);
		virtual void serialize (Serializer*);
};

class Cmd_ChangeSoldierCapacity:public PlayerCommand {
	private:
		int serial;
		int val;

	public:
		Cmd_ChangeSoldierCapacity (void):PlayerCommand() { } ; // For savegames
		Cmd_ChangeSoldierCapacity (int t, int p, Building* b, int i):PlayerCommand(t,p)
		{ serial=b->get_serial(); val=i; }

   // Write these commands to a file (for savegames)
		virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
		virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

		virtual int get_id(void) { return QUEUE_CMD_CHANGESOLDIERCAPACITY; } // Get this command id

		Cmd_ChangeSoldierCapacity (Deserializer*);

		virtual void execute (Game* g);
		virtual void serialize (Serializer*);
};

/////////////TESTING STUFF
class Cmd_EnemyFlagAction:public PlayerCommand {
    private:
	int serial;
	int action;
   int attacker;
   int number;
   int type;

    public:
	Cmd_EnemyFlagAction(void) : PlayerCommand() { } // For savegame loading
	Cmd_EnemyFlagAction
		(int t,
		 int p,
		 const Flag * const f,
		 int a,
		 int at,
		 int num,
		 int ty)
		:
		PlayerCommand(t,p)
	{ serial=f->get_serial(); action=a; attacker=at; number=num; type=ty; }

   // Write these commands to a file (for savegames)
   virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
   virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

   virtual int get_id(void) { return QUEUE_CMD_ENEMYFLAGACTION; } // Get this command id


	Cmd_EnemyFlagAction (Deserializer*);

	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};


#endif
