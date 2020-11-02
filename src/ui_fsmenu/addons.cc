/*
 * Copyright (C) 2020-2020 by the Widelands Development Team
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

#include "ui_fsmenu/addons.h"

#include <cstdlib>
#include <iomanip>
#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "graphic/image_cache.h"
#include "graphic/style_manager.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"
#include "scripting/lua_table.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_basic/progressbar.h"
#include "wlapplication.h"
#include "wlapplication_options.h"

constexpr int16_t kRowButtonSize = 32;
constexpr int16_t kRowButtonSpacing = 4;

// UI::Box by defaults limits its size to the window resolution. We use scrollbars,
// so we can and need to allow somewhat larger dimensions.
constexpr int32_t kHugeSize = std::numeric_limits<int32_t>::max() / 2;

struct ProgressIndicatorWindow : public UI::Window {
	ProgressIndicatorWindow(AddOnsCtrl* parent, const std::string& title)
	   : UI::Window(parent->get_parent(),
	                UI::WindowStyle::kFsMenu,
	                "progress",
	                0,
	                0,
	                parent->get_w(),
	                2 * kRowButtonSize,
	                title),
	     die_after_last_action(false),
	     box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, get_inner_w()),
	     txt1_(&box_,
	           UI::PanelStyle::kFsMenu,
	           UI::FontStyle::kFsMenuInfoPanelHeading,
	           "",
	           UI::Align::kCenter),
	     txt2_(&box_,
	           UI::PanelStyle::kFsMenu,
	           UI::FontStyle::kFsMenuInfoPanelParagraph,
	           "",
	           UI::Align::kLeft),
	     progress_(&box_,
	               UI::PanelStyle::kFsMenu,
	               0,
	               0,
	               get_w(),
	               kRowButtonSize,
	               UI::ProgressBar::Horizontal) {

		box_.add(&txt1_, UI::Box::Resizing::kFullSize);
		box_.add_space(kRowButtonSpacing);
		box_.add(&txt2_, UI::Box::Resizing::kFullSize);
		box_.add_space(2 * kRowButtonSpacing);
		box_.add(&progress_, UI::Box::Resizing::kFullSize);

		set_center_panel(&box_);
		center_to_parent();
	}
	~ProgressIndicatorWindow() override {
	}

	void die() override {
		UI::Window::die();
		if (!action_params.empty()) {
			throw wexception("Cancelled by user");
		}
	}

	void set_message_1(const std::string& msg) {
		txt1_.set_text(msg);
	}
	void set_message_2(const std::string& msg) {
		txt2_.set_text(msg);
	}
	UI::ProgressBar& progressbar() {
		return progress_;
	}

	// Bit complex design for the two download_xxx functions to ensure the
	// progress indicator window stays responsive during downloading
	std::function<void(const std::string&)> action_when_thinking;
	std::vector<std::string> action_params;
	bool die_after_last_action;
	void think() override {
		UI::Window::think();

		if (action_params.empty()) {
			end_modal(UI::Panel::Returncodes::kOk);
		} else {
			action_when_thinking(*action_params.begin());

			action_params.erase(action_params.begin());
			if (action_params.empty() && die_after_last_action) {
				die();
			}
		}
	}

private:
	UI::Box box_;
	UI::Textarea txt1_, txt2_;
	UI::ProgressBar progress_;
};

AddOnsCtrl::AddOnsCtrl(FullscreenMenuMain& fsmm)
   : UI::Window(&fsmm,
                UI::WindowStyle::kFsMenu,
                "addons",
                fsmm.calc_desired_window_x(UI::Window::WindowLayoutID::kFsMenuDefault),
                fsmm.calc_desired_window_y(UI::Window::WindowLayoutID::kFsMenuDefault),
                fsmm.calc_desired_window_width(UI::Window::WindowLayoutID::kFsMenuDefault),
                fsmm.calc_desired_window_height(UI::Window::WindowLayoutID::kFsMenuDefault),
                _("Add-On Manager")),
     fsmm_(fsmm),
     main_box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     buttons_box_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     warn_requirements_(
        &main_box_, 0, 0, get_w(), get_h() / 12, UI::PanelStyle::kFsMenu, "", UI::Align::kCenter),
     tabs_placeholder_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, 0, 0),
     tabs_(this, UI::TabPanelStyle::kFsMenu),
     installed_addons_outer_wrapper_(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     installed_addons_inner_wrapper_(
        &installed_addons_outer_wrapper_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     installed_addons_buttons_box_(
        &installed_addons_outer_wrapper_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     installed_addons_box_(&installed_addons_inner_wrapper_,
                           UI::PanelStyle::kFsMenu,
                           0,
                           0,
                           UI::Box::Vertical,
                           kHugeSize,
                           kHugeSize),
     browse_addons_outer_wrapper_(&tabs_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     browse_addons_inner_wrapper_(
        &browse_addons_outer_wrapper_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     browse_addons_buttons_box_(
        &browse_addons_outer_wrapper_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     browse_addons_buttons_inner_box_1_(
        &browse_addons_buttons_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     browse_addons_buttons_inner_box_2_(
        &browse_addons_buttons_inner_box_1_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     browse_addons_box_(&browse_addons_inner_wrapper_,
                        UI::PanelStyle::kFsMenu,
                        0,
                        0,
                        UI::Box::Vertical,
                        kHugeSize,
                        kHugeSize),
     filter_name_(&browse_addons_buttons_inner_box_1_, 0, 0, 100, UI::PanelStyle::kFsMenu),
     filter_verified_(&browse_addons_buttons_inner_box_2_,
                      UI::PanelStyle::kFsMenu,
                      Vector2i(0, 0),
                      _("Verified only"),
                      _("Show only verified add-ons in the Browse tab")),
     sort_order_(&browse_addons_buttons_inner_box_1_,
                 "sort",
                 0,
                 0,
                 0,
                 10,
                 filter_name_.get_h(),
                 _("Sort by"),
                 UI::DropdownType::kTextual,
                 UI::PanelStyle::kFsMenu,
                 UI::ButtonStyle::kFsMenuSecondary),
     filter_reset_(&browse_addons_buttons_inner_box_2_,
                   "f_reset",
                   0,
                   0,
                   24,
                   24,
                   UI::ButtonStyle::kFsMenuSecondary,
                   _("Reset"),
                   _("Reset the filters")),
     upgrade_all_(&buttons_box_,
                  "upgrade_all",
                  0,
                  0,
                  kRowButtonSize,
                  kRowButtonSize,
                  UI::ButtonStyle::kFsMenuSecondary,
                  ""),
     refresh_(&buttons_box_,
              "refresh",
              0,
              0,
              kRowButtonSize,
              kRowButtonSize,
              UI::ButtonStyle::kFsMenuSecondary,
              _("Refresh"),
              _("Refresh the list of add-ons available from the server")),
     ok_(&buttons_box_,
         "ok",
         0,
         0,
         kRowButtonSize,
         kRowButtonSize,
         UI::ButtonStyle::kFsMenuPrimary,
         _("OK")),
     autofix_dependencies_(&buttons_box_,
                           "autofix",
                           0,
                           0,
                           kRowButtonSize,
                           kRowButtonSize,
                           UI::ButtonStyle::kFsMenuSecondary,
                           _("Fix dependencies…"),
                           _("Try to automatically fix the dependency errors")),
     move_top_(&installed_addons_buttons_box_,
               "move_top",
               0,
               0,
               kRowButtonSize,
               kRowButtonSize,
               UI::ButtonStyle::kFsMenuSecondary,
               g_image_cache->get("images/ui_basic/scrollbar_up_fast.png"),
               _("Move selected add-on to top")),
     move_up_(&installed_addons_buttons_box_,
              "move_up",
              0,
              0,
              kRowButtonSize,
              kRowButtonSize,
              UI::ButtonStyle::kFsMenuSecondary,
              g_image_cache->get("images/ui_basic/scrollbar_up.png"),
              _("Move selected add-on one step up")),
     move_down_(&installed_addons_buttons_box_,
                "move_down",
                0,
                0,
                kRowButtonSize,
                kRowButtonSize,
                UI::ButtonStyle::kFsMenuSecondary,
                g_image_cache->get("images/ui_basic/scrollbar_down.png"),
                _("Move selected add-on one step down")),
     move_bottom_(&installed_addons_buttons_box_,
                  "move_bottom",
                  0,
                  0,
                  kRowButtonSize,
                  kRowButtonSize,
                  UI::ButtonStyle::kFsMenuSecondary,
                  g_image_cache->get("images/ui_basic/scrollbar_down_fast.png"),
                  _("Move selected add-on to bottom")) {
	installed_addons_buttons_box_.add(&move_top_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	installed_addons_buttons_box_.add_space(kRowButtonSpacing);
	installed_addons_buttons_box_.add(&move_up_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	installed_addons_buttons_box_.add_space(kRowButtonSize + 2 * kRowButtonSpacing);
	installed_addons_buttons_box_.add(&move_down_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	installed_addons_buttons_box_.add_space(kRowButtonSpacing);
	installed_addons_buttons_box_.add(&move_bottom_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	installed_addons_outer_wrapper_.add(
	   &installed_addons_inner_wrapper_, UI::Box::Resizing::kExpandBoth);
	installed_addons_outer_wrapper_.add_space(kRowButtonSpacing);
	installed_addons_outer_wrapper_.add(
	   &installed_addons_buttons_box_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	browse_addons_outer_wrapper_.add(&browse_addons_buttons_box_, UI::Box::Resizing::kFullSize);
	browse_addons_outer_wrapper_.add_space(2 * kRowButtonSpacing);
	browse_addons_outer_wrapper_.add(&browse_addons_inner_wrapper_, UI::Box::Resizing::kExpandBoth);

	installed_addons_inner_wrapper_.add(&installed_addons_box_, UI::Box::Resizing::kExpandBoth);
	browse_addons_inner_wrapper_.add(&browse_addons_box_, UI::Box::Resizing::kExpandBoth);
	tabs_.add("my", "", &installed_addons_outer_wrapper_);
	tabs_.add("all", "", &browse_addons_outer_wrapper_);

	/** TRANSLATORS: Sort add-ons alphabetically by name */
	sort_order_.add(_("Name"), AddOnSortingCriteria::kNameABC);
	/** TRANSLATORS: Sort add-ons alphabetically by name (inverted) */
	sort_order_.add(_("Name (descending)"), AddOnSortingCriteria::kNameCBA);
	/** TRANSLATORS: Sort add-ons by average rating */
	sort_order_.add(_("Best average rating"), AddOnSortingCriteria::kHighestRating, nullptr, true);
	/** TRANSLATORS: Sort add-ons by average rating */
	sort_order_.add(_("Worst average rating"), AddOnSortingCriteria::kLowestRating);
	/** TRANSLATORS: Sort add-ons by how often they were downloaded */
	sort_order_.add(_("Most often downloaded"), AddOnSortingCriteria::kMostDownloads);
	/** TRANSLATORS: Sort add-ons by how often they were downloaded */
	sort_order_.add(_("Least often downloaded"), AddOnSortingCriteria::kFewestDownloads);
	/** TRANSLATORS: Sort add-ons by upload date/time */
	sort_order_.add(_("Oldest"), AddOnSortingCriteria::kOldest);
	/** TRANSLATORS: Sort add-ons by upload date/time */
	sort_order_.add(_("Newest"), AddOnSortingCriteria::kNewest);

	filter_verified_.set_state(true);
	filter_name_.set_tooltip(_("Filter add-ons by name"));
	{
		uint8_t index = 0;
		for (const auto& pair : kAddOnCategories) {
			if (pair.first == AddOnCategory::kNone) {
				continue;
			}
			UI::Checkbox* c = new UI::Checkbox(
			   &browse_addons_buttons_box_, UI::PanelStyle::kFsMenu, Vector2i(0, 0),
			   g_image_cache->get(pair.second.icon),
			   (boost::format(_("Toggle category ‘%s’")) % pair.second.descname()).str());
			filter_category_[pair.first] = c;
			c->set_state(true);
			c->changed.connect([this, &pair]() { category_filter_changed(pair.first); });
			c->set_desired_size(kRowButtonSize, kRowButtonSize);
			browse_addons_buttons_box_.add(c, UI::Box::Resizing::kAlign, UI::Align::kCenter);
			browse_addons_buttons_box_.add_space(kRowButtonSpacing);
			++index;
		}
	}
	browse_addons_buttons_inner_box_2_.add(&filter_verified_, UI::Box::Resizing::kFullSize);
	browse_addons_buttons_inner_box_2_.add(&filter_reset_, UI::Box::Resizing::kExpandBoth);
	browse_addons_buttons_inner_box_1_.add(
	   &browse_addons_buttons_inner_box_2_, UI::Box::Resizing::kExpandBoth);
	browse_addons_buttons_inner_box_1_.add_space(kRowButtonSpacing);
	browse_addons_buttons_inner_box_1_.add(&filter_name_, UI::Box::Resizing::kExpandBoth);
	browse_addons_buttons_inner_box_1_.add_space(kRowButtonSpacing);
	browse_addons_buttons_inner_box_1_.add(&sort_order_, UI::Box::Resizing::kExpandBoth);
	browse_addons_buttons_box_.add(
	   &browse_addons_buttons_inner_box_1_, UI::Box::Resizing::kExpandBoth);

	filter_reset_.set_enabled(false);
	filter_name_.changed.connect([this]() {
		filter_reset_.set_enabled(true);
		rebuild();
	});
	filter_verified_.changed.connect([this]() {
		filter_reset_.set_enabled(true);
		rebuild();
	});
	sort_order_.selected.connect([this]() { rebuild(); });

	ok_.sigclicked.connect([this]() { end_modal<MenuTarget>(MenuTarget::kBack); });
	refresh_.sigclicked.connect([this]() {
		refresh_remotes();
		tabs_.activate(1);
	});
	autofix_dependencies_.sigclicked.connect([this]() { autofix_dependencies(); });

	filter_reset_.sigclicked.connect([this]() {
		filter_name_.set_text("");
		filter_verified_.set_state(true);
		for (auto& pair : filter_category_) {
			pair.second->set_state(true);
		}
		rebuild();
		filter_reset_.set_enabled(false);
	});
	upgrade_all_.sigclicked.connect([this]() {
		std::vector<std::pair<AddOnInfo, bool /* full upgrade */>> upgrades;
		bool all_verified = true;
		size_t nr_full_updates = 0;
		for (const RemoteAddOnRow* r : browse_) {
			if (r->upgradeable()) {
				const bool full_upgrade = r->full_upgrade_possible();
				upgrades.push_back(std::make_pair(r->info(), full_upgrade));
				if (full_upgrade) {
					all_verified &= r->info().verified;
					++nr_full_updates;
				}
			}
		}
		assert(!upgrades.empty());
		if (nr_full_updates > 0 && (!all_verified || !(SDL_GetModState() & KMOD_CTRL))) {
			// We ask for confirmation only for real upgrades. i18n-only upgrades are done silently.
			std::string text =
			   (boost::format(ngettext("Are you certain that you want to upgrade this %u add-on?\n",
			                           "Are you certain that you want to upgrade these %u add-ons?\n",
			                           nr_full_updates)) %
			    nr_full_updates)
			      .str();
			for (const auto& pair : upgrades) {
				if (pair.second) {
					text +=
					   (boost::format(_("\n· %1$s (%2$s) by %3$s")) % pair.first.descname() %
					    (pair.first.verified ? _("verified") : _("NOT VERIFIED")) % pair.first.author())
					      .str();
				}
			}
			UI::WLMessageBox w(this, UI::WindowStyle::kFsMenu, _("Upgrade All"), text,
			                   UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
				return;
			}
		}
		for (const auto& pair : upgrades) {
			upgrade(pair.first, pair.second);
		}
		rebuild();
	});

	move_up_.sigclicked.connect([this]() {
		const AddOnInfo info = selected_installed_addon();
		auto it = g_addons.begin();
		while (it->first.internal_name != info.internal_name) {
			++it;
		}
		const bool state = it->second;
		it = g_addons.erase(it);
		--it;
		g_addons.insert(it, std::make_pair(info, state));
		rebuild();
		focus_installed_addon_row(info);
	});
	move_down_.sigclicked.connect([this]() {
		const AddOnInfo info = selected_installed_addon();
		auto it = g_addons.begin();
		while (it->first.internal_name != info.internal_name) {
			++it;
		}
		const bool state = it->second;
		it = g_addons.erase(it);
		++it;
		g_addons.insert(it, std::make_pair(info, state));
		rebuild();
		focus_installed_addon_row(info);
	});
	move_top_.sigclicked.connect([this]() {
		const AddOnInfo info = selected_installed_addon();
		auto it = g_addons.begin();
		while (it->first.internal_name != info.internal_name) {
			++it;
		}
		const bool state = it->second;
		it = g_addons.erase(it);
		g_addons.insert(g_addons.begin(), std::make_pair(info, state));
		rebuild();
		focus_installed_addon_row(info);
	});
	move_bottom_.sigclicked.connect([this]() {
		const AddOnInfo info = selected_installed_addon();
		auto it = g_addons.begin();
		while (it->first.internal_name != info.internal_name) {
			++it;
		}
		const bool state = it->second;
		it = g_addons.erase(it);
		g_addons.push_back(std::make_pair(info, state));
		rebuild();
		focus_installed_addon_row(info);
	});

	buttons_box_.add_space(kRowButtonSpacing);
	buttons_box_.add(&upgrade_all_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kRowButtonSpacing);
	buttons_box_.add(&refresh_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kRowButtonSpacing);
	buttons_box_.add(&autofix_dependencies_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kRowButtonSpacing);
	buttons_box_.add(&ok_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kRowButtonSpacing);

	main_box_.add(&tabs_placeholder_, UI::Box::Resizing::kExpandBoth);
	main_box_.add_space(kRowButtonSpacing);
	main_box_.add(&warn_requirements_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(kRowButtonSpacing);
	main_box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(kRowButtonSpacing);

	// prevent assert failures
	installed_addons_box_.set_size(100, 100);
	browse_addons_box_.set_size(100, 100);
	installed_addons_inner_wrapper_.set_size(100, 100);
	browse_addons_inner_wrapper_.set_size(100, 100);

	installed_addons_inner_wrapper_.set_force_scrolling(true);
	browse_addons_inner_wrapper_.set_force_scrolling(true);

	refresh_remotes();
}

