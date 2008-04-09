/*
 * Copyright (C) 2004, 2007-2008 by the Widelands Development Team
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

#include "playercommand.h"

#include "log.h"
#include "game.h"
#include "instances.h"
#include "player.h"
#include "productionsite.h"
#include "soldier.h"
#include "streamwrite.h"
#include "wexception.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_map_object_saver.h"
#include "widelands_map_map_object_loader.h"

#include "upcast.h"

namespace Widelands {

enum {
	PLCMD_UNUSED=0,
	PLCMD_BULLDOZE,
	PLCMD_BUILD,
	PLCMD_BUILDFLAG,
	PLCMD_BUILDROAD,
	PLCMD_FLAGACTION,
	PLCMD_STARTSTOPBUILDING,
	PLCMD_ENHANCEBUILDING,
	PLCMD_CHANGETRAININGOPTIONS,
	PLCMD_DROPSOLDIER,
	PLCMD_CHANGESOLDIERCAPACITY,
	PLCMD_ENEMYFLAGACTION,
	PLCMD_SETWAREPRIORITY
};

/*** class PlayerCommand ***/

PlayerCommand::PlayerCommand (int32_t const time, Player_Number const s)
	: GameLogicCommand (time), sender(s), cmdserial(0)
{}

PlayerCommand::~PlayerCommand () {}

PlayerCommand* PlayerCommand::deserialize (StreamRead & des)
{
	switch (des.Unsigned8()) {
	case PLCMD_BULLDOZE:              return new Cmd_Bulldoze             (des);
	case PLCMD_BUILD:                 return new Cmd_Build                (des);
	case PLCMD_BUILDFLAG:             return new Cmd_BuildFlag            (des);
	case PLCMD_BUILDROAD:             return new Cmd_BuildRoad            (des);
	case PLCMD_FLAGACTION:            return new Cmd_FlagAction           (des);
	case PLCMD_STARTSTOPBUILDING:     return new Cmd_StartStopBuilding    (des);
	case PLCMD_ENHANCEBUILDING:       return new Cmd_EnhanceBuilding      (des);
	case PLCMD_SETWAREPRIORITY:       return new Cmd_SetWarePriority      (des);
	case PLCMD_CHANGETRAININGOPTIONS: return new Cmd_ChangeTrainingOptions(des);
	case PLCMD_DROPSOLDIER:           return new Cmd_DropSoldier          (des);
	case PLCMD_CHANGESOLDIERCAPACITY: return new Cmd_ChangeSoldierCapacity(des);
	case PLCMD_ENEMYFLAGACTION:       return new Cmd_EnemyFlagAction      (des);
	default:
		throw wexception
			("PlayerCommand::deserialize(): Invalid command id encountered");
	}
}

/*
 * Write this player command to a file. Call this from base classes
 */
#define PLAYER_COMMAND_VERSION 2
void PlayerCommand::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_COMMAND_VERSION);

	GameLogicCommand::Write(fw, egbase, mos);
	// Now sender
	fw.Unsigned8 (sender);
	fw.Unsigned32 (cmdserial);
}

void PlayerCommand::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (1 <= packet_version and packet_version <= PLAYER_COMMAND_VERSION) {
			GameLogicCommand::Read(fr, egbase, mol);
			sender    = fr.Unsigned8 ();
			cmdserial = 1 < packet_version ? fr.Unsigned32() : 0;
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("player command: %s", e.what());
	}
}

/*** class Cmd_Bulldoze ***/

Cmd_Bulldoze::Cmd_Bulldoze (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
}

void Cmd_Bulldoze::execute (Game* g)
{
	if (upcast(PlayerImmovable, pimm, g->objects().get_object(serial)))
		g->get_player(get_sender())->bulldoze(pimm);
}

