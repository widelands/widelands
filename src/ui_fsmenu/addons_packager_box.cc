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

#include "ui_fsmenu/addons_packager_box.h"

#include "base/i18n.h"
#include "graphic/image_cache.h"
#include "graphic/style_manager.h"
#include "logic/filesystem_constants.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/text_prompt.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/main.h"

namespace FsMenu {

constexpr int16_t kButtonSize = 32;
constexpr int16_t kSpacing = 4;

AddOnsPackagerBox::AddOnsPackagerBox(MainMenu& mainmenu, Panel* parent, uint32_t orientation)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, orientation), main_menu_(mainmenu) {
}

MapsAddOnsPackagerBox::MapsAddOnsPackagerBox(MainMenu& mainmenu, Panel* parent)
   : AddOnsPackagerBox(mainmenu, parent, UI::Box::Horizontal),
     box_dirstruct_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     last_category_(AddOns::AddOnCategory::kNone),
     box_maps_list_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_buttonsbox_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     map_add_(&box_buttonsbox_,
              "map_add",
              0,
              0,
              kButtonSize,
              kButtonSize,
              UI::ButtonStyle::kFsMenuSecondary,
              _("+"),
              _("Add selected map")),
     map_add_dir_(&box_buttonsbox_,
                  "map_add_dir",
                  0,
                  0,
                  kButtonSize,
                  kButtonSize,
                  UI::ButtonStyle::kFsMenuSecondary,
                  _("*"),
                  _("Create subdirectory")),
     map_delete_(&box_buttonsbox_,
                 "map_delete",
                 0,
                 0,
                 kButtonSize,
                 kButtonSize,
                 UI::ButtonStyle::kFsMenuSecondary,
                 _("–"),
                 _("Remove selected map or directory")),
     dirstruct_(&box_dirstruct_, 0, 0, 200, 0, UI::PanelStyle::kFsMenu),
     my_maps_(&box_maps_list_, 0, 0, 100, 0, UI::PanelStyle::kFsMenu) {
	box_buttonsbox_.add_inf_space();
	box_buttonsbox_.add(&map_add_);
	box_buttonsbox_.add_space(kSpacing);
	box_buttonsbox_.add(&map_delete_);
	box_buttonsbox_.add_inf_space();
	box_buttonsbox_.add(&map_add_dir_);
	box_buttonsbox_.add_inf_space();

	box_dirstruct_.add(new UI::Textarea(&box_dirstruct_, UI::PanelStyle::kFsMenu,
	                                    UI::FontStyle::kFsGameSetupHeadings, _("Directory Tree"),
	                                    UI::Align::kCenter),
	                   UI::Box::Resizing::kFullSize);
	box_dirstruct_.add_space(kSpacing);
	box_dirstruct_.add(&dirstruct_, UI::Box::Resizing::kExpandBoth);

	box_maps_list_.add(
	   new UI::Textarea(&box_maps_list_, UI::PanelStyle::kFsMenu,
	                    UI::FontStyle::kFsGameSetupHeadings, _("My Maps"), UI::Align::kCenter),
	   UI::Box::Resizing::kFullSize);
	box_maps_list_.add_space(kSpacing);
	box_maps_list_.add(&my_maps_, UI::Box::Resizing::kExpandBoth);

	add(&box_dirstruct_, UI::Box::Resizing::kExpandBoth);
	add_space(kSpacing);
	add(&box_buttonsbox_, UI::Box::Resizing::kFullSize);
	add_space(kSpacing);
	add(&box_maps_list_, UI::Box::Resizing::kExpandBoth);

	MainMenu::find_maps("maps/My_Maps", maps_list_);

	map_add_.set_enabled(false);
	my_maps_.selected.connect([this](uint32_t) { map_add_.set_enabled(true); });
	map_delete_.set_enabled(false);
	dirstruct_.selected.connect([this](uint32_t i) { map_delete_.set_enabled(i > 0); });

	map_add_.sigclicked.connect(
	   [this]() { clicked_add_or_delete_map_or_dir(ModifyAction::kAddMap); });
	map_add_dir_.sigclicked.connect(
	   [this]() { clicked_add_or_delete_map_or_dir(ModifyAction::kAddDir); });
	map_delete_.sigclicked.connect(
	   [this]() { clicked_add_or_delete_map_or_dir(ModifyAction::kDeleteMapOrDir); });
}