AddOnsCtrl::~AddOnsCtrl() {
	std::string text;
	for (const auto& pair : g_addons) {
		if (!text.empty()) {
			text += ',';
		}
		text += pair.first.internal_name + ':' + (pair.second ? "true" : "false");
	}
	set_config_string("addons", text);
	write_config();
}

inline const AddOnInfo& AddOnsCtrl::selected_installed_addon() const {
	return dynamic_cast<InstalledAddOnRow&>(*installed_addons_box_.focused_child()).info();
}
void AddOnsCtrl::focus_installed_addon_row(const AddOnInfo& info) {
	for (UI::Panel* p = installed_addons_box_.get_first_child(); p; p = p->get_next_sibling()) {
		if (dynamic_cast<InstalledAddOnRow&>(*p).info().internal_name == info.internal_name) {
			p->focus();
			return;
		}
	}
	NEVER_HERE();
}

void AddOnsCtrl::think() {
	UI::Panel::think();
	check_enable_move_buttons();
}

static bool category_filter_changing = false;
void AddOnsCtrl::category_filter_changed(const AddOnCategory which) {
	// protect against recursion
	if (category_filter_changing) {
		return;
	}
	category_filter_changing = true;

	// CTRL enables the selected category and disables all others
	if (SDL_GetModState() & KMOD_CTRL) {
		for (auto& pair : filter_category_) {
			pair.second->set_state(pair.first == which);
		}
	}

	filter_reset_.set_enabled(true);
	rebuild();
	category_filter_changing = false;
}

