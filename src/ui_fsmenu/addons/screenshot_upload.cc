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

#include "ui_fsmenu/addons/screenshot_upload.h"

#include <memory>

#include "base/log.h"
#include "graphic/image_cache.h"
#include "io/filesystem/layered_filesystem.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/addons/manager.h"

namespace FsMenu {
namespace AddOnsUI {

ScreenshotUploadWindow::ScreenshotUploadWindow(AddOnsCtrl& ctrl,
                                               std::shared_ptr<AddOns::AddOnInfo> info,
                                               std::shared_ptr<AddOns::AddOnInfo> remote)
   : UI::Window(&ctrl.get_topmost_forefather(),
                UI::WindowStyle::kFsMenu,
                "upload_screenshot",
                0,
                0,
                100,
                100,
                format(_("Upload Screenshot for ‘%s’"), info->internal_name)),
     box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     hbox_(&box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     vbox_(&hbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     descrbox_(&vbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     buttons_box_(&box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     ok_(&buttons_box_,
         "ok",
         0,
         0,
         7 * kRowButtonSize,
         kRowButtonSize,
         UI::ButtonStyle::kFsMenuPrimary,
         _("Upload")),
     cancel_(&buttons_box_,
             "cancel",
             0,
             0,
             7 * kRowButtonSize,
             kRowButtonSize,
             UI::ButtonStyle::kFsMenuSecondary,
             _("Cancel")),
     images_(&hbox_, 0, 0, 150, 200, UI::PanelStyle::kFsMenu),
     icon_(&vbox_, UI::PanelStyle::kFsMenu, 0, 0, 640, 360, nullptr),
     description_(&descrbox_, 0, 0, 300, UI::PanelStyle::kFsMenu),
     progress_(&buttons_box_,
               UI::PanelStyle::kFsMenu,
               UI::FontStyle::kFsMenuLabel,
               "",
               UI::Align::kCenter) {
	buttons_box_.add(&cancel_, UI::Box::Resizing::kFullSize);
	buttons_box_.add(&progress_, UI::Box::Resizing::kFillSpace, UI::Align::kCenter);
	buttons_box_.add(&ok_, UI::Box::Resizing::kFullSize);

	descrbox_.add(
	   new UI::Textarea(&descrbox_, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsMenuInfoPanelHeading,
	                    _("Description:"), UI::Align::kRight),
	   UI::Box::Resizing::kAlign, UI::Align::kCenter);
	descrbox_.add_space(kRowButtonSpacing);
	descrbox_.add(&description_, UI::Box::Resizing::kExpandBoth);

	vbox_.add(&icon_, UI::Box::Resizing::kExpandBoth);
	vbox_.add_space(kRowButtonSpacing);
	vbox_.add(&descrbox_, UI::Box::Resizing::kExpandBoth);

	hbox_.add(&images_, UI::Box::Resizing::kExpandBoth);
	hbox_.add_space(kRowButtonSpacing);
	hbox_.add(&vbox_, UI::Box::Resizing::kExpandBoth);

	box_.add(&hbox_, UI::Box::Resizing::kFullSize);
	box_.add_space(kRowButtonSpacing);
	box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);

	description_.set_tooltip(_("Description"));
	for (const std::string& img : g_fs->list_directory(kScreenshotsDir)) {
		images_.add(FileSystem::fs_filename(img.c_str()), img);
	}

	images_.selected.connect(
	   [this](uint32_t /* value */) { icon_.set_icon(g_image_cache->get(images_.get_selected())); });
	cancel_.sigclicked.connect([this]() { die(); });
	ok_.sigclicked.connect([this, &ctrl, info, remote]() {
		if (!images_.has_selection() || description_.text().empty()) {
			return;
		}
		const std::string& sel = images_.get_selected();
		progress_.set_text(_("Uploading…"));
		ok_.set_enabled(false);
		cancel_.set_enabled(false);
		do_redraw_now();
		try {
			ctrl.net().upload_screenshot(info->internal_name, sel, description_.text());
			if (remote != nullptr) {
				*remote = ctrl.net().fetch_one_remote(remote->internal_name);
				ctrl.rebuild(false);
			}
			die();
		} catch (const std::exception& e) {
			log_err(
			   "Upload screenshot %s for %s: %s", sel.c_str(), info->internal_name.c_str(), e.what());
			progress_.set_text("");
			UI::WLMessageBox m(
			   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Error"),
			   format(_("The screenshot ‘%1$s’ for the add-on ‘%2$s’ could not be uploaded "
			            "to the server.\n\nError Message:\n%3$s"),
			          sel, info->internal_name, e.what()),
			   UI::WLMessageBox::MBoxType::kOk);
			m.run<UI::Panel::Returncodes>();
			ok_.set_enabled(true);
			cancel_.set_enabled(true);
		}
	});

	set_center_panel(&box_);
	center_to_parent();

	initialization_complete();
}

void ScreenshotUploadWindow::think() {
	ok_.set_enabled(images_.has_selection() && !description_.text().empty());
	UI::Window::think();
}

}  // namespace AddOnsUI
}  // namespace FsMenu
