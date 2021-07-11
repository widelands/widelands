/*
 * Copyright (C) 2020-2021 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_ADDONS_H
#define WL_UI_FSMENU_ADDONS_H

#include <memory>
#include <set>
#include <vector>

#include "logic/addons.h"
#include "network/net_addons.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/editbox.h"
#include "ui_basic/icon.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/progressbar.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"
#include "ui_fsmenu/main.h"

namespace FsMenu {

class AddOnsCtrl;

// TODO(Nordfriese): All classes defined in addons.h and addons.cc except AddOnsCtrl should
// be moved to one or more new file pairs. Also put them all in a new sub-namespace.

struct ProgressIndicatorWindow : public UI::Window {
	ProgressIndicatorWindow(UI::Panel* parent, const std::string& title);
	~ProgressIndicatorWindow() override = default;

	void set_message_1(const std::string& msg) {
		txt1_.set_text(msg);
	}
	void set_message_2(const std::string& msg) {
		txt2_.set_text(msg);
	}
	void set_message_3(const std::string& msg) {
		txt3_.set_text(msg);
	}
	UI::ProgressBar& progressbar() {
		return progress_;
	}

private:
	UI::Panel::ModalGuard modal_;
	UI::Box box_, hbox_;
	UI::Textarea txt1_, txt2_, txt3_;
	UI::ProgressBar progress_;
};

struct InstalledAddOnRow : public UI::Panel {
	InstalledAddOnRow(Panel*, AddOnsCtrl*, std::shared_ptr<AddOns::AddOnInfo>, bool enabled);
	~InstalledAddOnRow() override {
	}
	const std::shared_ptr<AddOns::AddOnInfo> info() const {
		return info_;
	}
	void layout() override;
	void draw(RenderTarget&) override;

private:
	std::shared_ptr<AddOns::AddOnInfo> info_;
	bool enabled_;
	UI::Button uninstall_, toggle_enabled_;
	UI::Icon icon_, category_;
	UI::Textarea version_;
	UI::MultilineTextarea txt_;
};
struct RemoteAddOnRow : public UI::Panel {
	RemoteAddOnRow(Panel*,
	               AddOnsCtrl*,
	               const std::shared_ptr<AddOns::AddOnInfo>,
	               const AddOns::AddOnVersion& installed_version,
	               uint32_t installed_i18n_version);
	~RemoteAddOnRow() override {
	}
	void layout() override;
	void draw(RenderTarget&) override;
	const std::shared_ptr<AddOns::AddOnInfo> info() const {
		return info_;
	}
	bool upgradeable() const;
	bool full_upgrade_possible() const {
		return full_upgrade_possible_;
	}

private:
	std::shared_ptr<AddOns::AddOnInfo> info_;
	UI::Button install_, upgrade_, uninstall_, interact_;
	UI::Icon icon_, category_, verified_;
	UI::Textarea version_, bottom_row_left_, bottom_row_right_;
	UI::MultilineTextarea txt_;

	const bool full_upgrade_possible_;
};

class AddOnsCtrl : public UI::UniqueWindow {
public:
	AddOnsCtrl(MainMenu&, UI::UniqueWindow::Registry&);
	~AddOnsCtrl() override;

	void rebuild();
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

	const AddOns::AddOnsList& get_remotes() const {
		return remotes_;
	}
	std::shared_ptr<AddOns::AddOnInfo> find_remote(const std::string& name);
	bool is_remote(const std::string& name) const;

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

	MainMenu& fsmm_;
	UI::Box main_box_, buttons_box_;
	UI::MultilineTextarea warn_requirements_;
	UI::Panel tabs_placeholder_;
	UI::TabPanel tabs_;
	UI::Box installed_addons_outer_wrapper_, installed_addons_inner_wrapper_,
	   installed_addons_buttons_box_, installed_addons_box_, browse_addons_outer_wrapper_,
	   browse_addons_inner_wrapper_, browse_addons_buttons_box_, browse_addons_buttons_inner_box_1_,
	   browse_addons_buttons_inner_box_2_, browse_addons_box_, dev_box_;
	std::map<AddOns::AddOnCategory, UI::Checkbox*> filter_category_;
	std::vector<RemoteAddOnRow*> browse_;
	UI::EditBox filter_name_;
	UI::Checkbox filter_verified_;
	UI::Dropdown<AddOnSortingCriteria> sort_order_;
	UI::Dropdown<std::shared_ptr<AddOns::AddOnInfo>> upload_addon_, upload_screenshot_;
	UI::Checkbox upload_addon_accept_;
	UI::Button filter_reset_, upgrade_all_, refresh_, ok_, /* autofix_dependencies_, */ move_top_,
	   move_up_, move_down_, move_bottom_, launch_packager_, login_button_, contact_;

	void category_filter_changed(AddOns::AddOnCategory);
	void check_enable_move_buttons();
	std::shared_ptr<AddOns::AddOnInfo> selected_installed_addon() const;
	void focus_installed_addon_row(std::shared_ptr<AddOns::AddOnInfo>);

	// TODO(Nordfriese): Disabled autofix_dependencies for v1.0
	// void autofix_dependencies();

	AddOns::NetAddons network_handler_;

	AddOns::AddOnsList remotes_;
	void refresh_remotes();
	bool matches_filter(std::shared_ptr<AddOns::AddOnInfo>);

	std::string username_, password_;
};
}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_ADDONS_H
