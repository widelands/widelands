/*
 * Copyright (C) 2025 by the Widelands Development Team
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

#include "scripting/lua_root_notifications.h"

#include <memory>

#include "chat/chat.h"
#include "economy/economy.h"
#include "economy/expedition_bootstrap.h"
#include "graphic/note_graphic_resolution_changed.h"
#include "logic/map_objects/pinned_note.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "scripting/globals.h"
#include "wui/economy_options_window.h"
#include "wui/interactive_base.h"
#include "wui/quicknavigation.h"

namespace LuaRoot {
namespace LuaNotifications {

template <typename... Args> struct SignalImpl : public Wrapper {
	using Signal = Notifications::Signal<Args...>;

	LuaSubscriber::Message generate_message(Args... args) const;

	SignalImpl(std::string type, uint32_t serial, const Signal& signal) {
		persistance.serial = serial;
		persistance.type = type;

		signal.connect([this](Args... args) { owner->add_message(generate_message(args...)); });
	}
};

template <typename Note> struct NoteImpl : public Wrapper {
	explicit NoteImpl(std::string type) {
		persistance.serial = 0;
		persistance.type = type;

		subscriber_ = Notifications::subscribe<Note>(
		   [this](const Note& note) { owner->add_message(generate_message(note)); });
	}

	LuaSubscriber::Message generate_message(const Note& note) const;

	std::unique_ptr<Notifications::Subscriber<Note>> subscriber_;
};

Wrapper* create(const std::string& type) {
	if (type == PersistanceInfo::kBuilding) {
		return create_building();
	}
	if (type == PersistanceInfo::kChatMessage) {
		return create_chat_message();
	}
	if (type == PersistanceInfo::kEconomy) {
		return create_economy();
	}
	if (type == PersistanceInfo::kEconomyProfile) {
		return create_economy_profile();
	}
	if (type == PersistanceInfo::kExpeditionCancelled) {
		return create_expedition_cancelled();
	}
	if (type == PersistanceInfo::kFieldPossession) {
		return create_field_possession();
	}
	if (type == PersistanceInfo::kFieldTerrainChanged) {
		return create_field_terrain_changed();
	}
	if (type == PersistanceInfo::kGraphicResolutionChanged) {
		return create_graphic_resolution_changed();
	}
	if (type == PersistanceInfo::kImmovable) {
		return create_immovable();
	}
	if (type == PersistanceInfo::kPinnedNoteMoved) {
		return create_pinned_note_moved();
	}
	if (type == PersistanceInfo::kPlayerDetailsEvent) {
		return create_player_details();
	}
	if (type == PersistanceInfo::kProductionSiteOutOfResources) {
		return create_production_site_out_of_resources();
	}
	if (type == PersistanceInfo::kQuicknavChangedEvent) {
		return create_quicknav_changed();
	}
	if (type == PersistanceInfo::kShip) {
		return create_ship();
	}
	if (type == PersistanceInfo::kTrainingSiteSoldierTrained) {
		return create_training_site_soldier_trained();
	}
	return nullptr;
}

Wrapper* create_mapview_jump(lua_State* L) {
	return new SignalImpl(
	   PersistanceInfo::kMapViewJump, 0, get_egbase(L).get_ibase()->map_view()->jump);
}

Wrapper* create_mapview_changeview(lua_State* L) {
	return new SignalImpl(
	   PersistanceInfo::kMapViewChangeview, 0, get_egbase(L).get_ibase()->map_view()->changeview);
}

Wrapper* create_mapview_field_clicked(lua_State* L) {
	return new SignalImpl(PersistanceInfo::kMapViewFieldClicked, 0,
	                      get_egbase(L).get_ibase()->map_view()->field_clicked);
}

Wrapper* create_mapview_track_selection(lua_State* L) {
	return new SignalImpl(PersistanceInfo::kMapViewTrackSelection, 0,
	                      get_egbase(L).get_ibase()->map_view()->track_selection);
}

Wrapper* create_map_object_removed(const Widelands::MapObject& obj) {
	return new SignalImpl(PersistanceInfo::kMapObjectRemoved, obj.serial(), obj.removed);
}

Wrapper* create_building_muted(const Widelands::Building& obj) {
	return new SignalImpl(PersistanceInfo::kBuildingMuted, obj.serial(), obj.muted);
}

Wrapper* create_building() {
	return new NoteImpl<Widelands::NoteBuilding>(PersistanceInfo::kBuilding);
}

Wrapper* create_chat_message() {
	return new NoteImpl<ChatMessage>(PersistanceInfo::kChatMessage);
}

Wrapper* create_economy() {
	return new NoteImpl<Widelands::NoteEconomy>(PersistanceInfo::kEconomy);
}

Wrapper* create_economy_profile() {
	return new NoteImpl<NoteEconomyProfile>(PersistanceInfo::kEconomyProfile);
}

Wrapper* create_expedition_cancelled() {
	return new NoteImpl<Widelands::NoteExpeditionCanceled>(PersistanceInfo::kExpeditionCancelled);
}

Wrapper* create_field_possession() {
	return new NoteImpl<Widelands::NoteFieldPossession>(PersistanceInfo::kFieldPossession);
}

Wrapper* create_field_terrain_changed() {
	return new NoteImpl<Widelands::NoteFieldTerrainChanged>(PersistanceInfo::kFieldTerrainChanged);
}

Wrapper* create_graphic_resolution_changed() {
	return new NoteImpl<GraphicResolutionChanged>(PersistanceInfo::kGraphicResolutionChanged);
}

Wrapper* create_immovable() {
	return new NoteImpl<Widelands::NoteImmovable>(PersistanceInfo::kImmovable);
}

Wrapper* create_pinned_note_moved() {
	return new NoteImpl<Widelands::NotePinnedNoteMoved>(PersistanceInfo::kPinnedNoteMoved);
}

Wrapper* create_player_details() {
	return new NoteImpl<Widelands::NotePlayerDetailsEvent>(PersistanceInfo::kPlayerDetailsEvent);
}

Wrapper* create_production_site_out_of_resources() {
	return new NoteImpl<Widelands::NoteProductionSiteOutOfResources>(
	   PersistanceInfo::kProductionSiteOutOfResources);
}

Wrapper* create_quicknav_changed() {
	return new NoteImpl<NoteQuicknavChangedEvent>(PersistanceInfo::kQuicknavChangedEvent);
}

Wrapper* create_ship() {
	return new NoteImpl<Widelands::NoteShip>(PersistanceInfo::kShip);
}

Wrapper* create_training_site_soldier_trained() {
	return new NoteImpl<Widelands::NoteTrainingSiteSoldierTrained>(
	   PersistanceInfo::kTrainingSiteSoldierTrained);
}

template <> LuaSubscriber::Message SignalImpl<>::generate_message() const {
	return LuaSubscriber::Message();
}

template <> LuaSubscriber::Message SignalImpl<uint32_t>::generate_message(uint32_t serial) const {
	return {{"object", LuaSubscriber::Value(LuaSubscriber::Value::Type::kMapObject, serial)}};
}

template <>
LuaSubscriber::Message SignalImpl<const Widelands::NodeAndTriangle<>&>::generate_message(
   const Widelands::NodeAndTriangle<>& field) const {
	return {
	   {"node_x", field.node.x},
	   {"node_y", field.node.y},
	   {"triangle_x", field.triangle.node.x},
	   {"triangle_y", field.triangle.node.y},
	   {"triangle_t", std::string(field.triangle.t == Widelands::TriangleIndex::D ? "D" : "R")},
	};
}

template <>
LuaSubscriber::Message
NoteImpl<Widelands::NoteBuilding>::generate_message(const Widelands::NoteBuilding& note) const {
	std::string action;
	switch (note.action) {
	case Widelands::NoteBuilding::Action::kChanged:
		action = "changed";
		break;
	case Widelands::NoteBuilding::Action::kStartWarp:
		action = "start_warp";
		break;
	case Widelands::NoteBuilding::Action::kFinishWarp:
		action = "finish_warp";
		break;
	case Widelands::NoteBuilding::Action::kWorkersChanged:
		action = "workers_changed";
		break;
	default:
		throw wexception("Invalid building change action %d", static_cast<int>(note.action));
	}

	return {
	   {"object", LuaSubscriber::Value(LuaSubscriber::Value::Type::kMapObject, note.serial)},
	   {"action", action},
	};
}

template <>
LuaSubscriber::Message NoteImpl<ChatMessage>::generate_message(const ChatMessage& note) const {
	return {
	   {"sender", note.sender},  {"recipient", note.recipient}, {"msg", note.msg},
	   {"player", note.playern}, {"time", note.time},
	};
}

template <>
LuaSubscriber::Message
NoteImpl<Widelands::NoteEconomy>::generate_message(const Widelands::NoteEconomy& note) const {
	std::string action;
	switch (note.action) {
	case Widelands::NoteEconomy::Action::kMerged:
		action = "merged";
		break;
	case Widelands::NoteEconomy::Action::kDeleted:
		action = "deleted";
		break;
	default:
		throw wexception("Invalid economy change action %d", static_cast<int>(note.action));
	}
	return {
	   {"old_economy", note.old_economy},
	   {"new_economy", note.new_economy},
	   {"action", action},
	};
}

template <>
LuaSubscriber::Message
NoteImpl<NoteEconomyProfile>::generate_message(const NoteEconomyProfile& /*note*/) const {
	return LuaSubscriber::Message();
}