void AddOnsCtrl::check_enable_move_buttons() {
	const bool enable_move_buttons =
	   tabs_.active() == 0 && installed_addons_box_.focused_child() != nullptr;
	for (UI::Button* b : {&move_top_, &move_up_, &move_down_, &move_bottom_}) {
		b->set_enabled(enable_move_buttons);
	}
	if (enable_move_buttons) {
		const AddOnInfo& sel = selected_installed_addon();
		if (sel.internal_name == g_addons.begin()->first.internal_name) {
			move_top_.set_enabled(false);
			move_up_.set_enabled(false);
		}
		if (sel.internal_name == g_addons.back().first.internal_name) {
			move_down_.set_enabled(false);
			move_bottom_.set_enabled(false);
		}
	}
}

bool AddOnsCtrl::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_KP_ENTER:
		case SDLK_RETURN:
		case SDLK_ESCAPE:
			end_modal<MenuTarget>(MenuTarget::kBack);
			return true;
		default:
			break;
		}
	}
	return UI::Window::handle_key(down, code);
}

void AddOnsCtrl::refresh_remotes() {
	try {
		remotes_ = network_handler_.refresh_remotes();
	} catch (const std::exception& e) {
		std::string error = e.what();
		/** TRANSLATORS: This will be inserted into the string "Server Connection Error <br> by %s" */
		const std::string bug = _("a networking bug");
		remotes_ = {
		   AddOnInfo{"",
		             []() { return _("Server Connection Error"); },
		             [error]() {
			             return (boost::format(_("Unable to fetch the list of available add-ons from "
			                                     "the server!<br>Error Message: %s")) %
			                     error)
			                .str();
		             },
		             [bug]() { return bug; },
		             0,
		             0,
		             AddOnCategory::kNone,
		             {},
		             false,
		             {{}, {}, {}, {}},
		             0,
		             bug,
		             std::time(nullptr),
		             0,
		             0,
		             0.f,
		             {}}};
	}
	rebuild();
}

bool AddOnsCtrl::matches_filter(const AddOnInfo& info) {
	if (info.internal_name.empty()) {
		// always show error messages
		return true;
	}

	if (!filter_category_.at(info.category)->get_state()) {
		// wrong category
		return false;
	}

	if (filter_verified_.get_state() && !info.verified) {
		// not verified
		return false;
	}

	if (filter_name_.text().empty()) {
		// no text filter given, so we accept it
		return true;
	}
	for (const std::string& text :
	     {info.descname(), info.author(), info.internal_name, info.description()}) {
		if (text.find(filter_name_.text()) != std::string::npos) {
			// text filter found
			return true;
		}
	}
	// doesn't match the text filter
	return false;
}