void Cmd_Bulldoze::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_BULLDOZE);
	ser.Unsigned8 (get_sender());
	ser.Unsigned32(serial);
}
#define PLAYER_CMD_BULLDOZE_VERSION 1
void Cmd_Bulldoze::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_BULLDOZE_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			Serial const pimm_serial = fr.Unsigned32();
			try {
				serial = mol.get<Map_Object>(pimm_serial).get_serial();
			} catch (_wexception const & e) {
				throw wexception("map object %u: %s", pimm_serial, e.what());
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("bulldoze: %s", e.what());
	}
}
void Cmd_Bulldoze::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_BULLDOZE_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	// Now serial
	const Map_Object * const obj = egbase.objects().get_object(serial);
	assert(mos.is_object_known(obj));
	fw.Unsigned32(mos.get_object_file_index(obj));
}

/*** class Cmd_Build ***/

Cmd_Build::Cmd_Build (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	id     = static_cast<Building_Index::value_t>(des.Signed16  ());
	coords = des.Coords32  ();
}

void Cmd_Build::execute (Game* g)
{
	Player *player = g->get_player(get_sender());
	player->build(coords, id);
}

void Cmd_Build::serialize (StreamWrite & ser) {
	ser.Unsigned8 (PLCMD_BUILD);
	ser.Unsigned8 (get_sender());
	ser.Signed16  (id.value());
	ser.Coords32  (coords);
}
#define PLAYER_CMD_BUILD_VERSION 1
void Cmd_Build::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_BUILD_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			id     = static_cast<Building_Index::value_t>(fr.Unsigned16());
			coords = fr.Coords32  (egbase.map().extent());
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("build: %s", e.what());
	}
}

void Cmd_Build::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_BUILD_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	fw.Unsigned16(id.value());
	fw.Coords32  (coords);
}


/*** class Cmd_BuildFlag ***/

Cmd_BuildFlag::Cmd_BuildFlag (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	coords = des.Coords32  ();
}

void Cmd_BuildFlag::execute (Game* g)
{
	Player *player = g->get_player(get_sender());
	player->build_flag(coords);
}

void Cmd_BuildFlag::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_BUILDFLAG);
	ser.Unsigned8 (get_sender());
	ser.Coords32  (coords);
}
#define PLAYER_CMD_BUILDFLAG_VERSION 1
void Cmd_BuildFlag::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_BUILDFLAG_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			coords = fr.Coords32(egbase.map().extent());
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("build flag: %s", e.what());
	}
}
void Cmd_BuildFlag::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_BUILDFLAG_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	fw.Coords32  (coords);
}

/*** class Cmd_BuildRoad ***/

Cmd_BuildRoad::Cmd_BuildRoad (int32_t t, int32_t p, Path & pa) :
PlayerCommand(t, p),
path         (&pa),
start        (pa.get_start()),
nsteps       (pa.get_nsteps()),
steps        (0)
{}

Cmd_BuildRoad::Cmd_BuildRoad (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	start  = des.Coords32  ();
	nsteps = des.Unsigned16();

	// we cannot completely deserialize the path here because we don't have a Map
	path=0;
	steps=new char[nsteps];

	for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
		steps[i] = des.Unsigned8();
}

Cmd_BuildRoad::~Cmd_BuildRoad ()
{
	delete path;

	delete[] steps;
}

void Cmd_BuildRoad::execute (Game* g)
{
	if (path==0) {
		assert (steps!=0);

		path = new Path(start);
		for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
			path->append (g->map(), steps[i]);
	}

	Player *player = g->get_player(get_sender());
	player->build_road(*path);
}

