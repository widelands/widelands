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

#include "ui_fsmenu/addons_packager.h"

#include <memory>

#include "base/i18n.h"
#include "graphic/image_cache.h"
#include "graphic/style_manager.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"
#include "map_io/map_elemental_packet.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/text_prompt.h"
#include "ui_fsmenu/addons.h"
#include "wlapplication.h"

namespace FsMenu {

constexpr int16_t kButtonSize = 32;
constexpr int16_t kSpacing = 4;

AddOnsPackager::AddOnsPackager(MainMenu& parent, AddOnsCtrl& ctrl)
   : UI::Window(&parent,
                UI::WindowStyle::kFsMenu,
                "addons_packager",
                0,
                0,
                parent.calc_desired_window_width(UI::Window::WindowLayoutID::kFsMenuDefault),
                parent.calc_desired_window_height(UI::Window::WindowLayoutID::kFsMenuDefault),
                _("Add-Ons Packager")),
     main_menu_(parent),
     ctrl_(ctrl),
     main_box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     box_left_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_right_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_left_buttons_(&box_left_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     box_right_subbox_header_hbox_(&box_right_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     box_right_subbox_header_box_left_(&box_right_subbox_header_hbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_right_subbox_header_box_right_(&box_right_subbox_header_hbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_right_subbox_maps_dirstruct_hbox_(&box_right_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     box_right_subbox_dirstruct_(&box_right_subbox_maps_dirstruct_hbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_right_subbox_maps_list_(&box_right_subbox_maps_dirstruct_hbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_right_buttonsbox_(&box_right_subbox_maps_dirstruct_hbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_right_bottombox_(&box_right_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     name_(&box_right_subbox_header_box_right_, 0, 0, 100, UI::PanelStyle::kFsMenu),
     author_(&box_right_subbox_header_box_right_, 0, 0, 100, UI::PanelStyle::kFsMenu),
     version_(&box_right_subbox_header_box_right_, 0, 0, 100, UI::PanelStyle::kFsMenu),
     descr_(
        *new UI::MultilineEditbox(&box_right_subbox_header_box_right_, 0, 0, 100, 100, UI::PanelStyle::kFsMenu)),
     addon_new_(&box_left_buttons_,
                "addon_new",
                0,
                0,
                kButtonSize,
                kButtonSize,
                UI::ButtonStyle::kFsMenuSecondary,
                _("+"),
                _("New add-on")),
     addon_delete_(&box_left_buttons_,
                   "addon_delete",
                   0,
                   0,
                   kButtonSize,
                   kButtonSize,
                   UI::ButtonStyle::kFsMenuSecondary,
                   _("–"),
                   _("Delete this add-on")),
     discard_changes_(&box_right_bottombox_,
                      "discard_changes",
                      0,
                      0,
                      kButtonSize,
                      kButtonSize,
                      UI::ButtonStyle::kFsMenuSecondary,
                      _("Discard changes")),
     write_changes_(&box_right_bottombox_,
                    "write_changes",
                    0,
                    0,
                    kButtonSize,
                    kButtonSize,
                    UI::ButtonStyle::kFsMenuSecondary,
                    _("Save changes")),
     ok_(&box_right_bottombox_,
         "ok",
         0,
         0,
         kButtonSize,
         kButtonSize,
         UI::ButtonStyle::kFsMenuPrimary,
         _("OK")),
     map_add_(&box_right_buttonsbox_,
              "map_add",
              0,
              0,
              kButtonSize,
              kButtonSize,
              UI::ButtonStyle::kFsMenuSecondary,
              _("+"),
              _("Add selected map")),
     map_add_dir_(&box_right_buttonsbox_,
                  "map_add_dir",
                  0,
                  0,
                  kButtonSize,
                  kButtonSize,
                  UI::ButtonStyle::kFsMenuSecondary,
                  _("*"),
                  _("Create subdirectory")),
     map_delete_(&box_right_buttonsbox_,
                 "map_delete",
                 0,
                 0,
                 kButtonSize,
                 kButtonSize,
                 UI::ButtonStyle::kFsMenuSecondary,
                 _("–"),
                 _("Remove selected map or directory")),
     addons_(&box_left_, 0, 0, 250, 0, UI::PanelStyle::kFsMenu),
     dirstruct_(&box_right_subbox_dirstruct_, 0, 0, 200, 0, UI::PanelStyle::kFsMenu),
     my_maps_(&box_right_subbox_maps_list_, 0, 0, 100, 0, UI::PanelStyle::kFsMenu),
     update_in_progress_(false) {

	box_left_buttons_.add_inf_space();
	box_left_buttons_.add(&addon_new_);
	box_left_buttons_.add_inf_space();
	box_left_buttons_.add(&addon_delete_);
	box_left_buttons_.add_inf_space();

	box_right_subbox_header_box_right_.add(&name_, UI::Box::Resizing::kFullSize);
	box_right_subbox_header_box_right_.add_space(kSpacing);
	box_right_subbox_header_box_right_.add(&author_, UI::Box::Resizing::kFullSize);
	box_right_subbox_header_box_right_.add_space(kSpacing);
	box_right_subbox_header_box_right_.add(&version_, UI::Box::Resizing::kFullSize);
	box_right_subbox_header_box_right_.add_space(kSpacing);
	box_right_subbox_header_box_right_.add(&descr_, UI::Box::Resizing::kFullSize);

	box_right_subbox_header_box_left_.add_space(kSpacing);
	box_right_subbox_header_box_left_.add(
	   new UI::Textarea(&box_right_subbox_header_box_left_, UI::PanelStyle::kFsMenu,
	                    UI::FontStyle::kFsMenuInfoPanelHeading, _("Name:"), UI::Align::kRight),
	   UI::Box::Resizing::kFullSize);
	box_right_subbox_header_box_left_.add_space(3 * kSpacing);
	box_right_subbox_header_box_left_.add(
	   new UI::Textarea(&box_right_subbox_header_box_left_, UI::PanelStyle::kFsMenu,
	                    UI::FontStyle::kFsMenuInfoPanelHeading, _("Author:"), UI::Align::kRight),
	   UI::Box::Resizing::kFullSize);
	box_right_subbox_header_box_left_.add_space(3 * kSpacing);
	box_right_subbox_header_box_left_.add(
	   new UI::Textarea(&box_right_subbox_header_box_left_, UI::PanelStyle::kFsMenu,
	                    UI::FontStyle::kFsMenuInfoPanelHeading, _("Version:"), UI::Align::kRight),
	   UI::Box::Resizing::kFullSize);
	box_right_subbox_header_box_left_.add_space(3 * kSpacing);
	box_right_subbox_header_box_left_.add(new UI::Textarea(&box_right_subbox_header_box_left_, UI::PanelStyle::kFsMenu,
	                                        UI::FontStyle::kFsMenuInfoPanelHeading,
	                                        _("Description:"), UI::Align::kRight),
	                       UI::Box::Resizing::kFullSize);

	box_right_subbox_header_hbox_.add(&box_right_subbox_header_box_left_, UI::Box::Resizing::kFullSize);
	box_right_subbox_header_hbox_.add_space(kSpacing);
	box_right_subbox_header_hbox_.add(&box_right_subbox_header_box_right_, UI::Box::Resizing::kExpandBoth);

	box_right_buttonsbox_.add_inf_space();
	box_right_buttonsbox_.add(&map_add_);
	box_right_buttonsbox_.add_space(kSpacing);
	box_right_buttonsbox_.add(&map_delete_);
	box_right_buttonsbox_.add_inf_space();
	box_right_buttonsbox_.add(&map_add_dir_);
	box_right_buttonsbox_.add_inf_space();

	box_right_subbox_dirstruct_.add(new UI::Textarea(&box_right_subbox_dirstruct_, UI::PanelStyle::kFsMenu,
	                                        UI::FontStyle::kFsGameSetupHeadings, _("Directory Tree"),
	                                        UI::Align::kCenter),
	                       UI::Box::Resizing::kFullSize);
	box_right_subbox_dirstruct_.add_space(kSpacing);
	box_right_subbox_dirstruct_.add(&dirstruct_, UI::Box::Resizing::kExpandBoth);

	box_right_subbox_maps_list_.add(
	   new UI::Textarea(&box_right_subbox_maps_list_, UI::PanelStyle::kFsMenu,
	                    UI::FontStyle::kFsGameSetupHeadings, _("My Maps"), UI::Align::kCenter),
	   UI::Box::Resizing::kFullSize);
	box_right_subbox_maps_list_.add_space(kSpacing);
	box_right_subbox_maps_list_.add(&my_maps_, UI::Box::Resizing::kExpandBoth);

	box_right_subbox_maps_dirstruct_hbox_.add(&box_right_subbox_dirstruct_, UI::Box::Resizing::kExpandBoth);
	box_right_subbox_maps_dirstruct_hbox_.add_space(kSpacing);
	box_right_subbox_maps_dirstruct_hbox_.add(&box_right_buttonsbox_, UI::Box::Resizing::kFullSize);
	box_right_subbox_maps_dirstruct_hbox_.add_space(kSpacing);
	box_right_subbox_maps_dirstruct_hbox_.add(&box_right_subbox_maps_list_, UI::Box::Resizing::kExpandBoth);

	box_right_bottombox_.add(&discard_changes_, UI::Box::Resizing::kExpandBoth);
	box_right_bottombox_.add_space(kSpacing);
	box_right_bottombox_.add(&write_changes_, UI::Box::Resizing::kExpandBoth);
	box_right_bottombox_.add_space(kSpacing);
	box_right_bottombox_.add(&ok_, UI::Box::Resizing::kExpandBoth);

	box_right_.add(&box_right_subbox_header_hbox_, UI::Box::Resizing::kFullSize);
	box_right_.add_space(kButtonSize);
	box_right_.add(&box_right_subbox_maps_dirstruct_hbox_, UI::Box::Resizing::kExpandBoth);
	box_right_.add_space(kSpacing);
	box_right_.add(&box_right_bottombox_, UI::Box::Resizing::kFullSize);

	box_left_.add(&addons_, UI::Box::Resizing::kExpandBoth);
	box_left_.add_space(kSpacing);
	box_left_.add(&box_left_buttons_, UI::Box::Resizing::kFullSize);

	main_box_.add(&box_left_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(kSpacing);
	main_box_.add(&box_right_, UI::Box::Resizing::kExpandBoth);

	{
		std::vector<MainMenu::MapEntry> v;
		MainMenu::find_maps("maps/My_Maps", v);
		for (const MainMenu::MapEntry& entry : v) {
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

	map_add_.set_enabled(false);
	my_maps_.selected.connect([this](uint32_t) { map_add_.set_enabled(true); });
	map_delete_.set_enabled(false);
	dirstruct_.selected.connect([this](uint32_t i) { map_delete_.set_enabled(i > 0); });
	addons_.selected.connect([this](uint32_t) { addon_selected(); });
	addon_new_.sigclicked.connect([this]() { clicked_new_addon(); });
	addon_delete_.sigclicked.connect([this]() { clicked_delete_addon(); });
	map_add_.sigclicked.connect(
	   [this]() { clicked_add_or_delete_map_or_dir(ModifyAction::kAddMap); });
	map_add_dir_.sigclicked.connect(
	   [this]() { clicked_add_or_delete_map_or_dir(ModifyAction::kAddDir); });
	map_delete_.sigclicked.connect(
	   [this]() { clicked_add_or_delete_map_or_dir(ModifyAction::kDeleteMapOrDir); });
	discard_changes_.sigclicked.connect([this]() { clicked_discard_changes(); });
	write_changes_.sigclicked.connect([this]() { clicked_write_changes(); });
	ok_.sigclicked.connect([this]() { die(); });

	name_.changed.connect([this]() { current_addon_edited(); });
	author_.changed.connect([this]() { current_addon_edited(); });
	version_.changed.connect([this]() { current_addon_edited(); });
	descr_.changed.connect([this]() { current_addon_edited(); });

	initialize_mutable_addons();

	layout();
	center_to_parent();
}

bool AddOnsPackager::handle_key(const bool down, const SDL_Keysym code) {
	if (down && (code.sym == SDLK_KP_ENTER || code.sym == SDLK_RETURN)) {
		die();
		return true;
	}
	return Window::handle_key(down, code);
}

void AddOnsPackager::layout() {
	UI::Window::layout();
	if (!is_minimal()) {
		main_box_.set_size(get_inner_w(), get_inner_h());
	}
}

void AddOnsPackager::initialize_mutable_addons() {
	mutable_addons_.clear();
	addons_with_changes_.clear();

	for (const AddOns::AddOnState& a : AddOns::g_addons) {
		MutableAddOn m{a.first.internal_name,
		               a.first.unlocalized_descname,
		               a.first.unlocalized_description,
		               a.first.unlocalized_author,
		               AddOns::version_to_string(a.first.version, false),
		               a.first.category,
		               {{}, {}}};
		if (a.first.category == AddOns::AddOnCategory::kMaps) {
			recursively_initialize_tree_from_disk(
			   kAddOnDir + FileSystem::file_separator() + a.first.internal_name, m.tree);
		}
		mutable_addons_[a.first.internal_name] = m;
	}

	rebuild_addon_list("");
	check_for_unsaved_changes();
}

void AddOnsPackager::recursively_initialize_tree_from_disk(const std::string& dir,
                                                           MutableAddOn::DirectoryTree& tree) {
	for (const std::string& file : g_fs->list_directory(dir)) {
		if (FileSystem::filename_ext(file) == kWidelandsMapExtension) {
			tree.maps[FileSystem::fs_filename(file.c_str())] = file;
		} else if (g_fs->is_directory(file)) {
			MutableAddOn::DirectoryTree child;
			recursively_initialize_tree_from_disk(file, child);
			tree.subdirectories[FileSystem::fs_filename(file.c_str())] = child;
		}
	}
}

void AddOnsPackager::rebuild_addon_list(const std::string& select) {
	addons_.clear();
	for (const auto& pair : mutable_addons_) {
		addons_.add(pair.first, pair.first,
		            g_image_cache->get(AddOns::kAddOnCategories.at(pair.second.category).icon),
		            pair.first == select);
	}
	addon_selected();
}

MutableAddOn* AddOnsPackager::get_selected() {
	if (!addons_.has_selection()) {
		return nullptr;
	}
	for (auto& pair : mutable_addons_) {
		if (pair.first == addons_.get_selected()) {
			return &pair.second;
		}
	}
	NEVER_HERE();
}

void AddOnsPackager::addon_selected() {
	MutableAddOn* selected = get_selected();

	addon_delete_.set_enabled(selected);
	box_right_subbox_header_hbox_.set_visible(false);
	box_right_subbox_maps_dirstruct_hbox_.set_visible(false);

	if (!selected) {
		return;
	}

	box_right_subbox_header_hbox_.set_visible(true);

	update_in_progress_ = true;
	name_.set_text(selected->descname);
	descr_.set_text(selected->description);
	author_.set_text(selected->author);
	version_.set_text(selected->version);
	update_in_progress_ = false;

	if (selected->category != AddOns::AddOnCategory::kMaps) {
		return;
	}

	box_right_subbox_maps_dirstruct_hbox_.set_visible(true);
	rebuild_dirstruct(*selected);
}

void AddOnsPackager::rebuild_dirstruct(MutableAddOn& a, const std::vector<std::string>& select) {
	const std::string path = kAddOnDir + FileSystem::file_separator() + a.internal_name;

	dirstruct_.clear();
	dirstruct_to_tree_map_.clear();

	std::vector<std::string> toplevel_entry;
	dirstruct_to_tree_map_.push_back(toplevel_entry);

	dirstruct_.add(a.internal_name, path, g_image_cache->get("images/ui_basic/ls_wlscenario.png"),
	               toplevel_entry == select, _("Top-level directory"));
	do_recursively_rebuild_dirstruct(a.tree, 1, path, toplevel_entry, select);
}

void AddOnsPackager::do_recursively_rebuild_dirstruct(const MutableAddOn::DirectoryTree& tree,
                                                      const unsigned level,
                                                      const std::string& path,
                                                      const std::vector<std::string>& map_path,
                                                      const std::vector<std::string>& select) {
	for (const auto& pair : tree.subdirectories) {
		std::vector<std::string> entry = map_path;
		entry.push_back(pair.first);
		dirstruct_to_tree_map_.push_back(entry);

		const std::string subdir = path + FileSystem::file_separator() + pair.first;
		dirstruct_.add(pair.first, subdir, g_image_cache->get("images/ui_basic/ls_dir.png"),
		               entry == select, pair.first, "", level);

		do_recursively_rebuild_dirstruct(pair.second, level + 1, subdir, entry, select);
	}

	for (const auto& pair : tree.maps) {
		std::vector<std::string> entry = map_path;
		entry.push_back(pair.first);
		dirstruct_to_tree_map_.push_back(entry);

		dirstruct_.add(pair.first, path + FileSystem::file_separator() + pair.first,
		               g_image_cache->get("images/ui_basic/ls_wlmap.png"), entry == select,
		               pair.first, "", level);
	}
}

void AddOnsPackager::current_addon_edited() {
	if (update_in_progress_) {
		return;
	}

	const std::string& sel = addons_.get_selected();
	MutableAddOn& m = mutable_addons_.at(sel);

	m.descname = name_.text();
	m.author = author_.text();
	m.version = version_.text();
	m.description = descr_.get_text();

	addons_with_changes_[sel] = false;
	check_for_unsaved_changes();
}

void AddOnsPackager::clicked_new_addon() {
	UI::TextPrompt n(main_menu_, UI::WindowStyle::kFsMenu, _("New Add-On"), _("Enter the name for the new add-on."));
	UI::Dropdown<AddOns::AddOnCategory> category(&n.content_box(),
	               "category",
	               0,
	               0,
	               50,
	               12,
	               kButtonSize,
	               _("Category"),
	               UI::DropdownType::kTextual,
	               UI::PanelStyle::kFsMenu,
	               UI::ButtonStyle::kFsMenuSecondary);
	for (const auto& pair : AddOns::kAddOnCategories) {
		if (pair.first != AddOns::AddOnCategory::kNone) {
			category.add(pair.second.descname(), pair.first, g_image_cache->get(pair.second.icon),
			              pair.first == AddOns::AddOnCategory::kMaps);
		}
	}
	n.content_box().add(&category, UI::Box::Resizing::kFullSize);

	for (;;) {
		if (n.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
			return;
		}

		std::string name = n.text();

		if (name.empty() || !FileSystem::is_legal_filename(name)) {
			main_menu_.show_messagebox(
			   _("Invalid Name"), _("This name is invalid, please choose a different name."));
			continue;
		}

		if (name.size() <= kAddOnExtension.size() ||
		    name.compare(
		       name.size() - kAddOnExtension.size(), kAddOnExtension.size(), kAddOnExtension) != 0) {
			// Ensure add-on names always end with '.wad'
			name += kAddOnExtension;
			// The name was legal before, so it should be so still
			assert(FileSystem::is_legal_filename(name));
		}

		if (mutable_addons_.find(name) != mutable_addons_.end()) {
			main_menu_.show_messagebox(
			   _("Invalid Name"), _("An add-on with this internal name already exists."));
			continue;
		}
		if (addons_with_changes_.find(name) != addons_with_changes_.end()) {
			main_menu_.show_messagebox(
			   _("Invalid Name"), _("An add-on with this internal name was deleted recently. Please "
			                        "commit your changes before using this name again."));
			continue;
		}

		mutable_addons_[name] = MutableAddOn{
		   // These default strings are not localized because these editboxes are meant to be
		   // filled out in English. We will add localization markup to the resulting config file.
		   name, n.text(), "No description", "Nobody", "1.0.0", category.get_selected(), {{}, {}}};
		addons_with_changes_[name] = false;
		check_for_unsaved_changes();
		rebuild_addon_list(name);
		return;
	}
}

void AddOnsPackager::clicked_delete_addon() {
	const std::string& name = addons_.get_selected();

	bool is_remote = false;
	for (const AddOns::AddOnInfo& r : ctrl_.get_remotes()) {
		if (r.internal_name == name) {
			is_remote = true;
			break;
		}
	}

	UI::WLMessageBox m(
	   get_parent(), UI::WindowStyle::kFsMenu, _("Delete Add-on"),
	   (boost::format(
	   	is_remote ? _("Do you really want to delete the add-on ‘%s’?")
	   	: _("Do you really want to delete the local add-on ‘%s’?\n\nNote that this add-on can not be downloaded again from the server.")
	   ) % name).str(),
	   UI::WLMessageBox::MBoxType::kOkCancel);
	if (m.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
		return;
	}

	auto it = mutable_addons_.find(name);
	assert(it != mutable_addons_.end());
	addons_with_changes_[it->first] = true;
	mutable_addons_.erase(it);
	check_for_unsaved_changes();
	rebuild_addon_list("");
}

void AddOnsPackager::clicked_add_or_delete_map_or_dir(const ModifyAction action) {
	MutableAddOn* m = get_selected();
	assert(m);
	assert(m->category == AddOns::AddOnCategory::kMaps);

	MutableAddOn::DirectoryTree* tree = &m->tree;
	MutableAddOn::DirectoryTree* tree_parent = nullptr;
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
	} break;
	case ModifyAction::kAddDir: {
		UI::TextPrompt n(main_menu_, UI::WindowStyle::kFsMenu, _("New Directory"), _("Enter the name for the new directory."));
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

			tree->subdirectories[name] = MutableAddOn::DirectoryTree();
			select.push_back(name);
			break;
		}
	} break;
	case ModifyAction::kDeleteMapOrDir: {
		assert(!selected_map.empty() || !select.empty());

		UI::WLMessageBox mbox(
		   get_parent(), UI::WindowStyle::kFsMenu, _("Delete"),
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
	} break;
	}

	addons_with_changes_[m->internal_name] = false;
	check_for_unsaved_changes();
	rebuild_dirstruct(*m, select);
}

void AddOnsPackager::die() {
	if (!addons_with_changes_.empty()) {
		std::string msg = (boost::format(ngettext(
		                      // Comments to fix codecheck false-positive
		                      "If you quit the packager now, all changes to the following %u add-on "
		                      "will be discarded.",
		                      // Comments to fix codecheck false-positive
		                      "If you quit the packager now, all changes to the following %u add-ons "
		                      "will be discarded.",
		                      // Comments to fix codecheck false-positive
		                      addons_with_changes_.size())) %
		                   addons_with_changes_.size())
		                     .str();
		for (const auto& str : addons_with_changes_) {
			msg = (boost::format(_("%1$s\n· %2$s")) % msg % str.first).str();
		}

		UI::WLMessageBox m(get_parent(), UI::WindowStyle::kFsMenu, _("Quit Packager"), msg,
		                   UI::WLMessageBox::MBoxType::kOkCancel, UI::Align::kLeft);
		if (m.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
			return;
		}
	}

	UI::Window::die();
}

void AddOnsPackager::clicked_discard_changes() {
	assert(!addons_with_changes_.empty());

	std::string msg = (boost::format(ngettext(
	                      "Do you really want to discard all changes to the following %u add-on?",
	                      "Do you really want to discard all changes to the following %u add-ons?",
	                      addons_with_changes_.size())) %
	                   addons_with_changes_.size())
	                     .str();
	for (const auto& str : addons_with_changes_) {
		msg = (boost::format(_("%1$s\n· %2$s")) % msg % str.first).str();
	}

	UI::WLMessageBox m(get_parent(), UI::WindowStyle::kFsMenu, _("Discard Changes"), msg,
	                   UI::WLMessageBox::MBoxType::kOkCancel, UI::Align::kLeft);
	if (m.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
		initialize_mutable_addons();
	}
}

void AddOnsPackager::clicked_write_changes() {
	assert(!addons_with_changes_.empty());

	std::string msg =
	   (boost::format(
	       ngettext("Do you really want to commit all changes to the following %u add-on to disk?",
	                "Do you really want to commit all changes to the following %u add-ons to disk?",
	                addons_with_changes_.size())) %
	    addons_with_changes_.size())
	      .str();
	for (const auto& str : addons_with_changes_) {
		msg = (boost::format(_("%1$s\n· %2$s")) % msg % str.first).str();
	}

	UI::WLMessageBox m(get_parent(), UI::WindowStyle::kFsMenu, _("Confirm Saving"), msg,
	                   UI::WLMessageBox::MBoxType::kOkCancel, UI::Align::kLeft);
	if (m.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
		std::set<std::string> errors;
		for (const auto& pair : addons_with_changes_) {
			if (pair.second) {
				// Delete existing add-on
				const std::string directory = kAddOnDir + FileSystem::file_separator() + pair.first;
				if (g_fs->file_exists(directory)) {
					g_fs->fs_unlink(directory);
				}
			} else {
				// Write add-on
				if (!do_write_addon_to_disk(pair.first)) {
					errors.insert(pair.first);
				}
			}
		}

		// Clear list of changes, and re-insert failed add-ons
		addons_with_changes_.clear();
		for (const std::string& e : errors) {
			addons_with_changes_[e] = false;
		}
		check_for_unsaved_changes();

		// Update the global catalogue
		WLApplication::initialize_g_addons();
	}
}

static void parse_map_requirements(const MutableAddOn::DirectoryTree& tree,
                                   std::vector<std::string>& req) {
	for (const auto& pair : tree.subdirectories) {
		parse_map_requirements(pair.second, req);
	}

	// Place to save temp data
	Widelands::Map map;
	Widelands::MapElementalPacket packet;

	for (const auto& pair : tree.maps) {
		std::unique_ptr<FileSystem> fs(g_fs->make_sub_file_system(pair.second));
		assert(fs);

		packet.pre_read(*fs, &map);

		for (const auto& r : map.required_addons()) {
			if (std::find(req.begin(), req.end(), r.first) == req.end()) {
				req.push_back(r.first);
			}
		}
	}
}

static void do_recursively_copy_file_or_directory(const std::string& source,
                                                  const std::string& dest) {
	if (g_fs->is_directory(source)) {
		g_fs->ensure_directory_exists(dest);
		for (const std::string& file : g_fs->list_directory(source)) {
			do_recursively_copy_file_or_directory(
			   file, dest + FileSystem::file_separator() + FileSystem::fs_filename(file.c_str()));
		}
	} else {
		FileRead fr;
		fr.open(*g_fs, source);
		const size_t bytes = fr.get_size();
		std::unique_ptr<char[]> data(new char[bytes]);
		fr.data_complete(data.get(), bytes);

		FileWrite fw;
		fw.data(data.get(), bytes);
		fw.write(*g_fs, dest);
	}
}

static void do_recursively_create_filesystem_structure(const std::string& dir,
                                                       const MutableAddOn::DirectoryTree& tree,
                                                       const std::string& addon_basedir,
                                                       const std::string& backup_basedir) {
	// Dirs
	for (const auto& pair : tree.subdirectories) {
		const std::string subdir = dir + FileSystem::file_separator() + pair.first;
		g_fs->ensure_directory_exists(subdir);
		do_recursively_create_filesystem_structure(
		   subdir, pair.second, addon_basedir, backup_basedir);
	}

	// Maps
	for (const auto& pair : tree.maps) {
		// If the filepath starts with the add-on's path, we need to use the backup instead
		std::string source_path = pair.second;
		if (source_path.size() >= addon_basedir.size() &&
		    source_path.compare(0, addon_basedir.size(), addon_basedir) == 0) {
			assert(!backup_basedir.empty());
			std::string temp = source_path.substr(addon_basedir.size());
			source_path = backup_basedir;
			source_path += temp;
		}
		assert(source_path.size() > kWidelandsMapExtension.size());
		assert(source_path.compare(source_path.size() - kWidelandsMapExtension.size(),
		                           kWidelandsMapExtension.size(), kWidelandsMapExtension) == 0);

		do_recursively_copy_file_or_directory(
		   source_path, dir + FileSystem::file_separator() + pair.first);
	}
}

bool AddOnsPackager::do_write_addon_to_disk(const std::string& addon) {
	MutableAddOn& m = mutable_addons_.at(addon);

	// Check that the version string is valid and beautify it
	try {
		AddOns::AddOnVersion v = AddOns::string_to_version(m.version);
		m.version = AddOns::version_to_string(v, false);
		if (addons_.has_selection() && addons_.get_selected() == addon) {
			version_.set_text(m.version);
		}
	} catch (...) {
		main_menu_.show_messagebox(
		   _("Invalid Version"),
		   (boost::format(
		       _("‘%1$s’ is not a valid version string. The add-on ‘%2$s’ will not be saved.")) %
		    m.version % addon)
		      .str());
		return false;
	}

	const bool is_map = m.category == AddOns::AddOnCategory::kMaps;
	const std::string directory = kAddOnDir + FileSystem::file_separator() + m.internal_name;
	bool dir_exists = g_fs->file_exists(directory);
	const std::string profile_path = directory + FileSystem::file_separator() + kAddOnMainFile;

	// Step 1: Gather the requirements of all contained maps
	std::string requires;
	if (is_map) {
		std::vector<std::string> req;
		try {
			parse_map_requirements(m.tree, req);
		} catch (const WException& e) {
			main_menu_.show_messagebox(
			   _("Invalid Map File"),
			   (boost::format(_("The add-on ‘%1$s’ can not be saved because a map file is "
			                    "invalid.\n\nError message:\n%2$s")) %
			    addon % e.what())
			      .str());
			return false;
		}
		if (const size_t nr = req.size()) {
			requires = req[0];
			for (size_t i = 1; i < nr; ++i) {
				requires += ',';
				requires += req[i];
			}
		}
	}

	// Step 2: If the add-on exists on disk already and our add-on is of type Map Set,
	// make a backup copy of the whole original add-on in ~/.widelands/temp, then
	// delete the original add-on directory and create it anew.
	std::string backup_path;
	if (is_map && dir_exists) {
		backup_path =
		   kTempFileDir + FileSystem::file_separator() + m.internal_name + kTempFileExtension;
		if (g_fs->file_exists(backup_path)) {
			g_fs->fs_unlink(backup_path);
		}
		g_fs->fs_rename(directory, backup_path);
		dir_exists = false;
	}

	// Step 3: Create the `addon` file.
	// · If our add-on is not a map set, we need to read the original `addon` file (if it
	//   exists) to determine the requirements. Then write the file, and we are done.
	// · For maps, we need to gather the information regarding the maps' required add-ons
	//   before writing the profile so we can generate the correct `requires` string.
	g_fs->ensure_directory_exists(directory);
	// Write profile
	{
		Profile p;
		if (dir_exists && !is_map) {
			p.read(profile_path.c_str());
			requires = p.get_safe_section("global").get_safe_string("requires");
		}
		Section& s = p.create_section("global");

		s.set_translated_string("name", m.descname);
		s.set_translated_string("description", m.description);
		s.set_translated_string("author", m.author);
		s.set_string("version", m.version);
		s.set_string("category", AddOns::kAddOnCategories.at(m.category).internal_name);
		s.set_string("requires", requires);

		p.write(profile_path.c_str(), false);
	}

	if (!is_map) {
		return true;
	}

	// Step 4: Create the directory structure and copy the maps
	do_recursively_create_filesystem_structure(directory, m.tree, directory, backup_path);

	// Step 5: Delete the backup (if it existed)
	if (!backup_path.empty()) {
		g_fs->fs_unlink(backup_path);
	}

	return true;
}

}  // namespace FsMenu
