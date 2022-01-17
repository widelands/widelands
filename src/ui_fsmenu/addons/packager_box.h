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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_UI_FSMENU_ADDONS_PACKAGER_BOX_H
#define WL_UI_FSMENU_ADDONS_PACKAGER_BOX_H

#include "logic/mutable_addon.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_fsmenu/main.h"

namespace FsMenu {
namespace AddOnsUI {

/** Check whether the filename is valid. Returns the reason why it's invalid, or "" if valid. */
std::string check_addon_filename_validity(const std::string&);
/** Turn a filename into a filename that may be used in an add-on. */
void make_valid_addon_filename(std::string&,
                               const std::map<std::string, std::string>& names_already_in_use);

class AddOnsPackagerBox : public UI::Box {
public:
	AddOnsPackagerBox(MainMenu& mainmenu, Panel* parent, uint32_t orientation);

	void set_header_align(int32_t x) {
		header_align_ = x;
		layout();
	}

	void set_modified_callback(std::function<void()> modified_callback) {
		modified_ = std::move(modified_callback);
	}

	virtual void load_addon(AddOns::MutableAddOn*) {
	}

protected:
	// Used to align addon specific with general UI elements
	int32_t header_align_;
	std::function<void()> modified_;
	MainMenu& main_menu_;
};

class MapsAddOnsPackagerBox : public AddOnsPackagerBox {
public:
	MapsAddOnsPackagerBox(MainMenu& mainmenu, Panel* parent);
	void load_addon(AddOns::MutableAddOn*) override;

protected:
	UI::Box box_dirstruct_;

private:
	enum class ModifyAction { kAddMap, kAddDir, kDeleteMapOrDir };

	void rebuild_dirstruct(AddOns::MapsAddon*, const std::vector<std::string>& select);
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
	std::vector<MainMenu::MapEntry> maps_list_;
	AddOns::AddOnCategory last_category_;

	UI::Box box_maps_list_, box_buttonsbox_, box_dirstruct_displayname_;
	UI::Button map_add_, map_add_dir_, map_delete_;
	UI::Listselect<std::string> dirstruct_, my_maps_;
	UI::EditBox dirstruct_displayname_;
	UI::MultilineTextarea displayname_duplicate_;
	AddOns::MapsAddon* selected_;  // Not owned
};

class CampaignAddOnsPackagerBox : public AddOnsPackagerBox {
public:
	CampaignAddOnsPackagerBox(MainMenu& mainmenu, Panel* parent);
	void load_addon(AddOns::MutableAddOn*) override;
	void layout() override;

private:
	void edited();
	void edited_difficulty_icon();
	void edited_difficulty();
	std::string reverse_icon_lookup(const std::string& value);
	MapsAddOnsPackagerBox maps_box_;
	UI::Box difficulty_hbox_;
	UI::Dropdown<std::string> tribe_select_, icon_difficulty_;
	UI::EditBox difficulty_, short_desc_;
	UI::Textarea difficulty_label_;
	std::string last_difficulty_;
	AddOns::CampaignAddon* selected_;  // Not owned
};

}  // namespace AddOnsUI
}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_ADDONS_PACKAGER_BOX_H
