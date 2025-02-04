/*
 * Copyright (C) 2021-2025 by the Widelands Development Team
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

#include "ui_fsmenu/addons/progress.h"

#include "base/time_string.h"
#include "ui_fsmenu/addons/manager.h"

namespace AddOnsUI {

ProgressIndicatorWindow::ProgressIndicatorWindow(UI::Panel* parent,
                                                 UI::WindowStyle style,
                                                 const std::string& title)
   : UI::Window(parent,
                style,
                "progress",
                0,
                0,
                parent->get_inner_w() - 2 * kRowButtonSize,
                2 * kRowButtonSize,
                title),
     modal_(*this),
     box_(this, panel_style_, "main_box", 0, 0, UI::Box::Vertical, get_inner_w()),
     hbox_(&box_, panel_style_, "hbox", 0, 0, UI::Box::Horizontal),
     txt1_(&box_,
           panel_style_,
           "text_1",
           style == UI::WindowStyle::kFsMenu ? UI::FontStyle::kFsMenuInfoPanelParagraph :
                                               UI::FontStyle::kWuiInfoPanelParagraph,
           "",
           UI::Align::kCenter),
     txt2_(&hbox_,
           panel_style_,
           "text_2",
           style == UI::WindowStyle::kFsMenu ? UI::FontStyle::kFsMenuInfoPanelParagraph :
                                               UI::FontStyle::kWuiInfoPanelParagraph,
           "",
           UI::Align::kLeft),
     txt3_(&hbox_,
           panel_style_,
           "text_3",
           style == UI::WindowStyle::kFsMenu ? UI::FontStyle::kFsMenuInfoPanelParagraph :
                                               UI::FontStyle::kWuiInfoPanelParagraph,
           "",
           UI::Align::kRight),
     progress_(&box_,
               panel_style_,
               "progress",
               0,
               0,
               get_w(),
               kRowButtonSize,
               UI::ProgressBar::Horizontal) {

	hbox_.add(&txt2_, UI::Box::Resizing::kExpandBoth);
	hbox_.add(&txt3_, UI::Box::Resizing::kExpandBoth);
	box_.add(&txt1_, UI::Box::Resizing::kFullSize);
	box_.add_space(kRowButtonSpacing);
	box_.add(&hbox_, UI::Box::Resizing::kFullSize);
	box_.add_space(2 * kRowButtonSpacing);
	box_.add(&progress_, UI::Box::Resizing::kFullSize);

	set_center_panel(&box_);
	center_to_parent();

	initialization_complete();
}

struct HangupWindow : public UI::Window {
	static std::map<uint32_t, HangupWindow*> windows;

	HangupWindow(UI::Panel& parent,
	             UI::WindowStyle style,
	             AddOns::NetAddons& net_addons,
	             uint32_t id)
	   : UI::Window(&parent,
	                style,
	                "hangup",
	                0,
	                0,
	                parent.get_inner_w() / 2,
	                2 * kRowButtonSize,
	                _("Server Unresponsive")),
	     id_(id),
	     net_addons_(net_addons),
	     modal_(*this),
	     box_(this, panel_style_, "main_box", 0, 0, UI::Box::Vertical, get_inner_w()),
	     txt1_(&box_,
	           panel_style_,
	           "text",
	           style == UI::WindowStyle::kFsMenu ? UI::FontStyle::kFsMenuInfoPanelParagraph :
	                                               UI::FontStyle::kWuiInfoPanelParagraph,
	           _("The server is not responding…"),
	           UI::Align::kCenter),
	     txt2_(&box_,
	           panel_style_,
	           "time",
	           style == UI::WindowStyle::kFsMenu ? UI::FontStyle::kFsMenuInfoPanelParagraph :
	                                               UI::FontStyle::kWuiInfoPanelParagraph,
	           "",
	           UI::Align::kCenter),
	     interrupt_(&box_,
	                "interrupt",
	                0,
	                0,
	                0,
	                0,
	                style == UI::WindowStyle::kFsMenu ? UI::ButtonStyle::kFsMenuSecondary :
	                                                    UI::ButtonStyle::kWuiSecondary,
	                _("Cancel")) {
		box_.add(&txt1_, UI::Box::Resizing::kExpandBoth);
		box_.add_space(kRowButtonSpacing);
		box_.add(&txt2_, UI::Box::Resizing::kExpandBoth);
		box_.add_space(kRowButtonSpacing);
		box_.add(&interrupt_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

		interrupt_.sigclicked.connect([this, &net_addons]() {
			delete this;
			net_addons.interrupt();
		});

		set_center_panel(&box_);
		center_to_parent();

		windows.emplace(id_, this);
		initialization_complete();
	}

	~HangupWindow() override {
		windows.erase(id_);
	}

	void set_elapsed(uint32_t elapsed) {
		txt2_.set_text(gametimestring(elapsed, true));
		do_redraw_now();
	}

	void die() override {
		AddOns::NetAddons& net_addons = net_addons_;
		delete this;
		net_addons.interrupt();
	}

private:
	uint32_t id_;
	AddOns::NetAddons& net_addons_;
	UI::Panel::ModalGuard modal_;
	UI::Box box_;
	UI::Textarea txt1_, txt2_;
	UI::Button interrupt_;
};

std::map<uint32_t, HangupWindow*> HangupWindow::windows;

AddOns::HangupFn
create_hangup_function(UI::Panel& parent, UI::WindowStyle style, AddOns::NetAddons& net_addons) {
	static uint32_t fn_id_counter_(0);
	const uint32_t id = ++fn_id_counter_;

	return [id, &parent, style, &net_addons](uint32_t elapsed, bool hang_is_over) {
		constexpr uint32_t kTimeout = 7000;
		if (!hang_is_over && elapsed < kTimeout) {
			return;
		}

		auto it = HangupWindow::windows.find(id);
		HangupWindow* hupw;
		if (it == HangupWindow::windows.end()) {
			if (hang_is_over) {
				return;
			}

			hupw = new HangupWindow(parent, style, net_addons, id);
		} else {
			hupw = it->second;
		}

		hupw->set_elapsed(elapsed);

		if (hang_is_over) {
			delete hupw;
		}
	};
}

}  // namespace AddOnsUI
