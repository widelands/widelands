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

#include "game_io/game_preload_packet.h"

#include <ctime>
#include <memory>

#include <boost/format.hpp>

#include "base/time_string.h"
#include "build_info.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/minimap_renderer.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/playersmanager.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "wui/interactive_player.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/minimap.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 6;
constexpr const char* kMinimapFilename = "minimap.png";

// Win condition localization can come from the 'widelands' or 'win_conditions' textdomain.
std::string GamePreloadPacket::get_localized_win_condition() const {
	std::string result = _(win_condition_);
	i18n::Textdomain td("win_conditions");
	return _(result);
}

void GamePreloadPacket::read(FileSystem& fs, Game&, MapObjectLoader* const) {
	try {
		Profile prof;
		prof.read("preload", nullptr, fs);
		Section& s = prof.get_safe_section("global");
		int32_t const packet_version = s.get_int("packet_version");

		if (packet_version == kCurrentPacketVersion) {
			gametime_ = s.get_safe_int("gametime");
			mapname_ = s.get_safe_string("mapname");

			background_ = s.get_safe_string("background");
			player_nr_ = s.get_safe_int("player_nr");
			win_condition_ = s.get_safe_string("win_condition");
			number_of_players_ = s.get_safe_int("player_amount");
			version_ = s.get_safe_string("widelands_version");
			if (fs.file_exists(kMinimapFilename)) {
				minimap_path_ = kMinimapFilename;
			}
			savetimestamp_ = static_cast<time_t>(s.get_natural("savetimestamp"));
			gametype_ = static_cast<GameController::GameType>(s.get_natural("gametype"));
		} else {
			throw UnhandledVersionError("GamePreloadPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("preload: %s", e.what());
	}
}

void GamePreloadPacket::write(FileSystem& fs, Game& game, MapObjectSaver* const) {
	Profile prof;
	Section& s = prof.create_section("global");

	InteractivePlayer const* const ipl = game.get_ipl();

	s.set_int("packet_version", kCurrentPacketVersion);

	//  save some kind of header.
	s.set_int("gametime", game.get_gametime());
	const Map& map = game.map();
	s.set_string("mapname", map.get_name());  // Name of map

	if (ipl) {
		// player that saved the game.
		s.set_int("player_nr", ipl->player_number());
	} else {
		// Pretend that the first player saved the game
		for (Widelands::PlayerNumber p = 1; p <= map.get_nrplayers(); ++p) {
			if (game.get_player(p)) {
				s.set_int("player_nr", p);
				break;
			}
		}
	}
	s.set_int("player_amount", game.player_manager()->get_number_of_players());
	s.set_string("widelands_version", build_id());
	s.set_string("background", map.get_background());
	s.set_string("win_condition", game.get_win_condition_displayname());
	s.set_int("savetimestamp", static_cast<uint32_t>(time(nullptr)));
	s.set_int("gametype", static_cast<int32_t>(game.game_controller()->get_game_type()));

	prof.write("preload", false, fs);

	// Write minimap image
	if (!game.is_loaded()) {
		return;
	}

	std::unique_ptr<::StreamWrite> sw(fs.open_stream_write(kMinimapFilename));
	if (sw.get() != nullptr) {
		const MiniMapLayer layers =
		   MiniMapLayer::Owner | MiniMapLayer::Building | MiniMapLayer::Terrain;
		std::unique_ptr<Texture> texture;
		if (ipl != nullptr) {  // Player
			texture = draw_minimap(
			   game, &ipl->player(), ipl->view_area().rect(), MiniMapType::kStaticViewWindow, layers);
		} else {  // Observer
			texture = draw_minimap(game, nullptr, Rectf(), MiniMapType::kStaticMap, layers);
		}
		assert(texture != nullptr);
		save_to_png(texture.get(), sw.get(), ColorType::RGBA);
		sw->flush();
	}
}
}
