/*
 * Copyright (C) 2004 by the Widelands Development Team
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
	virtual ~PlayerCommand ();
	
	char get_sender() const { return sender; }
	
	virtual void serialize (Serializer*)=0;
	static PlayerCommand* deserialize (Deserializer*);
};

class Cmd_Bulldoze:public PlayerCommand {
    private:
	int serial;

    public:
	Cmd_Bulldoze (int t, int p, PlayerImmovable* pi):PlayerCommand(t,p)
	{ serial=pi->get_serial(); }
	
	Cmd_Bulldoze (Deserializer*);
	
	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};

class Cmd_Build:public PlayerCommand {
    private:
	Coords coords;
	int id;

    public:
	Cmd_Build (int t, int p, const Coords& c, int i):PlayerCommand(t,p)
	{ coords=c; id=i; }
	
	Cmd_Build (Deserializer*);
	
	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};

class Cmd_BuildFlag:public PlayerCommand {
    private:
	Coords coords;

    public:
	Cmd_BuildFlag (int t, int p, const Coords& c):PlayerCommand(t,p)
	{ coords=c; }
	
	Cmd_BuildFlag (Deserializer*);
	
	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};

class Cmd_BuildRoad:public PlayerCommand {
    private:
	Path* path;
	
	Coords start;
	int nsteps;
	char* steps;

    public:
	Cmd_BuildRoad (int, int, Path*);
	Cmd_BuildRoad (Deserializer*);
	
	virtual ~Cmd_BuildRoad ();
	
	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};

class Cmd_FlagAction:public PlayerCommand {
    private:
	int serial;
	int action;

    public:
	Cmd_FlagAction (int t, int p, Flag* f, int a):PlayerCommand(t,p)
	{ serial=f->get_serial(); action=a; }
	
	Cmd_FlagAction (Deserializer*);
	
	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};

class Cmd_StartStopBuilding:public PlayerCommand {
    private:
	int serial;

    public:
	Cmd_StartStopBuilding (int t, int p, Building* b):PlayerCommand(t,p)
	{ serial=b->get_serial(); }
	
	Cmd_StartStopBuilding (Deserializer*);
	
	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};

class Cmd_EnhanceBuilding:public PlayerCommand {
    private:
	int serial;
	int id;

    public:
	Cmd_EnhanceBuilding (int t, int p, Building* b, int i):PlayerCommand(t,p)
	{ serial=b->get_serial(); id=i; }
	
	Cmd_EnhanceBuilding (Deserializer*);
	
	virtual void execute (Game* g);
	virtual void serialize (Serializer*);
};


#endif

