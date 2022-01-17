/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_QUEUE_CMD_IDS_H
#define WL_LOGIC_QUEUE_CMD_IDS_H

#include <cstdint>

/*
 * This file contains the ids for the different
 * cmd_queue cmds. They are needed to save those
 * files to disk and (even more important) to load
 * them again.
 *
 * Some of this Commands (the player commands) have another
 * index, but this one is completley independent and only for
 * writing to files
 */

namespace Widelands {

// The command types are used by the QueueCmdFactory, for network serialization
// and for savegame compatibility.
// DO NOT change the order
// TODO(GunChleoc): Whenever we break savegame compatibility, clean this up and change data type to
// uint16_t.
enum class QueueCommandTypes : uint8_t {

	/* ID zero is reserved and must never be used */
	kNone = 0,

	/* PLAYER COMMANDS BELOW */
	kBuild,
	kBuildFlag,
	kBuildRoad,
	kFlagAction,
	kStartStopBuilding,
	kEnhanceBuilding,
	kBulldoze,

	kChangeTrainingOptions,
	kDropSoldier,
	kChangeSoldierCapacity,
	kEnemyFlagAction,

	kSetWarePriority,
	kSetWareTargetQuantity,
	// 14 removed post Build 21
	kSetWorkerTargetQuantity = 15,
	// 16 removed post Build 21

	// 17 was a command related to old events. removed

	kSetInputMaxFill = 18,

	kMessageSetStatusRead = 21,
	kMessageSetStatusArchived,

	kSetStockPolicy,
	kDismantleBuilding,

	kEvictWorker,

	kMilitarysiteSetSoldierPreference,
	kProposeTrade,
	kBuildWaterway,  // 28

	kShipSink = 121,
	kShipCancelExpedition,
	kStartOrCancelExpedition,
	kShipConstructPort,
	kShipScoutDirection,
	kShipExploreIsland,

	// The commands below are never serialized, but we still keep the IDs stable for savegame
	// compatibility.

	kDestroyMapObject,
	kAct,  // 128
	// 129 was a command related to old events. removed
	kIncorporate = 130,
	kLuaScript,
	kLuaCoroutine,
	kCalculateStatistics,
	kExpeditionConfig,
	kPickCustomStartingPosition,  // 135
	kCallEconomyBalance = 200,

	kDeleteMessage,
	kToggleMuteMessages,
	kMarkMapObjectForRemoval,  // 203

	kNetCheckSync = 250,
	kReplaySyncWrite,
	kReplaySyncRead,
	kReplayEnd  // 253
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_QUEUE_CMD_IDS_H