template <>
LuaSubscriber::Message NoteImpl<Widelands::NoteExpeditionCanceled>::generate_message(
   const Widelands::NoteExpeditionCanceled& /*note*/) const {
	return LuaSubscriber::Message();
}

template <>
LuaSubscriber::Message NoteImpl<Widelands::NoteFieldPossession>::generate_message(
   const Widelands::NoteFieldPossession& note) const {
	std::string action;
	switch (note.ownership) {
	case Widelands::NoteFieldPossession::Ownership::LOST:
		action = "lost";
		break;
	case Widelands::NoteFieldPossession::Ownership::GAINED:
		action = "gained";
		break;
	default:
		throw wexception("Invalid field possession ownership %d", static_cast<int>(note.ownership));
	}

	return {
	   {"x", note.fc.x},
	   {"y", note.fc.y},
	   {"player", note.player != nullptr ? note.player->player_number() : 0},
	   {"ownership", action},
	};
}

template <>
LuaSubscriber::Message NoteImpl<Widelands::NoteFieldTerrainChanged>::generate_message(
   const Widelands::NoteFieldTerrainChanged& note) const {
	std::string action;
	switch (note.action) {
	case Widelands::NoteFieldTerrainChanged::Change::kTerrain:
		action = "terrain";
		break;
	case Widelands::NoteFieldTerrainChanged::Change::kImmovable:
		action = "immovable";
		break;
	default:
		throw wexception("Invalid terrain change action %d", static_cast<int>(note.action));
	}
	return {
	   {"x", note.fc.x},
	   {"y", note.fc.y},
	   {"action", action},
	};
}

