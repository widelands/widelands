/*
 * Copyright (C) 2021 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_ADDONS_PACKAGER_H
#define WL_UI_FSMENU_ADDONS_PACKAGER_H

#include "logic/addons.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_fsmenu/main.h"

namespace FsMenu {

class AddOnsCtrl;

// Holds abstract information about add-ons, designed to be changed quickly and later written to
// disk
struct MutableAddOn {
	std::string internal_name, descname, description, author, version;
	AddOns::AddOnCategory category;

	// For Map Set add-ons only:
	struct DirectoryTree {
		std::map<std::string /* file name in add-on */, std::string /* path of source map */> maps;
		std::map<std::string, DirectoryTree> subdirectories;
	};
	DirectoryTree tree;
};

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
	   box_right_subbox_header_box_left_, box_right_subbox_header_box_right_, box_right_subbox_maps_dirstruct_hbox_, box_right_subbox_dirstruct_,
	   box_right_subbox_maps_list_, box_right_buttonsbox_, box_right_bottombox_;
	UI::EditBox name_, author_, version_;
	UI::MultilineEditbox& descr_;
	UI::Button addon_new_, addon_delete_, discard_changes_, write_changes_, ok_, map_add_,
	   map_add_dir_, map_delete_;
	UI::Listselect<std::string> addons_, dirstruct_, my_maps_;

	std::map<std::string /* internal name */, MutableAddOn> mutable_addons_;
	void initialize_mutable_addons();
	void recursively_initialize_tree_from_disk(const std::string& dir, MutableAddOn::DirectoryTree&);

	MutableAddOn* get_selected();

	void rebuild_addon_list(const std::string& select);
	void addon_selected();
	void rebuild_dirstruct(MutableAddOn&, const std::vector<std::string>& select = {});
	// To keep track of which selection index in `dirstruct_`
	// refers to which point of the file system hierarchy:
	std::vector<std::vector<std::string>> dirstruct_to_tree_map_;

	void do_recursively_rebuild_dirstruct(const MutableAddOn::DirectoryTree&,
	                                      unsigned level,
	                                      const std::string& path,
	                                      const std::vector<std::string>& map_path,
	                                      const std::vector<std::string>& select);

	std::map<std::string, bool /* delete this add-on */> addons_with_changes_;

	void current_addon_edited();

	inline void check_for_unsaved_changes() {
		discard_changes_.set_enabled(!addons_with_changes_.empty());
		write_changes_.set_enabled(!addons_with_changes_.empty());
	}

	enum class ModifyAction { kAddMap, kAddDir, kDeleteMapOrDir };
	void clicked_add_or_delete_map_or_dir(ModifyAction);
	void clicked_new_addon();
	void clicked_delete_addon();
	void clicked_discard_changes();
	void clicked_write_changes();
	bool do_write_addon_to_disk(const std::string& addon);

	bool update_in_progress_;
};

}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_ADDONS_PACKAGER_H
