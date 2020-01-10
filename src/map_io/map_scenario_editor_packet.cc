/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "map_io/map_scenario_editor_packet.h"

#include <memory>

#include "editor/editorinteractive.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 1;

void MapScenarioEditorPacket::read(FileSystem& fs,
                                   EditorGameBase& egbase,
                                   bool is_game,
                                   MapObjectLoader&) {
	FileRead fr;
	const bool file_exists = fr.try_open(fs, "binary/scenario");

	if (is_game) {
		if (!file_exists) {
			// Not even a scenario, there's nothing for us to do
			return;
		}
		upcast(Game, game, &egbase);
		assert(game);
		// We are starting a scenario designed with the editor. Now we need to enforce some
		// MapObject updates because task stacks, act() commands and the like are not
		// properly created in the editor.
		log("NOCOM: MapScenarioEditorPacket::read for games: There´s a LOT left to do!!!\n");
		const Map& map = egbase.map();
		const Field* eof = &map[map.max_index()];
		for (Field* f = &map[0]; f != eof; ++f) {
			for (Bob* b = f->get_first_bob(); b; b = b->get_next_bob()) {
				// b->reset_tasks(*game);
				if (upcast(Worker, w, b)) {
					if (PlayerImmovable* pi = w->get_location(*game)) {
						w->set_location(nullptr);
						w->set_location(pi);
					}
				}
				b->send_signal(*game, "wakeup");
				b->schedule_act(*game, 0);
			}
			if (f->get_immovable()) {
				f->get_immovable()->schedule_act(*game, 0);
				if (upcast(Flag, flag, f->get_immovable())) {
					flag->get_economy(wwWARE)->rebalance_supply();
					flag->get_economy(wwWORKER)->rebalance_supply();
				}
			}
		}
		return;
	}
	upcast(EditorInteractive, eia, egbase.get_ibase());
	assert(eia);

	eia->functions_.clear();
	eia->variables_.clear();
	eia->includes_global_.clear();
	eia->includes_local_.clear();

	if (!file_exists) {
		// Not a scenario
		eia->finalized_ = false;
		return;
	}

	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			eia->finalized_ = fr.unsigned_8();
			if (eia->finalized_) {
				eia->new_scripting_saver();

				// The ScriptingLoader constructor will immediately load all ScriptingObjects
				// (triggering both loading phases one after the other)
				ScriptingLoader loader(fr, eia->scripting_saver());

				for (uint32_t n = fr.unsigned_32(); n; --n) {
					eia->functions_.push_back(&loader.get<FS_LaunchCoroutine>(fr.unsigned_32()));
				}
				for (uint32_t n = fr.unsigned_32(); n; --n) {
					eia->variables_.push_back(&loader.get<FS_LocalVarDeclOrAssign>(fr.unsigned_32()));
				}
				for (uint32_t n = fr.unsigned_32(); n; --n) {
					eia->includes_global_.push_back(fr.c_string());
				}
				for (uint32_t n = fr.unsigned_32(); n; --n) {
					eia->includes_local_.push_back(fr.c_string());
				}
			}
		} else {
			throw UnhandledVersionError(
			   "MapScenarioEditorPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("scenario editor data: %s", e.what());
	}
}

void MapScenarioEditorPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver&) {
	upcast(EditorInteractive, eia, egbase.get_ibase());
	if (!eia) {
		throw GameDataError("MapScenarioEditorPacket may be saved only inside the editor");
	}
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	if (eia->finalized_) {
		fw.unsigned_8(1);
		eia->scripting_saver().save(fw);
		fw.unsigned_32(eia->functions_.size());
		for (const auto& f : eia->functions_) {
			fw.unsigned_32(f->serial());
		}
		fw.unsigned_32(eia->variables_.size());
		for (const auto& v : eia->variables_) {
			fw.unsigned_32(v->serial());
		}
		fw.unsigned_32(eia->includes_global_.size());
		for (const std::string& s : eia->includes_global_) {
			fw.c_string(s.c_str());
		}
		fw.unsigned_32(eia->includes_local_.size());
		for (const std::string& s : eia->includes_local_) {
			fw.c_string(s.c_str());
		}
	} else {
		fw.unsigned_8(0);
	}

	fw.write(fs, "binary/scenario");
}
}  // namespace Widelands
