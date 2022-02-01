/*
 * Copyright (C) 2021-2022 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_ADDONS_PACKAGER_H
#define WL_UI_FSMENU_ADDONS_PACKAGER_H

#include <memory>

#include "logic/addons.h"
#include "logic/mutable_addon.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_fsmenu/addons/manager.h"
#include "ui_fsmenu/addons/packager_box.h"
#include "ui_fsmenu/addons/progress.h"

namespace FsMenu {
namespace AddOnsUI {

class AddOnsPackager : public UI::Window {
public:
	explicit AddOnsPackager(MainMenu&, AddOnsCtrl&);

	WindowLayoutID window_layout_id() const override {
		return UI::Window::WindowLayoutID::kFsMenuDefault;
	}

	void layout() override;
	void die() override;
	bool handle_key(const bool down, const SDL_Keysym code) override;

private:
	MainMenu& main_menu_;
	AddOnsCtrl& ctrl_;

	UI::Box main_box_, box_left_, box_right_, box_left_buttons_, box_right_subbox_header_hbox_,
	   box_right_subbox_header_box_left_, box_right_subbox_header_box_right_,
	   box_right_addon_specific_, box_right_bottombox_;
	UI::EditBox name_, author_, version_;
	UI::MultilineEditbox& descr_;
	UI::Button addon_new_, addon_delete_, discard_changes_, write_changes_, ok_;
	UI::Listselect<std::string> addons_;

	std::map<std::string /* internal name */, std::unique_ptr<AddOns::MutableAddOn>> mutable_addons_;
	std::map<AddOns::AddOnCategory, std::shared_ptr<AddOnsPackagerBox>> addon_boxes_;
	void initialize_mutable_addons();

	AddOns::MutableAddOn* get_selected();

	void rebuild_addon_list(const std::string& select);
	void addon_selected();

	std::map<std::string, bool /* delete this add-on */> addons_with_changes_;

	void current_addon_edited();

	inline void check_for_unsaved_changes() {
		discard_changes_.set_enabled(!addons_with_changes_.empty());
		write_changes_.set_enabled(!addons_with_changes_.empty());
	}

	void clicked_new_addon();
	void clicked_delete_addon();
	void clicked_discard_changes();
	void clicked_write_changes();
	bool do_write_addon_to_disk(const std::string& addon);

	bool update_in_progress_;
	ProgressIndicatorWindow progress_window_;
};

}  // namespace AddOnsUI
}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_ADDONS_PACKAGER_H
