/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "editor/editorinteractive.h"

#include <memory>
#include <vector>

#include "base/i18n.h"
#include "base/log.h"
#include "build_info.h"
#include "economy/road.h"
#include "economy/waterway.h"
#include "editor/scripting/builtin.h"
#include "editor/scripting/constexpr.h"
#include "editor/scripting/function_statements.h"
#include "editor/scripting/variable.h"
#include "editor/ui_menus/help.h"
#include "editor/ui_menus/main_menu_map_options.h"
#include "editor/ui_menus/main_menu_new_map.h"
#include "editor/ui_menus/main_menu_random_map.h"
#include "editor/ui_menus/main_menu_save_map.h"
#include "editor/ui_menus/player_menu.h"
#include "editor/ui_menus/player_teams_menu.h"
#include "editor/ui_menus/scenario_tool_field_owner_options_menu.h"
#include "editor/ui_menus/scenario_tool_infrastructure_options_menu.h"
#include "editor/ui_menus/scenario_tool_road_options_menu.h"
#include "editor/ui_menus/scenario_tool_vision_options_menu.h"
#include "editor/ui_menus/scenario_tool_worker_options_menu.h"
#include "editor/ui_menus/tool_change_height_options_menu.h"
#include "editor/ui_menus/tool_change_resources_options_menu.h"
#include "editor/ui_menus/tool_noise_height_options_menu.h"
#include "editor/ui_menus/tool_place_critter_options_menu.h"
#include "editor/ui_menus/tool_place_immovable_options_menu.h"
#include "editor/ui_menus/tool_resize_options_menu.h"
#include "editor/ui_menus/tool_set_terrain_options_menu.h"
#include "editor/ui_menus/toolsize_menu.h"
#include "graphic/graphic.h"
#include "io/filewrite.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/ferry.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "map_io/map_loader.h"
#include "scripting/lua_table.h"
#include "sound/sound_handler.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "wui/interactive_base.h"

void EditorInteractive::unfinalize() {
	finalized_ = false;
	illustrating_vision_for_ = 0;
	allowed_buildings_windows_.clear();
	scripting_saver_.reset(nullptr);
	functions_.clear();
	variables_.clear();
	includes_.clear();
	set_display_flag(dfShowCensus, false);
	set_display_flag(dfShowOwnership, false);
	rebuild_scenario_tool_menu();
	rebuild_showhide_menu();
}

void EditorInteractive::init_allowed_buildings_windows_registries() {
	assert(finalized_);
	assert(allowed_buildings_windows_.empty());
	const unsigned nrplayers = egbase().map().get_nrplayers();
	for (Widelands::PlayerNumber p = 1; p <= nrplayers; ++p) {
		UI::UniqueWindow::Registry* r = new UI::UniqueWindow::Registry();
		r->open_window = [this, p, r]() { new EditorPlayerAllowedBuildingsWindow(this, p, *r); };
		allowed_buildings_windows_.push_back(std::unique_ptr<UI::UniqueWindow::Registry>(r));
	}
}

void EditorInteractive::finalize_clicked() {
	assert(!finalized_);
	UI::WLMessageBox m(
	   this, _("Finalize"),
	   _("Are you sure you want to finalize this map?\n\n"
	     "This means you will not be able to add or remove players, rename them, "
	     "or change their tribe and starting position.\n"
	     "Nor can the waterway length limit be changed any more.\n\n"
	     "This step is only required if you want to design a scenario with the editor."),
	   UI::WLMessageBox::MBoxType::kOkCancel);
	if (m.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
		return;
	}
	const std::string result = try_finalize();
	if (result.empty()) {
		// Success!
		return;
	}
	UI::WLMessageBox error(this, _("Finalize Failed"),
	                       (boost::format(_("Finalizing failed! Reason: %s")) % result).str(),
	                       UI::WLMessageBox::MBoxType::kOk);
	error.run<UI::Panel::Returncodes>();
}

