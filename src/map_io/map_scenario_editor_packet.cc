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

#include "economy/road.h"
#include "economy/workers_queue.h"
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
		// We are starting a scenario designed with the editor.
		// In the editor, we create workers in buildings and carriers on roads, but they are not told
		// to stay there because task stacks are only created in games. But they are there already, so
		// no requests are issued which we could fulfill. We therefore need to manually inform the
		// buildings and the workers that they are now bound to each other.
		const Map& map = egbase.map();
		const Field* eof = &map[map.max_index()];
		for (Field* f = &map[0]; f != eof; ++f) {
			upcast(Building, bld, f->get_immovable());
			upcast(ProductionSite, ps, bld);
			upcast(Road, road, f->get_immovable());

			for (Bob* b = f->get_first_bob(); b; b = b->get_next_bob()) {
				if (upcast(Worker, w, b)) {
					w->reset_tasks(*game);
					if (bld) {
						if (bld->soldier_control()) {
							// Is this a soldier that should be garrisoned in a military-/trainingsite?
							if (upcast(Soldier, s, w)) {
								bld->mutable_soldier_control()->incorporate_soldier(*game, *s);
							}
						}
					} else if (road) {
						// A carrier on a road
						if (upcast(Carrier, c, w)) {
							c->start_task_road(*game);
						}
					}
				}
			}
			if (bld) {
				// The editor messes up saved gametimes
				bld->leave_time_ = 0;

				if (ps) {
					// main_worker_ is not set yet in the editor, but perhaps a worker is there already
					assert(ps->main_worker_ < 0);
					const size_t nr_workers = ps->descr().working_positions().size();
					for (uint32_t i = 0; i < nr_workers; ++i) {
						if (Worker* worker = ps->working_positions_[i].worker) {
							ps->main_worker_ = i;
							worker->start_task_buildingwork(*game);
							break;
						}
					}
					// Make sure recruits stay in their barracks’ input queues
					for (InputQueue* iq : ps->input_queues_) {
						if (iq->get_type() == wwWORKER) {
							WorkersQueue& wq = dynamic_cast<WorkersQueue&>(*iq);
							std::vector<Worker*> workers = wq.workers_;
							wq.workers_.clear();
							for (Worker* w : workers) {
								wq.entered(w->descr().worker_index(), w);
							}
						}
					}
					ps->try_start_working(*game);
				}
			} else if (road) {
				road->wallet_ = road->busy_ ? kRoadMaxWallet : 0;
			} else if (upcast(Flag, flag, f->get_immovable())) {
				// Economy updates
				flag->get_economy(wwWARE)->rebalance_supply();
				flag->get_economy(wwWORKER)->rebalance_supply();
			}
		}
		return;
	}
	upcast(EditorInteractive, eia, egbase.get_ibase());
	assert(eia);

	eia->functions_.clear();
	eia->variables_.clear();
	eia->includes_.clear();

	if (!file_exists) {
		// Not a scenario
		eia->finalized_ = false;
		eia->player_relations_.reset(nullptr);
		return;
	}

	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			eia->finalized_ = fr.unsigned_8();
			if (eia->finalized_) {
				const unsigned nrplayers = eia->egbase().map().get_nrplayers();
				eia->player_relations_.reset(new uint8_t[nrplayers * nrplayers]);
				for (unsigned i = 0; i < nrplayers * nrplayers; ++i) {
					eia->player_relations_[i] = fr.unsigned_8();
				}

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
					eia->includes_.push_back(fr.c_string());
				}
			} else {
				eia->player_relations_.reset(nullptr);
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

		const unsigned nrplayers = eia->egbase().map().get_nrplayers();
		for (unsigned i = 0; i < nrplayers * nrplayers; ++i) {
			fw.unsigned_8(eia->player_relations_[i]);
		}

		eia->scripting_saver().delete_unused(*eia);
		eia->scripting_saver().save(fw);
		fw.unsigned_32(eia->functions_.size());
		for (const auto& f : eia->functions_) {
			fw.unsigned_32(f->serial());
		}
		fw.unsigned_32(eia->variables_.size());
		for (const auto& v : eia->variables_) {
			fw.unsigned_32(v->serial());
		}
		fw.unsigned_32(eia->includes_.size());
		for (const std::string& s : eia->includes_) {
			fw.c_string(s.c_str());
		}
	} else {
		fw.unsigned_8(0);
	}

	fw.write(fs, "binary/scenario");
}
}  // namespace Widelands
