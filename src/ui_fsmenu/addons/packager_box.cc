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

#include "ui_fsmenu/addons/packager_box.h"

#include "base/i18n.h"
#include "graphic/image_cache.h"
#include "graphic/style_manager.h"
#include "io/filesystem/illegal_filename_check.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/text_prompt.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/main.h"

namespace FsMenu {
namespace AddOnsUI {

constexpr int16_t kButtonSize = 32;
constexpr int16_t kSpacing = 4;

/* All restrictions on add-on filenames are imposed by the server. */
static const std::string kValidAddOnFilenameChars = "abcdefghijklmnopqrstuvwxyz"
                                                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                    "0123456789._-";
constexpr uint16_t kAddOnFileNameLengthLimit = 80;
static const std::string kInvalidAddOnFilenameSequences[] = {".."};

namespace {
inline bool selection_is_map(const std::vector<std::string>& select) {
	return !select.empty() && select.back().size() >= kWidelandsMapExtension.size() &&
	       select.back().compare(select.back().size() - kWidelandsMapExtension.size(),
	                             kWidelandsMapExtension.size(), kWidelandsMapExtension) == 0;
}
}  // namespace

std::string check_addon_filename_validity(const std::string& name) {
	if (name.empty() || !FileSystemHelper::is_legal_filename(name)) {
		return _("Invalid filename");
	}
	if (name.size() + kAddOnExtension.size() > kAddOnFileNameLengthLimit) {
		return _("Name too long");
	}

	size_t pos = name.find_first_not_of(kValidAddOnFilenameChars);
	if (pos != std::string::npos) {
		return format(_("Invalid character ‘%c’"), name.at(pos));
	}

	for (const std::string& q : kInvalidAddOnFilenameSequences) {
		if (name.find(q) != std::string::npos) {
			return format(_("Filename may not contain ‘%s’"), q);
		}
	}

	return std::string();
}

void make_valid_addon_filename(std::string& name,
                               const std::map<std::string, std::string>& names_already_in_use) {
	if (name.empty()) {
		name = "unnamed";
	}

	for (;;) {
		const size_t pos = name.find_first_not_of(kValidAddOnFilenameChars);
		if (pos == std::string::npos) {
			break;
		}
		name.at(pos) = '_';
	}

	for (const std::string& q : kInvalidAddOnFilenameSequences) {
		for (;;) {
			const size_t pos = name.find(q);
			if (pos == std::string::npos) {
				break;
			}
			for (size_t n = q.size(); n > 0; --n) {
				name.at(pos + n - 1) = '_';
			}
		}
	}

	size_t len = name.size();
	if (len > kAddOnFileNameLengthLimit) {
		name.erase(0, len - kAddOnFileNameLengthLimit);
	}

	if (names_already_in_use.count(name)) {
		for (int i = 1;; ++i) {
			std::string prefix = std::to_string(i);
			prefix += "_";
			std::string new_name = prefix;
			new_name += name;
			len = new_name.size();
			if (len > kAddOnFileNameLengthLimit) {
				new_name.erase(prefix.size(), len - kAddOnFileNameLengthLimit);
			}
			if (!names_already_in_use.count(new_name)) {
				name = new_name;
				break;
			}
		}
	}

	assert(FileSystemHelper::is_legal_filename(name));
}

AddOnsPackagerBox::AddOnsPackagerBox(MainMenu& mainmenu, Panel* parent, uint32_t orientation)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, orientation),
     header_align_(0),
     main_menu_(mainmenu) {
}

MapsAddOnsPackagerBox::MapsAddOnsPackagerBox(MainMenu& mainmenu, Panel* parent)
   : AddOnsPackagerBox(mainmenu, parent, UI::Box::Horizontal),
     box_dirstruct_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     last_category_(AddOns::AddOnCategory::kNone),
     box_maps_list_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_buttonsbox_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_dirstruct_displayname_(&box_dirstruct_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
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
     my_maps_(&box_maps_list_, 0, 0, 100, 0, UI::PanelStyle::kFsMenu),
     dirstruct_displayname_(&box_dirstruct_displayname_, 0, 0, 0, UI::PanelStyle::kFsMenu),
     displayname_duplicate_(&box_dirstruct_displayname_,
                            0,
                            0,
                            100,
                            100 /* kNoScrolling does not work in this situation! */,
                            UI::PanelStyle::kFsMenu) {
	box_buttonsbox_.add_inf_space();
	box_buttonsbox_.add(&map_add_);
	box_buttonsbox_.add_space(kSpacing);
	box_buttonsbox_.add(&map_delete_);
	box_buttonsbox_.add_inf_space();
	box_buttonsbox_.add(&map_add_dir_);
	box_buttonsbox_.add_inf_space();

	displayname_duplicate_.set_style(UI::FontStyle::kItalic);
	displayname_duplicate_.set_text(
	   _("The selected internal directory name is used multiple times. All directories with the "
	     "same internal name will also share the same display name."));

	box_dirstruct_displayname_.add(
	   new UI::Textarea(&box_dirstruct_displayname_, UI::PanelStyle::kFsMenu,
	                    UI::FontStyle::kFsGameSetupHeadings, _("Directory Display Name"),
	                    UI::Align::kCenter),
	   UI::Box::Resizing::kFullSize);
	box_dirstruct_displayname_.add_space(kSpacing);
	box_dirstruct_displayname_.add(&dirstruct_displayname_, UI::Box::Resizing::kFullSize);
	box_dirstruct_displayname_.add_space(kSpacing);
	box_dirstruct_displayname_.add(&displayname_duplicate_, UI::Box::Resizing::kFullSize);

	box_dirstruct_.add(new UI::Textarea(&box_dirstruct_, UI::PanelStyle::kFsMenu,
	                                    UI::FontStyle::kFsGameSetupHeadings, _("Directory Tree"),
	                                    UI::Align::kCenter),
	                   UI::Box::Resizing::kFullSize);
	box_dirstruct_.add_space(kSpacing);
	box_dirstruct_.add(&dirstruct_, UI::Box::Resizing::kExpandBoth);
	box_dirstruct_.add_space(kSpacing);
	box_dirstruct_.add(&box_dirstruct_displayname_, UI::Box::Resizing::kFullSize);
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

	my_maps_.selected.connect(
	   [this](uint32_t) { map_add_.set_enabled(dirstruct_.selection_index() > 0); });
	map_add_.set_enabled(false);
	map_delete_.set_enabled(false);
	dirstruct_.selected.connect([this](uint32_t i) {
		map_add_.set_enabled(i > 0 && my_maps_.has_selection());
		map_delete_.set_enabled(i > 0);
		if (i > 0 && !selection_is_map(dirstruct_to_tree_map_[i])) {
			box_dirstruct_displayname_.set_visible(true);
			const std::string& internal_dirname = dirstruct_to_tree_map_[i].back();
			const auto map = selected_->count_all_dirnames();
			const auto it = map.find(internal_dirname);
			displayname_duplicate_.set_visible(it != map.end() && it->second > 1);
			dirstruct_displayname_.set_text(selected_->get_dirname(internal_dirname));
		} else {
			box_dirstruct_displayname_.set_visible(false);
		}
	});

	dirstruct_displayname_.changed.connect([this]() {
		selected_->set_dirname(dirstruct_to_tree_map_[dirstruct_.selection_index()].back(),
		                       dirstruct_displayname_.text());
		if (modified_) {
			modified_();
		}
	});
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
			   format("%s<br>%s<br>%s<br>%s<br>%s",
			          g_style_manager->font_style(UI::FontStyle::kFsTooltipHeader)
			             .as_font_tag(entry.first.filename),
			          format(_("Name: %s"),
			                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			                    .as_font_tag(entry.first.localized_name)),
			          format(_("Size: %s"),
			                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			                    .as_font_tag(
			                       format(_("%1$u×%2$u"), entry.first.width, entry.first.height))),
			          format(_("Players: %s"),
			                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			                    .as_font_tag(std::to_string(entry.first.nrplayers))),
			          format(_("Description: %s"),
			                 g_style_manager->font_style(UI::FontStyle::kFsMenuInfoPanelParagraph)
			                    .as_font_tag(entry.first.description))));
		}
	}

	AddOns::MapsAddon* ma = dynamic_cast<AddOns::MapsAddon*>(a);
	std::vector<std::string> select;
	if (ma->get_tree()->maps.empty() && ma->get_tree()->subdirectories.size() == 1) {
		select.push_back(ma->get_tree()->subdirectories.begin()->first);
	}
	rebuild_dirstruct(ma, select);
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

	if (!dirstruct_.has_selection()) {
		dirstruct_.select(0);
	}
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
	if (selection_is_map(select)) {
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
		std::string filename = FileSystem::fs_filename(map.c_str());
		if (!g_fs->is_directory(map)) {
			UI::WLMessageBox mbox(
			   &main_menu_, UI::WindowStyle::kFsMenu, _("Zipped Map"),
			   format(_("The map ‘%s’ is not a directory. "
			            "Please consider disabling the ‘Compress Widelands data files’ option "
			            "in the options menu and resaving the map in the editor."
			            "\n\nDo you want to add this map anyway?"),
			          filename),
			   UI::WLMessageBox::MBoxType::kOkCancel, UI::Align::kLeft);
			if (mbox.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
				return;
			}
		}
		make_valid_addon_filename(filename, tree->maps);
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

			std::string err = check_addon_filename_validity(name);
			if (!err.empty()) {
				main_menu_.show_messagebox(
				   _("Invalid Name"),
				   format(
				      _("This name is invalid. Reason: %s\n\nPlease choose a different name."), err));
				continue;
			}
			for (const std::string& ext :
			     {kWidelandsMapExtension, kS2MapExtension1, kS2MapExtension2}) {
				if (name.size() >= ext.size() &&
				    name.compare(name.size() - ext.size(), ext.size(), ext) == 0) {
					err = format(_("Directories may not use the extension ‘%s’.\n\nPlease "
					               "choose a different name."),
					             ext);
					break;
				}
			}
			if (!err.empty()) {
				main_menu_.show_messagebox(_("Invalid Name"), err);
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
            format(_("Do you really want to delete the directory ‘%s’ and all its contents?"),
		             select.back()) :
            format(_("Do you really want to delete the map ‘%s’?"), selected_map),
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

	if (modified_) {
		modified_();
	}
	rebuild_dirstruct(selected_, select);
}

CampaignAddOnsPackagerBox::CampaignAddOnsPackagerBox(MainMenu& mainmenu, Panel* parent)
   : AddOnsPackagerBox(mainmenu, parent, UI::Box::Vertical),
     maps_box_(mainmenu, this),
     difficulty_hbox_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     tribe_select_(this,
                   "dropdown_tribe",
                   0,
                   0,
                   50,
                   8,
                   kButtonSize,
                   _("Tribe"),
                   UI::DropdownType::kTextual,
                   UI::PanelStyle::kFsMenu,
                   UI::ButtonStyle::kFsMenuSecondary),
     icon_difficulty_(&difficulty_hbox_,
                      "dropdown_difficulty",
                      0,
                      0,
                      50,
                      8,
                      kButtonSize,
                      _("Difficulty"),
                      UI::DropdownType::kPictorial,
                      UI::PanelStyle::kFsMenu,
                      UI::ButtonStyle::kFsMenuSecondary),
     difficulty_(&difficulty_hbox_, 0, 0, 100, UI::PanelStyle::kFsMenu),
     short_desc_(&difficulty_hbox_, 0, 0, 50, UI::PanelStyle::kFsMenu),
     difficulty_label_(&difficulty_hbox_,
                       UI::PanelStyle::kFsMenu,
                       UI::FontStyle::kFsMenuInfoPanelHeading,
                       _("Difficulty:"),
                       UI::Align::kRight) {
	std::vector<Widelands::TribeBasicInfo> tribeinfos = Widelands::get_all_tribeinfos(nullptr);
	for (const Widelands::TribeBasicInfo& tribeinfo : tribeinfos) {
		tribe_select_.add(tribeinfo.descname, tribeinfo.name, g_image_cache->get(tribeinfo.icon),
		                  false, tribeinfo.tooltip);
	}
	tribe_select_.select(tribeinfos.front().name);

	{
		// Translation provided in data/campaigns/campaigns.lua
		i18n::Textdomain td("maps");
		for (const auto& icon : AddOns::kDifficultyIcons) {
			std::string text(_(icon.first));
			text += " (\"" + icon.first + "\")";
			icon_difficulty_.add(text, icon.second, g_image_cache->get(icon.second), false);
		}
	}

	icon_difficulty_.selected.connect([this]() { edited_difficulty_icon(); });
	difficulty_.changed.connect([this]() { edited_difficulty(); });
	short_desc_.changed.connect([this]() { edited(); });
	tribe_select_.selected.connect([this]() { edited(); });
	maps_box_.set_modified_callback([this]() { edited(); });

	difficulty_.set_tooltip(_("The campaigns difficulty. One word."));
	short_desc_.set_tooltip(_("Short description, which will be appended to the difficulty."));

	difficulty_hbox_.add(&difficulty_label_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	difficulty_hbox_.add_space(kSpacing);
	difficulty_hbox_.add(&icon_difficulty_);
	difficulty_hbox_.add_space(kSpacing);
	difficulty_hbox_.add(&difficulty_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	difficulty_hbox_.add_space(kSpacing);
	difficulty_hbox_.add(&short_desc_, UI::Box::Resizing::kFillSpace, UI::Align::kCenter);

	add(&difficulty_hbox_, UI::Box::Resizing::kFullSize);
	add_space(kSpacing);
	add(&tribe_select_, UI::Box::Resizing::kFullSize);
	add_space(kSpacing);
	add(&maps_box_, UI::Box::Resizing::kExpandBoth);
}

std::string CampaignAddOnsPackagerBox::reverse_icon_lookup(const std::string& value) {
	// Reverse map lookup
	auto result = std::find_if(
	   AddOns::kDifficultyIcons.begin(), AddOns::kDifficultyIcons.end(),
	   [&value](const std::pair<std::string, std::string>& pair) { return pair.second == value; });
	assert(result != AddOns::kDifficultyIcons.end());
	return result->first;
}

void CampaignAddOnsPackagerBox::edited_difficulty_icon() {
	if (difficulty_.text().empty() || difficulty_.text() == last_difficulty_) {
		// Transfer icon to editbox
		last_difficulty_ = reverse_icon_lookup(icon_difficulty_.get_selected());
		difficulty_.set_text(last_difficulty_);
	}
	edited();
}

void CampaignAddOnsPackagerBox::edited_difficulty() {
	if (AddOns::kDifficultyIcons.count(difficulty_.text())) {
		// Transfer editbox to icon
		last_difficulty_ = difficulty_.text();
		icon_difficulty_.select(AddOns::kDifficultyIcons.at(last_difficulty_));
	}
	edited();
}

void CampaignAddOnsPackagerBox::edited() {
	selected_->set_difficulty(difficulty_.text());
	selected_->set_short_desc(short_desc_.text());
	selected_->set_tribe(tribe_select_.get_selected());
	selected_->set_difficulty_icon(icon_difficulty_.get_selected());
	if (modified_) {
		modified_();
	}
}

void CampaignAddOnsPackagerBox::load_addon(AddOns::MutableAddOn* a) {
	assert(a->get_category() == AddOns::AddOnCategory::kCampaign);
	selected_ = dynamic_cast<AddOns::CampaignAddon*>(a);
	const AddOns::CampaignAddon::CampaignInfo& metadata = selected_->get_metadata();
	if (metadata.tribe.empty()) {
		selected_->set_tribe(tribe_select_.get_selected());
	}
	tribe_select_.select(metadata.tribe);

	if (metadata.difficulty.empty()) {
		// The default difficulty. No markup intentionally
		selected_->set_difficulty("Easy.");
	}
	difficulty_.set_text(metadata.difficulty);

	if (metadata.short_desc.empty()) {
		selected_->set_short_desc(selected_->get_descname());
	}
	short_desc_.set_text(metadata.short_desc);

	if (metadata.difficulty_icon.empty()) {
		selected_->set_difficulty_icon(AddOns::kDifficultyIcons.at("Easy."));
	}
	icon_difficulty_.select(metadata.difficulty_icon);
	last_difficulty_ = reverse_icon_lookup(metadata.difficulty_icon);

	maps_box_.load_addon(a);
}

void CampaignAddOnsPackagerBox::layout() {
	if (header_align_) {
		difficulty_label_.set_fixed_width(header_align_);
	}
	AddOnsPackagerBox::layout();
}

}  // namespace AddOnsUI
}  // namespace FsMenu
