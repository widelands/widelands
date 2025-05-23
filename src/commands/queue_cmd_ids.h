/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_COMMANDS_QUEUE_CMD_IDS_H
#define WL_COMMANDS_QUEUE_CMD_IDS_H

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

struct GameLogicCommand;

// The command types are used by the QueueCmdFactory, for network serialization
// and for savegame compatibility.
// DO NOT change the order
// TODO(GunChleoc): Whenever we break savegame compatibility, clean this up and change data type to
// uint16_t.
enum class QueueCommandTypes : uint8_t {

	/* ID zero is reserved and must never be used */
	kNone = 0,

	/* PLAYER COMMANDS BELOW */
	kBuildBuilding,
	kBuildFlag,
	kBuildRoad,
	kFlagAction,
	kStartStopBuilding,
	kEnhanceBuilding,
	kBulldoze,

	kChangeTrainingOptions,
	kDropSoldier,
	kChangeSoldierCapacity,
	kAttack,

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

	kSetSoldierPreference,
	kProposeTrade,
	kBuildWaterway,
	kTradeAction,
	kExtendTrade,  // 30

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
	kPickCustomStartingPosition,
	kShipRefit,
	kWarshipCommand,
	kShipSetDestination,  // 138

	kCallEconomyBalance = 200,
	kDeleteMessage,
	kToggleMuteMessages,
	kMarkMapObjectForRemoval,
	kDiplomacy,
	kPinnedNote,
	kToggleInfiniteProduction,
	kBuildingName,
	kFleetTargets,  // 208

	kNetCheckSync = 250,
	kReplaySyncWrite,
	kReplaySyncRead,
	kReplayEnd  // 253
};

GameLogicCommand& create_correct_queue_command(Widelands::QueueCommandTypes id);

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_QUEUE_CMD_IDS_H
