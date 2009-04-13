/*
 * Copyright (C) 2004, 2007-2009 by the Widelands Development Team
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
#include "soldier.h"
#include "streamwrite.h"
#include "tribe.h"
#include "wexception.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_map_object_saver.h"
#include "widelands_map_map_object_loader.h"

#include "upcast.h"

namespace Widelands {

enum {
	PLCMD_UNUSED = 0,
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
	PLCMD_SETWAREPRIORITY,
	PLCMD_SETTARGETQUANTITY,
	PLCMD_RESETTARGETQUANTITY
};

/*** class PlayerCommand ***/

PlayerCommand::PlayerCommand (int32_t const time, Player_Number const s)
	: GameLogicCommand (time), m_sender(s), m_cmdserial(0)
{}

PlayerCommand * PlayerCommand::deserialize (StreamRead & des)
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
	case PLCMD_SETTARGETQUANTITY:     return new Cmd_SetTargetQuantity    (des);
	case PLCMD_RESETTARGETQUANTITY:   return new Cmd_ResetTargetQuantity  (des);
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
	fw.Unsigned8  (sender   ());
	fw.Unsigned32 (cmdserial());
}

void PlayerCommand::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (1 <= packet_version and packet_version <= PLAYER_COMMAND_VERSION) {
			GameLogicCommand::Read(fr, egbase, mol);
			m_sender    = fr.Unsigned8 ();
			m_cmdserial = 1 < packet_version ? fr.Unsigned32() : 0;
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("player command: %s", e.what());
	}
}

/*** class Cmd_Bulldoze ***/

Cmd_Bulldoze::Cmd_Bulldoze (StreamRead & des) :
	PlayerCommand (0, des.Unsigned8()),
	serial        (des.Unsigned32()),
	recurse       (des.Unsigned8())
{}

void Cmd_Bulldoze::execute (Game & game)
{
	if (upcast(PlayerImmovable, pimm, game.objects().get_object(serial)))
		game.player(sender()).bulldoze(*pimm, recurse);
}

void Cmd_Bulldoze::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_BULLDOZE);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned8 (recurse);
}
#define PLAYER_CMD_BULLDOZE_VERSION 2
void Cmd_Bulldoze::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if
			(1 <= packet_version and
			 packet_version <= PLAYER_CMD_BULLDOZE_VERSION)
		{
			PlayerCommand::Read(fr, egbase, mol);
			Serial const pimm_serial = fr.Unsigned32();
			try {
				serial = mol.get<Map_Object>(pimm_serial).serial();
				recurse = 2 <= packet_version ? fr.Unsigned8() : false;
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
	Map_Object const & obj = *egbase.objects().get_object(serial);
	assert(mos.is_object_known(obj));
	fw.Unsigned32(mos.get_object_file_index(obj));
	fw.Unsigned8(recurse);
}

/*** class Cmd_Build ***/

Cmd_Build::Cmd_Build (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	bi = Building_Index(static_cast<Building_Index::value_t>(des.Signed16()));
	coords = des.Coords32  ();
}

void Cmd_Build::execute (Game & game)
{
	game.player(sender()).build(coords, bi);
}

void Cmd_Build::serialize (StreamWrite & ser) {
	ser.Unsigned8 (PLCMD_BUILD);
	ser.Unsigned8 (sender());
	ser.Signed16  (bi.value());
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
			bi     =
				Building_Index
					(static_cast<Building_Index::value_t>(fr.Unsigned16()));
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
	fw.Unsigned16(bi.value());
	fw.Coords32  (coords);
}


/*** class Cmd_BuildFlag ***/

Cmd_BuildFlag::Cmd_BuildFlag (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	coords = des.Coords32  ();
}

void Cmd_BuildFlag::execute (Game & game)
{
	game.player(sender()).build_flag(coords);
}

void Cmd_BuildFlag::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_BUILDFLAG);
	ser.Unsigned8 (sender());
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
	path = 0;
	steps = new char[nsteps];

	for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
		steps[i] = des.Unsigned8();
}

