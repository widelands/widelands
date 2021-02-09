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

#ifndef WL_UI_FSMENU_ADDONS_PACKAGER_BOX_H
#define WL_UI_FSMENU_ADDONS_PACKAGER_BOX_H

#include "logic/mutable_addon.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_fsmenu/main.h"

namespace FsMenu {

class AddOnsPackagerBox : public UI::Box {
public:
	AddOnsPackagerBox(MainMenu& mainmenu,
	                  Panel* parent,
	                  UI::PanelStyle style,
	                  int32_t x,
	                  int32_t y,
	                  uint32_t orientation,
	                  int32_t max_x = 0,
	                  int32_t max_y = 0,
	                  uint32_t inner_spacing = 0);

	void set_modified_callback(std::function<void()> modified_callback) {
		modified_ = std::move(modified_callback);
	}

	virtual void load_addon(AddOns::MutableAddOn*) {
	}

protected:
	std::function<void()> modified_;
	MainMenu& main_menu_;
};

class MapsAddOnsPackagerBox : public AddOnsPackagerBox {
public:
	MapsAddOnsPackagerBox(MainMenu& mainmenu,
	                      Panel* parent,
	                      UI::PanelStyle style,
	                      int32_t x,
	                      int32_t y,
	                      uint32_t orientation,
	                      int32_t max_x = 0,
	                      int32_t max_y = 0,
	                      uint32_t inner_spacing = 0);
	void load_addon(AddOns::MutableAddOn*) override;

private:
	enum class ModifyAction { kAddMap, kAddDir, kDeleteMapOrDir };

	void rebuild_dirstruct(AddOns::MapsAddon*, const std::vector<std::string>& select = {});
	void do_recursively_rebuild_dirstruct(const AddOns::MapsAddon::DirectoryTree* tree,
	                                      const unsigned level,
	                                      const std::string& path,
	                                      const std::vector<std::string>& map_path,
	                                      const std::vector<std::string>& select);
	void recursively_initialize_tree_from_disk(const std::string& dir,
	                                           AddOns::MapsAddon::DirectoryTree&);
	void clicked_add_or_delete_map_or_dir(ModifyAction);

	// To keep track of which selection index in `dirstruct_`
	// refers to which point of the file system hierarchy:
	std::vector<std::vector<std::string>> dirstruct_to_tree_map_;

	UI::Box box_dirstruct_, box_maps_list_, box_buttonsbox_;
	UI::Button map_add_, map_add_dir_, map_delete_;
	UI::Listselect<std::string> dirstruct_, my_maps_;
	AddOns::MapsAddon* selected_;  // Not owned
};

}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_ADDONS_PACKAGER_BOX_H