void Cmd_BuildRoad::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_BUILDROAD);
	ser.Unsigned8 (get_sender());
	ser.Coords32  (start);
	ser.Unsigned16(nsteps);

	assert (path!=0 || steps!=0);

	for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
		ser.Unsigned8(path ? (*path)[i] : steps[i]);
}
#define PLAYER_CMD_BUILDROAD_VERSION 1
void Cmd_BuildRoad::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_BUILDROAD_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			start  = fr.Coords32  (egbase.map().extent());
			nsteps = fr.Unsigned16();
			path = 0;
			steps = new char[nsteps];
			for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
			steps[i] = fr.Unsigned8();
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("build road: %s", e.what());
	}
}
void Cmd_BuildRoad::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_BUILDROAD_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	fw.Coords32  (start);
	fw.Unsigned16(nsteps);
	for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
		fw.Unsigned8(path ? (*path)[i] : steps[i]);
}


/*** Cmd_FlagAction ***/
Cmd_FlagAction::Cmd_FlagAction (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	action = des.Unsigned8 ();
	serial = des.Unsigned32();
}

void Cmd_FlagAction::execute (Game* g)
{
	Player* player = g->get_player(get_sender());
	if (upcast(Flag, flag, g->objects().get_object(serial)))
		if (flag->get_owner() == player)
			player->flagaction (flag, action);
}

void Cmd_FlagAction::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_FLAGACTION);
	ser.Unsigned8 (get_sender());
	ser.Unsigned8 (action);
	ser.Unsigned32(serial);
}

#define PLAYER_CMD_FLAGACTION_VERSION 1
void Cmd_FlagAction::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_FLAGACTION_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			action                     = fr.Unsigned8 ();
			uint32_t const flag_serial = fr.Unsigned32();
			try {
				serial = mol.get<Map_Object>(flag_serial).get_serial();
			} catch (_wexception const & e) {
				throw wexception("flag %u: %s", flag_serial, e.what());
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("flag action: %s", e.what());
	}
}
void Cmd_FlagAction::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_FLAGACTION_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	// Now action
	fw.Unsigned8 (action);

	// Now serial
	const Map_Object * const obj = egbase.objects().get_object(serial);
	assert(mos.is_object_known(obj));
	fw.Unsigned32(mos.get_object_file_index(obj));
}

/*** Cmd_StartStopBuilding ***/

Cmd_StartStopBuilding::Cmd_StartStopBuilding (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial=des.Unsigned32();
}

void Cmd_StartStopBuilding::execute (Game* g)
{
	if (upcast(Building, building, g->objects().get_object(serial)))
		g->get_player(get_sender())->start_stop_building(building);
}

void Cmd_StartStopBuilding::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_STARTSTOPBUILDING);
	ser.Unsigned8 (get_sender());
	ser.Unsigned32(serial);
}
#define PLAYER_CMD_STOPBUILDING_VERSION 1
void Cmd_StartStopBuilding::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_STOPBUILDING_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			uint32_t const building_serial = fr.Unsigned32();
			try {
				serial = mol.get<Map_Object>(building_serial).get_serial();
			} catch (_wexception const & e) {
				throw wexception("building %u: %s", building_serial, e.what());
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("start/stop building: %s", e.what());
	}
}
void Cmd_StartStopBuilding::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_STOPBUILDING_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	const Map_Object * const obj = egbase.objects().get_object(serial);
	assert(mos.is_object_known(obj));
	fw.Unsigned32(mos.get_object_file_index(obj));
}


/*** Cmd_EnhanceBuilding ***/

Cmd_EnhanceBuilding::Cmd_EnhanceBuilding (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	id     = static_cast<Building_Index::value_t>(des.Unsigned16());
}

void Cmd_EnhanceBuilding::execute (Game* g)
{
	if (upcast(Building, building, g->objects().get_object(serial)))
		g->get_player(get_sender())->enhance_building(building, id);
}

