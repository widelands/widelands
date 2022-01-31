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

#include "ui_fsmenu/addons/packager.h"

#include <memory>

#include "base/i18n.h"
#include "base/string.h"
#include "base/warning.h"
#include "graphic/image_cache.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "logic/mutable_addon.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progressbar.h"
#include "ui_basic/text_prompt.h"
#include "wlapplication.h"
#include "wlapplication_options.h"

namespace FsMenu {
namespace AddOnsUI {

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
     box_right_subbox_header_box_left_(
        &box_right_subbox_header_hbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_right_subbox_header_box_right_(
        &box_right_subbox_header_hbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     box_right_addon_specific_(&box_right_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     box_right_bottombox_(&box_right_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     name_(&box_right_subbox_header_box_right_, 0, 0, 100, UI::PanelStyle::kFsMenu),
     author_(&box_right_subbox_header_box_right_, 0, 0, 100, UI::PanelStyle::kFsMenu),
     version_(&box_right_subbox_header_box_right_, 0, 0, 100, UI::PanelStyle::kFsMenu),
     descr_(*new UI::MultilineEditbox(
        &box_right_subbox_header_box_right_, 0, 0, 100, 100, UI::PanelStyle::kFsMenu)),
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
     addons_(&box_left_, 0, 0, 250, 0, UI::PanelStyle::kFsMenu),
     update_in_progress_(false),
     progress_window_(this, _("Writing Add-Ons…")) {
	progress_window_.set_visible(false);
	progress_window_.set_message_1(_("Please be patient while your changes are written."));

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
	box_right_subbox_header_box_left_.add(
	   new UI::Textarea(&box_right_subbox_header_box_left_, UI::PanelStyle::kFsMenu,
	                    UI::FontStyle::kFsMenuInfoPanelHeading, _("Description:"),
	                    UI::Align::kRight),
	   UI::Box::Resizing::kFullSize);

	box_right_subbox_header_hbox_.add(
	   &box_right_subbox_header_box_left_, UI::Box::Resizing::kFullSize);
	box_right_subbox_header_hbox_.add_space(kSpacing);
	box_right_subbox_header_hbox_.add(
	   &box_right_subbox_header_box_right_, UI::Box::Resizing::kExpandBoth);

	box_right_bottombox_.add(&discard_changes_, UI::Box::Resizing::kExpandBoth);
	box_right_bottombox_.add_space(kSpacing);
	box_right_bottombox_.add(&write_changes_, UI::Box::Resizing::kExpandBoth);
	box_right_bottombox_.add_space(kSpacing);
	box_right_bottombox_.add(&ok_, UI::Box::Resizing::kExpandBoth);

	box_right_.add(&box_right_subbox_header_hbox_, UI::Box::Resizing::kFullSize);
	box_right_.add_space(kButtonSize);
	box_right_.add(&box_right_addon_specific_, UI::Box::Resizing::kExpandBoth);
	box_right_.add_space(kSpacing);
	box_right_.add(&box_right_bottombox_, UI::Box::Resizing::kFullSize);

	box_left_.add(&addons_, UI::Box::Resizing::kExpandBoth);
	box_left_.add_space(kSpacing);
	box_left_.add(&box_left_buttons_, UI::Box::Resizing::kFullSize);

	main_box_.add(&box_left_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(kSpacing);
	main_box_.add(&box_right_, UI::Box::Resizing::kExpandBoth);

	addon_boxes_[AddOns::AddOnCategory::kMaps] =
	   std::make_shared<MapsAddOnsPackagerBox>(parent, &box_right_addon_specific_);
	addon_boxes_[AddOns::AddOnCategory::kCampaign] =
	   std::make_shared<CampaignAddOnsPackagerBox>(parent, &box_right_addon_specific_);

	for (auto&& pair : addon_boxes_) {
		pair.second->set_modified_callback([this] { current_addon_edited(); });
	}

	addons_.selected.connect([this](uint32_t) { addon_selected(); });
	addon_new_.sigclicked.connect([this]() { clicked_new_addon(); });
	addon_delete_.sigclicked.connect([this]() { clicked_delete_addon(); });
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
	initialization_complete();
}

bool AddOnsPackager::handle_key(const bool down, const SDL_Keysym code) {
	if (down && code.sym == SDLK_RETURN) {
		die();
		return true;
	}
	return Window::handle_key(down, code);
}

void AddOnsPackager::layout() {
	UI::Window::layout();
	AddOns::MutableAddOn* selected = get_selected();
	if (selected) {
		addon_boxes_[selected->get_category()]->set_header_align(
		   box_right_subbox_header_box_left_.get_w());
	}
	if (!is_minimal()) {
		main_box_.set_size(get_inner_w(), get_inner_h());
	}
}

void AddOnsPackager::initialize_mutable_addons() {
	mutable_addons_.clear();
	addons_with_changes_.clear();

	for (const AddOns::AddOnState& a : AddOns::g_addons) {
		mutable_addons_[a.first->internal_name] =
		   AddOns::MutableAddOn::create_mutable_addon(*a.first);
	}

	rebuild_addon_list("");
	check_for_unsaved_changes();
}

void AddOnsPackager::rebuild_addon_list(const std::string& select) {
	addons_.clear();
	for (const auto& pair : mutable_addons_) {
		addons_.add(pair.first, pair.first,
		            g_image_cache->get(AddOns::kAddOnCategories.at(pair.second->get_category()).icon),
		            pair.first == select);
		pair.second->set_callbacks(
		   [this](const size_t i) {
			   progress_window_.progressbar().set_state(0);
			   progress_window_.progressbar().set_total(std::max<size_t>(i, 1));
		   },
		   [this](const size_t i) {
			   progress_window_.progressbar().set_state(progress_window_.progressbar().get_state() +
			                                            i);
			   do_redraw_now();
		   });
	}
	addon_selected();
}

inline AddOns::MutableAddOn* AddOnsPackager::get_selected() {
	return addons_.has_selection() ? mutable_addons_.at(addons_.get_selected()).get() : nullptr;
}

void AddOnsPackager::addon_selected() {
	AddOns::MutableAddOn* selected = get_selected();

	addon_delete_.set_enabled(selected);
	box_right_subbox_header_hbox_.set_visible(false);
	for (auto&& pair : addon_boxes_) {
		pair.second->set_visible(false);
	}
	box_right_addon_specific_.clear();

	if (!selected) {
		return;
	}

	box_right_subbox_header_hbox_.set_visible(true);

	update_in_progress_ = true;
	name_.set_text(selected->get_descname());
	descr_.set_text(selected->get_description());
	author_.set_text(selected->get_author());
	version_.set_text(selected->get_version());
	update_in_progress_ = false;

	if (addon_boxes_.count(selected->get_category()) == 0) {
		return;
	}

	AddOnsPackagerBox* box = addon_boxes_[selected->get_category()].get();
	box_right_addon_specific_.add(box, UI::Box::Resizing::kExpandBoth);
	box->set_visible(true);
	box->load_addon(selected);
	layout();
}

void AddOnsPackager::current_addon_edited() {
	if (update_in_progress_) {
		return;
	}

	const std::string& sel = addons_.get_selected();
	AddOns::MutableAddOn* m = mutable_addons_.at(sel).get();

	m->update_info(name_.text(), author_.text(), descr_.get_text(), version_.text());

	addons_with_changes_[sel] = false;
	check_for_unsaved_changes();
}

void AddOnsPackager::clicked_new_addon() {
	UI::TextPrompt n(main_menu_, UI::WindowStyle::kFsMenu, _("New Add-On"),
	                 _("Enter the name for the new add-on."));
	UI::Dropdown<AddOns::AddOnCategory> category(
	   &n.content_box(), "category", 0, 0, 50, 12, kButtonSize, _("Category"),
	   UI::DropdownType::kTextual, UI::PanelStyle::kFsMenu, UI::ButtonStyle::kFsMenuSecondary);
	for (const auto& pair : AddOns::kAddOnCategories) {
		if (pair.first != AddOns::AddOnCategory::kNone) {
			category.add(pair.second.descname(), pair.first, g_image_cache->get(pair.second.icon),
			             pair.first == AddOns::AddOnCategory::kMaps);
		}
	}
	n.content_box().add(&category, UI::Box::Resizing::kFullSize);
	n.initialization_complete();

	for (;;) {
		if (n.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
			return;
		}

		std::string name = n.text();

		const std::string err = check_addon_filename_validity(name);
		if (!err.empty()) {
			main_menu_.show_messagebox(
			   _("Invalid Name"),
			   format(_("This name is invalid. Reason: %s\n\nPlease choose a different name."), err));
			continue;
		}

		if (name.size() <= kAddOnExtension.size() ||
		    name.compare(
		       name.size() - kAddOnExtension.size(), kAddOnExtension.size(), kAddOnExtension) != 0) {
			// Ensure add-on names always end with '.wad'
			name += kAddOnExtension;
			// The name was legal before, so it should be so still
			assert(check_addon_filename_validity(name).empty());
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

		AddOns::AddOnInfo a;
		// These default strings are not localized because these editboxes are meant to be
		// filled out in English. We will add localization markup to the resulting config file.
		a.internal_name = name;
		a.unlocalized_descname = n.text();
		a.unlocalized_description = "No description";
		a.unlocalized_author = get_config_string("realname", pgettext("author_name", "Unknown"));
		a.version = {1, 0, 0};
		a.category = category.get_selected();

		mutable_addons_[name] = AddOns::MutableAddOn::create_mutable_addon(a);
		addons_with_changes_[name] = false;
		check_for_unsaved_changes();
		rebuild_addon_list(name);
		return;
	}
}

void AddOnsPackager::clicked_delete_addon() {
	const std::string& name = addons_.get_selected();
	UI::WLMessageBox m(
	   get_parent(), UI::WindowStyle::kFsMenu, _("Delete Add-on"),
	   format(ctrl_.is_remote(name) ?
                _("Do you really want to delete the add-on ‘%s’?") :
                _("Do you really want to delete the local add-on ‘%s’?\n\nNote that this "
	               "add-on can not be downloaded again from the server."),
	          name),
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

void AddOnsPackager::die() {
	if (!addons_with_changes_.empty()) {
		std::string msg =
		   format(ngettext(
		             // Comments to fix codecheck false-positive
		             "If you quit the packager now, all changes to the following %u add-on "
		             "will be discarded.",
		             // Comments to fix codecheck false-positive
		             "If you quit the packager now, all changes to the following %u add-ons "
		             "will be discarded.",
		             // Comments to fix codecheck false-positive
		             addons_with_changes_.size()),
		          addons_with_changes_.size());
		for (const auto& str : addons_with_changes_) {
			msg = format(_("%1$s\n· %2$s"), msg, str.first);
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

	std::string msg;
	if (addons_with_changes_.size() == 1) {
		msg = format(_("Do you really want to discard all changes to the add-on ‘%s’?"),
		             addons_with_changes_.begin()->first);
	} else {
		msg =
		   format(ngettext("Do you really want to discard all changes to the following %u add-on?",
		                   "Do you really want to discard all changes to the following %u add-ons?",
		                   addons_with_changes_.size()),
		          addons_with_changes_.size());
		for (const auto& str : addons_with_changes_) {
			msg = format(_("%1$s\n· %2$s"), msg, str.first);
		}
	}

	UI::WLMessageBox m(&main_menu_, UI::WindowStyle::kFsMenu, _("Discard Changes"), msg,
	                   UI::WLMessageBox::MBoxType::kOkCancel, UI::Align::kLeft);
	if (m.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
		initialize_mutable_addons();
	}
}

void AddOnsPackager::clicked_write_changes() {
	assert(!addons_with_changes_.empty());

	std::string msg;
	if (addons_with_changes_.size() == 1) {
		msg = format(_("Do you really want to commit all changes to the add-on ‘%s’ to disk?"),
		             addons_with_changes_.begin()->first);
	} else {
		msg = format(
		   ngettext("Do you really want to commit all changes to the following %u add-on to disk?",
		            "Do you really want to commit all changes to the following %u add-ons to disk?",
		            addons_with_changes_.size()),
		   addons_with_changes_.size());
		for (const auto& str : addons_with_changes_) {
			msg = format(_("%1$s\n· %2$s"), msg, str.first);
		}
	}
	msg += "\n\n";
	msg += _("Warning: If you manually edited any files in this add-on, your changes may be "
	         "overwritten!");

	UI::WLMessageBox m(&main_menu_, UI::WindowStyle::kFsMenu, _("Confirm Saving"), msg,
	                   UI::WLMessageBox::MBoxType::kOkCancel, UI::Align::kLeft);
	if (m.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
		progress_window_.center_to_parent();
		progress_window_.move_to_top();
		progress_window_.set_visible(true);
		std::set<std::string> errors;
		for (const auto& pair : addons_with_changes_) {
			progress_window_.set_message_2(pair.first);
			do_redraw_now();
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
		progress_window_.set_message_2("");

		// Clear list of changes, and re-insert failed add-ons
		addons_with_changes_.clear();
		for (const std::string& e : errors) {
			addons_with_changes_[e] = false;
		}
		check_for_unsaved_changes();

		// Refresh packager box (some options are not always available)
		addon_selected();

		// Update the global catalogue
		WLApplication::initialize_g_addons();

		progress_window_.set_visible(false);
	}
}

bool AddOnsPackager::do_write_addon_to_disk(const std::string& addon) {
	AddOns::MutableAddOn* m = mutable_addons_.at(addon).get();

	// Check that the version string is valid and beautify it
	try {
		AddOns::AddOnVersion v = AddOns::string_to_version(m->get_version());
		m->set_version(AddOns::version_to_string(v, false));
		if (addons_.has_selection() && addons_.get_selected() == addon) {
			version_.set_text(m->get_version());
		}
	} catch (...) {
		main_menu_.show_messagebox(
		   _("Invalid Version"),
		   format(_("‘%1$s’ is not a valid version string. The add-on ‘%2$s’ will not be saved."),
		          m->get_version(), addon));
		return false;
	}

	try {
		return m->write_to_disk();
	} catch (const WLWarning& e) {
		main_menu_.show_messagebox(
		   _("Error Writing Addon"),
		   format(_("The add-on ‘%1$s’ can not be saved to disk:\n%2$s"), addon, e.what()));
		return false;
	}
}

}  // namespace AddOnsUI
}  // namespace FsMenu
