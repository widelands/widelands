/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

enum class QueueCommandTypes {

	/* ID zero is reserved and must never be used */
	kNone = 0,

	/* PLAYER COMMANDS BELOW */
	kBuild,
	kFlag,
	kBuildRoad,
	kFlagAction,
	kStopBuilding,
	kEnhanceBuilding,
	kBulldoze,

	kChangeTrainingOptions,
	kDropSoldier,
	kChangeSoldierCapacity,
	kEnemyFlagAction,

	kSetWarePriority,
	kSetWareTargetQuantity,
	kResetWareTargetQuantity,
	kSetWorkerTargetQuantity,
	kResetWorkerTargetQuantity,  // 16

	// 17 was a command related to old events. removed

	kSetInputMaxFill = 18,

	kMessageSetStatusRead = 21,
	kMessageSetStatusArchived,

	kSetStockPolicy,
	kDismantleBuilding,

	kEvictWorker,

	kMilitarysiteSetSoldierPreference,  // 26

	kSinkShip = 121,
	kShipCancelExpedition,
	kPortStartExpedition,
	kShipConstructPort,
	kShipScout,
	kShipExplore,

	kDestroyMapObject,
	kAct,  // 128
	// 129 was a command related to old events. removed
	kIncorporate = 130,
	kLuaScript,
	kLuaCoroutine,
	kCalculateStatistics,  // 133
	kCallEconomyBalance = 200,

	kDeleteMessage,  // 201

	kNetCheckSync = 250,
	kReplaySyncWrite,
	kReplaySyncRead,
	kReplayEnd,  // 253
	kHideRevealField
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_QUEUE_CMD_IDS_H