void MapsAddOnsPackagerBox::load_addon(AddOns::MutableAddOn* a) {
	assert(a->get_category() == AddOns::AddOnCategory::kMaps ||
	       a->get_category() == AddOns::AddOnCategory::kCampaign);
	if (a->get_category() != last_category_) {
		last_category_ = a->get_category();
		my_maps_.clear();
		for (const MainMenu::MapEntry& entry : maps_list_) {
			if (entry.first.maptype == MapData::MapType::kNormal &&
			    last_category_ == AddOns::AddOnCategory::kCampaign) {
				// Only include scenarios for campaigns
				continue;
			}
			my_maps_.add(
			   entry.first.localized_name, entry.first.filename, nullptr, false,
			   (boost::format("%s<br>%s<br>%s<br>%s<br>%s") %
			    g_style_manager->font_style(UI::FontStyle::kFsTooltipHeader)
			       .as_font_tag(entry.first.filename) %
			    (boost::format(_("Name: %s")) %
			     g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			        .as_font_tag(entry.first.localized_name))
			       .str() %
			    (boost::format(_("Size: %s")) %
			     g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			        .as_font_tag(
			           (boost::format(_("%1$u×%2$u")) % entry.first.width % entry.first.height).str()))
			       .str() %
			    (boost::format(_("Players: %s")) %
			     g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			        .as_font_tag(std::to_string(entry.first.nrplayers)))
			       .str() %
			    (boost::format(_("Description: %s")) %
			     g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			        .as_font_tag(entry.first.description))
			       .str())
			      .str());
		}
	}
	rebuild_dirstruct(dynamic_cast<AddOns::MapsAddon*>(a));
}

void MapsAddOnsPackagerBox::rebuild_dirstruct(AddOns::MapsAddon* a,
                                              const std::vector<std::string>& select) {
	const std::string path = kAddOnDir + FileSystem::file_separator() + a->get_internal_name();

	selected_ = a;

	dirstruct_.clear();
	dirstruct_to_tree_map_.clear();

	std::vector<std::string> toplevel_entry;
	dirstruct_to_tree_map_.push_back(toplevel_entry);

	dirstruct_.add(a->get_internal_name(), path,
	               g_image_cache->get("images/ui_basic/ls_wlscenario.png"), toplevel_entry == select,
	               _("Top-level directory"));
	do_recursively_rebuild_dirstruct(a->get_tree(), 1, path, toplevel_entry, select);
}

void MapsAddOnsPackagerBox::do_recursively_rebuild_dirstruct(
   const AddOns::MapsAddon::DirectoryTree* tree,
   const unsigned level,
   const std::string& path,
   const std::vector<std::string>& map_path,
   const std::vector<std::string>& select) {
	for (const auto& pair : tree->subdirectories) {
		std::vector<std::string> entry = map_path;
		entry.push_back(pair.first);
		dirstruct_to_tree_map_.push_back(entry);

		const std::string subdir = path + FileSystem::file_separator() + pair.first;
		dirstruct_.add(pair.first, subdir, g_image_cache->get("images/ui_basic/ls_dir.png"),
		               entry == select, pair.first, "", level);

		do_recursively_rebuild_dirstruct(&pair.second, level + 1, subdir, entry, select);
	}

	for (const auto& pair : tree->maps) {
		std::vector<std::string> entry = map_path;
		entry.push_back(pair.first);
		dirstruct_to_tree_map_.push_back(entry);

		dirstruct_.add(pair.first, path + FileSystem::file_separator() + pair.first,
		               g_image_cache->get("images/ui_basic/ls_wlmap.png"), entry == select,
		               pair.first, "", level);
	}
}