std::string EditorInteractive::try_finalize() {
	if (finalized_) {
		return _("Already finalized");
	}
	const Widelands::Map& map = egbase().map();
	const size_t nr_players = map.get_nrplayers();
	if (nr_players < 1) {
		return _("The map has no players");
	}
	const Widelands::Tribes& t = egbase().tribes();
	for (Widelands::PlayerNumber p = 1; p <= nr_players; ++p) {
		if (!t.tribe_exists(t.tribe_index(map.get_scenario_player_tribe(p)))) {
			return (boost::format(_("Invalid tribe \"%1$s\" for player %2$s (%3$s)")) %
			        map.get_scenario_player_tribe(p).c_str() % std::to_string(static_cast<int>(p)) %
			        map.get_scenario_player_name(p).c_str())
			   .str();
		}
		if (!map.get_starting_pos(p)) {
			return (boost::format(_("No starting position was set for player %s")) %
			        std::to_string(static_cast<int>(p)))
			   .str();
		}
	}
	finalized_ = true;
	init_allowed_buildings_windows_registries();
	new_scripting_saver();
	{  // Create a main function with some dummy content
		LuaFunction* lf = new LuaFunction("mission_thread");
		lf->init(*scripting_saver_);

		ConstexprString* str = new ConstexprString("Hello World :)");
		str->init(*scripting_saver_);
		FS_FunctionCall* print =
		   new FS_FunctionCall(builtin_f("print").function.get(), nullptr, {str});
		print->init(*scripting_saver_);
		lf->mutable_body().push_back(print);

		FS_FunctionCall* fc = new FS_FunctionCall(lf, nullptr, {});
		fc->init(*scripting_saver_);
		FS_LaunchCoroutine* lc = new FS_LaunchCoroutine(fc);
		lc->init(*scripting_saver_);
		functions_.push_back(lc);
	}
	tool_windows_.players.destroy();
	menu_windows_.mapoptions.destroy();
	rebuild_scenario_tool_menu();
	rebuild_showhide_menu();
	select_tool(tools_->info, EditorTool::ToolIndex::First);
	return "";
}

