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
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"
#include "ui_fsmenu/main.h"

class AddOnsCtrl;
struct ProgressIndicatorWindow;

struct InstalledAddOnRow : public UI::Panel {
	InstalledAddOnRow(Panel*, AddOnsCtrl*, const AddOnInfo&, bool enabled, bool is_first, bool is_last);
	~InstalledAddOnRow() override {
	}
	void layout() override;
	void draw(RenderTarget&) override;
private:
	UI::Button move_up_;
	UI::Button move_down_;
	UI::Button uninstall_;
	std::unique_ptr<UI::Button> toggle_enabled_;
	UI::Icon category_;
	UI::Textarea version_;
	UI::MultilineTextarea txt_;
};
struct RemoteAddOnRow : public UI::Panel {
	RemoteAddOnRow(Panel*, AddOnsCtrl*, const AddOnInfo&, uint32_t installed_version, uint32_t installed_i18n_version);
	~RemoteAddOnRow() override {
	}
	void layout() override;
	void draw(RenderTarget&) override;
	const AddOnInfo& info() const { return info_; }
	bool upgradeable() const;
	bool full_upgrade_possible() const { return full_upgrade_possible_; }
private:
	AddOnInfo info_;
	UI::Button install_, upgrade_, uninstall_, interact_;
	UI::Icon category_, verified_;
	UI::Textarea version_, bottom_row_left_, bottom_row_right_;
	UI::MultilineTextarea txt_;

	const bool full_upgrade_possible_;
};

class AddOnsCtrl : public UI::Window {
public:
	AddOnsCtrl(FullscreenMenuMain&);
	~AddOnsCtrl() override;

	void rebuild();
	void update_dependency_errors();

	void install(const AddOnInfo&);
	void upgrade(const AddOnInfo&, bool full_upgrade);

	bool handle_key(bool, SDL_Keysym) override;

protected:
	void layout() override;

private:
	FullscreenMenuMain& fsmm_;

	UI::Box main_box_;
	UI::MultilineTextarea warn_requirements_;
	UI::TabPanel tabs_;
	UI::Box installed_addons_wrapper_, browse_addons_wrapper_, installed_addons_box_, browse_addons_box_,
			filter_settings_, filter_name_box_, filter_buttons_box_;
	std::vector<RemoteAddOnRow*> browse_;
	UI::EditBox filter_name_;
	UI::Dropdown<std::string> filter_category_;
	UI::Checkbox filter_verified_;
	UI::Button ok_, filter_apply_, filter_reset_, upgrade_all_, refresh_, autofix_dependencies_;

	void autofix_dependencies();

	NetAddons network_handler_;

	std::vector<AddOnInfo> remotes_;
	void refresh_remotes();

	bool matches_filter(const AddOnInfo&, bool local);

	using Locales = std::set<std::string>;

	std::string download_addon(ProgressIndicatorWindow&, const AddOnInfo&);
	std::set<std::string> download_i18n(ProgressIndicatorWindow&, const AddOnInfo&, const Locales&);
};

#endif  // end of include guard: WL_UI_FSMENU_ADDONS_H