Cmd_BuildRoad::~Cmd_BuildRoad ()
{
	delete path;

	delete[] steps;
}

void Cmd_BuildRoad::execute (Game & game)
{
	if (path == 0) {
		assert (steps);

		path = new Path(start);
		for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
			path->append (game.map(), steps[i]);
	}

	game.player(sender()).build_road(*path);
}

void Cmd_BuildRoad::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_BUILDROAD);
	ser.Unsigned8 (sender());
	ser.Coords32  (start);
	ser.Unsigned16(nsteps);

	assert (path || steps);

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
	des         .Unsigned8 ();
	serial = des.Unsigned32();
}

void Cmd_FlagAction::execute (Game & game)
{
	Player & player = game.player(sender());
	if (upcast(Flag, flag, game.objects().get_object(serial)))
		if (&flag->owner() == &player)
			player.flagaction (*flag);
}

void Cmd_FlagAction::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_FLAGACTION);
	ser.Unsigned8 (sender());
	ser.Unsigned8 (0);
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
			fr                             .Unsigned8 ();
			uint32_t const flag_serial = fr.Unsigned32();
			try {
				serial = mol.get<Map_Object>(flag_serial).serial();
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
	fw.Unsigned8 (0);

	// Now serial
	Map_Object const & obj = *egbase.objects().get_object(serial);
	assert(mos.is_object_known(obj));
	fw.Unsigned32(mos.get_object_file_index(obj));
}

/*** Cmd_StartStopBuilding ***/

Cmd_StartStopBuilding::Cmd_StartStopBuilding (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
}

void Cmd_StartStopBuilding::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		game.player(sender()).start_stop_building(*building);
}

void Cmd_StartStopBuilding::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_STARTSTOPBUILDING);
	ser.Unsigned8 (sender());
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
				serial = mol.get<Map_Object>(building_serial).serial();
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
	Map_Object const & obj = *egbase.objects().get_object(serial);
	assert(mos.is_object_known(obj));
	fw.Unsigned32(mos.get_object_file_index(obj));
}


/*** Cmd_EnhanceBuilding ***/

Cmd_EnhanceBuilding::Cmd_EnhanceBuilding (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	bi = Building_Index(static_cast<Building_Index::value_t>(des.Unsigned16()));
}

void Cmd_EnhanceBuilding::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		game.player(sender()).enhance_building(building, bi);
}

void Cmd_EnhanceBuilding::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_ENHANCEBUILDING);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned16(bi.value());
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
				serial = mol.get<Map_Object>(building_serial).serial();
			} catch (_wexception const & e) {
				throw wexception("building %u: %s", building_serial, e.what());
			}
			bi =
				Building_Index
					(static_cast<Building_Index::value_t>(fr.Unsigned16()));
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
	Map_Object const & obj = *egbase.objects().get_object(serial);
	assert(mos.is_object_known(obj));
	fw.Unsigned32(mos.get_object_file_index(obj));

	// Now id
	fw.Unsigned16(bi.value());
}


/*** class Cmd_SetWarePriority ***/
Cmd_SetWarePriority::Cmd_SetWarePriority
	(int32_t const _duetime, Player_Number const _sender,
	 PlayerImmovable & imm,
	 int32_t const type, Ware_Index const index, int32_t const priority)
	:
	PlayerCommand(_duetime, _sender),
	m_serial     (imm.serial()),
	m_type       (type),
	m_index      (index),
	m_priority   (priority)
{}

void Cmd_SetWarePriority::execute(Game & game)
{
	upcast(ProductionSite, psite, game.objects().get_object(m_serial));

	if (!psite)
		return;
	if (psite->owner().get_player_number() != sender())
		return;

	psite->set_priority(m_type, m_index, m_priority);
}

#define PLAYER_CMD_SETWAREPRIORITY_VERSION 1