void Cmd_EnhanceBuilding::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_ENHANCEBUILDING);
	ser.Unsigned8 (get_sender());
	ser.Unsigned32(serial);
	ser.Unsigned16(id.value());
}
#define PLAYER_CMD_ENHANCEBUILDING_VERSION 1
void Cmd_EnhanceBuilding::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_ENHANCEBUILDING_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			uint32_t const building_serial = fr.Unsigned32();
			try {
				serial = mol.get<Map_Object>(building_serial).get_serial();
			} catch (_wexception const & e) {
				throw wexception("building %u: %s", building_serial, e.what());
			}
			id = static_cast<Building_Index::value_t>(fr.Unsigned16());
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("enhance building: %s", e.what());
	}
}
void Cmd_EnhanceBuilding::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_ENHANCEBUILDING_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	const Map_Object * const obj = egbase.objects().get_object(serial);
	assert(mos.is_object_known(obj));
	fw.Unsigned32(mos.get_object_file_index(obj));

	// Now id
	fw.Unsigned16(id.value());
}


/*** class Cmd_SetWarePriority ***/
Cmd_SetWarePriority::Cmd_SetWarePriority
	(int32_t duetime, Player_Number _sender,
	 PlayerImmovable* imm,
	 int32_t type, Ware_Index index, int32_t priority)
	: PlayerCommand(duetime, _sender)
{
	m_serial = imm->get_serial();
	m_type = type;
	m_index = index;
	m_priority = priority;
}

void Cmd_SetWarePriority::execute(Game* g)
{
	upcast(ProductionSite, psite, g->objects().get_object(m_serial));

	if (!psite)
		return;
	if (psite->get_owner()->get_player_number() != get_sender())
		return;

	psite->set_priority(m_type, m_index.value(), m_priority);
}

#define PLAYER_CMD_SETWAREPRIORITY_VERSION 1

void Cmd_SetWarePriority::Write(FileWrite& fw, Editor_Game_Base& egbase, Map_Map_Object_Saver& mos)
{
	fw.Unsigned16(PLAYER_CMD_SETWAREPRIORITY_VERSION);

	PlayerCommand::Write(fw, egbase, mos);

	const Map_Object * const obj = egbase.objects().get_object(m_serial);
	fw.Unsigned32(mos.get_object_file_index(obj));
	fw.Unsigned8(m_type);
	fw.Signed32(m_index.value());
	fw.Signed32(m_priority);
}

void Cmd_SetWarePriority::Read(FileRead& fr, Editor_Game_Base& egbase, Map_Map_Object_Loader& mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SETWAREPRIORITY_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			uint32_t const serial = fr.Unsigned32();
			try {
				m_serial = mol.get<Map_Object>(serial).get_serial();
			} catch (_wexception const & e) {
				throw wexception("site %u: %s", serial, e.what());
			}

			m_type = fr.Unsigned8();
			m_index = static_cast<Ware_Index::value_t>(fr.Signed32());
			m_priority = fr.Signed32();
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("enhance building: %s", e.what());
	}
}

Cmd_SetWarePriority::Cmd_SetWarePriority(StreamRead& des)
	: PlayerCommand(0, des.Unsigned8())
{
	m_serial = des.Unsigned32();
	m_type = des.Unsigned8();
	m_index = static_cast<Ware_Index::value_t>(des.Signed32());
	m_priority = des.Signed32();
}

void Cmd_SetWarePriority::serialize(StreamWrite& ser)
{
	ser.Unsigned8(PLCMD_SETWAREPRIORITY);
	ser.Unsigned8(get_sender());
	ser.Unsigned32(m_serial);
	ser.Unsigned8(m_type);
	ser.Signed32(m_index.value());
	ser.Signed32(m_priority);
}


/*** class Cmd_ChangeTrainingOptions ***/
Cmd_ChangeTrainingOptions::Cmd_ChangeTrainingOptions(StreamRead & des)
:
PlayerCommand (0, des.Unsigned8())
{
	serial    = des.Unsigned32();  //  Serial of the building
	attribute = des.Unsigned16();  //  Attribute to modify
	value     = des.Unsigned16();  //  New vale
}