void AddOnsCtrl::rebuild() {
	const uint32_t scrollpos_i =
	   installed_addons_inner_wrapper_.get_scrollbar() ?
	      installed_addons_inner_wrapper_.get_scrollbar()->get_scrollpos() :
	      0;
	const uint32_t scrollpos_b = browse_addons_inner_wrapper_.get_scrollbar() ?
	                                browse_addons_inner_wrapper_.get_scrollbar()->get_scrollpos() :
	                                0;
	installed_addons_box_.free_children();
	browse_addons_box_.free_children();
	installed_addons_box_.clear();
	browse_addons_box_.clear();
	browse_.clear();
	assert(installed_addons_box_.get_nritems() == 0);
	assert(browse_addons_box_.get_nritems() == 0);

	size_t index = 0;
	for (const auto& pair : g_addons) {
		if (index > 0) {
			installed_addons_box_.add_space(kRowButtonSize);
		}
		InstalledAddOnRow* i =
		   new InstalledAddOnRow(&installed_addons_box_, this, pair.first, pair.second);
		installed_addons_box_.add(i, UI::Box::Resizing::kFullSize);
		++index;
	}
	tabs_.tabs()[0]->set_title((boost::format(_("Installed (%u)")) % index).str());

	index = 0;
	std::list<AddOnInfo> remotes_to_show;
	for (const AddOnInfo& a : remotes_) {
		if (matches_filter(a)) {
			remotes_to_show.push_back(a);
		}
	}
	{
		const AddOnSortingCriteria sort_by = sort_order_.get_selected();
		remotes_to_show.sort([sort_by](const AddOnInfo& a, const AddOnInfo& b) {
			switch (sort_by) {
			case AddOnSortingCriteria::kNameABC:
				return a.descname().compare(b.descname()) < 0;
			case AddOnSortingCriteria::kNameCBA:
				return a.descname().compare(b.descname()) > 0;

			case AddOnSortingCriteria::kFewestDownloads:
				return a.download_count < b.download_count;
			case AddOnSortingCriteria::kMostDownloads:
				return a.download_count > b.download_count;

			case AddOnSortingCriteria::kOldest:
				return a.upload_timestamp < b.upload_timestamp;
			case AddOnSortingCriteria::kNewest:
				return a.upload_timestamp > b.upload_timestamp;

			case AddOnSortingCriteria::kLowestRating:
				if (a.votes == 0) {
					// Add-ons without votes should always end up
					// below any others when sorting by rating
					return false;
				} else if (b.votes == 0) {
					return true;
				} else if (std::abs(a.average_rating - b.average_rating) < 0.01f) {
					// ambiguity – always choose the one with more votes
					return a.votes > b.votes;
				} else {
					return a.average_rating < b.average_rating;
				}
			case AddOnSortingCriteria::kHighestRating:
				if (a.votes == 0) {
					return false;
				} else if (b.votes == 0) {
					return true;
				} else if (std::abs(a.average_rating - b.average_rating) < 0.01f) {
					return a.votes > b.votes;
				} else {
					return a.average_rating > b.average_rating;
				}
			}
			NEVER_HERE();
		});
	}
	std::vector<std::string> has_upgrades;
	for (const AddOnInfo& a : remotes_to_show) {
		if (0 < index++) {
			browse_addons_box_.add_space(kRowButtonSize);
		}
		uint32_t installed = kNotInstalled;
		uint32_t installed_i18n = kNotInstalled;
		for (const auto& pair : g_addons) {
			if (pair.first.internal_name == a.internal_name) {
				installed = pair.first.version;
				installed_i18n = pair.first.i18n_version;
				break;
			}
		}
		RemoteAddOnRow* r =
		   new RemoteAddOnRow(&browse_addons_box_, this, a, installed, installed_i18n);
		browse_addons_box_.add(r, UI::Box::Resizing::kFullSize);
		if (r->upgradeable()) {
			has_upgrades.push_back(a.descname());
		}
		browse_.push_back(r);
	}
	tabs_.tabs()[1]->set_title((boost::format(_("Browse (%u)")) % index).str());

	if (installed_addons_inner_wrapper_.get_scrollbar() && scrollpos_i) {
		installed_addons_inner_wrapper_.get_scrollbar()->set_scrollpos(scrollpos_i);
	}
	if (browse_addons_inner_wrapper_.get_scrollbar() && scrollpos_b) {
		browse_addons_inner_wrapper_.get_scrollbar()->set_scrollpos(scrollpos_b);
	}

	check_enable_move_buttons();
	upgrade_all_.set_title((boost::format(_("Upgrade all (%u)")) % has_upgrades.size()).str());
	upgrade_all_.set_enabled(!has_upgrades.empty());
	if (has_upgrades.empty()) {
		upgrade_all_.set_tooltip(_("No upgrades are available for your installed add-ons"));
	} else {
		std::string text =
		   (boost::format(ngettext(_("Upgrade the following %u add-on:"),
		                           _("Upgrade the following %u add-ons:"), has_upgrades.size())) %
		    has_upgrades.size())
		      .str();
		for (const std::string& name : has_upgrades) {
			text += "<br>";
			text += (boost::format(_("· %s")) % name).str();
		}
		upgrade_all_.set_tooltip(text);
	}
	update_dependency_errors();
}

void AddOnsCtrl::update_dependency_errors() {
	std::vector<std::string> warn_requirements;
	for (auto addon = g_addons.begin(); addon != g_addons.end(); ++addon) {
		if (!addon->second && kAddOnCategories.at(addon->first.category).can_disable_addons) {
			// Disabled, so we don't care about dependencies
			continue;
		}
		for (const std::string& requirement : addon->first.requirements) {
			std::vector<AddOnState>::iterator search_result = g_addons.end();
			bool too_late = false;
			for (auto search = g_addons.begin(); search != g_addons.end(); ++search) {
				if (search->first.internal_name == requirement) {
					search_result = search;
					break;
				}
				if (search == addon) {
					assert(!too_late);
					too_late = true;
				}
			}
			if (search_result == g_addons.end()) {
				warn_requirements.push_back(
				   (boost::format(_("· ‘%1$s’ requires ‘%2$s’ which could not be found")) %
				    addon->first.descname() % requirement)
				      .str());
			} else {
				if (!search_result->second &&
				    kAddOnCategories.at(search_result->first.category).can_disable_addons) {
					warn_requirements.push_back(
					   (boost::format(_("· ‘%1$s’ requires ‘%2$s’ which is disabled")) %
					    addon->first.descname() % search_result->first.descname())
					      .str());
				}
				if (too_late) {
					warn_requirements.push_back(
					   (boost::format(
					       _("· ‘%1$s’ requires ‘%2$s’ which is listed below the requiring add-on")) %
					    addon->first.descname() % search_result->first.descname())
					      .str());
				}
			}
			// Also warn if the add-on's requirements are present in the wrong order
			// (e.g. when A requires B,C but they are ordered C,B,A)
			for (const std::string& previous_requirement : addon->first.requirements) {
				if (previous_requirement == requirement) {
					break;
				}
				// check if `previous_requirement` comes before `requirement`
				std::string prev_descname;
				for (const AddOnState& a : g_addons) {
					if (a.first.internal_name == previous_requirement) {
						prev_descname = a.first.descname();
						break;
					} else if (a.first.internal_name == requirement) {
						warn_requirements.push_back(
						   (boost::format(
						       _("· ‘%1$s’ requires first ‘%2$s’ and then ‘%3$s’, but they are "
						         "listed in the wrong order")) %
						    addon->first.descname() % prev_descname % search_result->first.descname())
						      .str());
						break;
					}
				}
			}
		}
	}
	if (warn_requirements.empty()) {
		warn_requirements_.set_text("");
		warn_requirements_.set_tooltip("");
	} else {
		const unsigned nr_warnings = warn_requirements.size();
		std::string list;
		for (const std::string& msg : warn_requirements) {
			if (!list.empty()) {
				list += "<br>";
			}
			list += msg;
		}
		warn_requirements_.set_text(
		   (boost::format("<rt><p>%s</p><p>%s</p></rt>") %
		    g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
		       .as_font_tag((boost::format(ngettext(
		                        _("%u Dependency Error"), _("%u Dependency Errors"), nr_warnings)) %
		                     nr_warnings)
		                       .str()) %
		    g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph).as_font_tag(list))
		      .str());
		warn_requirements_.set_tooltip(_("Add-Ons with dependency errors may work incorrectly or "
		                                 "prevent games and maps from loading."));
	}
	autofix_dependencies_.set_enabled(!warn_requirements.empty());
	layout();
}

void AddOnsCtrl::layout() {
	if (!is_minimal()) {
		main_box_.set_size(get_inner_w(), get_inner_h());

		warn_requirements_.set_visible(!warn_requirements_.get_text().empty());

		// Box layouting does not work well together with this scrolling tab panel, so we
		// use a plain Panel as a fixed-size placeholder which is layouted by the box and
		// we manually position and resize the real tab panel on top of the placeholder.
		tabs_.set_pos(Vector2i(tabs_placeholder_.get_x(), tabs_placeholder_.get_y()));
		tabs_.set_size(tabs_placeholder_.get_w(), tabs_placeholder_.get_h());

		installed_addons_outer_wrapper_.set_max_size(
		   tabs_placeholder_.get_w(), tabs_placeholder_.get_h() - 2 * kRowButtonSize);
		browse_addons_inner_wrapper_.set_max_size(
		   tabs_placeholder_.get_w(),
		   tabs_placeholder_.get_h() - 2 * kRowButtonSize - browse_addons_buttons_box_.get_h());
	}

	UI::Window::layout();
}