void Cmd_SetWarePriority::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(PLAYER_CMD_SETWAREPRIORITY_VERSION);

	PlayerCommand::Write(fw, egbase, mos);

	Map_Object const & obj = *egbase.objects().get_object(m_serial);
	fw.Unsigned32(mos.get_object_file_index(obj));
	fw.Unsigned8(m_type);
	fw.Signed32(m_index.value());
	fw.Signed32(m_priority);
}

void Cmd_SetWarePriority::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SETWAREPRIORITY_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			uint32_t const serial = fr.Unsigned32();
			try {
				m_serial = mol.get<Map_Object>(serial).serial();
			} catch (_wexception const & e) {
				throw wexception("site %u: %s", serial, e.what());
			}

			m_type = fr.Unsigned8();
			m_index = Ware_Index(static_cast<Ware_Index::value_t>(fr.Signed32()));
			m_priority = fr.Signed32();
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("set ware priority: %s", e.what());
	}
}

Cmd_SetWarePriority::Cmd_SetWarePriority(StreamRead & des) :
	PlayerCommand(0, des.Unsigned8()),
	m_serial     (des.Unsigned32()),
	m_type       (des.Unsigned8()),
	m_index      (Ware_Index(static_cast<Ware_Index::value_t>(des.Signed32()))),
	m_priority   (des.Signed32())
{}

void Cmd_SetWarePriority::serialize(StreamWrite & ser)
{
	ser.Unsigned8(PLCMD_SETWAREPRIORITY);
	ser.Unsigned8(sender());
	ser.Unsigned32(m_serial);
	ser.Unsigned8(m_type);
	ser.Signed32(m_index.value());
	ser.Signed32(m_priority);
}


Cmd_ChangeTargetQuantity::Cmd_ChangeTargetQuantity
	(int32_t const _duetime, Player_Number const _sender,
	 uint32_t const _economy, Ware_Index const _ware_type)
	:
	PlayerCommand(_duetime, _sender),
	m_economy (_economy), m_ware_type(_ware_type)
{}

void Cmd_ChangeTargetQuantity::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	PlayerCommand::Write(fw, egbase, mos);
	fw.Unsigned32(economy());
	fw.CString
		(egbase.player(sender()).tribe().get_ware_descr(ware_type())->name());
}

void Cmd_ChangeTargetQuantity::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		PlayerCommand::Read(fr, egbase, mol);
		m_economy   = fr.Unsigned32();
		m_ware_type =
			egbase.player(sender()).tribe().ware_index(fr.CString());
	} catch (_wexception const & e) {
		throw wexception("change target quantity: %s", e.what());
	}
}

Cmd_ChangeTargetQuantity::Cmd_ChangeTargetQuantity(StreamRead & des)
	:
	PlayerCommand(0, des.Unsigned8()),
	m_economy    (des.Unsigned32()),
	m_ware_type  (des.Unsigned8())
{}

void Cmd_ChangeTargetQuantity::serialize(StreamWrite & ser)
{
	ser.Unsigned8 (sender());
	ser.Unsigned32(economy());
	ser.Unsigned8 (ware_type().value());
}


Cmd_SetTargetQuantity::Cmd_SetTargetQuantity
	(int32_t const _duetime, Player_Number const _sender,
	 uint32_t const _economy,
	 Ware_Index const _ware_type,
	 uint32_t const _permanent, uint32_t const _temporary)
	:
	Cmd_ChangeTargetQuantity(_duetime, _sender, _economy, _ware_type),
	m_permanent(_permanent), m_temporary(_temporary)
{}

void Cmd_SetTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	if
		(economy  () < player.get_nr_economies() and
		 ware_type() < player.tribe().get_nrwares())
	{
		Economy::Target_Quantity & tq =
			player.get_economy_by_number(economy())->m_target_quantities
				[ware_type().value()];
		tq.permanent     = m_permanent;
		tq.temporary     = m_temporary;
		tq.last_modified = duetime();
	}
}

#define PLAYER_CMD_SETTARGETQUANTITY_VERSION 1

