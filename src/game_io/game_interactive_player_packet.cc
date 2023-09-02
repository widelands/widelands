/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#include "game_io/game_interactive_player_packet.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "wui/interactive_player.h"
#include "wui/mapview.h"
#include "wui/mapviewpixelfunctions.h"

namespace Widelands {

namespace {

/* Changelog:
 * 6: v1.1
 * 7: Changed landmark handling
 * 8: Naval Warfare: Removed expedition port spaces
 */
constexpr uint16_t kCurrentPacketVersion = 8;

}  // namespace

void GameInteractivePlayerPacket::read(FileSystem& fs, Game& game, MapObjectLoader* mol) {
	try {
		FileRead fr;
		fr.open(fs, "binary/interactive_player");
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version <= kCurrentPacketVersion && packet_version >= 4) {
			PlayerNumber player_number = fr.unsigned_8();
			if (!(0 < player_number && player_number <= game.map().get_nrplayers())) {
				throw GameDataError("Invalid player number: %i.", player_number);
			}

			if (game.get_player(player_number) == nullptr) {
				// This happens if the player, that saved the game, was a spectator
				// and the slot for player 1 was not used in the game.
				// So now we try to create an InteractivePlayer object for another
				// player instead.
				const PlayerNumber max = game.map().get_nrplayers();
				for (player_number = 1; player_number <= max; ++player_number) {
					if (game.get_player(player_number) != nullptr) {
						break;
					}
				}
				if (player_number > max) {
					throw GameDataError("The game has no players!");
				}
			}

			Vector2f center_map_pixel = Vector2f::zero();
			center_map_pixel.x = fr.float_32();
			center_map_pixel.y = fr.float_32();

			uint32_t display_flags = fr.unsigned_32();

			InteractivePlayer* ipl = game.get_ipl();
			if (InteractiveBase* const ibase = game.get_ibase()) {
				ibase->map_view()->scroll_to_map_pixel(center_map_pixel, MapView::Transition::Jump);
			}
			if (ipl != nullptr) {  // Not in replays
#ifndef NDEBUG
				display_flags |= InteractiveBase::dfDebug;
#else
				display_flags &= ~InteractiveBase::dfDebug;
#endif
				ipl->set_display_flags(display_flags);
				ipl->set_player_number(player_number);
			}

			// Map landmarks
			// TODO(Nordfriese): Savegame compatibility v1.1
			if (InteractiveBase* const ibase = game.get_ibase()) {
				const size_t no_of_landmarks =
				   (packet_version >= 7) ? fr.unsigned_32() : fr.unsigned_8();
				auto& quicknav = ibase->quick_navigation();
				quicknav.landmarks().resize(no_of_landmarks);

				// TODO(tothxa): Only needed for savegame compatibility code for v1.1 below
				const Vector2f viewpoint_offset(
				   ibase->map_view()->get_w() / 2.f, ibase->map_view()->get_h() / 2.f);

				for (size_t i = 0; i < no_of_landmarks; ++i) {
					uint8_t set = fr.unsigned_8();
					const float x = fr.float_32();
					const float y = fr.float_32();
					const float zoom = fr.float_32();
					MapView::View view = {Vector2f(x, y), zoom};
					// TODO(tothxa): Savegame compatibility, remove after v1.2
					if (packet_version < 7) {
						// Reference point was top left up to v1.1, is center starting with v1.2.
						// The landmarks are shifted if the window size is different between saving and
						// restoring. (same as pre v1.2 behavior)
						view.viewpoint =
						   MapviewPixelFunctions::panel_to_map(view.viewpoint, zoom, viewpoint_offset);
					}
					if (set > 0) {
						quicknav.set_landmark(i, view);
					}
					if (packet_version >= 7) {
						quicknav.landmarks()[i].name = fr.string();
					}
				}

				// TODO(tothxa): Savegame compatibility, remove after v1.2
				//               This used to be a cache of expedition ships' primary seen port spaces
				if (packet_version < 8) {
					size_t nr_port_spaces = fr.unsigned_32();
					for (size_t i = 0; i < nr_port_spaces; ++i) {
						/* serial */ fr.unsigned_32();
						/* x */ fr.signed_16();
						/* y */ fr.signed_16();
					}
				}
				if (packet_version >= 6 && (fr.unsigned_8() != 0u) && (ipl != nullptr)) {
					ibase->load_windows(fr, *mol);
				}
			}
		} else {
			throw UnhandledVersionError(
			   "GameInteractivePlayerPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("interactive player: %s", e.what());
	}
}

/*
 * Write Function
 */
void GameInteractivePlayerPacket::write(FileSystem& fs, Game& game, MapObjectSaver* const mos) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	InteractiveBase* const ibase = game.get_ibase();
	InteractivePlayer* const iplayer = game.get_ipl();

	// Player number
	fw.unsigned_8(iplayer != nullptr ? iplayer->player_number() : 1);

	if (ibase != nullptr) {
		const Vector2f center = ibase->map_view()->view_area().rect().center();
		fw.float_32(center.x);
		fw.float_32(center.y);
	} else {
		fw.float_32(0.f);
		fw.float_32(0.f);
	}

	// Display flags
	fw.unsigned_32(ibase != nullptr ? ibase->get_display_flags() : 0);

	// Map landmarks
	if (ibase != nullptr) {
		const auto& landmarks = ibase->quick_navigation().landmarks();
		fw.unsigned_32(landmarks.size());
		for (const auto& lm : landmarks) {
			fw.unsigned_8(lm.set ? 1 : 0);
			fw.float_32(lm.view.viewpoint.x);
			fw.float_32(lm.view.viewpoint.y);
			fw.float_32(lm.view.zoom);
			fw.string(lm.name);
		}
	}

	if (iplayer != nullptr) {
		fw.unsigned_8(1);
		iplayer->save_windows(fw, *mos);
	} else {
		fw.unsigned_8(0);
	}

	/*
	 * Important:
	 * The number of bytes written by `iplayer->save_windows` can not be
	 * determined in advance, so any loading code that does not wish to
	 * load the windows will not be able to access any data saved beyond
	 * this call. Therefore, do not save any further data after this
	 * function call which is not intended to be read exclusively by
	 * loading callers that also read the window data.
	 */

	fw.write(fs, "binary/interactive_player");
}
}  // namespace Widelands