static void install_translation(const std::string& temp_locale_path,
                                const std::string& addon_name) {
	assert(g_fs->file_exists(temp_locale_path));

	// NOTE:
	// gettext expects a directory structure such as
	// "~/.widelands/addons_i18n/nds/LC_MESSAGES/addon_name.wad.mo"
	// where "nds" is the language abbreviation and "addon_name.wad" the add-on's name.
	// If we use a different structure, gettext will not find the translations!

	const std::string temp_filename = g_fs->fs_filename(temp_locale_path.c_str());  // nds.mo.tmp
	const std::string locale = temp_filename.substr(0, temp_filename.find('.'));    // nds

	const std::string new_locale_dir = i18n::kAddOnLocaleDir + FileSystem::file_separator() +
	                                   locale + FileSystem::file_separator() +
	                                   "LC_MESSAGES";  // addons_i18n/nds/LC_MESSAGES
	g_fs->ensure_directory_exists(new_locale_dir);

	const std::string new_locale_path =
	   new_locale_dir + FileSystem::file_separator() + addon_name + ".mo";

	assert(!g_fs->is_directory(new_locale_path));
	if (g_fs->file_exists(new_locale_path)) {
		// delete the outdated translation if present
		g_fs->fs_unlink(new_locale_path);
	}
	assert(!g_fs->file_exists(new_locale_path));

	// move translation file from temp location to the correct place
	g_fs->fs_rename(temp_locale_path, new_locale_path);

	assert(g_fs->file_exists(new_locale_path));
	assert(!g_fs->file_exists(temp_locale_path));
}

// TODO(Nordfriese): install() and upgrade() should also (recursively) install the add-on's
// requirements
void AddOnsCtrl::install(const AddOnInfo& remote) {
	ProgressIndicatorWindow piw(this, remote.descname());

	g_fs->ensure_directory_exists(kAddOnDir);

	piw.progressbar().set_total(remote.file_list.files.size() + remote.file_list.locales.size());

	const std::string path = download_addon(piw, remote);

	if (path.empty()) {
		// downloading failed
		return;
	}

	// Install the add-on
	{
		const std::string new_path = kAddOnDir + FileSystem::file_separator() + remote.internal_name;

		assert(g_fs->is_directory(path));
		if (g_fs->file_exists(new_path)) {
			// erase leftovers from manual uninstallations
			g_fs->fs_unlink(new_path);
		}
		assert(!g_fs->file_exists(new_path));

		g_fs->fs_rename(path, new_path);

		assert(!g_fs->file_exists(path));
		assert(g_fs->is_directory(new_path));
	}

	// Now download the translations
	for (const std::string& temp_locale_path : download_i18n(piw, remote)) {
		install_translation(temp_locale_path, remote.internal_name);
	}

	g_addons.push_back(std::make_pair(preload_addon(remote.internal_name), true));
}

// Upgrades the specified add-on. If `full_upgrade` is `false`, only translations will be updated.
void AddOnsCtrl::upgrade(const AddOnInfo& remote, const bool full_upgrade) {
	ProgressIndicatorWindow piw(this, remote.descname());

	piw.progressbar().set_total(remote.file_list.locales.size() +
	                            (full_upgrade ? remote.file_list.files.size() : 0));

	if (full_upgrade) {
		g_fs->ensure_directory_exists(kAddOnDir);

		const std::string path = download_addon(piw, remote);
		if (path.empty()) {
			// downloading failed
			return;
		}

		// Upgrade the add-on
		const std::string new_path = kAddOnDir + FileSystem::file_separator() + remote.internal_name;

		assert(g_fs->is_directory(path));
		assert(g_fs->is_directory(new_path));

		g_fs->fs_unlink(new_path);  // Uninstall the old version…

		assert(!g_fs->file_exists(new_path));

		g_fs->fs_rename(path, new_path);  // …and replace with the new one.

		assert(g_fs->is_directory(new_path));
		assert(!g_fs->file_exists(path));
	}

	// Now download the translations
	for (const std::string& temp_locale_path : download_i18n(piw, remote)) {
		install_translation(temp_locale_path, remote.internal_name);
	}

	for (auto& pair : g_addons) {
		if (pair.first.internal_name == remote.internal_name) {
			pair.first = preload_addon(remote.internal_name);
			return;
		}
	}
	NEVER_HERE();
}

std::string AddOnsCtrl::download_addon(ProgressIndicatorWindow& piw, const AddOnInfo& info) {
	try {
		piw.set_message_1((boost::format(_("Downloading ‘%s’…")) % info.descname()).str());

		const std::string temp_dir = g_fs->canonicalize_name(
		   i18n::get_homedir() + "/" + kTempFileDir + "/" + info.internal_name + kTempFileExtension);
		if (g_fs->file_exists(temp_dir)) {
			// cleanse outdated cache
			g_fs->fs_unlink(temp_dir);
		}
		g_fs->ensure_directory_exists(temp_dir);
		for (const std::string& subdir : info.file_list.directories) {
			std::string d(temp_dir);
			d.push_back('/');
			d += subdir;
			g_fs->ensure_directory_exists(g_fs->canonicalize_name(d));
		}

		piw.action_params = info.file_list.files;
		piw.action_when_thinking = [this, &info, &piw,
		                            temp_dir](const std::string& file_to_download) {
			piw.set_message_2(file_to_download);

			std::string checksum;
			for (size_t i = 0; i < info.file_list.files.size(); ++i) {
				if (info.file_list.files[i] == file_to_download) {
					checksum = info.file_list.checksums[i];
					break;
				}
			}
			if (checksum.empty()) {
				throw wexception("Checksum for '%s' not found", file_to_download.c_str());
			}

			network_handler_.download_addon_file(
			   info.internal_name + "/" + file_to_download, checksum,
			   g_fs->canonicalize_name(temp_dir + "/" + file_to_download));
			piw.progressbar().set_state(piw.progressbar().get_state() + 1);
		};
		piw.run<UI::Panel::Returncodes>();

		return temp_dir;
	} catch (const std::exception& e) {
		log_err("download_addon %s: %s", info.internal_name.c_str(), e.what());
		piw.end_modal(UI::Panel::Returncodes::kBack);
		UI::WLMessageBox w(
		   this, UI::WindowStyle::kFsMenu, _("Error"),
		   (boost::format(
		       _("The add-on ‘%1$s’ could not be downloaded from the server. Installing/upgrading "
		         "this add-on will be skipped.\n\nError Message:\n%2$s")) %
		    info.internal_name.c_str() % e.what())
		      .str(),
		   UI::WLMessageBox::MBoxType::kOk);
		w.run<UI::Panel::Returncodes>();
	}
	return "";
}

std::set<std::string> AddOnsCtrl::download_i18n(ProgressIndicatorWindow& piw,
                                                const AddOnInfo& info) {
	try {
		piw.set_message_1(
		   (boost::format(_("Downloading translations for ‘%s’…")) % info.descname()).str());

		// Download all known locales one by one.
		// TODO(Nordfriese): When we have a real server, we should let the server provide us
		// with info which locales are actually present on the server rather than trying to
		// fetch all we know about.
		// My dummy "server" currently has only 'nds' translations, and the attempts to download
		// the others take about one minute extra, which could be avoided.
		// In net_addons.cc, we can then also fail with a wexception if downloading one of them
		// fails, instead of only logging the error as we do now.

		std::set<std::string> result;
		piw.die_after_last_action = true;
		piw.action_params = info.file_list.locales;
		piw.action_when_thinking = [this, &info, &result,
		                            &piw](const std::string& locale_to_download) {
			piw.set_message_2(locale_to_download);

			std::string checksum;
			for (size_t i = 0; i < info.file_list.locales.size(); ++i) {
				if (info.file_list.locales[i] == locale_to_download) {
					checksum = info.file_list.checksums[info.file_list.files.size() + i];
					break;
				}
			}
			if (checksum.empty()) {
				throw wexception("Checksum for '%s' not found", locale_to_download.c_str());
			}

			const std::string str =
			   network_handler_.download_i18n(info.internal_name, checksum, locale_to_download);
			assert(!result.count(str));
			if (!str.empty()) {
				result.insert(str);
			}
			piw.progressbar().set_state(piw.progressbar().get_state() + 1);
		};
		piw.run<UI::Panel::Returncodes>();

		// If the translations were downloaded correctly, we also update the i18n version info
		if (!result.empty()) {
			Profile prof(kAddOnLocaleVersions.c_str());
			prof.pull_section("global").set_natural(info.internal_name.c_str(), info.i18n_version);
			prof.write(kAddOnLocaleVersions.c_str(), false);
		}

		return result;
	} catch (const std::exception& e) {
		log_err("download_i18n %s: %s", info.internal_name.c_str(), e.what());
		piw.end_modal(UI::Panel::Returncodes::kBack);
		UI::WLMessageBox w(
		   this, UI::WindowStyle::kFsMenu, _("Error"),
		   (boost::format(_(
		       "The translation files for the add-on ‘%1$s’ could not be downloaded from the server. "
		       "Installing/upgrading the translations for this add-on will be skipped.\n\nError "
		       "Message:\n%2$s")) %
		    info.internal_name.c_str() % e.what())
		      .str(),
		   UI::WLMessageBox::MBoxType::kOk);
		w.run<UI::Panel::Returncodes>();
	}
	return {};
}

