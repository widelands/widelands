/*
 * Copyright (C) 2006-2026 by the Widelands Development Team
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

#ifndef WL_NOTIFICATIONS_NOTE_IDS_H
#define WL_NOTIFICATIONS_NOTE_IDS_H

#include <cstdint>

// List all note ids here. They must be unique in the running
// system, this is easier to guarantee when they are all listed in
// one place.
enum class NoteId : uint32_t {
	Building,
	ChatMessage,
	Dropdown,
	Economy,
	EconomyProfile,
	ExpeditionCanceled,
	FieldPossession,
	FieldTerrainChanged,
	GameSettings,
	GraphicResolutionChanged,
	Hyperlink,
	Immovable,
	LoadingMessage,
	MapObjectDescription,
	MapObjectDescriptionTypeCheck,
	MapOptions,
	PinnedNoteMoved,
	PlayerDetailsEvent,
	ProductionSiteOutOfResources,
	QuicknavChangedEvent,
	Ship,
	Sound,
	ThreadSafeFunction,
	ThreadSafeFunctionHandled,
	TradeChanged,
	TrainingSiteSoldierTrained,
};

#endif  // end of include guard: WL_NOTIFICATIONS_NOTE_IDS_H
