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

#include "wexception.h"
#include "player.h"
#include "game.h"
#include "playercommand.h"
#include "network.h"

enum {
	PLCMD_UNUSED=0,
	PLCMD_BULLDOZE,
	PLCMD_BUILD,
	PLCMD_BUILDFLAG,
	PLCMD_BUILDROAD,
	PLCMD_FLAGACTION,
	PLCMD_STARTSTOPBUILDING,
	PLCMD_ENHANCEBUILDING	
};

/*** class PlayerCommand ***/

PlayerCommand::PlayerCommand (int t, char s):BaseCommand (t)
{
	sender=s;
}

PlayerCommand::~PlayerCommand ()
{
}

PlayerCommand* PlayerCommand::deserialize (Deserializer* des)
{
	switch (des->getchar()) {
	    case PLCMD_BULLDOZE:
		return new Cmd_Bulldoze(des);
	    case PLCMD_BUILD:
		return new Cmd_Build(des);
	    case PLCMD_BUILDFLAG:
		return new Cmd_BuildFlag(des);
	    case PLCMD_BUILDROAD:
		return new Cmd_BuildRoad(des);
	    case PLCMD_FLAGACTION:
		return new Cmd_FlagAction(des);
	    case PLCMD_STARTSTOPBUILDING:
		return new Cmd_StartStopBuilding(des);
	    case PLCMD_ENHANCEBUILDING:
		return new Cmd_EnhanceBuilding(des);
	    default:
		throw wexception("PlayerCommand::deserialize(): Invalid command id encountered");
	}
}

/*** class Cmd_Bulldoze ***/

Cmd_Bulldoze::Cmd_Bulldoze (Deserializer* des):PlayerCommand (0, des->getchar())
{
	serial=des->getlong();
}

void Cmd_Bulldoze::execute (Game* g)
{
	Player* player = g->get_player(get_sender());
	Map_Object* obj = g->get_objects()->get_object(serial);

	if (obj && obj->get_type() >= Map_Object::BUILDING)
		player->bulldoze(static_cast<PlayerImmovable*>(obj));
}

void Cmd_Bulldoze::serialize (Serializer* ser)
{
	ser->putchar (PLCMD_BULLDOZE);
	ser->putchar (get_sender());
	ser->putlong (serial);
}

/*** class Cmd_Build ***/

Cmd_Build::Cmd_Build (Deserializer* des):PlayerCommand (0, des->getchar())
{
	id=des->getshort();
	coords.x=des->getshort();
	coords.y=des->getshort();
}

void Cmd_Build::execute (Game* g)
{
	Player *player = g->get_player(get_sender());
	player->build(coords, id);
}

void Cmd_Build::serialize (Serializer* ser)
{
	ser->putchar (PLCMD_BUILD);
	ser->putchar (get_sender());
	ser->putshort (id);
	ser->putshort (coords.x);
	ser->putshort (coords.y);
}

/*** class Cmd_BuildFlag ***/

Cmd_BuildFlag::Cmd_BuildFlag (Deserializer* des):PlayerCommand (0, des->getchar())
{
	coords.x=des->getshort();
	coords.y=des->getshort();
}

void Cmd_BuildFlag::execute (Game* g)
{
	Player *player = g->get_player(get_sender());
	player->build_flag(coords);
}

void Cmd_BuildFlag::serialize (Serializer* ser)
{
	ser->putchar (PLCMD_BUILDFLAG);
	ser->putchar (get_sender());
	ser->putshort (coords.x);
	ser->putshort (coords.y);
}

/*** class Cmd_BuildRoad ***/

Cmd_BuildRoad::Cmd_BuildRoad (int t, int p, Path* pa):PlayerCommand(t,p)
{
	path=pa;
	start=path->get_start();
	nsteps=path->get_nsteps();
	steps=0;
}
	
Cmd_BuildRoad::Cmd_BuildRoad (Deserializer* des):PlayerCommand (0, des->getchar())
{
	int i;
	
	start.x=des->getshort();
	start.y=des->getshort();
	nsteps=des->getshort();
	
	// we cannot completely deserialize the path here because we don't have a Map
	path=0;
	steps=new char[nsteps];
	
	for (i=0;i<nsteps;i++)
	    steps[i]=des->getchar();
}

Cmd_BuildRoad::~Cmd_BuildRoad ()
{
	if (path!=0)
		delete path;
	
	if (steps!=0)
		delete[] steps;
}

void Cmd_BuildRoad::execute (Game* g)
{
	if (path==0) {
		assert (steps!=0);
		
		path=new Path(g->get_map(), start);
		for (int i=0; i<nsteps; i++)
			path->append (steps[i]);
	}
	
	Player *player = g->get_player(get_sender());
	player->build_road(path);
}

void Cmd_BuildRoad::serialize (Serializer* ser)
{
	ser->putchar (PLCMD_BUILDROAD);
	ser->putchar (get_sender());
	ser->putshort (start.x);
	ser->putshort (start.y);
	ser->putshort (nsteps);
	
	assert (path!=0 || steps!=0);
	
	for (int i=0;i<nsteps;i++)
		ser->putchar (path ? path->get_step(i) : steps[i]);
}

/*** Cmd_FlagAction ***/

Cmd_FlagAction::Cmd_FlagAction (Deserializer* des):PlayerCommand (0, des->getchar())
{
	action=des->getchar();
	serial=des->getlong();
}

void Cmd_FlagAction::execute (Game* g)
{
	Player* player = g->get_player(get_sender());
	Map_Object* obj = g->get_objects()->get_object(serial);

	if (obj && obj->get_type() == Map_Object::FLAG && static_cast<PlayerImmovable*>(obj)->get_owner() == player)
		player->flagaction (static_cast<Flag*>(obj), action);
}

void Cmd_FlagAction::serialize (Serializer* ser)
{
	ser->putchar (PLCMD_FLAGACTION);
	ser->putchar (get_sender());
	ser->putchar (action);
	ser->putlong (serial);
}

/*** Cmd_StartStopBuilding ***/

Cmd_StartStopBuilding::Cmd_StartStopBuilding (Deserializer* des):PlayerCommand (0, des->getchar())
{
	serial=des->getlong();
}

void Cmd_StartStopBuilding::execute (Game* g)
{
	Player* player = g->get_player(get_sender());
	Map_Object* obj = g->get_objects()->get_object(serial);

	if (obj && obj->get_type() >= Map_Object::BUILDING)
		player->start_stop_building(static_cast<PlayerImmovable*>(obj));
}

void Cmd_StartStopBuilding::serialize (Serializer* ser)
{
	ser->putchar (PLCMD_STARTSTOPBUILDING);
	ser->putchar (get_sender());
	ser->putlong (serial);
}

/*** Cmd_EnhanceBuilding ***/

Cmd_EnhanceBuilding::Cmd_EnhanceBuilding (Deserializer* des):PlayerCommand (0, des->getchar())
{
	serial=des->getlong();
	id=des->getshort();
}

void Cmd_EnhanceBuilding::execute (Game* g)
{
	Player* player = g->get_player(get_sender());
	Map_Object* obj = g->get_objects()->get_object(serial);

	if (obj && obj->get_type() >= Map_Object::BUILDING)
    		player->enhance_building(static_cast<PlayerImmovable*>(obj), id);
}

void Cmd_EnhanceBuilding::serialize (Serializer* ser)
{
	ser->putchar (PLCMD_ENHANCEBUILDING);
	ser->putchar (get_sender());
	ser->putlong (serial);
	ser->putshort (id);
}