static void uninstall(AddOnsCtrl* ctrl, const AddOnInfo& info) {
	if (!(SDL_GetModState() & KMOD_CTRL)) {
		UI::WLMessageBox w(
		   ctrl, UI::WindowStyle::kFsMenu, _("Uninstall"),
		   (boost::format(_("Are you certain that you want to uninstall this add-on?\n\n"
		                    "%1$s\n"
		                    "by %2$s\n"
		                    "Version %3$u\n"
		                    "Category: %4$s\n"
		                    "%5$s\n")) %
		    info.descname() % info.author() % info.version %
		    kAddOnCategories.at(info.category).descname() % info.description())
		      .str(),
		   UI::WLMessageBox::MBoxType::kOkCancel);
		if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
			return;
		}
	}

	// Delete the add-on…
	g_fs->fs_unlink(kAddOnDir + FileSystem::file_separator() + info.internal_name);

	// …and its translations
	for (const std::string& locale : g_fs->list_directory(i18n::kAddOnLocaleDir)) {
		g_fs->fs_unlink(locale + FileSystem::file_separator() + "LC_MESSAGES" +
		                FileSystem::file_separator() + info.internal_name + ".mo");
	}

	for (auto it = g_addons.begin(); it != g_addons.end(); ++it) {
		if (it->first.internal_name == info.internal_name) {
			g_addons.erase(it);
			return ctrl->rebuild();
		}
	}
	NEVER_HERE();
}

// UNTESTED
// Automatically fix all dependency errors by reordering add-ons and downloading missing ones.
// We make no guarantees inhowfar the existing order is preserved
// (e.g. if A currently comes before B, it may come after B after reordering even if
// there is no direct or indirect dependency relation between A and B).
void AddOnsCtrl::autofix_dependencies() {
	std::set<std::string> missing_requirements;

// Step 1: Enable all dependencies
step1:
	for (const AddOnState& addon_to_fix : g_addons) {
		if (addon_to_fix.second ||
		    !kAddOnCategories.at(addon_to_fix.first.category).can_disable_addons) {
			bool anything_changed = false;
			bool found = false;
			for (const std::string& requirement : addon_to_fix.first.requirements) {
				for (AddOnState& a : g_addons) {
					if (a.first.internal_name == requirement) {
						found = true;
						if (!a.second) {
							a.second = true;
							anything_changed = true;
						}
						break;
					}
				}
				if (!found) {
					missing_requirements.insert(requirement);
				}
			}
			if (anything_changed) {
				// concurrent modification – we need to start over
				goto step1;
			}
		}
	}

	// Step 2: Download missing add-ons
	for (const std::string& addon_to_install : missing_requirements) {
		bool found = false;
		for (const AddOnInfo& info : remotes_) {
			if (info.internal_name == addon_to_install) {
				install(info);
				found = true;
				break;
			}
		}
		if (!found) {
			UI::WLMessageBox w(
			   this, UI::WindowStyle::kFsMenu, _("Error"),
			   (boost::format(_("The required add-on ‘%s’ could not be found on the server.")) %
			    addon_to_install)
			      .str(),
			   UI::WLMessageBox::MBoxType::kOk);
			w.run<UI::Panel::Returncodes>();
		}
	}

	// Step 3: Get all add-ons into the correct order
	std::map<std::string, AddOnState> all_addons;

	for (const AddOnState& aos : g_addons) {
		all_addons[aos.first.internal_name] = aos;
	}

	std::multimap<unsigned /* number of dependencies */, AddOnState> addons_tree;
	for (const auto& pair : all_addons) {
		addons_tree.emplace(
		   std::make_pair(count_all_dependencies(pair.first, all_addons), pair.second));
	}
	// The addons_tree now contains a list of all add-ons sorted by number
	// of (direct plus indirect) dependencies
	g_addons.clear();
	for (const auto& pair : addons_tree) {
		g_addons.push_back(AddOnState(pair.second));
	}

	rebuild();
}

InstalledAddOnRow::InstalledAddOnRow(Panel* parent,
                                     AddOnsCtrl* ctrl,
                                     const AddOnInfo& info,
                                     bool enabled)
   : UI::Panel(parent,
               UI::PanelStyle::kFsMenu,
               0,
               0,
               3 * kRowButtonSize,
               2 * kRowButtonSize + 3 * kRowButtonSpacing),
     info_(info),
     uninstall_(this,
                "uninstall",
                0,
                0,
                24,
                24,
                UI::ButtonStyle::kFsMenuSecondary,
                g_image_cache->get("images/wui/menus/exit.png"),
                _("Uninstall")),
     toggle_enabled_(
        kAddOnCategories.at(info.category).can_disable_addons ?
           new UI::Button(this,
                          "on-off",
                          0,
                          0,
                          24,
                          24,
                          UI::ButtonStyle::kFsMenuSecondary,
                          g_image_cache->get(enabled ? "images/ui_basic/checkbox_checked.png" :
                                                       "images/ui_basic/checkbox_empty.png"),
                          enabled ? _("Disable") : _("Enable"),
                          UI::Button::VisualState::kFlat) :
           nullptr),
     category_(
        this, UI::PanelStyle::kFsMenu, g_image_cache->get(kAddOnCategories.at(info.category).icon)),
     version_(this,
              UI::PanelStyle::kFsMenu,
              UI::FontStyle::kFsMenuInfoPanelHeading,
              0,
              0,
              0,
              0,
              std::to_string(info.version),
              UI::Align::kCenter),
     txt_(this,
          0,
          0,
          24,
          24,
          UI::PanelStyle::kFsMenu,
          (boost::format("<rt><p>%s</p><p>%s</p><p>%s</p></rt>") %
           (boost::format(
               /** TRANSLATORS: Add-On localized name as header (Add-On internal name in italics) */
               _("%1$s %2$s")) %
            g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
               .as_font_tag(info.descname()) %
            g_style_manager->font_style(UI::FontStyle::kItalic)
               .as_font_tag((boost::format(_("(%s)")) % info.internal_name).str()))
              .str() %
           g_style_manager->font_style(UI::FontStyle::kItalic)
              .as_font_tag((boost::format(_("by %s")) % info.author()).str()) %
           g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
              .as_font_tag(info.description()))
             .str()) {

	uninstall_.sigclicked.connect([ctrl, info]() { uninstall(ctrl, info); });
	if (toggle_enabled_) {
		toggle_enabled_->sigclicked.connect([this, ctrl, info]() {
			for (auto& pair : g_addons) {
				if (pair.first.internal_name == info.internal_name) {
					pair.second = !pair.second;
					toggle_enabled_->set_pic(
					   g_image_cache->get(pair.second ? "images/ui_basic/checkbox_checked.png" :
					                                    "images/ui_basic/checkbox_empty.png"));
					toggle_enabled_->set_tooltip(pair.second ? _("Disable") : _("Enable"));
					return ctrl->update_dependency_errors();
				}
			}
			NEVER_HERE();
		});
	}
	category_.set_handle_mouse(true);
	category_.set_tooltip(
	   (boost::format(_("Category: %s")) % kAddOnCategories.at(info.category).descname()).str());
	version_.set_handle_mouse(true);
	version_.set_tooltip(
	   /** TRANSLATORS: (MajorVersion).(MinorVersion) */
	   (boost::format(_("Version: %1$u.%2$u")) % info.version % info.i18n_version).str());
	set_can_focus(true);
	layout();
}

