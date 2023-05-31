/*
 * Copyright (C) 2020-2023 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_ADDONS_MANAGER_H
#define WL_UI_FSMENU_ADDONS_MANAGER_H

#include <memory>
#include <set>
#include <vector>

#include "logic/addons.h"
#include "network/net_addons.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/textinput.h"
#include "ui_basic/window.h"
#include "ui_fsmenu/main.h"

namespace AddOnsUI {

class RemoteAddOnRow;

constexpr int16_t kRowButtonSize = 32;
constexpr int16_t kRowButtonSpacing = 4;

std::string time_string(const std::time_t& time);
std::string filesize_string(uint32_t bytes);
std::string underline_tag(const std::string& text);

struct AddOnQuality {
	const Image* icon;
	const std::string name, description;

	static const std::map<unsigned, std::function<AddOnQuality()>> kQualities;

private:
	AddOnQuality(const Image* i, const std::string& n, const std::string& d)
	   : icon(i), name(n), description(d) {
	}
};

class AddOnsCtrl : public UI::UniqueWindow {
public:
	AddOnsCtrl(FsMenu::MainMenu&, UI::UniqueWindow::Registry&);
	~AddOnsCtrl() override;

	void rebuild(bool need_to_update_dependency_errors);
	void update_dependency_errors();

	void install_or_upgrade(std::shared_ptr<AddOns::AddOnInfo>, bool only_translations);
	void upload_addon(std::shared_ptr<AddOns::AddOnInfo>);

	bool handle_key(bool, SDL_Keysym) override;

	WindowLayoutID window_layout_id() const override {
		return UI::Window::WindowLayoutID::kFsMenuDefault;
	}

	AddOns::NetAddons& net() {
		return network_handler_;
	}

	std::shared_ptr<AddOns::AddOnInfo> find_remote(const std::string& name);
	bool is_remote(const std::string& name) const;
	void erase_remote(std::shared_ptr<AddOns::AddOnInfo>);

	const std::string& username() const {
		return username_;
	}
	void set_login(const std::string& username, const std::string& password, bool show_error);
	void update_login_button(UI::Button*);
	void login_button_clicked();

protected:
	void layout() override;
	void think() override;

private:
	enum class AddOnSortingCriteria {
		kNameABC,
		kNameCBA,
		kMostDownloads,
		kFewestDownloads,
		kHighestRating,
		kLowestRating,
		kOldest,
		kNewest
	};

	FsMenu::MainMenu& fsmm_;
	UI::Box main_box_, buttons_box_;
	UI::MultilineTextarea warn_requirements_;
	UI::Panel tabs_placeholder_;
	UI::TabPanel tabs_;
	UI::Box installed_addons_outer_wrapper_, installed_addons_inner_wrapper_,
	   installed_addons_buttons_box_, installed_addons_box_, browse_addons_outer_wrapper_,
	   browse_addons_inner_wrapper_, browse_addons_buttons_box_, browse_addons_buttons_box_lvbox_,
	   browse_addons_buttons_box_rvbox_, browse_addons_buttons_box_category_box_,
	   browse_addons_buttons_box_right_hbox_, browse_addons_box_, dev_box_;
	std::map<AddOns::AddOnCategory, UI::Checkbox*> filter_category_;
	std::vector<RemoteAddOnRow*> browse_;
	UI::EditBox filter_name_;
	UI::Checkbox filter_verified_;
	UI::Dropdown<AddOnSortingCriteria> sort_order_;
	UI::Dropdown<uint8_t> filter_quality_;
	UI::Dropdown<std::shared_ptr<AddOns::AddOnInfo>> upload_addon_, upload_screenshot_;
	UI::Checkbox upload_addon_accept_;
	UI::Button filter_reset_, upgrade_all_, refresh_, ok_, /* autofix_dependencies_, */ move_top_,
	   move_up_, move_down_, move_bottom_, launch_packager_, login_button_, contact_;
	UI::Textarea server_name_;

	void category_filter_changed(AddOns::AddOnCategory);
	void check_enable_move_buttons();
	std::shared_ptr<AddOns::AddOnInfo> selected_installed_addon() const;
	void focus_installed_addon_row(std::shared_ptr<AddOns::AddOnInfo>);

	// TODO(Nordfriese): Disabled autofix_dependencies for v1.0
	// void autofix_dependencies();

	AddOns::NetAddons network_handler_;

	AddOns::AddOnsList remotes_;
	void refresh_remotes(bool showall);
	bool matches_filter(std::shared_ptr<AddOns::AddOnInfo>);

	std::string username_, password_;
};

}  // namespace AddOnsUI

#endif  // end of include guard: WL_UI_FSMENU_ADDONS_MANAGER_H