void EditorInteractive::write_lua(FileWrite& fw) const {
	const Widelands::Map& map = egbase().map();

	// Header
	fw.print_f("-- %s\n-- %s\n\n",
	           /** TRANSLATORS: "build_version (build_config)", e.g. "build20 (Release)" */
	           (boost::format(_("Automatically created by Widelands %1$s (%2$s)")) %
	            build_id().c_str() % build_type().c_str())
	              .str()
	              .c_str(),
	           _("Do not modify this file. All changes will be discarded the next time this map is "
	             "saved in the editor."));

	// i18n
	{
		const char* mapname = map.get_name().c_str();
		std::string textdomain = "";
		for (const char* c = mapname; *c; ++c) {
			if (*c == ' ' || *c == '_' || *c == '-' || *c == '.') {
				textdomain += '_';
			} else if ((*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9')) {
				textdomain += *c;
			} else if (*c >= 'A' && *c <= 'Z') {
				textdomain += (*c + 'a' - 'A');
			}
		}
		if (textdomain.empty()) {
			log_warn("Map name '%s' unsuited for creating a set_textdomain() argument\n", mapname);
			textdomain = "invalid_name";
		}
		fw.print_f("set_textdomain(\"scenario_%s.wmf\")\n", textdomain.c_str());
	}

	// Builtin includes
	{
		// We always include this one because it defines run()
		std::set<std::string> builtin_includes = {"scripting/coroutine.lua"};
		// Check which other includes to include
		for (const FS_FunctionCall* f : scripting_saver_->all<FS_FunctionCall>()) {
			const std::string bf = builtin_f(*f->get_function());
			if (!bf.empty()) {
				const BuiltinFunctionInfo& b = builtin_f(bf);
				if (!b.included_from.empty() && !builtin_includes.count(b.included_from)) {
					builtin_includes.insert(b.included_from);
				}
			}
		}
		fw.string("\n");
		for (const std::string& i : builtin_includes) {
			fw.print_f("include \"%s\"\n", i.c_str());
		}
	}

	// Global variables
	if (!variables_.empty()) {
		fw.string("\n");
		for (const auto& var : variables_) {
			var->write_lua(0, fw);
		}
	}

	// User-defined functions
	for (const auto& f : scripting_saver_->all<LuaFunction>()) {
		f->write_lua(0, fw);
		fw.string("\n");
	}

	// Hand-written includes
	// NOTE: Those should not contain "directly scripted" code but only functions which
	// can then be invoked from the user-defined functions here (not yet implemented)
	if (!includes_.empty()) {
		fw.string("\n");
		for (const std::string& i : includes_) {
			fw.print_f("include \"map:%s\"\n", i.c_str());
		}
	}

	// Autogenerate a function to initialize map objects and stuff as the editor user laboriously
	// defined them
	{
		auto generate_unused_name = [this](std::string basename) {
			while (scripting_saver_->get<Variable>(basename) ||
			       scripting_saver_->get<LuaFunction>(basename)) {
				basename = "_" + basename;  // NOLINT
			}
			return basename;
		};

		const std::string autogen_starting_function = generate_unused_name("autogenerated");
		fw.print_f("\nfunction %s()\n", autogen_starting_function.c_str());

#define write(text, ...) fw.print_f("   " text "\n", __VA_ARGS__)

		const std::string autogen_variable_name = generate_unused_name("temp_object");
		const char* var = autogen_variable_name.c_str();
		write("local %s", var);
		const std::string autogen_map_name_s = generate_unused_name("map");
		const char* autogen_map_name = autogen_map_name_s.c_str();
		write("local %s = wl.Game().map", autogen_map_name);

		const uint8_t nrplayers = map.get_nrplayers();
		std::unique_ptr<std::string[]> pvar_cache(new std::string[nrplayers]);
		;  // so the char*s won't go out of scope at once
		std::unique_ptr<const char*[]> pvar(new const char*[nrplayers]);
		for (unsigned i = 1; i <= nrplayers; ++i) {
			pvar_cache[i - 1] = generate_unused_name("p" + std::to_string(i));
			pvar[i - 1] = pvar_cache[i - 1].c_str();
			write("local %s = wl.Game().players[%u]", pvar[i - 1], i);
		}

		// Map objects

		std::set<const Widelands::Economy*> economies_to_save;
		{
			std::set<Widelands::Serial> saved_mos;
			for (size_t map_index = map.max_index(); map_index; --map_index) {
				const Widelands::Coords coords = map.coords(map_index - 1);
				const Widelands::Field& f = map[map_index - 1];
				if (f.get_immovable()) {
					bool skip = false;
					if (saved_mos.count(f.get_immovable()->serial())) {
						skip = true;
					} else {
						upcast(const Widelands::Building, bld, f.get_immovable());
						if (bld && bld->get_position() != coords) {
							skip = true;
						}
					}
					if (!skip) {
						saved_mos.insert(f.get_immovable()->serial());
						const Widelands::MapObjectType type = f.get_immovable()->descr().type();
						switch (type) {
						case Widelands::MapObjectType::FLAG: {
							const Widelands::Flag& flag =
							   dynamic_cast<const Widelands::Flag&>(*f.get_immovable());
							if (!economies_to_save.count(flag.get_economy(Widelands::wwWARE))) {
								economies_to_save.insert(flag.get_economy(Widelands::wwWARE));
							}
							if (!economies_to_save.count(flag.get_economy(Widelands::wwWORKER))) {
								economies_to_save.insert(flag.get_economy(Widelands::wwWORKER));
							}
							write("%s = %s:place_flag(%s:get_field(%u, %u), "
							      "true)",
							      var, pvar[flag.owner().player_number() - 1], autogen_map_name, coords.x,
							      coords.y);
							if (flag.current_wares()) {
								std::map<std::string, unsigned> wares;
								for (const Widelands::WareInstance* w : flag.get_wares()) {
									auto it = wares.find(w->descr().name());
									if (it == wares.end()) {
										wares[w->descr().name()] = 1;
									} else {
										++it->second;
									}
								}
								write("%s:set_wares({", var);
								for (const auto& pair : wares) {
									write("   %s = %u,", pair.first.c_str(), pair.second);
								}
								fw.string("   })\n");
							}
						} break;
						case Widelands::MapObjectType::ROAD:
						case Widelands::MapObjectType::WATERWAY: {
							const Widelands::RoadBase& road =
							   dynamic_cast<const Widelands::RoadBase&>(*f.get_immovable());
							const Widelands::Road* r = dynamic_cast<const Widelands::Road*>(&road);
							fw.print_f("   %s = %s:place_road(\"%s\", "
							           "%s:get_field(%u, %u).immovable",
							           var, pvar[road.owner().player_number() - 1],
							           r ? r->is_busy() ? "busy" : "normal" : "waterway", autogen_map_name,
							           road.get_flag(Widelands::RoadBase::FlagStart).get_position().x,
							           road.get_flag(Widelands::RoadBase::FlagStart).get_position().y);
							const size_t n = road.get_path().get_nsteps();
							for (size_t i = 0; i < n; ++i) {
								fw.print_f(
								   ", %s", Widelands::walkingdir_to_string(
								              static_cast<Widelands::WalkingDir>(road.get_path()[i]))
								              .c_str());
							}
							fw.print_f(")\n");
							if (r) {
								std::map<std::string, unsigned> workers;
								for (const Widelands::Carrier* c : r->carriers(egbase())) {
									if (c) {
										auto it = workers.find(c->descr().name());
										if (it == workers.end()) {
											workers[c->descr().name()] = 1;
										} else {
											++it->second;
										}
									}
								}
								write("%s:set_workers({", var);
								for (const auto& pair : workers) {
									write("   %s = %u,", pair.first.c_str(), pair.second);
								}
								fw.string("   })\n");
							} else {
								if (const Widelands::Ferry* ferry =
								       dynamic_cast<const Widelands::Waterway&>(road).get_ferry()) {
									write("%s:set_workers(\"%s\", 1)", var, ferry->descr().name().c_str());
								}
							}
						} break;
						case Widelands::MapObjectType::WAREHOUSE: {
							const Widelands::Warehouse& wh =
							   dynamic_cast<const Widelands::Warehouse&>(*f.get_immovable());
							write("%s = %s:place_building(\"%s\", "
							      "%s:get_field(%u, %u), false, true)",
							      var, pvar[wh.owner().player_number() - 1], wh.descr().name().c_str(),
							      autogen_map_name, wh.get_position().x, wh.get_position().y);

							std::map<std::string, unsigned> wares_workers;
							for (Widelands::DescriptionIndex di : wh.owner().tribe().wares()) {
								const std::string name = wh.owner().tribe().get_ware_descr(di)->name();
								write("%s:set_warehouse_policies(\"%s\", \"%s\")", var, name.c_str(),
								      Widelands::to_string(wh.get_ware_policy(di)).c_str());
								auto it = wares_workers.find(name);
								if (it == wares_workers.end()) {
									wares_workers[name] = wh.get_wares().stock(di);
								} else {
									it->second += wh.get_wares().stock(di);
								}
							}
							write("%s:set_wares({", var);
							for (const auto& pair : wares_workers) {
								write("   %s = %u,", pair.first.c_str(), pair.second);
							}
							fw.string("   })\n");

							wares_workers.clear();
							for (Widelands::DescriptionIndex di : wh.owner().tribe().workers()) {
								const std::string name = wh.owner().tribe().get_worker_descr(di)->name();
								write("%s:set_warehouse_policies(\"%s\", \"%s\")", var, name.c_str(),
								      Widelands::to_string(wh.get_worker_policy(di)).c_str());
								auto it = wares_workers.find(name);
								if (it == wares_workers.end()) {
									wares_workers[name] = wh.get_workers().stock(di);
								} else {
									it->second += wh.get_workers().stock(di);
								}
							}
							write("%s:set_workers({", var);
							for (const auto& pair : wares_workers) {
								write("   %s = %u,", pair.first.c_str(), pair.second);
							}
							fw.string("   })\n");
						} break;
						case Widelands::MapObjectType::MILITARYSITE: {
							const Widelands::MilitarySite& ms =
							   dynamic_cast<const Widelands::MilitarySite&>(*f.get_immovable());
							write("%s = %s:place_building(\"%s\", "
							      "%s:get_field(%u, %u), false, true)",
							      var, pvar[ms.owner().player_number() - 1], ms.descr().name().c_str(),
							      autogen_map_name, ms.get_position().x, ms.get_position().y);
							write("%s.prefer_heroes = %s", var,
							      ms.get_soldier_preference() == Widelands::SoldierPreference::kHeroes ?
							         "true" :
							         "false");
							write("%s.capacity = %u", var, ms.soldier_control()->soldier_capacity());
							std::map<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t>,
							         uint32_t>
							   soldiers;
							for (const Widelands::Soldier* s :
							     ms.soldier_control()->stationed_soldiers()) {
								std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t> key(
								   s->get_health_level(), s->get_attack_level(), s->get_defense_level(),
								   s->get_evade_level(), s->get_current_health());
								auto it = soldiers.find(key);
								if (it == soldiers.end()) {
									soldiers[key] = 1;
								} else {
									++it->second;
								}
							}
							write("%s:set_soldiers({", var);
							for (const auto& pair : soldiers) {
								write("   {%u, %u, %u, %u, %u} = %u,", std::get<0>(pair.first),
								      std::get<1>(pair.first), std::get<2>(pair.first),
								      std::get<3>(pair.first), std::get<4>(pair.first), pair.second);
							}
							fw.string("   })\n");
						} break;
						case Widelands::MapObjectType::TRAININGSITE: {
							const Widelands::TrainingSite& ts =
							   dynamic_cast<const Widelands::TrainingSite&>(*f.get_immovable());
							write("%s = %s:place_building(\"%s\", "
							      "%s:get_field(%u, %u), false, true)",
							      var, pvar[ts.owner().player_number() - 1], ts.descr().name().c_str(),
							      autogen_map_name, ts.get_position().x, ts.get_position().y);
							write("%s.capacity = %u", var, ts.soldier_control()->soldier_capacity());
							std::map<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t>,
							         uint32_t>
							   soldiers;
							for (const Widelands::Soldier* s :
							     ts.soldier_control()->stationed_soldiers()) {
								std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t> key(
								   s->get_health_level(), s->get_attack_level(), s->get_defense_level(),
								   s->get_evade_level(), s->get_current_health());
								auto it = soldiers.find(key);
								if (it == soldiers.end()) {
									soldiers[key] = 1;
								} else {
									++it->second;
								}
							}
							write("%s:set_soldiers({", var);
							for (const auto& pair : soldiers) {
								write("   {%u, %u, %u, %u, %u} = %u,", std::get<0>(pair.first),
								      std::get<1>(pair.first), std::get<2>(pair.first),
								      std::get<3>(pair.first), std::get<4>(pair.first), pair.second);
							}
							fw.string("   })\n");
						}
							FALLS_THROUGH;
						case Widelands::MapObjectType::PRODUCTIONSITE: {
							const Widelands::ProductionSite& ps =
							   dynamic_cast<const Widelands::ProductionSite&>(*f.get_immovable());
							if (type != Widelands::MapObjectType::TRAININGSITE) {
								write("%s = %s:place_building(\"%s\", "
								      "%s:get_field(%u, %u), false, true)",
								      var, pvar[ps.owner().player_number() - 1], ps.descr().name().c_str(),
								      autogen_map_name, ps.get_position().x, ps.get_position().y);
							}
							std::map<const char*, uint32_t> filled;
							for (const Widelands::InputQueue* q : ps.inputqueues()) {
								const char* name;
								if (q->get_type() == Widelands::wwWARE) {
									name = egbase().tribes().get_ware_descr(q->get_index())->name().c_str();
									write("%s:set_desired_fill(\"%s\", %u)", var, name, q->get_max_fill());
									write("%s:set_priority(\"%s\", \"%s\")", var, name,
									      Widelands::priority_to_string(
									         ps.get_priority(Widelands::wwWARE, q->get_index()))
									         .c_str());
								} else {
									name =
									   egbase().tribes().get_worker_descr(q->get_index())->name().c_str();
									write("%s:set_desired_fill(\"%s\", %u)", var, name, q->get_max_fill());
								}
								filled[name] = q->get_filled();
							}
							write("%s:set_inputs({", var);
							for (const auto& pair : filled) {
								write("   %s = %u,", pair.first, pair.second);
							}
							fw.string("   })\n");
						} break;
						case Widelands::MapObjectType::CONSTRUCTIONSITE: {
							const Widelands::ConstructionSite& cs =
							   dynamic_cast<const Widelands::ConstructionSite&>(*f.get_immovable());
							write("%s = %s:place_building(\"%s\", "
							      "%s:get_field(%u, %u), true, true)",
							      var, pvar[cs.owner().player_number() - 1], cs.building().name().c_str(),
							      autogen_map_name, cs.get_position().x, cs.get_position().y);
							write("%s.has_builder = %s", var, cs.has_builder(egbase()) ? "true" : "false");
							std::map<const char*, uint32_t> filled;
							for (size_t i = cs.nr_consume_waresqueues(); i; --i) {
								const Widelands::InputQueue& q = *cs.get_consume_waresqueue(i - 1);
								const char* name =
								   egbase().tribes().get_ware_descr(q.get_index())->name().c_str();
								write("%s:set_desired_fill(\"%s\", %u)", var, name, q.get_max_fill());
								write("%s:set_priority(\"%s\", \"%s\")", var, name,
								      Widelands::priority_to_string(
								         cs.get_priority(Widelands::wwWARE, q.get_index()))
								         .c_str());
								filled[name] = q.get_filled();
							}
							write("%s:set_inputs({", var);
							for (const auto& pair : filled) {
								write("   %s = %u,", pair.first, pair.second);
							}
							fw.string("   })\n");
							if (upcast(const Widelands::MilitarysiteSettings, ms, cs.get_settings())) {
								write("%s.setting_soldier_capacity = %u", var, ms->desired_capacity);
								write("%s.setting_prefer_heroes = %s", var,
								      ms->prefer_heroes ? "true" : "false");
							} else if (upcast(const Widelands::ProductionsiteSettings, ps,
							                  cs.get_settings())) {
								write("%s.setting_stopped = %s", var, ps->stopped ? "true" : "false");
								for (const auto& pair : ps->ware_queues) {
									const char* name =
									   cs.owner().tribe().get_ware_descr(pair.first)->name().c_str();
									write("%s:set_desired_fill(\"%s\", %u, true)", var, name,
									      pair.second.desired_fill);
									write("%s:set_priority(\"%s\", \"%s\", true)", var, name,
									      Widelands::priority_to_string(pair.second.priority).c_str());
								}
								for (const auto& pair : ps->worker_queues) {
									write("%s:set_desired_fill(\"%s\", %u, true)", var,
									      cs.owner().tribe().get_worker_descr(pair.first)->name().c_str(),
									      pair.second.desired_fill);
								}
								if (upcast(const Widelands::TrainingsiteSettings, ts, cs.get_settings())) {
									write("%s.setting_soldier_capacity = %u", var, ts->desired_capacity);
								}
							} else if (upcast(const Widelands::WarehouseSettings, ws, cs.get_settings())) {
								write("%s.setting_launch_expedition = %s", var,
								      ws->launch_expedition ? "true" : "false");
								for (const auto& pair : ws->ware_preferences) {
									write("%s:set_setting_warehouse_policy(\"%s\", \"%s\")", var,
									      cs.owner().tribe().get_ware_descr(pair.first)->name().c_str(),
									      Widelands::to_string(pair.second).c_str());
								}
								for (const auto& pair : ws->worker_preferences) {
									write("%s:set_setting_warehouse_policy(\"%s\", \"%s\")", var,
									      cs.owner().tribe().get_worker_descr(pair.first)->name().c_str(),
									      Widelands::to_string(pair.second).c_str());
								}
							}
						} break;
						case Widelands::MapObjectType::DISMANTLESITE: {
							const Widelands::DismantleSite& ds =
							   dynamic_cast<const Widelands::DismantleSite&>(*f.get_immovable());
							const int16_t xpos = ds.get_position().x;
							const int16_t ypos = ds.get_position().y;
							write("%s = %s:place_building(\"%s\", "
							      "%s:get_field(%d, %d), false, true)",
							      var, pvar[ds.owner().player_number() - 1], ds.building().name().c_str(),
							      autogen_map_name, xpos, ypos);
							write("%s:dismantle(%s)", var, ds.nr_dropout_waresqueues() ? "true" : "false");
							write("%s:get_field(%d, %d).immovable.has_builder = %s", autogen_map_name,
							      xpos, ypos, ds.has_builder(egbase()) ? "true" : "false");
						} break;
						default:
							break;
						}
					}
				}
			}
		}

		for (const Widelands::Economy* e : economies_to_save) {
			const Widelands::Coords flag = e->get_arbitrary_flag()->get_position();
			if (e->type() == Widelands::wwWARE) {
				write("%s = %s:get_field(%u, %u).immovable.ware_economy", var, autogen_map_name, flag.x,
				      flag.y);
				for (Widelands::DescriptionIndex di : e->owner().tribe().wares()) {
					const Widelands::WareDescr& d = *egbase().tribes().get_ware_descr(di);
					if (d.has_demand_check(e->owner().tribe().name())) {
						write("%s:set_target_quantity(\"%s\", %u)", var, d.name().c_str(),
						      e->target_quantity(di).permanent);
					}
				}
			} else {
				write("%s = %s:get_field(%u, %u).immovable.worker_economy", var, autogen_map_name,
				      flag.x, flag.y);
				for (Widelands::DescriptionIndex di : e->owner().tribe().workers()) {
					const Widelands::WorkerDescr& d = *egbase().tribes().get_worker_descr(di);
					if (d.has_demand_check()) {
						write("%s:set_target_quantity(\"%s\", %u)", var, d.name().c_str(),
						      e->target_quantity(di).permanent);
					}
				}
			}
		}

		// Players: Teams, relations, territory, vision

		for (unsigned p = 1; p <= nrplayers; ++p) {
			const Widelands::Player& player = egbase().player(p);
			write("%s.team = %u", pvar[p - 1], player.team_number());
			for (unsigned p2 = 1; p2 <= nrplayers; ++p2) {
				if (p2 != p) {
					write("%s:set_attack_forbidden(%u, %s)", pvar[p - 1], p2,
					      player.is_attack_forbidden(p2) ? "true" : "false");
				}
			}
			for (size_t map_index = map.max_index(); map_index; --map_index) {
				const Widelands::Coords c = map.coords(map_index - 1);
				if (map[map_index - 1].get_owned_by() == p) {
					write(
					   "%s:conquer(%s:get_field(%u, %u), 0)", pvar[p - 1], autogen_map_name, c.x, c.y);
				}
				const Widelands::SeeUnseeNode vis = player.get_vision(map_index - 1);
				if (vis == Widelands::SeeUnseeNode::kVisible) {
					write("%s:reveal_fields({%s:get_field(%u, %u)})", pvar[p - 1], autogen_map_name, c.x,
					      c.y);
				} else {
					write("%s:hide_fields({%s:get_field(%u, %u)}, %s)", autogen_map_name, pvar[p - 1],
					      c.x, c.y, Widelands::SeeUnseeNode::kUnexplored == vis ? "true" : "false");
				}
			}
		}

#undef write

		// Immediately invoke our function (not as a coroutine, to enforce immediate initialization)
		fw.print_f("end\n%s()\n", autogen_starting_function.c_str());
	}

	// Main function(s) call
	assert(!functions_.empty());
	for (const auto& f : functions_) {
		fw.string("\n");
		f->write_lua(0, fw);
	}
}