void InstalledAddOnRow::layout() {
	UI::Panel::layout();
	if (get_w() <= 2 * kRowButtonSize + 2 * kRowButtonSpacing) {
		// size not yet set
		return;
	}
	set_desired_size(get_w(), 2 * kRowButtonSize + 3 * kRowButtonSpacing);
	for (UI::Panel* p : std::vector<UI::Panel*>{&uninstall_, &category_, &version_}) {
		p->set_size(kRowButtonSize, kRowButtonSize);
	}
	if (toggle_enabled_) {
		toggle_enabled_->set_size(kRowButtonSize, kRowButtonSize);
		toggle_enabled_->set_pos(Vector2i(get_w() - kRowButtonSize, 0));
	}
	category_.set_pos(Vector2i(get_w() - 2 * kRowButtonSize - kRowButtonSpacing, 0));
	version_.set_pos(Vector2i(
	   get_w() - 2 * kRowButtonSize - kRowButtonSpacing, kRowButtonSize + 3 * kRowButtonSpacing));
	uninstall_.set_pos(Vector2i(get_w() - kRowButtonSize, kRowButtonSize + 3 * kRowButtonSpacing));
	txt_.set_size(get_w() - 2 * kRowButtonSize - 2 * kRowButtonSpacing,
	              2 * kRowButtonSize + 3 * kRowButtonSpacing);
	txt_.set_pos(Vector2i(0, 0));
}

void InstalledAddOnRow::draw(RenderTarget& r) {
	UI::Panel::draw(r);
	r.brighten_rect(Recti(0, 0, get_w(), get_h()), has_focus() ? -30 : -20);
}
void RemoteAddOnRow::draw(RenderTarget& r) {
	UI::Panel::draw(r);
	r.brighten_rect(Recti(0, 0, get_w(), get_h()), -20);
}

static std::string time_string(const std::time_t& time) {
	std::ostringstream oss("");
	oss << std::put_time(std::localtime(&time), "%c");
	return oss.str();
}
static std::string filesize_string(const uint32_t bytes) {
	if (bytes > 1000000000) {
		return (boost::format(_("%.2f GB")) % (bytes / 1000000000.f)).str();
	} else if (bytes > 1000000) {
		return (boost::format(_("%.2f MB")) % (bytes / 1000000.f)).str();
	} else if (bytes > 1000) {
		return (boost::format(_("%.2f kB")) % (bytes / 1000.f)).str();
	} else {
		return (boost::format(_("%u bytes")) % bytes).str();
	}
}

