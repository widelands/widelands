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

#include "filesystem.h"
#include "game.h"
#include "instances.h"
#include "network.h"
#include "player.h"
#include "playercommand.h"
#include "wexception.h"
#include "widelands_map_map_object_saver.h"
#include "widelands_map_map_object_loader.h"

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

/*
 * Write this player command to a file. Call this from base classes
 */
#define PLAYER_COMMAND_VERSION 1
void PlayerCommand::PlayerCmdWrite(FileWrite *fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos) {
   // First, write version
   fw->Unsigned16(PLAYER_COMMAND_VERSION);
      
   BaseCommand::BaseCmdWrite(fw,egbase,mos);
   // Now sender
   fw->Unsigned8(sender);
}
void PlayerCommand::PlayerCmdRead(FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* mol) {
   int version=fr->Unsigned16();
   if(version==PLAYER_COMMAND_VERSION) {
      BaseCommand::BaseCmdRead(fr,egbase,mol);
      sender=fr->Unsigned8();
   } else
      throw wexception("Unknown version in PlayerCommand::PlayerCmdRead: %i", version);
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
#define PLAYER_CMD_BULLDOZE_VERSION 1
void Cmd_Bulldoze::Read(FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* mol) {
 int version=fr->Unsigned16();
   if(version==PLAYER_CMD_BULLDOZE_VERSION) {
      // Read Player Command
      PlayerCommand::PlayerCmdRead(fr,egbase,mol);
      int fileserial=fr->Unsigned32();
      assert(mol->is_object_known(fileserial));
      serial=mol->get_object_by_file_index(fileserial)->get_serial();
   } else
      throw wexception("Unknown version in Cmd_Bulldoze::Read: %i", version);
}
void Cmd_Bulldoze::Write(FileWrite *fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos) {
   // First, write version
   fw->Unsigned16(PLAYER_CMD_BULLDOZE_VERSION);
   // Write base classes
   PlayerCommand::PlayerCmdWrite(fw, egbase, mos);
   // Now serial
   Map_Object* obj=egbase->get_objects()->get_object(serial);
   assert(mos->is_object_known(obj));
   fw->Unsigned32(mos->get_object_file_index(obj)); 
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
#define PLAYER_CMD_BUILD_VERSION 1
void Cmd_Build::Read(FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* mol) {
 int version=fr->Unsigned16();
   if(version==PLAYER_CMD_BUILD_VERSION) {
      // Read Player Command
      PlayerCommand::PlayerCmdRead(fr,egbase,mol);
      // id
      id=fr->Unsigned16();
      // Coords
      coords.x=fr->Unsigned16();
      coords.y=fr->Unsigned16();
   } else
      throw wexception("Unknown version in Cmd_Build::Read: %i", version);
}
void Cmd_Build::Write(FileWrite *fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos) {
   // First, write version
   fw->Unsigned16(PLAYER_CMD_BUILD_VERSION);
   // Write base classes
   PlayerCommand::PlayerCmdWrite(fw, egbase, mos);
   // Now id 
   fw->Unsigned16(id); 
   // Now Coords
   fw->Unsigned16(coords.x);
   fw->Unsigned16(coords.y);
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
#define PLAYER_CMD_BUILDFLAG_VERSION 1
void Cmd_BuildFlag::Read(FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* mol) {
 int version=fr->Unsigned16();
   if(version==PLAYER_CMD_BUILDFLAG_VERSION) {
      // Read Player Command
      PlayerCommand::PlayerCmdRead(fr,egbase,mol);
      // Coords
      coords.x=fr->Unsigned16();
      coords.y=fr->Unsigned16();
   } else
      throw wexception("Unknown version in Cmd_BuildFlag::Read: %i", version);
}
void Cmd_BuildFlag::Write(FileWrite *fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos) {
   // First, write version
   fw->Unsigned16(PLAYER_CMD_BUILDFLAG_VERSION);
   // Write base classes
   PlayerCommand::PlayerCmdWrite(fw, egbase, mos);
   // Now Coords
   fw->Unsigned16(coords.x);
   fw->Unsigned16(coords.y);
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
#define PLAYER_CMD_BUILDROAD_VERSION 1
void Cmd_BuildRoad::Read(FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* mol) {
 int version=fr->Unsigned16();
   if(version==PLAYER_CMD_BUILDROAD_VERSION) {
      // Read Player Command
      PlayerCommand::PlayerCmdRead(fr,egbase,mol);
      // Start Coords
      start.x=fr->Unsigned16();
      start.y=fr->Unsigned16();
      // Now read nsteps
      nsteps=fr->Unsigned16();
      steps= new char[nsteps];
      
      for (int i=0;i<nsteps;i++)
         steps[i]=fr->Unsigned8();
   } else
      throw wexception("Unknown version in Cmd_BuildRoad::Read: %i", version);
}
void Cmd_BuildRoad::Write(FileWrite *fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos) {
   // First, write version
   fw->Unsigned16(PLAYER_CMD_BUILDROAD_VERSION);
   // Write base classes
   PlayerCommand::PlayerCmdWrite(fw, egbase, mos);
   // Now Start Coords
   fw->Unsigned16(start.x);
   fw->Unsigned16(start.y);

   // Now nsteps
   fw->Unsigned16(nsteps);
	for (int i=0;i<nsteps;i++)
		fw->Unsigned8(path ? path->get_step(i) : steps[i]);
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
#define PLAYER_CMD_FLAGACTION_VERSION 1
void Cmd_FlagAction::Read(FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* mol) {
 int version=fr->Unsigned16();
   if(version==PLAYER_CMD_FLAGACTION_VERSION) {
      // Read Player Command
      PlayerCommand::PlayerCmdRead(fr,egbase,mol);
   
      // action
      action=fr->Unsigned8();

      // Serial
      int fileserial=fr->Unsigned32();
      assert(mol->is_object_known(fileserial));
      serial=mol->get_object_by_file_index(fileserial)->get_serial();
   } else
      throw wexception("Unknown version in Cmd_FlagAction::Read: %i", version);
}
void Cmd_FlagAction::Write(FileWrite *fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos) {
   // First, write version
   fw->Unsigned16(PLAYER_CMD_FLAGACTION_VERSION);
   // Write base classes
   PlayerCommand::PlayerCmdWrite(fw, egbase, mos);
   // Now action
   fw->Unsigned8(action);

   // Now serial
   Map_Object* obj=egbase->get_objects()->get_object(serial);
   assert(mos->is_object_known(obj));
   fw->Unsigned32(mos->get_object_file_index(obj)); 
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
#define PLAYER_CMD_STOPBUILDING_VERSION 1
void Cmd_StartStopBuilding::Read(FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* mol) {
 int version=fr->Unsigned16();
   if(version==PLAYER_CMD_STOPBUILDING_VERSION) {
      // Read Player Command
      PlayerCommand::PlayerCmdRead(fr,egbase,mol);
   
      // Serial
      int fileserial=fr->Unsigned32();
      assert(mol->is_object_known(fileserial));
      serial=mol->get_object_by_file_index(fileserial)->get_serial();
   } else
      throw wexception("Unknown version in Cmd_StartStopBuilding::Read: %i", version);
}
void Cmd_StartStopBuilding::Write(FileWrite *fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos) {
   // First, write version
   fw->Unsigned16(PLAYER_CMD_STOPBUILDING_VERSION);
   // Write base classes
   PlayerCommand::PlayerCmdWrite(fw, egbase, mos);

   // Now serial
   Map_Object* obj=egbase->get_objects()->get_object(serial);
   assert(mos->is_object_known(obj));
   fw->Unsigned32(mos->get_object_file_index(obj)); 
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
#define PLAYER_CMD_ENHANCEBUILDING_VERSION 1
void Cmd_EnhanceBuilding::Read(FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* mol) {
 int version=fr->Unsigned16();
   if(version==PLAYER_CMD_ENHANCEBUILDING_VERSION) {
      // Read Player Command
      PlayerCommand::PlayerCmdRead(fr,egbase,mol);
   
      // Serial
      int fileserial=fr->Unsigned32();
      assert(mol->is_object_known(fileserial));
      serial=mol->get_object_by_file_index(fileserial)->get_serial();

      // id
      id=fr->Unsigned16();
      
   } else
      throw wexception("Unknown version in Cmd_EnhanceBuilding::Read: %i", version);
}
void Cmd_EnhanceBuilding::Write(FileWrite *fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos) {
   // First, write version
   fw->Unsigned16(PLAYER_CMD_ENHANCEBUILDING_VERSION);
   // Write base classes
   PlayerCommand::PlayerCmdWrite(fw, egbase, mos);

   // Now serial
   Map_Object* obj=egbase->get_objects()->get_object(serial);
   assert(mos->is_object_known(obj));
   fw->Unsigned32(mos->get_object_file_index(obj)); 

   // Now id
   fw->Unsigned16(id);
}