template <>
LuaSubscriber::Message
NoteImpl<GraphicResolutionChanged>::generate_message(const GraphicResolutionChanged& note) const {
	return {
	   {"old_w", note.old_width},
	   {"old_h", note.old_height},
	   {"new_w", note.new_width},
	   {"new_h", note.new_height},
	};
}

template <>
LuaSubscriber::Message
NoteImpl<Widelands::NoteImmovable>::generate_message(const Widelands::NoteImmovable& note) const {
	std::string action;
	switch (note.ownership) {
	case Widelands::NoteImmovable::Ownership::LOST:
		action = "lost";
		break;
	case Widelands::NoteImmovable::Ownership::GAINED:
		action = "gained";
		break;
	default:
		throw wexception("Invalid immovable ownership %d", static_cast<int>(note.ownership));
	}

	return {
	   {"object", note.pi},
	   {"ownership", action},
	};
}

template <>
LuaSubscriber::Message NoteImpl<Widelands::NotePinnedNoteMoved>::generate_message(
   const Widelands::NotePinnedNoteMoved& note) const {
	return {
	   {"old_x", note.old_pos.x}, {"old_y", note.old_pos.y}, {"new_x", note.new_pos.x},
	   {"new_y", note.new_pos.y}, {"player", note.player},
	};
}

template <>
LuaSubscriber::Message NoteImpl<Widelands::NotePlayerDetailsEvent>::generate_message(
   const Widelands::NotePlayerDetailsEvent& note) const {
	return {
	   {"player", note.player.player_number()},
	};
}

template <>
LuaSubscriber::Message NoteImpl<Widelands::NoteProductionSiteOutOfResources>::generate_message(
   const Widelands::NoteProductionSiteOutOfResources& note) const {
	return {
	   {"object", note.ps},
	};
}

template <>
LuaSubscriber::Message NoteImpl<NoteQuicknavChangedEvent>::generate_message(
   const NoteQuicknavChangedEvent& /*note*/) const {
	return LuaSubscriber::Message();
}

template <>
LuaSubscriber::Message
NoteImpl<Widelands::NoteShip>::generate_message(const Widelands::NoteShip& note) const {
	std::string action;
	switch (note.action) {
	case Widelands::NoteShip::Action::kDestinationChanged:
		action = "destination_changed";
		break;
	case Widelands::NoteShip::Action::kWaitingForCommand:
		action = "waiting_for_command";
		break;
	case Widelands::NoteShip::Action::kNoPortLeft:
		action = "no_port_left";
		break;
	case Widelands::NoteShip::Action::kLost:
		action = "lost";
		break;
	case Widelands::NoteShip::Action::kGained:
		action = "gained";
		break;
	default:
		throw wexception("Invalid ship action %d", static_cast<int>(note.action));
	}
	return {
	   {"object", note.ship},
	   {"action", action},
	};
}

template <>
LuaSubscriber::Message NoteImpl<Widelands::NoteTrainingSiteSoldierTrained>::generate_message(
   const Widelands::NoteTrainingSiteSoldierTrained& note) const {
	return {
	   {"object", note.ts},
	};
}

}  // namespace LuaNotifications
}  // namespace LuaRoot
