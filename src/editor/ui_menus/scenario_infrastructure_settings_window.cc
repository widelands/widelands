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

#include "editor/ui_menus/scenario_infrastructure_settings_window.h"

#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "economy/ware_instance.h"
#include "editor/editorinteractive.h"
#include "editor/tools/scenario_infrastructure_settings_tool.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "wui/economy_options_window.h"

inline EditorInteractive& ScenarioFlagSettingsWindow::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

ScenarioFlagSettingsWindow::ScenarioFlagSettingsWindow(EditorInteractive& parent,
                                                       ScenarioInfrastructureSettingsTool& t,
                                                       Widelands::Flag& f)
   : UI::Window(
        &parent,
        "scenario_flag_settings_" + std::to_string(f.serial()),
        0,
        0,
        300,
        100,
        (boost::format(_("Flag at %1%×%2%")) % f.get_position().x % f.get_position().y).str()),
     main_box_(this, 0, 0, UI::Box::Vertical),
     wares_box_(&main_box_, 0, 0, UI::Box::Horizontal),
     economy_options_(&main_box_,
                      "economy_options",
                      0,
                      0,
                      50,
                      24,
                      UI::ButtonStyle::kWuiSecondary,
                      _("Economy Options"),
                      _("Change the target quantities for wares and workers")),
     tool_(&t),
     flag_(&f) {
	const uint32_t capacity = f.total_capacity();
	wares_on_flag_.reset(new Widelands::DescriptionIndex[capacity]);
	dropdowns_.reset(new std::unique_ptr<UI::Dropdown<Widelands::DescriptionIndex>>[capacity]);
	const Widelands::DescriptionIndex nr_wares = parent.egbase().tribes().nrwares();
	const Widelands::TribeDescr& tribe = f.owner().tribe();
	for (uint32_t i = 0; i < capacity; ++i) {
		dropdowns_[i].reset(new UI::Dropdown<Widelands::DescriptionIndex>(
		   &wares_box_, "ware_" + std::to_string(i), 50, 50, 34, 10, 34, _("Ware"),
		   UI::DropdownType::kPictorial, UI::PanelStyle::kWui, UI::ButtonStyle::kWuiSecondary));
		dropdowns_[i]->add(_("(Empty)"), Widelands::INVALID_INDEX,
		                   g_image_cache->get("images/wui/editor/no_ware.png"));
		for (Widelands::DescriptionIndex di = 0; di < nr_wares; ++di) {
			if (tribe.has_ware(di)) {
				const Widelands::WareDescr& d = *parent.egbase().tribes().get_ware_descr(di);
				dropdowns_[i]->add(d.descname(), di, d.icon());
			}
		}
		wares_box_.add(dropdowns_[i].get());
		dropdowns_[i]->selected.connect([this, i]() { select(i); });
	}

	main_box_.add(&wares_box_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(8);
	main_box_.add(&economy_options_, UI::Box::Resizing::kFullSize);
	economy_options_.sigclicked.connect([this]() { economy_options_clicked(); });

	set_center_panel(&main_box_);
	update();
}

void ScenarioFlagSettingsWindow::update() {
	Widelands::EditorGameBase& egbase = eia().egbase();
	Widelands::Flag* f = flag_.get(egbase);
	if (!f) {
		return;
	}
	const uint32_t capacity = f->total_capacity();

	size_t index = 0;
	for (const Widelands::WareInstance* w : f->get_wares()) {
		wares_on_flag_[index] = egbase.tribes().safe_ware_index(w->descr().name());
		++index;
	}
	for (; index < capacity; ++index) {
		wares_on_flag_[index] = Widelands::INVALID_INDEX;
	}

	for (uint32_t i = 0; i < capacity; ++i) {
		dropdowns_[i]->select(wares_on_flag_[i]);
	}
}

void ScenarioFlagSettingsWindow::select(uint32_t slot) {
	Widelands::EditorGameBase& egbase = eia().egbase();
	Widelands::Flag* f = flag_.get(egbase);
	if (!f) {
		return;
	}

	if (wares_on_flag_[slot] != Widelands::INVALID_INDEX) {
		assert(f->get_wares().size() > slot);
		assert(f->get_wares()[slot]->descr_index() == wares_on_flag_[slot]);
		f->get_ware(slot).remove(egbase);
	}

	const Widelands::DescriptionIndex di = dropdowns_[slot]->get_selected();
	if (di != Widelands::INVALID_INDEX) {
		Widelands::WareInstance& w =
		   *new Widelands::WareInstance(di, egbase.tribes().get_ware_descr(di));
		w.init(egbase);
		f->add_ware(egbase, w);
	}

	update();
}

void ScenarioFlagSettingsWindow::economy_options_clicked() {
	Widelands::EditorGameBase& egbase = eia().egbase();
	Widelands::Flag* f = flag_.get(egbase);
	if (!f) {
		return;
	}
	Widelands::Economy* ware_economy = f->get_economy(Widelands::wwWARE);
	Widelands::Economy* worker_economy = f->get_economy(Widelands::wwWORKER);
	assert(ware_economy);
	assert(worker_economy);
	bool window_open = false;
	if (ware_economy->get_options_window()) {
		window_open = true;
		EconomyOptionsWindow& window =
		   *static_cast<EconomyOptionsWindow*>(ware_economy->get_options_window());
		if (window.is_minimal()) {
			window.restore();
		}
		window.move_to_top();
	}
	if (worker_economy->get_options_window()) {
		window_open = true;
		EconomyOptionsWindow& window =
		   *static_cast<EconomyOptionsWindow*>(worker_economy->get_options_window());
		if (window.is_minimal()) {
			window.restore();
		}
		window.move_to_top();
	}
	if (!window_open) {
		new EconomyOptionsWindow(get_parent(), ware_economy, worker_economy, true);
	}
}

void ScenarioFlagSettingsWindow::think() {
	if (!flag()) {
		die();
	}
	UI::Window::think();
}

void ScenarioFlagSettingsWindow::unset_tool() {
	assert(tool_);
	tool_ = nullptr;
}

void ScenarioFlagSettingsWindow::die() {
	if (tool_) {
		tool_->window_closing(this);
	}
	UI::Window::die();
}

const Widelands::Flag* ScenarioFlagSettingsWindow::flag() const {
	return flag_.get(dynamic_cast<const EditorInteractive&>(*get_parent()).egbase());
}