void Cmd_SetTargetQuantity::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(PLAYER_CMD_SETTARGETQUANTITY_VERSION);
	Cmd_ChangeTargetQuantity::Write(fw, egbase, mos);
	fw.Unsigned32(m_permanent);
	fw.Unsigned32(m_temporary);
}

void Cmd_SetTargetQuantity::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SETTARGETQUANTITY_VERSION) {
			Cmd_ChangeTargetQuantity::Read(fr, egbase, mol);
			m_permanent = fr.Unsigned32();
			m_temporary = fr.Unsigned32();
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("set target quantity: %s", e.what());
	}
}

Cmd_SetTargetQuantity::Cmd_SetTargetQuantity(StreamRead & des)
	:
	Cmd_ChangeTargetQuantity(des),
	m_permanent             (des.Unsigned32()),
	m_temporary             (des.Unsigned32())
{}

void Cmd_SetTargetQuantity::serialize(StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SETTARGETQUANTITY);
	Cmd_ChangeTargetQuantity::serialize(ser);
	ser.Unsigned32(m_permanent);
	ser.Unsigned32(m_temporary);
}


Cmd_ResetTargetQuantity::Cmd_ResetTargetQuantity
	(int32_t const _duetime, Player_Number const _sender,
	 uint32_t const _economy,
	 Ware_Index const _ware_type)
	:
	Cmd_ChangeTargetQuantity(_duetime, _sender, _economy, _ware_type)
{}

void Cmd_ResetTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	Tribe_Descr const & tribe = player.tribe();
	if
		(economy  () < player.get_nr_economies() and
		 ware_type() < tribe.get_nrwares())
	{
		Economy::Target_Quantity & tq =
			player.get_economy_by_number(economy())->m_target_quantities
				[ware_type().value()];
		tq.temporary = tq.permanent =
			tribe.get_ware_descr(ware_type())->default_target_quantity();
		tq.last_modified = 0;
	}
}

#define PLAYER_CMD_RESETTARGETQUANTITY_VERSION 1

void Cmd_ResetTargetQuantity::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(PLAYER_CMD_SETTARGETQUANTITY_VERSION);
	Cmd_ChangeTargetQuantity::Write(fw, egbase, mos);
}

void Cmd_ResetTargetQuantity::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_RESETTARGETQUANTITY_VERSION) {
			Cmd_ChangeTargetQuantity::Read(fr, egbase, mol);
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("set target quantity: %s", e.what());
	}
}

Cmd_ResetTargetQuantity::Cmd_ResetTargetQuantity(StreamRead & des)
	: Cmd_ChangeTargetQuantity(des)
{}

void Cmd_ResetTargetQuantity::serialize(StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_RESETTARGETQUANTITY);
	Cmd_ChangeTargetQuantity::serialize(ser);
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

void Cmd_ChangeTrainingOptions::execute (Game & game)
{
	if (upcast(TrainingSite, trainingsite, game.objects().get_object(serial)))
		game.player(sender()).change_training_options
			(*trainingsite, attribute, value);
}