void Cmd_ChangeTrainingOptions::execute (Game* g)
{
	/* � Maybe we must check that the building is a training house ? */
	if (upcast(Building, building, g->objects().get_object(serial)))
		g->get_player(get_sender())->change_training_options
			(building, attribute, value);
}

void Cmd_ChangeTrainingOptions::serialize (StreamWrite & ser) {
	ser.Unsigned8 (PLCMD_CHANGETRAININGOPTIONS);
	ser.Unsigned8 (get_sender());
	ser.Unsigned32(serial);
	ser.Unsigned16(attribute);
	ser.Unsigned16(value);
}


#define PLAYER_CMD_CHANGETRAININGOPTIONS_VERSION 1
void Cmd_ChangeTrainingOptions::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_CHANGETRAININGOPTIONS_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			uint32_t const trainingsite_serial = fr.Unsigned32();
			try {
				serial    = mol.get<Map_Object>(trainingsite_serial).get_serial();
			} catch (_wexception const & e) {
				throw wexception
					("trainingsite %u: %s", trainingsite_serial, e.what());
			}
			attribute = fr.Unsigned16();
			value     = fr.Unsigned16();
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("change training options: %s", e.what());
	}
}

void Cmd_ChangeTrainingOptions::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_CHANGETRAININGOPTIONS_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	const Map_Object * const obj = egbase.objects().get_object(serial);
	assert(mos.is_object_known(obj));
	fw.Unsigned32(mos.get_object_file_index(obj));

	fw.Unsigned16(attribute);
	fw.Unsigned16(value);
}

/*** class Cmd_DropSoldier ***/

Cmd_DropSoldier::Cmd_DropSoldier(StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial  = des.Unsigned32(); //  Serial of the building
	soldier = des.Unsigned32(); //  Serial of soldier
}

void Cmd_DropSoldier::execute (Game* g)
{
	if (upcast(PlayerImmovable, player_imm, g->objects().get_object(serial)))
		if (upcast(Soldier, s, g->objects().get_object(soldier)))
			g->player(get_sender()).drop_soldier(player_imm, s);
}

void Cmd_DropSoldier::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_DROPSOLDIER);
	ser.Unsigned8 (get_sender());
	ser.Unsigned32(serial);
	ser.Unsigned32(soldier);
}

#define PLAYER_CMD_DROPSOLDIER_VERSION 1
void Cmd_DropSoldier::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_DROPSOLDIER_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			uint32_t const militarysite_serial = fr.Unsigned32();
			try {
				serial  = mol.get<Map_Object>(militarysite_serial).get_serial();
			} catch (_wexception const & e) {
				throw wexception
					("militarysite %u: %s", militarysite_serial, e.what());
			}
			uint32_t const soldier_serial = fr.Unsigned32();
			try {
				soldier = mol.get<Map_Object>(soldier_serial).get_serial();
			} catch (_wexception const & e) {
				throw wexception
					("soldier %u: %s",      soldier_serial,      e.what());
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("drop soldier: %s", e.what());
	}
}

void Cmd_DropSoldier::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_DROPSOLDIER_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	{
		const Map_Object * const obj = egbase.objects().get_object(serial);
		assert(mos.is_object_known(obj));
		fw.Unsigned32(mos.get_object_file_index(obj));
	}

	// Now soldier serial
	{
		const Map_Object * const obj = egbase.objects().get_object(serial);
		assert(mos.is_object_known(obj));
		fw.Unsigned16(mos.get_object_file_index(obj));
	}

}

/*** Cmd_ChangeSoldierCapacity ***/

Cmd_ChangeSoldierCapacity::Cmd_ChangeSoldierCapacity(StreamRead & des)
:
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	val    = des.Signed16();
}

void Cmd_ChangeSoldierCapacity::execute (Game* g)
{
	if (upcast(Building, building, g->objects().get_object(serial)))
		g->get_player(get_sender())->change_soldier_capacity(building, val);
}