void MapsAddOnsPackagerBox::clicked_add_or_delete_map_or_dir(const ModifyAction action) {
	assert(selected_);

	AddOns::MapsAddon::DirectoryTree* tree = selected_->get_tree();
	AddOns::MapsAddon::DirectoryTree* tree_parent = nullptr;
	std::vector<std::string> select = dirstruct_to_tree_map_[dirstruct_.selection_index()];

	std::string selected_map;
	if (!select.empty() && select.back().size() >= kWidelandsMapExtension.size() &&
	    select.back().compare(select.back().size() - kWidelandsMapExtension.size(),
	                          kWidelandsMapExtension.size(), kWidelandsMapExtension) == 0) {
		// Last entry is a map – pop it, we care only about directories here
		selected_map = select.back();
		select.pop_back();
	}
	for (const std::string& s : select) {
		tree_parent = tree;
		tree = &tree->subdirectories.at(s);
	}

	switch (action) {
	case ModifyAction::kAddMap: {
		const std::string& map = my_maps_.get_selected();
		const std::string filename = FileSystem::fs_filename(map.c_str());
		tree->maps[filename] = map;
		select.push_back(filename);
		break;
	}
	case ModifyAction::kAddDir: {
		UI::TextPrompt n(main_menu_, UI::WindowStyle::kFsMenu, _("New Directory"),
		                 _("Enter the name for the new directory."));
		for (;;) {
			if (n.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
				return;
			}

			std::string name = n.text();

			if (name.empty() || !FileSystem::is_legal_filename(name) ||
			    (name.size() >= kWidelandsMapExtension.size() &&
			     name.compare(name.size() - kWidelandsMapExtension.size(),
			                  kWidelandsMapExtension.size(), kWidelandsMapExtension) == 0)) {
				main_menu_.show_messagebox(
				   _("Invalid Name"), _("This name is invalid, please choose a different name."));
				continue;
			}

			if (tree->subdirectories.find(name) != tree->subdirectories.end()) {
				main_menu_.show_messagebox(
				   _("Invalid Name"), _("A directory with this name already exists."));
				continue;
			}

			tree->subdirectories[name] = AddOns::MapsAddon::DirectoryTree();
			select.push_back(name);
			break;
		}
		break;
	}
	case ModifyAction::kDeleteMapOrDir: {
		assert(!selected_map.empty() || !select.empty());

		UI::WLMessageBox mbox(
		   &main_menu_, UI::WindowStyle::kFsMenu, _("Delete"),
		   selected_map.empty() ?
		      (boost::format(
		          _("Do you really want to delete the directory ‘%s’ and all its contents?")) %
		       select.back())
		         .str() :
		      (boost::format(_("Do you really want to delete the map ‘%s’?")) % selected_map).str(),
		   UI::WLMessageBox::MBoxType::kOkCancel, UI::Align::kLeft);
		if (mbox.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
			return;
		}

		if (selected_map.empty()) {
			assert(tree_parent);
			auto it = tree_parent->subdirectories.find(select.back());
			assert(it != tree_parent->subdirectories.end());
			tree_parent->subdirectories.erase(it);
		} else {
			auto it = tree->maps.find(selected_map);
			assert(it != tree->maps.end());
			tree->maps.erase(it);
		}
		break;
	}
	}

	modified_();
	rebuild_dirstruct(selected_, select);
}

CampaignAddOnsPackagerBox::CampaignAddOnsPackagerBox(MainMenu& mainmenu, Panel* parent)
   : MapsAddOnsPackagerBox(mainmenu, parent),
     tribe_select_(&box_dirstruct_,
                   "dropdown_tribe",
                   0,
                   0,
                   50,
                   8,
                   kButtonSize,
                   _("Tribe"),
                   UI::DropdownType::kTextual,
                   UI::PanelStyle::kWui,
                   UI::ButtonStyle::kWuiSecondary) {
	std::vector<Widelands::TribeBasicInfo> tribeinfos = Widelands::get_all_tribeinfos();
	for (const Widelands::TribeBasicInfo& tribeinfo : tribeinfos) {
		tribe_select_.add(tribeinfo.descname, tribeinfo.name, g_image_cache->get(tribeinfo.icon),
		                  false, tribeinfo.tooltip);
	}
	tribe_select_.select(tribeinfos.front().name);
	tribe_select_.selected.connect([this]() {
		selected_->set_tribe(tribe_select_.get_selected());
		modified_();
	});

	box_dirstruct_.add(&tribe_select_, UI::Box::Resizing::kFullSize);
}

void CampaignAddOnsPackagerBox::load_addon(AddOns::MutableAddOn* a) {
	assert(a->get_category() == AddOns::AddOnCategory::kCampaign);
	selected_ = dynamic_cast<AddOns::CampaignAddon*>(a);
	// Only allow tribe configuration during first setup
	tribe_select_.set_visible(!selected_->luafile_exists());

	MapsAddOnsPackagerBox::load_addon(a);
}

}  // namespace FsMenu
