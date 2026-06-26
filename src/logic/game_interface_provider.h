/*
 * Copyright (C) 2026 by the Widelands Development Team
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

#ifndef WL_LOGIC_GAME_INTERFACE_PROVIDER_H
#define WL_LOGIC_GAME_INTERFACE_PROVIDER_H

#include <memory>

#include "base/vector.h"
#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

namespace Widelands {
class Bob;
class Game;
class MapObjectLoader;
struct MapObjectSaver;
struct Message;
struct MessageId;
}  // namespace Widelands
struct ChatProvider;
class FileRead;
class FileWrite;
class Texture;

class IGameInterface {
public:
	virtual ~IGameInterface() = default;

	virtual void cleanup_for_load() = 0;
	virtual void postload() = 0;
	virtual void info_panel_fast_forward_message_queue() = 0;

	virtual void load_plugins() = 0;
	virtual void rebuild_main_menu() = 0;
	virtual void notify_player_starting_pos(Widelands::PlayerNumber player, Widelands::Coords coords) = 0;
	virtual void notify_game_ended() = 0;
	virtual void notify_replay_ended() = 0;
	virtual void notify_desync() = 0;
	virtual void notify_message(Widelands::PlayerNumber pn, Widelands::MessageId id, const Widelands::Message& message, bool popup) = 0;
	virtual void request_watch_window(Widelands::PlayerNumber pn, Widelands::Bob& bob) = 0;

	virtual void log_message(const std::string& message, const std::string& tooltip = std::string()) const = 0;
	inline void log_message(const char* message, const char* tt = nullptr) const {
		log_message(std::string(message), tt == nullptr ? std::string() : std::string(tt));
	}

	virtual Widelands::PlayerNumber player_number() const = 0;
	virtual std::unique_ptr<Texture> draw_minimap_for_savegame() = 0;

	struct SaveloadingInformation {
		struct LandmarkInfo {
			bool set {false};
			float view_x, view_y, zoom;
			std::string name;
		};

		Widelands::PlayerNumber player_number {1};
		Vector2f mapview_center {Vector2f::zero()};
		uint32_t display_flags {0U};
		std::vector<LandmarkInfo> landmarks;
		bool should_saveload_windows {false};
	};

	virtual void gather_saveloading_information(SaveloadingInformation& data) = 0;
	virtual void restore_from_saveloading_information(SaveloadingInformation& data) = 0;
	virtual void load_windows(FileRead&, Widelands::MapObjectLoader&) = 0;
	virtual void save_windows(FileWrite&, Widelands::MapObjectSaver&) = 0;

	virtual void main_loop() = 0;
	virtual void end_main_loop() = 0;
};

class IGameInterfaceProvider {
public:
	virtual ~IGameInterfaceProvider() = default;
	virtual std::unique_ptr<IGameInterface> create(Widelands::Game& game, Widelands::PlayerNumber player_number, bool multiplayer, ChatProvider* chat_provider) = 0;
};

#endif  // end of include guard: WL_LOGIC_GAME_INTERFACE_PROVIDER_H
