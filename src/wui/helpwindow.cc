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

#include "wui/helpwindow.h"

#include <memory>

#include "base/i18n.h"
#include "graphic/text_layout.h"
#include "logic/map_objects/tribes/building.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/messagebox.h"

namespace UI {

constexpr int kPadding = 5;
constexpr int kButtonSize = 25;

BuildingHelpWindow::BuildingHelpWindow(InteractiveBase* const parent,
                                       UI::UniqueWindow::Registry& reg,
                                       const Widelands::BuildingDescr& building_description,
                                       const Widelands::TribeDescr& tribe,
                                       LuaInterface* const lua,
                                       uint32_t width,
                                       uint32_t height)
   : UI::UniqueWindow(parent,
                      UI::WindowStyle::kWui,
                      "encyclopedia_window",
                      &reg,
                      width,
                      height,
                      format(_("Help: %s"), building_description.descname())),
     parent_(parent),
     height_(height),
     vbox_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     hbox_(&vbox_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
     titlearea_(new Textarea(&hbox_,
                             UI::PanelStyle::kWui,
                             UI::FontStyle::kWuiInfoPanelHeading,
                             0,
                             0,
                             width,
                             kButtonSize)),
     b_back_(new Button(&hbox_,
                        "history_back",
                        0,
                        0,
                        kButtonSize,
                        kButtonSize,
                        UI::ButtonStyle::kWuiMenu,
                        g_image_cache->get("images/ui_basic/scrollbar_left.png"),
                        _("Back"))),
     textarea_(new MultilineTextarea(&vbox_, 0, 0, width, height_, UI::PanelStyle::kWui)),
     lua_(lua),
     tribe_(tribe) {
	assert(tribe.has_building(tribe.building_index(building_description.name())) ||
	       building_description.type() == Widelands::MapObjectType::MILITARYSITE);

	hbox_.add(b_back_);
	hbox_.add_space(kPadding);
	hbox_.add(titlearea_, UI::Box::Resizing::kFillSpace);

	vbox_.set_size(width, height);
	vbox_.set_max_size(width, height);
	vbox_.add(&hbox_);
	vbox_.add(textarea_, UI::Box::Resizing::kExpandBoth);

	b_back_->sigclicked.connect([this]() { clicked_back(); });

	load_help("building", building_description.name());
	set_center_panel(&vbox_);
	initialization_complete();
}

void BuildingHelpWindow::clicked_back() {
	assert(history_.size() > 1);
	history_.pop_back();
	bool to_self = (SDL_GetModState() & KMOD_CTRL) != 0;
	auto& last = to_self ? history_.front() : history_.back();
	while (to_self && history_.size() > 1) {
		history_.pop_back();
	}
	load_help(last.first, last.second, false);
}

bool BuildingHelpWindow::load_help(const std::string& type, const std::string& item, bool forward) {
	try {
		std::unique_ptr<LuaTable> t(lua_->run_script("tribes/scripting/help/" + type + "_help.lua"));
		std::unique_ptr<LuaCoroutine> cr(t->get_coroutine("func"));
		cr->push_arg(tribe_.name());
		cr->push_arg(item);
		cr->resume();
		std::unique_ptr<LuaTable> return_table = cr->pop_table();
		textarea_->set_text(as_richtext(return_table->get_string("text")));
		titlearea_->set_text(return_table->get_string("title"));
		if (forward) {
			history_.push_back(std::make_pair(type, item));
		}
		hbox_.set_visible(history_.size() > 1);
		textarea_->set_size(textarea_->get_w(), height_ - hbox_.is_visible() * kButtonSize);
		return true;
	} catch (LuaError& err) {
		textarea_->set_text(err.what());
		return false;
	}
}

void BuildingHelpWindow::handle_hyperlink(const std::string& action) {
	if (parent_->focused_child() != this) {
		// Not for us
		return;
	}
	if (parent_->egbase().descriptions().ware_exists(action) && load_help("ware", action)) {
		return;
	}
	if (parent_->egbase().descriptions().building_exists(action) && load_help("building", action)) {
		return;
	}
	if (parent_->egbase().descriptions().worker_exists(action) && load_help("worker", action)) {
		return;
	}
	if (parent_->egbase().descriptions().immovable_exists(action) &&
	    load_help("immovable", action)) {
		return;
	}

	log_err_time(parent_->egbase().get_gametime(), "Help window: Invalid hyperlink target '%s'",
	             action.c_str());
	UI::WLMessageBox m(parent_, UI::WindowStyle::kWui, _("Broken Link"),
	                   _("This hyperlink seems to be broken."), UI::WLMessageBox::MBoxType::kOk);
	m.run<UI::Panel::Returncodes>();
}

}  // namespace UI