void Cmd_ChangeSoldierCapacity::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_CHANGESOLDIERCAPACITY);
	ser.Unsigned8 (get_sender());
	ser.Unsigned32(serial);
	ser.Signed16(val);
}

#define PLAYER_CMD_CHANGESOLDIERCAPACITY_VERSION 1
void Cmd_ChangeSoldierCapacity::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_CHANGESOLDIERCAPACITY_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			uint32_t const militarysite_serial = fr.Unsigned32();
			try {
				serial = mol.get<Map_Object>(militarysite_serial).get_serial();
			} catch (_wexception const & e) {
				throw wexception
					("militarysite %u: %s", militarysite_serial, e.what());
			}
			val = fr.Signed16();
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("change soldier capacity: %s", e.what());
	}
}

void Cmd_ChangeSoldierCapacity::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_CHANGESOLDIERCAPACITY_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	const Map_Object * const obj = egbase.objects().get_object(serial);
	assert(mos.is_object_known(obj));
	fw.Unsigned32(mos.get_object_file_index(obj));

	// Now capacity
	fw.Signed16(val);

}

/*** Cmd_EnemyFlagAction ***/

Cmd_EnemyFlagAction::Cmd_EnemyFlagAction (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	action   = des.Unsigned8 ();
	serial   = des.Unsigned32();
	attacker = des.Unsigned8 ();
	number   = des.Unsigned8 ();
	type     = des.Unsigned8 ();
}

void Cmd_EnemyFlagAction::execute (Game* g)
{
	Player* player = g->get_player(get_sender());
	Map_Object* obj = g->objects().get_object(serial);
	PlayerImmovable* imm = static_cast<PlayerImmovable*>(obj);

	Player* real_player = g->get_player(attacker);

	log
		("player(%d)    imm->get_owner (%d)   real_player (%d)\n",
		 player->get_player_number(),
		 imm->get_owner()->get_player_number(),
		 real_player->get_player_number());

	if (upcast(Flag, flag, obj)) {
		if (Building const * const building = flag->get_building())
			if
				(imm->get_owner() != real_player
				 and
				 1
				 <
				 real_player->vision
				 	(Map::get_index
				 	 	(building->get_position(), g->map().get_width())))
			real_player->enemyflagaction (flag, action, attacker, number, type);
	} else
		log ("Cmd_EnemyFlagAction Player invalid or not seeing target.\n");
}

void Cmd_EnemyFlagAction::serialize (StreamWrite & ser) {
	ser.Unsigned8 (PLCMD_ENEMYFLAGACTION);
	ser.Unsigned8 (get_sender());
	ser.Unsigned8 (action);
	ser.Unsigned32(serial);
	ser.Unsigned8 (attacker);
	ser.Unsigned8 (number);
	ser.Unsigned8 (type);
}
#define PLAYER_CMD_ENEMYFLAGACTION_VERSION 2
void Cmd_EnemyFlagAction::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_ENEMYFLAGACTION_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			action   = fr.Unsigned8 ();
			uint32_t const flag_serial = fr.Unsigned32();
			try {
				serial = mol.get<Map_Object>(flag_serial).get_serial();
			} catch (_wexception const & e) {
				throw wexception("flag %u: %s", flag_serial, e.what());
			}
			attacker = fr.Unsigned8 ();
			number   = fr.Unsigned8 ();
			type     = fr.Unsigned8 ();
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("enemy flag action: %s", e.what());
	}
}

void Cmd_EnemyFlagAction::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_ENEMYFLAGACTION_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	// Now action
	fw.Unsigned8 (action);

	// Now serial
	const Map_Object * const obj = egbase.objects().get_object(serial);
	assert(mos.is_object_known(obj));
	fw.Unsigned32(mos.get_object_file_index(obj));

	// Now param
	fw.Unsigned8 (attacker);
	fw.Unsigned8 (number);
	fw.Unsigned8 (type);
}

};