void Cmd_ChangeTrainingOptions::serialize (StreamWrite & ser) {
	ser.Unsigned8 (PLCMD_CHANGETRAININGOPTIONS);
	ser.Unsigned8 (sender());
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
				serial    = mol.get<Map_Object>(trainingsite_serial).serial();
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
	Map_Object const & obj = *egbase.objects().get_object(serial);
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

void Cmd_DropSoldier::execute (Game & game)
{
	if (upcast(PlayerImmovable, player_imm, game.objects().get_object(serial)))
		if (upcast(Soldier, s, game.objects().get_object(soldier)))
			game.player(sender()).drop_soldier(*player_imm, *s);
}

void Cmd_DropSoldier::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_DROPSOLDIER);
	ser.Unsigned8 (sender());
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
			uint32_t const milsite_serial = fr.Unsigned32();
			try {
				serial  = mol.get<PlayerImmovable>(milsite_serial).serial();
			} catch (_wexception const & e) {
				throw wexception("militarysite %u: %s", milsite_serial, e.what());
			}
			uint32_t const soldier_serial = fr.Unsigned32();
			try {
				soldier = mol.get<Soldier>        (soldier_serial).serial();
			} catch (_wexception const & e) {
				throw wexception("soldier %u: %s",      soldier_serial, e.what());
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

	{ //  site serial
		Map_Object const & obj = *egbase.objects().get_object(serial);
		assert(mos.is_object_known(obj));
		fw.Unsigned32(mos.get_object_file_index(obj));
	}

	{ //  soldier serial
		Map_Object const & obj = *egbase.objects().get_object(soldier);
		assert(mos.is_object_known(obj));
		fw.Unsigned32(mos.get_object_file_index(obj));
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

void Cmd_ChangeSoldierCapacity::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		if (&building->owner() == game.get_player(sender()))
			if (upcast(SoldierControl, ctrl, building))
				ctrl->changeSoldierCapacity(val);
}

void Cmd_ChangeSoldierCapacity::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_CHANGESOLDIERCAPACITY);
	ser.Unsigned8 (sender());
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
				serial = mol.get<Map_Object>(militarysite_serial).serial();
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
	Map_Object const & obj = *egbase.objects().get_object(serial);
	assert(mos.is_object_known(obj));
	fw.Unsigned32(mos.get_object_file_index(obj));

	// Now capacity
	fw.Signed16(val);

}

/*** Cmd_EnemyFlagAction ***/

Cmd_EnemyFlagAction::Cmd_EnemyFlagAction (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	des         .Unsigned8 ();
	serial   = des.Unsigned32();
	des         .Unsigned8 ();
	number   = des.Unsigned8 ();
	des         .Unsigned8 ();
}

void Cmd_EnemyFlagAction::execute (Game & game)
{
	Player & player = game.player(sender());
	PlayerImmovable & imm =
		dynamic_cast<PlayerImmovable &>(*game.objects().get_object(serial));

	log
		("player(%u)    imm->owner (%d) number = %u\n",
		 player.get_player_number(), imm.owner().get_player_number(), number);

	if (upcast(Flag, flag, &imm)) {
		if (Building const * const building = flag->get_building())
			if
				(&imm.owner() != &player
				 and
				 1
				 <
				 player.vision
				 	(Map::get_index
				 	 	(building->get_position(), game.map().get_width())))
				player.enemyflagaction (*flag, sender(), number);
	} else
		log ("Cmd_EnemyFlagAction Player invalid or not seeing target.\n");
}

void Cmd_EnemyFlagAction::serialize (StreamWrite & ser) {
	ser.Unsigned8 (PLCMD_ENEMYFLAGACTION);
	ser.Unsigned8 (sender());
	ser.Unsigned8 (1);
	ser.Unsigned32(serial);
	ser.Unsigned8 (sender());
	ser.Unsigned8 (number);
	ser.Unsigned8 (0);
}
#define PLAYER_CMD_ENEMYFLAGACTION_VERSION 2
void Cmd_EnemyFlagAction::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_ENEMYFLAGACTION_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			fr           .Unsigned8 ();
			uint32_t const flag_serial = fr.Unsigned32();
			try {
				serial = mol.get<Map_Object>(flag_serial).serial();
			} catch (_wexception const & e) {
				throw wexception("flag %u: %s", flag_serial, e.what());
			}
			fr           .Unsigned8 ();
			number   = fr.Unsigned8 ();
			fr           .Unsigned8 ();
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
	fw.Unsigned8 (0);

	// Now serial
	Map_Object const & obj = *egbase.objects().get_object(serial);
	assert(mos.is_object_known(obj));
	fw.Unsigned32(mos.get_object_file_index(obj));

	// Now param
	fw.Unsigned8 (sender());
	fw.Unsigned8 (number);
	fw.Unsigned8 (0);
}

};
