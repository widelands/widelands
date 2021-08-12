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

#include "ui_fsmenu/addons/contact.h"

#include <boost/format.hpp>

#include "base/log.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/addons/manager.h"

namespace FsMenu {
namespace AddOnsUI {

ContactForm::ContactForm(AddOnsCtrl& ctrl)
   : UI::Window(&ctrl.get_topmost_forefather(),
                UI::WindowStyle::kFsMenu,
                "contact",
                0,
                0,
                100,
                100,
                _("Contact Form")),
     ctrl_(ctrl),
     box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     buttons_box_(&box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     message_(new UI::MultilineEditbox(&box_, 0, 0, 400, 200, UI::PanelStyle::kFsMenu)),
     ok_(&buttons_box_,
         "ok",
         0,
         0,
         kRowButtonSize,
         kRowButtonSize,
         UI::ButtonStyle::kFsMenuPrimary,
         _("Send message")),
     cancel_(&buttons_box_,
             "cancel",
             0,
             0,
             kRowButtonSize,
             kRowButtonSize,
             UI::ButtonStyle::kFsMenuSecondary,
             _("Cancel")) {
	buttons_box_.add(&cancel_, UI::Box::Resizing::kExpandBoth);
	box_.add_space(kRowButtonSpacing);
	buttons_box_.add(&ok_, UI::Box::Resizing::kExpandBoth);

	box_.add(message_, UI::Box::Resizing::kExpandBoth);
	box_.add_space(kRowButtonSpacing);
	box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);

	check_ok_button_enabled();

	cancel_.sigclicked.connect([this]() { die(); });
	ok_.sigclicked.connect([this]() {
		if (!check_ok_button_enabled()) {
			return;
		}
		UI::WLMessageBox w(&get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Send Message"),
		                   _("Send this message now?"), UI::WLMessageBox::MBoxType::kOkCancel);
		if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
			return;
		}
		try {
			ctrl_.net().contact(message_->get_text());
		} catch (const std::exception& e) {
			log_err("contact error: %s", e.what());
			UI::WLMessageBox m(
			   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Error"),
			   (boost::format(_("Unable to submit your enquiry.\nError message:\n%s")) % e.what())
			      .str(),
			   UI::WLMessageBox::MBoxType::kOk);
			m.run<UI::Panel::Returncodes>();
			return;
		}

		die();
		UI::WLMessageBox m(
		   &get_topmost_forefather(), UI::WindowStyle::kFsMenu, _("Sent"),
		   _("Your message has been sent to the Widelands Development Team, and we will respond "
		     "as soon as we can. Don’t forget to check your website inbox frequently."),
		   UI::WLMessageBox::MBoxType::kOk);
		m.run<UI::Panel::Returncodes>();
	});

	set_center_panel(&box_);
	center_to_parent();

	initialization_complete();
}

void ContactForm::think() {
	UI::Window::think();
	check_ok_button_enabled();
}

bool ContactForm::check_ok_button_enabled() {
	ok_.set_enabled(!message_->get_text().empty());
	return ok_.enabled();
}

}  // namespace AddOnsUI
}  // namespace FsMenu