struct RemoteInteractionWindow : public UI::Window {
public:
	RemoteInteractionWindow(AddOnsCtrl& parent, const AddOnInfo& info)
	   : UI::Window(parent.get_parent(),
	                UI::WindowStyle::kFsMenu,
	                info.internal_name,
	                parent.get_x() + kRowButtonSize,
	                parent.get_y() - kRowButtonSize,
	                parent.get_inner_w() - 2 * kRowButtonSize,
	                parent.get_inner_h() + 2 * kRowButtonSize,
	                info.descname()),
	     box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
	     txt_(&box_, 0, 0, 0, 0, UI::PanelStyle::kFsMenu, "", UI::Align::kLeft),
	     voting_(&box_,
	             "voting",
	             0,
	             0,
	             0,
	             11,
	             kRowButtonSize - kRowButtonSpacing,
	             _("Your vote"),
	             UI::DropdownType::kTextual,
	             UI::PanelStyle::kFsMenu,
	             UI::ButtonStyle::kFsMenuSecondary),
	     comment_(new UI::MultilineEditbox(&box_, 0, 0, get_inner_w(), 80, UI::PanelStyle::kFsMenu)),
	     submit_(
	        &box_, "submit", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Submit comment")),
	     ok_(&box_, "ok", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("OK")) {
		std::string text = "<rt><p>";
		text += g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
		           .as_font_tag((
		              info.votes ?
		                 (boost::format(ngettext("Average rating: %1$.3f (%2$u vote)",
		                                         "Average rating: %1$.3f (%2$u votes)", info.votes)) %
		                  info.average_rating % info.votes)
		                    .str() :
		                 _("No votes yet")));
		text += "</p><vspace gap=32><p>";
		text += g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
		           .as_font_tag(info.user_comments.empty() ?
		                           _("No comments yet.") :
		                           (boost::format(ngettext(
		                               "%u comment:", "%u comments:", info.user_comments.size())) %
		                            info.user_comments.size())
		                              .str());

		for (const auto& comment : info.user_comments) {
			text += "</p><vspace gap=32><p>";
			text += g_style_manager->font_style(UI::FontStyle::kItalic)
			           .as_font_tag(time_string(comment.timestamp));
			text += "<br>";
			text += g_style_manager->font_style(UI::FontStyle::kItalic)
			           .as_font_tag((boost::format(_("‘%1$s’ commented on version %2$u:")) %
			                         comment.username % comment.version)
			                           .str());
			text += "<br>";
			text += g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			           .as_font_tag(comment.message);
		}

		text += "</p></rt>";
		txt_.set_text(text);

		comment_->set_text("Writing comments and rating add-ons is not yet implemented.");
		ok_.sigclicked.connect([this]() { end_modal(UI::Panel::Returncodes::kBack); });

		voting_.add(_("Not voted"), 0, nullptr, true);
		for (unsigned i = 1; i <= 10; ++i) {
			voting_.add(std::to_string(i), i);
		}

		// not yet implemented
		voting_.set_enabled(false);
		submit_.set_enabled(false);

		box_.add(&txt_, UI::Box::Resizing::kExpandBoth);
		box_.add_space(kRowButtonSpacing);
		box_.add(&voting_, UI::Box::Resizing::kFullSize);
		box_.add_space(kRowButtonSpacing);
		box_.add(comment_, UI::Box::Resizing::kFullSize);
		box_.add(&submit_, UI::Box::Resizing::kFullSize);
		box_.add_space(kRowButtonSpacing);
		box_.add(&ok_, UI::Box::Resizing::kFullSize);
		box_.add_space(kRowButtonSpacing);
		box_.set_size(get_inner_w(), get_inner_h());
	}

private:
	UI::Box box_;
	UI::MultilineTextarea txt_;
	UI::Dropdown<uint8_t> voting_;
	UI::MultilineEditbox* comment_;
	UI::Button submit_, ok_;
};

RemoteAddOnRow::RemoteAddOnRow(Panel* parent,
                               AddOnsCtrl* ctrl,
                               const AddOnInfo& info,
                               uint32_t installed_version,
                               uint32_t installed_i18n_version)
   : UI::Panel(parent, UI::PanelStyle::kFsMenu, 0, 0, 3 * kRowButtonSize, 4 * kRowButtonSize),
     info_(info),
     install_(this,
              "install",
              0,
              0,
              24,
              24,
              UI::ButtonStyle::kFsMenuSecondary,
              g_image_cache->get("images/ui_basic/continue.png"),
              _("Install")),
     upgrade_(this,
              "upgrade",
              0,
              0,
              24,
              24,
              UI::ButtonStyle::kFsMenuSecondary,
              g_image_cache->get("images/wui/buildings/menu_up_train.png"),
              _("Upgrade")),
     uninstall_(this,
                "uninstall",
                0,
                0,
                24,
                24,
                UI::ButtonStyle::kFsMenuSecondary,
                g_image_cache->get("images/wui/menus/exit.png"),
                _("Uninstall")),
     interact_(this,
               "interact",
               0,
               0,
               24,
               24,
               UI::ButtonStyle::kFsMenuSecondary,
               "…",
               _("Comments and Votes")),
     category_(
        this, UI::PanelStyle::kFsMenu, g_image_cache->get(kAddOnCategories.at(info.category).icon)),
     verified_(this,
               UI::PanelStyle::kFsMenu,
               g_image_cache->get(info.verified ? "images/ui_basic/list_selected.png" :
                                                  "images/ui_basic/stop.png")),
     version_(this,
              UI::PanelStyle::kFsMenu,
              UI::FontStyle::kFsMenuInfoPanelHeading,
              0,
              0,
              0,
              0,
              std::to_string(info.version),
              UI::Align::kCenter),
     bottom_row_left_(this,
                      UI::PanelStyle::kFsMenu,
                      UI::FontStyle::kFsTooltip,
                      0,
                      0,
                      0,
                      0,
                      time_string(info.upload_timestamp),
                      UI::Align::kLeft),
     bottom_row_right_(
        this,
        UI::PanelStyle::kFsMenu,
        UI::FontStyle::kFsTooltip,
        0,
        0,
        0,
        0,
        info.internal_name.empty() ?
           "" :
           (boost::format(
               /** TRANSLATORS: Filesize · Download count · Average rating · Number of comments */
               _("%1$s   ⬇ %2$u   ★ %3$s   “” %4$u")) %
            filesize_string(info.total_file_size) % info.download_count %
            (info.votes ? (boost::format("%.2f") % info.average_rating).str() : "–") %
            info.user_comments.size())
              .str(),
        UI::Align::kRight),
     txt_(this,
          0,
          0,
          24,
          24,
          UI::PanelStyle::kFsMenu,
          (boost::format("<rt><p>%s</p><p>%s</p><p>%s</p></rt>")
           /** TRANSLATORS: Add-On localized name as header (Add-On internal name in italics) */
           % (boost::format(_("%1$s %2$s")) %
              g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelHeading)
                 .as_font_tag(info.descname()) %
              g_style_manager->font_style(UI::FontStyle::kItalic)
                 .as_font_tag((boost::format(_("(%s)")) % info.internal_name).str()))
                .str() %
           g_style_manager->font_style(UI::FontStyle::kItalic)
              .as_font_tag(info.author() == info.upload_username ?
                              (boost::format(_("by %s")) % info.author()).str() :
                              (boost::format(_("by %1$s (uploaded by %2$s)")) % info.author() %
                               info.upload_username)
                                 .str()) %
           g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
              .as_font_tag(info.description()))
             .str()),
     full_upgrade_possible_(installed_version < info.version) {

	assert(installed_version <= info.version);
	assert(installed_i18n_version <= info.i18n_version);

	interact_.sigclicked.connect([ctrl, info]() {
		RemoteInteractionWindow m(*ctrl, info);
		m.run<UI::Panel::Returncodes>();
	});
	uninstall_.sigclicked.connect([ctrl, info]() { uninstall(ctrl, info); });
	install_.sigclicked.connect([ctrl, info]() {
		// Ctrl-click skips the confirmation. Never skip for non-verified stuff though.
		if (!info.verified || !(SDL_GetModState() & KMOD_CTRL)) {
			UI::WLMessageBox w(
			   ctrl, UI::WindowStyle::kFsMenu, _("Install"),
			   (boost::format(_("Are you certain that you want to install this add-on?\n\n"
			                    "%1$s\n"
			                    "by %2$s\n"
			                    "%3$s\n"
			                    "Version %4$u\n"
			                    "Category: %5$s\n"
			                    "%6$s\n")) %
			    info.descname() % info.author() % (info.verified ? _("Verified") : _("NOT VERIFIED")) %
			    info.version % kAddOnCategories.at(info.category).descname() % info.description())
			      .str(),
			   UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
				return;
			}
		}
		ctrl->install(info);
		ctrl->rebuild();
	});
	upgrade_.sigclicked.connect([this, ctrl, info, installed_version]() {
		if (!info.verified || !(SDL_GetModState() & KMOD_CTRL)) {
			UI::WLMessageBox w(
			   ctrl, UI::WindowStyle::kFsMenu, _("Upgrade"),
			   (boost::format(_("Are you certain that you want to upgrade this add-on?\n\n"
			                    "%1$s\n"
			                    "by %2$s\n"
			                    "%3$s\n"
			                    "Installed version: %4$u\n"
			                    "Available version: %5$u\n"
			                    "Category: %6$s\n"
			                    "%7$s\n")) %
			    info.descname() % info.author() % (info.verified ? _("Verified") : _("NOT VERIFIED")) %
			    installed_version % info.version % kAddOnCategories.at(info.category).descname() %
			    info.description())
			      .str(),
			   UI::WLMessageBox::MBoxType::kOkCancel);
			if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
				return;
			}
		}
		ctrl->upgrade(info, full_upgrade_possible_);
		ctrl->rebuild();
	});
	if (info.internal_name.empty()) {
		install_.set_enabled(false);
		upgrade_.set_enabled(false);
		uninstall_.set_enabled(false);
		interact_.set_enabled(false);
	} else if (installed_version == kNotInstalled) {
		uninstall_.set_enabled(false);
		upgrade_.set_enabled(false);
	} else {
		install_.set_enabled(false);
		upgrade_.set_enabled(full_upgrade_possible_ || installed_i18n_version < info.i18n_version);
	}

	for (UI::Panel* p :
	     std::vector<UI::Panel*>{&category_, &version_, &verified_, &bottom_row_right_}) {
		p->set_handle_mouse(true);
	}
	category_.set_tooltip(
	   (boost::format(_("Category: %s")) % kAddOnCategories.at(info.category).descname()).str());
	version_.set_tooltip(
	   /** TRANSLATORS: (MajorVersion).(MinorVersion) */
	   (boost::format(_("Version: %1$u.%2$u")) % info.version % info.i18n_version).str());
	verified_.set_tooltip(
	   info.internal_name.empty() ?
	      _("Error") :
	      info.verified ?
	      _("Verified by the Widelands Development Team") :
	      _("This add-on was not checked by the Widelands Development Team yet. We cannot guarantee "
	        "that it does not contain harmful or offensive content."));
	bottom_row_right_.set_tooltip(
	   info.internal_name.empty() ?
	      "" :
	      (boost::format("%s<br>%s<br>%s<br>%s") %
	       (boost::format(
	           ngettext("Total size: %u byte", "Total size: %u bytes", info.total_file_size)) %
	        info.total_file_size)
	          .str() %
	       (boost::format(ngettext("%u download", "%u downloads", info.download_count)) %
	        info.download_count)
	          .str() %
	       (info.votes ?
	           (boost::format(ngettext("Average rating: %1$.3f (%2$u vote)",
	                                   "Average rating: %1$.3f (%2$u votes)", info.votes)) %
	            info.average_rating % info.votes)
	              .str() :
	           _("No votes yet")) %
	       (boost::format(ngettext("%u comment", "%u comments", info.user_comments.size())) %
	        info.user_comments.size())
	          .str())
	         .str());

	layout();
}

void RemoteAddOnRow::layout() {
	UI::Panel::layout();
	if (get_w() <= 2 * kRowButtonSize + 2 * kRowButtonSpacing) {
		// size not yet set
		return;
	}
	set_desired_size(get_w(), 4 * kRowButtonSize);
	for (UI::Panel* p : std::vector<UI::Panel*>{
	        &install_, &uninstall_, &interact_, &upgrade_, &category_, &version_, &verified_}) {
		p->set_size(kRowButtonSize, kRowButtonSize);
	}
	install_.set_pos(Vector2i(get_w() - kRowButtonSize, 0));
	upgrade_.set_pos(Vector2i(get_w() - kRowButtonSize, kRowButtonSize));
	uninstall_.set_pos(Vector2i(get_w() - kRowButtonSize, 2 * kRowButtonSize));
	interact_.set_pos(Vector2i(get_w() - kRowButtonSize, 3 * kRowButtonSize));
	category_.set_pos(Vector2i(get_w() - kRowButtonSize * 2 - kRowButtonSpacing, 0));
	version_.set_pos(Vector2i(get_w() - kRowButtonSize * 2 - kRowButtonSpacing, kRowButtonSize));
	verified_.set_pos(
	   Vector2i(get_w() - kRowButtonSize * 2 - kRowButtonSpacing, 2 * kRowButtonSize));
	txt_.set_size(get_w() - 2 * kRowButtonSize - 2 * kRowButtonSpacing, 3 * kRowButtonSize);
	txt_.set_pos(Vector2i(0, 0));
	bottom_row_left_.set_size(get_w() / 2 - kRowButtonSize, kRowButtonSize - 2 * kRowButtonSpacing);
	bottom_row_right_.set_size(get_w() / 2 - kRowButtonSpacing, bottom_row_left_.get_h());
	bottom_row_left_.set_pos(Vector2i(0, 4 * kRowButtonSize - bottom_row_left_.get_h()));
	bottom_row_right_.set_pos(
	   Vector2i(bottom_row_left_.get_w(), 4 * kRowButtonSize - bottom_row_right_.get_h()));
}

bool RemoteAddOnRow::upgradeable() const {
	return upgrade_.enabled();
}
